#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <list>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class InitNetwork;
class NegateNetwork;
class Network;
class ObsNetwork;
class Problem;
class ScoreNetwork;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<NegateNetwork*> start_negate_networks;
	ObsNetwork* start_obs_network;
	std::vector<InitNetwork*> start_init_networks;
	ScoreNetwork* start_score_network;
	ScoreNetwork* end_score_network;

	std::vector<Scope*> child_scopes;

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

	std::vector<double> state;
	/**
	 * - after start
	 */
	std::vector<double> obs;

	std::map<int, AbstractNodeHistory*> node_histories;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original,
				 Solution* parent_solution);
	~ScopeHistory();
};

#endif /* SCOPE_H */