#ifndef ABSTRACT_PROBLEM_H
#define ABSTRACT_PROBLEM_H

#include <vector>

class AbstractProblem {
public:
	virtual ~AbstractProblem() {};

	virtual void get_instance(std::vector<double>& obs,
							  double& target_val) = 0;
};

#endif /* ABSTRACT_PROBLEM_H */