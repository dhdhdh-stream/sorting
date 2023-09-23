/**
 * - evaluate state at both decision point as well as end
 *   - evaluate by comparing error with and without state
 *   - if state impact differs at decision point, finalize state
 *   - if state impact differs at end, add on/off
 *     - or remove if decision point is outside of scope
 */

// TODO: evaluate score_state to try to measure new misguess
// - use to determine if pass through

// - during branch experiments, still look for new state
//   - can be only way to make progress

// TODO: maybe save 1000 runs, and just look for obs on those?
