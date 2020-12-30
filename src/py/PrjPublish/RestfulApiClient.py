import socket, hashlib, zlib, json, time, struct, traceback
from cryptography.fernet import Fernet
from DataModel import DataModel

class RestfulApiClient(object):
    def __init__(self, addr = '127.0.0.1', port = 32002):
        self.addr, self.port, self.cookie = addr, port, None


    def Login(self, usr, pwd):
        try:
            return self.sendAndWaitRsp({"login":{"usr":usr, "pwd":pwd}})
        except Exception as e:
            traceback.print_exc()
            return None


    def Publish(self, prj, module, ver, gids, detail, code, hash, url):
        return self.sendAndWaitRsp({"publish":{"cookie":DataModel.Instance().GetCookie(), "project":prj, "module":module, "version":ver, "gids":gids, "detail":detail, "code":code, "hash":hash, "url":url}})


    def sendAndWaitRsp(self, data):
        try:
            buf = json.dumps(data).encode()
            hash = zlib.crc32(buf, 0x31)
            enKey = Fernet.generate_key()
            enBuf = Fernet(enKey).encrypt(buf)
            enHash = zlib.crc32(enBuf, 0x4f)
            with socket.socket() as s:
                s.connect((self.addr, self.port))
                s.sendall(struct.pack("<5I", len(enBuf), 1, enHash, hash, len(enKey)) + enKey + enBuf)
                data = self.__Read(s, 20) # header
                if data is None:return
                length, ver, enHash, hash, leEnKey = struct.unpack("<5I", data)
                data = self.__Read(s, length + leEnKey)
                if data is None:return
                enKey = data[:leEnKey]
                data = data[leEnKey:]
                if enHash != zlib.crc32(data, 0x32):return None
                deData = Fernet(enKey).decrypt(data)
                if hash != zlib.crc32(deData, 0x50):return None
                return json.loads(deData.decode())
        except Exception as e:
            traceback.print_exc()
            return None


    def __Read(self, s, length):
        try:
            data, tempdata = b'', b''
            while len(data) < length:
                tempdata = s.recv(length - len(data))
                if len(tempdata) < 1: return None
                data += tempdata
            return data
        except Exception as e:
            traceback.print_exc()
            return None