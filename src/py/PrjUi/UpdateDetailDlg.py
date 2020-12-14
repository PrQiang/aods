"""
    module update detail 
"""
import traceback
from RestfulApiClient import RestfulApiClient
from PySide2 import QtCore, QtWidgets, QtGui
from SmWidget import SmWidget


class CustonContextMenuTableWidget(QtWidgets.QTableWidget):
    def __init__(self, par = None):
        super().__init__(par)       
        self.setContextMenuPolicy(QtCore.Qt.DefaultContextMenu)
        self.horizontalHeader().setSectionResizeMode( QtWidgets.QHeaderView.Stretch)
        self.setToolTip("right click the selected items to bach control server to get details")
        self.verticalHeader().hide()
        self.setColumnCount(3)
        self.setSortingEnabled(True)
        self.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)


    def contextMenuEvent(self, event):
        try:
            if len(self.selectedItems()) < 1: return
            self.onBatchCtrl()
            # menu = QtWidgets.QMenu(self, "&popup")
            # actionBatchCtrl = QtWidgets.QAction("batch control")
            # actionBatchCtrl.setText("batch control")
            # actionBatchCtrl.setToolTip("now only ssh supported")
            # actionBatchCtrl.triggered.connect(self.onBatchCtrl)
            # menu.addAction(actionBatchCtrl)
            # menu.exec_(QtGui.QCursor.pos())
            # super().contextMenuEvent(event)
        except Exception as e:
            print(e)


    def onBatchCtrl(self):
        try:
            # query info from server
            rlt = RestfulApiClient().QueryServerInfo([item.data(QtCore.Qt.UserRole) for item in self.selectedItems() if item.data(QtCore.Qt.UserRole)])
            if not rlt or rlt.get("query_server_result", {}).get("result") != "success":return
            sw = SmWidget()
            for rid, si in rlt["query_server_result"]["detail"].items():
                si["rid"] = rid
                sw.appendItem(si)
            dlg = QtWidgets.QDialog(self)
            layout = QtWidgets.QGridLayout(dlg)
            layout.setSpacing(0)
            layout.addWidget(sw)
            dlg.showMaximized()
            dlg.exec_()
        except Exception as e:
            traceback.print_exc()


    def __ipType(ip):
        intip = socket.ntohl(struct.unpack("I",socket.inet_aton(str(ip)))[0])
        if (intip >= 0xa000000 and intip <= 0xaffffff) or (intip >= 0x64400000 and intip <= 0x647fffff) or \
            (intip >= 0xac100000 and intip <= 0xac1fffff) or (intip >= 0xc0a80000 and intip <= 0xc0a8ffff):
            return "lan"
        elif intip >= 0xa9fe0000 and intip <= 0xa9feffff:
            return "pre assinged"
        elif intip >= 0xE0000000 and intip <= 0xE00000ff: # 224.0.0.0～224.0.0.255
            return "reversed multicast"
        elif intip >= 0xE0000100 and intip <= 0xE00001ff: # 224.0.1.0～224.0.1.255
            return "public multicast"
        elif intip >= 0xE0000200 and intip <= 0xEEFFFFFF: # 224.0.2.0～238.255.255.255为
            return "user multicast"
        elif intip >= 0xEF000000 and intip <= 0xEFFFFFFF: # 239.0.0.0～239.255.255.255
            return "local multicast"
        elif intip > 0xEFFFFFFF: # 240.0.0.0 - 255.255.255.255
            return "reversed"
        elif intip == 0:
            return ""
        else:
            return "wan"


class UpdateDetailDlg(QtWidgets.QDialog):
    def __init__(self, prj, module, version, par = None):
        super().__init__(par)
        self.prj, self.module, self.version = prj, module, version
        self.__initUI()


    def __initUI(self):
        try:
            self.setWindowTitle("Update Details")
            hLayout = QtWidgets.QVBoxLayout(self)
            self.detail = QtWidgets.QPlainTextEdit()
            self.detail.setReadOnly(True)
            self.detail.setMaximumHeight(220)
            hLayout.addWidget(self.detail)
            self.tb = CustonContextMenuTableWidget()
            hLayout.addWidget(self.tb)
            # beauty this dialog
            with open("./qss/UpdateDetailDlg.qss", "rb") as f:
                self.setStyleSheet(f.read().decode())
            self.__updateData()
        except Exception as e:
            traceback.print_exc()


    def __updateData(self):
        try:
            self.tb.clear()
            self.tb.setHorizontalHeaderLabels(["time", "server", "status"])
            result = RestfulApiClient().QueryUpdateDetail(self.prj, self.module, self.version)
            if not result: return
            if result.get("query_update_result", {}).get("result") != "success":return
            dtLatest, detailLatest, totalUpdateInfo = None, None, None
            for (gid, dt, detail) in result["query_update_result"]["detail"][0]:
                if not dtLatest or dtLatest < dt:
                    dtLatest, detailLatest = dt, detail
                if not totalUpdateInfo: totalUpdateInfo = '%s group %s updated'%(dt, gid)
                else: totalUpdateInfo += '\r\n%s group %s updated'%(dt, gid)
            self.detail.setPlainText("%s %s %s detail:\r\n %s\r\n\r\n%s"%(self.prj, self.module, self.version, detailLatest, totalUpdateInfo))
            self.detail.adjustSize()
            for (rid, flag, rlt, dt) in result["query_update_result"]["detail"][1]:
                self.tb.insertRow(0)
                dtItem = QtWidgets.QTableWidgetItem(dt)
                ridItem = QtWidgets.QTableWidgetItem(flag)
                ridItem.setData(QtCore.Qt.UserRole, rid)
                rltItem = QtWidgets.QTableWidgetItem(rlt)
                if rlt not in ('更新成功', ): color = QtGui.QColor(0xFF, 0xB8, 00)
                else: color = QtGui.QColor(0x5F, 0xB8, 0x78)
                dtItem.setBackgroundColor(color)
                ridItem.setBackgroundColor(color)
                rltItem.setBackgroundColor(color)
                self.tb.setItem(0, 0, dtItem)
                self.tb.setItem(0, 1,ridItem)
                self.tb.setItem(0, 2, rltItem)
            self.tb.sortByColumn(2)
        except Exception as e:
            traceback.print_exc()