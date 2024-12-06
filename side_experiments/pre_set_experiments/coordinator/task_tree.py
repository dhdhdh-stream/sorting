import time
import subprocess
import shutil
import sys

BRANCH_FACTOR = 10
MERGE_NUM_TRIES = 10

STATUS_NOT_ADDED = 0
STATUS_ADDED = 1
STATUS_DONE = 2

class TaskNode:
	def __init__(self, layer):
		self.layer = layer

		if self.layer == 0:
			self.filenames = ['']
			self.statuses = [STATUS_NOT_ADDED]
		else:
			self.filenames = ['' for _ in range(MERGE_NUM_TRIES)]
			self.statuses = [STATUS_NOT_ADDED for _ in range(MERGE_NUM_TRIES)]

			self.children = []
			for _ in range(BRANCH_FACTOR):
				self.children.append(TaskNode(self.layer - 1))

		self.result = ''

	def __init__(self, file):
		self.layer = int(file.readline())

		if self.layer == 0:
			self.filenames = [file.readline()]
			self.statuses = [int(file.readline())]

			if self.statuses[0] == STATUS_ADDED:
				self.statuses[0] = STATUS_NOT_ADDED
		else:
			self.filenames = []
			self.statuses = []

			for m_index in range(MERGE_NUM_TRIES):
				self.filenames.append(file.readline())
				self.statuses.append(int(file.readline()))

				if self.statuses[m_index] == STATUS_ADDED:
					self.statuses[m_index] = STATUS_NOT_ADDED

			self.children = []
			for _ in range(BRANCH_FACTOR):
				self.children.append(TaskNode(file))

		self.result = file.readline()

	def init_and_add_tasks(self, tasks):
		# simply add leaf tasks to the back, non-leaf tasks to the front
		if self.layer == 0:
			if self.filenames[0] == '':
				curr_time_stamp = int(time.time())

				filename = 't_' + str(curr_time_stamp) + '.txt'

				result = subprocess.run(['./simple_init', filename], capture_output=True, text=True)
				print(result.stdout)

				self.filenames[0] = filename

			if self.statuses[0] == STATUS_NOT_ADDED:
				tasks.append([self, 0])
		else:
			if self.filenames[0] == '':
				all_children_done = True
				for c_index in range(BRANCH_FACTOR):
					if self.children[c_index].result == '':
						all_children_done = False
						break

				if all_children_done:
					starting_filename = 't_' + str(curr_time_stamp) + '.txt'

					combine_input = []
					combine_input.append('./combine')
					for c_index in range(BRANCH_FACTOR):
						combine_input.append(self.children[c_index].result)
					combine_input.append(starting_filename)

					result = subprocess.run(combine_input, capture_output=True, text=True)
					print(result.stdout)

					self.filenames[0] = starting_filename
					for c_index in range(1, BRANCH_FACTOR):
						filename = 't_' + str(curr_time_stamp + c_index) + '.txt'

						shutil.copyfile('saves/' + starting_filename, 'saves/' + filename)

						self.filenames[c_index] = filename

			if self.filenames[0] != '':
				for m_index in range(MERGE_NUM_TRIES):
					if self.statuses[m_index] == STATUS_NOT_ADDED:
						tasks.appendleft([self, m_index])

			for c_index in range(BRANCH_FACTOR):
				self.children[c_index].init_and_add_tasks(tasks)

	def finalize(self, index):
		self.statuses[index] = STATUS_DONE

		if self.layer == 0:
			self.result = self.filenames[0]
		else:
			is_done = True
			for m_index in range(MERGE_NUM_TRIES):
				if self.statuses[m_index] != STATUS_DONE:
					is_done = False
					break

			if is_done:
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

	def save(self, file):
		file.write(str(self.layer) + '\n')

		if self.layer == 0:
			file.write(self.filenames[0] + '\n')
			file.write(str(self.statuses[0]) + '\n')
		else:
			for m_index in range(MERGE_NUM_TRIES):
				file.write(self.filenames[m_index] + '\n')
				file.write(str(self.statuses[m_index]) + '\n')

			for c_index in range(BRANCH_FACTOR):
				self.children[c_index].save(file)

		file.write(self.result + '\n')

class TaskTree:
	def __init__(self):
		self.root = TaskNode(1)

	def __init__(self, file):
		self.root = TaskNode(file)

	def init_and_add_tasks(self, tasks):
		self.root.init_and_add_tasks(tasks)

	def expand(self):
		new_root = TaskNode(self.root.layer + 1)
		new_root.children[0] = self.root
		self.root = new_root

	def save(self):
		self.root.save(file)
