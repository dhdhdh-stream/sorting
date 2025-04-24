#ifndef PATTERN_BASE_EXPERIMENT_H
#define PATTERN_BASE_EXPERIMENT_H

// TODO: there are general rules that hold in all circumstances though
// - so not worth adding to score functions all the way down?

// TODO: add in branch experiment?
// - use score functions to help verify that pattern doesn't ruin solution?

/**
 * - for pre and post, fixed surrounding actions/pattern
 *   - look for fixed points + change
 */
// - might be uncertainty, so succeed if a proportion succeed in some way?
const int PATTERN_BASE_EXPERIMENT_STATE_EXPLORE = 0;
/**
 * - random surroundings, if fixed points match, capture
 */
const int PATTERN_BASE_EXPERIMENT_STATE_GATHER = 1;

// - patterns only have limited vision
//   - so problem if actions take you outside where patterns can see/hold

// - have rules and goals
//   - assume rules stop you from going to places too weird
//   - and stop you from doing things too stupid locally
// - goals are patterns
//   - assume pursuing goals keeps you local

class PatternBaseExperiment {
public:
	int state;
	int state_iter;


};

#endif /* PATTERN_BASE_EXPERIMENT_H */