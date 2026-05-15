#ifndef WORLD_MODEL_WRAPPER_H
#define WORLD_MODEL_WRAPPER_H

#include <fstream>
#include <vector>

class ProblemType;
class WorldModel;

class WorldModelWrapper {
public:
	int num_obs;
	int num_actions;

	WorldModel* world_model;

	std::vector<std::vector<std::vector<double>>> sample_obs;
	std::vector<std::vector<int>> sample_actions;
	std::vector<double> sample_target_vals;

	WorldModelWrapper(ProblemType* problem_type);
	WorldModelWrapper(std::string path,
					  std::string name);
	~WorldModelWrapper();

	void save(std::string path,
			  std::string name);
};

#endif /* WORLD_MODEL_WRAPPER_H */