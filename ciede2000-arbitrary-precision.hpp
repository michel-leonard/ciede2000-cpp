// This function written in C++ is not affiliated with the CIE (International Commission on Illumination),
// and is released into the public domain. It is provided "as is" without any warranty, express or implied.

#ifndef CIEDE2000_HPP
#define CIEDE2000_HPP

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <type_traits>

// The multiprecision CIE ΔE2000 implementation, which operates on two L*a*b* colors, and returns their difference.
// "l" ranges from 0 to 100, while "a" and "b" are unbounded and commonly clamped to the range of -128 to 127.
template <typename T>
T ciede2000(T l1, T a1, T b1, T l2, T a2, T b2, T kl = T(1.0), T kc = T(1.0), T kh = T(1.0), bool canonical = false) {
	// Working in C++17 with the CIEDE2000 color-difference formula.
	// kl, kc, kh are parametric factors to be adjusted according to
	// different viewing parameters such as textures, backgrounds...
	using std::sqrt;
	using std::atan2;
	using std::abs;
	using std::sin;
	using std::exp;
	const T pi = boost::math::constants::pi<T>();
	T n = (sqrt(a1 * a1 + b1 * b1) + sqrt(a2 * a2 + b2 * b2)) * T(0.5);
	T t = n * n;
	n *= t * t * t;
	// A factor involving chroma raised to the power of 7 designed to make
	// the influence of chroma on the total color difference more accurate.
	n = T(1.0) + T(0.5) * (T(1.0) - sqrt(n / (n + T(6103515625.0))));
	// Application of the chroma correction factor.
	const T c1 = sqrt(a1 * a1 * n * n + b1 * b1);
	const T c2 = sqrt(a2 * a2 * n * n + b2 * b2);
	// atan2 is preferred over atan because it accurately computes the angle of
	// a point (x, y) in all quadrants, handling the signs of both coordinates.
	T h1 = atan2(b1, a1 * n);
	T h2 = atan2(b2, a2 * n);
	if (h1 < T(0.0)) h1 += T(2.0) * pi;
	if (h2 < T(0.0)) h2 += T(2.0) * pi;
	// When the hue angles lie in different quadrants, the straightforward
	// average can produce a mean that incorrectly suggests a hue angle in
	// the wrong quadrant, the next lines handle this issue.
	T h_mean = (h1 + h2) * T(0.5);
	T h_delta = (h2 - h1) * T(0.5);
	const T epsilon = std::is_same_v<T, float> ? T("1E-6") : T("1E-14");
	// The part where most programmers get it wrong
	if (pi + epsilon < abs(h2 - h1)) {
		h_delta += pi;
		if (canonical && pi + epsilon < h_mean)
			// Sharma’s implementation, OpenJDK, ...
			h_mean -= pi;
		else
			// Lindbloom’s implementation, Netflix’s VMAF, ...
			h_mean += pi;
	}
	const T p = T(36.0) * h_mean - T(55.0) * pi;
	n = (c1 + c2) * T(0.5);
	t = n * n;
	n *= t * t * t;
	// The hue rotation correction term is designed to account for the
	// non-linear behavior of hue differences in the blue region.
	const T r_t = T(-2.0) * sqrt(n / (n + T(6103515625.0))) *
			sin(pi / T(3.0) * exp(p * p / (T(-25.0) * pi * pi)));
			
	n = (l1 + l2) * T(0.5);
	n = (n - T(50.0)) * (n - T(50.0));
	// Lightness.
	const T l = (l2 - l1) / (kl * (T(1.0) + (T(3.0) / T(200.0)) * n / sqrt(T(20.0) + n)));
	// These coefficients adjust the impact of different harmonic
	// components on the hue difference calculation.
	t = T(1.0) - (T(17.0) / T(100.0)) * sin(h_mean + pi / T(3.0)) +
				   (T(6.0)  / T(25.0))  * sin(T(2.0) * h_mean + pi * T(0.5)) +
				   (T(8.0)  / T(25.0))  * sin(T(3.0) * h_mean + T(8.0) * pi / T(15.0)) -
				   (T(1.0)  / T(5.0))   * sin(T(4.0) * h_mean + T(3.0) * pi / T(20.0));
				   
	n = c1 + c2;
	// Hue.
	const T h = T(2.0) * sqrt(c1 * c2) * sin(h_delta) / (kh * (T(1.0) + (T(3.0) / T(400.0)) * n * t));
	// Chroma.
	const T c = (c2 - c1) / (kc * (T(1.0) + (T(9.0) / T(400.0)) * n));
	// The result reflects the actual geometric distance in the color space.
	return sqrt(l * l + h * h + c * c + c * h * r_t);
}

// If you remove the constants 1E-6 and 1E-14, the code will continue to work, but CIEDE2000
// interoperability between all programming languages will no longer be guaranteed.

#endif // CIEDE2000_HPP

// Source code tested by Michel LEONARD
// Website: ciede2000.pages-perso.free.fr
