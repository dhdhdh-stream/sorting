#include "simple_problem.h"

using namespace std;

const int WORLD_SIZE = 5;

SimpleProblem::SimpleProblem() {
	uniform_int_distribution<int> target_distribution(0, WORLD_SIZE-1);
	this->target_index = target_distribution(generator);

	this->current_index = 0;

	this->hit_target = false;
	this->hit_non_target = false;
}

vector<double> SimpleProblem::get_observations() {
	vector<double> obs;

	if (this->current_index < 0
			|| this->current_index > WORLD_SIZE-1) {
		obs.push_back(-20.0);
	} else if (this->current_index == this->target_index) {
		if (this->hit_target) {
			obs.push_back(10.0);
		} else if (this->hit_non_target) {
			obs.push_back(-10.0);
		} else {
			obs.push_back(1.0);
		}
	} else {
		obs.push_back(-1.0);
	}

	return obs;
}

void SimpleProblem::perform_action(Action action) {
	switch (action.move) {
	case SIMPLE_ACTION_LEFT:
		this->current_index--;
		if (this->current_index < -1) {
			this->current_index = -1;
		}
		break;
	case SIMPLE_ACTION_RIGHT:
		this->current_index++;
		if (this->current_index > WORLD_SIZE) {
			this->current_index = WORLD_SIZE;
		}
		break;
	case SIMPLE_ACTION_CLICK:
		if (!this->hit_target && !this->hit_non_target) {
			if (this->current_index >= 0 && this->current_index < WORLD_SIZE) {
				if (this->current_index == this->target_index) {
					this->hit_target = true;
				} else {
					this->hit_non_target = true;
				}
			}
		}
		break;
	}
}

double SimpleProblem::score_result() {
	if (this->hit_target) {
		return 1.0;
	} else if (this->hit_non_target) {
		return -1.0;
	} else {
		return 0.0;
	}
}

#if defined(MDEBUG) && MDEBUG
Problem* SimpleProblem::copy_and_reset() {
	SimpleProblem* new_problem = new SimpleProblem();

	new_problem->target_index = this->target_index;

	return new_problem;
}

Problem* SimpleProblem::copy_snapshot() {
	SimpleProblem* new_problem = new SimpleProblem();

	new_problem->target_index = this->target_index;
	new_problem->current_index = this->current_index;
	new_problem->hit_target = this->hit_target;
	new_problem->hit_non_target = this->hit_non_target;

	return new_problem;
}
#endif /* MDEBUG */

void SimpleProblem::print() {
	cout << "this->target_index: " << this->target_index << endl;
	cout << "this->current_index: " << this->current_index << endl;
	cout << "this->hit_target: " << this->hit_target << endl;
	cout << "this->hit_non_target: " << this->hit_non_target << endl;
}

Problem* TypeSimpleProblem::get_problem() {
	return new SimpleProblem();
}

int TypeSimpleProblem::num_obs() {
	return 1;
}

int TypeSimpleProblem::num_possible_actions() {
	return 3;
}
