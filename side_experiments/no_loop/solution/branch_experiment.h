#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SEQUENCE = 1;

// TODO: add state explore
// TODO: explore performance can't be negative
// - it can be about even and hope that there's information
const int BRANCH_EXPERIMENT_STATE_EXPLORE = 0;
const int BRANCH_EXPERIMENT_STATE_TRAIN = 1;
const int BRANCH_EXPERIMENT_STATE_MEASURE = 2;
const int BRANCH_EXPERIMENT_STATE_DONE = 3;

class BranchExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<Sequence*> sequences;
	/**
	 * - don't worry about node ids as should have no impact
	 *   - sequences' scopes use new ids
	 */

	int exit_depth;
	int exit_node_id;

	int state;
	int state_iter;

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

	std::map<State*, StateStatus> score_state_snapshots;
	std::map<State*, StateStatus> experiment_score_state_snapshots;

	std::vector<int> test_obs_indexes;
	std::vector<double> test_obs_vals;


};

#endif /* BRANCH_EXPERIMENT_H */