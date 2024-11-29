#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <bitset>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator;
  //using key_compare = std::less<key_type>;                         // B+-Tree
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing
private:
    struct Bucket;
    Bucket** buckets;
    size_type roundNumber;
    size_type nextToSplit;
    size_type tableSize;
    size_type tableCapacity;
    size_type numOfKeys;
public:
  ADS_set(): roundNumber{1}, nextToSplit{0}, tableSize{2}, tableCapacity{4}, numOfKeys{0} {
    buckets = new Bucket*[tableCapacity];
    buckets[0] = new Bucket;
    buckets[1] = new Bucket;
  }                                           

  ADS_set(std::initializer_list<key_type> ilist): ADS_set{std::begin(ilist),std::end(ilist)} {}

  template<typename InputIt> 
  ADS_set(InputIt first, InputIt last): ADS_set{} {
    insert(first, last);
  } 

  ADS_set(const ADS_set &other): ADS_set(other.begin(), other.end()) {}

  ~ADS_set() {
    for (size_type i{0}; i < tableSize; i++) delete buckets[i];
    delete[] buckets;
  }

  ADS_set &operator=(const ADS_set &other) {
    if (this == &other) return *this;
    ADS_set temp{other};
    swap(temp);
    return *this;
  }
  
  ADS_set &operator=(std::initializer_list<key_type> ilist) {
    ADS_set tmp{ilist};
    swap(tmp);
    return *this;
  }

  size_type size() const {
    return numOfKeys;
  }                                              

  bool empty() const {
    return numOfKeys == 0;
  }                                              

  void insert(std::initializer_list<key_type> ilist) {
    insert(std::begin(ilist),std::end(ilist));
  }

  std::pair<iterator,bool> insert(const key_type &key) {
    size_type x = static_cast<size_type>(getIndex(key));
    int y = buckets[x]->find(key);

    if (y != -1) {
      return std::make_pair(Iterator(buckets, tableSize, x, static_cast<size_type>(y)), false);
    }

    numOfKeys++;
    if (buckets[x]->append(key)) {
      split();
      return std::make_pair(find(key), true);
    };

    return std::make_pair(Iterator(buckets, tableSize, x, buckets[x]->sz-1), true);
  }

  template<typename InputIt> void insert(InputIt first, InputIt last) {
    for (InputIt it {first}; it != last; it++) {
        const key_type& key = *it;
        if (count(key) != 0) continue;
        if (buckets[getIndex(key)]->append(key)) split();
        numOfKeys++;
    }
  } 

  void clear() {
    ADS_set temp;
    this->swap(temp);
  }

  size_type erase(const key_type &key) {
    size_type res = buckets[getIndex(key)]->erase(key);
    if (res == 1) {
      numOfKeys--;
    };
    return res;
  }

  size_type count(const key_type &key) const {
    return buckets[getIndex(key)]->count(key);
  }

  iterator find(const key_type &key) const {
    size_type x = static_cast<size_type>(getIndex(key));
    int y = buckets[x]->find(key);

    if (y == -1) return end();
    return Iterator(buckets, tableSize, x, static_cast<size_type>(y));
  }

  void swap(ADS_set &other) {
    std::swap(buckets, other.buckets);
    std::swap(roundNumber, other.roundNumber);
    std::swap(nextToSplit, other.nextToSplit);
    std::swap(tableSize, other.tableSize);
    std::swap(tableCapacity, other.tableCapacity);
    std::swap(numOfKeys, other.numOfKeys);
  }

  const_iterator begin() const {
    return Iterator(buckets, tableSize);
  }
  const_iterator end() const {
    return Iterator();
  }

  void dump(std::ostream &o = std::cerr) const {
    o << "[size=" << numOfKeys << "]\n";

    for (size_type i{0}; i < tableSize; i++) {
      std::string index{std::bitset<64>( i ).to_string()};
      if (i < nextToSplit || i > ((1u << roundNumber) - 1)) 
        index = index.substr(index.size() - (roundNumber + 1));
      else index = ' ' + index.substr(index.size() - roundNumber);
      o << index << " : ";

      Bucket* b{buckets[i]};

      for (size_type i{0}; i < b->sz; i++) {
        o << b->keys[i] << ' ';
        if ((i+1) % N == 0 && i < b->sz - 1) {
            o << "-> ";
        }
      }

      o << "\n";
    }
  }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
      if (lhs.numOfKeys != rhs.numOfKeys) return false;
      for (const auto& key : lhs) {
        if (!rhs.count(key)) {
          return false;
        }
      };

      return true;
    }

    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
      return !(lhs == rhs);
    }

    unsigned getIndex(const key_type& key) const {
        unsigned index = static_cast<unsigned>(hasher{}(key)) & ((1u << roundNumber) - 1);
        if (index < nextToSplit) index = static_cast<unsigned>(hasher{}(key)) & ((1u << (roundNumber+1)) - 1);;
        return index;
    }

    void split() {
      if (tableSize == tableCapacity) {
        tableCapacity *= 2;
        Bucket** newBuckets = new Bucket*[tableCapacity];

        for (size_type i{0}; i < tableSize; i++) {
            newBuckets[i] = buckets[i];
        }

        delete[] buckets;
        buckets = newBuckets;
      }

      nextToSplit++;
      buckets[tableSize++] = new Bucket;

      auto& b = buckets[nextToSplit-1];
      for (size_type i {0}; i < b->sz; i++) {
        unsigned index = getIndex(b->keys[i]);
        if (index != nextToSplit-1) {
          buckets[index]->append(b->keys[i]);
          b->keys[i] = b->keys[b->sz-1];
          b->sz--;
          i--;
        } 
      }

      //b->shrinkIfTooLarge();

      if(nextToSplit == static_cast<size_type>(1 << roundNumber)) { 
        roundNumber++; 
        nextToSplit = 0; 
      }
    }
};

