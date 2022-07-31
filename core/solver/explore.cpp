#include "explore.h"

#include "problem.h"

using namespace std;

vector<Action> explore(vector<int> pre_sequence,
					   SolutionNode* root_node,
					   vector<SolutionNode*> nodes,
					   default_random_engine generator) {
	while (true) {
		// check if problem instance reaches the explore point
		Problem p;
		SolutionNode* curr_node = root_node;
		bool on_right_path = true;
		vector<double> inputs;
		for (int s_index = 0; s_index < (int)pre_sequence.size(); s_index++) {
			inputs.push_back(p.get_observation());

			if (curr_node->children_indexes.size() != 1) {
				double best_score = numeric_limits<double>::min();
				int best_index = -1;
				for (int c_index = 0; c_index < (int)curr_node->children_indexes.size(); c_index++) {
					Network* child_network = curr_node->children_networks[c_index];
					child_network->activate(inputs);
					double predicted_score = child_network->val_val.acti_vals[0];
					if (predicted_score > best_score) {
						best_score = predicted_score;
						best_index = c_index;
					}
				}

				if (best_index != pre_sequence[s_index]) {
					on_right_path = false;
					break;
				}
			}

			p.perform_action(curr_node->children_actions[pre_sequence[s_index]]);
			curr_node = nodes[curr_node->children_indexes[pre_sequence[s_index]]];
		}
		if (!on_right_path) {
			continue;
		}

		// explore random sequence
		vector<Action> action_sequence;

		geometric_distribution<int> seq_length_dist(0.2);
		int add_seq_length = 1+seq_length_dist(generator);
		
		normal_distribution<double> write_val_dist(0.0, 2.0);
		for (int i = 0; i < add_seq_length; i++) {
			Action a(write_val_dist(generator), rand()%3);
			action_sequence.push_back(a);
		}

		for (int i = 0; i < (int)action_sequence.size(); i++) {
			p.perform_action(action_sequence[i]);
		}
		
		double score = p.score_result();
		if (score == 1.0) {
			// ofstream seq_display_file;
			// seq_display_file.open("../seq_display.txt");
			// seq_display_file << p.initial_world.size() << endl;
			// for (int i = 0; i < (int)p.initial_world.size(); i++) {
			// 	seq_display_file << p.initial_world[i] << endl;
			// }
			// seq_display_file << action_sequence.size() << endl;
			// for (int i = 0; i < (int)action_sequence.size(); i++) {
			// 	seq_display_file << action_sequence[i].write << "," << action_sequence[i].move << endl;
			// }

			return action_sequence;
		}
	}
}
