#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_EXIT = 3;

class Scope;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	bool is_explore;
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_state;
	int explore_iter;
	ScoreNetwork* explore_score_network;
	ScoreNetwork* explore_misguess_network;
	double explore_best_surprise;
	AbstractExperiment* explore_best_experiment;

	AbstractExperiment* experiment;

	virtual ~AbstractNode() {};
	virtual void save(std::ofstream& output_file) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */