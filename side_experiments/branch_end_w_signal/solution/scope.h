#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Factor;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<Scope*> child_scopes;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);
	void random_new_scope_end_activate(AbstractNode* starting_node,
									   std::vector<AbstractNode*>& possible_ends);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

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