#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "abstract_split.h"

class Sequence : public AbstractSplit {
public:
	std::vector<AbstractSplit*> sequence;
};

#endif /* SEQUENCE_H */