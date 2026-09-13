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

void predict_decision_helper(ProblemType* problem_type,
							 std::vector<int>& actions_1,
							 std::vector<int>& actions_2,
							 Solution* solution);

void train_decision_helper(ProblemType* problem_type,
						   std::vector<int>& actions_1,
						   std::vector<int>& actions_2);

#endif /* SOLUTION_HELPERS_H */