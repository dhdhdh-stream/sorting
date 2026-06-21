#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

class AbstractNode;
class ExperimentRun;
class Network;
class Wrapper;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count);
void create_experiment(ExperimentRun* run,
					   Wrapper* wrapper);
void create_force_experiment(ExperimentRun* run,
							 Wrapper* wrapper);

void calc_state_helper(std::vector<std::vector<double>>& obs,
					   std::vector<int>& actions,
					   Wrapper* wrapper,
					   std::vector<double>& branch_state);
double predict_helper(std::vector<double>& state,
					  AbstractNode* next_node,
					  Wrapper* wrapper);

void update_solution_helper(ExperimentRun* run,
							double target_val,
							Wrapper* wrapper);

/**
 * TODO: special case experiment from EndNode
 */
void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							AbstractNode* exit_next_node,
							Wrapper* wrapper);
void finalize_helper(AbstractNode* node_context,
					 bool is_branch,
					 std::vector<int> actions,
					 AbstractNode* exit_next_node,
					 Network* original_network,
					 Network* branch_network,
					 Wrapper* wrapper,
					 int type);

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average);

#endif /* SOLUTION_HELPERS_H */