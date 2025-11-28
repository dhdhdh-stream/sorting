#ifndef OFFSET_W_DIFF_H
#define OFFSET_W_DIFF_H

#include <vector>

#include "abstract_problem.h"

class OffsetWDiff : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* OFFSET_W_DIFF_H */