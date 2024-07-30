#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <vector>

class WorldState {
public:
	double average_val;

	std::vector<std::vector<double>> transitions;

	WorldState();
	WorldState(WorldState* original);

	void split_state(int state_index);
};

#endif /* WORLD_STATE_H */