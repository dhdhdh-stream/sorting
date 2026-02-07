#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <string>
#include <tuple>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractExperimentState;
class AbstractNode;
class BranchNode;
class Problem;
class ProblemType;
class Scope;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	Solution* solution;

	Solution* solution_snapshot;

	/**
	 * - iter variables
	 */
	AbstractExperiment* curr_experiment;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	double sum_signals;
	int signal_count;

	AbstractExperimentHistory* experiment_history;

	Problem* problem;

	std::vector<ScopeHistory*> result_scope_histories;
	std::vector<AbstractNode*> result_node_context;
	std::vector<AbstractExperimentState*> result_experiment_context;

	int result_num_actions;

	double result_sum_signals;
	int result_signal_count;

	#if defined(MDEBUG) && MDEBUG
	int run_index;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	SolutionWrapper(ProblemType* problem_type);
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