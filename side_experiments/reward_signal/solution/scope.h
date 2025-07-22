#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class AbstractNodeHistory;
class Factor;
class Problem;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Factor*> factors;

	double score_average_val;
	std::vector<Input> score_inputs;
	std::vector<double> score_input_averages;
	std::vector<double> score_input_standard_deviations;
	std::vector<double> score_weights;
	/**
	 * - protect signal once created
	 *   - score must be maintained for any experiment prior or within
	 *     - on measure, calc all reward signals
	 */

	std::vector<Scope*> child_scopes;

	std::vector<ScopeHistory*> explore_scope_histories;
	std::vector<double> explore_target_val_histories;

	Scope();
	~Scope();

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

	bool signal_initialized;
	double signal_val;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	ScopeHistory(ScopeHistory* original,
				 int max_index);
	~ScopeHistory();
};

#endif /* SCOPE_H */