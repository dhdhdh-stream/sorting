#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

#include "input.h"
#include "signal.h"

class AbstractExperiment;
class AbstractNode;
class AbstractNodeHistory;
class Factor;
class SignalExperiment;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Factor*> factors;

	/**
	 * - separate actions/obs for signal
	 *   - don't have to worry about changes destroying signal
	 *   - don't have to worry about branching
	 * 
	 * - simply only worry about signal at end
	 *   - impact of an explore will be calculated through train_existing step
	 */
	std::vector<int> signal_pre_actions;
	std::vector<int> signal_post_actions;
	std::vector<Signal*> signals;
	double miss_average_guess;

	std::vector<Scope*> child_scopes;

	double signal_misguess_average;
	double signal_misguess_standard_deviation;
	/**
	 * - simply save between updates
	 */

	SignalExperiment* signal_experiment;

	double average_hits_per_run;
	double average_instances_per_run;

	int last_updated_run_index;
	int sum_hits;
	int sum_instances;

	Scope();
	~Scope();

	void signal_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);

	void invalidate_factor(ScopeHistory* scope_history,
						   int f_index);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void clean();
	void measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	std::vector<std::vector<double>> signal_pre_obs;
	std::vector<std::vector<double>> signal_post_obs;

	bool signal_initialized;
	double signal_val;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	ScopeHistory(ScopeHistory* original,
				 int max_index);
	~ScopeHistory();
};

#endif /* SCOPE_H */