#include "new_scope_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const double ACTIONS_PER_TEST = 20.0;

bool NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	bool has_match = false;
	bool is_test;
	int location_index;
	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
		if (this->test_location_starts[t_index] == experiment_node
				&& this->test_location_is_branch[t_index] == is_branch) {
			has_match = true;
			is_test = true;
			location_index = t_index;
			break;
		}
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
		uniform_int_distribution<int> select_distribution(0, 3);
		if (select_distribution(generator) == 0) {
			if (is_test) {
				NewScopeExperimentHistory* history = new NewScopeExperimentHistory(this);
				history->test_location_index = location_index;
				run_helper.experiment_histories.push_back(history);

				switch (this->test_location_states[location_index]) {
				case LOCATION_STATE_MEASURE_NEW:
				case LOCATION_STATE_VERIFY_NEW_1ST:
				case LOCATION_STATE_VERIFY_NEW_2ND:
					ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
					this->new_scope->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;

					curr_node = this->test_location_exits[location_index];

					return true;
				}
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
				this->new_scope->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;

				curr_node = this->successful_location_exits[location_index];

				return true;
			}
		}
	}

	return false;
}

void NewScopeExperiment::back_activate(RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	run_helper.num_experiments_seen++;

	if (this->generalize_iter != -1) {
		int num_test_locations_seen = 0;
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
				this->test_location_starts[t_index]->id);
			if (it != scope_history->node_histories.end()) {
				if (this->test_location_starts[t_index]->type == NODE_TYPE_BRANCH) {
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch == this->test_location_is_branch[t_index]) {
						num_test_locations_seen++;
					}
				} else {
					num_test_locations_seen++;
				}
			}
		}

		double expected_number_of_experiments = (int)scope_history->node_histories.size() / ACTIONS_PER_TEST;
		if (expected_number_of_experiments > num_test_locations_seen) {
			vector<AbstractNode*> possible_starts;
			vector<bool> possible_is_branch;
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
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
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					if (this->test_location_starts[t_index] == it->second->node
							&& this->test_location_is_branch[t_index] == is_branch) {
						has_match = true;
						break;
					}
				}
				if (!has_match) {
					for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
						if (this->successful_location_starts[s_index] == it->second->node
								&& this->successful_location_is_branch[s_index] == is_branch) {
							has_match = true;
							break;
						}
					}
				}
				if (!has_match) {
					possible_starts.push_back(it->second->node);
					possible_is_branch.push_back(is_branch);
				}
			}

			uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
			int start_index = start_distribution(generator);

			AbstractNode* new_start_node = possible_starts[start_index];
			bool new_is_branch = possible_is_branch[start_index];

			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (new_start_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)new_start_node;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)new_start_node;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)new_start_node;
					if (new_is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)new_start_node;
					starting_node = obs_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			AbstractNode* new_exit_next_node = possible_exits[random_index];

			this->test_location_starts.push_back(new_start_node);
			this->test_location_is_branch.push_back(new_is_branch);
			this->test_location_exits.push_back(new_exit_next_node);
			this->test_location_states.push_back(LOCATION_STATE_MEASURE_EXISTING);
			this->test_location_existing_scores.push_back(0.0);
			this->test_location_existing_counts.push_back(0);
			this->test_location_new_scores.push_back(0.0);
			this->test_location_new_counts.push_back(0);

			new_start_node->experiments.insert(new_start_node->experiments.begin(), this);
		}
	}
}

void NewScopeExperiment::backprop(AbstractExperimentHistory* history) {

}

void NewScopeExperiment::update() {

}
