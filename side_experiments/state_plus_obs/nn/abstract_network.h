#ifndef ABSTRACT_NETWORK_H
#define ABSTRACT_NETWORK_H

const int NETWORK_TYPE_OBS = 0;
const int NETWORK_TYPE_SCORE = 1;
const int NETWORK_TYPE_ACTION = 2;
const int NETWORK_TYPE_INIT = 3;
const int NETWORK_TYPE_PASS_THROUGH = 4;
const int NETWORK_TYPE_TRANSITION = 5;

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