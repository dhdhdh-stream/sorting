#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
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
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void experiment_activate(Problem* problem,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void random_continue(AbstractNode* starting_node,
						 int num_following,
						 std::set<AbstractNode*>& potential_included_nodes);

	void measure_activate(Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history);

	#if defined(MDEBUG) && MDEBUG
	void new_scope_capture_verify_activate(Problem* problem,
										   RunHelper& run_helper,
										   ScopeHistory* scope_history);
	void verify_activate(Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
	void clear_verify();
	#endif /* MDEBUG */

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void clean();
	void measure_update();

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
	~ScopeHistory();
};

#endif /* SCOPE_H */