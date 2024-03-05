#include "branch_experiment.h"

#include "globals.h"
#include "pass_through_experiment.h"

using namespace std;

bool BranchExperiment::activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper) {
	bool is_selected = false;
	BranchExperimentHistory* history = NULL;
	if (run_helper.throw_id == -1
			|| run_helper.throw_id == this->throw_id) {
		if (run_helper.experiment_histories.size() > 0
				&& run_helper.experiment_histories.back()->experiment == this) {
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
				history = (BranchExperimentHistory*)run_helper.experiment_histories.back();
				is_selected = true;
			}
		} else {
			if (this->parent_experiment == NULL) {
				if (run_helper.experiment_histories.size() == 0) {
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
								history = new BranchExperimentHistory(this);
								run_helper.experiment_histories.push_back(history);
								is_selected = true;
							}

							run_helper.experiments_seen_order.push_back(this);
						}
					}
				}
			} else {
				switch (this->root_experiment->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
					if (run_helper.experiment_histories.size() > 0
							&& run_helper.experiment_histories.back()->experiment == this->parent_experiment) {
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
							PassThroughExperimentHistory* parent_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
							bool has_seen = false;
							for (int e_index = 0; e_index < (int)parent_pass_through_experiment_history->experiments_seen_order.size(); e_index++) {
								if (parent_pass_through_experiment_history->experiments_seen_order[e_index] == this) {
									has_seen = true;
									break;
								}
							}
							if (!has_seen) {
								double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
								uniform_real_distribution<double> distribution(0.0, 1.0);
								if (distribution(generator) < selected_probability) {
									history = new BranchExperimentHistory(this);
									run_helper.experiment_histories.push_back(history);
									is_selected = true;
								}

								parent_pass_through_experiment_history->experiments_seen_order.push_back(this);
							}
						}
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING:
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING:
					// do nothing either way
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
					if (run_helper.experiment_histories.size() == 1
							&& run_helper.experiment_histories[0]->experiment == this->root_experiment) {
						if (this->root_experiment->verify_experiments.back() == this) {
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
								/**
								 * - don't append to run_helper.experiment_histories
								 *   - let backprop occur on root
								 */
								is_selected = true;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (is_selected) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(context,
									run_helper,
									history);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 history);
			break;
		case BRANCH_EXPERIMENT_STATE_RETRAIN_EXISTING:
			retrain_existing_activate(curr_node,
									  problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper,
									  history);
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
							 run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_VERIFY_1ST:
		case BRANCH_EXPERIMENT_STATE_VERIFY_2ND:
			verify_activate(curr_node,
							problem,
							context,
							exit_depth,
							exit_node,
							run_helper);
			break;
		#if defined(MDEBUG) && MDEBUG
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper);
			break;
		#endif /* MDEBUG */
		case BRANCH_EXPERIMENT_STATE_ROOT_VERIFY:
			root_verify_activate(curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper);
			break;
		}

		return true;
	} else {
		return false;
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	BranchExperimentHistory* branch_experiment_history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								branch_experiment_history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 branch_experiment_history);
		break;
	case BRANCH_EXPERIMENT_STATE_RETRAIN_EXISTING:
		retrain_existing_backprop(target_val,
								  branch_experiment_history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   branch_experiment_history);
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
