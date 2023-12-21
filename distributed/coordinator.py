# Python3

import errno
import os
import paramiko
import stat
import sys
import time

print('Starting...')

def download_files(sftp_client, remote_dir, local_dir):
	if not exists_remote(sftp_client, remote_dir):
		return

	if not os.path.exists(local_dir):
		os.mkdir(local_dir)

	for filename in sftp_client.listdir(remote_dir):
		if stat.S_ISDIR(sftp_client.stat(remote_dir + filename).st_mode):
			# uses '/' path delimiter for remote server
			download_files(sftp_client, remote_dir + filename + '/', os.path.join(local_dir, filename))
		else:
			sftp_client.get(remote_dir + filename, os.path.join(local_dir, filename))

def exists_remote(sftp_client, path):
	try:
		sftp_client.stat(path)
	except IOError as e:
		if e.errno == errno.ENOENT:
			return False
		raise
	else:
		return True

workers = []

workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
for line in workers_file:
	arr = line.strip().split()
	workers.append([arr[0], arr[1], arr[2], arr[3]])
workers_file.close()

solution_file = open('saves/main/solution.txt', 'r')
curr_id = int(solution_file.readline())
solution_file.close()

while True:
	updated = False

	for worker in workers:
		client = paramiko.SSHClient()
		client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		client.connect(worker[1],
					   username=worker[2],
					   password=worker[3])

		client_sftp = client.open_sftp()

		try:
			worker_solution_file = client_sftp.file('workers/' + worker[0] + '/saves/' + worker[0] + '/solution.txt', 'r')
			worker_curr_id = int(worker_solution_file.readline())
			worker_solution_file.close()

			if worker_curr_id > curr_id:
				print(worker[0] + ' updated')

				download_files(client_sftp, 'workers/' + worker[0] + '/saves/' + worker[0] + '/', 'saves/main/')

				curr_id = worker_curr_id

				updated = True

		except IOError:
			pass

		client_sftp.close()
		client.close()

		if updated:
			break

	if updated:
		for worker in workers:
			client = paramiko.SSHClient()
			client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
			client.connect(worker[1],
						   username=worker[2],
						   password=worker[3])

			client_sftp = client.open_sftp()

			# TODO: potential issue where worker is just reading in previous update while new update is being written
			parent = 'saves/main'
			for dirpath, dirnames, filenames in os.walk(parent):
				remote_path = os.path.join('workers/' + worker[0] + '/saves/main', dirpath[len(parent)+1:])
				try:
					client_sftp.listdir(remote_path)
				except IOError:
					client_sftp.mkdir(remote_path)

				for filename in filenames:
					if filename != 'solution.txt':
						client_sftp.put(os.path.join(dirpath, filename), os.path.join(remote_path, filename))
			# update solution.txt last
			client_sftp.put('saves/main/solution.txt', 'workers/' + worker[0] + '/saves/main/solution.txt')

			client_sftp.close()
			client.close()

		# extra sleep to give workers time to update to help avoid a race condition
		time.sleep(40)

	time.sleep(20)

print('Done')
