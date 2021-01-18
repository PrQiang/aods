import threading, traceback
from PySide2 import QtCore, QtWidgets, QtGui
from paramiko.client import SSHClient, AutoAddPolicy
from paramiko import AuthenticationException
from paramiko.ssh_exception import NoValidConnectionsError
import pyte, sys, time


def ThreadRun(sshWidget):
    sshWidget.run()


class sshWidget(QtWidgets.QWidget):
    signalShowText = QtCore.Signal()

    def __init__(self, addr, usr, pwd, pk = None, kf = None):
        super().__init__()
        self.initialize(addr, usr, pwd, pk, kf)


    def initialize(self, addr, usr, pwd, pk, kf):
        try:
            self.signalShowText.connect(self.ShowText)
            self.hasEndedScreen = False
            self.interactiveWidget = QtWidgets.QTextEdit(self)
            self.interactiveWidget.setFont(QtGui.QFont({
                'win32': 'Consolas',
                'linux': 'Monospace',
                'darwin': 'Andale Mono'
            }.get(sys.platform, 'Courier'), 10))
            #self.interactiveWidget.setFocusPolicy(QtCore.Qt.ClickFocus)
            self.interactiveWidget.setStyleSheet("background-color : green; color : #cccccc;")
            self.interactiveWidget.installEventFilter(self)
            self.interactiveWidget.selectionChanged.connect(self.interactiveWidget.copy)
            self.interactiveWidget.setContextMenuPolicy(QtCore.Qt.NoContextMenu)
            self.interactiveWidget.setCursorWidth(5)
            lay = QtWidgets.QHBoxLayout()
            lay.addWidget(self.interactiveWidget)
            self.setLayout(lay)

            # proc vt stream
            self.defaultLines = 240
            self._vt = pyte.Screen(self.defaultLines, 120)
            self.stream = pyte.Stream()
            self.stream.attach(self._vt)

            # connect to ssh
            (host, port) = addr.split(':')
            self.ssh = SSHClient()
            self.ssh.set_missing_host_key_policy(AutoAddPolicy())
            self.ssh.connect(host, port=int(port), username=usr, password=pwd, pkey = pk, key_filename=kf)
            self.shell = self.ssh.invoke_shell()
            self.thread = threading.Thread(target = ThreadRun, args = (self, ), daemon = True)
            self.thread.start()
            return True
        except Exception as e:
            traceback.print_exc()
            return False


    def run(self):
        time.sleep(0.5)
        while True:
            try:
                buf = self.shell.recv(999)
                if len(buf) < 1: break
                self.stream.feed(buf.decode("utf-8"))
                self.signalShowText.emit()
            except Exception as e:
                self.stream.feed("the network is wrong for: %s"%(traceback.format_exc()))
                self.signalShowText.emit()
                break


    @QtCore.Slot()
    def ShowText(self):
        try:
            self.interactiveWidget.setPlainText("\n".join(self._vt.display))
            self.hasEndedScree = self.hasEndedScreen or (self._vt.cursor.y + 1) >= self._vt.lines
            tc = self.interactiveWidget.textCursor()
            tc.setPosition(self._vt.cursor.y * (self._vt.columns + 1) + self._vt.cursor.x, QtGui.QTextCursor.MoveAnchor)
            self.interactiveWidget.setTextCursor(tc)
            self.interactiveWidget.setFocus()
        except Exception as e:
            traceback.print_exc()


    def mousePressEvent(self, QMouseEvent):
        if QMouseEvent.button() == QtCore.Qt.RightButton:
            clipText = QtWidgets.QApplication.clipboard().text()
            if clipText: # send the clip text to ssh server when right click
                self.Send(clipText)
        else:
            pass


    def resizeEvent(self, event):
        if hasattr(self, "_vt"):
            sz = QtGui.QFontMetrics(self.font()).size(0, " ")    
            columns = int(event.size().width() / sz.width())
            self._vt.resize(self.defaultLines, columns)
            self.ShowText()
        super().resizeEvent(event)


    def eventFilter(self, obj, event):
        if obj == self.interactiveWidget:
            if event.type() == QtCore.QEvent.KeyPress:
                self.Send(
                {
                    QtCore.Qt.Key_Tab: "\t",
                    QtCore.Qt.Key_Backspace: "\x7f",
                    QtCore.Qt.Key_Up: "\033[A",
                    QtCore.Qt.Key_Down: "\033[B",
                    QtCore.Qt.Key_Left: "\033[D",
                    QtCore.Qt.Key_Right:"\033[C"
                }.get(event.key(), event.text()))
                return True
        return False


    def Send(self, text):
        try:
            self.shell.send(text)
        except Exception as e:
            traceback.print_exc()


    def destroy(self, v1, v2):
        try:
            self.shell.close()
            self.ssh.close()
            self.thread.join()
        except Exception as e:
            traceback.print_exc()        
        return super().destroy(v1, v2)