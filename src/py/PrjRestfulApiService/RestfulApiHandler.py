import socketserver, struct, zlib, json, time, hashlib
from cryptography.fernet import Fernet
from Logger import *
from DbEngine import DbEngine
from SafeMon import SafeMon

class RestfulApiHandler(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            data = self.__Read(20) # header
            if data is None:return
            length, ver, enHash, hash, leEnKey = struct.unpack("<5I", data)
            data = self.__Read(length + leEnKey)            
            if data is None:return
            enKey = data[:leEnKey]
            data = data[leEnKey:]            
            if enHash != zlib.crc32(data, 0x4f):return None
            deData = Fernet(enKey).decrypt(data)
            if hash != zlib.crc32(deData, 0x31):return None
            jData = json.loads(deData.decode())
            for k, v in jData.items():
                if k == 'login':
                    self.__login(v)
                elif k == 'query_pms':
                    self.__onQueryProjectModules(v)
                elif k == 'query_mi':
                    self.__onQueryModuleInfo(v)
                elif k == "query_update_detail":
                    self.__onQueryUpdateDetail(v)
                elif k == "query_server":
                    self.__onQueryServer(v)
                elif k == "update_server":
                    self.__onUpdateServer(v)
                elif k == "delete_servers":
                    self.__onDeleteServers(v)
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "handle failed: %s"%(e))


    def __Read(self, length):
        try:
            data, tempdata = b'', b''
            while len(data) < length:
                tempdata = self.request.recv(length - len(data))
                if len(tempdata) < 1:
                    return None
                data += tempdata
            return data
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "recv data(length: %s): %s"%(length, e))
            return None


    def __login(self, value):
        try:
            ui = DbEngine.Instance().queryUserInfo(value["usr"]) # login failed
            if not ui:
                self.__rsp({"login_result":{"result":"failed"}})
            else: # login Success general a cookie to cli
                dUi = json.loads(ui.decode())
                sha = hashlib.sha256()
                sha.update(value["pwd"].encode())
                if dUi["pwd"] != sha.hexdigest(): # check the password
                    self.__rsp({"login_result":{"result":"failed"}})
                    return
                sha = hashlib.sha256()
                sha.update(str(time.time()).encode())
                cookie = sha.hexdigest()
                DbEngine.Instance().UpdateCookie(cookie, value["usr"])
                self.__rsp({"login_result":{"result":"success", "cookie":cookie, "isAdmin":dUi["isAdmin"]}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "login error for %s"%(e))


    def __onQueryProjectModules(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"query_pms_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"query_pms_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check prjects for the usr                
            self.__rsp({ "query_pms_result":{"result":"success", "detail":DbEngine.Instance().queryProjectModules()}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query projects info(%s) error for %s"%(v, e))


    def __onQueryModuleInfo(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"query_mi_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"query_mi_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check prjects for the usr
            self.__rsp({ "query_mi_result":{"result":"success", "detail":DbEngine.Instance().queryModuleInfo(v["project"], v["module"])}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query projects info(%s) error for %s"%(v, e))


    def __onQueryUpdateDetail(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"query_update_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"query_update_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check permission
            self.__rsp({ "query_update_result":{"result":"success", "detail":DbEngine.Instance().queryUpdateDetail(v["project"], v["module"], v["version"])}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query update detail info(%s) error for %s"%(v, e))



    def __onQueryServer(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"query_server_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"query_server_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check permission
            self.__rsp({ "query_server_result":{"result":"success", "detail":DbEngine.Instance().queryServer(v["rids"], usr)}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query update detail info(%s) error for %s"%(v, e))
        

    def __onUpdateServer(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"update_server_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"update_server_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check permission
            self.__rsp({ "update_server_result":{"result":"success", "detail":DbEngine.Instance().updateServer(v["si"], usr)}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query update detail info(%s) error for %s"%(v, e))


    def __onDeleteServers(self, v):
        try:
            cookie = v.get("cookie")
            if not cookie:
                self.__rsp({"delete_servers_result":{"result":"failed for unathen"}})
                return
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:
                self.__rsp({"delete_servers_result":{"result":"failed", "detail":"Please authen first"}})
                return
            # todo check permission
            self.__rsp({ "delete_servers_result":{"result":"success", "detail":DbEngine.Instance().deleteServers(v["rids"])}})
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "query update detail info(%s) error for %s"%(v, e))


    def __rsp(self, data):
        try:
            buf = json.dumps(data).encode()            
            hash = zlib.crc32(buf, 0x50)
            enKey = Fernet.generate_key()
            enBuf = Fernet(enKey).encrypt(buf)
            enHash = zlib.crc32(enBuf, 0x32)
            self.request.sendall(struct.pack("<5I", len(enBuf), 1, enHash, hash, len(enKey)) + enKey + enBuf)
        except Excetpion as e:
            Log(LOG_ERROR, "RestfulApiHandler", "response error for %s"%(e))


    def __getCookieInfo(self, cookie):
        try:
            usr = DbEngine.Instance().queryCookieUser(cookie)
            if not usr:return None
            DbEngine.Instance().UpdateCookie(cookie, usr)
            return usr
        except Exception as e:
            Log(LOG_ERROR, "RestfulApiHandler", "chkcookie error for %s"%(e))
            return None