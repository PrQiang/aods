from PySide2 import QtWidgets, QtCore, QtGui
import threadpool

class ShellTableWidget(QtWidgets.QTableWidget):
    def __init__(self, backColor, mode = QtWidgets.QHeaderView.ResizeToContents):
        super().__init__()
        self.backColor = backColor
        self.horizontalHeader().setSectionResizeMode(mode)
        self.verticalHeader().setSectionResizeMode(mode)
        self.setColumnCount(4)
        self.items = []


    def appendItem(self, text):
        row, col = int(len(self.items)/self.columnCount()), len(self.items)%self.columnCount()
        if row + 1 > self.rowCount():
            self.insertRow(row)
        item = QtWidgets.QTableWidgetItem(text)
        #item.setBackground(self.backColor)
        item.setToolTip(text)
        self.setItem(row, col, item)
        self.items.append(text)


    def clear(self):
        self.items.clear()
        super().clear()


class BatchUploadDlg(QtWidgets.QDialog):
    signalResult = QtCore.Signal(bool, str)
    def __init__(self, batchItems, parent= None):
        super().__init__(parent)
        self.initUI(batchItems)


    def initUI(self, batchItems):
        self.setModal(True)
        self.signalResult.connect(self.doResult)
        # design first operator
        self.uploadFiles = QtWidgets.QTextEdit()
        self.uploadFiles.setPlaceholderText("Please select the local files to upload")
        self.btnSrc = QtWidgets.QPushButton("...")
        self.btnSrc.clicked.connect(self.selectUploadFiles)
        self.editDst = QtWidgets.QTextEdit()
        self.editDst.setPlaceholderText("Please input the destination of uploads")
        self.btnUpload = QtWidgets.QPushButton("upload")
        self.btnUpload.clicked.connect(self.upload)
        hLay = QtWidgets.QHBoxLayout()
        hLay.addWidget(self.uploadFiles)
        hLay.addWidget(self.btnSrc)
        hLay.addWidget(self.editDst)
        hLay.addWidget(self.btnUpload)
        w = QtWidgets.QWidget()
        w.setLayout(hLay)
        w.setMaximumHeight(100)

        # show
        self.tabWidget = QtWidgets.QTabWidget()
        self.widgetDoing = ShellTableWidget(QtGui.QColor(220,220,220),QtWidgets.QHeaderView.Stretch)
        self.tabWidget.addTab(self.widgetDoing, "todo")
        self.widgetSuc = ShellTableWidget(QtGui.QColor(144,238,144))
        self.tabWidget.addTab(self.widgetSuc, "sucess")
        self.widgetFai = ShellTableWidget(QtGui.QColor(225,120,120))
        self.tabWidget.addTab(self.widgetFai, "failed")
        vLay = QtWidgets.QVBoxLayout()
        vLay.addWidget(w)
        vLay.addWidget(self.tabWidget)
        self.setLayout(vLay)
        self.labelShells = batchItems
        for shell in self.labelShells:self.widgetDoing.appendItem(shell.Name())    
        self.setModal(True)
        self.showMaximized()
        with open("./qss/TabWidget.qss", "rb") as f:
            self.setStyleSheet(f.read().decode())


    def selectUploadFiles(self):
        dlg = QtWidgets.QFileDialog(None, "选择上传文件")
        dlg.setFileMode(QtWidgets.QFileDialog.ExistingFiles)
        dlg.setViewMode(QtWidgets.QFileDialog.Detail)
        dlg.setModal(True)
        if dlg.exec_() == QtWidgets.QDialog.Accepted:
            self.uploadFiles.setText('\r\n'.join(dlg.selectedFiles()))


    def doResult(self, isSuc, result):
        try:
            if isSuc: self.widgetSuc.appendItem(result)
            else: self.widgetFai.appendItem(result)
        except Exception as e:
            traceback.print_exc()


    def upload(self):
        files = self.uploadFiles.toPlainText().split("\n")
        if len(files) < 1:
            QtWidgets.QMessageBox(None, "Tooltip", "Please select upload file at first!").exec_()
            return
        uploadDir = self.editDst.toPlainText()
        self.widgetSuc.clear()
        self.widgetFai.clear()
        tp = threadpool.ThreadPool(min(len(self.labelShells), 20))
        for labelShell in self.labelShells:
            tp.putRequest(threadpool.WorkRequest(self.request, (files, uploadDir, labelShell)))
        tp.wait()


    def request(self, files, uploadDir, labelShell):
        try:
            result,desc = labelShell.Upload(files, uploadDir)
            if result is True:
                suc, fai = [], []
                for (fn, (rlt, span)) in desc:
                    if rlt == 'ok': suc.append('%s ==> total span： %d ms'%(fn, span))
                    else:fai.append("%s span: %s ms\r\n \t%s"%(fn, span, rlt))
                if len(suc) > 0:
                    self.signalResult.emit(True, "%s\r\n%s"%(labelShell.Name(), '\r\n'.join(suc)))
                else:
                    self.signalResult.emit(False, "%s\r\n%s"%(labelShell.Name(), '\r\n'.join(fai)))
            else:self.signalResult.emit(result, "%s\r\n%s"%(labelShell.Name(), desc))
        except Exception as e:
            traceback.print_exc()
