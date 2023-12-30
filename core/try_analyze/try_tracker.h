#ifndef TRY_TRACKER_H
#define TRY_TRACKER_H

const int TRY_INSERT = 0;
const int TRY_REMOVE = 1;
const int TRY_SUBSTITUTE = 2;

class TryTracker {
public:
	std::vector<TryInstance*> tries;

	std::map<std::pair<int, int>, TryImpact*> action_impacts;
	std::map<std::pair<int, AbstractNode*>, TryImpact*> node_impacts;

}

#endif /* TRY_TRACKER_H */