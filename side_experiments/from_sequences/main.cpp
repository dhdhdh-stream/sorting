/**
 * - Baum-Welch effective when states have different obs
 *   - and when there's no delay between actions and obs
 * 
 * - comparing sequences doesn't suffer from Markov limitations(?)
 *   - but if use HMM as the underlying model, then will suffer from Markov limitations
 * 
 * - maybe model should be connected model, but with path (instead of single state)
 *   - then can process path with like focus NN?
 *     - each state represents a symbol?
 * 
 * - use unknown rather than assume single state initially
 *   - random spot vs. random spot offset by 1 unlikely to be significantly different(?)
 *   - whereas from a fixed starting point may be significantly different
 * 
 * - maybe don't worry about HMM/Baum-Welch at all
 *   - build state through measuring sequences
 *   - then to determine obs (or anything), process the path
 */

/**
 * - updating average_val makes things converge
 *   - with each cycle, the value gets sharper and sharper
 *     - pushing the uncertainty into the transitions
 * 
 * - but if any transitions missing, then average_val will carry the uncertainty
 */

/**
 * - maybe simply allow transitions between nearby states
 *   - and maybe clean if unneeded afterwards?
 */
