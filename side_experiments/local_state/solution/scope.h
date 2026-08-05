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
class ActionNode;
class InitNetwork;
class InitNetworkHistory;
class Network;
class ObsNetwork;
class ObsNetworkHistory;
class Problem;
class ScoreNetwork;
class ScoreNetworkHistory;
class Solution;
class SolutionWrapper;
class TrainAbstractNodeHistory;

class ScopeHistory;
class TrainScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	int num_states;

	ObsNetwork* start_obs_network;
	std::vector<std::vector<Scope*>> start_init_network_scope_contexts;
	std::vector<std::vector<int>> start_init_network_node_contexts;
	std::vector<InitNetwork*> start_init_networks;

	ScoreNetwork* end_score_network;

	std::vector<ActionNode*> generic_action_nodes;

	std::vector<Scope*> child_scopes;

	std::list<double> reuse_last_scores;
	std::list<double> new_state_last_scores;

	std::vector<AbstractExperiment*> dependencies;

	Scope();
	~Scope();

	void start_activate(std::vector<double>& obs,
						SolutionWrapper* wrapper);

	void experiment_start_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);

	void train_activate(ScopeHistory* history,
						bool allow_drop,
						Eigen::VectorXf& state,
						TrainScopeHistory* train_scope_history);

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

	std::vector<double> obs;

	std::vector<bool> init_is_match;

	std::vector<AbstractNodeHistory*> node_histories;

	Eigen::VectorXf state;

	std::vector<AbstractExperimentHistory*> experiment_callback_histories;
	std::vector<int> experiment_callback_indexes;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

class TrainScopeHistory {
public:
	Scope* scope;

	bool is_drop;
	ObsNetworkHistory* start_obs_network_history;
	std::vector<InitNetworkHistory*> start_init_network_histories;

	std::vector<TrainAbstractNodeHistory*> node_histories;

	ScoreNetworkHistory* end_score_network_history;

	TrainScopeHistory(Scope* scope);
	~TrainScopeHistory();
};

#endif /* SCOPE_H */