#include "world_model.h"

#include "minesweeper.h"
#include "solution_wrapper.h"

using namespace std;

const int WIDTH = 9;
const int HEIGHT = 9;

WorldModel::WorldModel() {
	this->world = vector<vector<int>>(WIDTH, vector<int>(HEIGHT, 0));
	this->revealed = vector<vector<bool>>(WIDTH, vector<bool>(HEIGHT, false));
}

void update_world_model(SolutionWrapper* wrapper) {
	Minesweeper* minesweeper = (Minesweeper*)wrapper->problem;

	for (int x_index = -2; x_index <= 2; x_index++) {
		for (int y_index = -2; y_index <= 2; y_index++) {
			int x_coord = minesweeper->current_x + x_index;
			int y_coord = minesweeper->current_y + y_index;
			if (x_coord >= 0
					&& x_coord < WIDTH
					&& y_coord >= 0
					&& y_coord < HEIGHT) {
				wrapper->world_model->world[x_coord][y_coord] = minesweeper->get_observation_helper(x_coord, y_coord);
				wrapper->world_model->revealed[x_coord][y_coord] = true;
			}
		}
	}
}
