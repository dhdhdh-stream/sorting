#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <fstream>
#include <vector>

class WorldState {
public:
	double average_val;

	std::vector<std::vector<double>> transitions;

	WorldState();
	WorldState(WorldState* original);

	void split_state(int state_index);

	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_STATE_H */