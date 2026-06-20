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

	this->solution = new Solution();

	this->solution->timestamp = 0;
	this->solution->curr_score = 0.0;

	this->solution->node_counter = 0;

	StartNode* start_node = new StartNode();
	start_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[start_node->id] = start_node;

	start_node->average_instances_per_run = 1.0;

	// vector<ActionNode*> action_nodes;
	// uniform_int_distribution<int> action_distribution(0, this->num_actions-1);
	// for (int n_index = 0; n_index < INIT_NUM_ACTIONS; n_index++) {
	// 	ActionNode* action_node = new ActionNode();
	// 	action_node->id = this->solution->node_counter;
	// 	this->solution->node_counter++;
	// 	this->solution->nodes[action_node->id] = action_node;

	// 	action_node->action = action_distribution(generator);

	// 	action_node->average_instances_per_run = 1.0;

	// 	action_nodes.push_back(action_node);
	// }

	EndNode* end_node = new EndNode();
	end_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[end_node->id] = end_node;

	// start_node->next_node_id = action_nodes[0]->id;
	// start_node->next_node = action_nodes[0];

	// action_nodes[0]->ancestor_ids.push_back(start_node->id);

	// for (int a_index = 1; a_index < (int)action_nodes.size(); a_index++) {
	// 	action_nodes[a_index-1]->next_node_id = action_nodes[a_index]->id;
	// 	action_nodes[a_index-1]->next_node = action_nodes[a_index];

	// 	action_nodes[a_index]->ancestor_ids.push_back(action_nodes[a_index-1]->id);
	// }

	// action_nodes.back()->next_node_id = end_node->id;
	// action_nodes.back()->next_node = end_node;

	// end_node->ancestor_ids.push_back(action_nodes.back()->id);

	start_node->next_node_id = end_node->id;
	start_node->next_node = end_node;

	end_node->ancestor_ids.push_back(start_node->id);

	this->world_model = new WorldModel(this->num_obs,
									   this->num_actions);

	this->iter = 0;

	this->experiment_iter = 0;

	this->sample_index = 0;

	this->compare_experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::Wrapper(std::string path,
				 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	this->solution = new Solution();
	this->solution->load(input_file,
						 this);

	this->world_model = new WorldModel(input_file);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	this->num_actions = stoi(num_actions_line);

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));

		string change_line;
		getline(input_file, change_line);
		this->change_history.push_back(change_line);
	}

	input_file.close();

	this->iter = 0;

	this->experiment_iter = 0;

	this->sample_index = 0;

	this->compare_experiment = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

Wrapper::~Wrapper() {
	delete this->solution;

	delete this->world_model;
}

void Wrapper::save(string path,
				   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save(output_file,
						 this);

	this->world_model->save(output_file);

	output_file << this->num_obs << endl;
	output_file << this->num_actions << endl;

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
	}

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
