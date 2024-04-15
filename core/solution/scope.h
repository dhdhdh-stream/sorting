#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "metrics.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class AbstractExperimentHistory;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	int num_improvements;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  int& random_curr_depth,
							  bool& random_exceeded_limit,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);
	void inner_random_exit_activate(std::vector<Scope*>& scope_context,
									std::vector<AbstractNode*>& node_context,
									int& exit_depth,
									AbstractNode*& exit_node,
									int& random_curr_depth,
									bool& random_exceeded_limit);

	void step_through_activate(Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper,
							   ScopeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeHistory* history);
	void clear_verify();
	#endif /* MDEBUG */

	void measure_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  Metrics& metrics,
						  ScopeHistory* history);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	AbstractExperimentHistory* experiment_history;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */