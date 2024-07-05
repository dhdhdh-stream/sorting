#include "predicted_world.h"

#include <iostream>

#include "constants.h"

using namespace std;

PredictedWorld::PredictedWorld(int starting_obs) {
	this->val_weights = vector<vector<vector<double>>>(WORLD_X);
	this->val_counts = vector<vector<vector<double>>>(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->val_weights[x_index] = vector<vector<double>>(WORLD_Y);
		this->val_counts[x_index] = vector<vector<double>>(WORLD_Y);
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			this->val_weights[x_index][y_index] = vector<double>(3, 0.0);
			this->val_counts[x_index][y_index] = vector<double>(3, 1.0);
		}
	}

	// this->val_weights[3][3][starting_obs] = 10.0;
	this->val_weights[3][3][starting_obs] = 500.0;
	// this->val_counts[3][3][starting_obs] = 10.0;
	this->val_counts[3][3][starting_obs] = 500.0;

	this->loc = vector<vector<double>>(WORLD_X);
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		this->loc[x_index] = vector<double>(WORLD_Y, 0.0);
	}
	this->loc[3][3] = 1.0;
}

void PredictedWorld::print() {
	// temp
	cout << "0:" << endl;
	for (int x_index = 1; x_index < WORLD_X-1; x_index++) {
		for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
			// double val = this->val_weights[x_index][y_index][0]
			// 	/ this->val_counts[x_index][y_index][0];
			double val = this->val_counts[x_index][y_index][0];
			cout << val << " ";
		}
		cout << endl;
	}
	cout << "1:" << endl;
	for (int x_index = 1; x_index < WORLD_X-1; x_index++) {
		for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
			// double val = this->val_weights[x_index][y_index][1]
			// 	/ this->val_counts[x_index][y_index][1];
			double val = this->val_counts[x_index][y_index][1];
			cout << val << " ";
		}
		cout << endl;
	}
	cout << "2:" << endl;
	for (int x_index = 1; x_index < WORLD_X-1; x_index++) {
		for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
			// double val = this->val_weights[x_index][y_index][2]
			// 	/ this->val_counts[x_index][y_index][2];
			double val = this->val_counts[x_index][y_index][2];
			cout << val << " ";
		}
		cout << endl;
	}

	for (int x_index = 1; x_index < WORLD_X-1; x_index++) {
		for (int y_index = 1; y_index < WORLD_Y-1; y_index++) {
			double max_obs = 0.0;
			double max_weight = 0.0;
			for (int v_index = 0; v_index < (int)this->val_weights[x_index][y_index].size(); v_index++) {
				double curr_weight = this->val_weights[x_index][y_index][v_index]
					/ this->val_counts[x_index][y_index][v_index];
				if (curr_weight > max_weight) {
					max_obs = v_index;
					max_weight = curr_weight;
				}
			}
			cout << max_obs << " ";
		}
		cout << endl;
	}

	int max_x;
	int max_y;
	double max_weight = 0.0;
	for (int x_index = 0; x_index < WORLD_X; x_index++) {
		for (int y_index = 0; y_index < WORLD_Y; y_index++) {
			if (this->loc[x_index][y_index] > max_weight) {
				max_x = x_index;
				max_y = y_index;
				max_weight = this->loc[x_index][y_index];
			}
		}
	}
	cout << "max_x: " << max_x << endl;
	cout << "max_y: " << max_y << endl;
}
