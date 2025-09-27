#include <iostream> 
#include <vector>
using namespace std;
// Open-addressing hash table

template <typename keyType, typename ValueType>
class OpenAddressingHashTable
{
private:
    struct Node
    {
        keyType key_;
        ValueType value_;
    };
    std::vector<Node> table_;
    size_t current_size_; // dont expect the size to be negative
    size_t capacity_;

    size_t hash(const keyType& key) const 
    {
        return std::hash<keyType>()(key)%capacity_;
    }

    void expand()
    {
        size_t old_capacity = capacity_;
        capacity_ *= 2;

        std::vector<Node> new_table(capacity_);

        for(size_t i = 0; i < old_capacity; ++i)
        {
            if 
        }
    }

public:
    OpenAddressingHashTable(size_t initial_capacity = 16)
    {

    }
    
    void insert(const keyType& key, const ValueType& value){

    }
    {

    }

    bool contains(const keyType& key, const ValueType& value)
    {

    }

    bool remove() 
    {
        return false;
    }

    size_t get_size() const { return current_size_; }

    size_t get_capacity() const { return capacity_; }

};



int main() {
    return 0;
}