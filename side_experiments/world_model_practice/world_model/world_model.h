/**
 * - goal is not to predict outcome of everything that can be done
 *   - predict only when on familiar path
 *     - if in unfamiliar territory, just realize so
 *       - perhaps occasionally try to relocalize
 *       - gradually map out unknown later over time
 * - goal is just to track what has been done
 */

// - 2 main goals:
//   - localization/mapping
//     - terrain
//     - map
//       - map instance
//   - object recognition

// - always have field of view to track motion?
//   - or for better pattern recognition?

// - assume you are where your sensors tell you you are
//   - though sensors may also give you information about things further away
//     - or they may not

// - what is true about an instance might not be true overall
//   - but to figure out what is true about an instance might not be what should be done overall
//     - like verifying a 3-ring vs a 5-ring

// - maybe need actions specific to world model separate from solution?
//   - what information is needed?
//   - how can the world differ?

// - maybe need theories on how the world behaves as well?

// - maybe have probabilities of what the world looks like
//   - and don't have to resolve
//     - so can make progress without having to fully resolve world
//       - but can eliminate probabilities

// - again, could always be edge cases, but don't worry about them unless they become prominent
//   - like when solution heavily involves

// - before building solution, start by building world model

// - there is stuff to learn about in a single run, there is stuff to learn about in different runs

// - for single run, look for all kinds of loops, and combine
//   - can't really pay attention to actions before stable state
//     - may also never be stable state, but still be able to find temporary loops

// - many spots/world states may be so hard to reach, but also so important to reach that it takes a solution to consistently reach them
//   - which means single run not as relevant

// - note that after branch merge, state is new, unique from the 2 branches before it
//   - not just branch merge, but even just branch
