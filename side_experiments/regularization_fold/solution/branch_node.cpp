#include "branch_node.h"

using namespace std;

BranchNode::BranchNode(Scope* parent,
					   int id,
					   vector<int> branch_scope_context,
					   vector<int> branch_node_context,
					   bool branch_is_pass_through,
					   ScoreNetwork* branch_score_network,
					   ScoreNetwork* branch_misguess_network,
					   int branch_next_node_id,
					   ScoreNetwork* original_score_network,
					   ScoreNetwork* original_misguess_network,
					   int original_next_node_id,
					   double branch_weight) {
	this->type = NODE_TYPE_BRANCH;

	this->parent = parent;
	this->id = id;

	this->branch_scope_context = branch_scope_context;
	this->branch_node_context = branch_node_context;
	this->branch_is_pass_through = branch_is_pass_through;
	this->branch_score_network = branch_score_network;
	this->branch_misguess_network = branch_misguess_network;
	this->branch_next_node_id = branch_next_node_id;
	this->original_score_network = original_score_network;
	this->original_misguess_network = original_misguess_network;
	this->original_next_node_id = original_next_node_id;
	this->branch_weight = branch_weight;
}

BranchNode::BranchNode(ifstream& input_file,
					   Scope* parent,
					   int id) {
	this->type = NODE_TYPE_BRANCH;

	this->parent = parent;
	this->id = id;

	string context_size_line;
	getline(input_file, context_size_line);
	int context_size = stoi(context_size_line);
	for (int c_index = 0; c_index < context_size; c_index++) {
		string scope_context_line;
		getline(input_file, scope_context_line);
		this->branch_scope_context.push_back(stoi(scope_context_line));

		string node_context_line;
		getline(input_file, node_context_line);
		this->branch_node_context.push_back(stoi(node_context_line));
	}

	string is_pass_through_line;
	getline(input_file, is_pass_through_line);
	this->branch_is_pass_through = stoi(is_pass_through_line);

	if (!this->branch_is_pass_through) {
		ifstream branch_score_network_save_file;
		branch_score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_branch_score.txt");
		this->branch_score_network = new ScoreNetwork(branch_score_network_save_file);
		branch_score_network_save_file.close();

		ifstream branch_misguess_network_save_file;
		branch_misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_branch_misguess.txt");
		this->branch_misguess_network = new ScoreNetwork(branch_misguess_network_save_file);
		branch_misguess_network_save_file.close();
	}

	string branch_next_node_id_line;
	getline(input_file, branch_next_node_id_line);
	this->branch_next_node_id = stoi(branch_next_node_id_line);

	if (!this->branch_is_pass_through) {
		ifstream original_score_network_save_file;
		original_score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_original_score.txt");
		this->original_score_network = new ScoreNetwork(original_score_network_save_file);
		original_score_network_save_file.close();

		ifstream original_misguess_network_save_file;
		original_misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_original_misguess.txt");
		this->original_misguess_network = new ScoreNetwork(original_misguess_network_save_file);
		original_misguess_network_save_file.close();
	}

	string original_next_node_id_line;
	getline(input_file, original_next_node_id_line);
	this->original_next_node_id = stoi(original_next_node_id_line);

	string branch_weight_line;
	getline(input_file, branch_weight_line);
	this->branch_weight = stod(branch_weight_line);
}

BranchNode::~BranchNode() {
	if (this->branch_score_network != NULL) {
		delete this->branch_score_network;
	}
	
	if (this->branch_misguess_network != NULL) {
		delete this->branch_misguess_network;
	}

	if (this->original_score_network != NULL) {
		delete this->original_score_network;
	}

	if (this->original_misguess_network != NULL) {
		delete this->original_misguess_network;
	}
}

void BranchNode::activate(vector<ForwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope_id
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			history->is_branch = true;

			// no need to activate networks
		} else {
			history->state_vals_snapshot = *(context.back().state_vals);

			ScoreNetworkHistory* branch_score_network_history = new ScoreNetworkHistory(this->branch_score_network);
			this->branch_score_network->activate(history->state_vals_snapshot,
												 branch_score_network_history);
			double branch_score = run_helper.scale_factor*this->branch_score_network->output->acti_vals[0];
			/**
			 * - scale by scale_factor to keep equation predicted_score + scale_factor*score_update = target_val
			 * - also scale average_misguess by abs(scale_factor) to account for potentially lower impact locally
			 *   - (these 2 instances of scaling don't directly relate to each other)
			 */

			ScoreNetworkHistory* original_score_network_history = new ScoreNetworkHistory(this->original_score_network);
			this->original_score_network->activate(history->state_vals_snapshot,
												   original_score_network_history);
			double original_score = run_helper.scale_factor*this->original_score_network->output->acti_vals[0];

			ScoreNetworkHistory* branch_misguess_network_history = new ScoreNetworkHistory(this->branch_misguess_network);
			this->branch_misguess_network->activate(history->state_vals_snapshot,
													branch_misguess_network_history);

			ScoreNetworkHistory* original_misguess_network_history = new ScoreNetworkHistory(this->original_misguess_network);
			this->original_misguess_network->activate(history->state_vals_snapshot,
													  original_misguess_network_history);

			double score_diff = branch_score - original_score;
			double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
			if (score_val > 0.1) {
				history->is_branch = true;
			} else if (score_val < -0.1) {
				history->is_branch = false;
			} else {
				double misguess_diff = this->branch_misguess_network->output->acti_vals[0]
					- this->original_misguess_network->output->acti_vals[0];
				double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
				if (misguess_val < -0.1) {
					history->is_branch = true;
				} else if (misguess_val > 0.1) {
					history->is_branch = false;
				} else {
					if (rand()%2 == 0) {
						history->is_branch = true;
					} else {
						history->is_branch = false;
					}
				}
			}

			if (history->is_branch) {
				delete original_score_network_history;
				delete original_misguess_network_history;
				history->score_network_history = branch_score_network_history;
				history->misguess_network_history = branch_misguess_network_history;
				history->score_network_output = this->branch_score_network->output->acti_vals[0];
				history->misguess_network_output = this->branch_misguess_network->output->acti_vals[0];
			} else {
				delete branch_score_network_history;
				delete branch_misguess_network_history;
				history->score_network_history = original_score_network_history;
				history->misguess_network_history = original_misguess_network_history;
				history->score_network_output = this->original_score_network->output->acti_vals[0];
				history->misguess_network_output = this->original_misguess_network->output->acti_vals[0];
			}
		}
	} else {
		history->is_branch = false;

		// no need to activate networks
	}
}

