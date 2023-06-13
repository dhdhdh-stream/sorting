#ifndef SCOPE_EXIT_NODE_H
#define SCOPE_EXIT_NODE_H

class ScopeExitNode : public AbstractNode {
public:
	int exit_depth;
	int exit_node_id;

	std::vector<std::map<StateDefinition*, ExitNetwork*>> networks;

};

class ScopeExitNodeHistory : public AbstractNodeHistory {
public:
	std::vector<std::vector<double>> state_vals_snapshot;
};

#endif /* SCOPE_EXIT_NODE_H */