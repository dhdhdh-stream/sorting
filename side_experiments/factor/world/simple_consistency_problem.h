// - not hard enough
//   - loop gets sanded into place

// TODO: or want problem where there's reason to stay on track, and reason to stay off track

// - don't worry too much about merging points/getting mixed
//   - if truly need to be considered as separate, can separate later
//   - and getting mixed may lead to better generalization in other scenarios

// - so don't worry about pairing points
//   - not that meaningful if one point is mixed, and one point is not

// - try only worrying about averages

// - or perhaps have paired points, but still calculate deviation from expected
//   - and use pairing over average

// TODO: first verify that maintaining consistency still leads to good results
// - find multiple experiments that maintain consistency
// - run them together

#ifndef SIMPLE_CONSISTENCY_PROBLEM_H
#define SIMPLE_CONSISTENCY_PROBLEM_H

#include <vector>

#include "problem.h"

class SimpleConsistencyProblem : public Problem {
public:
	std::vector<std::vector<int>> world;
	int current_x;
	int current_y;

	std::vector<int> targets;
	int curr_target_index;

	double score;

	SimpleConsistencyProblem();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();
};

class TypeSimpleConsistencyProblem : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

#endif /* SIMPLE_CONSISTENCY_PROBLEM_H */