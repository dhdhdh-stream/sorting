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
	std::list<double> predict_last_scores;
	std::list<double> measure_last_scores;
	/**
	 * - compare predict against explore directly to not get stuck on mediocre predicts
	 */

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

	std::vector<double> obs;

	std::vector<AbstractNodeHistory*> node_histories;

	ScopeHistory(Scope* scope);
	~ScopeHistory();

	ScopeHistory* train_copy();
};

#endif /* SCOPE_H */