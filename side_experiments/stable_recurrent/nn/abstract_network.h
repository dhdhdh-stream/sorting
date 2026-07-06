#ifndef ABSTRACT_NETWORK_H
#define ABSTRACT_NETWORK_H

const int NETWORK_TYPE_OBS = 0;
const int NETWORK_TYPE_SCORE = 1;
const int NETWORK_TYPE_ACTION = 2;
const int NETWORK_TYPE_INIT = 3;
const int NETWORK_TYPE_NEGATE = 4;

class AbstractNetwork {
public:
	int type;

	virtual ~AbstractNetwork() {};
};

class AbstractNetworkHistory {
public:
	AbstractNetwork* network;

	virtual ~AbstractNetworkHistory() {};
};

#endif /* ABSTRACT_NETWORK_H */