#include "solution_wrapper.h"

#include "solution.h"

using namespace std;

SolutionWrapper::SolutionWrapper(int num_obs,
								 int num_possible_actions) {
	this->num_obs = num_obs;
	this->num_possible_actions = num_possible_actions;

	this->solution = new Solution();
	this->solution->init();

	this->run_index = 0;

	this->curr_experiment = NULL;
	this->best_experiment = NULL;
	this->improvement_iter = 0;

	this->sum_num_actions = 0;
	this->sum_num_confusion_instances = 0;
}

SolutionWrapper::SolutionWrapper(int num_obs,
								 int num_possible_actions,
								 std::string path,
								 std::string name) {
	this->num_obs = num_obs;
	this->num_possible_actions = num_possible_actions;

	this->solution = new Solution();
	this->solution->load(path, name);

	this->run_index = 0;

	this->curr_experiment = NULL;
	this->best_experiment = NULL;
	this->improvement_iter = 0;

	this->sum_num_actions = 0;
	this->sum_num_confusion_instances = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	this->solution->save(path, name);
}

void SolutionWrapper::save_for_display(string path,
									   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
