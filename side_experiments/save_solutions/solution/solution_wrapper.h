#ifndef SOLUTION_WRAPPER_H
#define SOLUTION_WRAPPER_H

#include <map>
#include <string>
#include <tuple>
#include <vector>

class BranchExperiment;
class BranchExperimentInstanceHistory;
class BranchExperimentOverallHistory;
class AbstractExperimentState;
class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class Signal;
class SignalExperiment;
class SignalExperimentInstanceHistory;
class Solution;

class SolutionWrapper {
public:
	int num_obs;

	int scope_counter;

	Solution* solution;

	/**
	 * TODO: move into scopes
	 */
	/**
	 * - for positive samples for training signals
	 *   - for an improvement to be made, on average, over half of the samples need to be positive
	 *     - so need to be particularly good at recognizing positive samples
	 * 
	 * - use successful solutions rather than "successful" instances
	 *   - "successful" instances can be more likely to be positive fluctuations
	 * 
	 * - replace randomly so older solutions have a chance to stay
	 *   - to try to have more diversity for generalization
	 */
	std::vector<Solution*> positive_solutions;
	std::vector<Solution*> trap_solutions;
	/**
	 * - simply reset after SignalExperiment cycle
	 * 
	 * TODO:
	 * - instead save and evaluate after SignalExperiment if causes confusion
	 *   - if so, rerun SignalExperiments and include from now on
	 */
	std::map<int, Signal*> signals;

	int last_signal_experiment_timestamp;

	/**
	 * - iter variables
	 */
	BranchExperiment* curr_experiment;
	BranchExperiment* best_experiment;
	std::vector<Solution*> next_positive_solutions;

	SignalExperiment* curr_signal_experiment;
	SignalExperiment* best_signal_experiment;

	int improvement_iter;
	/**
	 * - SignalExperiment can change actions, so will need to reset BranchExperiments
	 */

	/**
	 * - run variables
	 */
	std::vector<ScopeHistory*> scope_histories;
	std::vector<AbstractNode*> node_context;
	std::vector<AbstractExperimentState*> experiment_context;

	int num_actions;

	BranchExperimentOverallHistory* experiment_overall_history;
	std::vector<BranchExperimentInstanceHistory*> experiment_instance_histories;

	std::vector<SignalExperimentInstanceHistory*> signal_experiment_instance_histories;

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

	void measure_init();
	std::tuple<bool,bool,int> measure_step(std::vector<double> obs);
	void measure_end(double result);

	void explore_init();
	std::tuple<bool,bool,int> explore_step(std::vector<double> obs);
	void explore_end(double result);

	void signal_experiment_init();
	std::tuple<bool,bool,int> signal_experiment_step(std::vector<double> obs);
	void signal_experiment_end(double result);

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