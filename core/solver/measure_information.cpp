#include "measure_information.h"

#include <limits>

#include "network.h"
#include "problem.h"

using namespace std;

double measure_average_score(vector<int> pre_sequence,
							 SolutionNode* root_node,
							 vector<SolutionNode*> nodes,
							 vector<Action> candidate) {
	double average_score = 0.0;
	for (int p_index = 0; p_index < 100000; p_index++) {
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

			for (int i = 0; i < (int)candidate.size(); i++) {
				p.perform_action(candidate[i]);
			}

			average_score += p.score_result();
			break;
		}
	}
	average_score /= 100000;

	return average_score;
}

double measure_information(vector<int> pre_sequence,
						   SolutionNode* root_node,
						   vector<SolutionNode*> nodes,
						   vector<Action> candidate) {
	// measure average score
	double average_score = measure_average_score(pre_sequence,
												 root_node,
												 nodes,
												 candidate);

	// try to learn scores
	Network network((int)pre_sequence.size()+(int)candidate.size()+1, 100, 1);
	for (int epoch_index = 0; epoch_index < 20000; epoch_index++) {
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
				for (int i = 0; i < (int)candidate.size(); i++) {
					p.perform_action(candidate[i]);
					inputs.push_back(p.get_observation());
				}

				network.activate(inputs);

				double target = p.score_result();
				vector<double> errors;
				errors.push_back(target - network.val_val.acti_vals[0]);
				network.backprop(errors);
				break;
			}
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

	// measure information
	double think_good_and_good = 0.0;
	double think_good_but_bad = 0.0;
	double think_bad_but_good = 0.0;
	double think_bad_and_bad = 0.0;
	for (int p_index = 0; p_index < 100000; p_index++) {
		Problem p;

		vector<double> inputs;
		inputs.push_back(p.get_observation());
		for (int i = 0; i < (int)candidate.size(); i++) {
			p.perform_action(candidate[i]);
			inputs.push_back(p.get_observation());
		}

		network.activate(inputs);
		double think = network.val_val.acti_vals[0];

		double target = p.score_result();
		if (target > average_score) {
			if (think > average_score) {
				think_good_and_good += (target-average_score)*(think-average_score);
			} else {
				think_bad_but_good += (target-average_score)*(average_score-think);
			}
		} else {
			if (think > average_score) {
				think_good_but_bad += (average_score-target)*(think-average_score);
			} else {
				think_bad_and_bad += (average_score-target)*(average_score-think);
			}
		}
	}

	return think_good_and_good - think_good_but_bad;
}
