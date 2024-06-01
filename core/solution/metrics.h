#ifndef METRICS_H
#define METRICS_H

#include <vector>

class Scope;
class ScopeHistory;

class Metrics {
public:
	Scope* experiment_scope;
	std::vector<ScopeHistory*> scope_histories;
};

#endif /* METRICS_H */