template <typename Key, size_t N>
struct ADS_set<Key, N>::Bucket {
    key_type* keys;
    size_type capacity;
    size_type sz;
    size_type fakeCapacity;

    Bucket(): capacity{N}, sz{0}, fakeCapacity{N} {
        keys = new key_type[capacity];
    }

    ~Bucket() {
        delete[] keys;
    }

    bool count(const key_type& key) {
        for (size_t i {0}; i < sz; i++) {
            if (key_equal{}(keys[i], key)) return true;
        }
        return false;
    }

    void shrinkIfTooLarge() {
      if (sz <= capacity / 2) reserve(sz / N + N);
    }

    size_type erase(const key_type& key) {
      for (size_t i {0}; i < sz; i++) {
        if (key_equal{}(keys[i], key)) {
          keys[i] = keys[sz-1];
          sz--;
          //shrinkIfTooLarge();
          return 1;
        }
      }
      return 0;
    }

    int find(const key_type& key) {
      for (size_t i {0}; i < sz; i++) {
        if (key_equal{}(keys[i], key)) return i;
      }

      return -1;
    }

    bool append(const key_type& key) {
      if (sz == fakeCapacity) {
        fakeCapacity += N;
        if (sz == capacity) {
          reserve(capacity * 2);
        }
        keys[sz++] = key;
        return true;
      }
      
      keys[sz++] = key;
      return false;
    }

    //CHANGE THIS TO (OVERFLOWS+1)*2 LATER AND CHECK PERFORMANCE
    void reserve(size_type newCap) {
        key_type* newKeys = new key_type[newCap];
        for (size_t i {0}; i < sz; i++) {
            newKeys[i] = keys[i];
        }
        delete[] keys;
        keys = newKeys;
        capacity = newCap;
    }
};


template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;
private:
  Bucket** ht;
  size_type htSZ, x, y;
  pointer ptr;

  void advance() {
    if (ht[x]->sz <= y) {
      y = 0;
      x++;

      while (x < htSZ && ht[x]->sz == 0) {
        x++;
      }

      if (x == htSZ) {
        ptr = nullptr;
        return;
      }
    }

    ptr = ht[x]->keys+y;
  }

public:
  explicit Iterator(Bucket** ht, size_type htSZ): ht{ht}, htSZ{htSZ}, x{0}, y{0} {
    if (ht[x]->sz == 0) advance();
    else ptr = ht[x]->keys;
  };

  Iterator(Bucket** ht, size_t htSZ, size_t x, size_t y): ht{ht}, htSZ{htSZ}, x{x}, y{y} {
    ptr = ht[x]->keys+y;
  }

  Iterator(): ht{nullptr}, htSZ{0}, x{0}, y{0}, ptr{nullptr} {}

  reference operator*() const {
    return *ptr;
  };

  pointer operator->() const {
    return ptr;
  };
  Iterator &operator++() {
    y++;
    advance();
    return *this;
  };
  Iterator operator++(int) {
    Iterator temp {*this};
    ++*this;
    return temp;
  };
  friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
    return lhs.ptr == rhs.ptr;
  };
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
    return !(lhs == rhs);
  };
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }


#endif // ADS_SET_H

