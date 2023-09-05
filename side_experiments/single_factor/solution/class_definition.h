#ifndef CLASS_DEFINITION_H
#define CLASS_DEFINITION_H

#include <fstream>
#include <vector>

class ClassDefinition {
public:
	int id;

	std::vector<int> appeared_along;
	std::vector<int> same_spot;
	std::vector<int> correlated;

}

#endif /* CLASS_DEFINITION_H */