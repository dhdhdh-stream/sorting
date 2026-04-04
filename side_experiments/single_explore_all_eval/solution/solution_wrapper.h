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
class EvalExperiment;
class EvalExperimentHistory;
class ExploreExperiment;
class ExploreExperimentHistory;
class OuterExperiment;
class OuterExperimentHistory;
class Problem;
class ProblemType;
class Scope;
class ScopeHistory;
class Solution;

const int DAMAGE_STATE_REFINE = 0;
const int DAMAGE_STATE_RAMP = 1;

class SolutionWrapper {
public:
	Solution* solution;

	ExploreExperiment* curr_explore_experiment;

	int eval_iter;

	std::vector<double> score_histories;
	int history_index;

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	int damage_state;

	ExploreExperimentHistory* explore_experiment_history;
	std::map<EvalExperiment*, EvalExperimentHistory*> eval_experiment_histories;

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
				 std::string other_name);

	void save(std::string path,
			  std::string name);

	void save_for_display(std::string path,
						  std::string name);
};

#endif /* SOLUTION_WRAPPER_H */