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
  struct HashTable;
  HashTable hashTable;
  size_type numOfElements;
public:
  ADS_set(): hashTable{new Bucket*[1]}, numOfElements{0} {
    hashTable.buckets[0] = new Bucket;
  }                         // PH1

  ADS_set(std::initializer_list<key_type> ilist): ADS_set{std::begin(ilist),std::end(ilist)} {}                   // PH1

  template<typename InputIt> 
  ADS_set(InputIt first, InputIt last): ADS_set{} {
    insert(first, last);
  }     // PH1

  ADS_set(const ADS_set &other): ADS_set(other.begin(), other.end()) {
  }

  ~ADS_set() {}

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
    return numOfElements;
  }

  bool empty() const {
    return size() == 0;
  }

  void insert(std::initializer_list<key_type> ilist) {
    insert(std::begin(ilist),std::end(ilist));
  }                  // PH1


  std::pair<iterator,bool> insert(const key_type &key) {
    auto existingIt = find(key);

    if (existingIt == end()) {
      hashTable.insert(key);
      return std::make_pair(find(key), true);
    }
    
    return std::make_pair(existingIt, false);
  }

  template<typename InputIt> void insert(InputIt first, InputIt last) {
    for (InputIt it {first}; it != last; it++) {
      const key_type& key = *it;
      if (count(key) != 0) continue;
      hashTable.insert(key);
      numOfElements++;
    }
  }

  void clear() {
    hashTable.~HashTable();
    ADS_set temp;
    this->swap(temp);
  }

  size_type erase(const key_type &key) {
    unsigned index = hashTable.getIndex(key);
    bool removedKey = hashTable.buckets[index]->erase(key);
    if (removedKey) {
      numOfElements--;
      hashTable.buckets[index]->cleanup();
    };
    
    return numOfElements;
  };

  size_type count(const key_type &key) const {
    unsigned index = hashTable.getIndex(key);
    for (Bucket* b{hashTable.buckets[index]}; b != nullptr; b = b->nextBucket) {
      for (size_type i = 0; i < b->bucketSize; ++i) {
        if (key_equal{}(b->entries[i], key)) return 1;
      }
    }
    return 0;
  }                       // PH1

  iterator find(const key_type &key) const {
    size_t x = static_cast<size_t>(hashTable.getIndex(key));
    size_t y {0};

    for (Bucket* b{hashTable.buckets[x]}; b != nullptr; b = b->nextBucket) {
      for (size_type i = 0; i < b->bucketSize; ++i) {
        if (key_equal{}(b->entries[i], key)) {
          Iterator newIt(hashTable.buckets, hashTable.tableSize, x, y, i);
          return newIt;
        }
      }
      y++;
    }

    return this->end();
  }

  void swap(ADS_set &other) {
    std::swap(hashTable.buckets, other.hashTable.buckets);
    std::swap(hashTable.nextToSplit, other.hashTable.nextToSplit);
    std::swap(hashTable.roundNumber, other.hashTable.roundNumber);
    std::swap(hashTable.tableSize, other.hashTable.tableSize);
    std::swap(numOfElements, other.numOfElements);
  }

  const_iterator begin() const {
    return const_iterator{hashTable.buckets, hashTable.tableSize};
  };
  const_iterator end() const {
    return const_iterator{};
  };

  void dump(std::ostream &o = std::cerr) const {
    for (size_type i{0}; i < hashTable.tableSize; i++) {
      std::string index{std::bitset<64>( i ).to_string()};
      if (i < hashTable.nextToSplit || i > hashTable.tableSize - hashTable.nextToSplit - 1) 
        index = index.substr(index.size() - (hashTable.roundNumber + 1));
      else index = ' ' + index.substr(index.size() - hashTable.roundNumber);
      o << index << " : ";

      Bucket* b{hashTable.buckets[i]};
      while (b != nullptr) {
        for (size_type j{0}; j < b->bucketSize; j++) {
          o << b->entries[j] << ' ';
        } 
        b = b->nextBucket;
        if (b != nullptr) o << "-> ";
      }

      o << "\n";
    }
  }

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if (lhs.numOfElements != rhs.numOfElements) return false;
    bool elementNotFound {false};
    for (const auto& key : lhs) {
      if (!rhs.count(key)) {
        elementNotFound = true;
        return false;
      }
      return true;
    };
    return elementNotFound;
  }

  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
    return !(lhs == rhs);
  }
};



template <typename Key, size_t N>
struct ADS_set<Key, N>::Bucket {
  size_type bucketSize{0};
  size_type bucketMaxSize{N};
  key_type entries[N]{};
  Bucket* nextBucket{nullptr};

  bool append(key_type key) {
    if (bucketSize == bucketMaxSize) {
      if (nextBucket != nullptr) return (*nextBucket).append(key);

      nextBucket = new Bucket;
      (*nextBucket).append(key);
      return true;
    }

    entries[bucketSize] = key;
    bucketSize++;
    return false;
  }

