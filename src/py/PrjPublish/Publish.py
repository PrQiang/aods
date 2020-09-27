""" 实现版本脚本发布功能
"""
import zipfile, random, time, json, os, sys, zlib, paramiko
from Logger import*
from KafkaProducer import KafkaProducer

class Publish:
    def __init__(self, brokers, ftpsInfo, pubTopic, pubUrl):
        """
        # brokers： kafka地址: 如： 127.0.0.1:6539
        # ftpsInfo: ftp上传信息集, 如: [("127.0.0.1", 22, 'usr', 'pwd')]
        # pubUrl: 发布url地址: 如 http://aa.bb.com:8888/update/
        # pubTopic: 通知主题
        """
        self.producer, self.ftpsInfo, self.pubTopic, self.pubUrl = KafkaProducer(brokers), ftpsInfo,  pubTopic, pubUrl


    def Publish(self, prj, mn, ver, folder, pubFileName, pubType):
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
            self.producer.Produce(self.pubTopic, json.dumps({"publish":{"project":prj, "module":mn, "version":ver, "hash":fileHash, "code":sk.decode(),"indexes":{"beta":[1],"alpha":[2],"stable":[i for i in range(3, 256)]}.get(pubType, [1]),  "url":"%s%s"%(self.pubUrl, fn)}}).encode())
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
        "brokers":"192.168.66.124:9092",
        "pubUrl":"http://192.168.66.124:8210/",
        "pubTopic":"publish",
        "sftpAddr":[("192.168.66.124", 22, 'root', 'Baidu.com22')],
        "prjs":[
            # 项目名,模块名,发布版本号,待发布文件目录，待发布文件名称前缀, 发布类型beta, alpha, stable, ...
            ("aom", "aom-win", "0.0.0.0001", "D:\\test\\publish\\aods-x64-linux\\", "D:\\test\\publish\\aods-x64-linux", "beta")
            ]
    }
    p = Publish(cfg["brokers"], cfg["sftpAddr"], cfg["pubTopic"], cfg["pubUrl"])
    for (prj, mn, ver, folder, pubFileName, pubType) in cfg["prjs"]:
        if not p.Publish(prj, mn, ver, folder, pubFileName, pubType):
            os.system("pause")
            sys.exit(1)
    print("发布成功......")
    time.sleep(20.0)