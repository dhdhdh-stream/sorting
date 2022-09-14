#include "start_scope.h"

#include <iostream>

#include "candidate_start_branch.h"
#include "candidate_start_replace.h"
#include "definitions.h"
#include "jump_scope.h"
#include "solution_node_action.h"
#include "solution_node_empty.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

StartScope::StartScope() {
	this->node_type = NODE_TYPE_START_SCOPE;

	this->score_network = new ScoreNetwork(this->local_state_sizes);	// empty local_state_sizes

	this->average_misguess = 0.5;
	this->explore_weight = 0.0;

	this->explore_state = EXPLORE_STATE_EXPLORE;

	this->explore_small_jump_score_network = NULL;
	this->explore_small_no_jump_score_network = NULL;

	this->is_temp_node = false;
}

StartScope::StartScope(std::vector<int>& scope_states,
					   std::vector<int>& scope_locations,
					   std::ifstream& save_file) {
	this->node_type = NODE_TYPE_START_SCOPE;

	string score_network_name = "../saves/nns/start_score_" + to_string(id) + ".txt";
	ifstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network = new ScoreNetwork(score_network_save_file);
	score_network_save_file.close();

	string average_misguess_line;
	getline(save_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);

	string explore_weight_line;
	getline(save_file, explore_weight_line);
	this->explore_weight = stof(explore_weight_line);

	string path_size_line;
	getline(save_file, path_size_line);
	int path_size = stoi(path_size_line);
	this->path.reserve(path_size);
	scope_states.push_back(-1);
	scope_locations.push_back(0);
	for (int n_index = 0; n_index < path_size; n_index++) {
		string node_type_line;
		getline(save_file, node_type_line);
		int node_type = stoi(node_type_line);
		SolutionNode* new_node;
		if (node_type == NODE_TYPE_EMPTY) {
			new_node = new SolutionNodeEmpty(scope_states,
											 scope_locations,
											 save_file);
		} else if (node_type == NODE_TYPE_ACTION) {
			new_node = new SolutionNodeAction(scope_states,
											  scope_locations,
											  save_file);
		} else {
			// node_type == NODE_TYPE_JUMP_SCOPE
			new_node = new JumpScope(scope_states,
									 scope_locations,
									 save_file);
		}
		this->path.push_back(new_node);
	}

	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		this->path[n_index]->parent_scope = this;
		this->path[n_index]->scope_location = -1;
		this->path[n_index]->scope_child_index = -1;
		this->path[n_index]->scope_node_index = n_index;
	}
	for (int n_index = 0; n_index < (int)this->path.size()-1; n_index++) {
		this->path[n_index]->next = this->path[n_index+1];
	}
	if (this->path.size() > 0) {
		this->path[this->path.size()-1]->next = this;
	}

	scope_states.pop_back();
	scope_locations.pop_back();

	this->explore_state = EXPLORE_STATE_EXPLORE;

	this->explore_small_jump_score_network = NULL;
	this->explore_small_no_jump_score_network = NULL;

	this->is_temp_node = false;
}

StartScope::~StartScope() {
	delete this->score_network;

	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		delete this->path[n_index];
	}
}

SolutionNode* StartScope::re_eval(Problem& problem,
								  double& predicted_score,
								  vector<vector<double>>& state_vals,
								  vector<SolutionNode*>& scopes,
								  vector<int>& scope_states,
								  vector<ReEvalStepHistory>& instance_history,
								  vector<AbstractNetworkHistory*>& network_historys) {
	if (scopes.size() == 0) {
		// entering scope
		vector<double> obs;
		obs.push_back(problem.get_observation());

		this->score_network->mtx.lock();
		this->score_network->activate(state_vals,
									  obs,
									  predicted_score,
									  network_historys);
		predicted_score += this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		vector<double> empty_scope_states;
		state_vals.push_back(empty_scope_states);

		scopes.push_back(this);
		scope_states.push_back(-1);

		instance_history.push_back(ReEvalStepHistory(this,
													 predicted_score,
													 START_SCOPE_STATE_ENTER));

		if (this->path.size() > 0) {
			return this->path[0];
		}
	}

	// exiting scope
	state_vals.pop_back();

	scopes.pop_back();
	scope_states.pop_back();

	instance_history.push_back(ReEvalStepHistory(this,
												 0.0,
												 START_SCOPE_STATE_EXIT));

	return NULL;
}

void StartScope::re_eval_backprop(double score,
								  vector<vector<double>>& state_errors,
								  vector<ReEvalStepHistory>& instance_history,
								  vector<AbstractNetworkHistory*>& network_historys) {
	if (instance_history.back().scope_state == START_SCOPE_STATE_EXIT) {
		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else {
		// instance_history.back().scope_state == START_SCOPE_STATE_ENTER
		AbstractNetworkHistory* network_history = network_historys.back();

		this->score_network->mtx.lock();

		network_history->reset_weights();

		double predicted_score = this->score_network->previous_predicted_score_input->acti_vals[0]
			+ this->score_network->output->acti_vals[0];

		double misguess;
		vector<double> errors;
		if (score == 1.0) {
			if (predicted_score < 1.0) {
				errors.push_back(1.0 - predicted_score);
				misguess = abs(1.0 - predicted_score);
			} else {
				errors.push_back(0.0);
				misguess = 0.0;
			}
		} else {
			if (predicted_score > 0.0) {
				errors.push_back(0.0 - predicted_score);
				misguess = abs(0.0 - predicted_score);
			} else {
				errors.push_back(0.0);
				misguess = 0.0;
			}
		}
		this->score_network->backprop(errors);

		this->score_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();

		this->average_misguess = 0.9999*this->average_misguess + 0.0001*misguess;

		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

SolutionNode* StartScope::explore(Problem& problem,
								  double& predicted_score,
								  vector<vector<double>>& state_vals,
								  vector<SolutionNode*>& scopes,
								  vector<int>& scope_states,
								  vector<int>& scope_locations,
								  IterExplore*& iter_explore,
								  vector<ExploreStepHistory>& instance_history,
								  vector<AbstractNetworkHistory*>& network_historys,
								  bool& abandon_instance) {
	if (iter_explore != NULL
			&& iter_explore->explore_node == this
			&& scopes.back() == NULL) {
		scopes.pop_back();
		scope_states.pop_back();
		scope_locations.pop_back();
		// can't be loop

		// top layer, no state networks to be learned

		scope_locations.back() = this->jump_end_non_inclusive_index;

		instance_history.push_back(ExploreStepHistory(this,
													  false,
													  0.0,
													  -1,
													  -1,
													  true));

		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			if (iter_explore->jump_end_non_inclusive_index < (int)this->path.size()) {
				return this->path[iter_explore->jump_end_non_inclusive_index];
			}
		} else {
			if (this->jump_end_non_inclusive_index < (int)this->path.size()) {
				return this->path[this->jump_end_non_inclusive_index];
			}
		}
	}

	if (scopes.size() == 0) {
		// entering scope
		vector<double> obs;
		obs.push_back(problem.get_observation());

		this->score_network->mtx.lock();
		this->score_network->activate(state_vals,
									  obs,
									  predicted_score);
		predicted_score += this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		if (randuni() < this->explore_weight) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				int jump_end_non_inclusive_index = rand()%((int)this->path.size()+1);

				vector<SolutionNode*> explore_path;
				if (jump_end_non_inclusive_index == 0) {
					new_random_path(explore_path,
									false);
				} else {
					new_random_path(explore_path,
									true);
				}
				explore_path[explore_path.size()-1]->next = this;

				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_EXPLORE);
				iter_explore->explore_path = explore_path;
				iter_explore->jump_end_non_inclusive_index = jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FLAT);
				iter_explore->jump_end_non_inclusive_index = this->jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FLAT);
				iter_explore->jump_end_non_inclusive_index = this->jump_end_non_inclusive_index;
			}
		}

		vector<double> empty_scope_states;
		state_vals.push_back(empty_scope_states);

		scopes.push_back(this);
		scope_states.push_back(-1);
		scope_locations.push_back(0);

		instance_history.push_back(ExploreStepHistory(this,
													  false,
													  0.0,
													  START_SCOPE_STATE_ENTER,
													  -1,
													  false));

		if (iter_explore != NULL) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				scopes.push_back(NULL);
				scope_states.push_back(-1);
				scope_locations.push_back(-1);
				return iter_explore->explore_path[0];
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
				vector<double> obs;
				obs.push_back(problem.get_observation());
				if (rand()%3 != 0) {
					this->explore_small_jump_score_network->mtx.lock();
					this->explore_small_jump_score_network->activate(obs, network_historys);
					this->explore_small_jump_score_network->mtx.unlock();

					scopes.push_back(NULL);
					scope_states.push_back(-1);
					scope_locations.push_back(-1);
					return this->explore_path[0];
				} else {
					this->explore_small_no_jump_score_network->mtx.lock();
					this->explore_small_no_jump_score_network->activate(obs, network_historys);
					this->explore_small_no_jump_score_network->mtx.unlock();

					if (this->path.size() > 0) {
						return this->path[0];
					}
				}
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
				vector<double> obs;
				obs.push_back(problem.get_observation());

				this->explore_small_jump_score_network->mtx.lock();
				this->explore_small_jump_score_network->activate(obs);
				double jump_score = this->explore_small_jump_score_network->output->acti_vals[0];
				this->explore_small_jump_score_network->mtx.unlock();

				this->explore_small_no_jump_score_network->mtx.lock();
				this->explore_small_no_jump_score_network->activate(obs);
				double no_jump_score = this->explore_small_no_jump_score_network->output->acti_vals[0];
				this->explore_small_no_jump_score_network->mtx.unlock();

				if (jump_score > no_jump_score) {
					if (rand()%2 == 0) {
						scopes.push_back(NULL);
						scope_states.push_back(-1);
						scope_locations.push_back(-1);
						instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE;
						return this->explore_path[0];
					} else {
						instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE;
						if (this->path.size() > 0) {
							return this->path[0];
						}
					}
				} else {
					if (rand()%2 == 0) {
						scopes.push_back(NULL);
						scope_states.push_back(-1);
						scope_locations.push_back(-1);
						instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE;
						return this->explore_path[0];
					} else {
						instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE;
						if (this->path.size() > 0) {
							return this->path[0];
						}
					}
				}
			}
		}

		if (this->path.size() > 0) {
			return this->path[0];
		}
	}

	// exiting scope
	state_vals.pop_back();

	scopes.pop_back();
	scope_states.pop_back();
	scope_locations.pop_back();

	instance_history.push_back(ExploreStepHistory(this,
												  false,
												  0.0,
												  START_SCOPE_STATE_EXIT,
												  -1,
												  false));

	return NULL;
}

