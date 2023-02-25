#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_INNER_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_FOLD = 3;

class AbstractNode {
public:
	Scope* parent;
	int id;

	int type;

	~AbstractNode() {};

	virtual int activate(std::vector<double>& input_vals,
						 std::vector<double>& local_state_vals,
						 std::vector<std::vector<double>>& flat_vals,
						 double& predicted_score,
						 std::vector<int> scope_context,
						 std::vector<int> node_context,
						 std::vector<AbstractNodeHistory>& node_history,
						 int& early_exit_count,
						 int& early_exit_index) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */