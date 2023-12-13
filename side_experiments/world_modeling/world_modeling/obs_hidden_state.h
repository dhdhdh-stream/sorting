/**
 * - question is: does splitting obs result in better predictions?
 */

// - obs matter if can make an accurate prediction from it, and it matters?

// - world modeling can never be perfect
//   - can only predict impact of actions in general context
//     - but specific sequences can create unique factors that change impact

// - so is the goal to learn as many of these unique factors as possible?
//   - unique factors can be combinations of actions
//   - specific obs
//     - combinations of specific obs
//       - combinations of combinations

// - so what's the point?
//   - maybe the factors need to generalize?

// - do state values keep track of context, or do hidden states keep track of context?
//   - maybe state values keep track of linear contexts?
//   - hidden states keep track of non-linear contexts?

// - for hidden states, how to generalize?
//   - how to chain/combine hidden states into each other?

// - for state values, how to update?
//   - do you try to measure impact of all combinations of state values?
//     - probably no as would be non-linear

// - so in general context, actions modify state values passed to them
//   - state values then affect both obs and result

// - branch/create hidden state when significant non-linear impact

// - compound actions can get updated, so may need to update their impact every so often

#ifndef OBS_HIDDEN_STATE_H
#define OBS_HIDDEN_STATE_H

class ObsHiddenState : public AbstractHiddenState {
public:
	int id;

	std::vector<AbstractHiddenState*> transitions;
	std::vector<double> transition_means;
	std::vector<double> transition_variances;
	/**
	 * - for when no obs
	 */
	std::vector<double> transition_probabilities;
	
};

#endif /* OBS_HIDDEN_STATE_H */