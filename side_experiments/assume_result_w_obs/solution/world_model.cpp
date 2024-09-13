#include "world_model.h"

using namespace std;

WorldModel::WorldModel(vector<int>& location,
					   double val) {
	if (location.size() == 0) {
		this->val = val;
	} else {
		this->min_index = location[0];
		this->max_index = location[0];
		vector<int> next_location(location.begin() + 1, location.end());
		WorldModel* inner_world_model = new WorldModel(next_location,
													   val);
		this->vals.push_back(inner_world_model);
	}
}

WorldModel::WorldModel(WorldModel* original) {
	this->val = original->val;

	this->min_index = original->min_index;
	this->max_index = original->max_index;
	this->vals = vector<WorldModel*>(original->vals.size(), NULL);
	for (int v_index = 0; v_index < (int)original->vals.size(); v_index++) {
		if (original->vals[v_index] != NULL) {
			this->vals[v_index] = new WorldModel(original->vals[v_index]);
		}
	}
}

WorldModel::~WorldModel() {
	for (int v_index = 0; v_index < (int)this->vals.size(); v_index++) {
		if (this->vals[v_index] != NULL) {
			delete this->vals[v_index];
		}
	}
}

void WorldModel::update(vector<int>& location,
						double val) {
	if (location.size() == 0) {
		this->val = val;
	} else {
		if (location[0] < this->min_index) {
			int diff = this->min_index - location[0];
			this->vals.insert(this->vals.begin(), diff, NULL);
			this->min_index = location[0];
		}
		if (location[0] > this->max_index) {
			int diff = location[0] - this->max_index;
			this->vals.insert(this->vals.end(), diff, NULL);
			this->max_index = location[0];
		}

		vector<int> next_location(location.begin() + 1, location.end());
		if (this->vals[location[0] - this->min_index] == NULL) {
			WorldModel* inner_world_model = new WorldModel(next_location,
														   val);
			this->vals[location[0] - this->min_index] = inner_world_model;
		} else {
			return this->vals[location[0] - this->min_index]->update(
				next_location,
				val);
		}
	}
}

void WorldModel::get_val(vector<int>& location,
						 bool& is_init,
						 double& val) {
	if (location.size() == 0) {
		is_init = true;
		val = this->val;
	} else {
		if (location[0] < this->min_index
				|| location[0] > this->max_index
				|| this->vals[location[0] - this->min_index] == NULL) {
			is_init = false;
		} else {
			vector<int> next_location(location.begin() + 1, location.end());
			this->vals[location[0] - this->min_index]->get_val(
				next_location,
				is_init,
				val);
		}
	}
}

void WorldModel::gather_locations(vector<int>& curr_location,
								  vector<vector<int>>& locations) {
	if (this->vals.size() == 0) {
		locations.push_back(curr_location);
	} else {
		for (int v_index = 0; v_index < (int)this->vals.size(); v_index++) {
			if (this->vals[v_index] != NULL) {
				vector<int> next_location = curr_location;
				next_location.push_back(v_index);
				this->vals[v_index]->gather_locations(next_location,
													  locations);
			}
		}
	}
}
