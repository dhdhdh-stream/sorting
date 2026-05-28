#ifndef WRAPPER_H
#define WRAPPER_H

#include <fstream>
#include <vector>

class ExperimentRun;
class ProblemType;
class Run;
class Solution;
class WorldModel;

class Wrapper {
public:
	int num_obs;
	int num_actions;

	WorldModel* world_model;
	Solution* solution;

	std::vector<std::vector<std::vector<double>>> old_sample_obs;
	std::vector<std::vector<int>> old_sample_actions;
	std::vector<double> old_sample_target_vals;

	std::vector<std::vector<std::vector<double>>> new_sample_obs;
	std::vector<std::vector<int>> new_sample_actions;
	std::vector<double> new_sample_target_vals;

	int iter;

	#if defined(MDEBUG) && MDEBUG
	int run_index;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	Wrapper(ProblemType* problem_type);
	Wrapper(std::string path,
			std::string name);
	~Wrapper();

	void init(Run* run);
	std::pair<bool,int> step(std::vector<double> obs,
							 Run* run);

	void experiment_init(ExperimentRun* run);
	std::pair<bool,int> experiment_step(std::vector<double> obs,
										ExperimentRun* run);
	void experiment_end(double result,
						ExperimentRun* run);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* WRAPPER_H */