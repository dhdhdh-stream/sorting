#ifndef WORLD_H
#define WORLD_H

#include <vector>

const int WORLD_WIDTH = 4;
const int WORLD_HEIGHT = 4;

const int ACTION_UP = 0;
const int ACTION_RIGHT = 1;
const int ACTION_DOWN = 2;
const int ACTION_LEFT = 3;

class World {
public:
	std::vector<std::vector<int>> world;
	int curr_x;
	int curr_y;

	World();

	int get_observation();
	void perform_action(int action);

	void print();
};

#endif /* WORLD_H */