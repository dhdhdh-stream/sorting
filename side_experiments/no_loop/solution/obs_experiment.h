#ifndef OBS_EXPERIMENT_H
#define OBS_EXPERIMENT_H

class ObsExperiment {
public:
	std::vector<AbstractNode*> nodes;
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;

	FlatNetwork* flat_network;

	std::vector<StateNetwork*> state_networks;
	/**
	 * - don't have ending scale while training (i.e., scale always 1.0)
	 *   - add scale after state trained and readjusting impact
	 */

};

#endif /* OBS_EXPERIMENT_H */