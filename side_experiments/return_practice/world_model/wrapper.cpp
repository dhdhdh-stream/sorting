#include "wrapper.h"

#include "action_node.h"
#include "end_node.h"
#include "globals.h"
#include "problem.h"
#include "solution.h"
#include "start_node.h"
#include "world_model.h"

using namespace std;

const int INIT_NUM_ACTIONS = 10;

Wrapper::Wrapper(ProblemType* problem_type) {
	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	this->world_model = new WorldModel(this->num_obs,
									   this->num_actions);

	this->solution = new Solution();

	this->solution->timestamp = 0;
	this->solution->curr_score = 0.0;

	this->solution->node_counter = 0;

	StartNode* start_node = new StartNode();
	start_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[start_node->id] = start_node;

	vector<ActionNode*> action_nodes;
	uniform_int_distribution<int> action_distribution(0, this->num_actions-1);
	for (int n_index = 0; n_index < INIT_NUM_ACTIONS; n_index++) {
		ActionNode* action_node = new ActionNode();
		action_node->id = this->solution->node_counter;
		this->solution->node_counter++;
		this->solution->nodes[action_node->id] = action_node;

		action_node->action = action_distribution(generator);

		action_nodes.push_back(action_node);
	}

	EndNode* end_node = new EndNode();
	end_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[end_node->id] = end_node;

	start_node->next_node_id = action_nodes[0]->id;
	start_node->next_node = action_nodes[0];

	action_nodes[0]->ancestor_ids.push_back(start_node->id);

	for (int a_index = 1; a_index < (int)action_nodes.size(); a_index++) {
		action_nodes[a_index-1]->next_node_id = action_nodes[a_index]->id;
		action_nodes[a_index-1]->next_node = action_nodes[a_index];

		action_nodes[a_index]->ancestor_ids.push_back(action_nodes[a_index-1]->id);
	}

	action_nodes.back()->next_node_id = end_node->id;
	action_nodes.back()->next_node = end_node;

	end_node->ancestor_ids.push_back(action_nodes.back()->id);

	this->iter = 0;

	this->crazy = NULL;

	this->sample_index = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::Wrapper(std::string path,
				 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	this->num_actions = stoi(num_actions_line);

	this->world_model = new WorldModel(input_file);

	this->solution = new Solution();
	this->solution->load(input_file,
						 this);

	input_file.close();

	this->iter = 0;

	this->crazy = NULL;

	this->sample_index = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::~Wrapper() {
	delete this->world_model;

	delete this->solution;
}

void Wrapper::save(string path,
				   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->num_obs << endl;
	output_file << this->num_actions << endl;

	this->world_model->save(output_file);

	this->solution->save(output_file,
						 this);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void Wrapper::save_for_display(string path,
							   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
