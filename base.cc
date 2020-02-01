#include "base.h"

#include <cmath>

float SecondsPerFrame(int sample_rate) {
	return 1.0 / sample_rate;
}

Duration DurationPerFrame(int sample_rate) {
	return Duration(SecondsPerFrame(sample_rate) * Second.nanos());
}

float RadiansPerFrame(float frequency, float seconds_per_sample) {
	float rotations_per_sample = frequency * seconds_per_sample;
	return rotations_per_sample * TWO_PI;
}

float WrapRadians(float radians) {
	if (radians > TWO_PI) {
		return radians - TWO_PI * std::floor(radians/TWO_PI);
	} else if (radians < 0.0) {
		return radians - TWO_PI * std::floor(radians/TWO_PI);
	} else {
		return radians;
	}
}

int NextPowerOf2(int k) {
	int power = std::floor(std::log2((float)k)) + 1;
	return 1 << power;
}
