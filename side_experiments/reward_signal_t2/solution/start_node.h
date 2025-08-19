#ifndef START_NODE_H
#define START_NODE_H

#include <fstream>

#include "abstract_node.h"

class StartNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	StartNode();
	~StartNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void clean();
	void measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class StartNodeHistory : public AbstractNodeHistory {
public:
	StartNodeHistory(StartNode* node);
};
/**
 * - only a hack for signal implementation
 *   - don't bother copying
 */

#endif /* START_NODE_H */