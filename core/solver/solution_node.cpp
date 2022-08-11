#include "solution_node.h"

#include <cmath>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

SolutionNode::SolutionNode(Solver* solver,
						   int node_index,
						   int path_length) {
	this->solver = solver;
	this->node_index = node_index;
	this->path_length = path_length;

	this->average_score = 0.5;

	this->explore = new Explore(this);
}

SolutionNode::SolutionNode(Solver* solver,
						   int node_index,
						   ifstream& save_file) {
	this->solver = solver;
	this->node_index = node_index;

	string path_length_line;
	getline(save_file, path_length_line);
	this->path_length = stoi(path_length_line);

	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		string child_index_line;
		getline(save_file, child_index_line);
		this->children_indexes.push_back(stoi(child_index_line));

		Action a(save_file);
		this->children_actions.push_back(a);

		string score_network_name_line;
		getline(save_file, score_network_name_line);
		boost::algorithm::trim(score_network_name_line);
		this->children_score_network_names.push_back(score_network_name_line);

		ifstream child_score_network_save_file;
		child_score_network_save_file.open(score_network_name_line);
		Network* child_score_network = new Network(child_score_network_save_file);
		child_score_network_save_file.close();
		this->children_score_networks.push_back(child_score_network);

		string information_network_name_line;
		getline(save_file, information_network_name_line);
		boost::algorithm::trim(information_network_name_line);
		this->children_information_network_names.push_back(information_network_name_line);

		ifstream child_information_network_save_file;
		child_information_network_save_file.open(information_network_name_line);
		Network* child_information_network = new Network(child_information_network_save_file);
		child_information_network_save_file.close();
		this->children_information_networks.push_back(child_information_network);
	}

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);

	this->explore = new Explore(this);
}

SolutionNode::~SolutionNode() {
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		delete this->children_score_networks[c_index];
		delete this->children_information_networks[c_index];
	}

	delete this->explore;
}

void SolutionNode::add_child(int child_index,
							 Action child_action) {
	this->children_mtx.lock();
	
	this->children_indexes.push_back(child_index);
	this->children_actions.push_back(child_action);
	
	int network_size = 2*this->path_length*(3+this->path_length);

	Network* child_score_network = new Network(this->path_length, network_size, 1);
	this->children_score_networks.push_back(child_score_network);
	string child_score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->children_indexes.size()-1) + "_" + to_string(time(NULL)) + ".txt";
	this->children_score_network_names.push_back(child_score_network_name);

	Network* child_information_network = new Network(this->path_length, network_size, 1);
	this->children_information_networks.push_back(child_information_network);
	string child_information_network_name = "../saves/nns/info_" + to_string(this->node_index) \
		+ "_" + to_string(this->children_indexes.size()-1) + "_" + to_string(time(NULL)) + ".txt";
	this->children_information_network_names.push_back(child_information_network_name);

	this->children_mtx.unlock();
}

