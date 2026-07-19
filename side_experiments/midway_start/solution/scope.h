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

#if defined(MDEBUG) && MDEBUG
const int RUN_HISTORIES_NUM_SAVE = 10;
#else
const int RUN_HISTORIES_NUM_SAVE = 100;
#endif /* MDEBUG */

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

	std::list<double> last_scores;

	std::vector<std::vector<std::pair<AbstractNode*,bool>>> run_histories;
	int run_history_index;

	Scope();
	~Scope();

	void copy_from(Scope* original,
				   Solution* parent_solution);

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