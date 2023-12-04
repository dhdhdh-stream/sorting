# Python3

import os
import paramiko
import sys

if len(sys.argv) != 5:
	print('Usage: ./setup_worker [name] [host] [username] [password]')
	exit(1)

print('Starting...')

name = sys.argv[1]
host = sys.argv[2]
username = sys.argv[3]
password = sys.argv[4]

workers_file = open(os.path.expanduser('~/workers.txt'), 'a')
workers_file.write(name + ' ' + host + ' ' + username + ' ' + password + '\n')
workers_file.close()

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(host,
			   username=username,
			   password=password)
client_sftp = client.open_sftp()

stdin, stdout, stderr = client.exec_command('mkdir workers')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

# create separate directory, executable, saves for each worker

stdin, stdout, stderr = client.exec_command('mkdir workers/' + name)
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

try:
	client_sftp.stat('workers/' + name + '/worker')
	client_sftp.remove('workers/' + name + '/worker')
except IOError:
	pass
client_sftp.put('../core/worker', 'workers/' + name + '/worker')
stdin, stdout, stderr = client.exec_command('chmod +x workers/' + name + '/worker')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

stdin, stdout, stderr = client.exec_command('mkdir workers/' + name + '/saves')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

stdin, stdout, stderr = client.exec_command('mkdir workers/' + name + '/saves/curr')
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')



stdin, stdout, stderr = client.exec_command('mkdir saves/' + name)
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

parent = 'saves/main'
for dirpath, dirnames, filenames in os.walk(parent):
	remote_path = os.path.join('saves/main', dirpath[len(parent)+1:])
	try:
		client_sftp.listdir(remote_path)
	except IOError:
		client_sftp.mkdir(remote_path)

	for filename in filenames:
		client_sftp.put(os.path.join(dirpath, filename), os.path.join(remote_path, filename))

# client.exec_command('./worker ' + name)

stdin, stdout, stderr = client.exec_command('./worker ' + name)
for line in iter(lambda:stdout.readline(2048), ''):
	print(line, end='')

# difficult to kill through paramiko(?), so simply start and forget

# kill later through SSH if needed

client_sftp.close()
client.close()

print('Done')
