#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	std::vector<double> starting_likelihood;

	WorldModel();
	~WorldModel();
};

#endif /* WORLD_MODEL_H */