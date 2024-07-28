#include "world_model.h"

#include <cmath>

#include "constants.h"
#include "world_state.h"

using namespace std;

const double NEW_PENALTY = 2.0;

/**
 * - want S-distribution
 */
double adjusted_sigmoid(double x) {
	return exp(3-1.5*x) / (1 + exp(3-1.5*x));
}

WorldModel::WorldModel() {
	WorldState* starting_state = new WorldState();
	starting_state->id = (int)this->states.size();
	this->states.push_back(starting_state);
	starting_state->state = 1;
	starting_state->estimated_x = 0.0;
	starting_state->estimated_y = 0.0;

	this->curr_state_index = 0;
}

WorldModel::WorldModel(WorldModel* original) {
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		this->states.push_back(new WorldState(original->states[s_index]));
	}

	this->curr_state_index = original->curr_state_index;
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		delete this->states[s_index];
	}
}

void WorldModel::next_probabilities(int action,
									int new_obs,
									vector<WorldModel*>& next,
									vector<double>& next_probabilities) {
	double predicted_x = this->states[this->curr_state_index]->estimated_x;
	double predicted_y = this->states[this->curr_state_index]->estimated_y;
	switch (action) {
	case ACTION_LEFT:
		predicted_x -= 1.0;
		break;
	case ACTION_UP:
		predicted_y += 1.0;
		break;
	case ACTION_RIGHT:
		predicted_x += 1.0;
		break;
	case ACTION_DOWN:
		predicted_y -= 1.0;
		break;
	}

	bool has_close_match = false;
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		if (this->states[s_index]->state == new_obs) {
			if (s_index == this->curr_state_index) {
				bool o_has_close_match = false;
				for (int is_index = 0; is_index < (int)this->states.size(); is_index++) {
					if (this->states[is_index]->state == 0) {
						double distance = sqrt(
							(predicted_x - this->states[is_index]->estimated_x) * (predicted_x - this->states[is_index]->estimated_x)
							+ (predicted_y - this->states[is_index]->estimated_y) * (predicted_y - this->states[is_index]->estimated_y));

						if (distance < 5.0) {
							WorldModel* possibility = new WorldModel(this);
							possibility->travel(action,
												is_index);
							next.push_back(possibility);
							next_probabilities.push_back(adjusted_sigmoid(distance));
						}

						if (distance <= 1.0) {
							o_has_close_match = true;
						}
					}
				}

				if (!o_has_close_match) {
					WorldModel* possibility = new WorldModel(this);
					possibility->travel_new(action,
											0,
											predicted_x,
											predicted_y);
					next.push_back(possibility);
					next_probabilities.push_back(adjusted_sigmoid(NEW_PENALTY));
				}
			} else {
				double distance = sqrt(
					(predicted_x - this->states[s_index]->estimated_x) * (predicted_x - this->states[s_index]->estimated_x)
					+ (predicted_y - this->states[s_index]->estimated_y) * (predicted_y - this->states[s_index]->estimated_y));

				if (distance < 5.0) {
					WorldModel* possibility = new WorldModel(this);
					possibility->travel(action,
										s_index);
					next.push_back(possibility);
					next_probabilities.push_back(adjusted_sigmoid(distance));
				}

				if (distance <= 1.0) {
					has_close_match = true;
				}
			}
		}
	}

	if (!has_close_match) {
		WorldModel* possibility = new WorldModel(this);
		possibility->travel_new(action,
								new_obs,
								predicted_x,
								predicted_y);
		next.push_back(possibility);
		next_probabilities.push_back(adjusted_sigmoid(NEW_PENALTY));
	}
}

void WorldModel::travel(int action,
						int next_state_index) {
	map<int, pair<double,double>>::iterator curr_it = this->states[
		this->curr_state_index]->evidence.find(next_state_index);
	if (curr_it == this->states[this->curr_state_index]->evidence.end()) {
		curr_it = this->states[this->curr_state_index]->evidence.insert({next_state_index, {0.0, 0.0}}).first;
	}
	
	map<int, pair<double,double>>::iterator next_it = this->states[
		next_state_index]->evidence.find(this->curr_state_index);
	if (next_it == this->states[next_state_index]->evidence.end()) {
		next_it = this->states[next_state_index]->evidence.insert({this->curr_state_index, {0.0, 0.0}}).first;
	}

	switch (action) {
	case ACTION_LEFT:
		curr_it->second.first -= 1.0;
		next_it->second.first += 1.0;
		break;
	case ACTION_UP:
		curr_it->second.second += 1.0;
		next_it->second.second -= 1.0;
		break;
	case ACTION_RIGHT:
		curr_it->second.first += 1.0;
		next_it->second.first -= 1.0;
		break;
	case ACTION_DOWN:
		curr_it->second.second -= 1.0;
		next_it->second.second += 1.0;
		break;
	}

	this->curr_state_index = next_state_index;
}

