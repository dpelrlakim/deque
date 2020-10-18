#ifndef DEQUE_H
#define DEQUE_H

#define DEBUGGING 0

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <stdexcept>

/*                              ----Note about Readability----
 * The deque that I implemented can be visualized as an array of pointers that each point
 *   towards `chunks` of ten objects of type T. As such, you may notice that the iteration
 *   process resembles counting in base 10. 
 * In particular, you may find it helpful to read the code if you think of pointers (i.e. chunks) as 
 *   "tens digits" and each of the individual T-objects as "ones digits".
 */

namespace cs246e {
const size_t middleOfChunk = 4; // i.e. make room in the front and back
const size_t chunkSize = 10;

template <typename T> class deque {

  size_t firstChunk, startCursor, lastChunk, endCursor, cap;
  T **theDeque;

 public:
  deque();
  deque(std::initializer_list <T> init);
  deque(const deque &other);
  deque(T &&other);
  deque &operator=(deque other);
  T &at(size_t i);
  T &operator[](size_t i);
  const T &at(size_t i) const;
  const T &operator[](size_t i) const;
  size_t size() const;
  bool empty() const;

 private:
  void increment(size_t &cursor);
  void decrement(size_t &cursor);
  void makeChunk(const size_t chunk, const size_t lBound, const size_t uBound, const deque &other);
  void copyDeque(const deque &other);
  T &getElem(size_t i);
  const T &getElem(size_t i) const;
  void clearChunk(const size_t chunk, const size_t lBound, const size_t uBound);
  void clearDeque();
  void grow(std::string &&dir);
  
 public:
  ~deque();
  void push_front(const T &x);
  void pop_front();
  void push_back(const T &x);
  void pop_back();
  
  class iterator {
    T **p;
    size_t cursor;
    iterator(T **p, size_t cursor);
   public:
    bool operator!=(const iterator &other) const;
    iterator &operator++();
    T &operator*() const;
    friend class deque;
  };

  class const_iterator {
    T **p;
    size_t cursor;
    const_iterator(T **p, size_t cursor);
   public:
    bool operator!=(const const_iterator &other) const;
    const_iterator &operator++();
    const T &operator*() const;
    friend class deque;
  };

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  #if DEBUGGING
  void printStuff();
  #endif
};
// --------Debugging---------
#if DEBUGGING
template <typename T> void deque<T>::printStuff() {
  std::cout << "firstChunk: " << firstChunk << std::endl
            << "startCursor: " << startCursor << std::endl
            << "lastChunk: " << lastChunk << std::endl
            << "endCursor: " << endCursor << std::endl
            << "cap: " << cap << std::endl;
}
#endif

// ------Simple helpers------
template <typename T> void deque<T>::increment(size_t &cursor) {
  if (cursor == 9) cursor = 0;
  else ++cursor;
}
template <typename T> void deque<T>::decrement(size_t &cursor) {
  if (cursor == 0) cursor = 9;
  else --cursor;
}


// ------Construction------
template <typename T>
void deque<T>::makeChunk(const size_t chunk, const size_t lBound, const size_t uBound, const deque &other) {
  for (size_t i = lBound; i < uBound; i++) 
    new (theDeque[chunk] + i) T((other.theDeque)[chunk][i]);
}

template <typename T> void deque<T>::copyDeque(const deque &other) {
  size_t i = 0;
  while (i < cap) {
    if (i < firstChunk || i > lastChunk) {
      theDeque[i] = nullptr;
      ++i;
      continue;
    }
    theDeque[i] = static_cast <T*> (operator new (sizeof(T) * chunkSize));
    if (i == firstChunk && i == lastChunk) makeChunk(i, startCursor, endCursor, other);
    else if (i == firstChunk) makeChunk(i, startCursor, 10, other);
    else if (i == lastChunk) makeChunk(i, 0, endCursor, other);
    else makeChunk(i, 0, 10, other);
    ++i;
  }
}

template <typename T> deque<T>::deque(): firstChunk{0}, startCursor{middleOfChunk}, lastChunk{0}, 
  endCursor{middleOfChunk}, cap{1}, theDeque{new T*[cap]} {

  theDeque[0] = static_cast <T*> (operator new(sizeof(T) * chunkSize));
}

template <typename T> deque<T>::deque(std::initializer_list <T> init): 
  firstChunk{0}, startCursor{middleOfChunk}, lastChunk{(init.size() + middleOfChunk) / 10}, 
  endCursor{(init.size() + middleOfChunk) % 10}, cap{!endCursor? lastChunk: lastChunk + 1}, 
  theDeque{new T*[cap]} {

  size_t i = middleOfChunk;
  theDeque[0] = static_cast <T*> (operator new (sizeof(T) * chunkSize));
  for (auto t:init) {
    if (!(i % 10)) theDeque[i / 10] = static_cast <T*> (operator new (sizeof(T) * chunkSize));
    new (theDeque[i / 10] + i % 10) T(t);
    ++i;
  }
}

template <typename T> deque<T>::deque(const deque &other): firstChunk{other.firstChunk}, 
startCursor{other.startCursor}, lastChunk{other.lastChunk}, endCursor{other.endCursor}, cap{other.cap}, 
theDeque{other.theDeque? new T*[cap]: nullptr} {

  copyDeque(other);
}

template <typename T> deque<T>::deque(T &&other): firstChunk{other.firstChunk}, startCursor{other.startCursor},
lastChunk{other.lastChunk}, endCursor{other.endCursor}, cap{other.cap}, theDeque{other.theDeque} {
  
  other.theDeque = nullptr;
}

template <typename T> deque<T> &deque<T>::operator=(deque other) {
  std::swap(firstChunk, other.firstChunk);
  std::swap(startCursor, other.startCursor);
  std::swap(lastChunk, other.lastChunk);
  std::swap(endCursor, other.endCursor);
  std::swap(cap, other.cap);
  std::swap(theDeque, other.theDeque);
  return *this;
}


// ------Destruction------
template <typename T> void deque<T>::clearChunk(const size_t chunk, const size_t lBound, const size_t uBound) {
  for (size_t i = lBound; i < uBound; ++i)
    theDeque[chunk][i].~T();
  operator delete(theDeque[chunk]);
} 
template <typename T> void deque<T>::clearDeque() {
  size_t i = 0;
  while (i < cap) {
    if (i == firstChunk && i == lastChunk) clearChunk(i, startCursor, endCursor);
    else if (i == firstChunk) clearChunk(i, startCursor, 10);
    else if (i == lastChunk) clearChunk(i, 0, endCursor);
    else if (i > firstChunk && i < lastChunk) clearChunk(i, 0, 10);
    ++i;
  }
}
template <typename T> deque<T>::~deque() {
  deque::clearDeque();
  delete [] theDeque;
}


// ------Accessor Methods------
template <typename T> T &deque<T>::getElem(size_t i) {
  if (startCursor + i < 10) return theDeque[firstChunk][startCursor + i];
  size_t counter = 1;
  i -= (10 - startCursor); // shift i by size of first chunk
  while (i >= 10) {
    i -= 10;
    counter++;
  }
  return theDeque[firstChunk + counter][i];
} 
template <typename T> const T &deque<T>::getElem(size_t i) const {
  if (startCursor + i < 10) return theDeque[firstChunk][startCursor + i];
  size_t counter = 1;
  i -= (10 - startCursor);
  while (i >= 10) {
    i -= 10;
    counter++;
  }
  return theDeque[firstChunk + counter][i];
}
template <typename T> T &deque<T>::at(size_t i) {
  if (i >= size()) throw std::out_of_range("Index is out of range\n");
  return deque::getElem(i);
}
template <typename T> const T &deque<T>::at(size_t i) const {
  if (i >= size()) throw std::out_of_range("Index is out of range\n");
  return deque::getElem(i);
}
template <typename T> T &deque<T>::operator[](size_t i) { return deque::getElem(i); }
template <typename T> const T &deque<T>::operator[](size_t i) const { return deque::getElem(i); }
template <typename T> bool deque<T>::empty() const { return !size(); }
template <typename T> size_t deque<T>::size() const {
  size_t fin = lastChunk * 10 + endCursor;
  size_t beg = firstChunk * 10 + startCursor;
  return fin - beg;
}


// ------Push/Pop------
template <typename T> void deque<T>::grow(std::string &&dir) {
  if ((dir == "back") && (lastChunk != cap)) return;
  else if ((dir == "front") && (firstChunk || startCursor)) return;
  else {
    bool capped = (!endCursor && !empty()); // for off-by-1 errors
    size_t currentSize = lastChunk - firstChunk + 1;
    if (capped) --currentSize;
    size_t newCap = 3 * currentSize;
    T **newDQ = new T*[newCap];
    for (size_t i = 0; i < currentSize; i++) {
      newDQ[i] = nullptr;
      newDQ[currentSize + i] = theDeque[firstChunk + i];
      theDeque[firstChunk + i] = nullptr;
      newDQ[2 * currentSize + i] = nullptr;
    }
    delete [] theDeque;
    theDeque = newDQ;
    cap = newCap;
    firstChunk = currentSize;
    lastChunk = 2 * currentSize - 1;
    if (capped) ++lastChunk;
  }
}

template <typename T> void deque<T>::push_front(const T &x) {
  deque::grow("front");
  if (!startCursor) {
    --firstChunk;
    theDeque[firstChunk] = static_cast <T*> (operator new (sizeof(T) * chunkSize));
  }
  decrement(startCursor);
  new (theDeque[firstChunk] + startCursor) T(x);
}
template <typename T> void deque<T>::pop_front() {
  if (empty()) return;
  theDeque[firstChunk][startCursor].~T();
  increment(startCursor);
  if (!startCursor) {
    operator delete (theDeque[firstChunk]);
    ++firstChunk;
  }
}

template <typename T> void deque<T>::push_back(const T &x) {
  deque::grow("back");
  if (!endCursor)
    theDeque[lastChunk] = static_cast <T*> (operator new (sizeof(T) * chunkSize));
  new (theDeque[lastChunk] + endCursor) T(x);
  increment(endCursor);
  if (!endCursor) ++lastChunk;
}
template <typename T> void deque<T>::pop_back() {
  if (empty()) return;
  if (!endCursor) --lastChunk;
  decrement(endCursor);
  theDeque[lastChunk][endCursor].~T();
  if (!endCursor) operator delete (theDeque[lastChunk]);
}


// ------Iteration------
// non-const
template <typename T> deque<T>::iterator::iterator(T **p, size_t cursor): p{p}, cursor{cursor} {}
template <typename T> T &deque<T>::iterator::operator*() const { return (*p)[cursor]; }
template <typename T> bool deque<T>::iterator::operator!=(const iterator &other) const {
  return (p != other.p) || (cursor != other.cursor);
}
template <typename T> typename deque<T>::iterator &deque<T>::iterator::operator++() {
  if (cursor != 9) ++cursor;
  else {
    ++p;
    cursor = 0;
  }
  return *this;
}
// const
template <typename T> deque<T>::const_iterator::const_iterator(T **p, size_t cursor): p{p}, cursor{cursor} {}
template <typename T> const T &deque<T>::const_iterator::operator*() const { return (*p)[cursor]; }
template <typename T> bool deque<T>::const_iterator::operator!=(const const_iterator &other) const {
  return (p != other.p) || (cursor != other.cursor);
}
template <typename T> typename deque<T>::const_iterator &deque<T>::const_iterator::operator++() {
  if (cursor != 9) ++cursor;
  else {
    ++p;
    cursor = 0;
  }
  return *this;
}

template <typename T> typename deque<T>::iterator deque<T>::begin() {
  return iterator{ theDeque + firstChunk, startCursor };
}
template <typename T> typename deque<T>::iterator deque<T>::end() {
  return iterator{ theDeque + lastChunk, endCursor };
}
template <typename T> typename deque<T>::const_iterator deque<T>::begin() const {
  return iterator{ theDeque + firstChunk, startCursor };
}
template <typename T> typename deque<T>::const_iterator deque<T>::end() const {
  return iterator{ theDeque + lastChunk, endCursor };
}
}
#endif
