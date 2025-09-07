/**
 * - use explore for average misguess
 * 
 * - current needs to be as accurate as possible
 *   - i.e., need to try to match all
 * 
 * - trap and explore for counterexamples
 *   - trap doesn't need to all be matched
 *     - non-matches will have low score so won't be a threat
 *   - negative predictions don't need to be accurate
 *     - just need to be slightly negative for explore to shy away
 * 
 * - positive for generalization
 *   - wants to be matched as much as possible
 * 
 * - gather distribution:
 *   - 25% current
 *   - 25% existing
 *   - 25% trap
 *   - 25% explore
 * - determine matches from gathered
 *   - resulting distribution will likely not match this initial distribution
 */

/**
 * TODO: balance explore num samples with others
 */

#ifndef SIGNAL_EXPERIMENT_H
#define SIGNAL_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class Explore;
class Problem;
class Scope;
class ScopeHistory;
class SignalInstance;
class SignalNetwork;
class SolutionWrapper;

const int SIGNAL_EXPERIMENT_STATE_MEASURE_POSITIVE = 0;
/**
 * TODO: measure positive together at start of cycle
 */
const int SIGNAL_EXPERIMENT_STATE_FIND_SAFE = 1;
/**
 * - include current but not traps
 */
const int SIGNAL_EXPERIMENT_STATE_GATHER_TRAP = 2;
const int SIGNAL_EXPERIMENT_STATE_GATHER_CURRENT = 3;
const int SIGNAL_EXPERIMENT_STATE_EXPLORE = 4;
const int SIGNAL_EXPERIMENT_STATE_DONE = 5;

const int SIGNAL_EXPERIMENT_SOLUTION_TYPE_POSITIVE = 0;
const int SIGNAL_EXPERIMENT_SOLUTION_TYPE_TRAP = 1;
const int SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT = 2;

class SignalExperimentHistory;
class SignalExperiment : public AbstractExperiment {
public:
	int scope_context_id;

	int state;
	int state_iter;
	int solution_type;
	int solution_index;

	/**
	 * - using current signal actions
	 */
	std::vector<std::vector<double>> existing_positive_scores;

	std::vector<double> existing_positive_score_averages;
	std::vector<double> existing_positive_score_standard_deviations;

	std::vector<int> pre_actions;
	std::vector<int> post_actions;

	std::vector<double> new_current_scores;
	std::vector<std::vector<double>> new_positive_scores;

	std::vector<std::vector<std::vector<double>>> positive_pre_obs;
	std::vector<std::vector<std::vector<double>>> positive_post_obs;
	std::vector<double> positive_scores;

	std::vector<std::vector<std::vector<double>>> trap_pre_obs;
	std::vector<std::vector<std::vector<double>>> trap_post_obs;
	std::vector<double> trap_scores;

	std::vector<std::vector<std::vector<double>>> current_pre_obs;
	std::vector<std::vector<std::vector<double>>> current_post_obs;
	std::vector<double> current_scores;

	double average_instances_per_run;
	int num_instances_until_target;

	AbstractNode* explore_node;
	bool explore_is_branch;

	Scope* new_scope;
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;
	AbstractNode* exit_next_node;

	std::vector<std::vector<std::vector<double>>> explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> explore_post_obs;
	std::vector<double> explore_scores;

	std::vector<SignalInstance*> instances;
	double default_guess;

	double misguess_average;

	SignalExperiment(int scope_context_id,
					 SolutionWrapper* wrapper);
	~SignalExperiment();

	bool check_signal(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	bool measure_positive_check_signal(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper);
	void measure_positive_backprop(double target_val,
								   SolutionWrapper* wrapper);

	bool find_safe_check_signal(std::vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper);
	void find_safe_backprop(double target_val,
							SolutionWrapper* wrapper);

	bool gather_trap_check_signal(std::vector<double>& obs,
								  int& action,
								  bool& is_next,
								  SolutionWrapper* wrapper);
	void gather_trap_backprop(double target_val,
							  SolutionWrapper* wrapper);

	bool gather_current_check_signal(std::vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper);
	void gather_current_backprop(double target_val,
								 SolutionWrapper* wrapper);

	bool explore_check_signal(std::vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper);
	void explore_check_activate(AbstractNode* experiment_node,
								bool is_branch,
								SolutionWrapper* wrapper);
	void explore_experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper);
	void explore_set_action(int action,
							SolutionWrapper* wrapper);
	void explore_experiment_exit_step(SolutionWrapper* wrapper);
	void explore_backprop(double target_val,
						  SolutionWrapper* wrapper);

	void add(SolutionWrapper* wrapper);

private:
	void set_actions(SolutionWrapper* wrapper);
	void set_explore(SolutionWrapper* wrapper);
	bool check_instance_still_good(SignalInstance* instance,
								   SolutionWrapper* wrapper);
	bool split_helper(std::vector<bool>& positive_has_match,
					  std::vector<bool>& current_has_match,
					  std::vector<bool>& new_match_input_is_pre,
					  std::vector<int>& new_match_input_indexes,
					  std::vector<int>& new_match_input_obs_indexes,
					  SignalNetwork*& new_match_network);
	void train_score(std::vector<std::vector<std::vector<double>>>& pre_obs_histories,
					 std::vector<std::vector<std::vector<double>>>& post_obs_histories,
					 std::vector<double>& target_val_histories,
					 std::vector<bool>& new_score_input_is_pre,
					 std::vector<int>& new_score_input_indexes,
					 std::vector<int>& new_score_input_obs_indexes,
					 SignalNetwork*& new_score_network);
	void create_reward_signal_helper(SolutionWrapper* wrapper);
};

class SignalExperimentInstanceHistory {
public:
	ScopeHistory* scope_history;
	ScopeHistory* signal_needed_from;

	SignalExperimentInstanceHistory();
};

class SignalExperimentState : public AbstractExperimentState {
public:
	int step_index;

	SignalExperimentState(SignalExperiment* experiment);
};

#endif /* SIGNAL_EXPERIMENT_H */