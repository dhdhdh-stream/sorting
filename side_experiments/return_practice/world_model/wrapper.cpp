#include "wrapper.h"

#include "obs_node.h"
#include "problem.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

Wrapper::Wrapper(ProblemType* problem_type) {
	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	this->world_model = new WorldModel();

	this->solution = new Solution();

	this->solution->timestamp = 0;
	this->solution->curr_score = 0.0;

	this->solution->node_counter = 0;

	ObsNode* start_node = new ObsNode();
	start_node->id = this->solution->node_counter;
	this->solution->node_counter++;
	this->solution->nodes[start_node->id] = start_node;
	start_node->next_node_id = -1;
	start_node->next_node = NULL;

	this->solution->score_index = 0;

	this->iter = 0;

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

	this->world_model = new WorldModel();
	this->world_model->load(input_file);

	this->solution = new Solution();
	this->solution->load(input_file,
						 this);

	input_file.close();

	this->iter = 0;

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
