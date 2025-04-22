#ifndef CONNECTION_H
#define CONNECTION_H

const int CONNECTION_TYPE_ACTION = 0;
const int CONNECTION_TYPE_BRANCH_ORIGINAL = 1;
const int CONNECTION_TYPE_BRANCH_BRANCH = 2;
const int CONNECTION_TYPE_MERGE = 3;

class Connection {
public:
	int type;
	Action action;
	int node_id;

	
};

#endif /* CONNECTION_H */