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

void train_decision_on_state_helper(ProblemType* problem_type,
									std::vector<int>& actions_1,
									std::vector<int>& actions_2,
									Solution* solution);

void low_samples(ProblemType* problem_type,
				 std::vector<int>& actions);
void low_samples_w_average(ProblemType* problem_type,
						   std::vector<int>& actions);
void low_samples_w_predict(ProblemType* problem_type,
						   std::vector<int>& actions,
						   Solution* solution);

#endif /* SOLUTION_HELPERS_H */