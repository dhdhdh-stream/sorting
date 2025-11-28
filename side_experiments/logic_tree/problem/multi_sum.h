#ifndef MULTI_SUM_H
#define MULTI_SUM_H

#include <vector>

#include "abstract_problem.h"

class MultiSum : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* MULTI_SUM_H */