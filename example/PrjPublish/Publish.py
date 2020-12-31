""" 实现版本脚本发布功能
"""
import zipfile, random, time, json, os, sys, zlib, paramiko
from Logger import*
from DataModel import DataModel
from RestfulApiClient import RestfulApiClient

class Publish:
    def __init__(self, ftpsInfo, pubUrl):
        """
        # ftpsInfo: ftp上传信息集, 如: [("127.0.0.1", 22, 'usr', 'pwd')]
        # pubUrl: 发布url地址: 如 http://aa.bb.com:8888/update/
        """
        self.ftpsInfo, self.pubUrl = ftpsInfo,  pubUrl


    def Publish(self, prj, mn, ver, folder, pubFileName, detail = ''):
        try:
            decryptFileName, encryptFileName, data = '%s.%s.db'%(pubFileName, ver), '%s.%s.en.db'%(pubFileName, ver), None
            if not self.__packetFile(decryptFileName, folder):return print("打包%s:%s:%s失败"%(prj, mn, ver))# 打包文件
            with open(decryptFileName, 'rb') as f: data = f.read()
            if data is None: return print("读取文件(%s)失败"%(decryptFileName))
            lenData = len(data) # 计算定制hash计算
            half = int(lenData / 2)
            fileHash = "%08x%08x%08x%08x"%(lenData, zlib.crc32(data[0 : half]), zlib.crc32(data[half : ]), zlib.crc32(data))
            sk, skDict = self.__generalSk().encode(), self.__generalDict() # 生成加密密钥,字典
            data=[skDict[(((sk[n%len(sk)] << 8) & 0xFF00) | (data[n] & 0xFF)) & 0xFFFF] for n in range(lenData)]
            with open(encryptFileName, "wb") as f: f.write(bytes(data)) # 加密文件
            if not self.__sftpUploadEncryptFile(encryptFileName): return print("上传文件失败")
            dir, fn = os.path.split(encryptFileName)
            result = RestfulApiClient().Publish(prj, mn, ver, [1, ], detail, sk.decode(), fileHash, "%s%s"%(self.pubUrl, fn))
            Log(LOG_INFO, "Publish", json.dumps(result))
            return True
        except Exception as e:
            Log(LOG_ERROR, "Publish", "Run failed: %s"%e)
        return False


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


if __name__== '__main__':
    cfg = {
        "usr":"master", # 登录UI用账号
        "password":"master@er.com",# 登录UI用密码
        "pubUrl":"http://192.168.221.134:8210/", # 发布后下载url路径
        "pubTopic":"publish", # 
        "sftpAddr":[("192.168.221.134", 22, 'root', 'Baidu.com22')], # sftp上传ip、端口、账号、密码
        "prjs":[
            # 项目名,模块名,发布版本号,待发布文件目录，待发布文件名称前缀,发布描述
            ("aods", "aods-x64-win", "0.0.0.0001", "..\\aods-x64-win\\", "aods-x64-win", "fix the bug .1.023,1"),
            ("aods", "aods-x86-win", "0.0.0.0001", "..\\aods-x86-win\\", "aods-x86-win", "fix the bug .1.023,1"),
            ("aods", "aods-x64-linux", "0.0.0.0001", "..\\aods-x64-linux\\", "aods-x64-linux", "fix the bug .1.023,1"),
            ("aods", "aodc-x64-win", "0.0.0.0001", "..\\aodc-x64-win\\", "aodc-x64-win", "fix the bug .1.023,1")
            ]
    }    
    rlt = RestfulApiClient().Login(cfg['usr'], cfg['password']) # 修改为api发布消息
    if not rlt or rlt["login_result"]["result"] != "success":
        Log(LOG_ERROR, "Publish","Failed to login")
        sys.exit(1)
    DataModel.Instance().UpdateUser(rlt["login_result"])
    p = Publish(cfg["sftpAddr"], cfg["pubUrl"])
    for (prj, mn, ver, folder, pubFileName, detail) in cfg["prjs"]:
        if not p.Publish(prj, mn, ver, folder, pubFileName, detail):
            os.system("pause")
            sys.exit(1)
    print("执行结束......")
    time.sleep(20.0)