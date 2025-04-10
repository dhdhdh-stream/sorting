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
const int NEW_SCOPE_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 5;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 40;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 400;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void NewScopeExperiment::test_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	history->hit_test = true;

	switch (this->test_location_state) {
	case LOCATION_STATE_MEASURE_NEW:
	case LOCATION_STATE_VERIFY_NEW_1ST:
	case LOCATION_STATE_VERIFY_NEW_2ND:
		{
			ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
			this->new_scope->experiment_activate(problem,
												 run_helper,
												 inner_scope_history);
			delete inner_scope_history;

			curr_node = this->test_location_exit;
		}
		break;
	}
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	bool is_fail = false;

	switch (this->test_location_state) {
	case LOCATION_STATE_MEASURE_EXISTING:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_MEASURE_NEW;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_MEASURE_NEW:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_EXISTING_1ST;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score = 0.0;
				this->test_location_count = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_1ST:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_VERIFY_NEW_1ST;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_1ST:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_EXISTING_2ND;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score = 0.0;
				this->test_location_count = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_2ND:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_VERIFY_NEW_2ND;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_2ND:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				double new_score = this->test_location_new_score / NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS;
				double existing_score = this->test_location_existing_score / NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS;
				this->improvement += (new_score - existing_score);

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

				new_scope_node->average_instances_per_run = this->test_location_start->average_instances_per_run;

				this->successful_location_starts.push_back(this->test_location_start);
				this->successful_location_is_branch.push_back(this->test_location_is_branch);
				this->successful_scope_nodes.push_back(new_scope_node);

				vector<AbstractNode*> possible_starting_nodes;
				for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
						it != scope_context->nodes.end(); it++) {
					if (it->second->experiment == NULL
							&& it->second->average_instances_per_run > 0.5) {
						possible_starting_nodes.push_back(it->second);
					}
				}
				for (int s_index = 0; s_index < (int)this->successful_scope_nodes.size(); s_index++) {
					if (this->successful_scope_nodes[s_index]->experiment == NULL) {
						possible_starting_nodes.push_back(this->successful_scope_nodes[s_index]);
					}
				}

				uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
				AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

				vector<AbstractNode*> possible_exits;

				AbstractNode* random_start_node;
				switch (node_context->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)potential_starting_node;
						random_start_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)potential_starting_node;
						random_start_node = scope_node->next_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)potential_starting_node;
						if (is_branch) {
							random_start_node = branch_node->branch_next_node;
						} else {
							random_start_node = branch_node->original_next_node;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)potential_starting_node;
						random_start_node = obs_node->next_node;
					}
					break;
				}

				this->scope_context->random_exit_activate(
					random_start_node,
					possible_exits);

				uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
				AbstractNode* exit_next_node = possible_exits[exit_distribution(generator)];

				this->test_location_start = potential_starting_node;
				this->test_location_is_branch = is_branch;
				this->test_location_exit = exit_next_node;
				this->test_location_state = LOCATION_STATE_MEASURE_EXISTING;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score= 0.0;
				this->test_location_count = 0;

				this->generalize_iter++;
			} else {
				is_fail = true;
			}
		}

		break;
	}

	if (is_fail) {
		this->test_location_start->experiment = NULL;

		if (this->generalize_iter == -1
				&& this->successful_location_starts.size() == 0) {
			this->result = EXPERIMENT_RESULT_FAIL;
			/**
			 * - only continue if first succeeds
			 */
		} else {
			vector<AbstractNode*> possible_starting_nodes;
			for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
					it != scope_context->nodes.end(); it++) {
				if (it->second->experiment == NULL
						&& it->second->average_instances_per_run > 0.5) {
					possible_starting_nodes.push_back(it->second);
				}
			}
			for (int s_index = 0; s_index < (int)this->successful_scope_nodes.size(); s_index++) {
				if (this->successful_scope_nodes[s_index]->experiment == NULL) {
					possible_starting_nodes.push_back(this->successful_scope_nodes[s_index]);
				}
			}

			uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
			AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

			vector<AbstractNode*> possible_exits;

			AbstractNode* random_start_node;
			switch (node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)potential_starting_node;
					random_start_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)potential_starting_node;
					random_start_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)potential_starting_node;
					if (is_branch) {
						random_start_node = branch_node->branch_next_node;
					} else {
						random_start_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)potential_starting_node;
					random_start_node = obs_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				random_start_node,
				possible_exits);

			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			AbstractNode* exit_next_node = possible_exits[exit_distribution(generator)];

			this->test_location_start = potential_starting_node;
			this->test_location_is_branch = is_branch;
			this->test_location_exit = exit_next_node;
			this->test_location_state = LOCATION_STATE_MEASURE_EXISTING;
			this->test_location_existing_score = 0.0;
			this->test_location_new_score= 0.0;
			this->test_location_count = 0;
		}
	}

	if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
		#if defined(MDEBUG) && MDEBUG
		this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
		this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

		this->state = NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY;
		this->state_iter = 0;
		#else
		this->result = EXPERIMENT_RESULT_SUCCESS;
		#endif /* MDEBUG */
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
