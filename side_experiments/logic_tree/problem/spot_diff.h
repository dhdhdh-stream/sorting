#ifndef SPOT_DIFF_H
#define SPOT_DIFF_H

#include <vector>

#include "abstract_problem.h"

class SpotDiff : public AbstractProblem {
public:
	void get_instance(std::vector<double>& obs,
					  double& target_val);
};

#endif /* SPOT_DIFF_H */