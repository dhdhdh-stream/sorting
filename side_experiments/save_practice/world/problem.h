#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

const int ACTION_UP = 0;
const int ACTION_RIGHT = 1;
const int ACTION_DOWN = 2;
const int ACTION_LEFT = 3;

class Problem {
public:
	int current_x;
	int current_y;

	Problem();

	std::vector<double> get_observations();
	bool perform_action(int action);
};

void get_existing_solution(std::vector<int>& actions);

#endif /* PROBLEM_H */