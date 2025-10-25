#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <string>
#include <tuple>
#include <vector>

class AbstractExperimentHistory;
class AbstractExperimentState;
class AbstractNode;
class BranchExperiment;
class Problem;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	Solution* solution;

	/**
	 * - iter variables
	 */
	int improvement_iter;

	BranchExperiment* curr_branch_experiment;
	BranchExperiment* best_branch_experiment;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	double existing_result;

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

	void result_init();
	std::pair<bool,int> result_step(std::vector<double> obs);
	bool result_end(double result);

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