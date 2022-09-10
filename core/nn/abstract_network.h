#ifndef ABSTRACT_NETWORK_H
#define ABSTRACT_NETWORK_H

class AbstractNetwork {
public:
	virtual ~AbstractNetwork() {};
};

class AbstractNetworkHistory {
public:
	AbstractNetwork* network;

	virtual void reset_weights() = 0;
};

#endif /* ABSTRACT_NETWORK_H */