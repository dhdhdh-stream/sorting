import gymnasium as gym
from gymnasium.envs.toy_text.frozen_lake import generate_random_map

env = gym.make("FrozenLake-v1", desc=generate_random_map(size=4), render_mode="human")
observation, info = env.reset()

episode_over = False
while not episode_over:
    action = env.action_space.sample()  # agent policy that uses the observation and info
    observation, reward, terminated, truncated, info = env.step(action)

    print('reward: ' + str(reward))

    episode_over = terminated or truncated

env.close()
