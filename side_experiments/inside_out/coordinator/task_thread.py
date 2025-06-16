import os
import paramiko
import select
import time

EXPLORE_ITERS = 60

class TaskThread:
	def __init__(self, worker, tasknode, index):
		self.worker = worker
		self.tasknode = tasknode
		self.index = index

		self.client = paramiko.SSHClient()
		self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		self.client.connect(self.worker[1],
							username=self.worker[2],
							password=self.worker[3])

		self.transport = self.client.get_transport()

		self.channel = self.transport.open_session()

		client_sftp = self.client.open_sftp()
		client_sftp.put('saves/' + self.tasknode.filenames[self.index], 'distributed/' + self.tasknode.filenames[self.index])
		client_sftp.close()

		command = 'distributed/worker distributed/ ' + self.tasknode.filenames[self.index] + ' 2>&1'
		self.channel.exec_command(command)

		time.sleep(1)

		self.curr_iter = 0

	def check_status(self):
		temp_client = paramiko.SSHClient()
		temp_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		temp_client.connect(self.worker[1],
							username=self.worker[2],
							password=self.worker[3])

		stdin, stdout, stderr = temp_client.exec_command('head -n 1 distributed/' + self.tasknode.filenames[self.index])
		for line in iter(lambda:stdout.readline(2048), ''):
			line = line.strip()
			if line == 'reset':
				print('reset')

				self.tasknode.reset(self.index)

				self.channel = self.transport.open_session()

				client_sftp = self.client.open_sftp()
				client_sftp.put('saves/' + self.tasknode.filenames[self.index], 'distributed/' + self.tasknode.filenames[self.index])
				client_sftp.close()

				command = 'distributed/worker distributed/ ' + self.tasknode.filenames[self.index] + ' 2>&1'
				self.channel.exec_command(command)
			else:
				new_iter = int(line)

				if new_iter != self.curr_iter:
					self.curr_iter = new_iter

					temp_client_sftp = temp_client.open_sftp()

					temp_client_sftp.get('distributed/' + self.tasknode.filenames[self.index], 'saves/temp_' + self.tasknode.filenames[self.index])
					os.rename('saves/temp_' + self.tasknode.filenames[self.index], 'saves/' + self.tasknode.filenames[self.index])

					if self.curr_iter >= EXPLORE_ITERS:
						temp_client_sftp.remove('distributed/' + self.tasknode.filenames[self.index])

					temp_client_sftp.close()

		temp_client.close()

		rl, wl, xl = select.select([self.channel],[],[],0.0)
		if len(rl) > 0:
			message = self.channel.recv(1024)
			print('l' + str(self.tasknode.layer) + ' ' + self.worker[0] + ' ' + self.tasknode.filenames[self.index])
			print(message)

		if self.channel.closed and self.curr_iter < EXPLORE_ITERS:
			print('worker ' + self.worker[0] + ' ' + self.tasknode.filenames[self.index] + ' failed')
			exit(1)

	def close(self):
		self.client.close()
