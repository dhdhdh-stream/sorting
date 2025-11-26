#ifndef MINESWEEPER_SIGNAL_H
#define MINESWEEPER_SIGNAL_H

#include <vector>

#include "abstract_problem.h"

class MinesweeperSignal : public AbstractProblem {
public:
	std::vector<std::vector<std::vector<double>>> signal_pre_obs;
	std::vector<std::vector<std::vector<double>>> signal_post_obs;
	std::vector<std::vector<double>> signal_scores;
	std::vector<std::vector<std::vector<double>>> explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> explore_post_obs;
	std::vector<std::vector<double>> explore_scores;

	MinesweeperSignal();

	void get_instance(std::vector<double>& obs,
					  double& target_val);
};

#endif /* MINESWEEPER_SIGNAL_H */