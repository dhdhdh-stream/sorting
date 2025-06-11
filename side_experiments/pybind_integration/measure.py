import gymnasium as gym
import wrapper

MEASURE_ITERS = 1000

env = gym.make('CartPole-v1')

w = wrapper.Wrapper(4, 'saves/', 'main.txt')

sum_reward = 0.0
for _ in range(MEASURE_ITERS):
	obs, info = env.reset()
	w.init()
	while True:
		is_done, action = w.step(obs)
		if is_done:
			break
		obs, reward, terminated, truncated, info = env.step(action)
		sum_reward += reward
		if terminated or truncated:
			break
	w.end()

print('sum_reward: ' + str(sum_reward))
