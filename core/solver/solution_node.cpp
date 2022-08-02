#include "solution_node.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

SolutionNode::SolutionNode(Solver* solver,
						   int node_index,
						   int path_length) {
	this->solver = solver;
	this->node_index = node_index;
	this->path_length = path_length;

	this->score_network = new Network(this->path_length, 100, 1);
	this->score_network_name = "../saves/nns/sns_" + \
		to_string(this->node_index) + to_string(time(NULL)) + ".txt";
	this->average_score = 0.5;

	this->explore_state = EXPLORE_STATE_EXPLORE;
	this->current_explore_id = (unsigned)time(NULL);
	this->candidate_iter = 0;
	this->best_information = numeric_limits<double>::lowest();

	this->learn_scores_network = NULL;
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

		string write_line;
		getline(save_file, write_line);
		double write_val = stof(write_line);
		
		string move_line;
		getline(save_file, move_line);
		int move_val = stoi(move_line);
		
		Action a(write_val, move_val);
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
	}

	string score_network_name_line;
	getline(save_file, score_network_name_line);
	boost::algorithm::trim(score_network_name_line);
	this->score_network_name = score_network_name_line;
	
	ifstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);

	this->explore_state = EXPLORE_STATE_EXPLORE;
	this->current_explore_id = (unsigned)time(NULL);
	this->candidate_iter = 0;
	this->best_information = numeric_limits<double>::lowest();

	this->learn_scores_network = NULL;
}

SolutionNode::~SolutionNode() {
	for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
		delete this->children_score_networks[c_index];
	}

	delete this->score_network;

	if (this->learn_scores_network != NULL) {
		delete this->learn_scores_network;
	}
}

void SolutionNode::add_child(int child_index,
							 Action child_action) {
	this->children_mtx.lock();
	this->children_indexes.push_back(child_index);
	this->children_actions.push_back(child_action);
	Network* child_score_network = new Network(this->path_length, 100, 1);
	this->children_score_networks.push_back(child_score_network);
	string child_score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->children_indexes.size()-1) + "_" + to_string(time(NULL)) + ".txt";
	this->children_score_network_names.push_back(child_score_network_name);
	this->children_mtx.unlock();
}

void SolutionNode::process(vector<double> observations,
						   int& chosen_path,
						   int& explore_status,
						   int& explore_id,
						   int& explore_candidate_iter) {
	if (this->score_network->epoch < 10000) {
		// shouldn't have multiple children yet
		chosen_path = 0;
		explore_status = -1;
		explore_id = -1;
		explore_candidate_iter = -1;
		return;
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(observations);
		double think = this->score_network->val_val.acti_vals[0];
		this->score_network->mtx.unlock();

		if (think < this->average_score) {
			if (rand()%2 == 0) {
				this->explore_mtx.lock();
				chosen_path = -1;
				explore_status = this->explore_state;
				explore_id = this->current_explore_id;
				explore_candidate_iter = this->candidate_iter;
				this->explore_mtx.unlock();
				return;
			}
		} else {
			if (rand()%10 == 0) {
				this->explore_mtx.lock();
				chosen_path = -1;
				explore_status = this->explore_state;
				explore_id = this->current_explore_id;
				explore_candidate_iter = this->candidate_iter;
				this->explore_mtx.unlock();
				return;
			}
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
			}
		}
		if (new_index != -1) {
			chosen_path = new_index;
			explore_status = -1;
			explore_id = -1;
			explore_candidate_iter = -1;
			return;
		}

		// look for best score
		double best_score = numeric_limits<double>::lowest();
		int best_index = -1;
		for (int c_index = 0; c_index < num_children; c_index++) {
			Network* child_score_network = this->children_score_networks[c_index];
			child_score_network->mtx.lock();
			child_score_network->activate(observations);
			double predicted_score = child_score_network->val_val.acti_vals[0];
			child_score_network->mtx.unlock();
			if (predicted_score > best_score) {
				best_score = predicted_score;
				best_index = c_index;
			}
		}

		chosen_path = best_index;
		explore_status = -1;
		explore_id = -1;
		explore_candidate_iter = -1;
		return;
	}
}

void SolutionNode::update_score_network(vector<double> observations,
										double score,
										bool did_explore) {
	this->score_network->mtx.lock();
	this->score_network->activate(observations);
	vector<double> errors;
	double error = score - this->score_network->val_val.acti_vals[0];
	if (did_explore) {
		error *= 0.1;
	}
	errors.push_back(error);
	this->score_network->backprop(errors);
	if (did_explore) {
		this->average_score = 0.99999*this->average_score + 0.00001*score;
	} else {
		this->average_score = 0.999999*this->average_score + 0.000001*score;
	}
	this->score_network->mtx.unlock();
}

