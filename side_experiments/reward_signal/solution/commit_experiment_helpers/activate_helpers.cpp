// #include "commit_experiment.h"

// #include <iostream>

// #include "abstract_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"
// #include "solution_wrapper.h"

// using namespace std;

// void CommitExperiment::check_activate(AbstractNode* experiment_node,
// 									  bool is_branch,
// 									  SolutionWrapper* wrapper) {
// 	if (is_branch == this->is_branch) {
// 		CommitExperimentHistory* history;
// 		if (wrapper->experiment_history != NULL) {
// 			history = (CommitExperimentHistory*)wrapper->experiment_history;
// 		} else {
// 			history = new CommitExperimentHistory(this);
// 			wrapper->experiment_history = history;
// 		}

// 		switch (this->state) {
// 		case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
// 			train_existing_check_activate(wrapper,
// 										  history);
// 			break;
// 		case COMMIT_EXPERIMENT_STATE_EXPLORE:
// 			explore_check_activate(wrapper,
// 								   history);
// 			break;
// 		case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
// 			find_save_check_activate(wrapper);
// 			break;
// 		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
// 			commit_train_existing_check_activate(wrapper);
// 			break;
// 		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
// 			commit_train_new_check_activate(wrapper,
// 											history);
// 			break;
// 		case COMMIT_EXPERIMENT_STATE_MEASURE:
// 			measure_check_activate(wrapper);
// 			break;
// 		#if defined(MDEBUG) && MDEBUG
// 		case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
// 			capture_verify_check_activate(wrapper);
// 			break;
// 		#endif /* MDEBUG */
// 		}
// 	}
// }

// void CommitExperiment::experiment_step(vector<double>& obs,
// 									   int& action,
// 									   bool& is_next,
// 									   bool& fetch_action,
// 									   SolutionWrapper* wrapper) {
// 	CommitExperimentState* experiment_state = (CommitExperimentState*)wrapper->experiment_context.back();
// 	switch (this->state) {
// 	case COMMIT_EXPERIMENT_STATE_EXPLORE:
// 		explore_step(obs,
// 					 action,
// 					 is_next,
// 					 fetch_action,
// 					 wrapper,
// 					 experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
// 		find_save_step(obs,
// 					   action,
// 					   is_next,
// 					   fetch_action,
// 					   wrapper,
// 					   experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
// 		commit_train_existing_step(obs,
// 								   action,
// 								   is_next,
// 								   wrapper,
// 								   experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
// 		commit_train_new_step(obs,
// 							  action,
// 							  is_next,
// 							  wrapper,
// 							  experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_MEASURE:
// 		measure_step(obs,
// 					 action,
// 					 is_next,
// 					 wrapper,
// 					 experiment_state);
// 		break;
// 	#if defined(MDEBUG) && MDEBUG
// 	case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
// 		capture_verify_step(obs,
// 							action,
// 							is_next,
// 							wrapper,
// 							experiment_state);
// 		break;
// 	#endif /* MDEBUG */
// 	}
// }

// void CommitExperiment::set_action(int action,
// 								  SolutionWrapper* wrapper) {
// 	CommitExperimentState* experiment_state = (CommitExperimentState*)wrapper->experiment_context.back();
// 	switch (this->state) {
// 	case COMMIT_EXPERIMENT_STATE_EXPLORE:
// 		explore_set_action(action,
// 						   experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
// 		find_save_set_action(action,
// 							 experiment_state);
// 		break;
// 	}
// }

// void CommitExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
// 	CommitExperimentState* experiment_state = (CommitExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
// 	switch (this->state) {
// 	case COMMIT_EXPERIMENT_STATE_EXPLORE:
// 		explore_exit_step(wrapper,
// 						  experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
// 		find_save_exit_step(wrapper,
// 							experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
// 		commit_train_existing_exit_step(wrapper,
// 										experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
// 		commit_train_new_exit_step(wrapper,
// 								   experiment_state);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_MEASURE:
// 		measure_exit_step(wrapper,
// 						  experiment_state);
// 		break;
// 	#if defined(MDEBUG) && MDEBUG
// 	case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
// 		capture_verify_exit_step(wrapper,
// 								 experiment_state);
// 		break;
// 	#endif /* MDEBUG */
// 	}
// }

// void CommitExperiment::backprop(double target_val,
// 								SolutionWrapper* wrapper) {
// 	CommitExperimentHistory* history = (CommitExperimentHistory*)wrapper->experiment_history;
// 	switch (this->state) {
// 	case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
// 		train_existing_backprop(target_val,
// 								history);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_EXPLORE:
// 		explore_backprop(target_val,
// 						 history);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
// 		find_save_backprop(target_val);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
// 		commit_train_existing_backprop(target_val,
// 									   history);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
// 		commit_train_new_backprop(target_val,
// 								  history);
// 		break;
// 	case COMMIT_EXPERIMENT_STATE_MEASURE:
// 		measure_backprop(target_val);
// 		break;
// 	#if defined(MDEBUG) && MDEBUG
// 	case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
// 		capture_verify_backprop();
// 		break;
// 	#endif /* MDEBUG */
// 	}
// }
