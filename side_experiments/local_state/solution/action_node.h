#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"

class ActionNetwork;
class ActionNetworkHistory;
class InitNetwork;
class InitNetworkHistory;
class ObsNetwork;
class ObsNetworkHistory;
class Problem;
class ScopeHistory;
class ScoreNetwork;
class ScoreNetworkHistory;
class SolutionWrapper;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	bool is_generic;

	int action;

	ActionNetwork* action_network;

	ObsNetwork* obs_network;

	std::vector<std::vector<Scope*>> init_network_scope_contexts;
	std::vector<std::vector<int>> init_network_node_contexts;
	std::vector<InitNetwork*> init_networks;

	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_hit;
	double average_instances_per_run;

	ScoreNetwork* score_network;

	AbstractExperiment* experiment;

	int curr_num_instances;

	std::vector<AbstractExperiment*> dependencies;

	ActionNode();
	~ActionNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void step_callback(std::vector<double>& obs,
					   SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_step_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);

	void train_step(AbstractNodeHistory* history,
					bool allow_drop,
					Eigen::VectorXf& state,
					TrainScopeHistory* train_scope_history);

	void copy_from(ActionNode* original,
				   Solution* parent_solution);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs;

	std::vector<bool> init_is_match;

	Eigen::VectorXf state;

	ActionNodeHistory(ActionNode* node);
};

class TrainActionNodeHistory : public TrainAbstractNodeHistory {
public:
	ActionNetworkHistory* action_network_history;
	ObsNetworkHistory* obs_network_history;
	std::vector<InitNetworkHistory*> init_network_histories;

	ScoreNetworkHistory* score_network_history;

	TrainActionNodeHistory(ActionNode* node);
	~TrainActionNodeHistory();
};

#endif /* ACTION_NODE_H */