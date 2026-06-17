#ifndef PREDICT_RUN_H
#define PREDICT_RUN_H

#include <map>
#include <vector>

class AbstractNode;
class Wrapper;

class PredictRun {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;

	std::vector<double> state;
};

#endif /* PREDICT_RUN_H */