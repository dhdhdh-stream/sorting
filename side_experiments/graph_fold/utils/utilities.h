#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>

inline double randuni() {
	double value = (double)rand()/(double)RAND_MAX;
	return value;
}

#endif /* UTILITIES_H */