#ifndef CHUNK_H_
#define CHUNK_H_

#include "base.h"

// ChunkTx is used to communicate an array of items. The writer will
// write items in. Then "start transmission", which lets the readers
// know that the data is ready. Then the readers can read the items,
// until the write "stops transmission" to update the data.
template <typename T>
class ChunkTx {
public:
	ChunkTx() : cap_(0), size_(0), buf_(nullptr), tx_(false) {}
	ChunkTx(int n) : cap_(n), size_(0), buf_(new T[n]), tx_(false) {}
	ChunkTx(const ChunkTx<T>& other) :
		cap_(other.cap_), size_(other.size_),
		buf_(new T[other.cap_]), tx_(other.tx_) {
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

	int capacity() const { return cap_; }
	T* write_ptr() { return buf_; }
	void set_size(int size) {
		CHECK(0 <= size && size <= cap_);
		size_ = size;
	}

	const T* read_ptr() const {
		CHECK(tx_);
		return buf_;
	}
	int size() const { return size_; }

	void Stop() { tx_ = false; }
	void Start() { tx_ = true; }
	bool available() const { return tx_; }

private:
	int cap_, size_;
	T* buf_;
	bool tx_;
};

#endif  // CHUNK_H_