void StartScope::explore_backprop(double score,
								  vector<vector<double>>& state_errors,
								  IterExplore*& iter_explore,
								  vector<ExploreStepHistory>& instance_history,
								  vector<AbstractNetworkHistory*>& network_historys) {
	if (instance_history.back().is_explore_callback) {
		// iter_explore->explore_node == this

		// do nothing

		instance_history.pop_back();
		return;
	}

	if (instance_history.back().scope_state == START_SCOPE_STATE_EXIT) {
		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else {
		// instance_history.back()->scope_state == START_SCOPE_STATE_ENTER
		if (iter_explore != NULL
				&& iter_explore->explore_node == this) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				// do nothing
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
				if (network_historys.back()->network == this->explore_small_jump_score_network) {
					backprop_explore_small_jump_score_network(score,
															  network_historys);
				} else {
					backprop_explore_small_no_jump_score_network(score,
																 network_historys);
				}
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
				if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE) {
					this->explore_explore_explore_score += score;
					this->explore_explore_explore_count++;
				} else if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE) {
					this->explore_explore_no_explore_score += score;
					this->explore_explore_no_explore_count++;
				} else if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE) {
					this->explore_no_explore_explore_score += score;
					this->explore_no_explore_explore_count++;
				} else {
					// instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE
					this->explore_no_explore_no_explore_score += score;
					this->explore_no_explore_no_explore_count++;
				}
			}
		}

		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

