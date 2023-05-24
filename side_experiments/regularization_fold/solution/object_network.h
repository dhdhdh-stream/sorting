#ifndef OBJECT_NETWORK_H
#define OBJECT_NETWORK_H

class ObjectNetwork {
public:
	Network* network;

	std::vector<int> scope_object_index;
	std::vector<int> object_dependency_index;
	std::vector<int> state_index;
	int target_index;

	int score_input_index;
};

class ObjectNetworkHistory {
public:
	ObjectNetwork* object_network;

	NetworkHistory* network_history;
};

#endif /* OBJECT_NETWORK_H */