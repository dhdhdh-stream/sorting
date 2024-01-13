#include "clean_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "solution_helpers.h"
#include "solution.h"

using namespace std;

void CleanExperiment::measure_new_initial_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node) {
	{
		// exit
		vector<pair<int,AbstractNode*>> possible_exits;
		gather_possible_exits(possible_exits,
							  context,
							  this->scope_context,
							  this->node_context);

		// cout << "Clean" << endl;
		// cout << "explore" << endl;
		// cout << "this->scope_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->scope_context[c_index] << endl;
		// }
		// cout << "this->node_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->node_context[c_index] << endl;
		// }
		// cout << "possible_exits.size(): " << possible_exits.size() << endl;
		// for (int e_index = 0; e_index < (int)possible_exits.size(); e_index++) {
		// 	int node_id;
		// 	if (possible_exits[e_index].second == NULL) {
		// 		node_id = -1;
		// 	} else {
		// 		node_id = possible_exits[e_index].second->id;
		// 	}
		// 	cout << e_index << ": " << possible_exits[e_index].first << " " << node_id << endl;
		// }
		// cout << endl;

		if (possible_exits.size() == 1) {
			this->state = CLEAN_EXPERIMENT_STATE_FAIL;
			return;
		}

		geometric_distribution<int> distribution(0.5);
		int random_index = 1 + distribution(generator);
		if (random_index > (int)possible_exits.size()-1) {
			random_index = (int)possible_exits.size()-1;
		}
		this->clean_exit_depth = possible_exits[random_index].first;
		this->clean_exit_node = possible_exits[random_index].second;
	}

	{
		if (this->clean_exit_depth == 0) {
			curr_node = this->clean_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->clean_exit_depth-1;
			exit_node = this->clean_exit_node;
		}
	}
}

void CleanExperiment::measure_new_activate(AbstractNode*& curr_node,
										   int& exit_depth,
										   AbstractNode*& exit_node) {
	if (this->clean_exit_depth == 0) {
		curr_node = this->clean_exit_node;
	} else {
		curr_node = NULL;

		exit_depth = this->clean_exit_depth-1;
		exit_node = this->clean_exit_node;
	}
}

void CleanExperiment::measure_new_backprop(double target_val) {
	this->new_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->new_score /= solution->curr_num_datapoints;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->new_score >= this->existing_score) {
		#endif /* MDEBUG */
			this->existing_score = 0.0;
			this->new_score = 0.0;

			this->state = CLEAN_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		} else {
			this->state = CLEAN_EXPERIMENT_STATE_FAIL;
		}
	}
}
