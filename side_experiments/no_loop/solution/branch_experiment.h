#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SEQUENCE = 1;

class BranchExperiment {
public:
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<Sequence*> sequences;
	/**
	 * - don't worry about node ids as should have no impact
	 *   - sequences' scopes use new ids
	 */

	int exit_depth;
	int exit_node_id;

	double average_score;
	double average_misguess;
	double misguess_variance;

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

	std::vector<ActionNodeHistory*> action_histories;
	std::vector<SequenceHistory*> sequence_histories;

	ObsExperimentHistory* obs_experiment_history;


};

#endif /* BRANCH_EXPERIMENT_H */