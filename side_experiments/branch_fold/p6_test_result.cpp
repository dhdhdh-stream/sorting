/**
 * 0 - 2: blank
 * 1 - 2: 1 which is shared val
 * 2 - 2: 1 which is 1st branch score modifier
 * 3 - 2: 1 which is 2nd branch score modifier
 * 4 - 2: 1 which is choice
 * 5 - 3: 1 which is 1st branch val, 1 which is 2nd branch val
 * 6 - 2: 1 which is choice
 * 7 - 2: blank
 * - 1st branch:
 *   - 0 - 3: 1 which is val
 * - 2nd branch:
 *   - 0 - 2: 1 which is val
 *   - 1 - 2: blank
 *   - 2 - 3: 1 which is val
 * 0 - 2: blank
 * 1 - 3: 1 which is 1st branch score modifier
 * 2 - 2: 1 which is 2nd branch score modifier
 * 3 - 2: 1 which is shared val
 * 4 - 2: 1 which is shared score modifier
 * 5 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> flat_sizes;
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);

	double average_score = 0.0;

	vector<Node*> nodes;
	for (int i = 0; i < 15; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_14.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	vector<vector<double>> state_vals;
	double predicted_score = average_score;

	int flat_input_counter = 0;

	vector<vector<double>> flat_vals;
	flat_vals.reserve(11);

	int choice_val = 0;
	int shared_val = 0;
	int first_val = 0;
	double first_score_modifier = 0.0;
	double shared_score_modifier = 0.0;

	flat_vals.push_back(vector<double>(2));
	flat_vals[0][0] = rand()%2*2-1;
	flat_vals[0][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[0],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[1][0] = rand()%2*2-1;
	if (flat_vals[1][0] == 1.0) {
		shared_val++;
	}
	flat_vals[1][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[1],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[2][0] = rand()%2*2-1;
	first_score_modifier += flat_vals[2][0];
	flat_vals[2][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[2],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[3][0] = rand()%2*2-1;
	flat_vals[3][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[3],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[4][0] = rand()%2*2-1;
	if (flat_vals[4][0] == 1.0) {
		choice_val++;
	}
	flat_vals[4][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[4],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	flat_vals[5][0] = rand()%2*2-1;
	if (flat_vals[5][0] == 1.0) {
		first_val++;
	}
	flat_vals[5][1] = rand()%2*2-1;
	flat_vals[5][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[5],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;
	
	flat_vals.push_back(vector<double>(2));
	flat_vals[6][0] = rand()%2*2-1;
	if (flat_vals[6][0] == 1.0) {
		choice_val++;
	}
	flat_vals[6][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[6],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[7][0] = rand()%2*2-1;
	flat_vals[7][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[7],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	flat_vals[8][0] = rand()%2*2-1;
	if (flat_vals[8][0] == 1.0) {
		first_val++;
	}
	flat_vals[8][1] = rand()%2*2-1;
	flat_vals[8][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[8],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[9][0] = rand()%2*2-1;
	flat_vals[9][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[9],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(3));
	flat_vals[10][0] = rand()%2*2-1;
	first_score_modifier += flat_vals[10][0];
	flat_vals[10][1] = rand()%2*2-1;
	flat_vals[10][2] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[10],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[11][0] = rand()%2*2-1;
	flat_vals[11][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[11],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[12][0] = rand()%2*2-1;
	if (flat_vals[12][0] == 1.0) {
		shared_val++;
	}
	flat_vals[12][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[12],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[13][0] = rand()%2*2-1;
	shared_score_modifier += flat_vals[13][0];
	flat_vals[13][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[13],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	flat_vals.push_back(vector<double>(2));
	flat_vals[14][0] = rand()%2*2-1;
	flat_vals[14][1] = rand()%2*2-1;
	nodes[flat_input_counter]->activate(state_vals,
										flat_vals[14],
										predicted_score);
	cout << flat_input_counter << endl;
	cout << "input:";
	for (int i_index = 0; i_index < (int)flat_vals[flat_input_counter].size(); i_index++) {
		cout << " " << flat_vals[flat_input_counter][i_index];
	}
	cout << endl;
	cout << "state:" << endl;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
		}
	}
	cout << "predicted_score: " << predicted_score << endl;
	cout << "num score input networks: " << nodes[flat_input_counter]->score_input_networks.size() << endl;
	cout << "num input networks: " << nodes[flat_input_counter]->input_networks.size() << endl;
	cout << "compress_num_layers: " << nodes[flat_input_counter]->compress_num_layers << endl;
	cout << endl;
	flat_input_counter++;

	double final_val;
	if (choice_val%2 == 0) {
		final_val = (shared_val+first_val)%2 + first_score_modifier + shared_score_modifier;
	} else {
		final_val = -5.0;
	}
	cout << "final_val: " << final_val << endl;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}

	cout << "Done" << endl;
}
