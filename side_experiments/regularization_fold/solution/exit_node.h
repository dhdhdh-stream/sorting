#ifndef SCOPE_EXIT_NODE_H
#define SCOPE_EXIT_NODE_H

class ScopeExitNode : public AbstractNode {
public:
	/**
	 * -1 if normal exit
	 * - otherwise, >= 1
	 */
	int exit_depth;
	int exit_node_id;

	std::vector<std::map<StateDefinition*, ExitNetwork*>> networks;

	std::vector<int> exit_context;

};

class ScopeExitNodeHistory : public AbstractNodeHistory {
public:
	std::vector<std::vector<double>> state_vals_snapshot;

	std::vector<double> new_state_vals_snapshot;
};

#endif /* SCOPE_EXIT_NODE_H */