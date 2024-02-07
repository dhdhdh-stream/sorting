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

class ScopeHistory;
class Scope {
public:
	int id;
	std::string name;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	int starting_node_id;
	AbstractNode* starting_node;


};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<AbstractNodeHistory*> node_histories;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */