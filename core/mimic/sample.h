#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>

#include "action.h"

class Sample {
public:
	int id;

	std::vector<Action> actions;
	std::vector<std::vector<double>> obs;

	double result;

	Sample();
	Sample(int id);

	void save();
};

#endif /* SAMPLE_H */