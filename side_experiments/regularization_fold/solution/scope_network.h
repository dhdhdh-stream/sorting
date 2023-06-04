#ifndef SCOPE_NETWORK_H
#define SCOPE_NETWORK_H

class ScopeNetwork {
public:
	Network* network;

	std::vector<int> input_indexes;
};

class ScopeNetworkHistory {
public:
	ScopeNetwork* object_network;

	NetworkHistory* network_history;
};

#endif /* SCOPE_NETWORK_H */