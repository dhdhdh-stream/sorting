#include "loop.h"

#include "definitions.h"

using namespace std;

Loop::Loop(int path_length,
		   vector<Action> front,
		   vector<Action> loop,
		   vector<Action> back,
		   int loop_counter,
		   ActionDictionary* action_dictionary) {
	this->front = front;
	this->loop = loop;
	this->back = back;

	this->front_size = path_length;
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		this->front_size += action_dictionary->calculate_action_path_length(this->front[f_index]);
	}
	this->time_size = 0;
	for (int t_index = 0; t_index < (int)this->loop.size(); t_index++) {
		this->time_size += action_dictionary->calculate_action_path_length(this->loop[t_index]);
	}

	geometric_distribution<int> score_state_size_dist(0.3);
	this->score_state_size = 1+score_state_size_dist(generator);
	int score_network_input_size = this->score_state_size + this->front_size + this->time_size;
	int score_network_size = 2*score_network_input_size*(3+score_network_input_size);
	this->score_network = new Network(score_network_input_size,
									  score_network_size,
									  this->score_state_size+1);
	this->score_network_name = "../saves/nns/l_s_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";

	geometric_distribution<int> certainty_state_size_dist(0.5);
	this->certainty_state_size = certainty_state_size_dist(generator);
	int certainty_network_input_size = this->certainty_state_size \
		+ this->front_size + this->time_size + this->score_state_size;
	int certainty_network_size = 2*certainty_network_input_size*(3+certainty_network_input_size);
	this->certainty_network = new Network(certainty_network_input_size,
										  certainty_network_size,
										  this->certainty_state_size+1);
	this->certainty_network_name = "../saves/nns/l_c_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";

	geometric_distribution<int> halt_state_size_dist(0.5);
	this->halt_state_size = halt_state_size_dist(generator);
	int halt_network_input_size = this->halt_state_size + this->front_size \
		+ this->time_size + this->score_state_size + this->certainty_state_size;
	int halt_network_size = 2*halt_network_input_size*(3+halt_network_input_size);
	this->halt_network = new Network(halt_network_input_size,
									 halt_network_size,
									 this->halt_state_size+1);
	this->halt_network_name = "../saves/nns/l_h_" + \
		to_string(loop_counter) + to_string(time(NULL)) + ".txt";
}

Loop::Loop(ifstream& save_file) {

}

Loop::~Loop() {
	delete this->score_network;
	delete this->certainty_network;
	delete this->halt_network;
}

