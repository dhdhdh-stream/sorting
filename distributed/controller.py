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
			if not os.path.isfile(os.path.join(local_dir, filename)):
				sftp_client.get(remote_dir + filename, os.path.join(local_dir, filename))

def exists_remote(sftp_client, path):
	try:
		sftp_client.stat(path)
	except IOError, e:
		if e.errno == errno.ENOENT:
			return False
		raise
	else:
		return True

solution_file = open('saves/main/solution.txt', 'r')
curr_id = int(solution_file.readline())
solution_file.close()

while True:
	updated = False

	workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
	for line in workers_file:
		arr = line.strip().split()
		name = arr[0]
		host = arr[1]
		username = arr[2]
		password = arr[3]

		client = paramiko.SSHClient()
		client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		client.connect(host,
					   username=username,
					   password=password)

		client_sftp = client.open_sftp()

		try:
			worker_solution_file = client_sftp.file('saves/' + name + '/solution.txt', 'r')
			worker_curr_id = int(worker_solution_file.readline())
			worker_solution_file.close()

			if worker_curr_id > curr_id:
				download_files(client_sftp, 'saves/' + name + '/', 'saves/main/')

				curr_id = worker_curr_id

				updated = True

				break

		except IOError:
			# do nothing

		client_sftp.close()
		client.close()

	workers_file.close()

	if updated:
		workers_file = open(os.path.expanduser('~/workers.txt'), 'r')
		for line in workers_file:
			arr = line.strip().split()
			name = arr[0]
			host = arr[1]
			username = arr[2]
			password = arr[3]

			client = paramiko.SSHClient()
			client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
			client.connect(host,
						   username=username,
						   password=password)

			client_sftp = client.open_sftp()

			parent = 'saves/main'
			for dirpath, dirnames, filenames in os.walk(parent):
				remote_path = os.path.join('saves/main', dirpath[len(parent)+1:])
				try:
					client_sftp.listdir(remote_path)
				except IOError:
					client_sftp.mkdir(remote_path)

				for filename in filenames:
					client_sftp.put(os.path.join(dirpath, filename), os.path.join(remote_path, filename))

			client_sftp.close()
			client.close()

	time.sleep(20)

print('Done')
