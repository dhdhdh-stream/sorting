#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

const int LOOP_EXPERIMENT_STATE_EXPLORE = -1;

/**
 * - learn inputs and exits
 *   - for each input, it may be passed into the sequence twice:
 *     - copied, and passed in as input to use existing networks
 *     - updated with new networks
 */
const int LOOP_EXPERIMENT_STATE_EXISTING = 0;
/**
 * - multiple rounds to learn score
 *   - randomize number of iters
 *   - progress from inner to outer
 */
const int LOOP_EXPERIMENT_STATE_NEW = 1;
/**
 * - learn continue and halt networks
 */
const int LOOP_EXPERIMENT_STATE_LOOP = 2;
/**
 * - measure and modify solution if success
 */
const int LOOP_EXPERIMENT_STATE_DONE = 3;

class LoopExperiment {
public:


	std::vector<std::map<int, std::vector<StateNetwork*>>> exit_state_networks;

	std::vector<int> new_state_layers;
	std::vector<std::map<int, std::vector<StateNetwork*>>> new_state_networks;
	std::vector<ScoreNetwork*> new_state_ending_score_networks;

	int test_state_layer;
	std::map<int, std::vector<StateNetwork*>> test_state_networks;
	ScoreNetwork* test_score_network;

	/**
	 * - gradually update as inputs and new state get finalized
	 */
	std::vector<ScopeHistory*> seed_pre_context_histories;
	std::vector<ScopeHistory*> seed_post_context_histories;
	LoopExperimentHistory* seed_experiment_history;
	double seed_target_val;
	/**
	 * - update with values from finalized ending_score_networks
	 */
	double seed_predicted_score;

};

#endif /* LOOP_EXPERIMENT_H */