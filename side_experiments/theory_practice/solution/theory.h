/**
 * - responsibility on outside to improve theory, on inside to satisfy theory
 * 
 * - if theory fails, responsibility is on outermost layer
 *   - which is also the layer that detects the failure
 * 
 * - when failure occurs, have:
 *   - original theory, which had been largely robust
 *   - new edge case, which breaks theory
 * - need to generalize edge case:
 *   - ideally, find info/signals in the world
 *     - need world model, as action sequence likely too limited
 *   - at worst, generalize action sequences that cause issues
 */

// - idea is that current theory is directly correlated with outer result
//   - if not directly correlated, then theory needs to be modified?
//     - yes, because who else is getting modified
//       - outer taken as source of truth

// - initially, theory based off of single pattern
//   - that pattern becomes a theory, and if this just holds all the way down, you get:
//     - improve A which improves B which improves C, etc., which is good
//   - but if theory needs to be modified:
//     - add an uncorrelated pattern that is also correlated, then 2 theories which can separately be chased
//     - if pattern depends on info pattern, then 
//   - the realization that a theory needs to be modified can only be reached from the outside
//     - so modification needs to be on outer pattern

// - if have dependency, then pass dependency in, and also use dependency to adjust score

#ifndef THEORY_H
#define THEORY_H

class Theory {
public:
	/**
	 * - for when no patterns yet
	 */
	Scope* raw_solution;

	int node_counter;
	std::map<int, PlanNode*> plan_nodes;

	std::vector<PlanInput> score_inputs;
	Network* score_network;


};

#endif /* THEORY_H */