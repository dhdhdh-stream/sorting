#include "loop.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "utilities.h"

using namespace std;

Loop::Loop(vector<Action> front,
		   vector<Action> loop,
		   vector<Action> back) {
	loop_dictionary->counter_mtx.lock();
	int loop_counter = loop_dictionary->loop_counter;
	loop_dictionary->loop_counter++;
	loop_dictionary->counter_mtx.unlock();

	this->front = front;
	this->loop = loop;
	this->back = back;

	int front_size = 0;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		front_size += calculate_action_path_length(this->front[f_index]);
	}
	this->time_size = 0;
	for (int t_index = 0; t_index < (int)this->loop.size(); t_index++) {
		this->time_size += calculate_action_path_length(this->loop[t_index]);
	}

	geometric_distribution<int> score_state_size_dist(0.3);
	this->score_state_size = 1+score_state_size_dist(generator);
	int score_network_input_size = this->score_state_size + front_size + this->time_size;
	int score_network_size = 2*score_network_input_size*(3+score_network_input_size);
	this->score_network = new Network(score_network_input_size,
									  score_network_size,
									  this->score_state_size+1);
	this->score_network_name = "../saves/nns/l_s_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";

	geometric_distribution<int> certainty_state_size_dist(0.5);
	this->certainty_state_size = certainty_state_size_dist(generator);
	int certainty_network_input_size = this->certainty_state_size \
		+ front_size + this->time_size + this->score_state_size;
	int certainty_network_size = 2*certainty_network_input_size*(3+certainty_network_input_size);
	this->certainty_network = new Network(certainty_network_input_size,
										  certainty_network_size,
										  this->certainty_state_size+1);
	this->certainty_network_name = "../saves/nns/l_c_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";

	geometric_distribution<int> halt_state_size_dist(0.5);
	this->halt_state_size = halt_state_size_dist(generator);
	int halt_network_input_size = this->halt_state_size + front_size \
		+ this->time_size + this->score_state_size + this->certainty_state_size;
	int halt_network_size = 2*halt_network_input_size*(3+halt_network_input_size);
	this->halt_network = new Network(halt_network_input_size,
									 halt_network_size,
									 this->halt_state_size+1);
	this->halt_network_name = "../saves/nns/l_h_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";
}

Loop::Loop(ifstream& save_file) {
	string front_num_actions_line;
	getline(save_file, front_num_actions_line);
	int front_num_actions = stoi(front_num_actions_line);
	for (int f_index = 0; f_index < front_num_actions; f_index++) {
		Action a(save_file);
		this->front.push_back(a);
	}

	string loop_num_actions_line;
	getline(save_file, loop_num_actions_line);
	int loop_num_actions = stoi(loop_num_actions_line);
	for (int l_index = 0; l_index < loop_num_actions; l_index++) {
		Action a(save_file);
		this->loop.push_back(a);
	}

	string time_size_line;
	getline(save_file, time_size_line);
	this->time_size = stoi(time_size_line);

	string back_num_actions_line;
	getline(save_file, back_num_actions_line);
	int back_num_actions = stoi(back_num_actions_line);
	for (int b_index = 0; b_index < back_num_actions; b_index++) {
		Action a(save_file);
		this->back.push_back(a);
	}

	string score_state_size_line;
	getline(save_file, score_state_size_line);
	this->score_state_size = stoi(score_state_size_line);

	string score_network_name_line;
	getline(save_file, score_network_name_line);
	boost::algorithm::trim(score_network_name_line);
	this->score_network_name = score_network_name_line;

	ifstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string certainty_state_size_line;
	getline(save_file, certainty_state_size_line);
	this->certainty_state_size = stoi(certainty_state_size_line);

	string certainty_network_name_line;
	getline(save_file, certainty_network_name_line);
	boost::algorithm::trim(certainty_network_name_line);
	this->certainty_network_name = certainty_network_name_line;

	ifstream certainty_network_save_file;
	certainty_network_save_file.open(this->certainty_network_name);
	this->certainty_network = new Network(certainty_network_save_file);
	certainty_network_save_file.close();

	string halt_state_size_line;
	getline(save_file, halt_state_size_line);
	this->halt_state_size = stoi(halt_state_size_line);

	string halt_network_name_line;
	getline(save_file, halt_network_name_line);
	boost::algorithm::trim(halt_network_name_line);
	this->halt_network_name = halt_network_name_line;

	ifstream halt_network_save_file;
	halt_network_save_file.open(this->halt_network_name);
	this->halt_network = new Network(halt_network_save_file);
	halt_network_save_file.close();
}

Loop::~Loop() {
	delete this->score_network;
	delete this->certainty_network;
	delete this->halt_network;
}

void Loop::train(Problem* p,
				 double& score,
				 bool save_for_display,
				 vector<Action>* raw_actions) {
	vector<double> front_vals;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		p->perform_action(this->front[f_index],
						  &front_vals,
						  save_for_display,
						  raw_actions);
	}

	int iterations = rand()%6;

	if (this->score_network->epoch < 50000) {
		if (iterations == 0) {
			this->score_network->mtx.lock();
			
			vector<double> score_inputs;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_inputs.push_back(0.0);
			}
			score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				score_inputs.push_back(0.0);
			}
			this->score_network->activate(score_inputs);
			double predicted_score = this->score_network->val_val->acti_vals[this->score_state_size];

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p->perform_action(this->back[b_index],
								  NULL,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			vector<double> score_errors;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_errors.push_back(0.0);
			}
			if (score == 1.0) {
				if (predicted_score < 1.0) {
					score_errors.push_back(score - predicted_score);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (predicted_score > 0.0) {
					score_errors.push_back(score - predicted_score);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->score_network->backprop(score_errors);
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				this->score_network->input->errors[s_index] = 0.0;
			}
			this->score_network->increment();
			this->score_network->mtx.unlock();
		} else {
			vector<NetworkHistory*> score_network_historys;
			double score_state[this->score_state_size] = {};
			double predicted_score;
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				vector<double> time_val;
				for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
					p->perform_action(this->loop[l_index],
									  &time_val,
									  save_for_display,
									  raw_actions);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->mtx.lock();
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}
				predicted_score = this->score_network->val_val->acti_vals[this->score_state_size];
				this->score_network->mtx.unlock();
			}

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p->perform_action(this->back[b_index],
								  NULL,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			this->score_network->mtx.lock();
			double score_state_errors[this->score_state_size] = {};

			// top iteration
			vector<double> score_errors;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_errors.push_back(score_state_errors[s_index]);	// 0.0
			}
			if (score == 1.0) {
				if (predicted_score < 1.0) {
					score_errors.push_back(score - predicted_score);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (predicted_score > 0.0) {
					score_errors.push_back(score - predicted_score);
				} else {
					score_errors.push_back(0.0);
				}
			}
			score_network_historys[iterations-1]->reset_weights();
			this->score_network->backprop(score_errors);
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_state_errors[s_index] = this->score_network->input->errors[s_index];
				this->score_network->input->errors[s_index] = 0.0;
			}

			for (int iter_index = iterations-2; iter_index >= 0; iter_index--) {
				vector<double> score_errors;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_errors.push_back(score_state_errors[s_index]);
				}
				score_errors.push_back(0.0);

				score_network_historys[iter_index]->reset_weights();
				this->score_network->backprop(score_errors);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state_errors[s_index] = this->score_network->input->errors[s_index];
					this->score_network->input->errors[s_index] = 0.0;
				}
			}
			this->score_network->increment();
			for (int i = 0; i < (int)score_network_historys.size(); i++) {
				delete score_network_historys[i];
			}
			this->score_network->mtx.unlock();
		}
	} else if (this->certainty_network->epoch < 30000
			|| iterations == 0) {	// special 0-iter case for halt network
		if (iterations == 0) {
			scoped_lock lock(this->score_network->mtx, this->certainty_network->mtx);

			vector<double> score_inputs;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_inputs.push_back(0.0);
			}
			score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				score_inputs.push_back(0.0);
			}
			this->score_network->activate(score_inputs);
			double predicted_score = this->score_network->val_val->acti_vals[this->score_state_size];

			vector<double> certainty_inputs;
			for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
				certainty_inputs.push_back(0.0);
			}
			certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				certainty_inputs.push_back(0.0);
			}
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				certainty_inputs.push_back(0.0);
			}
			this->certainty_network->activate(certainty_inputs);
			double predicted_uncertainty = this->certainty_network->val_val->acti_vals[this->certainty_state_size];

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p->perform_action(this->back[b_index],
								  NULL,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			vector<double> score_errors;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_errors.push_back(0.0);
			}
			double score_error;
			if (score == 1.0) {
				if (predicted_score < 1.0) {
					score_error = score - predicted_score;
				} else {
					score_error = 0.0;
				}
			} else {
				if (predicted_score > 0.0) {
					score_error = score - predicted_score;
				} else {
					score_error = 0.0;
				}
			}
			score_errors.push_back(score_error);
			this->score_network->backprop(score_errors);
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				this->score_network->input->errors[s_index] = 0.0;
			}
			this->score_network->increment();

			vector<double> certainty_errors;
			for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
				certainty_errors.push_back(0.0);
			}
			double certainty_error;
			if (score_error == 0.0 && predicted_uncertainty < 0.0) {
				certainty_error = 0.0;
			} else {
				certainty_error = abs(score_error) - predicted_uncertainty;
			}
			certainty_errors.push_back(certainty_error);
			this->certainty_network->backprop(certainty_errors);
			for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
				this->certainty_network->input->errors[s_index] = 0.0;
			}
			this->certainty_network->increment();
		} else {
			vector<NetworkHistory*> score_network_historys;
			double score_state[this->score_state_size] = {};
			vector<NetworkHistory*> certainty_network_historys;
			double certainty_state[this->certainty_state_size] = {};
			vector<double> predicted_scores_1_start;
			vector<double> predicted_uncertaintys_1_start;
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				vector<double> time_val;
				for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
					p->perform_action(this->loop[l_index],
									  &time_val,
									  save_for_display,
									  raw_actions);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->mtx.lock();
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}
				predicted_scores_1_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);
				this->score_network->mtx.unlock();

				vector<double> certainty_inputs;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_inputs.push_back(certainty_state[s_index]);
				}
				certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
				certainty_inputs.insert(certainty_inputs.end(), time_val.begin(), time_val.end());
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					certainty_inputs.push_back(score_state[s_index]);
				}
				this->certainty_network->mtx.lock();
				this->certainty_network->activate(certainty_inputs, certainty_network_historys);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state[s_index] = this->certainty_network->val_val->acti_vals[s_index];
				}
				predicted_uncertaintys_1_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);
				this->certainty_network->mtx.unlock();
			}

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p->perform_action(this->back[b_index],
								  NULL,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			this->score_network->mtx.lock();
			double score_state_errors[this->score_state_size] = {};

			// top iteration
			vector<double> score_errors;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_errors.push_back(score_state_errors[s_index]);	// 0.0
			}
			if (score == 1.0) {
				if (predicted_scores_1_start[iterations-1] < 1.0) {
					score_errors.push_back(score - predicted_scores_1_start[iterations-1]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (predicted_scores_1_start[iterations-1] > 0.0) {
					score_errors.push_back(score - predicted_scores_1_start[iterations-1]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			score_network_historys[iterations-1]->reset_weights();
			this->score_network->backprop(score_errors);
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_state_errors[s_index] = this->score_network->input->errors[s_index];
				this->score_network->input->errors[s_index] = 0.0;
			}

			for (int iter_index = iterations-2; iter_index >= 0; iter_index--) {
				vector<double> score_errors;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_errors.push_back(score_state_errors[s_index]);
				}
				score_errors.push_back(0.0);
				score_network_historys[iter_index]->reset_weights();
				this->score_network->backprop(score_errors);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state_errors[s_index] = this->score_network->input->errors[s_index];
					this->score_network->input->errors[s_index] = 0.0;
				}
			}

			this->score_network->increment();
			for (int i = 0; i < (int)score_network_historys.size(); i++) {
				delete score_network_historys[i];
			}
			this->score_network->mtx.unlock();

			this->certainty_network->mtx.lock();
			double certainty_state_errors[this->certainty_state_size] = {};
			for (int iter_index = iterations-1; iter_index >= 0; iter_index--) {
				certainty_network_historys[iter_index]->reset_weights();
				vector<double> certainty_errors;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_errors.push_back(certainty_state_errors[s_index]);
				}
				double score_error;
				if (score == 1.0) {
					if (predicted_scores_1_start[iter_index] < 1.0) {
						score_error = score - predicted_scores_1_start[iter_index];
					} else {
						score_error = 0.0;
					}
				} else {
					if (predicted_scores_1_start[iter_index] > 0.0) {
						score_error = score - predicted_scores_1_start[iter_index];
					} else {
						score_error = 0.0;
					}
				}
				double certainty_error;
				if (score_error == 0.0 && predicted_uncertaintys_1_start[iter_index] < 0.0) {
					certainty_error = 0.0;
				} else {
					certainty_error = abs(score_error) - predicted_uncertaintys_1_start[iter_index];
				}
				certainty_errors.push_back(certainty_error);
				this->certainty_network->backprop(certainty_errors);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state_errors[s_index] = this->certainty_network->input->errors[s_index];
					this->certainty_network->input->errors[s_index] = 0.0;
				}
			}
			this->certainty_network->increment();
			for (int i = 0; i < (int)certainty_network_historys.size(); i++) {
				delete certainty_network_historys[i];
			}
			this->certainty_network->mtx.unlock();
		}
	} else {
		if (iterations == 0) {
			// can't reach, and halt network won't update anyways
		} else {
			vector<double> predicted_scores_0_start;
			vector<double> predicted_uncertaintys_0_start;
			vector<double> predicted_gains_0_start;

			vector<NetworkHistory*> halt_network_historys;

			// 0th iteration
			vector<double> score_inputs;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_inputs.push_back(0.0);
			}
			score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				score_inputs.push_back(0.0);
			}
			this->score_network->mtx.lock();
			this->score_network->activate(score_inputs);
			predicted_scores_0_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);
			this->score_network->mtx.unlock();

			vector<double> certainty_inputs;
			for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
				certainty_inputs.push_back(0.0);
			}
			certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				certainty_inputs.push_back(0.0);
			}
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				certainty_inputs.push_back(0.0);
			}
			this->certainty_network->mtx.lock();
			this->certainty_network->activate(certainty_inputs);
			predicted_uncertaintys_0_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);
			this->certainty_network->mtx.unlock();

			vector<double> halt_inputs;
			for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
				halt_inputs.push_back(0.0);
			}
			halt_inputs.insert(halt_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				halt_inputs.push_back(0.0);
			}
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				halt_inputs.push_back(0.0);
			}
			for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
				halt_inputs.push_back(0.0);
			}
			this->halt_network->mtx.lock();
			this->halt_network->activate(halt_inputs, halt_network_historys);
			predicted_gains_0_start.push_back(this->halt_network->val_val->acti_vals[this->halt_state_size]);
			this->halt_network->mtx.unlock();

			vector<NetworkHistory*> score_network_historys;
			double score_state[this->score_state_size] = {};
			vector<NetworkHistory*> certainty_network_historys;
			double certainty_state[this->certainty_state_size] = {};
			double halt_state[this->halt_state_size] = {};
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				vector<double> time_val;
				for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
					p->perform_action(this->loop[l_index],
									  &time_val,
									  save_for_display,
									  raw_actions);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->mtx.lock();
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}
				predicted_scores_0_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);
				this->score_network->mtx.unlock();

				vector<double> certainty_inputs;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_inputs.push_back(certainty_state[s_index]);
				}
				certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
				certainty_inputs.insert(certainty_inputs.end(), time_val.begin(), time_val.end());
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					certainty_inputs.push_back(score_state[s_index]);
				}
				this->certainty_network->mtx.lock();
				this->certainty_network->activate(certainty_inputs, certainty_network_historys);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state[s_index] = this->certainty_network->val_val->acti_vals[s_index];
				}
				predicted_uncertaintys_0_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);
				this->certainty_network->mtx.unlock();

				vector<double> halt_inputs;
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_inputs.push_back(halt_state[s_index]);
				}
				halt_inputs.insert(halt_inputs.end(), front_vals.begin(), front_vals.end());
				halt_inputs.insert(halt_inputs.end(), time_val.begin(), time_val.end());
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					halt_inputs.push_back(score_state[s_index]);
				}
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					halt_inputs.push_back(certainty_state[s_index]);
				}
				this->halt_network->mtx.lock();
				this->halt_network->activate(halt_inputs, halt_network_historys);
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_state[s_index] = this->halt_network->val_val->acti_vals[s_index];
				}
				predicted_gains_0_start.push_back(this->halt_network->val_val->acti_vals[this->halt_state_size]);
				this->halt_network->mtx.unlock();
			}

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p->perform_action(this->back[b_index],
								  NULL,
								  save_for_display,
								  raw_actions);
			}
			score = p->score_result();

			this->score_network->mtx.lock();
			double score_state_errors[this->score_state_size] = {};

			// top iteration
			score_network_historys[iterations-1]->reset_weights();
			vector<double> score_errors;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_errors.push_back(score_state_errors[s_index]);	// 0.0
			}
			if (score == 1.0) {
				if (predicted_scores_0_start[iterations] < 1.0) {
					score_errors.push_back(score - predicted_scores_0_start[iterations]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (predicted_scores_0_start[iterations] > 0.0) {
					score_errors.push_back(score - predicted_scores_0_start[iterations]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->score_network->backprop(score_errors);
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_state_errors[s_index] = this->score_network->input->errors[s_index];
				this->score_network->input->errors[s_index] = 0.0;
			}

			for (int iter_index = iterations-2; iter_index >= 0; iter_index--) {
				score_network_historys[iter_index]->reset_weights();
				vector<double> score_errors;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_errors.push_back(score_state_errors[s_index]);
				}
				score_errors.push_back(0.0);
				this->score_network->backprop(score_errors);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state_errors[s_index] = this->score_network->input->errors[s_index];
					this->score_network->input->errors[s_index] = 0.0;
				}
			}

			this->score_network->increment();
			for (int i = 0; i < (int)score_network_historys.size(); i++) {
				delete score_network_historys[i];
			}
			this->score_network->mtx.unlock();

			this->certainty_network->mtx.lock();
			double certainty_state_errors[this->certainty_state_size] = {};
			for (int iter_index = iterations-1; iter_index >= 0; iter_index--) {
				certainty_network_historys[iter_index]->reset_weights();
				vector<double> certainty_errors;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_errors.push_back(certainty_state_errors[s_index]);
				}
				double score_error;
				if (score == 1.0) {
					if (predicted_scores_0_start[iter_index+1] < 1.0) {
						score_error = score - predicted_scores_0_start[iter_index+1];
					} else {
						score_error = 0.0;
					}
				} else {
					if (predicted_scores_0_start[iter_index+1] > 0.0) {
						score_error = score - predicted_scores_0_start[iter_index+1];
					} else {
						score_error = 0.0;
					}
				}
				double certainty_error;
				if (score_error == 0.0 && predicted_uncertaintys_0_start[iter_index+1] < 0.0) {
					certainty_error = 0.0;
				} else {
					certainty_error = abs(score_error) - predicted_uncertaintys_0_start[iter_index+1];
				}
				certainty_errors.push_back(certainty_error);
				this->certainty_network->backprop(certainty_errors);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state_errors[s_index] = this->certainty_network->input->errors[s_index];
					this->certainty_network->input->errors[s_index] = 0.0;
				}
			}
			this->certainty_network->increment();
			for (int i = 0; i < (int)certainty_network_historys.size(); i++) {
				delete certainty_network_historys[i];
			}
			this->certainty_network->mtx.unlock();

			this->halt_network->mtx.lock();
			double halt_state_errors[this->halt_state_size] = {};
			// discard last iteration (i.e., iterations+1)
			for (int iter_index = iterations-1; iter_index >= 0; iter_index--) {
				halt_network_historys[iter_index]->reset_weights();
				vector<double> halt_errors;
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_errors.push_back(halt_state_errors[s_index]);
				}
				double next_predicted_score = max(min(predicted_scores_0_start[iter_index+1], 1.0), 0.0);
				double curr_predicted_score = max(min(predicted_scores_0_start[iter_index], 1.0), 0.0);
				double score_gain = next_predicted_score - curr_predicted_score;
				double next_predicted_uncertainty = max(predicted_uncertaintys_0_start[iter_index+1], 0.0);
				double curr_predicted_uncertainty = max(predicted_uncertaintys_0_start[iter_index], 0.0);
				double info_gain = curr_predicted_uncertainty - next_predicted_uncertainty;
				double halt_error;
				if (score_gain+info_gain > 0.0) {
					if (predicted_gains_0_start[iter_index] < 1.0) {
						halt_error = 1.0 - predicted_gains_0_start[iter_index];
					} else {
						halt_error = 0.0;
					}
				} else {
					if (predicted_gains_0_start[iter_index] > 0.0) {
						halt_error = 0.0 - predicted_gains_0_start[iter_index];
					} else {
						halt_error = 0.0;
					}
				}
				halt_errors.push_back(halt_error);
				this->halt_network->backprop(halt_errors);
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_state_errors[s_index] = this->halt_network->input->errors[s_index];
					this->halt_network->input->errors[s_index] = 0.0;
				}
			}
			this->halt_network->increment();
			for (int i = 0; i < (int)halt_network_historys.size(); i++) {
				delete halt_network_historys[i];
			}
			this->halt_network->mtx.unlock();
		}
	}
}

