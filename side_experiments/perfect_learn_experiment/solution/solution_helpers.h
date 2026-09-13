#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

class ProblemType;
class Solution;

void train_helper(std::vector<double>& obs,
				  std::vector<int>& actions,
				  double target_val,
				  Solution* solution);

void focus(ProblemType* problem_type,
		   std::vector<int>& actions,
		   Solution* solution);

void compare(ProblemType* problem_type,
			 std::vector<int>& actions);

#endif /* SOLUTION_HELPERS_H */