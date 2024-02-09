#include "branch_experiment.h"

using namespace std;

void BranchExperiment::activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper,
								AbstractExperimentHistory*& history) {
	bool is_selected = false;
	if (this->parent_pass_through_experiment != NULL) {
		is_selected = true;
	} else if (run_helper.experiment_history == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			bool select = false;
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				if (run_helper.experiments_seen_order[e_index] == this) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
			if (select) {
				run_helper.experiment_history = new BranchExperimentOverallHistory(this);

				is_selected = true;
			}
		}
	} else if (run_helper.experiment_history->experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(context,
									run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   context,
							   exit_depth,
							   exit_node,
							   run_helper,
							   history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 history);
			break;
		case BRANCH_EXPERIMENT_STATE_VERIFY_1ST:
		case BRANCH_EXPERIMENT_STATE_VERIFY_2ND:
			verify_activate(curr_node,
							problem,
							context,
							exit_depth,
							exit_node,
							run_helper,
							history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper,
									history);
			break;
		#endif /* MDEBUG */
		}
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper,
								AbstractExperimentHistory* history) {
	BranchExperimentOverallHistory* overall_history = (BranchExperimentOverallHistory*)history;

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE:
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_VERIFY_1ST:
	case BRANCH_EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(target_val,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
