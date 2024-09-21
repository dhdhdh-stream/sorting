#ifndef SAMPLE_GRAPH_H
#define SAMPLE_GRAPH_H

#include <fstream>
#include <vector>

class Sample;
class SampleGraphNode;

class SampleGraph {
public:
	/**
	 * - start at 0, end at 1
	 */
	std::vector<SampleGraphNode*> nodes;

	~SampleGraph();

	void init(Sample* initial_sample);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SAMPLE_GRAPH_H */