/**
 * - reducing field of vision might actually lead to better results
 *   - with wide field of vision, can make decision without moving
 *     - so recursion does nothing
 *   - with narrow field of vision, have to move before making decision
 *     - so can recurse after move, now affecting somewhere new
 */

// very difficult to make any progress
// - wide field of view much much easier to make progress

// yeah, so maybe requiring going back to origin not good
// - when every scope needs to go back to origin, pretty much requires recursion to make good progress
// - not to mention that it doesn't necessarily make sense to begin with

// - find other ways of creating in place actions

#ifndef FOCUS_MINESWEEPER_H
#define FOCUS_MINESWEEPER_H

#include <vector>

#include "problem.h"

class FocusMinesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	bool hit_mine;

	FocusMinesweeper();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result(int num_decisions,
						int num_actions);

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
	void print_obs();

private:
	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
	void print_obs_helper(int x, int y);
};

class TypeFocusMinesweeper : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

#endif /* FOCUS_MINESWEEPER_H */