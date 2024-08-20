#ifndef WORLD_TRUTH_H
#define WORLD_TRUTH_H

#include <vector>

class WorldTruth {
public:
	std::vector<std::vector<int>> vals;
	int curr_x;
	int curr_y;

	std::vector<int> action_queue;

	WorldTruth();

	void move(int action);
};

#endif /* WORLD_TRUTH_H */