#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "action_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "sorting.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int num_actions_until_experiment = -1;
int num_actions_after_experiment_to_skip = -1;
bool eval_experiment;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	// solution->load("", "initial_action");
	solution->load("", "2nd_layer");
	// solution->load("", "simple_initial_action");

	Scope* new_scope = new Scope();
	new_scope->id = solution->scope_counter;
	solution->scope_counter++;
	solution->scopes[new_scope->id] = new_scope;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	new_scope->nodes[0] = starting_noop_node;
	new_scope->node_counter = 1;

	solution->save("", "main");

	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	delete solution;

	cout << "Done" << endl;
}