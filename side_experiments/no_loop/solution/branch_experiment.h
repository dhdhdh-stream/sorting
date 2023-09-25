/**
 * - evaluate state at both decision point as well as end
 *   - evaluate by comparing error with and without state
 *   - if state impact differs at decision point, finalize state
 *   - if state impact differs at end, add on/off
 *     - or remove if decision point is outside of scope
 */

// TODO: evaluate score_state to try to measure new misguess
// - use to determine if pass through

// - during branch experiments, still look for new state
//   - can be only way to make progress

#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int STEP_TYPE_ACTION = 0;
const int STEP_TYPE_SEQUENCE = 1;

class BranchExperiment {
public:
	int num_steps;
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<Sequence*> sequences;

	std::map<ScoreState*, Scale*> score_state_scales;

	std::vector<ScoreState*> new_score_states;
	/**
	 * - if branch, then keep states needed for decision and remove rest
	 *   - not trained to be robust against branch
	 * - if replace, then can keep all
	 */
	std::vector<std::vector<std::vector<int>>> new_score_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> new_score_state_node_contexts;
	std::vector<std::vector<StateNetwork*>> new_score_state_networks;

	

};

#endif /* BRANCH_EXPERIMENT_H */