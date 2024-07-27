/**
 * TODO:
 * - think about how to merge states
 *   - especially if wormhole
 */

#ifndef WORLD_STATE_H
#define WORLD_STATE_H

class WorldState {
public:
	int id;

	int state;

	double estimated_x;
	double estimated_y;

	/**
	 * - don't include self
	 *   - instead, add obstacle
	 */
	std::map<int, std::pair<double,double>> evidence;

	WorldState();
	WorldState(WorldState* original);
};

#endif /* WORLD_STATE_H */