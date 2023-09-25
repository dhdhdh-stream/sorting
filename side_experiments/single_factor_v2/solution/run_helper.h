#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	/**
	 * - don't need to track running tally of predicted score
	 *   - instead, on experiment, calculate from partial and resolved score_state_vals
	 */

	std::map<ScoreState*, double> score_state_vals;

};

#endif /* RUN_HELPER_H */