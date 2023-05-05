#include "solution.h"

#include <iostream>
#include <random>

#include "action_node.h"
#include "branch_node.h"
#include "fold_score_node.h"
#include "fold_sequence_node.h"
#include "globals.h"
#include "loop_fold_node.h"
#include "pass_through_node.h"
#include "scope_node.h"
#include "state_network.h"
#include "utilities.h"

using namespace std;

Solution::Solution() {
	this->average_score = 0.0;

	// starting action ACTION_START
	ActionNode* starting_node = new ActionNode(vector<int>(),
											   vector<StateNetwork*>(),
											   new StateNetwork(1,
																0,
																0,
																0,
																20));
	this->scopes.push_back(new Scope(0,
									 vector<bool>(),
									 false,
									 vector<StateNetwork*>(),
									 NULL,
									 NULL,
									 NULL,
									 NULL,
									 0.0,
									 0.0,
									 0.0,
									 0.0,
									 vector<AbstractNode*>{starting_node}));
	this->scopes[0]->id = 0;

	this->max_depth = 1;
	this->depth_limit = 11;
}

Solution::Solution(ifstream& input_file) {
	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes.push_back(new Scope(scope_save_file));
		scope_save_file.close();
	}

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

// TODO: add breaks for recursion
void Solution::random_run_helper(int scope_id,
								 vector<bool>& is_inner_scope,
								 vector<int>& existing_scope_ids,
								 vector<Action>& actions,
								 vector<int>& scope_context,
								 vector<int>& node_context,
								 int& early_exit_depth,
								 int& early_exit_node_id,
								 bool& exceeded_depth) {
	early_exit_depth = -1;

	if ((int)scope_context.size() > this->depth_limit) {
		exceeded_depth = true;
		return;
	}

	Scope* scope = this->scopes[scope_id];

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	int curr_node_id = 0;
	while (true) {
		if (curr_node_id == -1) {
			break;
		}

		if (scope->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)scope->nodes[curr_node_id];

			if (action_node->action.move != ACTION_START) {
				is_inner_scope.push_back(false);
				existing_scope_ids.push_back(-1);
				actions.push_back(action_node->action);
			}

			curr_node_id = action_node->next_node_id;
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[curr_node_id];

			node_context.back() = curr_node_id;

			vector<bool> inner_is_inner_scope;
			vector<int> inner_existing_scope_ids;
			vector<Action> inner_actions;

			int inner_early_exit_depth;
			int inner_early_exit_node_id;

			random_run_helper(scope_node->inner_scope_id,
							  inner_is_inner_scope,
							  inner_existing_scope_ids,
							  inner_actions,
							  scope_context,
							  node_context,
							  inner_early_exit_depth,
							  inner_early_exit_node_id,
							  exceeded_depth);

			node_context.back() = -1;

			if (inner_early_exit_depth != -1) {
				is_inner_scope.insert(is_inner_scope.end(),
					inner_is_inner_scope.begin(), inner_is_inner_scope.end());
				existing_scope_ids.insert(existing_scope_ids.end(),
					inner_existing_scope_ids.begin(), inner_existing_scope_ids.end());
				actions.insert(actions.end(),
					inner_actions.begin(), inner_actions.end());

				if (inner_early_exit_depth == 1) {
					curr_node_id = inner_early_exit_node_id;
				} else {
					early_exit_depth = inner_early_exit_depth-1;
					early_exit_node_id = inner_early_exit_node_id;
					break;
				}
			} else {
				if (randuni() < 0.6) {
					is_inner_scope.push_back(true);
					existing_scope_ids.push_back(scope_node->inner_scope_id);
					actions.push_back(Action());
				} else {
					is_inner_scope.insert(is_inner_scope.end(),
						inner_is_inner_scope.begin(), inner_is_inner_scope.end());
					existing_scope_ids.insert(existing_scope_ids.end(),
						inner_existing_scope_ids.begin(), inner_existing_scope_ids.end());
					actions.insert(actions.end(),
						inner_actions.begin(), inner_actions.end());
				}

				curr_node_id = scope_node->next_node_id;
			}
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)scope->nodes[curr_node_id];

			bool matches_context = true;
			if (branch_node->branch_scope_context.size() > scope_context.size()) {
				matches_context = false;
			} else {
				// special case first scope context
				if (branch_node->branch_scope_context[0] != scope_context.back()) {
					matches_context = false;
				} else {
					for (int c_index = 1; c_index < (int)branch_node->branch_scope_context.size(); c_index++) {
						if (branch_node->branch_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
								|| branch_node->branch_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
							matches_context = false;
							break;
						}
					}
				}
			}

			if (matches_context) {
				if (branch_node->branch_is_pass_through) {
					if (branch_node->branch_exit_depth == 0) {
						curr_node_id = branch_node->branch_next_node_id;
					} else {
						early_exit_depth = branch_node->branch_exit_depth;
						early_exit_node_id = branch_node->branch_next_node_id;
						break;
					}
				} else {
					if (randuni() < branch_node->branch_weight) {
						if (branch_node->branch_exit_depth == 0) {
							curr_node_id = branch_node->branch_next_node_id;
						} else {
							early_exit_depth = branch_node->branch_exit_depth;
							early_exit_node_id = branch_node->branch_next_node_id;
							break;
						}
					} else {
						curr_node_id = branch_node->original_next_node_id;
					}
				}
			} else {
				curr_node_id = branch_node->original_next_node_id;
			}
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)scope->nodes[curr_node_id];

			curr_node_id = fold_score_node->existing_next_node_id;
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
			// shouldn't ever hit this case
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_LOOP_FOLD) {
			LoopFoldNode* loop_fold_node = (LoopFoldNode*)scope->nodes[curr_node_id];

			curr_node_id = loop_fold_node->next_node_id;
		} else {
			// scopes->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = (PassThroughNode*)scope->nodes[curr_node_id];

			curr_node_id = pass_through_node->next_node_id;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void Solution::new_sequence(vector<bool>& is_inner_scope,
							vector<int>& existing_scope_ids,
							vector<Action>& actions,
							bool can_be_empty) {
	int sequence_length;
	geometric_distribution<int> sequence_length_geo_dist(0.5);
	if (can_be_empty) {
		sequence_length = sequence_length_geo_dist(generator);
		if (sequence_length == 0) {
			if (rand()%10 != 0) {
				sequence_length++;
			}
		}
	} else {
		sequence_length = 1 + sequence_length_geo_dist(generator);
	}
	for (int s_index = 0; s_index < sequence_length; s_index++) {
		if (rand()%2 == 0) {
			is_inner_scope.push_back(false);
			int move = rand()%3;
			actions.push_back(Action(move));

			existing_scope_ids.push_back(-1);
		} else {
			vector<bool> run_is_inner_scope;
			vector<int> run_existing_scope_ids;
			vector<Action> run_actions;
			vector<int> scope_context;
			vector<int> node_context;
			int early_exit_depth;
			int early_exit_node_id;
			bool exceeded_depth = false;
			random_run_helper(0,
							  run_is_inner_scope,
							  run_existing_scope_ids,
							  run_actions,
							  scope_context,
							  node_context,
							  early_exit_depth,
							  early_exit_node_id,
							  exceeded_depth);

			if (run_is_inner_scope.size() == 0 || exceeded_depth) {
				is_inner_scope.push_back(false);
				int move = rand()%3;
				actions.push_back(Action(move));

				existing_scope_ids.push_back(-1);
			} else {
				geometric_distribution<int> segment_length_geo_dist(0.5);
				int segment_length = 1 + segment_length_geo_dist(generator);
				if (segment_length > (int)run_is_inner_scope.size()) {
					segment_length = (int)run_is_inner_scope.size();
				}

				int start_index;
				if (segment_length == (int)run_is_inner_scope.size()) {
					start_index = 0;
				} else {
					start_index = rand()%((int)run_is_inner_scope.size() - segment_length);
				}

				is_inner_scope.insert(is_inner_scope.end(),
					run_is_inner_scope.begin() + start_index,
					run_is_inner_scope.begin() + start_index + segment_length);
				existing_scope_ids.insert(existing_scope_ids.end(),
					run_existing_scope_ids.begin() + start_index,
					run_existing_scope_ids.begin() + start_index + segment_length);
				actions.insert(actions.end(),
					run_actions.begin() + start_index,
					run_actions.begin() + start_index + segment_length);
			}
		}
	}
}

void Solution::random_run_continuation_helper(int scope_id,
											  vector<int>& scope_context,
											  vector<int>& node_context,
											  int& early_exit_depth,
											  int& early_exit_node_id) {
	early_exit_depth = -1;

	if ((int)scope_context.size() > solution->depth_limit) {
		return;
	}

	Scope* scope = this->scopes[scope_id];

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	int curr_node_id = 0;
	while (true) {
		if (curr_node_id == -1) {
			break;
		}

		if (scope->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)scope->nodes[curr_node_id];

			curr_node_id = action_node->next_node_id;
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[curr_node_id];

			node_context.back() = curr_node_id;

			int inner_early_exit_depth;
			int inner_early_exit_node_id;

			random_run_continuation_helper(scope_node->inner_scope_id,
										   scope_context,
										   node_context,
										   inner_early_exit_depth,
										   inner_early_exit_node_id);

			node_context.back() = -1;

			if (inner_early_exit_depth != -1) {
				if (inner_early_exit_depth == 1) {
					curr_node_id = inner_early_exit_node_id;
				} else {
					early_exit_depth = inner_early_exit_depth-1;
					early_exit_node_id = inner_early_exit_node_id;
					break;
				}
			} else {
				curr_node_id = scope_node->next_node_id;
			}
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)scope->nodes[curr_node_id];

			bool matches_context = true;
			if (branch_node->branch_scope_context.size() > scope_context.size()) {
				matches_context = false;
			} else {
				// special case first scope context
				if (branch_node->branch_scope_context[0] != scope_context.back()) {
					matches_context = false;
				} else {
					for (int c_index = 1; c_index < (int)branch_node->branch_scope_context.size(); c_index++) {
						if (branch_node->branch_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
								|| branch_node->branch_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
							matches_context = false;
							break;
						}
					}
				}
			}

			if (matches_context) {
				if (branch_node->branch_is_pass_through) {
					if (branch_node->branch_exit_depth == 0) {
						curr_node_id = branch_node->branch_next_node_id;
					} else {
						early_exit_depth = branch_node->branch_exit_depth;
						early_exit_node_id = branch_node->branch_next_node_id;
						break;
					}
				} else {
					if (randuni() < branch_node->branch_weight) {
						if (branch_node->branch_exit_depth == 0) {
							curr_node_id = branch_node->branch_next_node_id;
						} else {
							early_exit_depth = branch_node->branch_exit_depth;
							early_exit_node_id = branch_node->branch_next_node_id;
							break;
						}
					} else {
						curr_node_id = branch_node->original_next_node_id;
					}
				}
			} else {
				curr_node_id = branch_node->original_next_node_id;
			}
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)scope->nodes[curr_node_id];

			curr_node_id = fold_score_node->existing_next_node_id;
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
			// shouldn't ever hit this case
		} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_LOOP_FOLD) {
			LoopFoldNode* loop_fold_node = (LoopFoldNode*)scope->nodes[curr_node_id];

			curr_node_id = loop_fold_node->next_node_id;
		} else {
			// scopes->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = (PassThroughNode*)scope->nodes[curr_node_id];

			curr_node_id = pass_through_node->next_node_id;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void Solution::random_run_continuation(int explore_node_next_node_id,
									   vector<int>& scope_context,
									   vector<int>& node_context,
									   vector<int>& potential_exit_depths,
									   vector<int>& potential_next_node_ids) {
	int starting_scope_index = (int)scope_context.size()-1;
	int curr_scope_index = starting_scope_index;

	vector<int> curr_scope_context = scope_context;
	vector<int> curr_node_context = node_context;

	Scope* scope = this->scopes[scope_context[starting_scope_index]];
	int curr_node_id = explore_node_next_node_id;

	while (curr_scope_index >= 0) {
		while (true) {
			potential_exit_depths.push_back(starting_scope_index - curr_scope_index);
			potential_next_node_ids.push_back(curr_node_id);

			if (curr_node_id == -1) {
				curr_scope_index--;
				curr_scope_context.pop_back();
				curr_node_context.pop_back();
				if (curr_scope_index >= 0) {
					scope = this->scopes[scope_context[curr_scope_index]];
					ScopeNode* context_scope_node = (ScopeNode*)scope->nodes[node_context[curr_scope_index]];
					curr_node_context.back() = -1;
					curr_node_id = context_scope_node->next_node_id;
				}
				break;
			}

			if (scope->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)scope->nodes[curr_node_id];
				curr_node_id = action_node->next_node_id;
			} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)scope->nodes[curr_node_id];

				curr_node_context.back() = curr_node_id;

				int inner_early_exit_depth;
				int inner_early_exit_node_id;

				random_run_continuation_helper(scope_node->inner_scope_id,
											   curr_scope_context,
											   curr_node_context,
											   inner_early_exit_depth,
											   inner_early_exit_node_id);

				curr_node_context.back() = -1;

				if (inner_early_exit_depth != -1) {
					if (inner_early_exit_depth == 1) {
						curr_node_id = inner_early_exit_node_id;
					} else {
						curr_scope_index -= (inner_early_exit_depth-1);
						for (int e_index = 0; e_index < inner_early_exit_depth-1; e_index++) {
							curr_scope_context.pop_back();
							curr_node_context.pop_back();
						}
						scope = this->scopes[scope_context[curr_scope_index]];
						curr_node_context.back() = -1;
						curr_node_id = inner_early_exit_node_id;
						break;
					}
				} else {
					curr_node_id = scope_node->next_node_id;
				}
			} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)scope->nodes[curr_node_id];

				bool matches_context = true;
				if (branch_node->branch_scope_context.size() > scope_context.size()) {
					matches_context = false;
				} else {
					// special case first scope context
					if (branch_node->branch_scope_context[0] != scope_context.back()) {
						matches_context = false;
					} else {
						for (int c_index = 1; c_index < (int)branch_node->branch_scope_context.size(); c_index++) {
							if (branch_node->branch_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
									|| branch_node->branch_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
								matches_context = false;
								break;
							}
						}
					}
				}

				if (matches_context) {
					if (branch_node->branch_is_pass_through) {
						if (branch_node->branch_exit_depth == 0) {
							curr_node_id = branch_node->branch_next_node_id;
						} else {
							curr_scope_index -= branch_node->branch_exit_depth;
							for (int e_index = 0; e_index < branch_node->branch_exit_depth; e_index++) {
								curr_scope_context.pop_back();
								curr_node_context.pop_back();
							}
							scope = this->scopes[scope_context[curr_scope_index]];
							curr_node_context.back() = -1;
							curr_node_id = branch_node->branch_next_node_id;
							break;
						}
					} else {
						if (randuni() < branch_node->branch_weight) {
							if (branch_node->branch_exit_depth == 0) {
								curr_node_id = branch_node->branch_next_node_id;
							} else {
								curr_scope_index -= branch_node->branch_exit_depth;
								for (int e_index = 0; e_index < branch_node->branch_exit_depth; e_index++) {
									curr_scope_context.pop_back();
									curr_node_context.pop_back();
								}
								scope = this->scopes[scope_context[curr_scope_index]];
								curr_node_context.back() = -1;
								curr_node_id = branch_node->branch_next_node_id;
								break;
							}
						} else {
							curr_node_id = branch_node->original_next_node_id;
						}
					}
				} else {
					curr_node_id = branch_node->original_next_node_id;
				}
			} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SCORE) {
				FoldScoreNode* fold_score_node = (FoldScoreNode*)scope->nodes[curr_node_id];

				curr_node_id = fold_score_node->existing_next_node_id;
			} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
				// shouldn't ever hit this case
			} else if (scope->nodes[curr_node_id]->type == NODE_TYPE_LOOP_FOLD) {
				LoopFoldNode* loop_fold_node = (LoopFoldNode*)scope->nodes[curr_node_id];

				curr_node_id = loop_fold_node->next_node_id;
			} else {
				// scopes->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
				PassThroughNode* pass_through_node = (PassThroughNode*)scope->nodes[curr_node_id];

				curr_node_id = pass_through_node->next_node_id;
			}
		}
	}
}

