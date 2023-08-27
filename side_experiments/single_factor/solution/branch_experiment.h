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
 */

const int BRANCH_EXPERIMENT_STATE_EXPLORE = -1;

/**
 * - later inputs (and exits) can have dependencies on earlier inputs
 */
const int BRANCH_EXPERIMENT_STATE_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_DONE = 2;



#endif /* BRANCH_EXPERIMENT_H */