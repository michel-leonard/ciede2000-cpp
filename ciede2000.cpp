// This function written in C++ is not affiliated with the CIE (International Commission on Illumination),
// and is released into the public domain. It is provided "as is" without any warranty, express or implied.

#include <cmath>

// Expressly defining pi ensures that the code works on different platforms.
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937511
#endif

// The classic CIE ΔE2000 implementation, which operates on two L*a*b* colors, and returns their difference.
// "l" ranges from 0 to 100, while "a" and "b" are unbounded and commonly clamped to the range of -128 to 127.
template<typename T>
static T ciede2000(T l1, T a1, T b1, T l2, T a2, T b2, T kl, T kc, T kh, bool canonical) {
	// Working in C++ with the CIEDE2000 color-difference formula.
	// kl, kc, kh are parametric factors to be adjusted according to
	// different viewing parameters such as textures, backgrounds...
	T n = (std::sqrt(a1 * a1 + b1 * b1) + std::sqrt(a2 * a2 + b2 * b2)) * T(0.5);
	n = n * n * n * n * n * n * n;
	// A factor involving chroma raised to the power of 7 designed to make
	// the influence of chroma on the total color difference more accurate.
	n = T(1.0) + T(0.5) * (T(1.0) - std::sqrt(n / (n + T(6103515625.0))));
	// Application of the chroma correction factor.
	const T c1 = std::sqrt(a1 * a1 * n * n + b1 * b1);
	const T c2 = std::sqrt(a2 * a2 * n * n + b2 * b2);
	// atan2 is preferred over atan because it accurately computes the angle of
	// a point (x, y) in all quadrants, handling the signs of both coordinates.
	T h1 = std::atan2(b1, a1 * n);
	T h2 = std::atan2(b2, a2 * n);
	h1 += (h1 < T(0.0)) * T(2.0) * T(M_PI);
	h2 += (h2 < T(0.0)) * T(2.0) * T(M_PI);
	// When the hue angles lie in different quadrants, the straightforward
	// average can produce a mean that incorrectly suggests a hue angle in
	// the wrong quadrant, the next lines handle this issue.
	T h_mean = (h1 + h2) * T(0.5);
	T h_delta = (h2 - h1) * T(0.5);
	constexpr T epsilon = T(sizeof(T) == sizeof(float) ? 1E-6 : 1E-14);
	// The part where most programmers get it wrong
	if (T(M_PI) + epsilon < std::fabs(h2 - h1)) {
		h_delta += T(M_PI);
		if (canonical && T(M_PI) + epsilon < h_mean)
			// Sharma’s implementation, OpenJDK, ...
			h_mean -= T(M_PI);
		else
			// Lindbloom’s implementation, Netflix’s VMAF, ...
			h_mean += T(M_PI);
	}
	const T p = T(36.0) * h_mean - T(55.0) * T(M_PI);
	n = (c1 + c2) * T(0.5);
	n = n * n * n * n * n * n * n;
	// The hue rotation correction term is designed to account for the
	// non-linear behavior of hue differences in the blue region.
	const T r_t = T(-2.0) * std::sqrt(n / (n + T(6103515625.0)))
			* std::sin(T(M_PI) / T(3.0) * std::exp(p * p / (T(-25.0) * T(M_PI) * T(M_PI))));
	n = (l1 + l2) * T(0.5);
	n = (n - T(50.0)) * (n - T(50.0));
	// Lightness.
	const T l = (l2 - l1) / (kl * (T(1.0) + T(3.0) / T(200.0) * n / std::sqrt(T(20.0) + n)));
	// These coefficients adjust the impact of different harmonic
	// components on the hue difference calculation.
	const T t = T(1.0) 	- T(17.0) / T(100.0) * std::sin(h_mean + T(M_PI) / T(3.0))
				+ T(6.0) / T(25.0) * std::sin(T(2.0) * h_mean + T(M_PI) / T(2.0))
				+ T(8.0) / T(25.0) * std::sin(T(3.0) * h_mean + T(8.0) * T(M_PI) / T(15.0))
				- T(1.0) / T(5.0) * std::sin(T(4.0) * h_mean + T(3.0) * T(M_PI) / T(20.0));
	n = c1 + c2;
	// Hue.
	const T h = T(2.0) * std::sqrt(c1 * c2) * std::sin(h_delta) / (kh * (T(1.0) + T(3.0) / T(400.0) * n * t));
	// Chroma.
	const T c = (c2 - c1) / (kc * (T(1.0) + T(9.0) / T(400.0) * n));
	// The result reflects the actual geometric distance in the color space.
	// Given a tolerance of 0.00014 in 32 bits.
	// Given a tolerance of 3.4e-13 in 64 bits.
	return std::sqrt(l * l + h * h + c * c + c * h * r_t);
}

// If you remove the constant 1E-14, the code will continue to work, but CIEDE2000
// interoperability between all programming languages will no longer be guaranteed.
