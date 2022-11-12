#ifndef ABSTRACT_SCOPE_H
#define ABSTRACT_SCOPE_H

const int SCOPE_TYPE_ACTION = 0;
const int SCOPE_TYPE_SCOPE = 1;

class AbstractScope {
public:
	int type;

	virtual ~AbstractScope() {};
};

#endif /* ABSTRACT_SCOPE_H */