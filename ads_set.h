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
  //class /* iterator type (implementation-defined) */;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  //using const_iterator = /* iterator type */;
  //using iterator = const_iterator;
  //using key_compare = std::less<key_type>;                         // B+-Tree
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing
private:
  struct Bucket;
  struct HashTable;
  HashTable hashTable;
  size_type numOfElements;
public:
  ADS_set(): hashTable{new Bucket*[2]} ,numOfElements{0} {
    hashTable.buckets[0] = new Bucket;
    hashTable.buckets[1] = new Bucket;
  }                         // PH1
  ADS_set(std::initializer_list<key_type> ilist);                   // PH1
  template<typename InputIt> ADS_set(InputIt first, InputIt last);     // PH1
  //ADS_set(const ADS_set &other);

  //~ADS_set() {}

  //ADS_set &operator=(const ADS_set &other);
  //ADS_set &operator=(std::initializer_list<key_type> ilist);

  size_type size() const {
    return numOfElements;
  }

  bool empty() const {
    return size() == 0;
  }

  unsigned getLastNBits(key_type key, size_type n) {
    unsigned hashedValue = static_cast<unsigned>(std::hash<key_type>{}(key));
    unsigned mask = (1 << n) - 1;
    unsigned last_n_bits = hashedValue & mask;
    return last_n_bits;
  }

  void insert(std::initializer_list<key_type> ilist) {
    insert(std::begin(ilist),std::end(ilist));
  }                  // PH1

  //std::pair<iterator,bool> insert(const key_type &key);

  template<typename InputIt> void insert(InputIt first, InputIt last) {
    for (InputIt it {first}; it != last; it++) {
      unsigned index = getLastNBits(*it, hashTable.roundNumber);
      if (index < hashTable.nextToSplit) index = getLastNBits(*it, hashTable.roundNumber+1);
      (*hashTable.buckets[index]).append(*it);
      numOfElements++;
    }
  }

  // void clear();
  // size_type erase(const key_type &key);

  size_type count(const key_type &key) const;                          // PH1
  // iterator find(const key_type &key) const;

  // void swap(ADS_set &other);

  // const_iterator begin() const;
  // const_iterator end() const;

  void dump(std::ostream &o = std::cerr) const {
    for (size_type i{0}; i < hashTable.tableSize; i++) {
      std::string index{std::bitset<64>( i ).to_string()};
      if (i < hashTable.nextToSplit) index = index.substr(index.size() - (hashTable.roundNumber + 1));
      else index = index.substr(index.size() - hashTable.roundNumber);

      o << index << ' ';
      Bucket currentBucket = *hashTable.buckets[i];
      for (size_type j{0}; j < currentBucket.bucketSize; j++) {
        o << currentBucket.entries[j] << ' ';
      } 
      o << "\n";
    }

  }

  // friend bool operator==(const ADS_set &lhs, const ADS_set &rhs);
  // friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs);
};

template <typename Key, size_t N>
struct ADS_set<Key, N>::Bucket {
  size_type bucketSize{0};
  size_type bucketMaxSize{N};
  key_type entries[N];
  Bucket* nextBucket{nullptr};

  bool append(key_type key) {
    entries[bucketSize] = key;
    bucketSize++;
    return true;
  }
};


template <typename Key, size_t N>
struct ADS_set<Key, N>::HashTable {
  Bucket** buckets{nullptr};
  size_type tableSize{2};
  size_type roundNumber{1};
  size_type nextToSplit{0};

  ~HashTable() {
    for (size_type i{0}; i < tableSize; i++) delete buckets[i];
    delete[] buckets;
  }
};


#if 0
template <typename Key, size_t N>
class ADS_set<Key,N>::/* iterator type */ {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit /* iterator type */(/* implementation-dependent */);
  reference operator*() const;
  pointer operator->() const;
  /* iterator type */ &operator++();
  /* iterator type */ operator++(int);
  friend bool operator==(const /* iterator type */ &lhs, const /* iterator type */ &rhs);
  friend bool operator!=(const /* iterator type */ &lhs, const /* iterator type */ &rhs);
};


template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }
#endif

#endif // ADS_SET_H

