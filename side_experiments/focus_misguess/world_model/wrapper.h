#ifndef WRAPPER_H
#define WRAPPER_H

#include <vector>

class ProblemType;
class WorldModel;

class Wrapper {
public:
	int num_obs;
	int num_actions;

	WorldModel* world_model;

	std::vector<std::vector<std::vector<double>>> sample_obs;
	std::vector<std::vector<int>> sample_actions;
	std::vector<double> sample_target_vals;
	std::vector<double> sample_predicted_scores;
	std::vector<double> sample_predicted_misguesses;

	Wrapper(ProblemType* problem_type);
	~Wrapper();
};

#endif /* WRAPPER_H */