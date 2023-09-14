#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;

class Scope;

class AbstractNode {
public:
	int type;

	virtual ~AbstractNode() {};
	virtual void activate(int& curr_spot,
						  int& curr_0_index,
						  std::vector<int>& spots,
						  std::vector<bool>& switches,
						  int& num_actions) = 0;
	virtual void fetch_context(std::vector<Scope*>& scope_context,
							   std::vector<int>& node_context,
							   int& curr_num_action,
							   int target_num_action) = 0;
};

#endif /* ABSTRACT_NODE_H */