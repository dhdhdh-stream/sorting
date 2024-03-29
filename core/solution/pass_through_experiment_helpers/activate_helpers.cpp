#include "pass_through_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"

using namespace std;

bool PassThroughExperiment::activate(AbstractNode*& curr_node,
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
		PassThroughExperimentHistory* history = NULL;
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = (PassThroughExperimentHistory*)run_helper.experiment_histories[match_index];
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
							history = new PassThroughExperimentHistory(this);
							run_helper.experiment_histories.push_back(history);
							is_selected = true;
						}

						run_helper.experiments_seen_order.push_back(this);
					}
				}
			} else {
				switch (this->root_experiment->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
					{
						vector<AbstractExperiment*> ancestors;
						AbstractExperiment* curr_experiment = this;
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
										run_helper.experiments_seen_order.push_back(ancestors[0]);
										run_helper.experiment_histories.push_back(new PassThroughExperimentHistory((PassThroughExperiment*)ancestors[0]));

										for (int e_index = 1; e_index < (int)ancestors.size()-1; e_index++) {
											PassThroughExperimentHistory* parent_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
											parent_pass_through_experiment_history->experiments_seen_order.push_back(ancestors[e_index]);
											run_helper.experiment_histories.push_back(new PassThroughExperimentHistory((PassThroughExperiment*)ancestors[e_index]));
										}

										PassThroughExperimentHistory* parent_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
										parent_pass_through_experiment_history->experiments_seen_order.push_back(this);
										run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(this));

										history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
										is_selected = true;
									} else {
										run_helper.experiments_seen_order.push_back(ancestors[0]);
									}
								}
							} else {
								PassThroughExperimentHistory* ancestor_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
								bool has_seen = false;
								for (int e_index = 0; e_index < (int)ancestor_pass_through_experiment_history->experiments_seen_order.size(); e_index++) {
									if (ancestor_pass_through_experiment_history->experiments_seen_order[e_index] == ancestors[ancestor_index]) {
										has_seen = true;
										break;
									}
								}

								if (!has_seen) {
									double selected_probability = 1.0 / (1.0 + ancestors[ancestor_index]->average_remaining_experiments_from_start);
									uniform_real_distribution<double> distribution(0.0, 1.0);
									if (distribution(generator) < selected_probability) {
										for (int e_index = ancestor_index; e_index < (int)ancestors.size()-1; e_index++) {
											PassThroughExperimentHistory* parent_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
											parent_pass_through_experiment_history->experiments_seen_order.push_back(ancestors[e_index]);
											run_helper.experiment_histories.push_back(new PassThroughExperimentHistory((PassThroughExperiment*)ancestors[e_index]));
										}

										PassThroughExperimentHistory* parent_pass_through_experiment_history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
										parent_pass_through_experiment_history->experiments_seen_order.push_back(this);
										run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(this));

										history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();
										is_selected = true;
									} else {
										ancestor_pass_through_experiment_history->experiments_seen_order.push_back(ancestors[ancestor_index]);
									}
								}
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
									run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(this->root_experiment));
									is_selected = true;
								}
								run_helper.experiments_seen_order.push_back(this->root_experiment);
							}
						}
					}

					break;
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
