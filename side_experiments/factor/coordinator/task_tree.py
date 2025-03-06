import time
import subprocess
import shutil
import sys

BRANCH_FACTOR = 4
MERGE_NUM_TRIES = 4

STATUS_NOT_ADDED = 0
STATUS_ADDED = 1
STATUS_DONE = 2

class TaskNode:
	def __init__(self, layer=None, file=None):
		if file == None:
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
		else:
			self.layer = int(file.readline().strip())

			if self.layer == 0:
				self.filenames = [file.readline().strip()]
				self.statuses = [int(file.readline().strip())]

				if self.statuses[0] == STATUS_ADDED:
					self.statuses[0] = STATUS_NOT_ADDED
			else:
				self.filenames = []
				self.statuses = []

				for m_index in range(MERGE_NUM_TRIES):
					self.filenames.append(file.readline().strip())
					self.statuses.append(int(file.readline().strip()))

					if self.statuses[m_index] == STATUS_ADDED:
						self.statuses[m_index] = STATUS_NOT_ADDED

				self.children = []
				for _ in range(BRANCH_FACTOR):
					self.children.append(TaskNode(file=file))

			self.result = file.readline().strip()

	def init_and_add_tasks(self, tasks):
		if self.layer == 0:
			if self.filenames[0] == '':
				curr_time_stamp = int(time.time())

				filename = 't_' + str(curr_time_stamp) + '.txt'

				result = subprocess.run(['./simple_init', filename], capture_output=True, text=True)
				print(result.stdout)

				self.filenames[0] = filename

				time.sleep(1)

			if self.statuses[0] == STATUS_NOT_ADDED:
				tasks.appendleft([self, 0])
				self.statuses[0] = STATUS_ADDED
		else:
			if self.filenames[0] == '':
				all_children_done = True
				for c_index in range(BRANCH_FACTOR):
					if self.children[c_index].result == '':
						all_children_done = False
						break

				if all_children_done:
					for m_index in range(0, MERGE_NUM_TRIES):
						curr_time_stamp = int(time.time())

						output_filename = 't_' + str(curr_time_stamp) + '.txt'

						combine_input = []
						combine_input.append('./combine')
						for c_index in range(BRANCH_FACTOR):
							combine_input.append(self.children[c_index].result)
						combine_input.append(output_filename)

						result = subprocess.run(combine_input, capture_output=True, text=True)
						print(result.stdout)

						self.filenames[m_index] = output_filename

						time.sleep(1)

			if self.filenames[0] != '':
				for m_index in range(MERGE_NUM_TRIES):
					if self.statuses[m_index] == STATUS_NOT_ADDED:
						tasks.appendleft([self, m_index])
						self.statuses[m_index] = STATUS_ADDED

			for c_index in range(BRANCH_FACTOR-1, -1, -1):
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

	def reset(self, index):
		if self.layer == 0:
			result = subprocess.run(['./simple_init', self.filenames[index]], capture_output=True, text=True)
			print(result.stdout)
		else:
			combine_input = []
			combine_input.append('./combine')
			for c_index in range(BRANCH_FACTOR):
				combine_input.append(self.children[c_index].result)
			combine_input.append(self.filenames[index])

			result = subprocess.run(combine_input, capture_output=True, text=True)
			print(result.stdout)

		time.sleep(1)

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
	def __init__(self, file=None):
		if file == None:
			self.root = TaskNode(1)
		else:
			self.root = TaskNode(file=file)

	def init_and_add_tasks(self, tasks):
		self.root.init_and_add_tasks(tasks)

	def expand(self):
		new_root = TaskNode(self.root.layer + 1)
		new_root.children[0] = self.root
		self.root = new_root

	def save(self, file):
		self.root.save(file)
