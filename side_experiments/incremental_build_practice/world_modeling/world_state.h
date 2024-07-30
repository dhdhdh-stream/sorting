#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <vector>

class WorldState {
public:
	double average_val;

	std::vector<std::vector<double>> transitions;

	WorldState() {};
};

#endif /* WORLD_STATE_H */