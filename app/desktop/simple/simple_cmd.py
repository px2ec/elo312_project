#!/usr/bin/env python
import socket
import json
import sys

import argparse

SYNC = 2
SET_ALARM = 4
TCP_IP = '10.0.1.72'
TCP_PORT = 1313
BUFFER_SIZE = 1024

class devTransport(object):
	def __init__(self, host):
		self.host = host
		self.port = TCP_PORT
		self.socket = None
		try:
			self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		except socket.error as e:
			raise TransportError(e.strerror)

		try:
			self.socket.connect((host, self.port))
		except socket.error as e:
			raise TransportError(e.strerror)

	def ntpSync(self):
		MESSAGE = json.dumps({"INTR": SYNC})
		self.socket.sendall(MESSAGE)
		databuff = self.socket.recv(BUFFER_SIZE)
		data = json.loads(databuff)
		print "Status from esp8266", data["STATUS"]

	def setAlarm(self, hour, minute):
		MESSAGE = json.dumps({"INTR": SET_ALARM, "VALUES": {"H": int(hour), "m": int(minute)}})
		self.socket.sendall(MESSAGE)
		databuff = self.socket.recv(BUFFER_SIZE)
		data = json.loads(databuff)
		print "Status from esp8266", data["STATUS"]

	def __del__(self):
		self.socket.close()

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='IoC simple communication.')
	parser.add_argument('--ip',    default=TCP_IP, help='Device IP')
	parser.add_argument('--ntpsync', action='store_true', help='NTP sync command')
	parser.add_argument('--setalarm', default=None, help='Set alarm')
	args = parser.parse_args()
	devt = None

	if args.ip:
		devt = devTransport(args.ip)

	if args.ntpsync:
		if devt == None:
			devt = devTransport(TCP_IP)
		devt.ntpSync()
		sys.exit(0)

	if args.setalarm:
		if devt == None:
			devt = devTransport(TCP_IP)
		tarr = args.setalarm.strip().split(':')
		devt.setAlarm(tarr[0], tarr[1])

	sys.exit(0)




