#ifndef END_NODE_H
#define END_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class AbstractExperiment;

class EndNode : public AbstractNode {
public:
	EndNode();

	void step(int& action,
			  bool& is_next,
			  Run* run);

	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void experiment_step_start(ExperimentRun* run);

	void predict_step(PredictRun* run);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file,
			  Wrapper* wrapper);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class EndNodeHistory : public AbstractNodeHistory {
public:
	EndNodeHistory(EndNode* node);
};

#endif /* END_NODE_H */