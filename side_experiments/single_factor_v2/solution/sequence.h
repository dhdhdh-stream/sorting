/**
 * - starting state network won't have dependency on state so can feel free to pass in anything
 *   - then can check starting state dependency to see if can remove input
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	// TODO: have both input and output, which can connect to stuff in back

	// TODO: maybe split between fetches, which focuses on ending scope, vs executes, which focuses on starting scope given data
	// - yeah, maybe think of this as reasons to try something
	// - so an execute would be to pass in relevant data to enable a good sequence of actions to be performed
	// - so pick start before data is used

	// - maybe track scope parents, so can add some pre-execution before fetching

	// - target most impactful state/decisions

	// - for early exit, if information is found, can exit

	// - can also early exit if impact has mostly been found
	//   - though not precise because certain actions may be necessary even without impact
	//     - or makes more sense to target branch ends

	// - input passed in won't have anything to do with score
	//   - if score state breaks (likely with only partially using scope), just remove

	// - so a fetch sequence is:
	//   - start from anywhere
	//   - exit when correlation above a certain amount, +/- end of loops, extra actions, etc.
	//   - fetched state, along with any related is fed into next sequence or end

	// - an apply sequence is:
	//   - look for dominant factor for decision (and maybe for one or two inner)
	//   - start anywhere before decision
	//   - exit at branch end +/- extra actions

	// - actually, without learning state for back, cannot:
	//   - if back requires information for good decisions, but information is part of new path, cannot work
	//     - even if new path will be better

	// - maybe need rewinding
	//   - i.e., world modeling
	//     - so can compare state at end for correlation
	//     - but need perfect world modeling
	//       - perfect accuracy even into unknown?
	//       - no, original path is known, so actually doable
	//         - but if possible to predict future from past, then that would have already been factored into state
	//           - so no, still impossible

	// - is the answer to just hope that someone else solves the full path and to copy that?
	//   - so a different try finds both the state using a better way and how to utilize it
	//     - passes along how to find the state
	//       - the current agent applies it, luckily links it to the end state, and gets better way to fetch state alongside good decision making?
	// - or just get ridiculously lucky where a sequence is tried that is both good and generates the right information

	// - other approach is to make things continuous
	//   - try both sides of branches, different iters of loops, etc., generate error signals
	//     - but would take an impractically long time to learn anyways
	//       - definitely waaay more efficient making one change at a time in the forward direction

	// - maybe fix starting point and ending point
	//   - then measure starting state correlation and ending state correlation
	//     - get an idea of which states were impacted by existing path
	//       - then try to fill ones that are most impactful

	// - can also try setting values to +1/-1

	// - to prevent branches from screwing things up, states should only be taking into account if ending seen?
	//   - otherwise, if XOR, error signals will squash before branch to 0.0

	// - actually, that's why sequences are better than raw actions
	//   - sequences have the possibility of generating state

	// - only worry about state that has been initialized but not resolved
	//   - what about initialized within?
	//     - maybe keep track of what is initialized on each branch

	// - maybe only thing that isn't touched is resolved
	//   - everything else, just guess and pray

	// - maybe just accept that will initially be worse
	//   - but add an exploration factor that scales off information

	// - making branches depend on context makes copying difficult
	//   - e.g., when creating a new sequence
	// - maybe copy within context
	//   - so new starting node, ending node
	//     - but scope nodes will still behave the same way
	//   - but exits may not make sense
	//   - oh, and node IDs wouldn't match anyways
	// - yeah, just keep things as is
	//   - pass in input which may not matter
	//   - if really want to copy full logic, need to use outside scope

	// TODO: empirically test likelihood of randomly guessing state working
};

#endif /* SEQUENCE_H */