#ifndef UTILITIES_H
#define UTILITIES_H

#include <cmath>

inline double cumulative_normal(double x) {
	return 0.5 * std::erfc(-x * M_SQRT1_2);
}

#endif /* UTILITIES_H */