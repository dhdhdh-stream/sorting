/**
 * TODO: track init/action likelihoods
 */

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <vector>

class Assignment {
public:
	int init_x;
	int init_y;

	std::vector<int> x_impact;
	std::vector<int> y_impact;
};

#endif /* ASSIGNMENT_H */