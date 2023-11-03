#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::experiment_activate(int& curr_node_id,
												Problem& problem,
												vector<ContextLayer>& context,
												int& exit_depth,
												int& exit_node_id,
												RunHelper& run_helper,
												AbstractExperimentHistory*& history) {
	int normal_exit = true;
	int starting_curr_node_id = curr_node_id;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node_id
				problem,
				context,
				exit_depth,
				exit_node_id,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}

		if (curr_node_id != starting_curr_node_id || exit_depth != -1 || run_helper.exceeded_depth) {
			normal_exit = false;
			break;
		}
	}

	if (curr_node_id != starting_curr_node_id) {
		map<int, int>::iterator it = this->node_id_to_step_index.find(curr_node_id);
		if (it != this->node_id_to_step_index.end()) {
			for (int s_index = it->second; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
					this->best_actions[s_index]->activate(
						curr_node_id
						problem,
						context,
						exit_depth,
						exit_node_id,
						run_helper,
						action_node_history);
					delete action_node_history;
				} else {
					SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
					this->best_sequences[s_index]->activate(problem,
															context,
															run_helper,
															sequence_history);
					delete sequence_history;
				}
			}

			if (this->best_exit_depth == 0) {
				curr_node_id = this->best_exit_node_id;
			} else {
				exit_depth = this->best_exit_depth-1;
				exit_node_id = this->best_exit_node_id;
			}
		}
	}

	if (normal_exit) {
		if (this->best_exit_depth == 0) {
			curr_node_id = this->best_exit_node_id;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node_id = this->best_exit_node_id;
		}
	}
}
