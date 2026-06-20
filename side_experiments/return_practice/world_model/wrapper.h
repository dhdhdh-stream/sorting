#ifndef WRAPPER_H
#define WRAPPER_H

#include <fstream>
#include <vector>

class CompareExperiment;
class ExperimentRun;
class ForceExperiment;
class ProblemType;
class Run;
class Solution;
class WorldModel;

class Wrapper {
public:
	Solution* solution;

	WorldModel* world_model;

	int num_obs;
	int num_actions;

	std::vector<double> improvement_history;
	std::vector<std::string> change_history;

	int iter;

	int experiment_iter;
	/**
	 * - fully reset experiments every so often
	 *   - to enable experiments in different places
	 */

	std::vector<std::vector<std::vector<double>>> sample_obs;
	std::vector<std::vector<int>> sample_actions;
	std::vector<double> sample_target_vals;
	int sample_index;

	CompareExperiment* compare_experiment;

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