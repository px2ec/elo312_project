import socket
import json

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