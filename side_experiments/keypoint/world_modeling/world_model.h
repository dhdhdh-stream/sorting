#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class Keypoint;

class WorldModel {
public:
	std::vector<Keypoint*> keypoints;

	std::vector<std::vector<int>> path_actions;
	std::vector<std::vector<double>> path_obs;
	std::vector<int> path_start_indexes;
	std::vector<int> path_end_indexes;
	std::vector<double> path_success_likelihoods;

	WorldModel();
	~WorldModel();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* WORLD_MODEL_H */