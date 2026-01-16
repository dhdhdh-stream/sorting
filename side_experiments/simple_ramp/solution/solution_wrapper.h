#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractExperimentState;
class AbstractNode;
class EvalExperiment;
class EvalExperimentHistory;
class ExploreExperiment;
class ExploreExperimentHistory;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

class SolutionWrapper {
public:
	Solution* solution;

	int scope_counter;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	bool should_explore;
	AbstractExperiment* curr_explore;
	bool has_explore;
	std::vector<AbstractExperiment*> explore_order_seen;

	std::map<ExploreExperiment*, ExploreExperimentHistory*> explore_histories;
	std::map<EvalExperiment*, EvalExperimentHistory*> eval_histories;

	Problem* problem;
	/**
	 * - for debugging
	 */

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

	bool is_done();

	void clean_scopes();

	void combine(std::string other_path,
				 std::string other_name,
				 int starting_scope_counter);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */