import traceback, paramiko, zipfile, time
from PySide2 import QtWidgets, QtGui, QtCore
from RestfulApiClient import RestfulApiClient
from UIBaseDlg import UIBaseDlg

class PublishDlg(UIBaseDlg):
    def __init__(self, prj, module, ver, hash, code, url, detail, gids, par = None):
        super().__init__("publish", par = par)
        self.__initUi()
        self.hash, self.code, self.url = hash, code, url
        self.__initData(prj, module, ver, detail, gids)
        

    def __initUi(self):
        try:
            gLayout = QtWidgets.QGridLayout()
            gLayout.setSpacing(10)

            self.lePrj = QtWidgets.QLineEdit()
            self.lePrj.setPlaceholderText("Please input project")
            self.lePrj.setReadOnly(True)
            gLayout.addWidget(self.lePrj, 0, 1, QtCore.Qt.AlignCenter)

            self.leModule = QtWidgets.QLineEdit()
            self.leModule.setPlaceholderText("Please input module")
            self.lePrj.setReadOnly(True)
            gLayout.addWidget(self.leModule, 1, 1, QtCore.Qt.AlignCenter)

            self.leVer = QtWidgets.QLineEdit()
            self.leVer.setPlaceholderText("Please input version")
            self.leVer.setReadOnly(True)
            gLayout.addWidget(self.leVer, 2, 1, QtCore.Qt.AlignCenter)

            self.leGroup = QtWidgets.QLineEdit()
            self.leGroup.setPlaceholderText("Please input group ids with comma separation")
            gLayout.addWidget(self.leGroup, 3, 1, QtCore.Qt.AlignCenter)

            self.pteDetail = QtWidgets.QLabel()
            self.pteDetail.setMinimumHeight(220)
            #self.pteDetail.setReadOnly(True)
            #gLayout.addWidget(self.pteDetail, 4, 1, QtCore.Qt.AlignCenter)

            self.btn = QtWidgets.QPushButton("Commit")
            self.btn.clicked.connect(self.onClickCommit)
            gLayout.addWidget(self.btn, 4, 1, QtCore.Qt.AlignCenter)
            vLayout = QtWidgets.QVBoxLayout(self)
            vLayout.addLayout(self.hLayout)
            vLayout.addSpacerItem(QtWidgets.QSpacerItem(self.width(), 32))
            vLayout.addLayout(gLayout)
            vLayout.addSpacerItem(QtWidgets.QSpacerItem(self.width(), 32))
            self.adjustSize()

        except Exception as e:
            traceback.print_exc()


    def __initData(self, prj, module, ver, detail, gids):
        try:
            self.lePrj.setText(prj)
            self.leModule.setText(module)
            self.leVer.setText(ver)
            self.pteDetail.setText(detail)
            self.gids = gids
        except Exception as e:
            traceback.print_exc()


    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.drawPixmap(0, 0, self.pmp)


    def onClickCommit(self):
        try:
            prj, module, ver, gids, detail = self.lePrj.text(), self.leModule.text(), self.leVer.text(), self.leGroup.text().split(','), self.pteDetail.text()
            if not prj:
                self.lePrj.setFocus()
                return
            if not module:
                self.leModule.setFocus()
                return
            if not ver:
                self.leVer.setFocus()
                return
            if not gids:
                self.leGroup.setFocus()
                return
            intGids = []
            for gid in gids:
                try:
                    intGid = int(gid)
                    if intGid < 1 or intGid > 255:
                        self.leGroup.setFocus()
                        return
                    intGids.append(intGid)
                except Exception as e:
                    self.leGroup.setFocus()
                    return
            if not detail:
                self.pteDetail.setFocus()
                return
            RestfulApiClient().Publish(prj, module, ver, intGids, detail, self.code, self.hash, self.url)
            self.accept()
        except Excepiton as e:
            traceback.print_exc()


    def Packet(self, prj, mn, ver, folder, pubFileName):
        try:
            decryptFileName, encryptFileName, data = '%s.%s.db'%(pubFileName, ver), '%s.%s.en.db'%(pubFileName, ver), None
            if not self.__packetFile(decryptFileName, folder):return print("打包%s:%s:%s失败"%(prj, mn, ver)), None, None# 打包文件
            with open(decryptFileName, 'rb') as f: data = f.read()
            if data is None: return print("读取文件(%s)失败"%(decryptFileName)), None, None
            lenData = len(data) # 计算定制hash计算
            half = int(lenData / 2)
            fileHash = "%08x%08x%08x%08x"%(lenData, zlib.crc32(data[0 : half]), zlib.crc32(data[half : ]), zlib.crc32(data))
            sk, skDict = self.__generalSk().encode(), self.__generalDict() # 生成加密密钥,字典
            data=[skDict[(((sk[n%len(sk)] << 8) & 0xFF00) | (data[n] & 0xFF)) & 0xFFFF] for n in range(lenData)]
            with open(encryptFileName, "wb") as f: f.write(bytes(data)) # 加密文件
            return encryptFileName, fileHash, sk.decode()
        except Exception as e:
            traceback.print_exc()
        return None, None, None


    def __packetFile(self, fileName, folder):
        try:
            with zipfile.ZipFile(fileName, 'w', zipfile.ZIP_DEFLATED) as zf:
                for root, sroot, files in os.walk(folder):
                    for file in files:
                        fullFileName = os.path.join(root, file)
                        zf.write(fullFileName, fullFileName.replace(folder, ""))
            return True
        except Exception as e:
            Log(LOG_ERROR, "Publish", "__packetFile(%s) failed: %s"%(fileName, e))
        return False


    def __generalSk(self, skLen = 16):
        kv = '0123456789ABCDEFGHIJKLMNOPQRSTUabcdefghijklmnopqrstuvwxyzVWXYZ0123456789fghijklmnopqrstuvwx'
        lenKv, rd = len(kv), random.Random(time.time())
        return ''.join([kv[rd.randint(0, lenKv - 1)] for i in range(skLen)])


    def __generalDict(self):
        return bytes([(i + 10) % 256 for i in range(0, 65536)])


    def __sftpUploadEncryptFile(self, fileWithPath):
        try:
            for (addr, port, usr, pwd) in self.ftpsInfo:
                if not self.__singleSFtpUpload(addr, port, usr, pwd, fileWithPath): return False
            return True
        except Exception as e:
            Log(LOG_ERROR, "Publish", "__sftpUploadEncryptFile(%s) failed: %s"%(fileWithPath, e))
            return False


    def __singleSFtpUpload(self, addr, port, usr, pwd, fileWithPath):
        try:
            transport = paramiko.Transport((addr, int(port)))
            transport.connect(username=usr, password=pwd)
            sftp = paramiko.SFTPClient.from_transport(transport)
            dir, fn = os.path.split(fileWithPath)
            sftp.put(fileWithPath, '/var/www/html/%s'%fn)
            sftp.close()
            return True
        except Exception as e:
            Log(LOG_ERROR, "Publish", "__singleFtpUpload(%s:%s, %s, %s) failed: %s"%( addr, port, usr, fileWithPath, e))
        return False