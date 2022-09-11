#ifndef ACTION_DICTIONARY_H
#define ACTION_DICTIONARY_H

#include <fstream>
#include <vector>

#include "solution_node.h"

class ActionDictionary {
public:
	std::vector<std::vector<SolutionNode*>> actions;

	void load(std::ifstream& save_file);
	void save(std::ofstream& save_file);
};

#endif /* ACTION_DICTIONARY_H */