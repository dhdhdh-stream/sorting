#ifndef MINESWEEPER_SIGNAL_H
#define MINESWEEPER_SIGNAL_H

#include <vector>

#include "abstract_problem.h"

class MinesweeperSignal : public AbstractProblem {
public:
	std::vector<std::vector<std::vector<double>>> train_signal_pre_obs;
	std::vector<std::vector<std::vector<double>>> train_signal_post_obs;
	std::vector<std::vector<double>> train_signal_scores;
	std::vector<std::vector<std::vector<double>>> train_explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> train_explore_post_obs;
	std::vector<std::vector<double>> train_explore_scores;

	std::vector<std::vector<std::vector<double>>> test_signal_pre_obs;
	std::vector<std::vector<std::vector<double>>> test_signal_post_obs;
	std::vector<std::vector<double>> test_signal_scores;
	std::vector<std::vector<std::vector<double>>> test_explore_pre_obs;
	std::vector<std::vector<std::vector<double>>> test_explore_post_obs;
	std::vector<std::vector<double>> test_explore_scores;

	MinesweeperSignal();

	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* MINESWEEPER_SIGNAL_H */