void SolutionNode::process(vector<double> observations,
						   int& chosen_path,
						   vector<double>& guesses,
						   bool& force_eval,
						   bool save_for_display,
						   ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	// look for new child
	int new_index = -1;
	for (int c_index = 0; c_index < num_children; c_index++) {
		Network* child_score_network = this->children_score_networks[c_index];
		if (child_score_network->epoch < 10000) {
			new_index = c_index;
			break;
		} else if (child_score_network->epoch < 20000) {
			child_score_network->mtx.lock();
			child_score_network->activate(observations);
			double predicted_score = child_score_network->val_val->acti_vals[0];
			child_score_network->mtx.unlock();
			
			predicted_score = max(min(predicted_score, 1.0), 0.0);

			guesses.push_back(predicted_score);

			break;
		}
	}
	if (new_index != -1) {
		if (save_for_display) {
			display_file << "new_child" << endl;
			display_file << new_index << endl;
		}

		force_eval = true;

		chosen_path = new_index;
		return;
	}

	if (rand()%20 == 0) {
		chosen_path = rand()%(int)num_children;

		Network* child_score_network = this->children_score_networks[chosen_path];
		child_score_network->mtx.lock();
		child_score_network->activate(observations);
		double predicted_score = child_score_network->val_val->acti_vals[0];
		child_score_network->mtx.unlock();
		
		predicted_score = max(min(predicted_score, 1.0), 0.0);

		guesses.push_back(predicted_score);

		force_eval = true;

		if (save_for_display) {
			display_file << "random" << endl;
			display_file << chosen_path << endl;
		}

		return;
	}

	double best_index_score = numeric_limits<double>::lowest();
	double best_index_misguess = numeric_limits<double>::max();
	int best_index = -1;
	vector<double> predicted_scores;
	vector<double> predicted_misguesses;
	for (int c_index = 0; c_index < num_children; c_index++) {
		Network* child_score_network = this->children_score_networks[c_index];
		child_score_network->mtx.lock();
		child_score_network->activate(observations);
		double predicted_score = child_score_network->val_val->acti_vals[0];
		child_score_network->mtx.unlock();
		
		predicted_score = max(min(predicted_score, 1.0), 0.0);

		Network* child_information_network = this->children_information_networks[c_index];
		child_information_network->mtx.lock();
		child_information_network->activate(observations);
		double predicted_misguess = 1.0 - child_information_network->val_val->acti_vals[0];
		child_information_network->mtx.unlock();

		predicted_misguess = max(predicted_misguess, 0.0);

		double curr_information = predicted_score - predicted_misguess;

		if (save_for_display) {
			predicted_scores.push_back(predicted_score);
			predicted_misguesses.push_back(predicted_misguess);
		}

		double previous_information = best_index_score - best_index_misguess;
		if (curr_information > previous_information) {
			best_index_score = predicted_score;
			best_index_misguess = predicted_misguess;
			best_index = c_index;
		}
	}

	if ((best_index_score != 1.0 || best_index_misguess != 0.0)
			&& !force_eval) {
		if (this->children_indexes[best_index] == 0) {
			// explore if will simply hit halt again
			if (save_for_display) {
				display_file << "forced_explore" << endl;
			}

			chosen_path = -1;
			return;
		} else if (best_index_score + best_index_misguess < this->average_score) {
			if (rand()%2 == 0) {
				if (save_for_display) {
					display_file << "good_explore" << endl;
				}

				chosen_path = -1;
				return;
			}
		} else {
			if (rand()%10 == 0) {
				if (save_for_display) {
					display_file << "random_explore" << endl;
				}

				chosen_path = -1;
				return;
			}
		}
	}

	if (save_for_display) {
		display_file << "process" << endl;
		for (int c_index = 0; c_index < num_children; c_index++) {
			display_file << predicted_scores[c_index] << endl;
			display_file << predicted_misguesses[c_index] << endl;
		}
		display_file << best_index << endl;
	}

	guesses.push_back(best_index_score);

	chosen_path = best_index;
	return;
}

void SolutionNode::update_average(double score) {
	// don't bother locking (just loses 1 update?)
	this->average_score = 0.99999*this->average_score + 0.00001*score;
}

void SolutionNode::update_score_network(vector<double> observations,
										int chosen_path,
										double score) {
	Network* child_score_network = this->children_score_networks[chosen_path];
	child_score_network->mtx.lock();
	child_score_network->activate(observations);
	vector<double> child_score_errors;
	if (score == 1.0) {
		if (child_score_network->val_val->acti_vals[0] < 1.0) {
			child_score_errors.push_back(1.0 - child_score_network->val_val->acti_vals[0]);
		} else {
			child_score_errors.push_back(0.0);
		}
	} else {
		if (child_score_network->val_val->acti_vals[0] > 0.0) {
			child_score_errors.push_back(0.0 - child_score_network->val_val->acti_vals[0]);
		} else {
			child_score_errors.push_back(0.0);
		}
	}
	child_score_network->backprop(child_score_errors);
	child_score_network->increment();
	child_score_network->mtx.unlock();
}

void SolutionNode::update_information_network(vector<double> observations,
											  int chosen_path,
											  double misguess) {
	Network* child_information_network = this->children_information_networks[chosen_path];
	child_information_network->mtx.lock();
	child_information_network->activate(observations);
	vector<double> information_errors;
	if (misguess == 0.0 && child_information_network->val_val->acti_vals[0] > 1.0) {
		information_errors.push_back(0.0);
	} else {
		double information_target = 1.0 - misguess;
		information_errors.push_back(information_target - child_information_network->val_val->acti_vals[0]);
	}
	child_information_network->backprop(information_errors);
	child_information_network->increment();
	child_information_network->mtx.unlock();
}

void SolutionNode::save(ofstream& save_file) {
	save_file << this->path_length << endl;
	
	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	save_file << num_children << endl;
	for (int c_index = 0; c_index < num_children; c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		this->children_actions[c_index].save(save_file);

		save_file << this->children_score_network_names[c_index] << endl;
		ofstream child_score_network_save_file;
		child_score_network_save_file.open(this->children_score_network_names[c_index]);
		this->children_score_networks[c_index]->save(child_score_network_save_file);
		child_score_network_save_file.close();

		save_file << this->children_information_network_names[c_index] << endl;
		ofstream child_information_network_save_file;
		child_information_network_save_file.open(this->children_information_network_names[c_index]);
		this->children_information_networks[c_index]->save(child_information_network_save_file);
		child_information_network_save_file.close();
	}

	save_file << this->average_score << endl;
}

void SolutionNode::save_for_display(ofstream& save_file) {
	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	save_file << num_children << endl;
	for (int c_index = 0; c_index < num_children; c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		this->children_actions[c_index].save(save_file);
	}

	save_file << this->average_score << endl;
}
