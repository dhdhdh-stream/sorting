#ifndef OBJECT_NETWORK_H
#define OBJECT_NETWORK_H

class ObjectNetwork {
public:
	Network* network;

	std::vector<int> object_index;
	std::vector<int> state_index;
	int target_index;
};

class ObjectNetworkHistory {
public:
	ObjectNetwork* object_network;

	NetworkHistory* network_history;
};

#endif /* OBJECT_NETWORK_H */