void StartScope::explore_increment(double score,
								   IterExplore*& iter_explore) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		if (score == 1.0) {
			this->explore_path = iter_explore->explore_path;
			for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
				if (this->explore_path[n_index]->node_type != NODE_TYPE_ACTION
						&& this->explore_path[n_index]->node_type != NODE_TYPE_EMPTY) {
					SolutionNode* deep_copy = this->explore_path[n_index]->deep_copy(0);
					deep_copy->set_is_temp_node(true);
					if (n_index > 0) {
						this->explore_path[n_index-1]->next = deep_copy;
					}
					deep_copy->next = this->explore_path[n_index]->next;
					this->explore_path[n_index] = deep_copy;
				}

				vector<int> start_scope_local_state_sizes;
				start_scope_local_state_sizes.push_back(0);
				this->explore_path[n_index]->initialize_local_state(start_scope_local_state_sizes);
			}

			this->jump_end_non_inclusive_index = iter_explore->jump_end_non_inclusive_index;

			this->explore_small_jump_score_network = new Network(1, 4, 1);
			this->explore_small_no_jump_score_network = new Network(1, 4, 1);

			this->explore_state = EXPLORE_STATE_LEARN_FLAT;
			this->explore_iter_index = 0;
		} else {
			for (int n_index = 0; n_index < (int)iter_explore->explore_path.size(); n_index++) {
				// non-recursive, only need to check top layer
				if (iter_explore->explore_path[n_index]->node_type == NODE_TYPE_ACTION
						|| iter_explore->explore_path[n_index]->node_type == NODE_TYPE_EMPTY) {
					delete iter_explore->explore_path[n_index];
				}
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 3000000) {
			this->explore_state = EXPLORE_STATE_MEASURE_FLAT;
			this->explore_iter_index = 0;

			this->explore_explore_explore_count = 0;
			this->explore_explore_explore_score = 0.0;
			this->explore_explore_no_explore_count = 0;
			this->explore_explore_no_explore_score = 0.0;
			this->explore_no_explore_explore_count = 0;
			this->explore_no_explore_explore_score = 0.0;
			this->explore_no_explore_no_explore_count = 0;
			this->explore_no_explore_no_explore_score = 0.0;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 100000) {
			for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
				if (this->explore_path[n_index]->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)this->explore_path[n_index];
					cout << node_action->action.to_string() << endl;
				} else {
					cout << "C" << endl;
				}
			}
			cout << endl;

			bool branch_better = false;
			bool branch_not_worse = false;
			double explore_explore_score = 0.0;
			double explore_no_explore_score = 0.0;
			if (this->explore_explore_explore_count > 0
					&& this->explore_explore_no_explore_count > 0) {
				explore_explore_score = this->explore_explore_explore_score/this->explore_explore_explore_count;
				explore_no_explore_score = this->explore_explore_no_explore_score/this->explore_explore_no_explore_count;
				cout << "explore_explore_score: " << explore_explore_score << endl;
				cout << "explore_no_explore_score: " << explore_no_explore_score << endl;
				if (explore_explore_score > (explore_no_explore_score + 0.03*(1.0 - solution->average_score))) {
					branch_better = true;
					branch_not_worse = true;
				} else if (explore_explore_score > 0.97*explore_no_explore_score) {
					branch_not_worse = true;
				}
			}

			bool can_replace = false;
			if (this->explore_no_explore_explore_count > 0
					&& this->explore_no_explore_no_explore_count > 0) {
				double no_explore_explore_score = this->explore_no_explore_explore_score/this->explore_no_explore_explore_count;
				double no_explore_no_explore_score = this->explore_no_explore_no_explore_score/this->explore_no_explore_no_explore_count;
				cout << "no_explore_explore_score: " << no_explore_explore_score << endl;
				cout << "no_explore_no_explore_score: " << no_explore_no_explore_score << endl;
				if (no_explore_explore_score > 0.97*no_explore_no_explore_score) {
					can_replace = true;
				}
			} else {
				can_replace = true;
			}

			if (branch_better) {
				if (can_replace) {
					// replace
					double original_score = (this->explore_explore_no_explore_score+this->explore_no_explore_no_explore_score)
						/(this->explore_explore_no_explore_count+this->explore_no_explore_no_explore_count);
					double replace_score = (this->explore_explore_explore_score+this->explore_no_explore_explore_score)
						/(this->explore_explore_explore_count+this->explore_no_explore_explore_count);

					double score_increase = replace_score - original_score;

					CandidateStartReplace* new_candidate = new CandidateStartReplace(
						this,
						EXPLORE_REPLACE_TYPE_SCORE,
						score_increase,
						0.0,
						this->jump_end_non_inclusive_index,
						this->explore_path);
					solution->candidates.push_back(new_candidate);
					cout << "CandidateStartReplace added" << endl;

					this->explore_path.clear();

					delete this->explore_small_jump_score_network;
					this->explore_small_jump_score_network = NULL;
					delete this->explore_small_no_jump_score_network;
					this->explore_small_no_jump_score_network = NULL;
				} else {
					// branch
					double branch_percent = (this->explore_explore_explore_count+this->explore_explore_no_explore_count)/100000;
					double score_increase = explore_explore_score - explore_no_explore_score;

					CandidateStartBranch* new_candidate = new CandidateStartBranch(
						this,
						branch_percent,
						score_increase,
						this->jump_end_non_inclusive_index,
						this->explore_path,
						this->explore_small_jump_score_network,
						this->explore_small_no_jump_score_network);
					solution->candidates.push_back(new_candidate);
					cout << "CandidateStartBranch added" << endl;

					this->explore_path.clear();

					this->explore_small_jump_score_network = NULL;
					this->explore_small_no_jump_score_network = NULL;
				}
			} else if (((this->explore_explore_explore_count == 0 && this->explore_explore_no_explore_count == 0)
					|| branch_not_worse) && can_replace) {
				vector<SolutionNode*> replacement_path;
				replacement_path.push_back(this);	// always include self for something to compare against
				for (int n_index = 0; n_index < this->jump_end_non_inclusive_index; n_index++) {
					replacement_path.push_back(this->path[n_index]);
				}

				double min_replacement_path_misguess = numeric_limits<double>::max();
				for (int n_index = 0; n_index < (int)replacement_path.size(); n_index++) {
					replacement_path[n_index]->get_min_misguess(min_replacement_path_misguess);
				}
				cout << "min_replacement_path_misguess: " << min_replacement_path_misguess << endl;

				double min_new_path_misguess = numeric_limits<double>::max();
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->get_min_misguess(min_new_path_misguess);
				}
				cout << "min_new_path_misguess: " << min_new_path_misguess << endl;

				// TODO: add if equal, choose shorter path
				if (min_new_path_misguess < 0.97*min_replacement_path_misguess) {
					// replace
					double info_gain = 1.0 - min_new_path_misguess/min_replacement_path_misguess;

					CandidateStartReplace* new_candidate = new CandidateStartReplace(
						this,
						EXPLORE_REPLACE_TYPE_INFO,
						0.0,
						info_gain,
						this->jump_end_non_inclusive_index,
						this->explore_path);
					solution->candidates.push_back(new_candidate);
					cout << "CandidateStartReplace added" << endl;

					this->explore_path.clear();

					delete this->explore_small_jump_score_network;
					this->explore_small_jump_score_network = NULL;
					delete this->explore_small_no_jump_score_network;
					this->explore_small_no_jump_score_network = NULL;
				} else {
					// abandon
					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						delete this->explore_path[n_index];
					}
					this->explore_path.clear();

					delete this->explore_small_jump_score_network;
					this->explore_small_jump_score_network = NULL;
					delete this->explore_small_no_jump_score_network;
					this->explore_small_no_jump_score_network = NULL;
				}
			} else {
				// abandon
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					delete this->explore_path[n_index];
				}
				this->explore_path.clear();

				delete this->explore_small_jump_score_network;
				this->explore_small_jump_score_network = NULL;
				delete this->explore_small_no_jump_score_network;
				this->explore_small_no_jump_score_network = NULL;
			}

			this->explore_state = EXPLORE_STATE_EXPLORE;
		}
	}
}

