
import sys
sys.path.append("./uimodules")
sys.path.append("../utils")

# From Qt 4 Designer --------------------------------------------------------------------------------------
import ui_mainwindow

from devtransport import devTransport

from PyQt4 import QtCore, QtGui

class MainWindow(QtGui.QMainWindow, ui_mainwindow.Ui_MainWindow):
	def __init__(self):
		super(MainWindow, self).__init__()
		self.setupUi(self)
		self.devt = None
		self.lvitmemodel = QtGui.QStandardItemModel()
		self.actionConectar.triggered.connect(self.actionconectartrigered)
		self.pBsync.clicked.connect(self.pbsyncclicked)
		self.pBalarm.clicked.connect(self.pbalarmclicked)
		self.ip = ''

	def actionconectartrigered(self):
		text, boolean = QtGui.QInputDialog.getText(self, 'Connect', 'IP address')
		if not boolean:
			return None
		item = QtGui.QStandardItem(text)
		self.lvitmemodel.appendRow(item)
		self.listView.setModel(self.lvitmemodel)
		self.ip = ''.join([str(x) for x in text])
		self.devt = devTransport(self.ip)

	def pbsyncclicked(self):
		if self.devt == None:
			return None
		self.devt.ntpSync()
		self._reinitsock()

	def pbalarmclicked(self):
		if self.devt == None:
			return None
		text, boolean = QtGui.QInputDialog.getText(self, 'Set alarm', 'Time')
		if not boolean:
			return None
		tstr = ''.join([str(x) for x in text])
		tarr = tstr.strip().split(':')
		text, boolean = QtGui.QInputDialog.getText(self, 'Set alarm', 'Tone 1-6')
		if not boolean:
			return None
		tstr = ''.join([str(x) for x in text])
		self.devt.setAlarm(tarr[0], tarr[1], tstr)
		self._reinitsock()

	def _reinitsock(self):
		del self.devt
		self.devt = devTransport(self.ip)

	def __del__(self):
		del self.devt