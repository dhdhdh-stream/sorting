#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class Action;
class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> world_states;
	/**
	 * - simply starting at world_states[0]
	 */

	int num_states;

	WorldModel();
	~WorldModel();

	void init();

	bool activate(std::vector<double>& obs_sequence,
				  std::vector<Action*>& action_sequence,
				  std::vector<std::vector<int>>& action_state_sequence,
				  std::vector<std::vector<double>>& state_vals_sequence,
				  double target_val);

	void measure_activate(std::vector<double>& obs_sequence,
						  std::vector<Action*>& action_sequence,
						  std::vector<std::vector<int>>& action_state_sequence,
						  std::vector<std::vector<double>>& state_vals_sequence,
						  double target_val);

	void generate();

	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */