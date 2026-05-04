/**
 * TODO: options, dimensions
 */

#ifndef INDEPENDENT_H
#define INDEPENDENT_H

#include "abstract_split.h"

class Independent : public AbstractSplit {
public:
	std::vector<AbstractSplit*> solution;
};

#endif /* INDEPENDENT_H */