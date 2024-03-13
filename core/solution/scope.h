/**
 * - the way "practice" works is:
 *   - scope shared between expensive instance and cheap instance
 *   - scope improved using cheap instances (i.e., practice)
 *   - benefits expensive instance
 * 
 * - exploring simply is trying random things
 *   - but the better explorer is the one who has more sophisticated things to try
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class PassThroughExperimentHistory;
class Problem;

class ScopeHistory;
class Scope {
public:
	int id;
	std::string name;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	int starting_node_id;
	AbstractNode* starting_node;

	ScopeHistory* sample_run;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void step_through_activate(Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   RunHelper& run_helper,
							   ScopeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void clear_verify();
	#endif /* MDEBUG */

	void success_reset();
	void fail_reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	PassThroughExperimentHistory* pass_through_experiment_history;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */