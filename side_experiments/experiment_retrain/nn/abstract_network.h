#ifndef ABSTRACT_NETWORK_H
#define ABSTRACT_NETWORK_H

const int NETWORK_TYPE_OBS = 0;
const int NETWORK_TYPE_SCORE = 1;
const int NETWORK_TYPE_ACTION = 2;

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