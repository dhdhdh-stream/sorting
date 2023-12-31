#ifndef TRY_IMPACT_H
#define TRY_IMPACT_H

class TryImpact {
public:
	/**
	 * - if over 100, rolling
	 */
	int overall_count;
	double overall_impact;
	/**
	 * - if insert, use new; if remove, use original
	 */
	std::map<int, std::pair<int, double>> action_pre_impacts;
	std::map<AbstractNode*, std::pair<int, double>> node_pre_impacts;
	std::map<int, std::pair<int, double>> action_post_impacts;
	std::map<AbstractNode*, std::pair<int, double>> node_post_impacts;

	TryImpact();

	double calc_impact(TryInstance* try_instance,
					   int index);
	void backprop(double impact_diff,
				  TryInstance* try_instance,
				  int index);
};

#endif /* TRY_IMPACT_H */