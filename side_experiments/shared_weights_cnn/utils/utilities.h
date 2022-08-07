#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>

inline double randnorm() {
	double value = (double)rand()/(double)RAND_MAX;
	return value;
}

#endif /* UTILITIES_H */