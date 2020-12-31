import threading, redis, json, random, datetime, struct
from KafkaProducer import KafkaProducer
from cryptography.fernet import Fernet
from Logger import *

class DbEngine(object):
    _lock = threading.Lock()

    def __init__(self):
        addr, port, pwd = "192.168.221.134", 65379, 'pd3@a^,.)992'
        self.db_mgr = redis.Redis(host=addr, port = port, db = 1, password=pwd) # server manager
        self.db_pub = redis.Redis(host=addr, port = int(port), db = 2, password=pwd) # publish collection
        self.db_log = redis.Redis(host=addr, port = int(port), db = 3, password=pwd) # publish log
        self.db_ui = redis.Redis(host=addr, port = port, db = 5, password=pwd) # user info
        self.db_cookie = redis.Redis(host=addr, port = port, db = 6, password=pwd) # user cookie
        self.db_umi = redis.Redis(host=addr, port = port, db = 7, password=pwd) # user mange server info
        self.ttl, self.p = 3600, KafkaProducer("192.168.221.134:9092")


    @classmethod
    def Instance(cls):
        if not hasattr(DbEngine, "_instance"):
            with DbEngine._lock:
                if not hasattr(DbEngine, "_instance"):
                    DbEngine._instance = DbEngine()
        return DbEngine._instance


    def queryUserInfo(self, usr):
        try:
            return self.db_ui.get(usr)
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query login info for: %s"%e)
            return None


    def UpdateCookie(self, cookie, usr):
        self.db_cookie.set(cookie, usr, self.ttl)


    def queryCookieUser(self, cookie):
        return self.db_cookie.get(cookie)


    def queryProjectModules(self):
        try:
            prjs = {}
            for key in self.db_pub.keys():
                index, prj, module, ver = key.decode().split('#')
                if prj not in prjs: prjs[prj] = [module]
                elif module not in prjs[prj]:prjs[prj].append(module)
            return prjs
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server base info for: %s"%(e))
            return {}


    def queryModuleInfo(self, prj, module):
        try:            
            vis = {} # collect data
            for key in self.db_pub.keys("*#%s#%s#*"%(prj, module)):
                gid, p,  m, v = key.decode().split('#')
                hash, code, url, dt, detail, usr = json.loads(self.db_pub.get(key))
                if v not in vis: vis[v] = (dt, gid, hash, url, detail, usr)
                else:
                    (dt2, gid2, hash, url, detail, usr) = vis[v]
                    if dt2 < dt: dt2 = dt
                    gid2 += ', %s'%(gid)
                    vis[v] = (dt2, gid2, hash, url, detail, usr)
            # sort by the date
            return sorted(vis.items(), key=lambda item: item[1][0])
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server base info for: %s"%(e))
            return []


    def queryUpdateDetail(self, prj, module, ver):
        try:
            pubResult = []
            for key in self.db_pub.keys("*#%s#%s#*"%(prj, module)):
                gid, p,  m, v = key.decode().split('#')
                gid = int(gid, 16)
                hash, code, url, dt, detail, usr = json.loads(self.db_pub.get(key))
                if v != ver: continue
                pubResult.append((gid, hash, code, url, dt, detail, usr))
            result = []
            for key in self.db_log.keys("*#%s#%s#%s"%(prj, module, ver)):
                rid = key.decode().split('#')[0]
                rlt, dt = self.db_log.get(key).decode().split('#')
                si = self.db_mgr.get(rid)
                if si == b'null' or not si: continue
                si = json.loads(si.decode())
                result.append((rid, si.get("flag", si.get("ip", rid)), rlt, dt))
            return [pubResult, result]
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server base info for: %s"%(e))
            return [[], []]


    def queryAllServer(self, usr):
        try:
            srvs = {}
            for rid in self.db_mgr.keys("*"):
                info = self.db_mgr.get(rid).decode()
                if info == 'null': continue
                srvs[rid.decode()] = json.loads(self.db_mgr.get(rid).decode())
            for key in self.db_umi.keys("%s#*"%(usr)):
                rid = key.decode().split('#')[-1]
                if rid not in srvs: continue
                srvs[rid].update(json.loads(self.__decryptUsiInfo(self.db_umi.get(key)).decode()))
                
            return srvs
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query all server info for: %s"%(e))
            return


    def queryServer(self, rids, usr):
        try:
            if rids is None: return self.queryAllServer(usr)
            srvs, i = {}, 0
            for info in (self.db_mgr.mget(rids) or []):
                if info == b'null':srvs[rids[i]] ={}
                else: srvs[rids[i]] = json.loads(info.decode())
                i += 1
            i = 0
            for info in (self.db_umi.mget(['%s#%s'%(usr, rid) for rid in rids])):
                if info:
                    srvs[rids[i]].update(json.loads(self.__decryptUsiInfo(info).decode()))
                i += 1
            return srvs # ignor the order
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server info for: %s"%(e))
            return {}


    def updateServer(self, si, usr):
        try:
            if "deploy" in si:si.pop("deploy")
            rid = si.pop("rid")
            newSi = self.db_mgr.get(rid)
            if newSi == b'null': return 'failed for %s deleted'%rid
            newSi = json.loads(newSi.decode())
            newSi.update({"flag":si["flag"], "location":si.get("location",""), "ma":si.get("ma", "")})
            self.db_mgr.set(rid, json.dumps(newSi))
            info = self.db_umi.get("%s#%s"%(usr, rid))
            if info == b'null' or not info: info = {}
            else:info = json.loads(self.__decryptUsiInfo(info).decode())
            info.update({"usr":si.get("usr", ""), "pwd":si.get("pwd", "")})
            self.db_umi.set("%s#%s"%(usr, rid) , self.__encryptUsiInfo(json.dumps(info).encode()))
            return 'success'
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to update server(%s) info for: %s"%(rid, e))
            return 'failed for %s'%e


    def deleteServers(self, rids):
        try:
            for rid in rids: # delete from server manager
                self.db_mgr.delete(rid)
            # self.db_log.delete(rids) # consider to delete rid log collection
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to delete servers(%s) for: %s"%(','.join(rids), e))
            return 'failed for %s'%e


    def publish(self, prj, module, ver, gids, detail, code, hash, url, usr):
        try:
            indexes, dt = [], datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            for gid in gids: # pass
                if self.db_pub.set('%02x#%s#%s#%s'%(gid, prj, module, ver), json.dumps((hash, code, url, dt, detail, usr))):
                    indexes.append(gid)
                    pmk = '%02x#%s#%s'%(gid, prj, module)
                    for key in self.db_mgr.keys('%02x*'%gid):
                        srv = json.loads(self.db_mgr.get(key).decode())
                        if pmk in srv.get("deploy", {}):
                            self.db_log.set("%s#%s#%s#%s"%(key.decode(), prj, module, ver), '准备更新#%s'%(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")))
            self.p.Produce("publish", json.dumps({"publish":{"indexes":indexes, "project":prj, "module":module, "version":ver, "hash":hash, "code":code, "url":url, "datetime":dt}}).encode())
            return "success"
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to publish %s/%s %s %s for: %s"%(prj, module, ver, ','.join(gids), e))
            return ['failed for %s'%e]


    def __encryptUsiInfo(self, buf):
        try:
            enKey = Fernet.generate_key()
            enBuf = Fernet(enKey).encrypt(buf)
            return struct.pack("<I",len(enKey)) + enKey + enBuf
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "encrypt the buf(%s) failed for %s"%(buf, e))
            return b''


    def __decryptUsiInfo(self, buf):
        try:
            (l, ) = struct.unpack("<I", buf[:4])
            if len(buf) < l + 4:return b'{}'
            return Fernet(buf[4:4+l]).decrypt(buf[4+l:])
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "decrypt the buf(%s) failed for %s"%(buf, e))
            return b'{}'