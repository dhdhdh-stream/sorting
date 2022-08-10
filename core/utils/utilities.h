#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>

#include "action.h"

inline double randnorm() {
	double value = (double)rand()/(double)RAND_MAX;
	return value;
}

int calculate_action_path_length(Action action);

#endif /* UTILITIES_H */