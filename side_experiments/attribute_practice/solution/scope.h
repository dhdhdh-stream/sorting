#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractExperimentHistory;
class AbstractNode;
class AbstractNodeHistory;
class BuildNetwork;
class Problem;
class Solution;
class SolutionWrapper;

#if defined(MDEBUG) && MDEBUG
const int SIGNAL_NUM_SAMPLES = 50;
#else
const int SIGNAL_NUM_SAMPLES = 5000;
#endif /* MDEBUG */

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<Scope*> child_scopes;

	/**
	 * - predict directly against final result
	 *   - don't try to attribute impact to each part of solution
	 *     - setup and actual execution equally responsible for success
	 *     - subtracting many signals from true instable
	 */
	// BuildNetwork* pre_signal;
	// BuildNetwork* post_signal;
	BuildNetwork* signal;

	std::vector<std::vector<double>> explore_pre_obs_histories;
	std::vector<std::vector<double>> explore_post_obs_histories;
	std::vector<double> explore_target_val_histories;
	int explore_history_index;

	std::vector<std::vector<double>> existing_pre_obs_histories;
	std::vector<std::vector<double>> existing_post_obs_histories;
	std::vector<double> existing_target_val_histories;
	int existing_history_index;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<double> pre_obs_history;
	std::vector<double> post_obs_history;

	std::vector<AbstractExperimentHistory*> experiment_callback_histories;
	std::vector<double> experiment_callback_indexes;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original,
				 Solution* parent_solution);
	~ScopeHistory();
};

#endif /* SCOPE_H */