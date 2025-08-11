#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <string>
#include <tuple>
#include <vector>

class AbstractExperiment;
class AbstractExperimentInstanceHistory;
class AbstractExperimentOverallHistory;
class AbstractExperimentState;
class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	int num_obs;

	Solution* solution;

	/**
	 * - iter variables
	 */
	AbstractExperiment* curr_experiment;
	AbstractExperiment* best_experiment;
	int improvement_iter;

	Scope* curr_explore_scope;
	int curr_explore_tries;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	AbstractExperimentOverallHistory* experiment_overall_history;
	std::vector<AbstractExperimentInstanceHistory*> experiment_instance_histories;

	Problem* problem;
	/**
	 * - for debugging
	 */

	#if defined(MDEBUG) && MDEBUG
	int run_index;
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