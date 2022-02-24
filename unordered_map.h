#pragma once
#include <functional>
#include <type_traits>
#include <vector>
#include <cassert>
#include <memory>

template<typename T, typename Allocator = std::allocator<T>>
class List {
public:
    explicit List(const Allocator& _alloc = Allocator()): root(nullptr), sz(0), alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(_alloc)) {
        makeEnd();
    }
    
    List(size_t count, const T& val, const Allocator& _alloc = Allocator());
    List(size_t count, const Allocator& _alloc = Allocator());
    List(const List& other);
    List(List&& other);
    ~List();

    Allocator get_allocator() const {
        return alloc;
    }
    
    size_t size() const {
        return sz;
    }

    List& operator=(const List& other);
    List& operator = (List&& other);

    template <bool isConst>
    class commonIterator;
    
    using iterator = commonIterator<false>;
    using const_iterator = commonIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return root -> right;
    }

    iterator end() {
        return root;
    }

    const_iterator cbegin() const {
        return root -> right;
    }

    const_iterator cend() const  {
        return root;
    }

    const_iterator begin() const {
        return cbegin();
    }   

    const_iterator end() const {
        return cend();
    }
    
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }
    
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(cend());
    }
    
    const_reverse_iterator rend() const {
        return const_reverse_iterator(cend());
    }
    
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    
    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }
    
    void insert(const_iterator it, const T& val);
    void erase(const_iterator it);

    void push_back(const T& val) {
        insert(end(), val);
    }
    
    void push_front(const T& val) {
        insert(begin(), val);
    }
    
    void pop_back() {
        erase(--end());
    }

    void pop_front() {
        erase(begin());
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* x = alloc.allocate(1);
        alloc_t.construct(&x->val, std::forward<Args>(args)...);
        sz++;
        x -> left = root -> left;
        x -> right = root;
        root -> left -> right = x;
        root -> left = x;
    }

private:
    void makeEnd();

    void destructEnd() {
        alloc.deallocate(root, 1);
    }

    struct Node;

    Node* root;
    size_t sz;

    using AllocatorNode = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    AllocatorNode alloc;
    Allocator alloc_t;
};

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& val, const Allocator& _alloc): List(_alloc) {
    for (size_t i = 0; i < count; ++i) {
        push_back(val);
    }
}

template<typename T, typename Allocator>  
List<T, Allocator>::List(size_t count, const Allocator& _alloc): List(_alloc) {
    if (!count) {
        return;
    }
    Node* prev = root;
    Node* ptr = nullptr;
    for (sz = 0; sz < count; ++sz) {
        ptr = alloc.allocate(1);
        alloc.construct(ptr);

        ptr -> left = prev;
        prev -> right = ptr;
            
        prev = ptr;
    }
    ptr -> right = root;
    root -> left = ptr;     
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const List& other) :
    List(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator()))  {

    for (auto it = other.begin(); it != other.end(); ++it) {
        push_back(*it);
    }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(List&& other) {
    sz = other.sz;
    root = other.root;
    alloc = std::move(other.alloc);
    other.sz = 0;
    other.makeEnd();
}

template<typename T, typename Allocator>
List<T, Allocator>::~List() {
    while(sz != 0) {
        pop_back();
    }
    destructEnd();
}

template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
    while(sz != 0) {
        pop_back();
    }
    for (auto it = other.begin(); it != other.end(); ++it) {
        push_back(*it);
    }
    if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
        alloc = other.get_allocator();
    }
    return *this;
}

template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator = (List&& other) {
    while(sz != 0) {
        pop_back();
    }
    destructEnd();
    sz = other.sz;
    root = other.root;
    other.sz = 0;
    other.makeEnd();
    return *this;
}

template<typename T, typename Allocator>
void List<T, Allocator>::insert(const_iterator it, const T& val) {
    ++sz;
    Node* ptr_new = alloc.allocate(1);
    Node* ptr_old = it.getPtr();
        
    alloc.construct(ptr_new, val);
        
    ptr_new -> left  = ptr_old -> left;
    ptr_new -> right = ptr_old;
        
    ptr_old -> left -> right = ptr_new;
    ptr_old -> left          = ptr_new;
}

template<typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator it) {
    --sz;
    Node* ptr = it.getPtr();
        
    ptr -> left -> right = ptr -> right;
    ptr -> right -> left = ptr -> left;

    alloc.destroy(ptr);
    alloc.deallocate(ptr, 1);
}

template<typename T, typename Allocator>
void List<T, Allocator>::makeEnd() {
    root = alloc.allocate(1);
    root -> left = root;
    root -> right = root;
}

template<typename T, typename Allocator>
struct List<T, Allocator>::Node {
    T val;

    Node* left;
    Node* right;
    
    Node(): left(nullptr), right(nullptr) {}
    Node(const T& _val):
        val(_val), left(nullptr), right(nullptr) {}
};

template<typename T, typename Allocator>
template <bool isConst>
class List<T,Allocator>::commonIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional<isConst, const T, T>;
    using pointer = typename std::conditional<isConst, const T *, T *>::type;
    using reference = typename std::conditional<isConst, const T &, T &>::type;
    using difference_type = std::ptrdiff_t;

    operator commonIterator<true>() {
        return commonIterator<true>(ptr);
    }

    commonIterator(Node* other) : ptr(other) {}
    commonIterator(const commonIterator& other): ptr(other.ptr) {}

    commonIterator& operator = (const commonIterator& other) {
        ptr = other.ptr;
        return *this;
    }

    commonIterator& operator ++() {
        ptr = ptr -> right;
        return *this;
    }

    commonIterator& operator --() {
        ptr = ptr -> left;
        return *this;
    }

    commonIterator operator ++(int) {
        commonIterator n(*this);
        ++(*this);
        return n;
    }

    commonIterator operator --(int) {
        commonIterator n(*this);
        --(*this);
        return n;
    }

    bool operator == (const commonIterator& other) const {
        return ptr == other.ptr;
    }

    bool operator != (const commonIterator& other) const {
        return !((*this) == other);
    }

    reference operator *() const {
        return ptr -> val;
    }

    pointer operator -> () const {
        return &(ptr -> val);
    }

    commonIterator getIter() const {
        return *this;
    }

private:
    Node* getPtr() const {return ptr;}
    
    Node* ptr;

    friend class List<T, Allocator>;
};

template <typename Key, typename Value, 
          typename Hash  = std::hash<Key>, 
          typename Equal = std::equal_to<Key>, 
          typename Alloc = std::allocator<std::pair<const Key, Value>>
         >
class UnorderedMap {
public:
    using NodeType          = std::pair<const Key, Value>;
    using Iterator          = typename List<NodeType, Alloc>::iterator;
    using ConstIterator     = typename List<NodeType, Alloc>::const_iterator;

    UnorderedMap(): table(1) {}

    UnorderedMap(const UnorderedMap& other): data(other.data) {
        rehash();
    }

    UnorderedMap(UnorderedMap&& other): table(std::move(other.table)), data(std::move(other.data)) {}

    UnorderedMap& operator = (const UnorderedMap& other);
    UnorderedMap& operator = (UnorderedMap&& other);
    Value& operator[] (const Key& key);
    Value& at(const Key& key);
    const Value& at(const Key& key) const;

    size_t size() const {
        return data.size();
    }

    Iterator begin() {
        return data.begin();
    }

    Iterator end() {
        return data.end();
    }

    Iterator begin() const {
        return data.cbegin();
    }

    Iterator end() const {
        return data.cend();
    }

    ConstIterator cbegin() const {
        return data.cbegin();
    }

    ConstIterator cend() const {
        return data.cend();
    }

    Iterator find(const Key& key);
    template<typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args);
    void erase(ConstIterator it);
    void erase(ConstIterator from, ConstIterator to);

    std::pair<Iterator, bool> insert(const NodeType& val) {
        return emplace(val);
    }

    std::pair<Iterator, bool> insert(NodeType&& val) {
        return emplace(std::forward<NodeType>(val));
    }

    template <class P>
    std::pair<Iterator, bool> insert(P&& value) {
        return emplace(std::forward<P>(value));
    }

    template<typename InputIterator>
    void insert(InputIterator from, InputIterator to);
    void reserve(size_t sz);

    size_t max_size() {
        return data.size() * max_load_factor_value;
    }

    double load_factor() {
        return data.size() / static_cast<double>(table.size());
    }

    double maxLoadFactor() {
        return max_load_factor_value;
    }

private:
    using Alloc_chain = typename std::allocator_traits<Alloc>::template rebind_alloc<Iterator>;
    using Alloc_table = typename std::allocator_traits<Alloc>::template rebind_alloc<std::vector<Iterator, Alloc_chain>>;

    void rehash();
    void fix();
    std::pair<size_t, size_t> find_pos(const Key& key) const;

    double max_load_factor_value = 1;

    std::vector<std::vector<Iterator, Alloc_chain>, Alloc_table> table;
    List<NodeType, Alloc> data;
};

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator = (const UnorderedMap& other) {
    data = other.data;
    rehash();
    return *this;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator = (UnorderedMap&& other) {
    data = std::move(other.data);
    table = std::move(other.table);
    return *this;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[] (const Key& key) {
    Iterator it = find(key);
    if (it == end()) {
        it = emplace(NodeType(key, Value())).first;
    }
    return it -> second;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key) {
    Iterator it = find(key);
    if (it == end()) {
        throw std::out_of_range("Out of range");
    }
    return it -> second;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
const Value& UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key& key) const {
    ConstIterator it = find(key);
    if (it == cend()) {
        throw std::out_of_range("Out of range");
    }
    return it -> second;
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator 
  UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) {
    std::pair<size_t, size_t> pos = find_pos(key);

    if (pos.first == table.size()) {
        return end();
    }

    return table[pos.first][pos.second];
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::Iterator, bool> 
  UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args&&... args) {
    data.emplace_back(std::forward<Args>(args)...);
        
    Iterator it = end();
    --it;

    if (find(it -> first) != end()) {
        data.pop_back();
        return {end(), false};
    }

    size_t direct_adress = Hash()(it -> first) % table.size();
    table[direct_adress].push_back(it);

    fix();

    return {it, true};
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::ConstIterator it) {
    size_t direct_adress = Hash()(it->first) % table.size();
    for (size_t i = 0; i < table[direct_adress].size(); ++i) {
        if (Equal()(table[direct_adress][i]->first, it->first)) {
            std::swap(table[direct_adress][i], table[direct_adress][table[direct_adress].size() - 1]);
            table[direct_adress].pop_back();
            break;
        }
    }
    data.erase(it);
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(ConstIterator from, ConstIterator to) {
    while (from != to) {
        erase(from++);
    }
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename InputIterator>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(InputIterator from, InputIterator to) {
    while (from != to) {
        emplace(*from++);
    }
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t sz) {
    if (table.size() >= sz) {
        return;
    }

    table.resize(sz);
    rehash();
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
std::pair<size_t, size_t> UnorderedMap<Key, Value, Hash, Equal, Alloc>::find_pos(const Key& key) const {
    if (data.size() == 0) {
        return {table.size(), 0};
    }

    size_t direct_adress = Hash()(key) % table.size();
    for (size_t i = 0; i < table[direct_adress].size(); ++i) {
        if (Equal()(table[direct_adress][i] -> first, key)) {
            return {direct_adress, i};
        }
    }
    
    return {table.size(), 0};
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::fix() {
    if (table.size() < data.size() * max_load_factor_value) {
        rehash();
    }
}

template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash() {
        size_t cur_size = table.size();

        while (cur_size < data.size() * max_load_factor_value) {
            cur_size <<= 1;
            cur_size += 1;      
        }

        table.clear();
        table.resize(cur_size);

        for (auto i = data.begin(); i != data.end(); ++i) {
            size_t direct_adress = Hash()(i -> first) % table.size();
            table[direct_adress].push_back(i);
        }
}
