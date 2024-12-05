import time
import subprocess
import sys

BRANCH_FACTOR = 10
MERGE_NUM_TRIES = 10

class TaskNode:
	def __init__(self, layer):
		self.layer = layer

	def init_tasks(self):
		curr_time_stamp = int(time.time())

		if self.layer == 0:
			filename = str(curr_time_stamp) + '.txt'

			result = subprocess.run(['./simple_init', filename], capture_output=True, text=True)
			print(result.stdout)

			self.filenames = [filename]
			self.is_done = [False]
		else:
			

			self.filenames = [None for _ in range(MERGE_NUM_TRIES)]
			self.is_done = [False for _ in range(MERGE_NUM_TRIES)]

	def finalize(self):
		if self.layer == 0:
			self.result = self.filenames[0]
		else:
			best_score = sys.float_info.min
			best_index = -1

			for c_index in range(MERGE_NUM_TRIES):
				possible_file = open('saves/' + self.filenames[c_index], 'r')
				possible_timestamp = int(possible_file.readline())
				possible_average_score = float(possible_file.readline())
				possible_file.close()

				if possible_average_score > best_score:
					best_score = possible_average_score
					best_index = c_index

			self.result = self.filenames[best_index]

class TaskTree:
	def __init__(self):
		self.root = TaskNode(1)
		for c_index in range(BRANCH_FACTOR):

