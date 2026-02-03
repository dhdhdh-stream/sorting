#ifndef MAPPING_H
#define MAPPING_H

#include <map>
#include <vector>

class Mapping {
public:
	std::vector<std::vector<std::map<int,int>>> world;
	int curr_x;
	int curr_y;

	Mapping();

	void print();
};

#endif /* MAPPING_H */