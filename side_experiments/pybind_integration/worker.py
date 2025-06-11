import gymnasium as gym
import subprocess
import sys
import time
import wrapper

MEASURE_ITERS = 1000

EXPLORE_ITERS = 10

if len(sys.argv) != 3:
	print("Usage: python3 worker.py [path] [filename]")
	exit()

print('Starting...', flush=True)

path = sys.argv[1]
filename = sys.argv[2]

env = gym.make('CartPole-v1')

w = wrapper.Wrapper(4, path, filename)

start_time = time.time()
while True:
	curr_time = time.time()
	if curr_time - start_time > 20:
		print('a', flush=True)
		start_time = curr_time

	obs, info = env.reset()
	w.experiment_init()
	sum_reward = 0.0
	while True:
		is_done, is_fetch, s_action = w.experiment_step(obs)
		if is_done:
			break

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
			w.init()
			while True:
				is_done, s_action = w.step(obs)
				if is_done:
					break
				action = pickle.loads(s_action)
				obs, reward, terminated, truncated, info = env.step(action)
				sum_reward += reward
				if terminated or truncated:
					break
			w.end()
		print('sum_reward: ' + str(sum_reward), flush=True)

		w.save(path, filename)

		timestamp = subprocess.Popen('head -n 1 ' + path + filename, shell=True, stdout=subprocess.PIPE).stdout.read().strip()
		if int(timestamp) >= EXPLORE_ITERS:
			break

print('Done', flush=True)
