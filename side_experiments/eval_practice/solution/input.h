#ifndef INPUT_H
#define INPUT_H

#include <vector>

class Scope;

class Input {
public:
	std::vector<Scope*> scope_context;
	std::vector<int> node_context;
	int obs_index;
};

#endif /* INPUT_H */