void StartScope::re_eval_increment() {
	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		this->path[n_index]->re_eval_increment();
	}
}

SolutionNode* StartScope::deep_copy(int inclusive_start_layer) {
	// should not happen
	return NULL;
}

void StartScope::set_is_temp_node(bool is_temp_node) {
	// should not happen
}

void StartScope::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	// should not happen
}

void StartScope::setup_flat(vector<int>& loop_scope_counts,
							int& curr_index,
							SolutionNode* explore_node) {
	// should not happen
}

void StartScope::setup_new_state(SolutionNode* explore_node,
								 int new_state_size) {
	// should not happen
}

void StartScope::get_min_misguess(double& min_misguess) {
	if (this->average_misguess < min_misguess) {
		min_misguess = this->average_misguess;
	}

	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		this->path[n_index]->get_min_misguess(min_misguess);
	}
}

void StartScope::cleanup_explore(SolutionNode* explore_node) {
	// should not happen
}

void StartScope::collect_new_state_networks(SolutionNode* explore_node,
											vector<SolutionNode*>& existing_nodes,
											vector<Network*>& new_state_networks) {
	// should not happen
}

void StartScope::insert_scope(int layer,
							  int new_state_size) {
	// should not happen
}

void StartScope::reset_explore() {
	this->explore_state = EXPLORE_STATE_EXPLORE;

	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
	this->explore_path.clear();

	if (this->explore_small_jump_score_network != NULL) {
		delete this->explore_small_jump_score_network;
		this->explore_small_jump_score_network = NULL;
	}
	if (this->explore_small_no_jump_score_network != NULL) {
		delete this->explore_small_no_jump_score_network;
		this->explore_small_no_jump_score_network = NULL;
	}

	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		this->path[n_index]->reset_explore();
	}
}

void StartScope::save(std::vector<int>& scope_states,
					  std::vector<int>& scope_locations,
					  std::ofstream& save_file) {
	string score_network_name = "../saves/nns/start_score_" + to_string(id) + ".txt";
	ofstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->average_misguess << endl;
	save_file << this->explore_weight << endl;

	save_file << this->path.size() << endl;
	scope_states.push_back(-1);
	scope_locations.push_back(0);
	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		save_file << this->path[n_index]->node_type << endl;
		this->path[n_index]->save(scope_states,
								  scope_locations,
								  save_file);
	}

	scope_states.pop_back();
	scope_locations.pop_back();
}

void StartScope::save_for_display(std::ofstream& save_file) {
	save_file << this->explore_weight << endl;

	save_file << this->path.size() << endl;
	for (int n_index = 0; n_index < (int)this->path.size(); n_index++) {
		save_file << this->path[n_index]->node_type << endl;
		this->path[n_index]->save_for_display(save_file);
	}
}
