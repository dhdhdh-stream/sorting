#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Factor;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Factor*> factors;

	std::vector<Scope*> child_scopes;
	std::vector<int> child_scope_tries;
	std::vector<int> child_scope_successes;

	int num_generalize_successes;

	/**
	 * - will not include new nodes
	 *   - simply don't consider when comparing existing vs. explore
	 */
	std::vector<ScopeHistory*> existing_scope_histories;
	std::vector<double> existing_target_val_histories;

	Scope();
	~Scope();

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

	int num_actions_snapshot;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	ScopeHistory(ScopeHistory* original,
				 int max_index,
				 int num_actions_snapshot);
	~ScopeHistory();
};

#endif /* SCOPE_H */