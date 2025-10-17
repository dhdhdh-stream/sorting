#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <string>
#include <tuple>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractExperimentState;
class AbstractNode;
class Problem;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	Solution* solution;

	/**
	 * - iter variables
	 */
	AbstractExperiment* curr_experiment;
	AbstractExperiment* best_experiment;
	int improvement_iter;

	int regather_counter;
	int num_regather;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	int run_index;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	Problem* problem;
	#endif /* MDEBUG */

	SolutionWrapper();
	SolutionWrapper(std::string path,
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
				 std::string other_name,
				 int starting_size);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */