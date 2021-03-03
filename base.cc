#include "base.h"

#include <cmath>

float SecondsPerFrame(int sample_rate) {
	return 1.0 / sample_rate;
}

Duration DurationPerFrame(int sample_rate) {
	return Duration(SecondsPerFrame(sample_rate) * Second.nanos());
}

int NextPowerOf2(int k) {
	int power = std::floor(std::log2((float)k)) + 1;
	return 1 << power;
}
