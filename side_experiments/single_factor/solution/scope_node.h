#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_indexes;
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;

	/**
	 * - state indexes that aren't passed into top layer
	 *   - so includes state indexes skip passed into lower layers
	 *     - may result in duplicate values across pre_obs_snapshots
	 *     - but will also prevent values from missing
	 */
	std::vector<int> pre_obs_state_indexes;
	/**
	 * - for inner scope state indexes
	 *   - so target_layer == 0
	 *     - simply due to the way state networks are constructed during experiments
	 * 
	 * - don't activate/clear for Sequence EndingScopeNodeHelpers
	 */
	std::vector<int> pre_state_network_indexes;
	std::vector<StateNetwork*> pre_state_networks;

	Scale* scope_scale_mod;

	/**
	 * - activate even if early exit
	 */
	std::vector<int> post_state_network_indexes;
	std::vector<StateNetwork*> post_state_networks;

	int next_node_id;

	bool is_explore;
	int explore_curr_try;
	double explore_best_surprise;
	AbstractExperiment* explore_best_experiment;

	AbstractExperiment* experiment;



	void seed_activate(ScopeNodeHistory* seed_history,
					   ScopeNodeHistory* history);

};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	/**
	 * - state that isn't passed in
	 */
	std::vector<double> pre_obs_snapshot;

	std::vector<int> pre_state_network_indexes;
	std::vector<StateNetworkHistory*> pre_state_network_histories;

	ScopeHistory* inner_scope_history;

	/**
	 * - state initialized in inner
	 */
	std::vector<double> post_obs_snapshot;

	std::vector<int> post_state_network_indexes;
	std::vector<StateNetworkHistory*> post_state_network_histories;

	AbstractExperimentHistory* experiment_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);	// deep copy for seed
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */