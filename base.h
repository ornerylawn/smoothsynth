#ifndef BASE_H_
#define BASE_H_

#include <cmath>
#include <iostream>

const float PI = 3.14159265358979;
const float TWO_PI = 2 * PI;

#define RETURN_IF_ERROR(EXP) \
	do {																							\
		Optional<Error> maybe_err = EXP;								\
		if (!maybe_err.is_nil()) { return maybe_err; }	\
	} while(0)

#define CHECK(EXP)																										\
	do {																																\
		if (EXP) {																												\
		} else {																													\
			std::cout << "CHECK (" << __FILE__ << ":" << __LINE__ << "): "	\
								<< #EXP << std::endl;																	\
			exit(1);																												\
		}																																	\
} while(0)

// TODO: we'd like to be able to toss the optional values around and
// not have too many copies going on. How do we get move semantics?
template <typename T>
class Optional {
public:
	Optional() : is_nil_(true) {}
	explicit Optional(const T& value) : is_nil_(false), value_(value) {}

	bool is_nil() const { return is_nil_; }
	const T& ValueOrDie() const { CHECK(!is_nil_); return value_; }

private:
	bool is_nil_;
	T value_;
};

template <typename T>
Optional<T> Nil() { return Optional<T>(); }

template <typename T>
Optional<T> AsOptional(const T& value) { return Optional<T>(value); }

class Error {
public:
	Error() {}
	explicit Error(const std::string& str) : str_(str) {}
	const std::string& str() const { return str_; }
private:
	std::string str_;
};

template <typename T>
class FixedArray {
public:
	FixedArray() : size_(0), buf_(nullptr) {}
	explicit FixedArray(int n) : size_(n), buf_(new T[n]) {}
	FixedArray(int n, const T& value) : size_(n), buf_(new T[n]) {
		for (int i = 0; i < n; i++) {
			buf_[i] = value;
		}
	}
	FixedArray(const FixedArray<T>& other) : size_(other.size_), buf_(new T[size_]) {
		for (int i = 0; i < size_; i++) {
			buf_[i] = other.buf_[i];
		}
	}
	~FixedArray() { delete[] buf_; }

	FixedArray<T>& operator=(const FixedArray<T>& other) {
		delete[] buf_;
		size_ = other.size_;
		buf_ = new T[size_];
		for (int i = 0; i < size_; i++) {
			buf_[i] = other.buf_[i];
		}
		return *this;
	}

	int size() const { return size_; }
	T* buf() const { return buf_; }
	T& operator[](int i) const {
		CHECK(0 <= i && i <= size_);
		return buf_[i];
	}
private:
	int size_;
	T* buf_;
};

template <typename T>
class ArrayView {
public:
	ArrayView() : array_(nullptr), lo_(0), hi_(0) {}
	explicit ArrayView(const FixedArray<T>* array) : array_(array), lo_(0), hi_(array->size()) {}
	ArrayView(const FixedArray<T>* array, int lo, int hi) : array_(array), lo_(lo), hi_(hi) {}
	int size() const { return hi_ - lo_; }
	const T* buf() const { return array_->buf(); }
	const T& operator[](int i) const {
		CHECK(0 <= i && i <= size());
		return (*array_)[lo_+i];
	}
private:
	const FixedArray<T>* array_;
	int lo_;
	int hi_;
};

class Duration {
public:
	Duration() : nanos_(0) {}
	explicit Duration(int64_t nanos) : nanos_(nanos) {}
	int64_t nanos() const { return nanos_; }
	Duration operator*(Duration other) {
		return Duration(nanos_ * other.nanos_);
	}
	Duration operator+(Duration other) {
		return Duration(nanos_ + other.nanos_);
	}
	Duration& operator=(Duration other) {
		nanos_ = other.nanos_;
		return *this;
	}
	Duration& operator+=(Duration other) {
		nanos_ += other.nanos_;
		return *this;
	}
	bool operator>=(Duration other) {
		return nanos_ >= other.nanos_;
	}
	float operator/(Duration other) {
		return nanos_ / (double)other.nanos_;
	}
private:
	int64_t nanos_;
};

const Duration Nanosecond(1);
const Duration Microsecond = Duration(1000) * Nanosecond;
const Duration Millisecond = Duration(1000) * Microsecond;
const Duration Second = Duration(1000) * Millisecond;

float SecondsPerFrame(int sample_rate);

Duration DurationPerFrame(int sample_rate);

int NextPowerOf2(int k);

inline float RadiansPerFrame(float frequency, float seconds_per_sample) {
  float rotations_per_sample = frequency * seconds_per_sample;
	return rotations_per_sample * TWO_PI;
}

inline float WrapRadians(float radians) {
  if (radians > TWO_PI) {
		return radians - TWO_PI * std::floor(radians/TWO_PI);
	} else if (radians < 0.0) {
		return radians - TWO_PI * std::floor(radians/TWO_PI);
	} else {
		return radians;
	}
}

inline float LinearInterpolation(float left, float right, float amount) {
  return left + amount*(right-left);
}

#endif  // BASE_H_
