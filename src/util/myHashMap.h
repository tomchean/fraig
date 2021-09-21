/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
template <class Data>
class HashKey
{
  public:
    HashKey( Data i1,Data i2) {
      input[0] = i1;
      input[1] = i2;
    }
    Data operator() () const { return input[0] + input[1]; }
    bool operator == (const HashKey& k) const { 
      if(input[0] == k.input[0] && input[1] == k.input[1])  return true; 
      else return false;
    }

  private:
    Data input[2];
};


template <class Data>
class HashKey1
{
  public:
    HashKey1( Data i1) {
      input = i1;
    }
    Data operator() () const { return input; }
    bool operator == (const HashKey1& k) const { 
      if(input == k.input)  return true; 
      else return false;
    }

  private:
    Data input;
};


template <class Data>
class HashKey_fec 
{
  public:
    HashKey_fec( Data i1,Data i2) {
      if(i1 > SIZE_MAX/2){
        input[0] = i1;
      }
      else{
        input[0] = ~i1;
      }
      input[1] = i2;
    }
    Data operator() () const { return input[0] + input[1]; }
    bool operator == (const HashKey_fec& k) const {
      if(input[1] == k.input[1] && input[0] == k.input[0]) { return true; }
      else return false;
    }

  private:
    Data input[2];
};


template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

    public:
      iterator(vector<HashNode>*  _buckets ,size_t  _numBuckets) : _buckets(_buckets), _numBuckets(_numBuckets),_index1(0),_index2(0){
        
        while (_index1 < (_numBuckets)){
          if(_buckets[_index1].size() != 0){ 
            return;
          }
          _index1++;
        }
        _index1 = _numBuckets;
      }
      ~iterator() {}
      iterator& gote_end() {_index1 = _numBuckets;_index2=0; return(*this); }
      const HashNode& operator * () const { return _buckets[_index1][_index2]; }
      HashNode& operator * () { return _buckets[_index1][_index2]; }
      iterator& operator ++ () { 
        if(_index2 < _buckets[_index1].size()-1) {
          _index2++; 
          return(*this);
          }
        while (_index1 < (_numBuckets-1)){
          _index1++;
          if(_buckets[_index1].size() != 0){ 
            _index2 =0 ;
            return(*this);
          }
        } 
        _index1 = _numBuckets;
        _index2=0;
        return (*this); 
      }
      iterator& operator -- () { 
        if(_index2 > 0) {
          _index2--; 
          return(*this);
          }
        size_t tmp = _index1;  
        while(_index1 > 0){
          _index1--;
          if(_buckets[_index1].size() != 0) {
            _index2 = _buckets[_index1].size()-1;
            return (*this);
          }
        } 
        _index1 = tmp;
        return (*this); 
      }
      iterator operator ++ (int) { iterator ret = (*this); ++(*this); return ret; }
      iterator operator -- (int) { iterator ret = (*this); --(*this); return ret; }
      bool operator != (const iterator& i) const { if(i._buckets == _buckets && i._numBuckets == _numBuckets && i._index1 == _index1 && i._index2 == _index2) return false; else return true; }
      bool operator == (const iterator& i) const { if((*this) != i) return false; else return true; }
      iterator& operator = (const iterator& i){ _buckets = i._buckets; _numBuckets =i._numBuckets; _index1 = i._index1; _index2 = i._index2; return(*this); }

    private:
      vector<HashNode>* _buckets;
      size_t _numBuckets,_index1,_index2;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { return iterator(_buckets,_numBuckets); }
   // Pass the end
   iterator end() const { return iterator(_buckets,_numBuckets).gote_end(); }
   // return true if no valid data
   bool empty() const {return (begin() == end()); }
   // number of valid data
   size_t size() const { 
    size_t s = 0;
    iterator i = iterator(_buckets,_numBuckets);
    while(i != end()){
      i++;
      s++;
    }
    return s; 
  }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const {  
    size_t num =  bucketNum(k);
    for(size_t i=0; i <_buckets[num].size();i++){
      if(_buckets[num][i] ==  k){
        return true;
      }      
    }
    return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const { 
    size_t num =  bucketNum(k);
    for(size_t i=0; i <_buckets[num].size();i++){
      if( _buckets[num][i].first ==  k){
        d = _buckets[num][i].second;
        return true;
      }      
    }
    return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) { 
    size_t num =  bucketNum(k);
    for(size_t i=0; i <_buckets[num].size();i++){
      if(_buckets[num][i].first ==  k){
        _buckets[num][i].second = d;
        return true;
      }      
    }
     _buckets[num].push_back(HashNode(k,d));
    return false;
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) { 
    size_t num =  bucketNum(k);
    for(size_t i=0; i <_buckets[num].size();i++){
      if(_buckets[num][i].first ==  k){
        return false;
      }      
    }
     _buckets[num].push_back(HashNode(k,d));
    return true;
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) { 
    size_t num =  bucketNum(k);
    for(size_t i=0; i <_buckets[num].size();i++){
      if(_buckets[num][i].first ==  k){
        _buckets[num].erase(_buckets[num].begin()+i);
        return true;          
      }      
    }
    return false;
   }


private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
