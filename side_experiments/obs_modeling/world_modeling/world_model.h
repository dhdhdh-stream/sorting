#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

class WorldModel {
public:
	std::vector<WorldState*> world_states;
	/**
	 * - simply starting at world_states[0]
	 */

	int num_states;

};

#endif /* WORLD_MODEL_H */