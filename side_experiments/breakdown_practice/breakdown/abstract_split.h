#ifndef ABSTRACT_SPLIT_H
#define ABSTRACT_SPLIT_H

const int SPLIT_TYPE_ACT = 0;
const int SPLIT_TYPE_SEQ = 1;
const int SPLIT_TYPE_IND = 2;

class AbstractSplit {
public:
	int type;

	virtual ~AbstractSplit() {};
};

#endif /* ABSTRACT_SPLIT_H */