#ifndef SPOT_DIFF_H
#define SPOT_DIFF_H

#include <vector>

#include "abstract_problem.h"

class SpotDiff : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* SPOT_DIFF_H */