#ifndef EXPERIMENT_RUN_H
#define EXPERIMENT_RUN_H

#include <map>
#include <vector>

class AbstractExperimentState;
class AbstractNode;
class AbstractNodeHistory;
class CompareExperimentHistory;
class ForceExperiment;
class ForceExperimentHistory;
class Network;
class StateNetworkHistory;
class Wrapper;

class ExperimentRun {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;
	AbstractExperimentState* experiment_context;

	std::vector<double> state;

	bool should_force;
	std::map<ForceExperiment*, ForceExperimentHistory*> force_experiment_histories;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<std::vector<double>> obs_histories;
	std::vector<int> action_histories;

	std::vector<std::vector<StateNetworkHistory*>> obs_network_histories;
	std::vector<std::vector<StateNetworkHistory*>> action_network_histories;
	std::vector<std::vector<Network*>> taken_branch_node_networks;

	CompareExperimentHistory* compare_experiment_history;

	ExperimentRun();
	~ExperimentRun();
};

#endif /* EXPERIMENT_RUN_H */