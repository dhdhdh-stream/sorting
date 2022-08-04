#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "action.h"
#include "action_dictionary.h"

class Problem {
public:
	std::vector<double> initial_world;
	
	int current_pointer;
	std::vector<double> current_world;

	Problem(std::vector<double>& observations);
	~Problem();

	void perform_action(Action action,
						std::vector<double>& observations,
						ActionDictionary* action_dictionary);
	double score_result();

	void print();

private:
	double get_observation();
};

#endif /* PROBLEM_H */