/**
 * - both creating new state and distance to existing state decrease probability
 */

#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	int curr_state_index;

	WorldModel();
	WorldModel(WorldModel* original);
	~WorldModel();

	void next_probabilities(int action,
							int new_obs,
							std::vector<WorldModel*>& next,
							std::vector<double>& next_probabilities);

	void travel(int action,
				int next_state_index);
	void travel_new(int action,
					int new_state,
					double predicted_x,
					double predicted_y);

	void rebalance();
	void rebalance_helper(int state_index,
						  std::vector<bool>& moved_nodes);

	bool equals(WorldModel* rhs);
};

#endif /* WORLD_MODEL_H */