import gymnasium as gym
import wrapper

# env = gym.make('CartPole-v1')
env = gym.make('CartPole-v1', render_mode='human')

w = wrapper.Wrapper(4, 2, 'saves/', 'main.txt')

obs, info = env.reset()
w.init()
sum_reward = 0.0
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
