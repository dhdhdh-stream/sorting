#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"

class ObsNetwork;
class ObsNetworkHistory;
class PredictNetwork;
class PredictNetworkHistory;
class Problem;
class ScopeHistory;
class SolutionWrapper;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	bool is_generic;

	int action;

	ObsNetwork* obs_network;

	PredictNetwork* predict_network;

	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_hit;
	double average_instances_per_run;
	std::vector<AbstractExperiment*> experiments;

	int curr_num_instances;

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
					Eigen::VectorXf& state,
					bool& is_done,
					TrainScopeHistory* train_scope_history);
	void train_predict_step(AbstractNodeHistory* history,
							Eigen::VectorXf& state,
							TrainScopeHistory* train_scope_history);

	void predict_step(Eigen::VectorXf& state,
					  AbstractNode*& node_context);

	void copy_from(ActionNode* original);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs;

	ActionNodeHistory(ActionNode* node);
};

class TrainActionNodeHistory : public TrainAbstractNodeHistory {
public:
	ObsNetworkHistory* obs_network_history;

	TrainActionNodeHistory(ActionNode* node);
	~TrainActionNodeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error);
};

class TrainPredictActionNodeHistory : public TrainAbstractNodeHistory {
public:
	PredictNetworkHistory* predict_network_history;

	TrainPredictActionNodeHistory(ActionNode* node);
	~TrainPredictActionNodeHistory();

	void backprop(double target_val,
				  Eigen::VectorXf& state_error);
};

#endif /* ACTION_NODE_H */