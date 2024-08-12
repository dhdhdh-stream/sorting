#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	std::vector<double> starting_likelihood;

	WorldModel();
	WorldModel(WorldModel* original);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void split_state(int state_index);

	/**
	 * - new_state_indexes.size() == actions.size() + 1
	 */
	void add_path(int original_state_index,
				  int starting_state_index,
				  std::vector<int>& new_state_indexes,
				  int ending_state_index,
				  int starting_action,
				  std::vector<int>& actions,
				  int ending_action);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */