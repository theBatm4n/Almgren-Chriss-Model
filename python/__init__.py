"""
Almgren-Chriss Python Interface
===============================
Python bindings for the Almgren-Chriss optimal execution engine.

Usage:
    >>> from almgren_chriss import TradingEngine, Order
    >>> engine = TradingEngine()
    >>> engine.initialize()
"""

from .almgren_chriss import (
    TradingEngine, Order, OrderStatus, ExecutionMetrics,
    PENDING, ACTIVE, COMPLETED, CANCELLED
)

__version__ = "1.0.0"
__all__ = [
    'TradingEngine', 
    'Order', 
    'OrderStatus', 
    'ExecutionMetrics',
    'PENDING', 'ACTIVE', 'COMPLETED', 'CANCELLED'
]