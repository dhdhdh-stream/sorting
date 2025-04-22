// - what do you gain by merging?
//   - it is only generalization
//     - if holds, then actions taken from one part should also be taken by another
//     - for later, decisions made due to one part should also be taken due to another

// - if used for decision later, then same spot
// - if same actions/decisions taken from, then 

// - not fatal to merge and then branch
//   - so can merge relatively aggressively

// - give up on the idea of tracking where you are exactly
//   - especially if probabilities involved anyways
//   - instead always roughly track through landmarks

// - if there's uncertainty, average/standard deviation will not align with simple actions

// - don't have loops
//   - if there's any uncertainty, then will build up, and will invalidate
//     - so in many cases, don't really have too much meaning?
// - patterns are what's needed instead

// - have like biomes and landmarks?
//   - but biomes can be connected
//     - but biomes/HMMs may not work if state changes
//       - so not worth to focus on building?
//         - no systematic way to do so

// - so it is just patterns that don't change regardless of state
//   - at least in the short run (e.g., within the timeline of a scope)
//     - in the long run, may be unreliable, and something else will be the pattern
//   - with part of the pattern being something that signals

// - and can instead have connections between patterns
//   - (which don't change due to state as due to the assumptions)

// - initially, can't distinguish between a pattern and something that changes
//   - purposefully look for patterns

#ifndef SPOT_H
#define SPOT_H

class State;

class Spot {
public:
	std::vector<State*> states;

	std::map<Connection, Impact> connections;

	/**
	 * - if linearly correlated
	 *   - 0.0 if not correlated
	 * 
	 * - if only 1 state, and low standard deviation, don't bother
	 */
	std::map<State*, double> relations;

	std::set<Spot*> failed_merges;

};

#endif /* SPOT_H */