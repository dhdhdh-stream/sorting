#include "setup_decision_making.h"

#include <limits>

#include "problem.h"

using namespace std;

void setup_decision_making(SolutionNode* target_node,
						   vector<int> pre_sequence,
						   SolutionNode* root_node,
						   vector<SolutionNode*> nodes) {
	for (int network_index = 0; network_index < (int)target_node->children_indexes.size(); network_index++) {
		if (target_node->children_networks[network_index] == NULL) {
			Network* network = new Network((int)pre_sequence.size()+1, 100, 1);
			for (int epoch_index = 0; epoch_index < 50000; epoch_index++) {
				for (int iter_index = 0; iter_index < 100; iter_index++) {
					while (true) {
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

						inputs.push_back(p.get_observation());
						network->activate(inputs);

						p.perform_action(curr_node->children_actions[network_index]);
						curr_node = nodes[curr_node->children_indexes[network_index]];
						
						while (true) {
							if (curr_node->node_index == 0) {
								break;
							}

							inputs.push_back(p.get_observation());

							if (curr_node->children_indexes.size() == 1) {
								p.perform_action(curr_node->children_actions[0]);
								curr_node = nodes[curr_node->children_indexes[0]];
							} else {
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

								p.perform_action(curr_node->children_actions[best_index]);
								curr_node = nodes[curr_node->children_indexes[best_index]];
							}
						}

						double target = p.score_result();
						vector<double> errors;
						errors.push_back(target - network->val_val.acti_vals[0]);
						network->backprop(errors);
						break;
					}
				}

				double max_update = 0.0;
				network->calc_max_update(max_update,
										 0.001,
										 0.2);
				double factor = 1.0;
				if (max_update > 0.01) {
					factor = 0.01/max_update;
				}
				network->update_weights(factor,
										0.001,
										0.2);
			}

			target_node->children_networks[network_index] = network;

			ofstream save_file;
			string save_file_name = "../saves/nns/" + to_string(time(NULL)) + ".txt";
			save_file.open(save_file_name);
			network->save_weights(save_file);
			save_file.close();
			target_node->children_network_names[network_index] = save_file_name;
		}
	}
}
