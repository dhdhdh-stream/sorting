#ifndef TRY_H
#define TRY_H

class Try {
public:
	AbstractNode* parent;

	std::vector<int> scope_context;
	std::vector<int> node_context;

	std::vector<AbstractTryAction*> sequence;

};

#endif /* TRY_H */