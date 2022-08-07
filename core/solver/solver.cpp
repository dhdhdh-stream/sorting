#include "solver.h"

#include <iostream>
#include <fstream>
#include <limits>

#include "definitions.h"
#include "network.h"

using namespace std;

Solver::Solver() {
	// SolutionNode* halt_node = new SolutionNode(this, 0, -1);
	// this->nodes.push_back(halt_node);
	// SolutionNode* root_node = new SolutionNode(this, 1, 1);
	// this->nodes.push_back(root_node);
	// root_node->add_child(0, HALT);
	// this->current_node_index = 2;

	// this->action_dictionary = new ActionDictionary();

	ifstream save_file;
	save_file.open("../saves/1659805791.txt");
	string num_nodes_line;
	
	getline(save_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		SolutionNode* node = new SolutionNode(this,
											  n_index,
											  save_file);
		this->nodes.push_back(node);
	}
	this->current_node_index = num_nodes;

	this->action_dictionary = new ActionDictionary(save_file);

	save_file.close();
}

Solver::~Solver() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}

	delete this->action_dictionary;
}

void Solver::add_nodes(SolutionNode* starting_point,
					   vector<Action> candidate) {
	this->nodes_mtx.lock();
	int sum_path_length = 0;
	vector<SolutionNode*> new_nodes;
	for (int a_index = 0; a_index < (int)candidate.size(); a_index++) {
		int new_path_length = this->action_dictionary->calculate_action_path_length(candidate[a_index]);
		sum_path_length += new_path_length;

		SolutionNode* new_node = new SolutionNode(
			this,
			this->current_node_index,
			starting_point->path_length+sum_path_length);
		this->nodes.push_back(new_node);
		this->current_node_index++;

		new_nodes.push_back(new_node);
	}
	this->nodes_mtx.unlock();

	new_nodes[(int)new_nodes.size()-1]->add_child(0, HALT);
	for (int n_index = (int)new_nodes.size()-2; n_index >= 0; n_index--) {
		new_nodes[n_index]->add_child(new_nodes[n_index+1]->node_index, candidate[n_index+1]);
	}
	starting_point->add_child(new_nodes[0]->node_index, candidate[0]);

	// simple heuristic to add compound actions for now
	int starting_index = rand()%(int)candidate.size();
	int ending_index = starting_index+1+rand()%((int)candidate.size()-starting_index);
	vector<Action> new_compound_action(candidate.begin()+starting_index, \
		candidate.begin()+ending_index);
	this->action_dictionary->add_action(new_compound_action);
}

