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
// temp
class ExploreState;
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
	int experiment_iter;

	// temp
	int num_experiments;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;
	std::vector<ConfusionState*> confusion_context;
	// temp
	std::vector<ExploreState*> explore_context;

	int num_actions;
	int num_confusion_instances;

	AbstractExperimentHistory* experiment_history;

	// temp
	bool has_explore;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	Problem* problem;
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

	// temp
	void explore_init();
	std::tuple<bool,bool,int> explore_step(std::vector<double> obs);
	void explore_set_action(int action);
	void explore_end(double result);

	#if defined(MDEBUG) && MDEBUG
	void verify_init();
	std::pair<bool,int> verify_step(std::vector<double> obs);
	void verify_end();
	#endif /* MDEBUG */

	void measure_init();
	std::pair<bool,int> measure_step(std::vector<double> obs);
	void measure_end(double result);
	void measure_update(double new_score);

	bool is_done();

	void clean_scopes();

	void combine(std::string other_path,
				 std::string other_name);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */