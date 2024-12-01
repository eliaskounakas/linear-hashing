#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
//ONLY USED FOR DUMP
#include <bitset>

template <typename Key, size_t N =7>
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
    using key_compare = std::less<key_type>;                         // B+-Tree
    using key_equal = std::equal_to<key_type>;                       // Hashing
    using hasher = std::hash<key_type>;                              // Hashing
private:
    struct Bucket;
    size_type numOfElements;
    size_type roundNumber;
    size_type nextToSplit;
    size_type tableSize;
    size_type tableMaxSize;
    Bucket** buckets;
public:
    ADS_set(): numOfElements{0}, roundNumber{1}, nextToSplit{0}, tableSize{2}, tableMaxSize{4}, buckets{new Bucket*[tableMaxSize]} {
        buckets[0] = new Bucket();
        buckets[1] = new Bucket();
    }

    ADS_set(std::initializer_list<key_type> ilist): ADS_set{std::begin(ilist),std::end(ilist)} {}

    template<typename InputIt> 
    ADS_set(InputIt first, InputIt last): ADS_set{} {
        insert(first, last);
    }

    ADS_set(const ADS_set &other): ADS_set(other.begin(), other.end()) {}

    ~ADS_set() {
        for (size_type i{0}; i < tableSize; i++) deleteLinkedBuckets(buckets[i]);
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
        return numOfElements;
    }

    bool empty() const {
        return numOfElements == 0;
    }

    void insert(std::initializer_list<key_type> ilist) {
        insert(std::begin(ilist),std::end(ilist));
    }

    std::pair<iterator,bool> insert(const key_type &key) {
        size_t x = static_cast<size_t>(getIndex(key));
        size_t y {0};
    
        bool end {false};
        Bucket* b = buckets[x];

        while (!end) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                if (key_equal{}(b->entries[i], key)) {
                    Iterator it (buckets, tableSize, x, y, i);
                    return std::make_pair(it, false);
                }
            }
            if (b->nextBucket == nullptr) {
                end = true;
            } else {
                y++;
                b = b->nextBucket;
            }
        }
        
        numOfElements++;
        if(b->append(key)) {
            split();
            return std::make_pair(find(key), true);
        }

        Iterator it (buckets, tableSize, x, y, b->bucketSize-1);
        return std::make_pair(it, true);
    }

    template<typename InputIt> 
    void insert(InputIt first, InputIt last) {
        for (InputIt it {first}; it != last; it++) {
            add(*it);
        }
    }

    void clear() {
        ADS_set temp;
        this->swap(temp);
    }

    size_type erase(const key_type &key) {
        unsigned index = getIndex(key);

        Bucket* prev {nullptr};
        for (Bucket* b{buckets[index]}; b != nullptr; b = b->nextBucket) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                if (key_equal{}(b->entries[i], key)) {
                    b->entries[i] = b->entries[b->bucketSize-1];
                    b->bucketSize--;
                    numOfElements--;

                    if (b->bucketSize > 0 || !(prev || b->nextBucket)) return 1;

                    if (!prev) {
                        buckets[index] = b->nextBucket;
                    } else if (!b->nextBucket) {
                        prev->nextBucket = nullptr;
                    } else {
                        prev->nextBucket = b->nextBucket;
                    }

                    delete b;
                    return 1;
                }
            }
            prev = b;
        }

        return 0;
    }

    size_type count(const key_type &key) const {
        unsigned index = getIndex(key);
        for (Bucket* b{buckets[index]}; b != nullptr; b = b->nextBucket) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                if (key_equal{}(b->entries[i], key)) return 1;
            }
        }
        return 0;
    }

    iterator find(const key_type &key) const {
        size_t x = static_cast<size_t>(getIndex(key));
        size_t y {0};

        for (Bucket* b{buckets[x]}; b != nullptr; b = b->nextBucket) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                if (key_equal{}(b->entries[i], key)) {
                    return Iterator(buckets, tableSize, x, y, i);
                }
            }
            y++;
        }

        return end();
    }

    void swap(ADS_set &other) {
        std::swap(buckets, other.buckets);
        std::swap(nextToSplit, other.nextToSplit);
        std::swap(roundNumber, other.roundNumber);
        std::swap(tableSize, other.tableSize);
        std::swap(tableMaxSize, other.tableMaxSize);
        std::swap(numOfElements, other.numOfElements);
    }

    const_iterator begin() const {
        return const_iterator{buckets, tableSize};
    };
    const_iterator end() const {
        return const_iterator{};
    };

    void dump(std::ostream &o = std::cerr) const {
        o << "[size = " << numOfElements << "\n";
            for (size_type i{0}; i < tableSize; i++) {
            std::string index{std::bitset<64>( i ).to_string()};
            if (i < nextToSplit || i > ((1u << roundNumber) - 1)) 
                index = index.substr(index.size() - (roundNumber + 1));
            else index = ' ' + index.substr(index.size() - roundNumber);
            o << index << " : ";

            Bucket* b{buckets[i]};
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
        size_type hashedKey = hasher{}(key);
        unsigned index = hashedKey & ((1u << roundNumber) - 1);
        if (index < nextToSplit) index = hashedKey & ((1u << (roundNumber+1)) - 1);;
        return index;
    }

    void add(const key_type& key) {
        unsigned index = getIndex(key);
        for (Bucket* b{buckets[index]}; b != nullptr; b = b->nextBucket) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                if (key_equal{}(b->entries[i], key)) return;
            }
        }
        
        numOfElements++;
        if (buckets[index]->append(key)) split();
    }

    void deleteLinkedBuckets(Bucket* currentBucket) {
        if (currentBucket == nullptr) {
            return;
        }
        deleteLinkedBuckets(currentBucket->nextBucket);
        delete currentBucket;
    }

    void split() {
        nextToSplit++;
        if (tableSize == tableMaxSize) {
            tableMaxSize *= 2;
            Bucket** newBuckets = new Bucket*[tableMaxSize];

            for (size_type i{0}; i < tableSize; i++) {
                newBuckets[i] = buckets[i];
            }

            delete[] buckets;
            buckets = newBuckets;
        } 

        buckets[tableSize++] = new Bucket;
        Bucket* newBucket = new Bucket;
        for (Bucket* b{buckets[nextToSplit-1]}; b != nullptr; b = b->nextBucket) {
            for (size_type i = 0; i < b->bucketSize; ++i) {
                unsigned index = getIndex(b->entries[i]);
                if (index == nextToSplit-1) newBucket->append(std::move(b->entries[i]));
                else buckets[index]->append(std::move(b->entries[i]));
            }
        }

        deleteLinkedBuckets(buckets[nextToSplit-1]);
        buckets[nextToSplit-1] = newBucket;


        if(nextToSplit == static_cast<size_type>(1 << roundNumber)) { 
            roundNumber++; 
            nextToSplit = 0; 
        }
    }
};