void Solver::single_pass(bool save_for_display) {
	ofstream display_file;
	if (save_for_display) {
		this->display_mtx.lock();

		string display_file_name = "../display.txt";
		display_file.open(display_file_name);

		this->nodes_mtx.lock();
		int num_nodes = (int)this->nodes.size();
		this->nodes_mtx.unlock();
		
		display_file << num_nodes << endl;
		for (int n_index = 0; n_index < num_nodes; n_index++) {
			this->nodes[n_index]->save_for_display(display_file);
		}
	}

	vector<double> observations;
	Problem p(observations);

	SolutionNode* curr_node = this->nodes[1];
	bool force_eval = false;
	if (rand()%20 == 0) {
		force_eval = true;
	}

	vector<double> guesses;

	vector<SolutionNode*> visited_nodes;
	vector<int> chosen_paths;
	vector<int> path_lengths;

	int explore_status;
	int explore_id;
	int explore_candidate_iter;

	while (true) {
		int chosen_path;
		curr_node->process(observations,
						   chosen_path,
						   guesses,
						   explore_status,
						   explore_id,
						   explore_candidate_iter,
						   force_eval,
						   save_for_display,
						   display_file);

		visited_nodes.push_back(curr_node);
		chosen_paths.push_back(chosen_path);
		path_lengths.push_back((int)observations.size());

		if (chosen_path == -1) {
			if (save_for_display) {
				display_file << -1 << endl;
			}

			break;
		} else {
			if (curr_node->children_indexes[chosen_path] == 0) {
				if (save_for_display) {
					display_file << 0 << endl;
				}

				break;
			}

			p.perform_action(curr_node->children_actions[chosen_path],
							 observations,
							 this->action_dictionary);
			curr_node = nodes[curr_node->children_indexes[chosen_path]];
		}
	}

	if (explore_status == -1) {
		double score = p.score_result();

		double sum_misguess = 0.0;
		for (int n_index = (int)visited_nodes.size()-1; n_index >= 0; n_index--) {
			vector<double> partial_observations(observations.begin(), \
				observations.begin()+path_lengths[n_index]);

			sum_misguess += abs(score - guesses[n_index]);
			double curr_misguess = sum_misguess/((int)visited_nodes.size()-n_index);

			visited_nodes[n_index]->update(partial_observations,
										   chosen_paths[n_index],
										   score,
										   curr_misguess);
		}

		if (save_for_display) {
			vector<Action> raw_actions;
			SolutionNode* action_curr_node = this->nodes[1];
			for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
				this->action_dictionary->convert_to_raw_actions(
					action_curr_node->children_actions[chosen_paths[i]],
					raw_actions);

				action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
			}
			display_file << raw_actions.size() << endl;
			for (int i = 0; i < (int)raw_actions.size(); i++) {
				display_file << raw_actions[i].write << endl;
				display_file << raw_actions[i].move << endl;
			}
		}
	} else {
		if (explore_status == EXPLORE_STATE_EXPLORE) {
			vector<Action> candidate;
			geometric_distribution<int> seq_length_dist(0.2);
			int add_seq_length = 1+seq_length_dist(generator);

			vector<int> compound_actions_tried;
			normal_distribution<double> write_val_dist(0.0, 2.0);
			for (int i = 0; i < add_seq_length; i++) {
				if (this->action_dictionary->actions.size() > 0 && rand()%3 == 0) {
					int compound_index = this->action_dictionary->select_compound_action();
					compound_actions_tried.push_back(compound_index);
					Action a(compound_index);
					candidate.push_back(a);
				} else {
					Action a(write_val_dist(generator), rand()%3);
					candidate.push_back(a);
				}
			}

			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i],
								 observations,
								 this->action_dictionary);
			}

			double score = p.score_result();

			if (score == 1.0) {
				visited_nodes[visited_nodes.size()-1]->update_explore_candidate(
					explore_status,
					explore_id,
					explore_candidate_iter,
					candidate);

				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					this->action_dictionary->num_success[compound_actions_tried[c_index]]++;
					this->action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			} else {
				for (int c_index = 0; c_index < (int)compound_actions_tried.size(); c_index++) {
					this->action_dictionary->count[compound_actions_tried[c_index]]++;
				}
			}

			if (save_for_display) {
				vector<Action> raw_actions;
				SolutionNode* action_curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					this->action_dictionary->convert_to_raw_actions(
						action_curr_node->children_actions[chosen_paths[i]],
						raw_actions);

					action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
				}
				for (int i = 0; i < (int)candidate.size(); i++) {
					this->action_dictionary->convert_to_raw_actions(
						candidate[i],
						raw_actions);
				}
				display_file << raw_actions.size() << endl;
				for (int i = 0; i < (int)raw_actions.size(); i++) {
					display_file << raw_actions[i].write << endl;
					display_file << raw_actions[i].move << endl;
				}
			}
		} else {
			vector<Action> candidate = visited_nodes[visited_nodes.size()-1]->current_candidate;
			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i],
								 observations,
								 this->action_dictionary);
			}

			double score = p.score_result();

			vector<double> partial_observations(observations.begin(), \
					observations.begin()+path_lengths[visited_nodes.size()-1]);
			visited_nodes[visited_nodes.size()-1]->update_explore(
				partial_observations,
				observations,
				explore_status,
				explore_id,
				explore_candidate_iter,
				score);

			double sum_misguess = abs(score - guesses[(int)visited_nodes.size()-1]);

			for (int n_index = (int)visited_nodes.size()-2; n_index >= 0; n_index--) {
				vector<double> partial_observations(observations.begin(), \
					observations.begin()+path_lengths[n_index]);

				sum_misguess += abs(score - guesses[n_index]);
				double curr_misguess = sum_misguess/((int)visited_nodes.size()-n_index);

				visited_nodes[n_index]->update(partial_observations,
											   chosen_paths[n_index],
											   score,
											   curr_misguess);
			}

			if (save_for_display) {
				vector<Action> raw_actions;
				SolutionNode* action_curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					this->action_dictionary->convert_to_raw_actions(
						action_curr_node->children_actions[chosen_paths[i]],
						raw_actions);

					action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
				}
				for (int i = 0; i < (int)candidate.size(); i++) {
					this->action_dictionary->convert_to_raw_actions(
						candidate[i],
						raw_actions);
				}
				display_file << raw_actions.size() << endl;
				for (int i = 0; i < (int)raw_actions.size(); i++) {
					display_file << raw_actions[i].write << endl;
					display_file << raw_actions[i].move << endl;
				}
			}
		}
	}

	if (save_for_display) {
		display_file << p.initial_world.size() << endl;
		for (int i = 0; i < (int)p.initial_world.size(); i++) {
			display_file << p.initial_world[i] << endl;
		}

		this->display_mtx.unlock();
	}
}

void Solver::save() {
	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);

	this->nodes_mtx.lock();
	int num_nodes = (int)this->nodes.size();
	this->nodes_mtx.unlock();

	save_file << num_nodes << endl;
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		this->nodes[n_index]->save(save_file);
	}

	this->action_dictionary->save(save_file);

	save_file.close();
}
