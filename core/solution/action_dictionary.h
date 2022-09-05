#ifndef ACTION_DICTIONARY_H
#define ACTION_DICTIONARY_H

class ActionDictionary {
public:
	// either single scope, or path of actions
	std::vector<std::vector<SolutionNode*>> actions;

	// never save empty nodes
};

#endif /* ACTION_DICTIONARY_H */