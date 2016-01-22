import socket
import json

SYNC = 2
SET_ALARM = 4
TCP_IP = '10.0.1.72'
TCP_PORT = 1313
BUFFER_SIZE = 1024

class TransportError(Exception):
	"""Custom exception to represent errors with a transport
	"""
	def __init__(self, message):
		self.message = message

class devTransport(object):
	__count = 0

	def __init__(self, host):
		self.host = host
		self.port = TCP_PORT
		self.sckt = None
		try:
			self.sckt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		except socket.error as e:
			raise TransportError(e.strerror)
		try:
			self.sckt.connect((host, self.port))
		except socket.error as e:
			raise TransportError(e.strerror)

	def ntpSync(self):
		MESSAGE = json.dumps({"INTR": SYNC})
		databuff = self.__communication(MESSAGE, 3)
		if databuff == None:
			print "Error, try again"
			return None
		data = json.loads(databuff)
		print "Status from esp8266:", data["STATUS"]

	def setAlarm(self, hour, minute, tone):
		MESSAGE = json.dumps({"INTR": SET_ALARM, "VALUES": {"H": int(hour), "m": int(minute), "TONE" : int(tone)}})
		databuff = self.__communication(MESSAGE, 3)
		if databuff == None:
			print "Error, try again"
			return None
		data = json.loads(databuff)
		print "Status from esp8266:", data["STATUS"]

	def __communication(self, message, times):
		if self.__count == times:
			self.__count = 0;
			return None
		self.sckt.sendall(message)
		databuff = self.sckt.recv(BUFFER_SIZE).strip()
		if '{' in databuff and '}' in databuff:
			return databuff
		return self.__communication(message, times)

	def __del__(self):
		self.sckt.close()
		del self.sckt