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
 */

const int BRANCH_EXPERIMENT_STATE_EXPLORE = -1;

/**
 * - later inputs (and exits) can have dependencies on earlier inputs
 */
const int BRANCH_EXPERIMENT_STATE_EXISTING = 0;
const int BRANCH_EXPERIMENT_STATE_NEW = 1;
const int BRANCH_EXPERIMENT_STATE_DONE = 2;



#endif /* BRANCH_EXPERIMENT_H */