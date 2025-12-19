#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Factor;
class Network;
class Problem;
class SignalExperiment;
class Solution;
class SolutionWrapper;

const int SIGNAL_STATUS_INIT = 0;
const int SIGNAL_STATUS_TEST = 1;
const int SIGNAL_STATUS_ESTABLISHED = 2;

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

	std::vector<std::vector<std::vector<double>>> existing_pre_obs;
	std::vector<std::vector<std::vector<double>>> existing_post_obs;
	std::vector<std::vector<double>> existing_target_vals;

	std::vector<std::vector<double>> explore_pre_obs;
	std::vector<std::vector<double>> explore_post_obs;
	std::vector<double> explore_target_vals;

	SignalExperiment* signal_experiment;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void update_signals(SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

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

	std::vector<double> pre_obs;
	std::vector<double> post_obs;

	bool has_explore;

	bool signal_initialized;
	double signal_val;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */