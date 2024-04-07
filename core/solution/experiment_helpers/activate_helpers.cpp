#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

bool Experiment::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper) {
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
		bool is_selected = false;
		ExperimentHistory* history = NULL;
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = run_helper.experiment_histories[match_index];
			is_selected = true;
		} else {
			if (this->parent_experiment == NULL) {
				if (run_helper.experiment_histories.size() == 0) {
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
							history = new ExperimentHistory(this);
							run_helper.experiment_histories.push_back(history);
							is_selected = true;
						}

						run_helper.experiments_seen_order.push_back(this);
					}
				}
			} else {
				switch (this->root_experiment->state) {
				case EXPERIMENT_STATE_EXPERIMENT:
					{
						vector<Experiment*> ancestors;
						Experiment* curr_experiment = this;
						while (true) {
							ancestors.insert(ancestors.begin(), curr_experiment);

							if (curr_experiment->parent_experiment == NULL) {
								break;
							} else {
								curr_experiment = curr_experiment->parent_experiment;
							}
						}

						int ancestor_index = 0;
						for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
							if (run_helper.experiment_histories[e_index]->experiment == ancestors[e_index]) {
								ancestor_index++;
							} else {
								break;
							}
						}

						if (ancestor_index == (int)run_helper.experiment_histories.size()) {
							bool is_continue = true;
							if (run_helper.experiment_histories.size() == 0) {
								bool has_seen = false;
								for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
									if (run_helper.experiments_seen_order[e_index] == ancestors[0]) {
										has_seen = true;
										break;
									}
								}
								if (!has_seen) {
									double selected_probability = 1.0 / (1.0 + ancestors[0]->average_remaining_experiments_from_start);
									uniform_real_distribution<double> distribution(0.0, 1.0);
									if (distribution(generator) < selected_probability) {
										run_helper.experiment_histories.push_back(new ExperimentHistory(ancestors[0]));
									} else {
										is_continue = false;
									}
									run_helper.experiments_seen_order.push_back(ancestors[0]);
								} else {
									is_continue = false;
								}
							} else {
								ExperimentHistory* ancestor_experiment_history = run_helper.experiment_histories.back();
								bool has_seen = false;
								for (int e_index = 0; e_index < (int)ancestor_experiment_history->experiments_seen_order.size(); e_index++) {
									if (ancestor_experiment_history->experiments_seen_order[e_index] == ancestors[ancestor_index]) {
										has_seen = true;
										break;
									}
								}
								if (!has_seen) {
									double selected_probability = 1.0 / (1.0 + ancestors[ancestor_index]->average_remaining_experiments_from_start);
									uniform_real_distribution<double> distribution(0.0, 1.0);
									if (distribution(generator) < selected_probability) {
										run_helper.experiment_histories.push_back(new ExperimentHistory(ancestors[ancestor_index]));
									} else {
										is_continue = false;
									}
									ancestor_experiment_history->experiments_seen_order.push_back(ancestors[ancestor_index]);
								} else {
									is_continue = false;
								}
							}

							for (int e_index = ancestor_index+1; e_index < (int)ancestors.size(); e_index++) {
								if (!is_continue) {
									break;
								}

								ExperimentHistory* parent_experiment_history = run_helper.experiment_histories.back();

								double selected_probability = 1.0 / (1.0 + ancestors[e_index]->average_remaining_experiments_from_start);
								uniform_real_distribution<double> distribution(0.0, 1.0);
								if (distribution(generator) < selected_probability) {
									run_helper.experiment_histories.push_back(new ExperimentHistory(ancestors[e_index]));
								} else {
									is_continue = false;
								}
								parent_experiment_history->experiments_seen_order.push_back(ancestors[e_index]);
							}

							if (is_continue) {
								history = run_helper.experiment_histories.back();
								is_selected = true;
							}
						}
					}

					break;
				case EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING:
				case EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING:
					/**
					 * - select root_experiment for proper comparison
					 */
					{
						bool is_verify = false;
						for (int e_index = 0; e_index < (int)this->root_experiment->verify_experiments.size(); e_index++) {
							if (this->root_experiment->verify_experiments[e_index] == this) {
								is_verify = true;
								break;
							}
						}
						if (is_verify) {
							if (run_helper.experiment_histories.size() == 0) {
								bool has_seen = false;
								for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
									if (run_helper.experiments_seen_order[e_index] == this->root_experiment) {
										has_seen = true;
										break;
									}
								}
								if (!has_seen) {
									double selected_probability = 1.0 / (1.0 + this->root_experiment->average_remaining_experiments_from_start);
									uniform_real_distribution<double> distribution(0.0, 1.0);
									if (distribution(generator) < selected_probability) {
										run_helper.experiment_histories.push_back(new ExperimentHistory(this->root_experiment));
									}
									run_helper.experiments_seen_order.push_back(this->root_experiment);
								}
							}
						}
					}

					break;
				case EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST:
				case EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND:
					{
						bool is_verify = false;
						for (int e_index = 0; e_index < (int)this->root_experiment->verify_experiments.size(); e_index++) {
							if (this->root_experiment->verify_experiments[e_index] == this) {
								is_verify = true;
								break;
							}
						}
						if (is_verify) {
							if (run_helper.experiment_histories.size() == 1
									&& run_helper.experiment_histories[0]->experiment == this->root_experiment) {
								/**
								 * - don't append to run_helper.experiment_histories
								 *   - let backprop occur on root
								 * 
								 * - leave history as NULL and special case EXPERIMENT
								 */
								is_selected = true;
							} else if (run_helper.experiment_histories.size() == 0) {
								bool has_seen = false;
								for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
									if (run_helper.experiments_seen_order[e_index] == this->root_experiment) {
										has_seen = true;
										break;
									}
								}
								if (!has_seen) {
									double selected_probability = 1.0 / (1.0 + this->root_experiment->average_remaining_experiments_from_start);
									uniform_real_distribution<double> distribution(0.0, 1.0);
									if (distribution(generator) < selected_probability) {
										run_helper.experiment_histories.push_back(new ExperimentHistory(this->root_experiment));
										is_selected = true;
									}
									run_helper.experiments_seen_order.push_back(this->root_experiment);
								}
							}
						}
					}

					break;
				}
			}
		}

		if (is_selected) {
			switch (this->state) {
			case EXPERIMENT_STATE_TRAIN_EXISTING:
				train_existing_activate(context,
										run_helper,
										history);
				break;
			case EXPERIMENT_STATE_EXPLORE_CREATE:
				explore_create_activate(context,
										run_helper,
										history);
				break;
			case EXPERIMENT_STATE_EXPLORE_MEASURE:
				explore_measure_activate(curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper,
										 history);
				break;
			case EXPERIMENT_STATE_TRAIN_NEW:
				train_new_activate(curr_node,
								   problem,
								   context,
								   exit_depth,
								   exit_node,
								   run_helper,
								   history);
				break;
			case EXPERIMENT_STATE_MEASURE:
				measure_activate(curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper);
				break;
			case EXPERIMENT_STATE_VERIFY_1ST:
			case EXPERIMENT_STATE_VERIFY_2ND:
				verify_activate(curr_node,
								problem,
								context,
								exit_depth,
								exit_node,
								run_helper);
				break;
			#if defined(MDEBUG) && MDEBUG
			case EXPERIMENT_STATE_CAPTURE_VERIFY:
				capture_verify_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper);
				break;
			#endif /* MDEBUG */
			case EXPERIMENT_STATE_ROOT_VERIFY:
				root_verify_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
				break;
			case EXPERIMENT_STATE_EXPERIMENT:
				experiment_activate(curr_node,
									context,
									run_helper,
									history);
				break;
			case EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST:
			case EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND:
				experiment_verify_activate(curr_node,
										   context,
										   run_helper);
				break;
			}

			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void Experiment::backprop(double target_val,
						  RunHelper& run_helper) {
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper);
		break;
	case EXPERIMENT_STATE_EXPLORE_CREATE:
		explore_create_backprop(target_val,
								run_helper);
		break;
	case EXPERIMENT_STATE_EXPLORE_MEASURE:
		explore_measure_backprop(target_val,
								 run_helper);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case EXPERIMENT_STATE_VERIFY_1ST:
	case EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(target_val,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	case EXPERIMENT_STATE_EXPERIMENT:
		experiment_backprop(target_val,
							run_helper);
		break;
	case EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING:
	case EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING:
		experiment_verify_existing_backprop(target_val,
											run_helper);
		break;
	case EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST:
	case EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND:
		experiment_verify_backprop(target_val,
								   run_helper);
		break;
	}
}
