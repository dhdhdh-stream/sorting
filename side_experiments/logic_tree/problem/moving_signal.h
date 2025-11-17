#ifndef MOVING_SIGNAL_H
#define MOVING_SIGNAL_H

#include <vector>

#include "abstract_problem.h"

class MovingSignal : public AbstractProblem {
public:
	void get_instance(std::vector<double>& obs,
					  double& target_val);
};

#endif /* MOVING_SIGNAL_H */