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

	ifstream save_file;
	save_file.open("../saves/1659479462.txt");
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
}

Solver::~Solver() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}
}

void Solver::add_nodes(SolutionNode* starting_point,
					   vector<Action> candidate) {
	// theading probably not worth, but if want to proceed, this is a critical section
	SolutionNode* curr_node = starting_point;
	for (int a_index = 0; a_index < (int)candidate.size(); a_index++) {
		SolutionNode* new_node = new SolutionNode(
			this,
			this->current_node_index,
			starting_point->path_length+1+a_index);
		this->nodes.push_back(new_node);
		this->current_node_index++;

		// for threading, add_child probably should be called in reverse
		curr_node->add_child(new_node->node_index, candidate[a_index]);
		curr_node = new_node;
	}

	curr_node->add_child(0, HALT);
}

void Solver::single_pass(bool save_for_display) {
	Problem p;

	SolutionNode* curr_node = this->nodes[1];
	vector<double> observations;

	vector<SolutionNode*> visited_nodes;
	vector<int> chosen_paths;

	int explore_status;
	int explore_id;
	int explore_candidate_iter;

	while (true) {
		observations.push_back(p.get_observation());

		int chosen_path;
		curr_node->process(observations,
						   chosen_path,
						   explore_status,
						   explore_id,
						   explore_candidate_iter);

		visited_nodes.push_back(curr_node);
		chosen_paths.push_back(chosen_path);

		if (chosen_path == -1) {
			break;
		} else {
			if (curr_node->children_indexes[chosen_path] == 0) {
				break;
			}

			p.perform_action(curr_node->children_actions[chosen_path]);
			curr_node = nodes[curr_node->children_indexes[chosen_path]];
		}
	}

	if (explore_status == -1) {
		double score = p.score_result();

		for (int n_index = 0; n_index < (int)visited_nodes.size(); n_index++) {
			vector<double> partial_observations(observations.begin(), \
				observations.begin()+1+n_index);

			visited_nodes[n_index]->update(partial_observations,
										   chosen_paths[n_index],
										   score,
										   false);
		}

		if (save_for_display) {
			ofstream display_file;
			string display_file_name = "../display.txt";
			display_file.open(display_file_name);
			display_file << this->nodes.size() << endl;
			for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
				this->nodes[n_index]->save_for_display(display_file);
			}
			
			display_file << p.initial_world.size() << endl;
			for (int i = 0; i < (int)p.initial_world.size(); i++) {
				display_file << p.initial_world[i] << endl;
			}

			display_file << chosen_paths.size()-1 << endl;
			SolutionNode* curr_node = this->nodes[1];
			for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
				display_file << curr_node->node_index << "," << chosen_paths[i] << endl;

				curr_node = this->nodes[curr_node->children_indexes[chosen_paths[i]]];
			}

			display_file << "no_explore" << endl;

			int num_actions = (int)chosen_paths.size()-1;
			display_file << num_actions << endl;
			SolutionNode* action_curr_node = this->nodes[1];
			for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
				display_file << action_curr_node->children_actions[chosen_paths[i]].write << endl;
				display_file << action_curr_node->children_actions[chosen_paths[i]].move << endl;

				action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
			}

			display_file.close();
		}
	} else {
		if (explore_status == EXPLORE_STATE_EXPLORE) {
			vector<Action> candidate;
			geometric_distribution<int> seq_length_dist(0.2);
			int add_seq_length = 1+seq_length_dist(generator);

			normal_distribution<double> write_val_dist(0.0, 2.0);
			for (int i = 0; i < add_seq_length; i++) {
				Action a(write_val_dist(generator), rand()%3);
				candidate.push_back(a);
			}

			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i]);
			}

			double score = p.score_result();

			for (int n_index = 0; n_index < (int)visited_nodes.size()-1; n_index++) {
				vector<double> partial_observations(observations.begin(), \
					observations.begin()+1+n_index);

				visited_nodes[n_index]->update(partial_observations,
											   chosen_paths[n_index],
											   score,
											   true);
			}

			visited_nodes[visited_nodes.size()-1]->update_explore_candidate(
				observations,
				explore_status,
				explore_id,
				explore_candidate_iter,
				score,
				candidate);

			if (save_for_display) {
				ofstream display_file;
				string display_file_name = "../display.txt";
				display_file.open(display_file_name);
				display_file << this->nodes.size() << endl;
				for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
					this->nodes[n_index]->save_for_display(display_file);
				}
				
				display_file << p.initial_world.size() << endl;
				for (int i = 0; i < (int)p.initial_world.size(); i++) {
					display_file << p.initial_world[i] << endl;
				}

				display_file << chosen_paths.size()-1 << endl;
				SolutionNode* curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					display_file << curr_node->node_index << "," << chosen_paths[i] << endl;

					curr_node = this->nodes[curr_node->children_indexes[chosen_paths[i]]];
				}

				display_file << "explore_" << explore_status << endl;

				int num_actions = (int)chosen_paths.size()-1+(int)candidate.size();
				display_file << num_actions << endl;
				SolutionNode* action_curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					display_file << action_curr_node->children_actions[chosen_paths[i]].write << endl;
					display_file << action_curr_node->children_actions[chosen_paths[i]].move << endl;

					action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
				}
				for (int i = 0; i < (int)candidate.size(); i++) {
					display_file << candidate[i].write << endl;
					display_file << candidate[i].move << endl;
				}

				display_file.close();
			}
		} else {
			vector<Action> candidate = visited_nodes[visited_nodes.size()-1]->current_candidate;
			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i]);
				observations.push_back(p.get_observation());
			}

			double score = p.score_result();

			for (int n_index = 0; n_index < (int)visited_nodes.size()-1; n_index++) {
				vector<double> partial_observations(observations.begin(), \
					observations.begin()+1+n_index);

				visited_nodes[n_index]->update(partial_observations,
											   chosen_paths[n_index],
											   score,
											   true);
			}

			vector<double> partial_observations(observations.begin(), \
					observations.begin()+visited_nodes.size());
			visited_nodes[visited_nodes.size()-1]->update_explore(
				partial_observations,
				observations,
				explore_status,
				explore_id,
				explore_candidate_iter,
				score);

			if (save_for_display) {
				ofstream display_file;
				string display_file_name = "../display.txt";
				display_file.open(display_file_name);
				display_file << this->nodes.size() << endl;
				for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
					this->nodes[n_index]->save_for_display(display_file);
				}
				
				display_file << p.initial_world.size() << endl;
				for (int i = 0; i < (int)p.initial_world.size(); i++) {
					display_file << p.initial_world[i] << endl;
				}

				display_file << chosen_paths.size()-1 << endl;
				SolutionNode* curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					display_file << curr_node->node_index << "," << chosen_paths[i] << endl;

					curr_node = this->nodes[curr_node->children_indexes[chosen_paths[i]]];
				}

				display_file << "explore_" << explore_status << endl;

				int num_actions = (int)chosen_paths.size()-1+(int)candidate.size();
				display_file << num_actions << endl;
				SolutionNode* action_curr_node = this->nodes[1];
				for (int i = 0; i < (int)chosen_paths.size()-1; i++) {
					display_file << action_curr_node->children_actions[chosen_paths[i]].write << endl;
					display_file << action_curr_node->children_actions[chosen_paths[i]].move << endl;

					action_curr_node = this->nodes[action_curr_node->children_indexes[chosen_paths[i]]];
				}
				for (int i = 0; i < (int)candidate.size(); i++) {
					display_file << candidate[i].write << endl;
					display_file << candidate[i].move << endl;
				}

				display_file.close();
			}
		}
	}
}

void Solver::save() {
	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);
	save_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(save_file);
	}
	save_file.close();
}
