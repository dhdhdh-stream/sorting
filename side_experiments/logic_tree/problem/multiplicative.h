#ifndef MULTIPLICATIVE_H
#define MULTIPLICATIVE_H

#include <vector>

#include "abstract_problem.h"

class Multiplicative : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* MULTIPLICATIVE_H */