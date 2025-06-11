#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <string>
#include <tuple>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractExperimentState;
class AbstractNode;
class ConfusionState;
class Problem;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	int num_obs;

	Solution* solution;

	int run_index;

	/**
	 * - iter variables
	 */
	AbstractExperiment* curr_experiment;
	AbstractExperiment* best_experiment;
	int improvement_iter;

	int sum_num_actions;
	int sum_num_confusion_instances;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;
	std::vector<ConfusionState*> confusion_context;

	int num_actions;
	int num_confusion_instances;

	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	Problem* problem;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	SolutionWrapper(int num_obs);
	SolutionWrapper(int num_obs,
					std::string path,
					std::string name);
	~SolutionWrapper();

	void init();
	std::pair<bool,int> step(std::vector<double> obs);
	void end();

	void experiment_init();
	std::tuple<bool,bool,int> experiment_step(std::vector<double> obs);
	void set_action(int action);
	void experiment_end(double result);

	#if defined(MDEBUG) && MDEBUG
	void verify_init();
	std::pair<bool,int> verify_step(std::vector<double> obs);
	void verify_end();
	#endif /* MDEBUG */

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */