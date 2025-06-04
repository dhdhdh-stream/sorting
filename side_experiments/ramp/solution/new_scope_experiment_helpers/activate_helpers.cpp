#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void NewScopeExperiment::pre_activate(RunHelper& run_helper) {
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
		= run_helper.experiment_histories.find(this);
	if (it == run_helper.experiment_histories.end()) {
		run_helper.experiment_histories[this] = new NewScopeExperimentHistory(this);
	} else {
		NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)it->second;
		history->local_successful_hits.clear();
	}
}

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	bool has_match = false;
	bool is_test;
	int location_index;
	if (this->test_location_start == experiment_node
			&& this->test_location_is_branch == is_branch) {
		has_match = true;
		is_test = true;
	}
	if (!has_match) {
		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			if (this->successful_location_starts[s_index] == experiment_node
					&& this->successful_location_is_branch[s_index] == is_branch) {
				has_match = true;
				is_test = false;
				location_index = s_index;
				break;
			}
		}
	}

	if (has_match) {
		NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_histories[this];

		if (is_test) {
			run_helper.num_experiment_instances++;

			history->hit_test = true;

			if (history->is_active) {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
				this->new_scope->experiment_activate(problem,
													 run_helper,
													 inner_scope_history);
				delete inner_scope_history;

				curr_node = this->test_location_exit;
			}
		} else {
			if (history->is_active) {
				history->local_successful_hits.push_back(location_index);

				this->successful_scope_nodes[location_index]->new_scope_activate(
					curr_node,
					problem,
					run_helper,
					scope_history);
			}
		}
	}
}

void NewScopeExperiment::back_activate(RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	if (this->test_location_start == NULL) {
		NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_histories[this];
		if (history->is_active) {
			uniform_int_distribution<int> select_distribution(0, history->instance_count);
			if (select_distribution(generator) == 0) {
				vector<AbstractNode*> possible_starts;
				vector<bool> possible_is_branch;
				for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
						it != scope_history->node_histories.end(); it++) {
					if (it->second->node->experiment == NULL) {
						bool is_branch;
						switch (it->second->node->type) {
						case NODE_TYPE_ACTION:
						case NODE_TYPE_SCOPE:
						case NODE_TYPE_OBS:
							is_branch = false;
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								is_branch = branch_node_history->is_branch;
							}
							break;
						}

						bool has_match = false;
						for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
							if (this->successful_location_starts[s_index] == it->second->node
									&& this->successful_location_is_branch[s_index] == is_branch) {
								has_match = true;
								break;
							}
						}
						if (!has_match) {
							possible_starts.push_back(it->second->node);
							possible_is_branch.push_back(is_branch);
						}
					}
				}
				for (int s_index = 0; s_index < (int)history->local_successful_hits.size(); s_index++) {
					AbstractNode* possible_node = this->successful_scope_nodes[history->local_successful_hits[s_index]];
					if (possible_node->experiment == NULL) {
						possible_starts.push_back(possible_node);
						possible_is_branch.push_back(false);
					}
				}

				if (possible_starts.size() > 0) {
					uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
					int start_index = start_distribution(generator);

					history->potential_start = possible_starts[start_index];
					history->potential_is_branch = possible_is_branch[start_index];
				}
			}
			history->instance_count++;
		}
	}
}

void NewScopeExperiment::backprop(double target_val,
								  RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_histories[this];

	if (history->hit_test) {
		if (history->is_active) {
			this->test_location_new_sum_score += target_val;
			this->test_location_new_count++;
		} else {
			this->test_location_existing_sum_score += target_val;
			this->test_location_existing_count++;
		}

		if (this->test_location_existing_count + this->test_location_new_count >= MEASURE_NUM_DATAPOINTS) {
			double existing_score = this->test_location_existing_sum_score / this->test_location_existing_count;
			double new_score = this->test_location_new_sum_score / this->test_location_new_count;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->test_location_existing_sum_score = 0.0;
				this->test_location_existing_count = 0;
				this->test_location_new_sum_score = 0.0;
				this->test_location_new_count = 0;

				switch (this->test_location_state) {
				case LOCATION_STATE_MEASURE_1_PERCENT:
					this->test_location_state = LOCATION_STATE_MEASURE_5_PERCENT;
					break;
				case LOCATION_STATE_MEASURE_5_PERCENT:
					this->test_location_state = LOCATION_STATE_MEASURE_10_PERCENT;
					break;
				case LOCATION_STATE_MEASURE_10_PERCENT:
					this->test_location_state = LOCATION_STATE_MEASURE_25_PERCENT;
					break;
				case LOCATION_STATE_MEASURE_25_PERCENT:
					this->test_location_state = LOCATION_STATE_MEASURE_50_PERCENT;
					break;
				case LOCATION_STATE_MEASURE_50_PERCENT:
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = this->scope_context;
					new_scope_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_scope_node->scope = this->new_scope;

					if (this->test_location_exit == NULL) {
						new_scope_node->next_node_id = -1;
						new_scope_node->next_node = NULL;
					} else {
						new_scope_node->next_node_id = this->test_location_exit->id;
						new_scope_node->next_node = this->test_location_exit;
					}

					this->successful_location_starts.push_back(this->test_location_start);
					this->successful_location_is_branch.push_back(this->test_location_is_branch);
					this->successful_scope_nodes.push_back(new_scope_node);

					this->test_location_start = NULL;

					if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
						this->improvement = new_score - existing_score;

						cout << "NewScopeExperiment success" << endl;
						cout << "this->improvement: " << this->improvement << endl;

						this->result = EXPERIMENT_RESULT_SUCCESS;
					}
				}
			} else {
				this->test_location_start->experiment = NULL;
				this->test_location_start = NULL;

				cout << "NewScopeExperiment fail " << this->test_location_state << endl;

				if (this->generalize_iter == -1
						&& this->successful_location_starts.size() == 0) {
					this->result = EXPERIMENT_RESULT_FAIL;
					/**
					 * - only continue if first succeeds
					 */
				} else {
					this->generalize_iter++;
					if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
						this->result = EXPERIMENT_RESULT_FAIL;
					}
				}
			}
		}
	} else if (this->test_location_start == NULL) {
		if (history->is_active) {
			if (history->potential_start != NULL) {
				vector<AbstractNode*> possible_exits;

				AbstractNode* starting_node;
				switch (history->potential_start->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)history->potential_start;
						starting_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)history->potential_start;
						starting_node = scope_node->next_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)history->potential_start;
						if (history->potential_is_branch) {
							starting_node = branch_node->branch_next_node;
						} else {
							starting_node = branch_node->original_next_node;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)history->potential_start;
						starting_node = obs_node->next_node;
					}
					break;
				}

				this->scope_context->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				AbstractNode* exit_next_node = possible_exits[random_index];

				this->test_location_start = history->potential_start;
				this->test_location_is_branch = history->potential_is_branch;
				this->test_location_exit = exit_next_node;
				this->test_location_state = LOCATION_STATE_MEASURE_1_PERCENT;
				this->test_location_existing_sum_score = 0.0;
				this->test_location_existing_count = 0;
				this->test_location_new_sum_score = 0.0;
				this->test_location_new_count = 0;

				cout << "NewScopeExperiment new test" << endl;

				history->potential_start->experiment = this;
			}
		}
	}
}
