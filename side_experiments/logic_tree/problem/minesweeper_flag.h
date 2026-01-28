#ifndef MINESWEEPER_FLAG_H
#define MINESWEEPER_FLAG_H

#include <vector>

#include "abstract_problem.h"

class MinesweeperFlag : public AbstractProblem {
public:
	void get_train_instance(std::vector<double>& obs,
							double& target_val);
	void get_test_instance(std::vector<double>& obs,
						   double& target_val);
};

#endif /* MINESWEEPER_FLAG_H */