#include "new_info_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

bool NewInfoExperiment::activate(AbstractNode* experiment_node,
								 bool is_branch,
								 AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	bool is_selected = false;
	NewInfoExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		int match_index = -1;
		for (int e_index = 0; e_index < (int)run_helper.experiment_histories.size(); e_index++) {
			if (run_helper.experiment_histories[e_index]->experiment == this) {
				match_index = e_index;
				break;
			}
		}
		if (match_index != -1) {
			history = (NewInfoExperimentHistory*)run_helper.experiment_histories[match_index];
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
							history = new NewInfoExperimentHistory(this);
							run_helper.experiment_histories.push_back(history);
							is_selected = true;
						}

						run_helper.experiments_seen_order.push_back(this);
					}
				}
			} else {
				switch (this->root_experiment->root_state) {
				case ROOT_EXPERIMENT_STATE_EXPERIMENT:
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
										switch (ancestors[0]->type) {
										case EXPERIMENT_TYPE_BRANCH:
											{
												BranchExperiment* branch_ancestor = (BranchExperiment*)ancestors[0];
												run_helper.experiment_histories.push_back(new BranchExperimentHistory(branch_ancestor));
											}
											break;
										// case EXPERIMENT_TYPE_PASS_THROUGH:
										// 	{
										// 		PassThroughExperiment* pass_through_ancestor = (PassThroughExperiment*)ancestors[0];
										// 		run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(pass_through_ancestor));
										// 	}
										// 	break;
										case EXPERIMENT_TYPE_NEW_INFO:
											{
												NewInfoExperiment* new_info_ancestor = (NewInfoExperiment*)ancestors[0];
												run_helper.experiment_histories.push_back(new NewInfoExperimentHistory(new_info_ancestor));
											}
											break;
										}
									} else {
										is_continue = false;
									}
									run_helper.experiments_seen_order.push_back(ancestors[0]);
								} else {
									is_continue = false;
								}
							} else {
								AbstractExperimentHistory* ancestor_experiment_history = run_helper.experiment_histories.back();
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
										switch (ancestors[ancestor_index]->type) {
										case EXPERIMENT_TYPE_BRANCH:
											{
												BranchExperiment* branch_ancestor = (BranchExperiment*)ancestors[ancestor_index];
												run_helper.experiment_histories.push_back(new BranchExperimentHistory(branch_ancestor));
											}
											break;
										// case EXPERIMENT_TYPE_PASS_THROUGH:
										// 	{
										// 		PassThroughExperiment* pass_through_ancestor = (PassThroughExperiment*)ancestors[ancestor_index];
										// 		run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(pass_through_ancestor));
										// 	}
										// 	break;
										case EXPERIMENT_TYPE_NEW_INFO:
											{
												NewInfoExperiment* new_info_ancestor = (NewInfoExperiment*)ancestors[ancestor_index];
												run_helper.experiment_histories.push_back(new NewInfoExperimentHistory(new_info_ancestor));
											}
											break;
										}
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

								AbstractExperimentHistory* parent_experiment_history = run_helper.experiment_histories.back();

								double selected_probability = 1.0 / (1.0 + ancestors[e_index]->average_remaining_experiments_from_start);
								uniform_real_distribution<double> distribution(0.0, 1.0);
								if (distribution(generator) < selected_probability) {
									switch (ancestors[e_index]->type) {
									case EXPERIMENT_TYPE_BRANCH:
										{
											BranchExperiment* branch_ancestor = (BranchExperiment*)ancestors[e_index];
											run_helper.experiment_histories.push_back(new BranchExperimentHistory(branch_ancestor));
										}
										break;
									// case EXPERIMENT_TYPE_PASS_THROUGH:
									// 	{
									// 		PassThroughExperiment* pass_through_ancestor = (PassThroughExperiment*)ancestors[e_index];
									// 		run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(pass_through_ancestor));
									// 	}
									// 	break;
									case EXPERIMENT_TYPE_NEW_INFO:
										{
											NewInfoExperiment* new_info_ancestor = (NewInfoExperiment*)ancestors[e_index];
											run_helper.experiment_histories.push_back(new NewInfoExperimentHistory(new_info_ancestor));
										}
										break;
									}
								} else {
									is_continue = false;
								}
								parent_experiment_history->experiments_seen_order.push_back(ancestors[e_index]);
							}

							if (is_continue) {
								history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();
								is_selected = true;
							}
						}
					}

					break;
				case ROOT_EXPERIMENT_STATE_VERIFY_EXISTING:
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
										switch (this->root_experiment->type) {
										case EXPERIMENT_TYPE_BRANCH:
											{
												BranchExperiment* branch_root = (BranchExperiment*)this->root_experiment;
												run_helper.experiment_histories.push_back(new BranchExperimentHistory(branch_root));
											}
											break;
										// case EXPERIMENT_TYPE_PASS_THROUGH:
										// 	{
										// 		PassThroughExperiment* pass_through_root = (PassThroughExperiment*)this->root_experiment;
										// 		run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(pass_through_root));
										// 	}
										// 	break;
										case EXPERIMENT_TYPE_NEW_INFO:
											{
												NewInfoExperiment* new_info_root = (NewInfoExperiment*)this->root_experiment;
												run_helper.experiment_histories.push_back(new NewInfoExperimentHistory(new_info_root));
											}
											break;
										}
									}
									run_helper.experiments_seen_order.push_back(this->root_experiment);
								}
							}
						}
					}

					break;
				case ROOT_EXPERIMENT_STATE_VERIFY:
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
										switch (this->root_experiment->type) {
										case EXPERIMENT_TYPE_BRANCH:
											{
												BranchExperiment* branch_root = (BranchExperiment*)this->root_experiment;
												run_helper.experiment_histories.push_back(new BranchExperimentHistory(branch_root));
											}
											break;
										// case EXPERIMENT_TYPE_PASS_THROUGH:
										// 	{
										// 		PassThroughExperiment* pass_through_root = (PassThroughExperiment*)this->root_experiment;
										// 		run_helper.experiment_histories.push_back(new PassThroughExperimentHistory(pass_through_root));
										// 	}
										// 	break;
										case EXPERIMENT_TYPE_NEW_INFO:
											{
												NewInfoExperiment* new_info_root = (NewInfoExperiment*)this->root_experiment;
												run_helper.experiment_histories.push_back(new NewInfoExperimentHistory(new_info_root));
											}
											break;
										}
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
	}

	if (is_selected) {
		switch (this->state) {
		case NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(context,
									  history);

			return false;
		case NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO:
			explore_info_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  history);

			return false;
		case NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE:
			return explore_sequence_activate(curr_node,
											 problem,
											 context,
											 run_helper,
											 history);
		case NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   context,
							   run_helper,
							   history);

			return true;
		case NEW_INFO_EXPERIMENT_STATE_MEASURE:
			return measure_activate(curr_node,
									problem,
									context,
									run_helper,
									history);
		case NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING:
			verify_existing_activate(context,
									 history);

			return false;
		case NEW_INFO_EXPERIMENT_STATE_VERIFY:
			return verify_activate(curr_node,
								   problem,
								   context,
								   run_helper,
								   history);
		#if defined(MDEBUG) && MDEBUG
		case NEW_INFO_EXPERIMENT_STATE_CAPTURE_VERIFY:
			return capture_verify_activate(curr_node,
										   problem,
										   run_helper);
		#endif /* MDEBUG */
		case NEW_INFO_EXPERIMENT_STATE_ROOT_VERIFY:
			return root_verify_activate(curr_node,
										problem,
										run_helper);
		case NEW_INFO_EXPERIMENT_STATE_EXPERIMENT:
			switch (this->root_state) {
			case ROOT_EXPERIMENT_STATE_EXPERIMENT:
				return experiment_activate(curr_node,
										   problem,
										   context,
										   run_helper,
										   history);
			case ROOT_EXPERIMENT_STATE_VERIFY_EXISTING:
				experiment_verify_existing_activate(context,
													history);

				return false;
			case ROOT_EXPERIMENT_STATE_VERIFY:
				return experiment_verify_activate(curr_node,
												  problem,
												  context,
												  run_helper,
												  history);
			}
		}
	}

	return false;
}