void BranchNode::backprop(vector<BackwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	/**
	 * - don't bother factoring in score networks in scale_factor_error
	 *   - right path should be 0.0
	 *   - wrong path will be inaccurate
	 */

	if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
			|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		if (history->is_branch) {
			this->branch_weight = 0.9999*this->branch_weight + 0.0001;
		} else {
			this->branch_weight = 0.9999*this->branch_weight + 0.0;
		}

		if (history->score_network_history != NULL) {
			double branch_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->score_network_output;

			double predicted_score_error = run_helper.target_val - branch_predicted_score;

			ScoreNetwork* score_network = history->score_network_history->network;
			score_network->backprop_weights_with_no_error_signal(
				run_helper.scale_factor*predicted_score_error,
				0.002,
				history->state_vals_snapshot,
				history->score_network_history);
		}

		if (history->misguess_network_history != NULL) {
			double misguess_error = run_helper.final_misguess - history->misguess_network_output;

			ScoreNetwork* misguess_network = history->misguess_network_history->network;
			misguess_network->backprop_weights_with_no_error_signal(
				misguess_error,
				0.002,
				history->state_vals_snapshot,
				history->misguess_network_history);
		}
	} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		if (!run_helper.backprop_is_pre_experiment) {
			if (history->score_network_history != NULL) {
				double branch_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->score_network_output;

				double predicted_score_error = run_helper.target_val - branch_predicted_score;

				ScoreNetwork* score_network = history->score_network_history->network;
				score_network->backprop_errors_with_no_weight_change(
					run_helper.scale_factor*predicted_score_error,
					*(context.back().state_errors),
					history->state_vals_snapshot,
					history->score_network_history);
			}

			if (history->misguess_network_history != NULL) {
				double misguess_error = run_helper.final_misguess - history->misguess_network_output;

				ScoreNetwork* misguess_network = history->misguess_network_history->network;
				misguess_network->backprop_errors_with_no_weight_change(
					misguess_error,
					*(context.back().state_errors),
					history->state_vals_snapshot,
					history->misguess_network_history);
			}
		}
	}
}

void BranchNode::save(ofstream& output_file) {
	output_file << this->branch_scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->branch_scope_context.size(); c_index++) {
		output_file << this->branch_scope_context[c_index] << endl;
		output_file << this->branch_node_context[c_index] << endl;
	}
	output_file << this->branch_is_pass_through << endl;

	if (!this->branch_is_pass_through) {
		ofstream branch_score_network_save_file;
		branch_score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_branch_score.txt");
		this->branch_score_network->save(branch_score_network_save_file);
		branch_score_network_save_file.close();

		ofstream branch_misguess_network_save_file;
		branch_misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_branch_misguess.txt");
		this->branch_misguess_network->save(branch_misguess_network_save_file);
		branch_misguess_network_save_file.close();
	}

	output_file << this->branch_next_node_id << endl;

	if (!this->branch_is_pass_through) {
		ofstream original_score_network_save_file;
		original_score_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_original_score.txt");
		this->original_score_network->save(original_score_network_save_file);
		original_score_network_save_file.close();

		ofstream original_misguess_network_save_file;
		original_misguess_network_save_file.open("saves/nns/" + to_string(this->parent->id) + "_" + to_string(this->id) + "_original_misguess.txt");
		this->original_misguess_network->save(original_misguess_network_save_file);
		original_misguess_network_save_file.close();
	}

	output_file << this->original_next_node_id << endl;

	output_file << this->branch_weight << endl;
}

void BranchNode::save_for_display(ofstream& output_file) {

}

BranchNodeHistory::BranchNodeHistory(BranchNode* node) {
	this->node = node;

	this->score_network_history = NULL;
	this->misguess_network_history = NULL;
}

BranchNodeHistory::~BranchNodeHistory() {
	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}

	if (this->misguess_network_history != NULL) {
		delete this->misguess_network_history;
	}
}