void Loop::pass_through(Problem* p,
						vector<double>* observations,
						bool save_for_display,
						vector<Action>* raw_actions) {
	vector<double> front_vals;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		p->perform_action(this->front[f_index],
						  &front_vals,
						  save_for_display,
						  raw_actions);
	}

	// 0th iteration
	vector<double> halt_inputs;
	for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
		halt_inputs.push_back(0.0);
	}
	halt_inputs.insert(halt_inputs.end(), front_vals.begin(), front_vals.end());
	for (int t_index = 0; t_index < this->time_size; t_index++) {
		halt_inputs.push_back(0.0);
	}
	for (int s_index = 0; s_index < this->score_state_size; s_index++) {
		halt_inputs.push_back(0.0);
	}
	for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
		halt_inputs.push_back(0.0);
	}
	this->halt_network->mtx.lock();
	this->halt_network->activate(halt_inputs);
	double halt = this->halt_network->val_val->acti_vals[this->halt_state_size];
	this->halt_network->mtx.unlock();

	if (halt < 0.0) {
		observations->insert(observations->end(), front_vals.begin(), front_vals.end());
		for (int s_index = 0; s_index < this->score_state_size; s_index++) {
			observations->push_back(0.0);
		}
		for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
			observations->push_back(0.0);
		}
		for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
			observations->push_back(0.0);
		}
		observations->push_back(0);
		for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
			p->perform_action(this->back[b_index],
							  observations,
							  save_for_display,
							  raw_actions);
		}
		return;
	}

	double score_state[this->score_state_size] = {};
	double certainty_state[this->certainty_state_size] = {};
	double halt_state[this->halt_state_size] = {};

	int iter_index = 0;	
	while (true) {
		iter_index++;
		// don't loop more than 20 iterations for now
		if (iter_index > 20) {
			break;
		}

		vector<double> time_val;
		for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
			p->perform_action(this->loop[l_index],
							  &time_val,
							  save_for_display,
							  raw_actions);
		}

		vector<double> score_inputs;
		for (int s_index = 0; s_index < this->score_state_size; s_index++) {
			score_inputs.push_back(score_state[s_index]);
		}
		score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
		score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
		this->score_network->mtx.lock();
		this->score_network->activate(score_inputs);
		for (int s_index = 0; s_index < this->score_state_size; s_index++) {
			score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
		}
		this->score_network->mtx.unlock();

		vector<double> certainty_inputs;
		for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
			certainty_inputs.push_back(certainty_state[s_index]);
		}
		certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
		certainty_inputs.insert(certainty_inputs.end(), time_val.begin(), time_val.end());
		for (int s_index = 0; s_index < this->score_state_size; s_index++) {
			certainty_inputs.push_back(score_state[s_index]);
		}
		this->certainty_network->mtx.lock();
		this->certainty_network->activate(certainty_inputs);
		for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
			certainty_state[s_index] = this->certainty_network->val_val->acti_vals[s_index];
		}
		this->certainty_network->mtx.unlock();

		vector<double> halt_inputs;
		for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
			halt_inputs.push_back(halt_state[s_index]);
		}
		halt_inputs.insert(halt_inputs.end(), front_vals.begin(), front_vals.end());
		halt_inputs.insert(halt_inputs.end(), time_val.begin(), time_val.end());
		for (int s_index = 0; s_index < this->score_state_size; s_index++) {
			halt_inputs.push_back(score_state[s_index]);
		}
		for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
			halt_inputs.push_back(certainty_state[s_index]);
		}
		this->halt_network->mtx.lock();
		this->halt_network->activate(halt_inputs);
		for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
			halt_state[s_index] = this->halt_network->val_val->acti_vals[s_index];
		}
		double halt = this->halt_network->val_val->acti_vals[this->halt_state_size];
		this->halt_network->mtx.unlock();

		if (halt < 0.0) {
			break;
		}
	}

	observations->insert(observations->begin(), front_vals.begin(), front_vals.end());
	for (int s_index = 0; s_index < this->score_state_size; s_index++) {
		observations->push_back(score_state[s_index]);
	}
	for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
		observations->push_back(certainty_state[s_index]);
	}
	for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
		observations->push_back(halt_state[s_index]);
	}
	observations->push_back(iter_index);
	for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
		p->perform_action(this->back[b_index],
						  observations,
						  save_for_display,
						  raw_actions);
	}
	return;
}

int Loop::path_length() {
	int sum_length = 0;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		sum_length += calculate_action_path_length(this->front[f_index]);
	}
	sum_length += this->score_state_size;
	sum_length += this->certainty_state_size;
	sum_length += this->halt_state_size;
	for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
		sum_length += calculate_action_path_length(this->back[b_index]);
	}

	sum_length += 1;	// number of iterations

	return sum_length;
}

void Loop::save(ofstream& save_file) {
	save_file << this->front.size() << endl;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		this->front[f_index].save(save_file);
	}

	save_file << this->loop.size() << endl;
	for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
		this->loop[l_index].save(save_file);
	}

	save_file << this->time_size << endl;

	save_file << this->back.size() << endl;
	for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
		this->back[b_index].save(save_file);
	}

	save_file << this->score_state_size << endl;

	save_file << this->score_network_name << endl;
	ofstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->certainty_state_size << endl;

	save_file << this->certainty_network_name << endl;
	ofstream certainty_network_save_file;
	certainty_network_save_file.open(this->certainty_network_name);
	this->certainty_network->save(certainty_network_save_file);
	certainty_network_save_file.close();

	save_file << this->halt_state_size << endl;

	save_file << this->halt_network_name << endl;
	ofstream halt_network_save_file;
	halt_network_save_file.open(this->halt_network_name);
	this->halt_network->save(halt_network_save_file);
	halt_network_save_file.close();
}
