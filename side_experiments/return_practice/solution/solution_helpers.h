#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

class ExperimentRun;
class Wrapper;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count);
void create_experiment(ExperimentRun* run,
					   Wrapper* wrapper);
void create_crazy(ExperimentRun* run,
				  Wrapper* wrapper);

void update_solution_helper(ExperimentRun* run,
							double target_val);

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average);
void measure_test(Wrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */