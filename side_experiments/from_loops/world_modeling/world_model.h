/**
 * - don't divide between a world and objects
 *   - everything is an object
 *     - like, if on a ship traveling the sea, will be multiple "worlds" to track anyways
 * 
 * - all states/paths added must loop back to self
 *   - as must be repeatable to be added
 *     - and in order to be repeatable, must loop back to self
 */

#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class WorldState;

class WorldModel {
public:
	std::vector<WorldState*> states;

	/**
	 * TODO: remove
	 */
	std::vector<double> starting_likelihood;

	WorldModel();
	~WorldModel();

	void save_for_display(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */