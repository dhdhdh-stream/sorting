#ifndef INFO_SCOPE_NODE_H
#define INFO_SCOPE_NODE_H

class InfoScopeNodeHistory;
class InfoScopeNode : public AbstractNode {
public:
	InfoScope* scope;

	int next_node_id;
	AbstractNode* next_node;

	InfoScopeNode();
	InfoScopeNode(InfoScopeNode* original,
				  Solution* parent_solution);
	~InfoScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class InfoScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	bool is_positive;

	InfoScopeNodeHistory();
	InfoScopeNodeHistory(InfoScopeNodeHistory* original);
	~InfoScopeNodeHistory();
};

#endif /* INFO_SCOPE_NODE_H */