void SolutionNode::update(vector<double> observations,
						  int chosen_path,
						  double score,
						  bool did_explore) {
	update_score_network(observations,
						 score,
						 did_explore);

	Network* child_score_network = this->children_score_networks[chosen_path];
	child_score_network->mtx.lock();
	child_score_network->activate(observations);
	vector<double> errors;
	double error = score - child_score_network->val_val.acti_vals[0];
	if (did_explore) {
		error *= 0.1;
	}
	errors.push_back(error);
	child_score_network->backprop(errors);
	child_score_network->mtx.unlock();
}

void SolutionNode::update_explore(vector<double> observations,
								  vector<double> full_observations,
								  int explore_status,
								  int explore_id,
								  int explore_candidate_iter,
								  double score) {
	// update_score_network(observations,
	// 					 score,
	// 					 true);

	this->explore_mtx.lock();
	if (explore_status == this->explore_state &&
			explore_id == this->current_explore_id &&
			explore_candidate_iter == this->candidate_iter) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			// should not occur
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_AVERAGE) {
			this->measure_average_p_index++;
			this->measure_average_average_score += score;

			if (this->measure_average_p_index == 100000) {
				this->explore_state = EXPLORE_STATE_LEARN_SCORES;
				this->measure_average_average_score /= 100000;

				this->learn_scores_network = new Network(
					this->path_length+(int)this->current_candidate.size(),
					100,
					1);
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SCORES) {
			this->learn_scores_network->activate(full_observations);
			vector<double> errors;
			errors.push_back(score - this->learn_scores_network->val_val.acti_vals[0]);
			this->learn_scores_network->backprop(errors);

			if (this->learn_scores_network->epoch == 10000) {
				this->explore_state = EXPLORE_STATE_MEASURE_INFORMATION;
				this->measure_information_p_index = 0;
				this->measure_information_think_good_and_good = 0.0;
				this->measure_information_think_good_but_bad = 0.0;
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_INFORMATION) {
			this->learn_scores_network->activate(full_observations);
			double think = this->learn_scores_network->val_val.acti_vals[0];

			if (score > this->measure_average_average_score) {
				if (think > this->measure_average_average_score) {
					this->measure_information_think_good_and_good += \
						(score - this->measure_average_average_score) \
						* (think - this->measure_average_average_score);
				}
			} else {
				if (think > this->measure_average_average_score) {
					this->measure_information_think_good_but_bad += \
						(this->measure_average_average_score - score) \
						* (think - this->measure_average_average_score);
				}
			}

			this->measure_information_p_index++;

			if (this->measure_information_p_index == 100000) {
				double information = this->measure_information_think_good_and_good \
					- this->measure_information_think_good_but_bad;
				if (information > this->best_information) {
					this->best_information = information;
					this->best_candidate = this->current_candidate;
				}

				for (int a_index = 0; a_index < (int)this->current_candidate.size(); a_index++) {
					cout << this->current_candidate[a_index].to_string() << endl;
				}
				cout << "information: " << information << endl;
				cout << endl;

				this->current_candidate.clear();
				delete this->learn_scores_network;
				this->learn_scores_network = NULL;

				this->explore_state = EXPLORE_STATE_EXPLORE;
				this->candidate_iter++;

				if (this->candidate_iter == 20) {
					this->solver->add_nodes(this, this->best_candidate);

					this->current_explore_id = (unsigned)time(NULL);
					this->candidate_iter = 0;
					this->best_information = numeric_limits<double>::lowest();
					this->best_candidate.clear();
				}
			}
		}
	}
	this->explore_mtx.unlock();
}

void SolutionNode::update_explore_candidate(vector<double> observations,
											int explore_status,
											int explore_id,
											int explore_candidate_iter,
											double score,
											vector<Action> candidate) {
	// update_score_network(observations,
	// 					 score,
	// 					 true);

	this->explore_mtx.lock();
	if (explore_status == this->explore_state &&
			explore_id == this->current_explore_id &&
			explore_candidate_iter == this->candidate_iter) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			if (score == 1.0) {
				this->current_candidate = candidate;

				this->explore_state = EXPLORE_STATE_MEASURE_AVERAGE;
				this->measure_average_p_index = 0;
				this->measure_average_average_score = 0.0;
			}
		}
	}
	this->explore_mtx.unlock();
}

void SolutionNode::save(ofstream& save_file) {
	save_file << this->path_length << endl;
	
	save_file << this->children_indexes.size() << endl;
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		save_file << this->children_actions[c_index].write << endl;
		save_file << this->children_actions[c_index].move << endl;
		save_file << this->children_score_network_names[c_index] << endl;
		
		ofstream child_score_network_save_file;
		child_score_network_save_file.open(this->children_score_network_names[c_index]);
		this->children_score_networks[c_index]->save(child_score_network_save_file);
		child_score_network_save_file.close();
	}
	
	save_file << this->score_network_name << endl;

	ofstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->average_score << endl;
}

void SolutionNode::save_for_display(ofstream& save_file) {
	save_file << this->children_indexes.size() << endl;
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		save_file << this->children_actions[c_index].write << endl;
		save_file << this->children_actions[c_index].move << endl;
	}
	save_file << this->explore_state << endl;
}
