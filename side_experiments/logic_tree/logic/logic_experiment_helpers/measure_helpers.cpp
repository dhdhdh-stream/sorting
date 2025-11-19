#include "logic_experiment.h"

#include <iostream>

#include "abstract_logic_node.h"
#include "constants.h"
#include "logic_helpers.h"
#include "network.h"

using namespace std;

void LogicExperiment::measure_activate(vector<double>& obs,
									   double target_val) {
	bool is_match;
	switch (this->split_type) {
	case SPLIT_TYPE_GREATER:
		if (obs[this->obs_index] > this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_GREATER_EQUAL:
		if (obs[this->obs_index] >= this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_LESSER:
		if (obs[this->obs_index] < this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_LESSER_EQUAL:
		if (obs[this->obs_index] <= this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_WITHIN:
		if (abs(obs[this->obs_index] - this->split_target) < this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_WITHIN_EQUAL:
		if (abs(obs[this->obs_index] - this->split_target) <= this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_WITHOUT:
		if (abs(obs[this->obs_index] - this->split_target) > this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_WITHOUT_EQUAL:
		if (abs(obs[this->obs_index] - this->split_target) >= this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER:
		if (obs[this->obs_index] - obs[this->rel_obs_index] > this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER_EQUAL:
		if (obs[this->obs_index] - obs[this->rel_obs_index] >= this->split_target) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN:
		if (abs((obs[this->obs_index] - obs[this->rel_obs_index]) - this->split_target) < this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN_EQUAL:
		if (abs((obs[this->obs_index] - obs[this->rel_obs_index]) - this->split_target) <= this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT:
		if (abs((obs[this->obs_index] - obs[this->rel_obs_index]) - this->split_target) > this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT_EQUAL:
		if (abs((obs[this->obs_index] - obs[this->rel_obs_index]) - this->split_target) >= this->split_range) {
			is_match = true;
		} else {
			is_match = false;
		}
		break;
	}
	if (is_match) {
		double existing_predicted = logic_eval_helper(
			this->node_context,
			obs);
		double existing_misguess = (target_val - existing_predicted)
			* (target_val - existing_predicted);

		this->eval_network->activate(obs);
		double new_predicted = this->eval_network->output->acti_vals[0];
		double new_misguess = (target_val - new_predicted)
			* (target_val - new_predicted);

		// // temp
		// for (int x_index = 0; x_index < 5; x_index++) {
		// 	for (int y_index = 0; y_index < 5; y_index++) {
		// 		cout << obs[x_index * 5 + y_index] << " ";
		// 	}
		// 	cout << endl;
		// }
		// cout << "target_val: " << target_val << endl;
		// cout << "existing_predicted: " << existing_predicted << endl;
		// cout << "new_predicted: " << new_predicted << endl;
		// cout << endl;

		this->sum_improvement += (existing_misguess - new_misguess);
		this->count++;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		this->improvement = this->sum_improvement * this->node_context->weight;

		// temp
		if (this->sum_improvement > 0.0) {
			switch (this->split_type) {
			case SPLIT_TYPE_GREATER:
				cout << this->obs_index << " greater than " << this->split_target << endl;
				break;
			case SPLIT_TYPE_GREATER_EQUAL:
				cout << this->obs_index << " greater equal than " << this->split_target << endl;
				break;
			case SPLIT_TYPE_LESSER:
				cout << this->obs_index << " lesser than " << this->split_target << endl;
				break;
			case SPLIT_TYPE_LESSER_EQUAL:
				cout << this->obs_index << " lesser equal than " << this->split_target << endl;
				break;
			case SPLIT_TYPE_WITHIN:
				cout << this->obs_index << " within " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_WITHIN_EQUAL:
				cout << this->obs_index << " within equal " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_WITHOUT:
				cout << this->obs_index << " without " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_WITHOUT_EQUAL:
				cout << this->obs_index << " without equal " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_GREATER:
				cout << this->obs_index << " greater than " << this->rel_obs_index << " by " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_GREATER_EQUAL:
				cout << this->obs_index << " greater equal than " << this->rel_obs_index << " by " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_WITHIN:
				cout << this->obs_index << " diff " << this->rel_obs_index << " within " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_WITHIN_EQUAL:
				cout << this->obs_index << " diff " << this->rel_obs_index << " within equal " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_WITHOUT:
				cout << this->obs_index << " diff " << this->rel_obs_index << " without " << this->split_range << " of " << this->split_target << endl;
				break;
			case SPLIT_TYPE_REL_WITHOUT_EQUAL:
				cout << this->obs_index << " diff " << this->rel_obs_index << " without equal " << this->split_range << " of " << this->split_target << endl;
				break;
			}
			cout << "this->sum_improvement: " << this->sum_improvement << endl;
			cout << "this->node_context->weight: " << this->node_context->weight << endl;
			cout << "this->count: " << this->count << endl;
			cout << "this->improvement: " << this->improvement << endl;
		}

		this->state = LOGIC_EXPERIMENT_STATE_DONE;
	}
}
