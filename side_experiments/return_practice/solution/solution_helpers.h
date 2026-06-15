#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

class AbstractNode;
class ExperimentRun;
class Wrapper;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count);
void create_experiment(ExperimentRun* run,
					   Wrapper* wrapper);

double predict_helper(std::vector<double>& state,
					  AbstractNode* next_node,
					  Wrapper* wrapper);

void update_solution_helper(ExperimentRun* run,
							double target_val,
							Wrapper* wrapper);

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average);

#endif /* SOLUTION_HELPERS_H */