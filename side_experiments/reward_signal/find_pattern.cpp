// - when high variance, signal already weak
// - when selecting what is captured, still relatively random
//   - so selected input likely weak as well

// - maybe start with something that is known to have a strong signal
//   - then train a familiarity NN on its inputs

// - when measuring, measure co-correlation, and select strongest factor

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "simple.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE_GATHER = 40;
#else
const int NUM_EXPLORE_GATHER = 4000;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	SolutionWrapper* solution_wrapper = new SolutionWrapper(
		problem_type->num_obs());

	vector<int> actions;
	actions.push_back(0);
	actions.push_back(1);
	actions.push_back(2);
	actions.push_back(2);
	actions.push_back(3);
	actions.push_back(3);
	actions.push_back(0);
	actions.push_back(0);
	actions.push_back(1);
	actions.push_back(1);
	actions.push_back(2);
	actions.push_back(2);
	actions.push_back(3);
	actions.push_back(3);
	actions.push_back(0);
	actions.push_back(0);
	actions.push_back(1);
	actions.push_back(1);
	actions.push_back(2);
	actions.push_back(2);
	actions.push_back(3);
	actions.push_back(3);
	actions.push_back(0);
	actions.push_back(0);
	actions.push_back(1);
	actions.push_back(2);

	Scope* scope = solution_wrapper->solution->scopes[0];
	ObsNode* node_context = (ObsNode*)scope->nodes[0];

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)actions.size(); s_index++) {
		ActionNode* new_action_node = new ActionNode();
		new_action_node->parent = scope;
		new_action_node->id = scope->node_counter;
		scope->node_counter++;
		scope->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = actions[s_index];

		new_nodes.push_back(new_action_node);
	}

	ObsNode* new_ending_node = new ObsNode();
	new_ending_node->parent = scope;
	new_ending_node->id = scope->node_counter;
	scope->node_counter++;

	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_OBS) {
			ObsNode* obs_node = (ObsNode*)it->second;
			if (obs_node->next_node == NULL) {
				obs_node->next_node_id = new_ending_node->id;
				obs_node->next_node = new_ending_node;

				new_ending_node->ancestor_ids.push_back(obs_node->id);

				break;
			}
		}
	}

	scope->nodes[new_ending_node->id] = new_ending_node;

	new_ending_node->next_node_id = -1;
	new_ending_node->next_node = NULL;

	int exit_node_id = new_ending_node->id;
	AbstractNode* exit_node = new_ending_node;

	int start_node_id = new_nodes[0]->id;
	AbstractNode* start_node = new_nodes[0];

	if (node_context->next_node != NULL) {
		for (int a_index = 0; a_index < (int)node_context->next_node->ancestor_ids.size(); a_index++) {
			if (node_context->next_node->ancestor_ids[a_index] == node_context->id) {
				node_context->next_node->ancestor_ids.erase(
					node_context->next_node->ancestor_ids.begin() + a_index);
				break;
			}
		}
	}

	node_context->next_node_id = start_node_id;
	node_context->next_node = start_node;

	start_node->ancestor_ids.push_back(node_context->id);

	for (int s_index = 0; s_index < (int)actions.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)actions.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = new_nodes[s_index+1]->id;
			next_node = new_nodes[s_index+1];
		}

		ActionNode* action_node = (ActionNode*)new_nodes[s_index];
		action_node->next_node_id = next_node_id;
		action_node->next_node = next_node;

		next_node->ancestor_ids.push_back(new_nodes[s_index]->id);
	}

	clean_scope(scope,
				solution_wrapper);

	solution_wrapper->solution->clean();

	while (true) {
		Problem* problem = problem_type->get_problem();

		solution_wrapper->explore_init();

		while (true) {
			vector<double> obs = problem->get_observations();

			tuple<bool,bool,int> next = solution_wrapper->explore_step(obs);
			if (get<0>(next)) {
				break;
			} else if (get<1>(next)) {
				uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
				int new_action = action_distribution(generator);

				solution_wrapper->explore_set_action(new_action);

				problem->perform_action(new_action);
			} else {
				problem->perform_action(get<2>(next));
			}
		}

		double target_val = problem->score_result();

		solution_wrapper->explore_end(target_val);

		delete problem;

		if (solution_wrapper->solution->scopes[0]->explore_scope_histories.size() >= NUM_EXPLORE_GATHER) {
			break;
		}
	}

	solution_wrapper->solution->scopes[0]->update_pattern();

	solution_wrapper->save("saves/", "main.txt");
	solution_wrapper->save_for_display("../", "display.txt");

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
