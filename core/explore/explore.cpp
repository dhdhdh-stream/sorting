#include "explore.h"

#include <iostream>
#include <random>

#include "definitions.h"
#include "utilities.h"

using namespace std;

Explore::Explore(SolutionNode* parent) {
	this->parent = parent;

	this->state = EXPLORE_STATE_EXPLORE;
	this->current_id = (unsigned)time(NULL);
	this->candidate_iter = 0;
	this->best_score = numeric_limits<double>::lowest();

	this->learn_scores_network = NULL;
	
	this->best_loop = NULL;
	this->current_loop = NULL;
}

Explore::~Explore() {
	if (this->learn_scores_network != NULL) {
		delete this->learn_scores_network;
	}
	
	if (this->best_loop != NULL) {
		delete this->best_loop;
	}
	if (this->current_loop != NULL) {
		delete this->current_loop;
	}
}

void Explore::cycle() {
	ExploreNode* curr_node = this->root;

	
}

void Explore::process(Problem* p,
					  vector<double>* observations,
					  double& score,
					  bool save_for_display,
					  vector<Action>* raw_actions) {
	this->mtx.lock();
	int explore_state = this->state;
	int explore_id = this->current_id;
	int explore_candidate_iter = this->candidate_iter;
	this->mtx.unlock();

	int path_length = (int)observations->size();

	if (explore_state == EXPLORE_STATE_EXPLORE) {
		if (rand()%2 == 0) {
			vector<int> compound_actions_tried;

			vector<Action> new_path_candidate;

			geometric_distribution<int> seq_length_dist(0.2);
			int seq_length = 1+seq_length_dist(generator);

			normal_distribution<double> write_val_dist(0.0, 2.0);
			for (int i = 0; i < seq_length; i++) {
				if (action_dictionary->actions.size() > 0 && rand()%3 == 0) {
					int compound_index = action_dictionary->select_compound_action();
					compound_actions_tried.push_back(compound_index);
					Action a(COMPOUND, compound_index);
					new_path_candidate.push_back(a);
				} else {
					Action a(write_val_dist(generator), rand()%3);
					new_path_candidate.push_back(a);
				}
			}

			for (int i = 0; i < (int)new_path_candidate.size(); i++) {
				p->perform_action(new_path_candidate[i],
								  observations,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			if (score == 1.0) {
				this->mtx.lock();
				if (explore_state == this->state &&
						explore_id == this->current_id &&
						explore_candidate_iter == this->candidate_iter) {
					this->state = NEW_PATH_STATE_MEASURE_AVERAGE;
					
					this->current_new_path = new_path_candidate;

					this->iter_index = 0;
					this->average_score = 0.0;
				}
				this->mtx.unlock();

				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					action_dictionary->num_success[compound_actions_tried[c_index]]++;
					action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			} else {
				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			}
		} else {
			vector<int> compound_actions_tried;
			normal_distribution<double> write_val_dist(0.0, 2.0);

			vector<Action> front_actions;
			geometric_distribution<int> front_size_dist(0.5);
			int front_size = front_size_dist(generator);
			for (int i = 0; i < front_size; i++) {
				if (action_dictionary->actions.size() > 0 && rand()%3 == 0) {
					int compound_index = action_dictionary->select_compound_action();
					compound_actions_tried.push_back(compound_index);
					Action a(COMPOUND, compound_index);
					front_actions.push_back(a);
				} else {
					Action a(write_val_dist(generator), rand()%3);
					front_actions.push_back(a);
				}
			}

			vector<Action> loop_actions;
			geometric_distribution<int> loop_size_dist(0.5);
			int loop_size = 1+loop_size_dist(generator);
			for (int i = 0; i < loop_size; i++) {
				if (action_dictionary->actions.size() > 0 && rand()%3 == 0) {
					int compound_index = action_dictionary->select_compound_action();
					compound_actions_tried.push_back(compound_index);
					Action a(COMPOUND, compound_index);
					loop_actions.push_back(a);
				} else {
					Action a(write_val_dist(generator), rand()%3);
					loop_actions.push_back(a);
				}
			}

			vector<Action> back_actions;
			geometric_distribution<int> back_size_dist(0.5);
			int back_size = back_size_dist(generator);
			for (int i = 0; i < back_size; i++) {
				if (action_dictionary->actions.size() > 0 && rand()%3 == 0) {
					int compound_index = action_dictionary->select_compound_action();
					compound_actions_tried.push_back(compound_index);
					Action a(COMPOUND, compound_index);
					back_actions.push_back(a);
				} else {
					Action a(write_val_dist(generator), rand()%3);
					back_actions.push_back(a);
				}
			}

			int iterations = 3 + rand()%3;

			for (int f_index = 0; f_index < (int)front_actions.size(); f_index++) {
				p->perform_action(front_actions[f_index],
								  observations,
								  save_for_display,
								  raw_actions);
			}
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				for (int l_index = 0; l_index < (int)loop_actions.size(); l_index++) {
					p->perform_action(loop_actions[l_index],
									  observations,
									  save_for_display,
									  raw_actions);
				}
			}
			for (int b_index = 0; b_index < (int)back_actions.size(); b_index++) {
				p->perform_action(back_actions[b_index],
								  observations,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			if (score == 1.0) {
				this->mtx.lock();
				if (explore_state == this->state &&
						explore_id == this->current_id &&
						explore_candidate_iter == this->candidate_iter) {
					this->state = LOOP_STATE_CHECK_DIFFERENT_ITERS;

					this->current_front_actions = front_actions;
					this->current_loop_actions = loop_actions;
					this->current_back_actions = back_actions;

					this->iter_index = 0;

					for (int i = 0; i < 7; i++) {
						this->different_iters_success_counts[i] = 0;
					}
				}
				this->mtx.unlock();

				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					action_dictionary->num_success[compound_actions_tried[c_index]]++;
					action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			} else {
				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			}
		}
	} else if (explore_state == NEW_PATH_STATE_MEASURE_AVERAGE) {
		for (int i = 0; i < (int)this->current_new_path.size(); i++) {
			p->perform_action(this->current_new_path[i],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		score = p->score_result();

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;
			this->average_score += score;

			if (this->iter_index == 100000) {
				this->state = NEW_PATH_STATE_LEARN_SCORES;
				this->average_score /= 100000.0;

				int input_size = path_length;
				for (int a_index = 0; a_index < (int)this->current_new_path.size(); a_index++) {
					input_size += calculate_action_path_length(this->current_new_path[a_index]);
				}
				int network_size = 2*input_size*(3+input_size);
				this->learn_scores_network = new Network(input_size, network_size, 1);
			}
		}
		this->mtx.unlock();
	} else if (explore_state == NEW_PATH_STATE_LEARN_SCORES) {
		for (int i = 0; i < (int)this->current_new_path.size(); i++) {
			p->perform_action(this->current_new_path[i],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		score = p->score_result();

		this->learn_scores_network->mtx.lock();
		this->learn_scores_network->activate(*observations);
		vector<double> errors;
		if (score == 1.0) {
			if (this->learn_scores_network->val_val->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->learn_scores_network->val_val->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->learn_scores_network->val_val->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->learn_scores_network->val_val->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->learn_scores_network->backprop(errors);
		this->learn_scores_network->increment();
		this->learn_scores_network->mtx.unlock();

		if (this->learn_scores_network->epoch == 10000) {
			this->mtx.lock();
			if (explore_state == this->state &&
					explore_id == this->current_id &&
					explore_candidate_iter == this->candidate_iter) {
				this->state = NEW_PATH_STATE_MEASURE_INFORMATION;

				this->iter_index = 0;

				this->think_good_and_good = 0.0;
				this->think_good_but_bad = 0.0;
			}
			this->mtx.unlock();
		}
	} else if (explore_state == NEW_PATH_STATE_MEASURE_INFORMATION) {
		for (int i = 0; i < (int)this->current_new_path.size(); i++) {
			p->perform_action(this->current_new_path[i],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		score = p->score_result();

		this->learn_scores_network->mtx.lock();
		this->learn_scores_network->activate(*observations);
		double think = this->learn_scores_network->val_val->acti_vals[0];
		this->learn_scores_network->mtx.unlock();

		think = max(min(think, 1.0), 0.0);

		if (score > this->average_score) {
			if (think > this->average_score) {
				this->think_good_and_good += (score - this->average_score) \
					* (think - this->average_score);
			}
		} else {
			if (think > this->average_score) {
				this->think_good_but_bad += (this->average_score - score) \
					* (think - this->average_score);
			}
		}

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;

			if (this->iter_index == 100000) {
				double information = this->think_good_and_good - this->think_good_but_bad;
				if (information > this->best_score) {
					this->best_score = information;

					this->best_candidate_type = EXPLORE_TYPE_NEW_PATH;
					this->best_new_path = this->current_new_path;
				}

				cout << this->parent->node_index << endl;
				for (int a_index = 0; a_index < (int)this->current_new_path.size(); a_index++) {
					cout << this->current_new_path[a_index].to_string() << endl;
				}
				cout << "information: " << information << endl;
				cout << endl;

				this->current_new_path.clear();
				
				delete this->learn_scores_network;
				this->learn_scores_network = NULL;

				this->state = EXPLORE_STATE_EXPLORE;
				this->candidate_iter++;
			}
		}
		this->mtx.unlock();
	} else if (explore_state == LOOP_STATE_CHECK_DIFFERENT_ITERS) {
		int iterations = rand()%7;

		for (int f_index = 0; f_index < (int)this->current_front_actions.size(); f_index++) {
			p->perform_action(this->current_front_actions[f_index],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		for (int iter_index = 0; iter_index < iterations; iter_index++) {
			for (int l_index = 0; l_index < (int)this->current_loop_actions.size(); l_index++) {
				p->perform_action(this->current_loop_actions[l_index],
								  observations,
								  save_for_display,
								  raw_actions);
			}
		}
		for (int b_index = 0; b_index < (int)this->current_back_actions.size(); b_index++) {
			p->perform_action(this->current_back_actions[b_index],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		score = p->score_result();

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;
			if (score == 1.0) {
				this->different_iters_success_counts[iterations]++;
			}

			if (this->iter_index == 100000) {
				int num_iterations_useful = 0;
				for (int i = 0; i < 7; i++) {
					if (this->different_iters_success_counts[i] > 0) {
						num_iterations_useful++;
					}
				}

				if (num_iterations_useful >= 3) {
					this->state = LOOP_STATE_LEARN_LOOP;

					this->iter_index = 0;

					this->current_loop = new Loop(this->current_front_actions,
												  this->current_loop_actions,
												  this->current_back_actions);
				} else {
					// don't need to clear current loop vectors

					this->state = EXPLORE_STATE_EXPLORE;
					this->candidate_iter++;
				}
			}
		}
		this->mtx.unlock();
	} else if (explore_state == LOOP_STATE_LEARN_LOOP) {
		this->current_loop->train(p,
								  score,
								  save_for_display,
								  raw_actions);

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;

			if (this->iter_index == 11000000) {
				this->state = LOOP_STATE_MEASURE_AVERAGE;

				this->iter_index = 0;
				this->average_score = 0.0;
			}
		}
		this->mtx.unlock();
	} else if (explore_state == LOOP_STATE_MEASURE_AVERAGE) {
		this->current_loop->pass_through(p,
										 observations,
										 save_for_display,
										 raw_actions);
		score = p->score_result();

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;
			this->average_score += score;

			if (this->iter_index == 100000) {
				this->state = LOOP_STATE_LEARN_SCORES;
				this->average_score /= 100000.0;

				int input_size = path_length + this->current_loop->path_length();
				int network_size = 2*input_size*(3+input_size);
				this->learn_scores_network = new Network(input_size, network_size, 1);
			}
		}
		this->mtx.unlock();
	} else if (explore_state == LOOP_STATE_LEARN_SCORES) {
		this->current_loop->pass_through(p,
										 observations,
										 save_for_display,
										 raw_actions);
		score = p->score_result();

		this->learn_scores_network->mtx.lock();
		this->learn_scores_network->activate(*observations);
		vector<double> errors;
		if (score == 1.0) {
			if (this->learn_scores_network->val_val->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->learn_scores_network->val_val->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->learn_scores_network->val_val->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->learn_scores_network->val_val->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->learn_scores_network->backprop(errors);
		this->learn_scores_network->increment();
		this->learn_scores_network->mtx.unlock();

		if (this->learn_scores_network->epoch == 10000) {
			this->mtx.lock();
			if (explore_state == this->state &&
					explore_id == this->current_id &&
					explore_candidate_iter == this->candidate_iter) {
				this->state = LOOP_STATE_MEASURE_INFORMATION;

				this->iter_index = 0;

				this->think_good_and_good = 0.0;
				this->think_good_but_bad = 0.0;
			}
			this->mtx.unlock();
		}
	} else if (explore_state == LOOP_STATE_MEASURE_INFORMATION) {
		this->current_loop->pass_through(p,
										 observations,
										 save_for_display,
										 raw_actions);
		score = p->score_result();

		this->learn_scores_network->mtx.lock();
		this->learn_scores_network->activate(*observations);
		double think = this->learn_scores_network->val_val->acti_vals[0];
		this->learn_scores_network->mtx.unlock();

		think = max(min(think, 1.0), 0.0);

		if (score > this->average_score) {
			if (think > this->average_score) {
				this->think_good_and_good += (score - this->average_score) \
					* (think - this->average_score);
			}
		} else {
			if (think > this->average_score) {
				this->think_good_but_bad += (this->average_score - score) \
					* (think - this->average_score);
			}
		}

		this->mtx.lock();
		if (explore_state == this->state &&
				explore_id == this->current_id &&
				explore_candidate_iter == this->candidate_iter) {
			this->iter_index++;

			if (this->iter_index == 100000) {
				double information = this->think_good_and_good - this->think_good_but_bad;
				
				cout << this->parent->node_index << endl;
				cout << "front:" << endl;
				for (int f_index = 0; f_index < (int)this->current_loop->front.size(); f_index++) {
					cout << this->current_loop->front[f_index].to_string() << endl;
				}
				cout << "loop:" << endl;
				for (int l_index = 0; l_index < (int)this->current_loop->loop.size(); l_index++) {
					cout << this->current_loop->loop[l_index].to_string() << endl;
				}
				cout << "back:" << endl;
				for (int b_index = 0; b_index < (int)this->current_loop->back.size(); b_index++) {
					cout << this->current_loop->back[b_index].to_string() << endl;
				}
				cout << "information: " << information << endl;
				cout << endl;

				if (information > this->best_score) {
					this->best_score = information;

					this->best_candidate_type = EXPLORE_TYPE_LOOP;
					
					if (this->best_loop != NULL) {
						delete this->best_loop;
					}
					this->best_loop = this->current_loop;
					this->current_loop = NULL;
				} else {
					delete this->current_loop;
					this->current_loop = NULL;
				}

				delete this->learn_scores_network;
				this->learn_scores_network = NULL;

				this->state = EXPLORE_STATE_EXPLORE;
				this->candidate_iter++;
			}
		}
		this->mtx.unlock();
	}

	this->mtx.lock();
	if (this->candidate_iter == 20) {
		if (this->best_candidate_type == EXPLORE_TYPE_NEW_PATH) {
			this->parent->solver->add_nodes(this->parent, this->best_new_path);
		} else if (this->best_candidate_type == EXPLORE_TYPE_LOOP) {
			loop_dictionary->established_mtx.lock();
			loop_dictionary->established.push_back(this->best_loop);
			int established_index = (int)loop_dictionary->established.size()-1;
			loop_dictionary->established_mtx.unlock();

			Action loop_action(LOOP, established_index);
			vector<Action> candidate;
			candidate.push_back(loop_action);

			this->parent->solver->add_nodes(this->parent, candidate);

			this->best_loop = NULL;
		}

		this->current_id = (unsigned)time(NULL);
		this->candidate_iter = 0;
		this->best_score = numeric_limits<double>::lowest();

		this->best_new_path.clear();
		if (this->best_loop != NULL) {
			delete this->best_loop;
			this->best_loop = NULL;
		}
	}
	this->mtx.unlock();
}

void Explore::generate_tree() {
	ExploreNode* curr_node = this->root;
	while (true) {
		curr_node->process();

		// TODO: revisit nodes occasionally
		if (curr_node->children.size() == 0) {
			break;
		}

		double best_uct = numeric_limits<double>::lowest();
		int best_index = -1;
		for (int c_index = 0; c_index < (int)curr_node->children.size(); c_index++) {
			double uct = curr_node->children[c_index] + \
				1.414*sqrt(log(curr_node->count+1)/(curr_node->children[c_index]->count+1));

			if (uct > best_uct) {
				best_uct = uct;
				best_index = c_index;
			}
		}
	}

	this->iter_index = 0;
}

void Explore::iteration() {

}
