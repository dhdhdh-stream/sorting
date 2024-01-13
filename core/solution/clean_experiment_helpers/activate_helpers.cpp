#include "clean_experiment.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void CleanExperiment::activate(AbstractNode*& curr_node,
							   vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper) {
	if (run_helper.experiment_history == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			bool select = false;
			set<AbstractExperiment*>::iterator it = run_helper.experiments_seen.find(this);
			if (it == run_helper.experiments_seen.end()) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}

				run_helper.experiments_seen_order.push_back(this);
				run_helper.experiments_seen.insert(this);
			}
			if (select) {
				run_helper.experiment_history = new CleanExperimentOverallHistory(this);

				switch (this->state) {
				case CLEAN_EXPERIMENT_STATE_MEASURE_NEW:
					if (this->state_iter == 0) {
						measure_new_initial_activate(curr_node,
													 context,
													 exit_depth,
													 exit_node);
					} else {
						measure_new_activate(curr_node,
											 exit_depth,
											 exit_node);
					}
					break;
				case CLEAN_EXPERIMENT_STATE_VERIFY_1ST:
				case CLEAN_EXPERIMENT_STATE_VERIFY_2ND:
					verify_new_activate(curr_node,
										exit_depth,
										exit_node);
					break;
				}
			}
		}
	} else if (run_helper.experiment_history->experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			switch (this->state) {
			case CLEAN_EXPERIMENT_STATE_MEASURE_NEW:
				measure_new_activate(curr_node,
									 exit_depth,
									 exit_node);
				break;
			case CLEAN_EXPERIMENT_STATE_VERIFY_1ST:
			case CLEAN_EXPERIMENT_STATE_VERIFY_2ND:
				verify_new_activate(curr_node,
									exit_depth,
									exit_node);
				break;
			}
		}
	}
}

void CleanExperiment::backprop(double target_val,
							   RunHelper& run_helper,
							   CleanExperimentOverallHistory* history) {
	switch (this->state) {
	case CLEAN_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case CLEAN_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_backprop(target_val);
		break;
	case CLEAN_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case CLEAN_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case CLEAN_EXPERIMENT_STATE_VERIFY_1ST:
	case CLEAN_EXPERIMENT_STATE_VERIFY_2ND:
		verify_new_backprop(target_val);
		break;
	}

	delete history;
}