template <typename Key, size_t N>
struct ADS_set<Key, N>::Bucket {
  size_type bucketSize{0};
  key_type entries[N]{};
  Bucket* nextBucket{nullptr};

  bool append(const key_type& key) {
    Bucket* curr = this;

    while (curr->bucketSize == N) {
      if (curr->nextBucket == nullptr) {
        curr->nextBucket = new Bucket;
        curr = curr->nextBucket;
        curr->entries[0] = key;
        curr->bucketSize++;
        return true;
      }

      curr = curr->nextBucket;
    }

    curr->entries[curr->bucketSize++] = key;
    return false;
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
    Bucket** buckets;
    Bucket* currBucket; 
    size_type tableSize;
    size_type bucketIndex; 
    size_type entryIndex; 
    pointer currPtr; 

    void advanceToNextValidBucket() {
        while (bucketIndex < tableSize) {
            Bucket* bucket = buckets[bucketIndex];
            
            while (bucket != nullptr) {
                if (bucket->bucketSize > 0) {
                    currBucket = bucket;
                    currPtr = bucket->entries;
                    return;
                }
                bucket = bucket->nextBucket;
            }
            
            ++bucketIndex;
        }
        
        currBucket = nullptr;
        currPtr = nullptr;
    }

public:
    explicit Iterator(Bucket** buckets, size_t tableSize) : buckets{buckets}, tableSize{tableSize}, bucketIndex{0}, entryIndex{0} {
        advanceToNextValidBucket();
    }

    Iterator(Bucket** buckets, size_t tableSize, size_t bucketIndex, size_t chainIndex, size_t entryIndex) : 
        buckets{buckets}, 
        tableSize{tableSize}, 
        bucketIndex{bucketIndex}, 
        entryIndex{entryIndex} {

        currBucket = buckets[bucketIndex];
        for (size_t i = 0; i < chainIndex; ++i) {
            currBucket = currBucket->nextBucket;
        }
        currPtr = &currBucket->entries[entryIndex];
    }

    Iterator(): buckets{nullptr}, 
        currBucket{nullptr}, 
        tableSize{0}, 
        bucketIndex{0}, 
        entryIndex{0}, 
        currPtr{nullptr} {}

    reference operator*() const { 
        return *currPtr; 
    }

    pointer operator->() const { 
        return currPtr; 
    }

    Iterator& operator++() {
        ++entryIndex;

        if (entryIndex >= currBucket->bucketSize) {
            if (currBucket->nextBucket != nullptr) {
                currBucket = currBucket->nextBucket;
                entryIndex = 0;
                currPtr = currBucket->entries;
            } else {
                ++bucketIndex;
                entryIndex = 0;
                advanceToNextValidBucket();
            }
        } else {
            currPtr = &currBucket->entries[entryIndex];
        }

        return *this;
    }


    Iterator operator++(int) {
        Iterator temp(*this);
        ++(*this);
        return temp;
    }

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
        return lhs.currPtr == rhs.currPtr;
    }

    friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
        return !(lhs == rhs);
    }
};


template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H