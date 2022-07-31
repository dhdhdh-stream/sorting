#include "solver.h"

#include <iostream>
#include <fstream>
#include <limits>

#include "definitions.h"
#include "network.h"

using namespace std;

Solver::Solver() {
	this->root = new SolutionTreeNode(NULL, 0, 0.0, 0.0);
	
	// ifstream save_file;
	// save_file.open("../saves/1659216301.txt");
	// this->root = new SolutionTreeNode(save_file);
	// save_file.close();

	this->generator.seed(SEED);
}

Solver::~Solver() {
	delete this->root;
}

vector<Action> exploration(vector<Action> pre_sequence,
						   vector<Action> post_sequence,
						   default_random_engine generator) {
	while (true) {
		vector<Action> action_sequence = pre_sequence;

		geometric_distribution<int> seq_length_dist(0.2);
		int add_seq_length = 1+seq_length_dist(generator);
		
		normal_distribution<double> write_val_dist(0.0, 2.0);
		for (int i = 0; i < add_seq_length; i++) {
			Action a(write_val_dist(generator), rand()%3);
			action_sequence.push_back(a);
		}

		action_sequence.insert(action_sequence.end(),
			post_sequence.begin(), post_sequence.end());

		Problem p;
		for (int i = 0; i < (int)action_sequence.size(); i++) {
			p.perform_action(action_sequence[i]);
		}
		
		double score = p.score_result();
		if (score == 1.0) {
			ofstream seq_display_file;
			seq_display_file.open("../seq_display.txt");
			seq_display_file << p.initial_world.size() << endl;
			for (int i = 0; i < (int)p.initial_world.size(); i++) {
				seq_display_file << p.initial_world[i] << endl;
			}
			seq_display_file << action_sequence.size() << endl;
			for (int i = 0; i < (int)action_sequence.size(); i++) {
				seq_display_file << action_sequence[i].write << "," << action_sequence[i].move << endl;
			}

			return action_sequence;
		}
	}
}

double measure_score(vector<Action> candidate) {
	double sum_score = 0.0;
	for (int p_index = 0; p_index < 100000; p_index++) {
		Problem p;

		for (int i = 0; i < (int)candidate.size(); i++) {
			p.perform_action(candidate[i]);
		}

		sum_score += p.score_result();
	}
	return sum_score;
}

double measure_information(vector<Action> candidate) {
	Network network((int)candidate.size()+1, 100, 1);

	for (int epoch_index = 0; epoch_index < 20000; epoch_index++) {
		for (int iter_index = 0; iter_index < 100; iter_index++) {
			Problem p;

			vector<double> inputs;
			inputs.push_back(p.get_observation());
			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i]);
				inputs.push_back(p.get_observation());
			}

			network.activate(inputs);

			double target = p.score_result();
			vector<double> errors;
			errors.push_back(target - network.val_val.acti_vals[0]);
			network.backprop(errors);
		}

		double max_update = 0.0;
		network.calc_max_update(max_update,
								0.001,
								0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		network.update_weights(factor,
							   0.001,
							   0.2);
	}

	double eval_score = 0.0;
	for (int p_index = 0; p_index < 100000; p_index++) {
		Problem p;

		vector<double> inputs;
		inputs.push_back(p.get_observation());
		for (int i = 0; i < (int)candidate.size(); i++) {
			p.perform_action(candidate[i]);
			inputs.push_back(p.get_observation());
		}

		network.activate(inputs);

		double target = p.score_result();
		eval_score += abs(target - network.val_val.acti_vals[0]);
	}

	return eval_score;
}

void Solver::simple_pass() {
	// travel down to branching point
	SolutionTreeNode* curr_node = this->root;
	vector<Action> pre_sequence;
	while (true) {
		if (curr_node->children.size() == 0) {
			break;
		}

		// TODO: need to come up with a good heuristic
		double best_information = numeric_limits<double>::max();
		int best_index = -1;
		for (int c_index = 0; c_index < (int)curr_node->children.size(); c_index++) {
			if (curr_node->children[c_index]->information < best_information) {
				best_information = curr_node->children[c_index]->information;
				best_index = c_index;
			}
		}

		// TODO: need to add condition to explore early

		pre_sequence.push_back(curr_node->children_actions[best_index]);
		curr_node = curr_node->children[best_index];
	}

	// explore
	double best_score = 0.0;
	double best_information = numeric_limits<double>::max();
	vector<Action> best_candidate;
	for (int c_index = 0; c_index < 50; c_index++) {
		vector<Action> empty_post_sequence;
		vector<Action> candidate = exploration(pre_sequence, empty_post_sequence, this->generator);
		double score = measure_score(candidate);
		double information = measure_information(candidate);
		
		for (int a_index = 0; a_index < (int)candidate.size(); a_index++) {
			cout << candidate[a_index].to_string() << endl;
		}
		cout << "score: " << score << endl;
		cout << "information: " << information << endl;
		cout << endl;

		if (information < best_information) {
			best_score = score;
			best_information = information;
			best_candidate = candidate;
		}
	}
	
	// TODO: need to setup decision making

	// update tree
	for (int a_index = 0; a_index < (int)best_candidate.size(); a_index++) {
		SolutionTreeNode* child = new SolutionTreeNode(curr_node, 0, 0.0, 0.0);
		curr_node->children.push_back(child);
		curr_node->children_actions.push_back(best_candidate[a_index]);
		curr_node->children_network_names.push_back("");
		curr_node = child;
	}
	curr_node->has_halt = true;
	while (curr_node != NULL) {
		curr_node->count++;
		if (best_information > curr_node->information) {
			curr_node->score = best_score;
			curr_node->information = best_information;
		}

		curr_node = curr_node->parent;
	}

	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);
	this->root->save(save_file);
	save_file.close();

	ofstream tree_display_file;
	string tree_display_file_name = "../tree_display.txt";
	tree_display_file.open(tree_display_file_name);
	this->root->save(tree_display_file);
	tree_display_file.close();
}

void Solver::run() {
	simple_pass();
	simple_pass();
}
