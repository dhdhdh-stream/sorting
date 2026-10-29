#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <map>
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
	Solution* best_solution;

	int iters_since_update;

	std::vector<AbstractExperiment*> experiments;
	std::vector<int> experiment_iters;
	/**
	 * - fully reset experiments every so often
	 *   - to enable experiments in different places
	 */

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	std::vector<std::map<BranchNode*, bool>> ramp_tracker;

	bool is_explore;
	int experiment_index;
	std::vector<std::map<AbstractExperiment*, AbstractExperimentHistory*>> experiment_histories;

	Problem* problem;

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

	bool is_done();

	void clean_scopes();

	void combine(std::string other_path,
				 std::string other_name,
				 int starting_num_scopes);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */