# python/websocket_app.py
import sys
import os
import json
import time
import threading
from datetime import datetime
from flask import Flask, render_template, jsonify, request
from flask_socketio import SocketIO, emit

# Add build directory
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../build'))

try:
    import almgren_chriss
    print("Almgren-Chriss module loaded successfully!")
except ImportError as e:
    print(f"Failed to import module: {e}")
    sys.exit(1)

# Create Flask app
app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret-key-123' # Dummy secret key for Flask sessions
socketio = SocketIO(app, cors_allowed_origins="*")

# Global engine and orders
engine = None
orders = {}  # {order_id: {'order': Order, 'status': str, 'metrics': dict, 'progress': list}}
next_order_num = 1

def init_engine():
    global engine
    engine = almgren_chriss.TradingEngine()
    engine.initialize()
    print("✅ TradingEngine initialized")

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/orders', methods=['GET'])
def get_orders():
    """Get all orders"""
    orders_list = []
    for order_id, data in orders.items():
        orders_list.append({
            'id': order_id,
            'symbol': data['order'].symbol,
            'total_shares': data['order'].total_shares,
            'side': 'BUY' if data['order'].is_buy else 'SELL',
            'initial_price': data['order'].initial_price,
            'status': data['status'],
            'progress': data.get('progress', 0),
            'metrics': data.get('metrics', {})
        })
    return jsonify(orders_list)

@app.route('/api/orders', methods=['POST'])
def create_order():
    """Create a new order"""
    global next_order_num
    
    data = request.json
    order = almgren_chriss.Order()
    order.symbol = data.get('symbol', 'AAPL')
    order.total_shares = int(data.get('quantity', 1000))
    order.is_buy = data.get('side', 'SELL') == 'BUY'
    order.initial_price = float(data.get('price', 150.0))
    order.time_horizon = float(data.get('time_horizon', 30.0))
    order.risk_aversion = float(data.get('risk_aversion', 1.0))
    
    # Submit order
    order_id = engine.submit_order(order)
    
    # Store order data
    orders[order_id] = {
        'order': order,
        'status': 'PENDING',
        'progress': 0,
        'metrics': {
            'total_shares': order.total_shares,
            'executed_shares': 0,
            'average_price': 0,
            'implementation_shortfall': 0
        },
        'history': []
    }
    
    # Broadcast new order
    socketio.emit('order_created', {
        'id': order_id,
        'symbol': order.symbol,
        'total_shares': order.total_shares,
        'side': 'BUY' if order.is_buy else 'SELL',
        'status': 'PENDING'
    })

    # threading.Thread(target=execute_order, args=(order_id,), daemon=True).start()
    
    return jsonify({'id': order_id, 'message': 'Order created'})

@app.route('/api/orders/<order_id>/start', methods=['POST'])
def start_order(order_id):
    """Start execution of an order"""
    if order_id not in orders:
        return jsonify({'error': 'Order not found'}), 404
    
    try:
        # Start execution in background thread
        socketio.start_background_task(execute_order, order_id)
        return jsonify({'message': 'Execution started'})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/orders/<order_id>', methods=['DELETE'])
def cancel_order(order_id):
    """Cancel an order"""
    if order_id not in orders:
        return jsonify({'error': 'Order not found'}), 404
    
    try:
        engine.cancel_order(order_id)
        orders[order_id]['status'] = 'CANCELLED'
        
        socketio.emit('order_updated', {
            'id': order_id,
            'status': 'CANCELLED'
        })
        
        return jsonify({'message': 'Order cancelled'})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/stats', methods=['GET'])
def get_stats():
    """Get engine statistics"""
    active_orders = sum(1 for o in orders.values() if o['status'] in ['PENDING', 'ACTIVE'])
    completed_orders = sum(1 for o in orders.values() if o['status'] == 'COMPLETED')
    total_shares = sum(o['order'].total_shares for o in orders.values())
    
    return jsonify({
        'active_orders': active_orders,
        'completed_orders': completed_orders,
        'total_orders': len(orders),
        'total_shares': total_shares
    })

def execute_order(order_id):
    """Start order execution - C++ callbacks will handle updates"""
    if order_id not in orders:
        return
    
    try:
        print(f"🚀 Starting execution for order: {order_id}")
        
        # Emit status update via WebSocket
        orders[order_id]['status'] = 'ACTIVE'
        socketio.emit('order_status_changed', {
            'order_id': order_id,
            'status': 'ACTIVE'
        })
        
        # Start execution in C++ engine
        # This will trigger C++ callbacks which will emit WebSocket events
        engine.start_execution(order_id)
        
        print(f"✅ Execution started for {order_id}. C++ callbacks will handle updates.")
        
    except Exception as e:
        print(f"❌ Error starting execution for {order_id}: {e}")
        socketio.emit('order_status_changed', {
            'order_id': order_id,
            'status': 'ERROR',
            'error': str(e)
        })

def setup_callbacks():
    """Setup callbacks from C++ to Python"""
    
    def execution_callback(order_id, symbol, shares, price, total_executed, total_shares):
        """Called from C++ when a trade chunk executes"""
        progress = (total_executed / total_shares * 100) if total_shares > 0 else 0
        # Emit trade execution event
        socketio.emit('trade_executed', {
            'order_id': order_id,
            'symbol': symbol,
            'shares': shares,
            'price': price,
            'total_executed': total_executed,
            'total_shares': total_shares,
            'progress': progress,
            'timestamp': datetime.now().isoformat()
        })
        
        # Also emit progress update
        socketio.emit('order_progress', {
            'order_id': order_id,
            'progress': progress,
            'total_executed': total_executed,
            'total_shares': total_shares,
            'timestamp': datetime.now().isoformat()
        })
        
        print(f"⚡ C++ Execution: {order_id} - {shares:.0f} shares of {symbol} @ ${price:.2f} ({progress:.1f}%)")
    
    def status_callback(order_id, status_code):
        """Called from C++ when order status changes"""
        status_map = {
            0: 'PENDING',
            1: 'ACTIVE', 
            2: 'COMPLETED',
            3: 'CANCELLED'
        }
        status = status_map.get(status_code, 'UNKNOWN')
        
        # Update local order status
        if order_id in orders:
            orders[order_id]['status'] = status
        
        # Emit status change event
        socketio.emit('order_status_changed', {
            'order_id': order_id,
            'status': status,
            'timestamp': datetime.now().isoformat()
        })
        
        print(f"🔄 C++ Status: {order_id} -> {status}")
    
    def progress_callback(order_id, progress):
        """Called from C++ when progress updates"""
        # Emit progress update
        socketio.emit('order_progress', {
            'order_id': order_id,
            'progress': progress,
            'timestamp': datetime.now().isoformat()
        })
        
        print(f"📊 C++ Progress: {order_id} -> {progress:.1f}%")
    
    # Register callbacks with C++ engine
    engine.set_execution_callback(execution_callback)
    engine.set_status_callback(status_callback)
    engine.set_progress_callback(progress_callback)
    print("✅ Callbacks registered with C++ engine")

@socketio.on('connect')
def handle_connect():
    print('Client connected')
    emit('connected', {'message': 'Connected to Almgren-Chriss server', 'time': datetime.now().isoformat()})

@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')

if __name__ == '__main__':
    # Initialize engine
    init_engine()
    setup_callbacks()
    # Check if templates directory exists
    templates_dir = os.path.join(os.path.dirname(__file__), 'templates')
    if not os.path.exists(templates_dir):
        os.makedirs(templates_dir, exist_ok=True)
        print(f"⚠️ Created templates directory: {templates_dir}")
    
    # Check if index.html exists
    index_path = os.path.join(templates_dir, 'index.html')
    if not os.path.exists(index_path):
        print(f"❌ index.html not found at: {index_path}")
        print("Please make sure templates/index.html exists")
        exit(1)
    else:
        print(f"✅ Found index.html at: {index_path}")
    
    print("🚀 Starting Almgren-Chriss WebSocket App on http://localhost:5000")
    print("📊 Open your browser to: http://localhost:5000")
    
    # Use host='0.0.0.0' if you want to access from other devices
    socketio.run(app, 
                 debug=True, 
                 port=5000, 
                 allow_unsafe_werkzeug=True,
                 host='127.0.0.1')  # Change to '0.0.0.0' for network access