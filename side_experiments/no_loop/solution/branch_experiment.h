#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SEQUENCE = 1;

const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN = 1;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 2;
const int BRANCH_EXPERIMENT_STATE_DONE = 3;

class BranchExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	double average_remaining_experiments_from_start;
	double average_instances_per_run;
	/**
	 * - when triggering an experiment, it becomes live everywhere
	 *   - for one selected instance, always trigger branch and experiment
	 *     - for everywhere else, trigger accordingly
	 *       - so experiment score_state scales need to be adjusted after ObsExperiment
	 * 
	 * - set probabilities after average_instances_per_run to 50%
	 */

	int state;
	int state_iter;

	std::vector<int> curr_step_types;
	std::vector<ActionNode*> curr_actions;
	std::vector<Sequence*> curr_sequences;
	int curr_exit_depth;
	int curr_exit_node_id;

	double best_surprise;
	/**
	 * - score improvement divided by average_misguess
	 */
	std::vector<int> best_step_types;
	std::vector<ActionNode*> best_actions;
	std::vector<Sequence*> best_sequences;
	/**
	 * - don't worry about node ids as should have no impact
	 *   - sequences' scopes use new ids
	 */
	int best_exit_depth;
	int best_exit_node_id;

	double average_score;
	double average_misguess;
	double misguess_variance;

	int branch_count;
	double combined_score;

	std::map<State*, Scale*> score_state_scales;

	std::vector<State*> new_score_states;
	/**
	 * - if branch, then keep states needed for decision and remove rest
	 *   - not trained to be robust against branch
	 * - if replace, then can keep all
	 */
	std::vector<std::vector<AbstractNode*>> new_score_state_nodes;
	std::vector<std::vector<std::vector<int>>> new_score_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_score_state_node_contexts;
	std::vector<std::vector<int>> new_score_state_obs_indexes;

	ObsExperiment* obs_experiment;


};

class BranchExperimentHistory {
public:
	BranchExperiment* experiment;

	std::vector<SequenceHistory*> sequence_histories;

	double existing_predicted_score;

	ScopeHistory* parent_scope_history;
	std::map<State*, StateStatus> experiment_score_state_snapshots;



};

#endif /* BRANCH_EXPERIMENT_H */