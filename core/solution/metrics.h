#ifndef METRICS_H
#define METRICS_H

#include <vector>

class Scope;
class AbstractScopeHistory;

class Metrics {
public:
	Scope* experiment_scope;
	std::vector<AbstractScopeHistory*> scope_histories;

	Scope* new_scope;
	std::vector<AbstractScopeHistory*> new_scope_histories;
};

#endif /* METRICS_H */