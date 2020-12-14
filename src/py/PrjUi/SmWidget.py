""" server manager widget
"""
import traceback
from BatchCmdDlg import BatchCmdDlg
from BatchUploadDlg import BatchUploadDlg
from PySide2 import QtCore, QtWidgets, QtGui
from sshShell import sshShell
from RestfulApiClient import RestfulApiClient


class SmWidget(QtWidgets.QWidget):
    def __init__(self, all = False):
        super().__init__()
        self.initUi()
        self.__initData(all)


    def initUi(self):
        try:
            self.searchLe = QtWidgets.QLineEdit()
            self.searchLe.textChanged.connect(self.search)
            self.searchBtn = QtWidgets.QPushButton("search")
            self.searchBtn.setFixedWidth(44)
            self.firstLineLay = QtWidgets.QHBoxLayout()
            self.firstLineLay.addSpacerItem(QtWidgets.QSpacerItem(600, 30))
            self.firstLineLay.addWidget(QtWidgets.QLabel("search"))
            self.firstLineLay.addWidget(self.searchLe)
            self.btnBatchCmd = QtWidgets.QPushButton("batch cmd")
            self.btnBatchCmd.clicked.connect(self.onBatchCmdClicked)
            self.firstLineLay.addWidget(self.btnBatchCmd)
            self.btnBatchUpload = QtWidgets.QPushButton("batch upload")
            self.btnBatchUpload.clicked.connect(self.onBatchUploadClicked)
            self.firstLineLay.addWidget(self.btnBatchUpload)
            self.btnRefresh = QtWidgets.QPushButton("refresh")
            self.btnRefresh.clicked.connect(self.onRefresh)
            self.firstLineLay.addWidget(self.btnRefresh)
            w = QtWidgets.QWidget()
            w.setLayout(self.firstLineLay)
            w.setFixedHeight(44)
            names = ["tag","ip", "location", "name", "password", "manage address", "system"]
            self.tableWidget = QtWidgets.QTableWidget(0, len(names))
            for i in range(self.tableWidget.columnCount()):
                item = QtWidgets.QTableWidgetItem(names[i])
                self.tableWidget.setHorizontalHeaderItem(i, item)
            self.tableWidget.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
            self.tableWidget.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.Fixed)
            self.tableWidget.setSelectionBehavior (QtWidgets.QAbstractItemView.SelectRows)
            self.tableWidget.itemChanged.connect(self.onItemChanged)
            self.vLay = QtWidgets.QVBoxLayout(self)
            self.vLay.addWidget(w)
            self.vLay.addWidget(self.tableWidget)
            self.setLayout(self.vLay)
            with open("./qss/SmWidget.qss", "rb") as f:
                self.setStyleSheet(f.read().decode())
        except Exception as e:
            traceback.print_exc()


    def onRefresh(self):
        while self.tableWidget.rowCount() > 1:
            self.tableWidget.removeRow(0)
        self.__initData(True)


    def __initData(self, all):
        try:
            if not all:return
            rlt = RestfulApiClient().QueryServerInfo(None)
            if not rlt or rlt.get("query_server_result", {}).get("result") != "success":return
            for rid, si in rlt["query_server_result"]["detail"].items():
                si["rid"] = rid
                self.appendItem(si)
        except Exception as e:
            traceback.print_exc()


    def appendItem(self, si):
        self.tableWidget.itemChanged.disconnect(self.onItemChanged)
        self.tableWidget.insertRow(self.tableWidget.rowCount())
        row = self.tableWidget.rowCount() - 1

        # flag
        item = QtWidgets.QTableWidgetItem(si.get("flag", ""))
        item.setData(QtCore.Qt.UserRole, si)
        self.tableWidget.setItem(row, 0, item)

        # ip
        item = QtWidgets.QTableWidgetItem(si.get("ip", ''))
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 1, item)

        # location
        item = QtWidgets.QTableWidgetItem(si.get("location", ''))
        self.tableWidget.setItem(row, 2, item)

        # usr
        item = QtWidgets.QTableWidgetItem(si.get("usr", ''))
        self.tableWidget.setItem(row, 3, item)

        # password
        item = QtWidgets.QTableWidgetItem(si.get("pwd", ''))
        self.tableWidget.setItem(row, 4, item)

        # manage addr
        item = QtWidgets.QTableWidgetItem(si.get("ma", ''))
        item.setToolTip("manage address must be like ip:port, eg: 1.1.1.1:22")
        self.tableWidget.setItem(row, 5, item)        

        # system info
        item = QtWidgets.QTableWidgetItem(si.get("system", ''))
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 6, item)

        self.tableWidget.itemChanged.connect(self.onItemChanged)


    def search(self, searchText):
        try:
            showRows = []
            for item in self.tableWidget.findItems(searchText, QtCore.Qt.MatchContains&(~QtCore.Qt.MatchCaseSensitive)):
                if item.row() not in showRows: showRows.append(item.row())
            for i in range(self.tableWidget.rowCount()):
                if i not in showRows: self.tableWidget.hideRow(i)
                else:self.tableWidget.showRow(i)
        except Exception as e:
            traceback.print_exc()


    def getBatchSelectedShell(self):
        batchList = []
        for item in self.tableWidget.selectedItems():
            if item.column() != 0:continue  
            if self.tableWidget.isRowHidden(item.row()):continue
            si = item.data(QtCore.Qt.UserRole)
            if si:
                try:
                    batchList.append(sshShell("%s(%s%s)"%(item.text(),si.get("location"), si.get("ip")), si.get("ma"), si.get("usr"), si.get("pwd"), si.get("pkey"), si.get("key_filename"), 'utf8'))
                except Exception as e:
                    print("add shell(%s) error : %s"%(si, e))
        return batchList


    def onBatchCmdClicked(self):
        try:
            BatchCmdDlg(self.getBatchSelectedShell(), self).exec()
        except Exception as e:
            traceback.print_exc()


    def onBatchUploadClicked(self):
        BatchUploadDlg(self.getBatchSelectedShell(), self).exec()


    def onItemChanged(self, item):
        try:
            si = self.tableWidget.item(item.row(), 0).data(QtCore.Qt.UserRole)
            col = item.column()
            if col == 0: # change the flag/tag
                si["flag"] = item.text()
            elif col == 2: # location
                si["location"] = item.text()
            elif col == 3: # usr
                si["usr"] = item.text()
            elif col == 4: # password
                si["pwd"] = item.text()
            elif col == 5: # manage address
                chk1 = item.text().split(":")
                if len(chk1) != 2: # check the ip and port maybe a better choice 
                    QtWidgets.QMessageBox(None, "tips", "must be like ip:port, eg: 1.1.1.1:22", parent = self)
                    return
                si["ma"] = item.text() 
            else: return
            RestfulApiClient().UpdateServerInfo(si) # update chg info (async, show the failed info when failed later)
            self.tableWidget.item(item.row(),0).setData(QtCore.Qt.UserRole, si)
        except Exception as e:
           traceback.print_exc()
