#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <list>
#include <map>
#include <vector>

#include <Eigen/Dense>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;
class AbstractNodeHistory;
class InitNetwork;
class NegateNetwork;
class Network;
class ObsNetwork;
class Problem;
class ScoreNetwork;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<NegateNetwork*> start_negate_networks;
	ObsNetwork* start_obs_network;
	std::vector<std::vector<Scope*>> start_init_network_scope_contexts;
	std::vector<std::vector<int>> start_init_network_node_contexts;
	std::vector<InitNetwork*> start_init_networks;

	ScoreNetwork* explore_score_network;

	std::vector<AbstractExperiment*> dependencies;

	std::vector<Scope*> child_scopes;

	std::list<double> reuse_last_scores;
	std::list<double> new_state_last_scores;

	Scope();
	~Scope();

	void start_activate(std::vector<double>& obs,
						SolutionWrapper* wrapper);

	void experiment_start_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	Eigen::VectorXf state;
	std::vector<double> obs;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<AbstractExperimentHistory*> experiment_callback_histories;
	std::vector<int> experiment_callback_indexes;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */