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
class AbstractNetworkHistory;
class AbstractNode;
class AbstractNodeHistory;
class BranchNode;
class ExploreExperiment;
class ExploreExperimentHistory;
class Problem;
class ProblemType;
class Scope;
class ScopeHistory;
class Solution;

const int RUN_TYPE_EXISTING = 0;
const int RUN_TYPE_EXPLORE = 1;

class SolutionWrapper {
public:
	Solution* solution;

	int experiment_iter;
	/**
	 * - fully reset experiments every so often
	 *   - to enable experiments in different places
	 */

	int iters_since_update;

	/**
	 * - run variables
	 */
	std::vector<double> state;
	std::vector<double> prev_state;
	std::vector<double> partial_state;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	int run_type;

	std::map<ExploreExperiment*, ExploreExperimentHistory*> explore_experiment_histories;

	std::vector<AbstractNetworkHistory*> network_histories;
	std::vector<AbstractNetworkHistory*> partial_network_histories;

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

	void init(std::vector<double> obs);
	std::pair<bool,int> step(std::vector<double> obs);
	void end();

	void experiment_init(std::vector<double> obs);
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