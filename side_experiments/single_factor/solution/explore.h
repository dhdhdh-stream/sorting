/**
 * - for inputs, try negating input too
 *   - since dependency can be either polarity
 */

#ifndef EXPLORE_H
#define EXPLORE_H

const int EXPLORE_STATE_EXPLORE_SETUP = 0;
const int EXPLORE_STATE_EXPLORE = 1;
const int EXPLORE_STATE_EXPERIMENT_SETUP = 2;
// TODO: when for decision, use a lot more pre_obs_snapshots
const int EXPLORE_STATE_EXPERIMENT_DECISION = 2;
const int EXPLORE_STATE_EXPERIMENT_OVERALL = 3;
const int EXPLORE_STATE_DONE = 4;

class Explore {
public:
	AbstractNode* explore_node;

	std::vector<int> scope_context;
	std::vector<int> node_context;

	int state;
	int state_iter;

	ScoreNetwork* score_network;
	ScoreNetwork* misguess_network;

	double best_surprise;
	AbstractExperiment* best_experiment;
	ScopeHistory* best_seed;	// TODO: include explore

	std::vector<std::vector<AbstractNode*>> obs_nodes;
	std::vector<std::vector<bool>> obs_is_pre;
	std::vector<std::vector<int>> obs_indexes;
	std::vector<std::vector<std::vector<int>>> obs_scope_contexts;
	std::vector<std::vector<std::vector<int>>> obs_node_contexts;
	std::vector<std::vector<StateNetwork*>> state_networks;
	std::vector<Scale*> scales;

	std::vector<AbstractNode*> test_obs_nodes;
	std::vector<bool> test_obs_is_pre;
	std::vector<int> test_obs_indexes;
	/**
	 * - include -1s if in Sequence
	 */
	std::vector<std::vector<int>> test_obs_scope_contexts;
	std::vector<std::vector<int>> test_obs_node_contexts;
	FlatNetwork* test_flat_network;
	std::vector<StateNetwork*> test_state_networks;
	Scale* test_scale;

	/**
	 * - index 0 is global
	 * 
	 * - learn during experiment
	 *   - discard if > 0.75
	 *   - set to 0.0 if < 0.1
	 */
	/**
	 * - setting this way means that in-between contexts can't benefit from state_weights potentially becoming 0.0
	 *   - but harm is just a small bit of runtime
	 *     - TODO: can prevent this through better logic/targets
	 */
	std::vector<std::vector<double>> weight_mods;

};

class ExploreHistory {
public:
	Explore* explore;

	AbstractExperimentHistory* experiment_history;

	double running_average_score_snapshot;
	double running_average_misguess_snapshot;
	double scale_factor_snapshot;

	ScoreNetworkHistory* score_network_history;
	double score_network_output;
	ScoreNetworkHistory* misguess_network_history;
	double misguess_network_output;

	/**
	 * - index 0 is global
	 */
	std::vector<ScopeHistory*> context_scope_histories;

	std::vector<double> state_vals;

	std::vector<std::vector<double>> test_obs_vals;

};

#endif /* EXPLORE_H */