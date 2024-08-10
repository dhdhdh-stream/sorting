#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <fstream>
#include <vector>

class WorldState {
public:
	double average_val;

	std::vector<std::vector<double>> transitions;
	std::vector<double> unknown_transitions;
	/**
	 * - once at unknown, 80% back to unknown, 20% evenly split between existing
	 */

	WorldState();
	WorldState(WorldState* original);
	WorldState(std::ifstream& input_file);

	void add_state();

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_STATE_H */