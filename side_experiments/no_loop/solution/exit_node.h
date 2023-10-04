#ifndef EXIT_NODE_H
#define EXIT_NODE_H

class ExitNode : public AbstractNode {
public:
	int exit_depth;
	int exit_node_id;

	ExitNode();
	ExitNode(std::ifstream& input_file,
			 int id);
	~ExitNode();

	void save(ofstream& output_file);
};

class ExitNodeHistory : public AbstractNodeHistory {
public:
	ExitNodeHistory(ExitNode* node);
};

#endif /* EXIT_NODE_H */