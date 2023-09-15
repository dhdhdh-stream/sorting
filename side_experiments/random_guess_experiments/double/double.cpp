/**
 * - 10 spots, with XOR obs at 1, 3, 7, 9
 * - 10 states, only 1st impacts results
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "action_node.h"
#include "constants.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	default_random_engine generator;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	geometric_distribution<int> distribution(0.3);

	std::vector<AbstractNode*> starting_nodes;

	// #0: spot 0 -> 1 XOR
	{
		vector<int> state_indexes{
			0,
			4,
			9
		};
		vector<bool> state_xor_switch{
			true,
			false,
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #1: spot 1 -> 2
	{
		vector<int> state_indexes{
			1,
			3,
			6
		};
		vector<bool> state_xor_switch{
			false,
			true,
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #2: spot 2 -> 3
	{
		vector<int> state_indexes{
			5
		};
		vector<bool> state_xor_switch{
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #3: spot 3 -> 4
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #4: spot 4 -> 4
	{
		vector<int> state_indexes{
			5
		};
		vector<bool> state_xor_switch{
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_STAY,
										  state_indexes,
										  state_xor_switch));
	}

	// #5: spot 4 -> 5
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #6: spot 5 -> 6
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #7: spot 6 -> 5
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #8: spot 5 -> 4
	{
		vector<int> state_indexes{
			3,
			8
		};
		vector<bool> state_xor_switch{
			false,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #9: spot 4 -> 5
	{
		vector<int> state_indexes{
			3,
			8
		};
		vector<bool> state_xor_switch{
			false,
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #10: spot 5 -> 6
	{
		vector<int> state_indexes{
			2,
			5,
			9
		};
		vector<bool> state_xor_switch{
			true,
			true,
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #11: spot 6 -> 7 XOR
	{
		vector<int> state_indexes{
			0,
			1,
			2
		};
		vector<bool> state_xor_switch{
			true,
			true,
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #12: spot 7 -> 6
	{
		vector<int> state_indexes{
			6
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #13: spot 6 -> 5
	{
		vector<int> state_indexes{
			9
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #14: spot 5 -> 4
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #15: spot 4 -> 4
	{
		vector<int> state_indexes{
			3,
			6,
			7,
			8
		};
		vector<bool> state_xor_switch{
			false,
			false,
			false,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_STAY,
										  state_indexes,
										  state_xor_switch));
	}

	// #16: spot 4 -> 3 XOR
	{
		vector<int> state_indexes{
			0,
			1,
			8
		};
		vector<bool> state_xor_switch{
			false,
			true,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #17: spot 3 -> 2
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #18: spot 2 -> 3
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #19: spot 3 -> 4
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #20: spot 4 -> 4
	{
		vector<int> state_indexes{
			4
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_STAY,
										  state_indexes,
										  state_xor_switch));
	}

	// #21: spot 4 -> 5
	{
		vector<int> state_indexes{
			7
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #22: spot 5 -> 6
	{
		vector<int> state_indexes{
			4
		};
		vector<bool> state_xor_switch{
			true
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #23: spot 6 -> 5
	{
		vector<int> state_indexes{
			7
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_LEFT,
										  state_indexes,
										  state_xor_switch));
	}

	// #24: spot 5 -> 6
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #25: spot 6 -> 6
	{
		vector<int> state_indexes{
			1,
			2
		};
		vector<bool> state_xor_switch{
			false,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_STAY,
										  state_indexes,
										  state_xor_switch));
	}

	// #26: spot 6 -> 7
	{
		vector<int> state_indexes{
			5,
			9
		};
		vector<bool> state_xor_switch{
			false,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #27: spot 7 -> 8
	{
		vector<int> state_indexes{
			2,
			7
		};
		vector<bool> state_xor_switch{
			true,
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	// #28: spot 8 -> 8
	{
		vector<int> state_indexes;
		vector<bool> state_xor_switch;
		starting_nodes.push_back(new ActionNode(ACTION_STAY,
										  state_indexes,
										  state_xor_switch));
	}

	// #29: spot 8 -> 9 XOR
	{
		vector<int> state_indexes{
			0
		};
		vector<bool> state_xor_switch{
			false
		};
		starting_nodes.push_back(new ActionNode(ACTION_RIGHT,
										  state_indexes,
										  state_xor_switch));
	}

	Scope* start = new Scope(starting_nodes);
	Scope* curr_solution = new Scope(start);

	int num_actions;
	{
		int curr_spot = 0;
		int curr_0_index = 0;
		int curr_1_index = 1;
		vector<int> spots_0;
		vector<bool> switches_0;
		vector<int> spots_1;
		vector<bool> switches_1;
		num_actions = 0;
		curr_solution->activate(curr_spot,
								curr_0_index,
								curr_1_index,
								spots_0,
								switches_0,
								spots_1,
								switches_1,
								num_actions);
	}

	int iter_index = 0;
	while (true) {
		Scope* test_solution = new Scope(curr_solution);

		int explore_node = rand()%num_actions;

		vector<Scope*> explore_scope_context;
		vector<int> explore_node_context;
		int curr_num_action = 0;
		test_solution->fetch_context(explore_scope_context,
									 explore_node_context,
									 curr_num_action,
									 explore_node);

		int num_following_nodes = 0;
		for (int c_index = 0; c_index < (int)explore_scope_context.size(); c_index++) {
			num_following_nodes += (int)explore_scope_context[c_index]->nodes.size()
				- (explore_node_context[c_index]+1);
		}

		int remove_nodes = rand()%(num_following_nodes+1);
		for (int c_index = (int)explore_scope_context.size()-1; c_index >= 0; c_index--) {
			if (remove_nodes >= (int)explore_scope_context[c_index]->nodes.size() - (explore_node_context[c_index]+1)) {
				remove_nodes -= ((int)explore_scope_context[c_index]->nodes.size() - (explore_node_context[c_index]+1));

				while ((int)explore_scope_context[c_index]->nodes.size() > explore_node_context[c_index]+1) {
					delete explore_scope_context[c_index]->nodes[explore_node_context[c_index]+1];
					explore_scope_context[c_index]->nodes.erase(
						explore_scope_context[c_index]->nodes.begin() + explore_node_context[c_index]+1);
				}
			} else {
				for (int n_index = 0; n_index < remove_nodes; n_index++) {
					delete explore_scope_context[c_index]->nodes[explore_node_context[c_index]+1];
					explore_scope_context[c_index]->nodes.erase(
						explore_scope_context[c_index]->nodes.begin() + explore_node_context[c_index]+1);
				}

				break;
			}
		}

		int num_parts = distribution(generator);
		for (int p_index = 0; p_index < num_parts; p_index++) {
			int random_length = 1 + distribution(generator);
			if (random_length > num_actions-1) {
				random_length = num_actions-1;
			}

			int rand_start = rand()%(num_actions-random_length);

			Scope* part = new Scope(start);

			vector<Scope*> start_scope_context;
			vector<int> start_node_context;
			int start_curr_num_action = 0;
			part->fetch_context(start_scope_context,
								start_node_context,
								start_curr_num_action,
								rand_start);

			vector<Scope*> end_scope_context;
			vector<int> end_node_context;
			int end_curr_num_action = 0;
			part->fetch_context(end_scope_context,
								end_node_context,
								end_curr_num_action,
								rand_start + random_length);

			for (int c_index = 0; c_index < (int)end_scope_context.size(); c_index++) {
				while ((int)end_scope_context[c_index]->nodes.size() > end_node_context[c_index]+1) {
					delete end_scope_context[c_index]->nodes[end_node_context[c_index]+1];
					end_scope_context[c_index]->nodes.erase(
						end_scope_context[c_index]->nodes.begin() + end_node_context[c_index]+1);
				}
			}

			for (int c_index = 0; c_index < (int)start_scope_context.size(); c_index++) {
				for (int n_index = 0; n_index < start_node_context[c_index]; n_index++) {
					delete start_scope_context[c_index]->nodes[0];
					start_scope_context[c_index]->nodes.erase(start_scope_context[c_index]->nodes.begin());
				}
			}

			while (true) {
				if (part->nodes.size() == 1 && part->nodes[0]->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)part->nodes[0];
					Scope* inner = new Scope(scope_node->scope);
					delete part;
					part = inner;
				} else {
					break;
				}
			}

			vector<int> new_indexes;
			vector<int> new_target_indexes;

			vector<int> remaining_indexes;
			vector<int> remaining_target_indexes;
			for (int i = 0; i < 10; i++) {
				remaining_indexes.push_back(i);
				remaining_target_indexes.push_back(i);
			}

			int rand_num_indexes = 3 + rand()%3;
			for (int i_index = 0; i_index < rand_num_indexes; i_index++) {
				int rand_index = rand()%(int)remaining_indexes.size();
				int rand_target_index = rand()%(int)remaining_target_indexes.size();

				new_indexes.push_back(remaining_indexes[rand_index]);
				new_target_indexes.push_back(remaining_target_indexes[rand_target_index]);

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
				remaining_target_indexes.erase(remaining_target_indexes.begin() + rand_target_index);
			}

			ScopeNode* new_scope_node = new ScopeNode(new_indexes,
													  new_target_indexes,
													  part);

			explore_scope_context.back()->nodes.insert(
				explore_scope_context.back()->nodes.begin() + explore_node_context.back()+1,
				new_scope_node);
		}

		int curr_spot = 0;
		int curr_0_index = 0;
		int curr_1_index = 1;
		vector<int> spots_0;
		vector<bool> switches_0;
		vector<int> spots_1;
		vector<bool> switches_1;
		int test_num_actions = 0;
		test_solution->activate(curr_spot,
								curr_0_index,
								curr_1_index,
								spots_0,
								switches_0,
								spots_1,
								switches_1,
								test_num_actions);

		bool fail = false;

		int num_seen_spot_0_1 = 0;
		int num_seen_spot_0_3 = 0;
		int num_seen_spot_0_7 = 0;
		int num_seen_spot_0_9 = 0;
		bool sign_0 = true;
		for (int s_index = 0; s_index < (int)spots_0.size(); s_index++) {
			if (spots_0[s_index] == 1) {
				num_seen_spot_0_1++;
			} else if (spots_0[s_index] == 3) {
				num_seen_spot_0_3++;
			} else if (spots_0[s_index] == 7) {
				num_seen_spot_0_7++;
			} else if (spots_0[s_index] == 9) {
				num_seen_spot_0_9++;
			} else {
				fail = true;
				break;
			}

			if (switches_0[s_index]) {
				sign_0 = !sign_0;
			}
		}

		int num_seen_spot_1_2 = 0;
		int num_seen_spot_1_3 = 0;
		int num_seen_spot_1_6 = 0;
		int num_seen_spot_1_7 = 0;
		bool sign_1 = true;
		for (int s_index = 0; s_index < (int)spots_1.size(); s_index++) {
			if (spots_1[s_index] == 2) {
				num_seen_spot_1_2++;
			} else if (spots_1[s_index] == 3) {
				num_seen_spot_1_3++;
			} else if (spots_1[s_index] == 6) {
				num_seen_spot_1_6++;
			} else if (spots_1[s_index] == 7) {
				num_seen_spot_1_7++;
			} else {
				fail = true;
				break;
			}

			if (switches_1[s_index]) {
				sign_1 = !sign_1;
			}
		}

		if (!fail
				&& sign_0
				&& num_seen_spot_0_1%2 == 1
				&& num_seen_spot_0_3%2 == 1
				&& num_seen_spot_0_7%2 == 1
				&& num_seen_spot_0_9%2 == 1
				&& sign_1
				&& num_seen_spot_1_2%2 == 1
				&& num_seen_spot_1_3%2 == 1
				&& num_seen_spot_1_6%2 == 1
				&& num_seen_spot_1_7%2 == 1
				&& test_num_actions < num_actions) {
			cout << "success " << test_num_actions << endl;

			delete curr_solution;
			curr_solution = test_solution;
			num_actions = test_num_actions;

			int curr_spot = 0;
			int curr_0_index = 0;
			int curr_1_index = 1;
			test_solution->print(curr_spot,
								 curr_0_index,
								 curr_1_index);

			cout << endl;
		} else {
			delete test_solution;
		}

		iter_index++;
		if (iter_index%1000000 == 0) {
			cout << iter_index << endl;
		}
	}

	delete start;
	delete curr_solution;

	cout << "Done" << endl;
}
