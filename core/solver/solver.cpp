#include "solver.h"

#include <iostream>
#include <fstream>
#include <limits>

#include "definitions.h"
#include "network.h"
#include "utilities.h"

using namespace std;

Solver::Solver() {
	SolutionNode* halt_node = new SolutionNode(this, 0, -1);
	this->nodes.push_back(halt_node);
	SolutionNode* root_node = new SolutionNode(this, 1, 1);
	this->nodes.push_back(root_node);
	root_node->add_child(0, HALT);
	this->current_node_index = 2;

	action_dictionary = new ActionDictionary();
	loop_dictionary = new LoopDictionary();

	// ifstream save_file;
	// save_file.open("../saves/1660228742.txt");
	// string num_nodes_line;
	
	// getline(save_file, num_nodes_line);
	// int num_nodes = stoi(num_nodes_line);
	// for (int n_index = 0; n_index < num_nodes; n_index++) {
	// 	SolutionNode* node = new SolutionNode(this,
	// 										  n_index,
	// 										  save_file);
	// 	this->nodes.push_back(node);
	// }
	// this->current_node_index = num_nodes;

	// action_dictionary = new ActionDictionary(save_file);
	// loop_dictionary = new LoopDictionary(save_file);

	// save_file.close();
}

Solver::~Solver() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}

	delete action_dictionary;
	delete loop_dictionary;
}

void Solver::add_nodes(SolutionNode* starting_point,
					   vector<Action> candidate) {
	this->nodes_mtx.lock();
	int sum_path_length = 0;
	vector<SolutionNode*> new_nodes;
	for (int a_index = 0; a_index < (int)candidate.size(); a_index++) {
		int new_path_length = calculate_action_path_length(candidate[a_index]);
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
	action_dictionary->add_action(new_compound_action);
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
	Problem p(&observations);

	vector<Action> raw_actions;

	SolutionNode* curr_node = this->nodes[1];
	bool force_eval = false;
	if (rand()%20 == 0) {
		force_eval = true;
	}

	vector<double> guesses;

	vector<SolutionNode*> visited_nodes;
	vector<int> chosen_paths;
	vector<int> path_lengths;

	while (true) {
		int chosen_path;
		curr_node->process(observations,
						   chosen_path,
						   guesses,
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
							 &observations,
							 save_for_display,
							 &raw_actions);
			curr_node = nodes[curr_node->children_indexes[chosen_path]];
		}
	}

	double score;
	if (chosen_paths[chosen_paths.size()-1] != -1) {
		score = p.score_result();
	
		if (force_eval) {
			if (guesses.size() < visited_nodes.size()) {
				// new node still learning its scores
				for (int n_index = (int)visited_nodes.size()-1; n_index >= 0; n_index--) {
					visited_nodes[n_index]->update_average(score);

					vector<double> partial_observations(observations.begin(),
						observations.begin()+path_lengths[n_index]);

					visited_nodes[n_index]->update_score_network(partial_observations,
												   chosen_paths[n_index],
												   score);
				}
			} else {
				double sum_misguess = 0.0;
				for (int n_index = (int)visited_nodes.size()-1; n_index >= 0; n_index--) {
					visited_nodes[n_index]->update_average(score);

					vector<double> partial_observations(observations.begin(),
						observations.begin()+path_lengths[n_index]);

					visited_nodes[n_index]->update_score_network(partial_observations,
												   chosen_paths[n_index],
												   score);

					sum_misguess += abs(score - guesses[n_index]);
					double curr_misguess = sum_misguess/((int)visited_nodes.size()-n_index);

					visited_nodes[n_index]->update_information_network(
						partial_observations,
						chosen_paths[n_index],
						curr_misguess);
				}
			}
		}
	} else {
		visited_nodes[visited_nodes.size()-1]->explore->process(
			&p,
			&observations,
			score,
			save_for_display,
			&raw_actions);
	}

	if (save_for_display) {
		display_file << raw_actions.size() << endl;
		for (int i = 0; i < (int)raw_actions.size(); i++) {
			display_file << raw_actions[i].write << endl;
			display_file << raw_actions[i].move << endl;
		}

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

	action_dictionary->save(save_file);
	loop_dictionary->save(save_file);

	save_file.close();
}
