#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "action.h"

class Problem {
public:
	std::vector<double> initial_world;
	
	int current_pointer;
	std::vector<double> current_world;

	Problem(std::vector<double>* observations);
	~Problem();

	void perform_action(Action action,
						std::vector<double>* observations,
						bool save_for_display,
						std::vector<Action>* raw_actions);
	double score_result();

	void print();

private:
	double get_observation();
};

#endif /* PROBLEM_H */