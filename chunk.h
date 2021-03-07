#ifndef CHUNK_H_
#define CHUNK_H_

#include "base.h"

// ChunkTx is used to communicate an array of items. The writer will write items
// in. Then "start transmission", which lets the readers know that the data is
// available. Not threadsafe because there is a natural cycle due to the audio
// callback, at the beginning there will be no readers and we can "stop
// transmission" to prepare the graph.
template <typename T>
class ChunkTx {
 public:
  ChunkTx() : cap_(0), size_(0), buf_(nullptr), tx_(false) {}
  ChunkTx(int n) : cap_(n), size_(0), buf_(new T[n]), tx_(false) {}
  ChunkTx(const ChunkTx<T>& other)
      : cap_(other.cap_),
        size_(other.size_),
        buf_(new T[other.cap_]),
        tx_(other.tx_) {
    for (int i = 0; i < size_; i++) {
      buf_[i] = other.buf_[i];
    }
  }
  ~ChunkTx() { delete[] buf_; }

  ChunkTx<T>& operator=(const ChunkTx<T>& other) {
    delete[] buf_;
    cap_ = other.cap_;
    size_ = other.size_;
    buf_ = new T[cap_];
    for (int i = 0; i < size_; i++) {
      buf_[i] = other.buf_[i];
    }
    return *this;
  }

  // Writing is done directly via pointer, which is allows for optimizations
  // like calls to memcpy.
  T* write_ptr() { return buf_; }

  void set_size(int size) {
    CHECK(0 <= size && size <= cap_);
    size_ = size;
  }

  void set_tx(bool tx) { tx_ = tx; }

  int capacity() const { return cap_; }
  int size() const { return size_; }
  bool tx() const { return tx_; }

  const T* read_ptr() const {
    CHECK(tx_);
    return buf_;
  }

 private:
  int cap_, size_;
  T* buf_;
  bool tx_;
};

#endif  // CHUNK_H_
