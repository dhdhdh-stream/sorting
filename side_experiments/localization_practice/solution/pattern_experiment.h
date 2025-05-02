#ifndef PATTERN_EXPERIMENT_H
#define PATTERN_EXPERIMENT_H

/**
 * - pattern 10 in length
 *   - 4 points have to match
 *     - at least 3 unique values
 */
const int PATTERN_EXPERIMENT_STATE_FIND_POTENTIAL = 0;
/**
 * - lead sequence succeeds 20% of the time
 *   - simply for experiment efficiency
 * - return sequence succeeds 50% of the time repeated
 */
const int PATTERN_EXPERIMENT_STATE_VERIFY = 1;

class PatternExperiment {
public:

};

#endif /* PATTERN_EXPERIMENT_H */