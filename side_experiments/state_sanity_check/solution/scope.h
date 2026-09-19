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
class Problem;
class ScopeNode;
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

	std::vector<std::vector<Scope*>> start_init_network_scope_contexts;
	std::vector<std::vector<int>> start_init_network_node_contexts;
	std::vector<InitNetwork*> start_init_networks;

	std::vector<InitNetwork*> obs_networks;

	std::vector<Scope*> child_scopes;

	std::list<double> train_reuse_last_scores;
	std::list<double> measure_reuse_last_scores;
	std::list<double> train_new_state_last_scores;
	std::list<double> measure_new_state_last_scores;

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

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

class TrainScopeHistory {
public:
	Scope* scope;

	bool is_drop;
	std::vector<InitNetworkHistory*> start_init_network_histories;
	std::vector<InitNetworkHistory*> obs_network_histories;

	std::vector<TrainAbstractNodeHistory*> node_histories;

	TrainScopeHistory(Scope* scope);
	~TrainScopeHistory();
};

#endif /* SCOPE_H */