void Loop::train(std::vector<double>& observations,
				 Problem& p,
				 ActionDictionary* action_dictionary,
				 double& score) {
	for (int f_index = 0; f_index < (int)this->front.size(); f_index++) {
		p.perform_action(this->front[f_index],
						 observations,
						 action_dictionary);
	}
	vector<double> front_vals(observations);

	int iterations = rand()%6;

	if (this->score_network->epoch < 20000) {
		if (iterations == 0) {
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
				p.perform_action(this->back[b_index],
								 observations,
								 action_dictionary);
			}
			double score = p.score_result();

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
		} else {
			vector<NetworkHistory*> score_network_historys;
			double score_state[this->score_state_size] = {};
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				vector<double> time_val;
				for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
					p.perform_action(this->loop[l_index],
									 time_val,
									 action_dictionary);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}

				observations.insert(observations.begin(), time_val.begin(), time_val.end());
			}

			double predicted_score = this->score_network->val_val->acti_vals[this->score_state_size];

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p.perform_action(this->back[b_index],
								 observations,
								 action_dictionary);
			}
			double score = p.score_result();

			double score_state_errors[this->score_state_size] = {};

			// top iteration
			score_network_historys[iterations-1]->reset_weights();
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
		}
	} else if (this->certainty_network->epoch < 20000 \
			|| iterations == 0) {	// special 0-iter case for halt network
		if (iterations == 0) {
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
				p.perform_action(this->back[b_index],
								 observations,
								 action_dictionary);
			}
			double score = p.score_result();

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
					p.perform_action(this->loop[l_index],
									 time_val,
									 action_dictionary);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}

				predicted_scores_1_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);

				vector<double> certainty_inputs;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_inputs.push_back(certainty_state[s_index]);
				}
				certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
				certainty_inputs.insert(certainty_inputs.end(), time_val.begin(), time_val.end());
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					certainty_inputs.push_back(score_state[s_index]);
				}
				this->certainty_network->activate(certainty_inputs, certainty_network_historys);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state[s_index] = this->certainty_network->val_val->acti_vals[s_index];
				}

				predicted_uncertaintys_1_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);

				observations.insert(observations.begin(), time_val.begin(), time_val.end());
			}

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p.perform_action(this->back[b_index],
								 observations,
								 action_dictionary);
			}
			double score = p.score_result();

			double score_state_errors[this->score_state_size] = {};

			// top iteration
			score_network_historys[iterations-1]->reset_weights();
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
		}
	} else {
		if (iterations == 0) {
			// can't reach, and halt network won't update anyways
		} else {
			vector<double> predicted_scores_0_start;
			vector<double> predicted_uncertaintys_0_start;
			vector<double> predicted_gains_1_start;

			// 0th iteration
			vector<double> score_inputs;
			for (int s_index = 0; s_index < this->score_state_size; s_index++) {
				score_inputs.push_back(0.0);
			}
			score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
			for (int t_index = 0; t_index < this->time_size; t_index++) {
				score_inputs.push_back(0.0);
			}
			this->score_network->activate(score_inputs);
			predicted_scores_0_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);

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
			predicted_uncertaintys_0_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);

			vector<NetworkHistory*> score_network_historys;
			double score_state[this->score_state_size] = {};
			vector<NetworkHistory*> certainty_network_historys;
			double certainty_state[this->certainty_state_size] = {};
			vector<NetworkHistory*> halt_network_historys;
			double halt_state[this->halt_state_size] = {};
			for (int iter_index = 0; iter_index < iterations; iter_index++) {
				vector<double> time_val;
				for (int l_index = 0; l_index < (int)this->loop.size(); l_index++) {
					p.perform_action(this->loop[l_index],
									 time_val,
									 action_dictionary);
				}

				vector<double> score_inputs;
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_inputs.push_back(score_state[s_index]);
				}
				score_inputs.insert(score_inputs.end(), front_vals.begin(), front_vals.end());
				score_inputs.insert(score_inputs.end(), time_val.begin(), time_val.end());
				this->score_network->activate(score_inputs, score_network_historys);
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					score_state[s_index] = this->score_network->val_val->acti_vals[s_index];
				}

				predicted_scores_0_start.push_back(this->score_network->val_val->acti_vals[this->score_state_size]);

				vector<double> certainty_inputs;
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_inputs.push_back(certainty_state[s_index]);
				}
				certainty_inputs.insert(certainty_inputs.end(), front_vals.begin(), front_vals.end());
				certainty_inputs.insert(certainty_inputs.end(), time_val.begin(), time_val.end());
				for (int s_index = 0; s_index < this->score_state_size; s_index++) {
					certainty_inputs.push_back(score_state[s_index]);
				}
				this->certainty_network->activate(certainty_inputs, certainty_network_historys);
				for (int s_index = 0; s_index < this->certainty_state_size; s_index++) {
					certainty_state[s_index] = this->certainty_network->val_val->acti_vals[s_index];
				}

				predicted_uncertaintys_0_start.push_back(this->certainty_network->val_val->acti_vals[this->certainty_state_size]);

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
				this->halt_network->activate(halt_inputs, halt_network_historys);
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_state[s_index] = this->halt_network->val_val->acti_vals[s_index];
				}

				predicted_gains_1_start.push_back(this->halt_network->val_val->acti_vals[this->halt_state_size]);

				observations.insert(observations.begin(), time_val.begin(), time_val.end());
			}

			for (int b_index = 0; b_index < (int)this->back.size(); b_index++) {
				p.perform_action(this->back[b_index],
								 observations,
								 action_dictionary);
			}
			double score = p.score_result();

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

			double halt_state_errors[this->halt_state_size] = {};
			for (int iter_index = iterations-1; iter_index >= 0; iter_index--) {
				halt_network_historys[iter_index]->reset_weights();
				vector<double> halt_errors;
				for (int s_index = 0; s_index < this->halt_state_size; s_index++) {
					halt_errors.push_back(halt_state_errors[s_index]);
				}
				double score_gain = predicted_scores_0_start[iter_index+1] - predicted_scores_0_start[iter_index];
				double info_gain = predicted_uncertaintys_0_start[iter_index+1] - predicted_uncertaintys_0_start[iter_index];
				double halt_error = (score_gain+info_gain) - predicted_gains_1_start[iter_index];
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
		}
	}
}

void Loop::save(ofstream& save_file) {

}
