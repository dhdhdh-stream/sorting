#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <list>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Network;
class Problem;
class Solution;
class SolutionWrapper;

const int RUN_NUM_SAVE = 10;

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
	/**
	 * - main goal of reusing scopes is generalization
	 *   - want to use the same scopes as often as possible
	 *     - so for outer, only add if used
	 * - if goal is for explore to cover large distance, also have create_new_scope()
	 */

	std::list<double> last_scores;

	Scope();
	~Scope();

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
	ScopeHistory(ScopeHistory* original,
				 Solution* parent_solution);
	~ScopeHistory();
};

#endif /* SCOPE_H */