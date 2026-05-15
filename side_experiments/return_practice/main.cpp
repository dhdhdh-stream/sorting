// old world modeling notes:
// - world modeling difficulties:
//   - some paths are only good under certain conditions
//   - potentially unable to change any action along path
//   - world may be constantly changing
//   - solution may be constantly changing
//   - actions may do different things depending on conditions

// - e.g.:
//   - going left, right gets you back...
//   - ...and going left, left, click, right, right kind of gets you back
//     - but may have accomplished something
// - so going somewhere then coming back doesn't necessarily mean that nothing has been accomplished

// - sometimes, solution may intend to perform something choosing between several spots
//   - so cannot arbitrarily exit and enter parts of the solution?
//     - don't know where to return to

// - when a change is made:
//   - does the change itself have an impact?
//     - doesn't impact "location"
//   - ...or does it better set up something behind?
//     - only about "location"
//   - can do both of course too

// - try not jumping into a branch
//   - but allow exiting a branch

// - let's say trying to replace a path that includes scopes
//   - unimaginable to try something, then find a way back to match everything that original path did

// TODO: try to find bare minimum that still generates good progress

// - humans choosing between options of similar size
//   - e.g., going to college vs. finding a job
//     - not, e.g., going to college vs. making a sandwich

// - start by learning how to move around?
//   - push committal as late as possible?

// - initially, for each action, learn when safe to do
//   - then try to put together a solution from that

// - maybe try classifying parts of the solution, and only swap in equivalent
//   - i.e., don't delete big parts of the solution without something equivalent?

// - just try to create Markov
//   - don't forward-backward to fit

// - what's a new state vs. what is existing?
//   - new state is new information
//     - but can replace old state if old state not going to be seen again

// - try greedy one step changes

// - have local map state and long distance states
//   - when updating, update local and long distance states
//     - so don't update far away

// - states in inconsistent state, but location is singular

// - first update surroundings using new information
// - then determine new location
// - or vice versa

// - when updating surroundings, surrounding can't use its surroundings?
//   - because they are currently being updated?
//   - or at least, can't use updated surroundings
//     - or, update current location (i.e., 1-spot), keep the rest un-updated, then update everything together

// - so, starting from a fixed location, order is:
//   - update current location using surroundings plus new information
//   - update surroundings using their surroundings with just new current location information
//     - also update globals with just new current location information
//   - make next move, and get data
//   - select location, and repeat

// - to test, compare against predicting result with just move but no information
//   - reinforcement learning style, where you hard commit to a location

// - states probably have to be distinctive for progress to be made
// - need large field of view



// - has to be world modeling
//   - can't be memorizing what happened
//   - has to be finding some structure that enables predictions

// - some sort of states + transitions
//   - so can potentially handle infinite actions

// - maybe not necessarily fixed values, but trends

// - 2 different ways for states:
//   - states have values, localize location within states
//     - these are objects?
//   - state values need to be set
//     - this is motion?

// - also need to be able to recognize patterns as objects?

// - how would you even create an object to be recognized?
// - how would you create multiple new states at once?

// - how to reconcile long range stuff?
//   - can't be about tile filling
//     - meaningless in uncertainty
// - has to be key points and general location?
// - but also kind of need tile filling short range?

// - for now, don't worry about objects?
//   - just focus on long range and short range localization

// - can't be tile filling at all, even short range, if there's uncertainty
//   - impossible to reconcile
//   - or can tile fill, but have to expect/live with uncertainty
//     - means predicting exact action impacts won't be exact?

// - maybe just straight-up use optical flow to build short-term, short range map

// - what about long term?
//   - how to avoid checklist?
//     - i.e., how to build states and operate on them
//   - maybe still simulate motion
//     - but give score the closer it is to something good?

// - transform everything into some sort of direction
//   - for predicting what's good and bad
//   - for being able to generate insights
// - but how to take into account what can happen?
//   - i.e., details of a run
//   - can change direction of actions?
// - but also have global state?

// - the direction values tell where I am...
//   - ...but how to track surroundings?
//     - so can't really process giant surroundings anyways
//     - can analyze a local snapshot
//     - and can have global state
//       - i.e., track keypoints
//     - so still use optical flow?

// - but how to create direction?
//   - is it just signals again?
//     - is goal to predict score? or to predict world?
//     - probably goal is to predict world
//       - more likely to generate insight than purely chasing after score?

// - doesn't track objects
//   - is that an issue?
//     - could make better judgements?
//       - probably not worth tracking though
//       - not simulating problem to that degree
//         - uncertainty probably makes things unreasonable anyways

// - goal isn't to predict everything
//   - it is to predict things that are relevant
//     - but what's relevant?
//       - things that impact score
//       - so first look for scenarios that have high impact on score
//         - maybe start naively? since don't have global state/direction?

// TODO: experiment with how well things would go if avoid danger
// - don't branch into dangerous areas
// - don't try to remove impactful areas
// - don't try dangerous new scopes

// - but danger also includes destroying progress?
//   - so not just about avoiding something destructive
//   - even preventing progress is "danger"?
//     - then it really can just take any form
//       - ...then back to analysis

// - has to be simulation for short range
//   - and be careful for long range?

// - maybe about proximity to danger
//   - in the short range, not just pattern, but patterns leading to pattern
//   - in long range, when directions get close to danger

// - there is locality in the way data is gathered
//   - that's why optical flow makes sense as a way to remove duplicates
//     - but perhaps not even necessary/important

// - optical flow probably not that meaningful
//   - e.g., focusing on computer screen
//     - surroundings aren't moving, but what's relevant is what's on screen

// - focus is only on predicting score...
//   - ...but, e.g., why does performing actions in 1 way work, while another way doesn't?
//     - so now need to predict this extra quality
//     - why does something work sometimes and not others
//       - predict that as well

// - also need to understand what impacts score negatively

// - maybe:
//   - find something that impacts score (both positively and negatively)
//     - perhaps if negatively, then try less hard
//   - try replacing actions, switching order, etc.
//   - examine one thing at a time?
//     - so if hit other things that impact score, don't focus on them super hard?
//       - beyond noting that they do impact score?
//   - everything needs to transform into some distance/value?

// - if an action cannot be replaced by anything else, then it must do a thing
// - if an action can be replaced, then it and what it can be replaced with must do a thing

// - if action order can be switched, then actions independently do a thing
// - if action order can't be switched at some point, then what's done before the point must do a thing
//   - and there might be a dependency on that thing for what follows

// - for set of independent actions, solve linearly for number of directions
//   - but action impact can depend on context
//     - but difficult to predict context?
//       - to predict action impact, do you predict context, then predict action impact?
//         - or just predict average action impact?
//         - do you try to gather information to predict context?

// - probably don't predict context
//   - and for scopes, don't predict details

// TODO: experiment with number of directions
// - if A, B need to balance, C, D need to balance, and E, F, need to balance, then obviously 3
//   - how to measure this?
// - try summing to different number of factors

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "simple.h"
#include "world_model_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	string filename;
	WorldModelWrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
		wrapper = new WorldModelWrapper(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		wrapper = new WorldModelWrapper(problem_type);
	}

	while (true) {
		Problem* problem = problem_type->get_problem();

		
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
