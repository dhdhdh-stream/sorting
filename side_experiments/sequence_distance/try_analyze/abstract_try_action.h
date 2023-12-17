#ifndef ABSTRACT_TRY_ACTION_H
#define ABSTRACT_TRY_ACTION_H

const int TRY_ACTION_TYPE_ACTION = 0;
const int TRY_ACTION_TYPE_SCOPE = 1;
const int TRY_ACTION_TYPE_ROOT = 2;

class AbstractTryAction {
public:
	int type;

	virtual ~AbstractTryAction() {};
};

#endif /* ABSTRACT_TRY_ACTION_H */