#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <utility>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class Solution;
class SolutionWrapper;

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	ObsNode();
	~ObsNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void result_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(ObsNode* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */