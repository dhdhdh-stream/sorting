# TODO: normalize rewards, and find special

import gymnasium as gym
import pickle
import sys
import time
import wrapper

print('Starting...')

MEASURE_ITERS = 4000

env = gym.make('LunarLander-v3')

if len(sys.argv) == 2:
	filename = sys.argv[1]
	w = wrapper.Wrapper(8, 'saves/', filename)
else:
	w = wrapper.Wrapper(8)

while True:
	obs, info = env.reset()
	w.experiment_init()
	solution_done = False
	sum_reward = 0.0
	while True:
		if solution_done:
			action = env.action_space.sample()
		else:
			solution_done, is_fetch, s_action = w.experiment_step(obs)
			if solution_done:
				continue

			if is_fetch:
				action = env.action_space.sample()
				s_action = pickle.dumps(action)
				w.set_action(s_action)
			else:
				action = pickle.loads(s_action)

		obs, reward, terminated, truncated, info = env.step(action)
		sum_reward += reward
		if terminated or truncated:
			break
	has_updated = w.experiment_end(sum_reward)
	if has_updated:
		sum_reward = 0.0
		for _ in range(MEASURE_ITERS):
			obs, info = env.reset()
			w.measure_init()
			solution_done = False
			while True:
				if solution_done:
					action = env.action_space.sample()
				else:
					solution_done, s_action = w.measure_step(obs)
					if solution_done:
						continue
					action = pickle.loads(s_action)

				obs, reward, terminated, truncated, info = env.step(action)
				sum_reward += reward
				if terminated or truncated:
					break
			w.measure_end()

		new_score = sum_reward / MEASURE_ITERS
		print('new_score: ' + str(new_score))
		w.measure_update(new_score)

		w.save('saves/', 'main.txt')
		w.save_for_display('../', 'display.txt')
