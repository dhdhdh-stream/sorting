#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <list>
#include <map>
#include <vector>

#include <Eigen/Dense>

class AbstractNode;
class AbstractNodeHistory;
class ActionNode;
class Network;
class ObsNetwork;
class ObsNetworkHistory;
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

	ObsNetwork* obs_network;

	ScoreNetwork* score_network;

	std::vector<Scope*> child_scopes;

	std::vector<ActionNode*> generic_action_nodes;
	std::vector<ScopeNode*> generic_scope_nodes;

	std::vector<std::list<double>> last_scores;

	Scope();
	~Scope();

	void start_activate(std::vector<double>& obs,
						SolutionWrapper* wrapper);

	void experiment_start_activate(std::vector<double>& obs,
								   SolutionWrapper* wrapper);

	void train_activate(ScopeHistory* history,
						Eigen::VectorXf& state,
						bool& is_done,
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

	std::vector<AbstractNodeHistory*> node_histories;

	int explore_index;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

class TrainScopeHistory {
public:
	Scope* scope;

	ObsNetworkHistory* obs_network_history;

	std::vector<TrainAbstractNodeHistory*> node_histories;

	bool is_explore;
	ScoreNetworkHistory* score_network_history;

	TrainScopeHistory(Scope* scope);
	~TrainScopeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error);
};

#endif /* SCOPE_H */