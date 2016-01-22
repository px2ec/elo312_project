# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mainwindow.ui'
#
# Created: Wed Jan 20 13:22:04 2016
#      by: PyQt4 UI code generator 4.10.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(460, 344)
        MainWindow.setMinimumSize(QtCore.QSize(460, 344))
        MainWindow.setMaximumSize(QtCore.QSize(460, 344))
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.pBsync = QtGui.QPushButton(self.centralwidget)
        self.pBsync.setGeometry(QtCore.QRect(350, 20, 94, 29))
        self.pBsync.setObjectName(_fromUtf8("pBsync"))
        self.pBalarm = QtGui.QPushButton(self.centralwidget)
        self.pBalarm.setGeometry(QtCore.QRect(350, 60, 94, 29))
        self.pBalarm.setObjectName(_fromUtf8("pBalarm"))
        self.listView = QtGui.QListView(self.centralwidget)
        self.listView.setGeometry(QtCore.QRect(20, 20, 311, 251))
        self.listView.setObjectName(_fromUtf8("listView"))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 460, 27))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        self.menuCliente = QtGui.QMenu(self.menubar)
        self.menuCliente.setObjectName(_fromUtf8("menuCliente"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)
        self.actionConectar = QtGui.QAction(MainWindow)
        self.actionConectar.setObjectName(_fromUtf8("actionConectar"))
        self.actionDetectar = QtGui.QAction(MainWindow)
        self.actionDetectar.setObjectName(_fromUtf8("actionDetectar"))
        self.actionSalir = QtGui.QAction(MainWindow)
        self.actionSalir.setObjectName(_fromUtf8("actionSalir"))
        self.menuCliente.addAction(self.actionConectar)
        self.menuCliente.addAction(self.actionDetectar)
        self.menuCliente.addSeparator()
        self.menuCliente.addAction(self.actionSalir)
        self.menubar.addAction(self.menuCliente.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(_translate("MainWindow", "Internet of Clocks", None))
        self.pBsync.setText(_translate("MainWindow", "Sincronizar", None))
        self.pBalarm.setText(_translate("MainWindow", "Alarma", None))
        self.menuCliente.setTitle(_translate("MainWindow", "Cliente", None))
        self.actionConectar.setText(_translate("MainWindow", "Conectar", None))
        self.actionDetectar.setText(_translate("MainWindow", "Detectar", None))
        self.actionSalir.setText(_translate("MainWindow", "Salir", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    MainWindow = QtGui.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())

