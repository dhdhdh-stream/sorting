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