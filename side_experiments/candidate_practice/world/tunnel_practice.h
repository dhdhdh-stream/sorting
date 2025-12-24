/**
 * - obs:
 *   - location
 *   - target
 *   - chase
 *   - [10 noise]
 */

// - how to tunnel?
//   - focus on narrow span of time
//   - either focus on narrow obs or narrow image?
//     - look for fuzzy image that correlates with success?
//     - maybe goal is to make a pattern appear more often?

// - when using tunnel, unless average significantly worse, then completely ignore true?

// - so from good situation, pick pattern
//   - filter to 5% most similar and see if correlate to success
//   - if does, then commit
//     - on train new, check that average is not significantly worse
//       - if not, fully commit, ignoring everything else

// - continuous or no?
//   - maybe have trying to match pattern as much as possible
//     - with closer matches having higher score
//       - can kind of get continuous without explicitly looking for it
//         - (which seems impractical)
// - correlate similarity with true?

// - maybe start with trying to hit a discrete pattern more
// - maybe have both similarity and signal

// - maybe even if there's noise, trying to optimize will only improve chase
//   - so even if not similar, will still work?

#ifndef TUNNEL_PRACTICE_H
#define TUNNEL_PRACTICE_H

#include <vector>

#include "problem.h"

const int TUNNEL_PRACTICE_ACTION_LEFT = 0;
const int TUNNEL_PRACTICE_ACTION_RIGHT = 1;
const int TUNNEL_PRACTICE_ACTION_CLICK = 2;

class TunnelPractice : public Problem {
public:
	int curr_index;

	std::vector<int> targets;
	int curr_target_index;

	double score;

	std::vector<double> noise;

	TunnelPractice();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeTunnelPractice : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* TUNNEL_PRACTICE_H */