#ifndef MANUAL_SOLUTION_H
#define MANUAL_SOLUTION_H

#include <vector>

#include "action.h"

class ManualSolution {
public:
	virtual ~ManualSolution() {};
	virtual void step(std::vector<double>& observations,
					  bool& done,
					  std::vector<Action>& actions) = 0;
};

#endif /* MANUAL_SOLUTION_H */