void NewInfoExperiment::back_activate(vector<ContextLayer>& context,
									  RunHelper& run_helper) {
	switch (this->state) {
	case NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_back_activate(context,
									   run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO:
		explore_info_back_activate(context,
								   run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE:
		explore_sequence_back_activate(context,
									   run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_back_activate(context,
								run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_MEASURE:
		measure_back_activate(context,
							  run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_back_activate(context,
									  run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_VERIFY:
		verify_back_activate(context,
							 run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_EXPERIMENT:
		switch (this->root_state) {
		case ROOT_EXPERIMENT_STATE_EXPERIMENT:
			experiment_back_activate(context,
									 run_helper);
			break;
		case ROOT_EXPERIMENT_STATE_VERIFY_EXISTING:
			experiment_verify_existing_back_activate(context,
													 run_helper);
			break;
		case ROOT_EXPERIMENT_STATE_VERIFY:
			experiment_verify_back_activate(context,
											run_helper);
			break;
		}

		break;
	}
}

void NewInfoExperiment::backprop(double target_val,
								 RunHelper& run_helper) {
	switch (this->state) {
	case NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO:
		explore_info_backprop(target_val,
							  run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE:
		explore_sequence_backprop(target_val,
								  run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case NEW_INFO_EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val,
						run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_INFO_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	case NEW_INFO_EXPERIMENT_STATE_EXPERIMENT:
		switch (this->root_state) {
		case ROOT_EXPERIMENT_STATE_EXPERIMENT:
			experiment_backprop(run_helper);
			break;
		case ROOT_EXPERIMENT_STATE_VERIFY_EXISTING:
			experiment_verify_existing_backprop(target_val,
												run_helper);
			break;
		case ROOT_EXPERIMENT_STATE_VERIFY:
			experiment_verify_backprop(target_val,
									   run_helper);
			break;
		}

		break;
	}
}
