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
class BranchEndNodeHistory;
class BranchNode;
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

	AbstractExperiment* curr_experiment;
	AbstractExperiment* best_experiment;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	std::vector<BranchNode*> branch_node_stack;
	std::vector<std::vector<double>> branch_node_stack_obs;
	// temp
	std::vector<std::pair<int, int>> branch_node_location;

	int num_actions;

	AbstractExperimentHistory* experiment_history;

	std::vector<std::vector<BranchNode*>> experiment_callbacks;
	/**
	 * - when callback hit, also add to BranchEndNode histories
	 */
	std::vector<std::vector<BranchNode*>> branch_end_node_callbacks;
	std::vector<BranchEndNodeHistory*> branch_end_node_callback_histories;

	Problem* problem;

	#if defined(MDEBUG) && MDEBUG
	int run_index;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
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