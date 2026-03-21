# CIEDE2000 color difference formula in C++

This page presents the CIEDE2000 color difference, implemented in the C++ programming language.

![Logo](https://raw.githubusercontent.com/michel-leonard/ciede2000-color-matching/refs/heads/main/docs/assets/images/logo.jpg)

## Our CIEDE2000 offer

This templated production-ready file, released in 2026, contain the CIEDE2000 algorithm.

Source File|Type|Bits|Purpose|Advantage|
|:--:|:--:|:--:|:--:|:--:|
[ciede2000.cpp](./ciede2000.cpp)|`float`|32|General|Lightness, Speed|
[ciede2000.cpp](./ciede2000.cpp)|`double`|64|Scientific|Interoperability|

### Software Versions

- C++11, C++14, C++17, C++20, C++23, C++26
- G++ 13.3
- Clang++ 16

### Example Usage

We calculate the CIEDE2000 distance between two colors, first without and then with parametric factors.

```cpp
// You must name this file "demo.cpp" and include the contents of "ciede2000.cpp" here

#include <iostream>

int main() {
	// Example of two L*a*b* colors
	const double l1 = 49.3, a1 = 115.6, b1 = 15.8;
	const double l2 = 55.5, a2 = 68.6, b2 = -3.7;

	auto delta_e = ciede2000(l1, a1, b1, l2, a2, b2);
	printf("CIEDE2000 = %.14f\n", delta_e); // ΔE2000 = 12.66523228451225

	// Example of parametric factors used in the textile industry
	const double kl = 2.0, kc = 1.0, kh = 1.0;

	// Perform a CIEDE2000 calculation compliant with that of Gaurav Sharma
	const bool canonical = true;

	delta_e = ciede2000(l1, a1, b1, l2, a2, b2, kl, kc, kh, canonical);
	printf("CIEDE2000 = %.14f\n", delta_e); // ΔE2000 = 11.51241722397888
}
```

- Compile with `clang++` or `g++ -std=c++14 -Wall -Wextra -Wpedantic -Ofast -o ciede2000-demo demo.cpp`
- Execute `./ciede2000-demo` to display the calculated color differences

## Public Domain Licence

You are free to use these files, even for commercial purposes.
