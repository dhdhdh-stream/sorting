#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class NewScopeExperiment;
class PotentialCommit;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Scope*> child_scopes;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void result_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void experiment_activate(Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);
	void random_continue(AbstractNode* starting_node,
						 int num_following,
						 std::set<AbstractNode*>& potential_included_nodes);

	void new_scope_activate(Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* scope_history);

	void measure_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void commit_gather_activate(Problem* problem,
								std::vector<ContextLayer>& context,
								RunHelper& run_helper,
								int& node_count,
								AbstractNode*& potential_node_context,
								bool& potential_is_branch);
	void commit_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 PotentialCommit* potential_commit);

	void set_activate(Problem* problem,
					  std::vector<ContextLayer>& context,
					  RunHelper& run_helper,
					  bool& experiment_seen,
					  std::map<std::pair<AbstractNode*,bool>, int>& nodes_seen);

	#if defined(MDEBUG) && MDEBUG
	void new_scope_capture_verify_activate(Problem* problem,
										   std::vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   ScopeHistory* scope_history);
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void clear_verify();
	#endif /* MDEBUG */

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

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

	std::map<int, AbstractNodeHistory*> node_histories;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */