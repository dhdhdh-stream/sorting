#ifndef PREDICTED_WORLD_H
#define PREDICTED_WORLD_H

#include <vector>

class PredictedWorld {
public:
	std::vector<std::vector<std::vector<double>>> val_weights;
	std::vector<std::vector<std::vector<double>>> val_counts;

	std::vector<std::vector<double>> loc;

	PredictedWorld(int starting_obs);

	void print();
};

#endif /* PREDICTED_WORLD_H */