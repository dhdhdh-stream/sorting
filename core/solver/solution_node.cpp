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
	int network_size = 2*this->path_length*(3+this->path_length);
	this->score_network = new Network(this->path_length, network_size, 1);
	this->score_network_name = "../saves/nns/sns_" + \
		to_string(this->node_index) + to_string(time(NULL)) + ".txt";
	this->certainty_network = new Network(this->path_length, network_size, 1);
	this->certainty_network_name = "../saves/nns/snc_" + \
		to_string(this->node_index) + to_string(time(NULL)) + ".txt";

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

		string move_line;
		getline(save_file, move_line);
		int move_val = stoi(move_line);

		if (move_val == COMPOUND) {
			string compound_index_line;
			getline(save_file, compound_index_line);
			int compound_index = stoi(compound_index_line);
			Action a(compound_index);
			this->children_actions.push_back(a);
		} else {
			string write_line;
			getline(save_file, write_line);
			double write_val = stof(write_line);
			Action a(write_val, move_val);
			this->children_actions.push_back(a);
		}

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

	string score_network_name_line;
	getline(save_file, score_network_name_line);
	boost::algorithm::trim(score_network_name_line);
	this->score_network_name = score_network_name_line;
	
	ifstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string certainty_network_name_line;
	getline(save_file, certainty_network_name_line);
	boost::algorithm::trim(certainty_network_name_line);
	this->certainty_network_name = certainty_network_name_line;

	ifstream certainty_network_save_file;
	certainty_network_save_file.open(this->certainty_network_name);
	this->certainty_network = new Network(certainty_network_save_file);
	certainty_network_save_file.close();

	this->explore_state = EXPLORE_STATE_EXPLORE;
	this->current_explore_id = (unsigned)time(NULL);
	this->candidate_iter = 0;
	this->best_information = numeric_limits<double>::lowest();

	this->learn_scores_network = NULL;
}

SolutionNode::~SolutionNode() {
	for (int c_index = 0; c_index < (int)this->children_indexes.size(); c_index++) {
		delete this->children_score_networks[c_index];
		delete this->children_information_networks[c_index];
	}

	delete this->score_network;
	delete this->certainty_network;

	if (this->learn_scores_network != NULL) {
		delete this->learn_scores_network;
	}
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
						   int& explore_status,
						   int& explore_id,
						   int& explore_candidate_iter,
						   bool& force_eval,
						   bool save_for_display,
						   ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	this->score_network->mtx.lock();
	this->score_network->activate(observations);
	double think = this->score_network->val_val->acti_vals[0];
	this->score_network->mtx.unlock();

	guesses.push_back(think);

	if (this->score_network->epoch < 10000) {
		if (save_for_display) {
			display_file << "new_node" << endl;
		}

		force_eval = true;

		// shouldn't have multiple children yet
		chosen_path = 0;
		explore_status = -1;
		explore_id = -1;
		explore_candidate_iter = -1;
		return;
	}

	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	// look for new child
	int new_index = -1;
	for (int c_index = 0; c_index < num_children; c_index++) {
		Network* child_score_network = this->children_score_networks[c_index];
		if (child_score_network->epoch < 20000) {
			new_index = c_index;
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
		explore_status = -1;
		explore_id = -1;
		explore_candidate_iter = -1;
		return;
	}

	if (rand()%20 == 0) {
		chosen_path = rand()%(int)num_children;
		explore_status = -1;
		explore_id = -1;
		explore_candidate_iter = -1;

		if (save_for_display) {
			display_file << "random" << endl;
			display_file << chosen_path << endl;
		}

		return;
	}

	this->certainty_network->mtx.lock();
	this->certainty_network->activate(observations);
	double uncertainty = 1.0 - this->certainty_network->val_val->acti_vals[0];
	this->certainty_network->mtx.unlock();

	if (save_for_display) {
		display_file << think << endl;
		display_file << uncertainty << endl;
	}

	if (!force_eval) {
		if (think+uncertainty < this->average_score) {
			// TODO: should compare if a child has even more certainty
			if (rand()%2 == 0) {
				if (save_for_display) {
					display_file << "good_explore" << endl;
				}

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
				if (save_for_display) {
					display_file << "random_explore" << endl;
				}

				this->explore_mtx.lock();
				chosen_path = -1;
				explore_status = this->explore_state;
				explore_id = this->current_explore_id;
				explore_candidate_iter = this->candidate_iter;
				this->explore_mtx.unlock();
				return;
			}
		}
	}

	double best_information = numeric_limits<double>::lowest();
	int best_index = -1;
	vector<double> predicted_scores;
	vector<double> predicted_misguesses;
	for (int c_index = 0; c_index < num_children; c_index++) {
		Network* child_score_network = this->children_score_networks[c_index];
		child_score_network->mtx.lock();
		child_score_network->activate(observations);
		double predicted_score = child_score_network->val_val->acti_vals[0];
		child_score_network->mtx.unlock();
		
		Network* child_information_network = this->children_information_networks[c_index];
		child_information_network->mtx.lock();
		child_information_network->activate(observations);
		double predicted_misguess = 1.0 - child_information_network->val_val->acti_vals[0];
		child_information_network->mtx.unlock();

		double predicted_information = predicted_score - predicted_misguess;

		if (save_for_display) {
			predicted_scores.push_back(predicted_score);
			predicted_misguesses.push_back(predicted_misguess);
		}

		if (predicted_information > best_information) {
			best_information = predicted_information;
			best_index = c_index;
		}
	}

	// explore if will simply hit halt again
	if (!force_eval && this->children_indexes[best_index] == 0) {
		if (save_for_display) {
			display_file << "forced_explore" << endl;
		}

		this->explore_mtx.lock();
		chosen_path = -1;
		explore_status = this->explore_state;
		explore_id = this->current_explore_id;
		explore_candidate_iter = this->candidate_iter;
		this->explore_mtx.unlock();
		return;
	}

	if (save_for_display) {
		display_file << "process" << endl;
		for (int c_index = 0; c_index < num_children; c_index++) {
			display_file << predicted_scores[c_index] << endl;
			display_file << predicted_misguesses[c_index] << endl;
		}
		display_file << best_index << endl;
	}

	chosen_path = best_index;
	explore_status = -1;
	explore_id = -1;
	explore_candidate_iter = -1;
	return;
}

void SolutionNode::update_self(vector<double> observations,
							   double score) {
	this->score_network->mtx.lock();
	this->score_network->activate(observations);
	vector<double> score_errors;
	double score_error = score - this->score_network->val_val->acti_vals[0];
	score_errors.push_back(score_error);
	this->score_network->backprop(score_errors);
	this->score_network->increment();

	// share lock
	this->average_score = 0.99999*this->average_score + 0.00001*score;

	this->score_network->mtx.unlock();

	this->certainty_network->mtx.lock();
	this->certainty_network->activate(observations);
	vector<double> certainty_errors;
	double certainty_target = 1.0 - abs(score_error);
	certainty_errors.push_back(certainty_target - this->certainty_network->val_val->acti_vals[0]);
	this->certainty_network->backprop(certainty_errors);
	this->certainty_network->increment();
	this->certainty_network->mtx.unlock();
}

void SolutionNode::update(vector<double> observations,
						  int chosen_path,
						  double score,
						  double misguess) {
	update_self(observations,
				score);

	Network* child_score_network = this->children_score_networks[chosen_path];
	child_score_network->mtx.lock();
	child_score_network->activate(observations);
	vector<double> score_errors;
	score_errors.push_back(score - child_score_network->val_val->acti_vals[0]);
	child_score_network->backprop(score_errors);
	child_score_network->increment();
	child_score_network->mtx.unlock();

	Network* child_information_network = this->children_information_networks[chosen_path];
	child_information_network->mtx.lock();
	child_information_network->activate(observations);
	vector<double> information_errors;
	double information_target = 1.0 - misguess;
	information_errors.push_back(information_target - child_information_network->val_val->acti_vals[0]);
	child_information_network->backprop(information_errors);
	child_information_network->increment();
	child_information_network->mtx.unlock();
}

void SolutionNode::update_explore(vector<double> observations,
								  vector<double> full_observations,
								  int explore_status,
								  int explore_id,
								  int explore_candidate_iter,
								  double score) {
	update_self(observations,
				score);

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

				int input_size = this->path_length;
				for (int a_index = 0; a_index < (int)this->current_candidate.size(); a_index++) {
					input_size += this->solver->action_dictionary->calculate_action_path_length(
						this->current_candidate[a_index]);
				}
				int network_size = 2*input_size*(3+input_size);
				this->learn_scores_network = new Network(input_size, network_size, 1);
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SCORES) {
			this->learn_scores_network->activate(full_observations);
			vector<double> errors;
			errors.push_back(score - this->learn_scores_network->val_val->acti_vals[0]);
			this->learn_scores_network->backprop(errors);
			this->learn_scores_network->increment();

			if (this->learn_scores_network->epoch == 10000) {
				this->explore_state = EXPLORE_STATE_MEASURE_INFORMATION;
				this->measure_information_p_index = 0;
				this->measure_information_think_good_and_good = 0.0;
				this->measure_information_think_good_but_bad = 0.0;
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_INFORMATION) {
			this->learn_scores_network->activate(full_observations);
			double think = this->learn_scores_network->val_val->acti_vals[0];

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

				cout << this->node_index << endl;
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

void SolutionNode::update_explore_candidate(int explore_status,
											int explore_id,
											int explore_candidate_iter,
											vector<Action> candidate) {
	this->explore_mtx.lock();
	if (explore_status == this->explore_state &&
			explore_id == this->current_explore_id &&
			explore_candidate_iter == this->candidate_iter) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			this->current_candidate = candidate;

			this->explore_state = EXPLORE_STATE_MEASURE_AVERAGE;
			this->measure_average_p_index = 0;
			this->measure_average_average_score = 0.0;
		}
	}
	this->explore_mtx.unlock();
}

void SolutionNode::save(ofstream& save_file) {
	save_file << this->path_length << endl;
	
	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	save_file << num_children << endl;
	for (int c_index = 0; c_index < num_children; c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		save_file << this->children_actions[c_index].move << endl;
		if (this->children_actions[c_index].move == COMPOUND) {
			save_file << this->children_actions[c_index].compound_index << endl;
		} else {
			save_file << this->children_actions[c_index].write << endl;
		}

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

	save_file << this->score_network_name << endl;
	ofstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->certainty_network_name << endl;
	ofstream certainty_network_save_file;
	certainty_network_save_file.open(this->certainty_network_name);
	this->certainty_network->save(certainty_network_save_file);
	certainty_network_save_file.close();
}

void SolutionNode::save_for_display(ofstream& save_file) {
	this->children_mtx.lock();
	int num_children = (int)this->children_indexes.size();
	this->children_mtx.unlock();

	save_file << num_children << endl;
	for (int c_index = 0; c_index < num_children; c_index++) {
		save_file << this->children_indexes[c_index] << endl;
		save_file << this->children_actions[c_index].move << endl;
		if (this->children_actions[c_index].move == COMPOUND) {
			save_file << this->children_actions[c_index].compound_index << endl;
		} else {
			save_file << this->children_actions[c_index].write << endl;
		}
	}
	save_file << this->average_score << endl;
}
