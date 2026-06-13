#ifndef HIT_ALL_H
#define HIT_ALL_H

#include <vector>

#include "problem.h"

class HitAll : public Problem {
public:
	std::vector<int> world;
	int curr_index;

	HitAll();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();
};

class TypeHitAll : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* HIT_ALL_H */