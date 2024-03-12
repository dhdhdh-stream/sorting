#include "pass_through_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"

using namespace std;

bool PassThroughExperiment::activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 RunHelper& run_helper) {
	bool is_selected = false;
	PassThroughExperimentHistory* history = NULL;
	if (run_helper.throw_id == this->throw_id) {
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			bool matches_context = false;
			if (this->is_fuzzy_match) {
				int c_index = (int)this->scope_context.size()-2;
				int l_index = (int)context.size()-2;
				while (true) {
					if (c_index < 0) {
						matches_context = true;
						break;
					}

					if (l_index < 0) {
						break;
					}

					if (this->scope_context[c_index] == context[l_index].scope
							&& this->node_context[c_index] == context[l_index].node) {
						c_index--;
					}
					l_index--;
				}
			} else {
				int c_index = (int)this->scope_context.size()-2;
				int l_index = (int)context.size()-2;
				while (true) {
					if (c_index < 0) {
						matches_context = true;
						break;
					}

					if (l_index < 0) {
						break;
					}

					if (this->scope_context[c_index] == context[l_index].scope
							&& this->node_context[c_index] == context[l_index].node) {
						c_index--;
						l_index--;
					} else {
						break;
					}
				}
			}

			if (matches_context) {
				history = (PassThroughExperimentHistory*)run_helper.experiment_histories[match_index];
				is_selected = true;
			}
		} else {
			if (this->parent_experiment == NULL) {
				if (run_helper.experiment_histories.size() == 0) {
					bool matches_context = false;
					if (this->is_fuzzy_match) {
						int c_index = (int)this->scope_context.size()-2;
						int l_index = (int)context.size()-2;
						while (true) {
							if (c_index < 0) {
								matches_context = true;
								break;
							}

							if (l_index < 0) {
								break;
							}

							if (this->scope_context[c_index] == context[l_index].scope
									&& this->node_context[c_index] == context[l_index].node) {
								c_index--;
							}
							l_index--;
						}
					} else {
						int c_index = (int)this->scope_context.size()-2;
						int l_index = (int)context.size()-2;
						while (true) {
							if (c_index < 0) {
								matches_context = true;
								break;
							}

							if (l_index < 0) {
								break;
							}

							if (this->scope_context[c_index] == context[l_index].scope
									&& this->node_context[c_index] == context[l_index].node) {
								c_index--;
								l_index--;
							} else {
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
								history = new PassThroughExperimentHistory(this);
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
						bool matches_context = false;
						if (this->is_fuzzy_match) {
							int c_index = (int)this->scope_context.size()-2;
							int l_index = (int)context.size()-2;
							while (true) {
								if (c_index < 0) {
									matches_context = true;
									break;
								}

								if (l_index < 0) {
									break;
								}

								if (this->scope_context[c_index] == context[l_index].scope
										&& this->node_context[c_index] == context[l_index].node) {
									c_index--;
								}
								l_index--;
							}
						} else {
							int c_index = (int)this->scope_context.size()-2;
							int l_index = (int)context.size()-2;
							while (true) {
								if (c_index < 0) {
									matches_context = true;
									break;
								}

								if (l_index < 0) {
									break;
								}

								if (this->scope_context[c_index] == context[l_index].scope
										&& this->node_context[c_index] == context[l_index].node) {
									c_index--;
									l_index--;
								} else {
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
									history = new PassThroughExperimentHistory(this);
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
						bool is_verify = false;
						for (int e_index = 0; e_index < (int)this->root_experiment->verify_experiments.size(); e_index++) {
							if (this->root_experiment->verify_experiments[e_index] == this) {
								is_verify = true;
								break;
							}
						}
						if (is_verify) {
							bool matches_context = false;
							if (this->is_fuzzy_match) {
								int c_index = (int)this->scope_context.size()-2;
								int l_index = (int)context.size()-2;
								while (true) {
									if (c_index < 0) {
										matches_context = true;
										break;
									}

									if (l_index < 0) {
										break;
									}

									if (this->scope_context[c_index] == context[l_index].scope
											&& this->node_context[c_index] == context[l_index].node) {
										c_index--;
									}
									l_index--;
								}
							} else {
								int c_index = (int)this->scope_context.size()-2;
								int l_index = (int)context.size()-2;
								while (true) {
									if (c_index < 0) {
										matches_context = true;
										break;
									}

									if (l_index < 0) {
										break;
									}

									if (this->scope_context[c_index] == context[l_index].scope
											&& this->node_context[c_index] == context[l_index].node) {
										c_index--;
										l_index--;
									} else {
										break;
									}
								}
							}

							if (matches_context) {
								/**
								 * - don't append to run_helper.experiment_histories
								 *   - let backprop occur on root
								 * 
								 * - leave history as NULL and special case EXPERIMENT
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
		case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(history);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_CREATE:
			explore_create_activate(curr_node,
									context,
									run_helper,
									history);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_MEASURE:
			explore_measure_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW:
			measure_new_activate(curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW:
		case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW:
			verify_new_activate(curr_node,
								problem,
								context,
								exit_depth,
								exit_node,
								run_helper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY:
			root_verify_activate(curr_node,
								 run_helper);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
			experiment_activate(curr_node,
								context,
								run_helper,
								history);
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
		case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
			experiment_verify_new_activate(curr_node,
										   run_helper);
			break;
		}

		return true;
	} else {
		return false;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	PassThroughExperimentHistory* pass_through_experiment_history =
		(PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper,
								  pass_through_experiment_history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_CREATE:
		explore_create_backprop(pass_through_experiment_history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_MEASURE:
		explore_measure_backprop(target_val,
								 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_backprop(target_val,
							 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW:
		verify_new_backprop(target_val,
							run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
		experiment_backprop(target_val,
							run_helper,
							pass_through_experiment_history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING:
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING:
		experiment_verify_existing_backprop(target_val,
											run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
		experiment_verify_new_backprop(target_val,
									   run_helper);
		break;
	}
}
