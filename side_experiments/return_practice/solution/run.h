#ifndef RUN_H
#define RUN_H

#include <vector>

class AbstractNode;
class Wrapper;

class Run {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;

	std::vector<double> state;
};

#endif /* RUN_H */