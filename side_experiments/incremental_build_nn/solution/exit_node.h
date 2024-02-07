#ifndef EXIT_NODE_H
#define EXIT_NODE_H

class ExitNode : public AbstractNode {
public:
	/**
	 * - always > 0
	 *   - if want to exit within scope, connect directly
	 */
	int exit_depth;

	int exit_node_parent_id;
	int exit_node_id;
	AbstractNode* exit_node;


}

#endif /* EXIT_NODE_H */