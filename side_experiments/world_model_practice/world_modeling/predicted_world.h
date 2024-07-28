#ifndef PREDICTED_WORLD_H
#define PREDICTED_WORLD_H

#include <vector>

class WorldModel;

class PredictedWorld {
public:
	std::vector<WorldModel*> possible_models;
	std::vector<double> possible_weights;

	PredictedWorld();
	~PredictedWorld();
};

#endif /* PREDICTED_WORLD_H */