void Solution::backtrack_for_loop_helper(ScopeHistory* scope_history,
										 int& remaining_length,
										 vector<bool>& is_inner_scope,
										 vector<int>& existing_scope_ids,
										 vector<Action>& actions) {
	for (int i_index = (int)scope_history->node_histories.size()-1; i_index >= 0; i_index--) {
		for (int h_index = (int)scope_history->node_histories[i_index].size()-1; h_index >= 0; h_index--) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)scope_history->node_histories[i_index][h_index]->node;

				if (action_node->action.move == ACTION_START) {
					remaining_length = 0;
					return;
				} else {
					is_inner_scope.push_back(false);
					existing_scope_ids.push_back(-1);
					actions.push_back(action_node->action);
					remaining_length--;
					if (remaining_length == 0) {
						return;
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				if (!scope_node_history->inner_is_early_exit
						&& randuni() < 0.6) {
					ScopeNode* scope_node = (ScopeNode*)scope_history->node_histories[i_index][h_index]->node;

					is_inner_scope.push_back(true);
					existing_scope_ids.push_back(scope_node->inner_scope_id);
					actions.push_back(Action());
					remaining_length--;
					if (remaining_length == 0) {
						return;
					}
				} else {
					backtrack_for_loop_helper(scope_node_history->inner_scope_history,
											  remaining_length,
											  is_inner_scope,
											  existing_scope_ids,
											  actions);

					if (remaining_length == 0) {
						return;
					}
				}
			}
		}
	}
}

