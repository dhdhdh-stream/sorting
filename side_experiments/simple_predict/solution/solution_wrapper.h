#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <Eigen/Dense>

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
	int new_since_update;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	std::vector<Eigen::VectorXf> states;

	int num_actions;

	std::vector<ScopeHistory*> existing_scope_histories;
	std::vector<double> existing_target_val_histories;
	/**
	 * - separate existing and explore
	 *   - updating on explore significantly hurts results
	 *     - even if, e.g., updating only post explore
	 */
	std::vector<ScopeHistory*> explore_scope_histories;
	std::vector<double> explore_target_val_histories;
	int train_iter_index;

	int diversity_index;
	bool has_explore;

	std::vector<std::map<AbstractExperiment*, AbstractExperimentHistory*>> experiment_histories;

	Problem* problem;

	// temp
	double error_sum;
	int error_count;

	#if defined(MDEBUG) && MDEBUG
	int run_index;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	SolutionWrapper(ProblemType* problem_type);
	SolutionWrapper(std::string path,
					std::string name);
	~SolutionWrapper();

	void init(std::vector<double> obs);
	std::pair<bool,int> step(std::vector<double> obs);
	void end();

	void experiment_init(std::vector<double> obs);
	std::pair<bool,int> experiment_step(std::vector<double> obs);
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