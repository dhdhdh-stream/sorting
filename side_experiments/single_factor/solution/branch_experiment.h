#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

/**
 * - initially, mostly follow seed decisions, then loosen up later
 * 
 * - for sequence seeding, save input vals (i.e., their initialization) but scale them down gradually
 * 
 * - lasso and zero only after trained
 * 
 * - for loops:
 *   - 0: don't modify states
 *     - both for sequence inputs and exit
 *       - or maybe add on top of?
 *   - 1: add state one-by-one, from inner to outer, to get scores right
 *   - 2: add state one-by-one to get loops right
 *     - or don't need to add state by this point?
 *   - 3: measure
 * 
 * - for branch:
 *   - for all sequence inputs and exit, learn networks
 *     - if anything can be reused, figure out through correlation later?
 * 
 * - actually, there's an ordering to all state, including sequence inputs and exits
 *   - so can train those 1-by-1?
 *   - note that some state isn't useful until end
 *     - so those inputs shouldn't matter and should be 0'd?
 * 
 * - if can be continued, then a scope can have input state and new initialized state
 *   - input state won't be passed on, but it will initialize new
 *     - so things work without much extra work needed?
 * 
 * - for exits:
 *   - first learn inputs, and new states
 *   - have exit be able to depend on those
 *     - increase lasso weights inwards (as opposed to new state/inputs increasing weight outwards)
 * 
 * - actually, increase lasso weights both outwards and inwards
 *   - can use different networks for different distances if have to?
 * 
 * - seed includes everything
 *   - initial as well as inner, and all the way to end
 * 
 * - only branches, loops, and finals send error signals
 *   - if there are none, then no need for input
 *     - though can chain
 *       - though if chain, then there's no source of truth?
 *         - maybe can borrow final score network from outside?
 *           - but may be missing its dependencies
 *         - maybe just go for it fully
 * 
 * - use lasso'd networks to determine where to check for correlation
 * 
 * - for chain correlations, simply multiply
 *   - not actually accurate, but only used for exploration anyways
 *     - then relearn correlation after to measure true?
 *       - yeah, easy to check
 * 
 * - don't have score networks
 *   - instead use state updates to guesstimate if have to
 * 
 * - prior to explore, first learn score network
 *   - probably fix context first too
 * 
 * - so update will be for making minor improvements/adjustments
 *   - overtime, can essentially find new solutions on its own
 *     - but will be very incremental
 * 
 * - actually, decide both starting point and ending point before explore
 * 
 * - OK, ran a bunch of experiments, and here are the observations:
 *   - when constructing state, use a single ending score network
 *     - makes the state impacts/error signals proportional
 *     - use seeding to overcome XORs
 *   - update 1 state at a time
 *     - can always make progress
 *       - though initially, may not be clean/ideal
 *       - but keep adding state, and hopefully, eventually, ideal state will be found
 *       - then let impact of state gradually refactor until everything is ideal
 *     - state networks only have to depend on itself and obs
 *       - no need to check other state
 *         - but can check other state at scope starts and ends to constrain
 *   - scale state network outputs by their dependency on obs
 *     - this makes it such that state networks don't only depend on their state
 *       - i.e., causing unnecessary state networks, which just scale and mess things up
 *   - for branch/loop score networks, don't need hidden layer
 *     - can just use flat linear layer
 *       - (also conveniently removing the issue of expanding the hidden layer)
 *   - when using lasso to remove factors, stop zero-ing
 *     - actually, instead of lasso, scale obs, and remove networks with low impact
 *       - actually, in addition, lasso just state
 * 
 * - so need changes:
 *   - score networks only appear at end of scope
 *     - so predicted score only updates at end of scope
 *     - though also update predicted score at branches
 *   - when exploring, select context beforehand and learn score network
 *   - during experiment:
 *     - first, learn all existing state
 *       - for exit, learn through ending score networks
 *       - for inner, use inner branch/loop networks
 *       - also learn branch/new path weight
 *     - then learn new state from inner to outer
 *     - learn everything before measuring
 *   - still update
 *     - update weights of state for ending score networks
 *     - update decision making for branch/loop networks
 *       - randomly select other branch/other iterations
 *         - shouldn't deviate far from existing solution initially, but over time, might lead to big changes
 * 
 * - don't activate state unless reach decision point?
 *   - specifically, don't initialize until needed, then backfill
 * 
 * - scope nodes have pre networks and post networks
 *   - pre networks for inner
 *   - post networks for exit
 * 
 * - scope nodes will have multiple "observations" in one spot
 *   - in which case still treat them one-by-one?
 *     - nah, not difficult to handle multiple
 *       - only pre needs some construction
 *         - though need to think about initialization
 * 
 * - on scope node, split into passed in, not passed in, and newly initialized
 * 
 * - still need scale
 * 
 * - when enabling random branches/loops, also keep track of average damage
 *   - then modify score by average damage?
 *     - actually, should just try one thing at a time
 *       - maybe every spot gets it's turn every 1000th iter?
 *         - doesn't work if there's 1000 branches ahead
 *           - maybe scale by number of branches
 *             - not by number of branches, max number of branches seen in single run
 * 
 * - for ending score networks in scope node, on backprop:
 *   - fire in reverse order, and modify predicted score for each
 *     - because score networks depend on previous score networks
 * 
 * - potentially remove misguess
 *   - difficult to retrain in new context?
 *     - would need to constantly update predicted misguess just like score
 *       - and then may be awkward with scale factor being for 2 different things
 * - maybe need to keep track of and go against average misguess?
 *   - and maybe scale doesn't matter?
 *     - or can have a temporary scale to calculate state, but which can then be dropped?
 * - OK, use single scale, and keep track of an average that applies only to remeasure
 * 
 * - in general, maybe predicted score isn't the right thing to compare against
 *   - maybe should be average score when choosing randomly
 * 
 * - during experiment, when randomly selecting branches, scale possibility by max number of decisions?
 * 
 * - scale represents impact of inner scope
 *   - so simply use for both misguess and score
 * 
 * - when remeasuring, compare against an average that is constantly updated
 * 
 * - or actually, once state is trained, then it doesn't really matter whether comparing to average
 *   - so key is what is needed when retraining state in experiment
 * 
 * - hmm, conflict between remeasure and reuse
 *   - remeasure to change decisions is good
 *     - corresponding state will exist, even if it wasn't trained representatively
 *       - but should be OK, as even if state is actively confusing, will still have previous state to choose right branch
 * 
 * - can potentially use ending score scale?
 *   - can potentially train score network at end of sequence to help train states?
 * 
 * - maybe just train score networks everywhere
 *   - then they become ending score networks?
 *     - (and branch networks)
 * 
 * - for loops, don't copy, but just try to learn best possible, but then add new state
 * 
 * - key is to update predicted score at each branch
 * 
 * - don't have branch weight
 *   - when exploring, just let it keep going, and cut it off
 *     - so sequences are open ended-ish
 * 
 * - maybe divide state between scalers and XORs
 *   - scalers cannot depend on themselves (i.e., the past)
 *   - XORs cannot scale
 * 
 * - for XORs, have way of removing impact of previous state
 *   - forget
 * 
 * - but scalers should be able to overwrite too?
 * 
 * - but anyways, once split into scalers and XORs, all the zeroing stuff should work
 *   - then things should be able to be taken from anywhere, fed into anywhere, and continued from anywhere
 * 
 * - maybe have some way where the scaler can depend on the XOR
 * 
 * - what about if first step is get a value, and second step is to cut in half?
 *   - not depending makes this impossible?
 *     - no, the first step would just save half the value, and second step does nothing
 *     - and ignore multiplications, so maybe this works?
 * 
 * - why seeding works
 *   - 0: establish baseline, then slowly introduce differences
 *   - 1: hope that there are differences that are identifiable
 *     - e.g., all match, except for last
 *       - so up until last, strong signal, and last can use signal plus obs to make correct decision
 *     - or like all match, except in the middle
 *   - 2: now after being able to detect and signal simple mistakes, identify where two wrongs make a right
 *   - 3: XOR found
 * 
 * - what does it take to learn XOR
 *   - for each step, a notion of what matches and what doesn't match
 *   - for each step, whether or not it matching matters
 * 
 * - initially, there is the seed val, which hopefully differs from the average
 *   - learn to recognize seed val
 *     - use final error
 *       - maybe just go above vs below average
 *     - initially dominated by seed obs
 *   - so also can recognize seed mismatches
 * - state represents likelihood of mismatch
 * - then if mismatch and mismatch, switch to match
 * - starting network will go match or no match regardless
 *   - then can create accurate scopes again?
 * 
 * - initially, if score is positive, then match, if negative, then mismatch
 *   - then slowly begin incorporating state
 *     - if state is mismatch, then flip the signs (and scale?)
 *       - but then, how to handle start?
 *         - maybe if match/mismatch have impact on final score?
 *           - no, initially, all steps correlated
 * 
 * - actually, don't need to ramp up
 *   - for XORs, initially all values are meaningless
 *     - but with seeding, the seed obs will take values that are different
 *     - and if keeping track of running state val, it will represent if matches seed
 *       - but then, what matches starts expanding
 *         - then mismatches start appearing, and are reported through state
 *           - but then 2 wrongs leading to a right starts happening, and XOR is found
 * 
 * - xor + scalar can't handle val first, then number of times to multiple after
 * 
 * - partial/continuous error signals are too important initially
 *   - so cannot train xor directly
 * 
 * - maybe normalize the output
 *   - shift by mean, and divide by standard deviation
 *     - yes, this is the answer
 * 
 * - to find starting point, look for spot where obs_weight dominates?
 * 
 * - seeding still helps, but have to have a slight touch
 *   - 10% of samples maybe
 *     - or 5% even actually
 *   - also use strong learning rate
 * 
 * - maybe if obs weight for a state >50%, then consider there to be a dependency/similarity?
 *   - not including state maybe
 * 
 * - each node can keep track of state that it needs
 *   - can be determined from start node
 *     - use to determine what state is needed on exploration
 * 
 * - try negating input too
 *   - since dependency can be either polarity
 * 
 * - lasso doesn't work with normalized state because have to know what previous state is to cancel it out
 * 
 * - yeah, go back to zeroing
 *   - with normalized state, zeroing probably pushes towards closest? which is fine
 * 
 * - one issue with zeroing is that if there are duplicates, then can learn to be robust against either missing
 *   - so then removing both breaks
 * 
 * - maybe have both zeroing and clearing prior
 *   - or remove zeroing
 *     - can never be certain whether it's safe or not safe
 * 
 * - so just go with finding start, then measuring impact?
 * 
 * - maybe start by normalizing only by mean
 *   - then, after determining impact, begin normalizing by standard deviation
 *     - actually, will just be a simple equation to normalize after?
 *       - except for branches and merges?
 * 
 * - normalizing by standard deviation speeds up learning by so much
 *   - oh, but only if shallow
 *     - makes things much worse if deep
 * 
 * - without any norm, learning probably relies on signal vs. no signal
 *   - with norm mean, that is destroyed
 *     - maybe learn normal first without anything, then try to norm mean?
 * 
 * - actually, once learned, can calculate norm without having to relearn anything?
 *   - no, branches/loops screw things up
 *     - have to adjust learnings
 *     - yeah, there's no right mathematical answer
 *       - easy to end up in situations where the proper mathematical thing to do is nothing
 *       - instead, it's more about pushing things down and hoping the networks can adjust
 *         - not a math problem, but a learning problem
 * 
 * - what if randomize process order?
 *   - so can depend on circumstance, but not ordering of the calculation
 * 
 * - don't use scale for ending, as may need to shift mean
 *   - no, just keep using scales
 *     - shifting mean adds another way for things to screw up
 * 
 * TODO: try to find practical XOR limit
 * 
 * - answer maybe to just go flat into RNN
 *   - then just mark obs that are important
 *     - then don't process, and simply save as state
 * 
 * - actually, can't go deep even with flat
 *   - can only select things to pay attention to and try?
 *     - which only makes types more important
 * 
 * - yeah, so the way this works is that to solve a 10-way XOR, need:
 *   - all 10 inputs to align in a hidden node, 2^10 (as well as align at output layer, so another 2x)
 *   - when things align, all the other inputs don't either overshadow the signal straight up, or they don't bounce around and ruin things
 * - so hidden layer needs to be like 1000x
 *   - not practical
 *     - humans can barely pay attention to like 3 things as well
 *       - though can effectively pay attention to a lot by building states up
 * 
 * - with obs size 10:
 *   - with 20 hidden size, can barely get 6-way XORs, reliably get 5-ways
 *   - with 10 hidden size, can barely get 5-way XORs, reliably get 4-ways
 * 
 * - or maybe, it's like this:
 *   - with flat, can go deeper, but still limited by XOR size
 *     - can easily get 5-way XOR with obs size 20
 *   - with RNN, can't go very deep, but everything is limited by XOR size anyways?
 *     - can't solve with obs size 20 even with hidden size 40
 *       - can solve obs size kind of reliably with hidden size 100, so it still matters a bit
 *         - but cannot solve obs size 20
 * 
 * - easy to select obs off of seed
 *   - can even use to determine context?
 *     - no, context is already selected to train score
 * 
 * - loops shouldn't influence the count that much as helped by them being able to be empty
 * 
 * - actually, RNNs just don't work that well
 *   - only reason why I was finding success with seeding and XORs was because obs_vals were +1 or -1
 *     - so it was very easy to learn "match" vs "not match"
 *       - but if the range is bigger, then seeding doesn't matter anymore
 * 
 * - so back to folding?
 * 
 * - when folding, don't worry about getting perfect answer with multiple state
 *   - just try to learn best single state
 *     - the fold is based on a selection of obs anyways, and won't be perfect
 * 
 * - actually, only use flat to determine what to pay attention to
 *   - then use normalized, directly connected RNNs to actually create the state
 *     - direct seems to have a better floor than summed
 *       - but summed seems to have a better ceiling
 *   - no, use summed
 *     - summed can kind of pass error signals through
 * 
 * - perhaps have ending score scale modifiable
 *   - that way if branch, and branch destroyed value, can adjust
 * 
 * - never change or update state once created
 *   - even if branch and state basically becomes meaningless
 *     - instead create new state that score networks can use to hopefully adjust
 */

const int BRANCH_EXPERIMENT_STATE_EXPLORE = -1;

/**
 * - later inputs (and exits) can have dependencies on earlier inputs
 */
const int BRANCH_EXPERIMENT_STATE_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_DONE = 2;



class BranchExperiment {
public:

	// TODO: when initial training, add a mean
	// - after, change to scale

	std::vector<std::vector<int>> curr_flat_context_indexes;
	std::vector<std::vector<int>> curr_flat_state_indexes;
	std::vector<int> curr_flat_obs_indexes;
	FlatNetwork* curr_flat_network;

}

#endif /* BRANCH_EXPERIMENT_H */