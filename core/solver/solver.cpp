#include "solver.h"

#include <iostream>
#include <fstream>
#include <limits>

#include "definitions.h"
#include "explore.h"
#include "measure_information.h"
#include "network.h"
#include "setup_decision_making.h"

using namespace std;

Solver::Solver() {
	// SolutionNode* halt_node = new SolutionNode(0);
	// this->nodes.push_back(halt_node);
	// SolutionNode* root_node = new SolutionNode(1);
	// this->nodes.push_back(root_node);
	// this->current_node_index = 2;

	ifstream save_file;
	save_file.open("../saves/1659307210.txt");
	string num_nodes_line;
	getline(save_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		SolutionNode* node = new SolutionNode(n_index, save_file);
		this->nodes.push_back(node);
	}
	this->current_node_index = num_nodes;

	this->generator.seed(SEED);
}

Solver::~Solver() {
	for (int i = 0; i < (int)this->nodes.size(); i++) {
		delete this->nodes[i];
	}
}

void Solver::simple_pass() {
	// try random path for now
	SolutionNode* curr_node = this->nodes[1];
	vector<int> pre_sequence;
	while (true) {
		if (curr_node->children_indexes.size() == 0) {
			break;
		}

		int next_index = rand()%(int)curr_node->children_indexes.size();

		if (curr_node->children_indexes[next_index] == 0) {
			// this is all a hack, but next node is HALT node, so break early
			break;
		} else {
			pre_sequence.push_back(next_index);
			curr_node = this->nodes[curr_node->children_indexes[next_index]];
		}
	}

	// explore
	double best_information = numeric_limits<double>::min();
	vector<Action> best_candidate;
	for (int c_index = 0; c_index < 50; c_index++) {
		vector<Action> candidate = explore(pre_sequence,
										   this->nodes[1],
										   this->nodes,
										   this->generator);
		double information = measure_information(pre_sequence,
												 this->nodes[1],
												 this->nodes,
												 candidate);
		
		SolutionNode* curr_node_for_print = this->nodes[1];
		for (int s_index = 0; s_index < (int)pre_sequence.size(); s_index++) {
			cout << curr_node_for_print->children_actions[pre_sequence[s_index]].to_string() << endl;
			curr_node_for_print = this->nodes[curr_node_for_print->children_indexes[pre_sequence[s_index]]];
		}
		for (int a_index = 0; a_index < (int)candidate.size(); a_index++) {
			cout << candidate[a_index].to_string() << endl;
		}
		cout << "information: " << information << endl;
		cout << endl;

		if (information > best_information) {
			best_information = information;
			best_candidate = candidate;
		}
	}
	
	SolutionNode* branch_node = curr_node;

	// add nodes
	for (int a_index = 0; a_index < (int)best_candidate.size(); a_index++) {
		SolutionNode* new_node = new SolutionNode(this->current_node_index);
		this->nodes.push_back(new_node);
		this->current_node_index++;
		
		curr_node->children_indexes.push_back(new_node->node_index);
		curr_node->children_actions.push_back(best_candidate[a_index]);
		curr_node->children_networks.push_back(NULL);
		curr_node->children_network_names.push_back("");
		
		curr_node = new_node;
	}
	curr_node->count = 1;
	curr_node->information = best_information;
	curr_node->children_indexes.push_back(0);
	curr_node->children_actions.push_back(HALT);
	curr_node->children_networks.push_back(NULL);
	curr_node->children_network_names.push_back("");

	setup_decision_making(branch_node,
						  pre_sequence,
						  this->nodes[1],
						  this->nodes);

	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);
	save_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(save_file);
	}
	save_file.close();

	ofstream solution_display_file;
	string solution_display_file_name = "../solution_display.txt";
	solution_display_file.open(solution_display_file_name);
	solution_display_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->save(solution_display_file);
	}
	solution_display_file.close();
}

void Solver::run() {
	simple_pass();
	simple_pass();
	simple_pass();
}
