#ifndef MOVING_VS_OPENING_H
#define MOVING_VS_OPENING_H

#include <vector>

#include "abstract_problem.h"

class MovingVsOpening : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* MOVING_VS_OPENING_H */