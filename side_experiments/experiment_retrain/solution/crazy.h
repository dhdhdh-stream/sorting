#ifndef CRAZY_H
#define CRAZY_H

#include "abstract_node.h"

/**
 * - to identify, node == NULL
 */
class CrazyHistory : public AbstractNodeHistory {
public:
	int action;

	CrazyHistory();
};

#endif /* CRAZY_H */