  bool erase(key_type key) {
    for (size_type i = 0; i < bucketSize; ++i) {
      if (key_equal{}(entries[i], key)) {
        entries[i] = entries[bucketSize-1];

        bucketSize--;
        return true;
      };
    }

    if (nextBucket == nullptr) {
      return false;
    } else {
      return nextBucket->erase(key);
    }
  }

  void cleanup() {
    if (nextBucket != nullptr) {
      if (nextBucket->bucketSize == 0) {
        delete nextBucket;
        nextBucket = nullptr;
      }
      else nextBucket->cleanup();
    } 
  }
};


template <typename Key, size_t N>
struct ADS_set<Key, N>::HashTable {
  Bucket** buckets{nullptr};
  size_type tableSize{1};
  size_type roundNumber{0};
  size_type nextToSplit{0};

  void deleteLinkedBuckets(Bucket* currentBucket) {
    if (currentBucket == nullptr) {
        return;
    }
    deleteLinkedBuckets(currentBucket->nextBucket);
    delete currentBucket;
  }

  unsigned getLastNBits(key_type key, size_type n) const {
    unsigned hashedValue = static_cast<unsigned>(hasher{}(key));
    unsigned mask = (1 << n) - 1;
    unsigned last_n_bits = hashedValue & mask;
    return last_n_bits;
  }

  unsigned getIndex(const key_type& key) const {
    unsigned index = getLastNBits(key, roundNumber);
    if (index < nextToSplit) index = getLastNBits(key, roundNumber+1);
    return index;
  }

  void insert(key_type key) {
      unsigned index = getIndex(key);
      bool overflow = buckets[index]->append(key);
      if (overflow) split();
  }

  void split() {
    Bucket** newBuckets = new Bucket*[tableSize + 1];
    for (size_type i{0}; i < tableSize; i++) {
      newBuckets[i] = buckets[i];
    }
    
    newBuckets[tableSize] = new Bucket;
    tableSize++;
    delete[] buckets;
    buckets = newBuckets;

    //Kopiere alle Werte aus dem Bucket, der gesplittet werden soll, um sie neu zu hashen.
    size_type splitEntriesCount{0};
    for (Bucket* b{buckets[nextToSplit]}; b != nullptr; b = b->nextBucket) {
      splitEntriesCount += b->bucketSize;
    }

    key_type* splitEntries = new key_type[splitEntriesCount];
    size_type count{0};

    for (Bucket* b{buckets[nextToSplit]}; b != nullptr; b = b->nextBucket) {
      for (size_type i = 0; i < b->bucketSize; ++i) {
        splitEntries[count] = b->entries[i];
        count++;
      }
    }

    deleteLinkedBuckets(buckets[nextToSplit]);
    buckets[nextToSplit] = new Bucket;

    nextToSplit++;
    if(nextToSplit == static_cast<size_type>(1 << roundNumber)) { 
      roundNumber++; 
      nextToSplit = 0; 
    }

    for (size_type i = 0; i < splitEntriesCount; ++i) {
      unsigned index = getIndex(splitEntries[i]);
      buckets[index]->append(splitEntries[i]);
    }
  

    delete[] splitEntries;
  }

  ~HashTable() {
    for (size_type i{0}; i < tableSize; i++) deleteLinkedBuckets(buckets[i]);
    delete[] buckets;
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
  Bucket* currBucket;
  size_type x, y, z, tableSZ;
  pointer ptr;

  void skip() {
    if (++z >= currBucket->bucketSize) {
      z = 0;

      if (currBucket->nextBucket == nullptr) {
        y = 0;
        ++x;

        if (x >= tableSZ) {
          ptr = nullptr;
          return;
        }

      } else {
        ++y;
      }

      currBucket = ht[x];
      for (size_t i {0}; i < y; i++) {
        currBucket = currBucket->nextBucket;
      }

      if (currBucket->bucketSize == 0) {
        skip();
      } else {
        ptr = currBucket->entries;
      }
    }
  }

public:
  explicit Iterator(Bucket** ht, size_t tableSZ): ht{ht}, x{0}, y{0}, z{0}, tableSZ{tableSZ} {
    this->currBucket = ht[0];
    if (this->currBucket->bucketSize == 0) {
      skip();
    } else {
       ptr = currBucket->entries;
    }
  }

  Iterator(Bucket** ht, size_t tableSZ, size_t x, size_t y, size_t z): ht{ht}, x{x}, y{y}, z{z}, tableSZ{tableSZ} {
    this->currBucket = ht[x];
    for (size_t i {0}; i < y; i++) {
      this->currBucket = currBucket->nextBucket;
    }
    ptr = currBucket->entries+z;
  }

  Iterator(): ht{nullptr}, currBucket{nullptr}, x{0}, y{0}, z{0}, tableSZ{0}, ptr{nullptr} {}

  reference operator*() const {
    return *ptr;
  };

  pointer operator->() const {
    return ptr;
  };
  Iterator &operator++() {
    ++ptr;
    skip();
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

#endif


