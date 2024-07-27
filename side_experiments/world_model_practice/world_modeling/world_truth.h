/**
 * - 0 means un-enterable
 */

#ifndef WORLD_TRUTH_H
#define WORLD_TRUTH_H

#include <vector>

/**
 * - including boundaries
 */
const int WORLD_X = 21;
const int WORLD_Y = 21;

class WorldTruth {
public:
	std::vector<std::vector<int>> vals;
	int curr_x;
	int curr_y;

	WorldTruth();

	void print();
};

#endif /* WORLD_TRUTH_H */