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

	double get_observation();
	void perform_action(Action action);
	double score_result();

	void print();
};

#endif /* PROBLEM_H */