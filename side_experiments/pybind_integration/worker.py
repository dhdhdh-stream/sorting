import gymnasium as gym
import pickle
import subprocess
import sys
import time
import wrapper

MEASURE_ITERS = 1000

EXPLORE_ITERS = 30

if len(sys.argv) != 3:
	print("Usage: python3 worker.py [path] [filename]")
	exit()

print('Starting...', flush=True)

path = sys.argv[1]
filename = sys.argv[2]

env = gym.make('LunarLander-v3')

w = wrapper.Wrapper(8, path, filename)

start_time = time.time()
while True:
	curr_time = time.time()
	elapsed_time = curr_time - start_time
	if elapsed_time > 20:
		print(elapsed_time, flush=True)
		start_time = curr_time

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
			curr_time = time.time()
			elapsed_time = curr_time - start_time
			if elapsed_time > 20:
				print(elapsed_time, flush=True)
				start_time = curr_time

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
		print('new_score: ' + str(new_score), flush=True)
		w.measure_update(new_score)

		w.save(path, filename)

		timestamp = subprocess.Popen('head -n 1 ' + path + filename, shell=True, stdout=subprocess.PIPE).stdout.read().strip()
		if int(timestamp) >= EXPLORE_ITERS:
			break

print('Done', flush=True)
