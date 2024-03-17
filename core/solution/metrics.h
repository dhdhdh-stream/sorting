#ifndef METRICS_H
#define METRICS_H

#include <map>

class Scope;

class Metrics {
public:
	std::map<Scope*, int> scope_counts;

	Metrics();
};

#endif /* METRICS_H */