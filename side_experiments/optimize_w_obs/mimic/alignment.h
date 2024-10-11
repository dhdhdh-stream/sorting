#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#include <utility>
#include <vector>

class Sample;

class Alignment {
public:
	Sample* sample;

	std::vector<std::vector<std::pair<std::vector<int>,std::vector<int>>>> step_nodes;

	Alignment(Sample* sample);

	double score();

	void print();
};

#endif /* ALIGNMENT_H */