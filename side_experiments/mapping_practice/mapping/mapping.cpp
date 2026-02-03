#include "mapping.h"

#include <iostream>

#include "world.h"

using namespace std;

Mapping::Mapping() {
	this->world = vector<vector<map<int,int>>>(WORLD_WIDTH, vector<map<int,int>>(WORLD_HEIGHT));
}

void Mapping::print() {
	for (int x_index = 0; x_index < WORLD_WIDTH; x_index++) {
		for (int y_index = 0; y_index < WORLD_HEIGHT; y_index++) {
			if (this->world[x_index][y_index].size() == 0) {
				cout << "- ";
			} else {
				int max_obs = 0;
				int max_count = 0;
				for (map<int,int>::iterator it = this->world[x_index][y_index].begin();
						it != this->world[x_index][y_index].end(); it++) {
					if (it->second > max_count) {
						max_obs = it->first;
						max_count = it->second;
					}
				}
				cout << max_obs << " ";
			}
		}
		cout << endl;
	}
	cout << "this->curr_x: " << this->curr_x << endl;
	cout << "this->curr_y: " << this->curr_y << endl;
}
