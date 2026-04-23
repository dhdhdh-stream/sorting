#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class SolutionWrapper;

class WorldModel {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;

	WorldModel();

	void print();
};

void update_world_model(SolutionWrapper* wrapper);

#endif /* WORLD_MODEL_H */