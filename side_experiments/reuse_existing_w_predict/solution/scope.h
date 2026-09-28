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

	std::vector<Scope*> child_scopes;

	std::vector<std::list<double>> last_scores;
	std::list<double> predict_last_scores;

	Scope();
	~Scope();

	void train_activate(ScopeHistory* history,
						Eigen::VectorXf& state,
						bool& hit_explore,
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

	std::vector<AbstractNodeHistory*> node_histories;

	int explore_index;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

class TrainScopeHistory {
public:
	Scope* scope;

	std::vector<TrainAbstractNodeHistory*> node_histories;

	TrainScopeHistory(Scope* scope);
	~TrainScopeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error,
				  SolutionWrapper* wrapper);
	void update(int iter_index);
};

#endif /* SCOPE_H */