void Solution::backtrack_for_loop(vector<ScopeHistory*>& context_histories,
								  vector<bool>& is_inner_scope,
								  vector<int>& existing_scope_ids,
								  vector<Action>& actions) {
	geometric_distribution<int> loop_length_geo_dist(0.3);
	int remaining_length = 1 + loop_length_geo_dist(generator);

	for (int c_index = (int)context_histories.size()-1; c_index >= 0; c_index--) {
		for (int i_index = (int)context_histories[c_index]->node_histories.size()-1; i_index >= 0; i_index--) {
			for (int h_index = (int)context_histories[c_index]->node_histories[i_index].size()-1; h_index >= 0; h_index--) {
				if (context_histories[c_index]->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)context_histories[c_index]->node_histories[i_index][h_index]->node;

					if (action_node->action.move == ACTION_START) {
						remaining_length = 0;
						return;
					} else {
						is_inner_scope.push_back(false);
						existing_scope_ids.push_back(-1);
						actions.push_back(action_node->action);
						remaining_length--;
						if (remaining_length == 0) {
							return;
						}
					}
				} else if (context_histories[c_index]->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)context_histories[c_index]->node_histories[i_index][h_index];
					if (!scope_node_history->inner_is_early_exit
							&& randuni() < 0.6) {
						ScopeNode* scope_node = (ScopeNode*)context_histories[c_index]->node_histories[i_index][h_index]->node;

						is_inner_scope.push_back(true);
						existing_scope_ids.push_back(scope_node->inner_scope_id);
						actions.push_back(Action());
						remaining_length--;
						if (remaining_length == 0) {
							return;
						}
					} else {
						backtrack_for_loop_helper(scope_node_history->inner_scope_history,
												  remaining_length,
												  is_inner_scope,
												  existing_scope_ids,
												  actions);

						if (remaining_length == 0) {
							return;
						}
					}
				}
			}
		}
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->average_score << endl;

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes[s_index]->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->max_depth << endl;
}
