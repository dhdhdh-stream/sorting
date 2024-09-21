#ifndef SAMPLE_GRAPH_NODE_H
#define SAMPLE_GRAPH_NODE_H

#include <vector>

#include "action.h"

class SampleGraphNode {
public:
	int id;

	Action action;

	std::vector<int> previous_node_ids;
	std::vector<int> next_node_ids;

	void save_for_display(std::ofstream& output_file);
};

#endif /* SAMPLE_GRAPH_NODE_H */