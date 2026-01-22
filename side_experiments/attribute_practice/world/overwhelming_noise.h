#ifndef OVERWHELMING_NOISE_H
#define OVERWHELMING_NOISE_H

#include <vector>

#include "problem.h"

class OverwhelmingNoise : public Problem {
public:
	unsigned long starting_seed;
	unsigned long current_seed;

	std::vector<double> curr_obs;

	int score;

	int noise;

	OverwhelmingNoise();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeOverwhelmingNoise : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* OVERWHELMING_NOISE_H */