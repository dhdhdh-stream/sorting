#include "world_helpers.h"

#include "globals.h"
#include "minesweeper.h"

using namespace std;

const int WIDTH = 9;
const int HEIGHT = 9;

void get_instance(std::vector<double>& obs,
				  double& result) {
	Minesweeper problem;

	uniform_int_distribution<int> border_distribution(0, 3);
	if (border_distribution(generator) == 0) {
		vector<pair<int,int>> possibilities;
		for (int x_index = 0; x_index < WIDTH; x_index++) {
			for (int y_index = 0; y_index < HEIGHT; y_index++) {
				if (!problem.revealed[x_index][y_index]) {
					if (x_index < WIDTH-1
							&& problem.revealed[x_index+1][y_index]) {
						possibilities.push_back({x_index, y_index});
					} else if (x_index > 0
							&& problem.revealed[x_index-1][y_index]) {
						possibilities.push_back({x_index, y_index});
					} else if (y_index < HEIGHT-1
							&& problem.revealed[x_index][y_index+1]) {
						possibilities.push_back({x_index, y_index});
					} else if (y_index > 0
							&& problem.revealed[x_index][y_index-1]) {
						possibilities.push_back({x_index, y_index});
					}
				}
			}
		}

		uniform_int_distribution<int> distribution(0, possibilities.size()-1);
		int index = distribution(generator);
		problem.current_x = possibilities[index].first;
		problem.current_y = possibilities[index].second;
	} else {
		uniform_int_distribution<int> x_distribution(0, WIDTH-1);
		problem.current_x = x_distribution(generator);
		uniform_int_distribution<int> y_distribution(0, HEIGHT-1);
		problem.current_y = y_distribution(generator);
	}

	obs = problem.get_observations();
	if (problem.revealed[problem.current_x][problem.current_y]) {
		result = 0.0;
	} else {
		if (problem.world[problem.current_x][problem.current_y] == -1) {
			result = 0.2;
		} else {
			result = -1.0;
		}
	}
}