void WorldModel::travel_new(int action,
							int new_state,
							double predicted_x,
							double predicted_y) {
	WorldState* world_state = new WorldState();
	world_state->id = (int)this->states.size();
	this->states.push_back(world_state);
	world_state->state = new_state;
	world_state->estimated_x = predicted_x;
	world_state->estimated_y = predicted_y;

	switch (action) {
	case ACTION_LEFT:
		this->states[this->curr_state_index]->evidence[world_state->id] = {-1.0, 0.0};
		world_state->evidence[this->curr_state_index] = {1.0, 0.0};
		break;
	case ACTION_UP:
		this->states[this->curr_state_index]->evidence[world_state->id] = {0.0, 1.0};
		world_state->evidence[this->curr_state_index] = {0.0, -1.0};
		break;
	case ACTION_RIGHT:
		this->states[this->curr_state_index]->evidence[world_state->id] = {1.0, 0.0};
		world_state->evidence[this->curr_state_index] = {-1.0, 0.0};
		break;
	case ACTION_DOWN:
		this->states[this->curr_state_index]->evidence[world_state->id] = {0.0, -1.0};
		world_state->evidence[this->curr_state_index] = {0.0, 1.0};
		break;
	}

	this->curr_state_index = world_state->id;
}

void WorldModel::rebalance_helper(int state_index,
								  vector<bool>& moved_nodes) {
	int local_x = this->states[state_index]->estimated_x;
	int local_y = this->states[state_index]->estimated_y;

	for (map<int, pair<double,double>>::iterator it = this->states[state_index]->evidence.begin();
			it != this->states[state_index]->evidence.end(); it++) {
		if (!moved_nodes[it->first]) {
			double new_x = this->states[it->first]->estimated_x;
			double new_y = this->states[it->first]->estimated_y;
			if (it->second.first > 0.0) {
				if (new_x > local_x + 2.0) {
					new_x = local_x + 2.0;
				}
				if (new_x < local_x) {
					new_x = local_x;
				}
			}
			if (it->second.first < 0.0) {
				if (new_x < local_x - 2.0) {
					new_x = local_x - 2.0;
				}
				if (new_x > local_x) {
					new_x = local_x;
				}
			}
			if (it->second.second > 0.0) {
				if (new_y > local_y + 2.0) {
					new_y = local_y + 2.0;
				}
				if (new_y < local_y) {
					new_y = local_y;
				}
			}
			if (it->second.second < 0.0) {
				if (new_y < local_y - 2.0) {
					new_y = local_y - 2.0;
				}
				if (new_y > local_y) {
					new_y = local_y;
				}
			}

			if (new_x == local_x && new_y == local_y) {
				if (abs(it->second.first) > abs(it->second.second)) {
					if (it->second.first > 0.0) {
						new_x += 1.0;
					} else {
						new_x -= 1.0;
					}
				} else {
					if (it->second.second > 0.0) {
						new_y += 1.0;
					} else {
						new_y -= 1.0;
					}
				}
			}

			if (new_x != this->states[it->first]->estimated_x
					|| new_y != this->states[it->first]->estimated_y) {
				moved_nodes[it->first] = true;

				this->states[it->first]->estimated_x = new_x;
				this->states[it->first]->estimated_y = new_y;

				rebalance_helper(it->first,
								 moved_nodes);
			}
		}
	}
}

void WorldModel::rebalance() {
	while (true) {
		vector<bool> moved_nodes(this->states.size(), false);

		for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
			rebalance_helper(s_index,
							 moved_nodes);
		}

		bool has_moved = false;
		for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
			if (moved_nodes[s_index]) {
				has_moved = true;
				break;
			}
		}

		if (!has_moved) {
			break;
		}
	}
}

bool WorldModel::equals(WorldModel* rhs) {
	if (this->states.size() != rhs->states.size()) {
		return false;
	}

	if (this->curr_state_index != rhs->curr_state_index) {
		return false;
	}

	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		if (this->states[s_index]->state != rhs->states[s_index]->state) {
			return false;
		}

		if (this->states[s_index]->estimated_x != rhs->states[s_index]->estimated_x
				|| this->states[s_index]->estimated_y != rhs->states[s_index]->estimated_y) {
			return false;
		}
	}

	return true;
}
