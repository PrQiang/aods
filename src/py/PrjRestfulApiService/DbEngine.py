import threading, redis, json, random, datetime
from Logger import *

class DbEngine(object):
    _lock = threading.Lock()

    def __init__(self):
        addr, port, pwd = "192.168.66.118", 65379, 'pd3@a^,.)992'
        self.db_mgr = redis.Redis(host=addr, port = port, db = 1, password=pwd) # server manager
        self.db_pub = redis.Redis(host=addr, port = int(port), db = 2, password=pwd) # publish collection
        self.db_log = redis.Redis(host=addr, port = int(port), db = 3, password=pwd) # publish log
        self.db_cli = redis.Redis(host=addr, port = port, db = 5, password=pwd) # user info
        self.db_cookie = redis.Redis(host=addr, port = port, db = 6, password=pwd) # user cookie
        self.db_usi = redis.Redis(host=addr, port = port, db = 7, password=pwd) # user mange server info
        self.ttl = 3600


    @classmethod
    def Instance(cls):
        if not hasattr(DbEngine, "_instance"):
            with DbEngine._lock:
                if not hasattr(DbEngine, "_instance"):
                    DbEngine._instance = DbEngine()
        return DbEngine._instance


    def queryUserInfo(self, usr):
        try:
            return self.db_cli.get(usr)
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
                index, prj, module = key.decode().split('#')
                if prj not in prjs: prjs[prj] = [module]
                elif module not in prjs[prj]:prjs[prj].append(module)
            return prjs
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server base info for: %s"%(e))
            return {}


    def queryModuleInfo(self, prj, module):
        try:            
            vis = {} # collect data
            for key in self.db_pub.keys("*#%s#%s"%(prj, module)):
                gid = key.decode().split('#')[0]
                for v, (hash, code, url, dtDetail) in json.loads(self.db_pub.get(key)).items(): 
                    lst = dtDetail.split("#")
                    dt, detail = lst[0], '#'.join(lst[1:])
                    if v not in vis: vis[v] = (dt, gid, hash, url, detail)
                    else:
                        (dt2, gid2, hash, url, detail) = vis[v]
                        if dt2 < dt: dt2 = dt
                        gid2 += ', %s'%(gid)
                        vis[v] = (dt2, gid2, hash, url, detail)
            # sort by the date
            return sorted(vis.items(), key=lambda item: item[1][0])
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to query server base info for: %s"%(e))
            return []


    def queryUpdateDetail(self, prj, module, ver):
        try:
            pubResult = []
            for key in self.db_pub.keys("*#%s#%s"%(prj, module)):
                gid = key.decode().split('#')[0]
                for v, (hash, code, url, dtDetail) in json.loads(self.db_pub.get(key)).items(): 
                    if v != ver: continue
                    lst = dtDetail.split("#")
                    dt, detail = lst[0], '#'.join(lst[1:])
                    pubResult.append((gid, dt, detail))
            result = []
            for key in self.db_log.keys("*#%s#%s#%s"%(prj, module, ver)):
                rid = key.decode().split('#')[0]
                rlt, dt = self.db_log.get(key).decode().split('#')
                si = self.db_mgr.get(rid)
                if si == b'null': continue
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
            for key in self.db_usi.keys("%s#*"%(usr)):
                rid = key.decode().split('#')[-1]
                if rid not in srvs: continue
                srvs[rid].update(json.loads(self.db_usi.get(key).decode()))
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
            for info in (self.db_usi.mget(['%s#%s'%(usr, rid) for rid in rids])):
                if info: srvs[rids[i]].update(json.loads(info.decode()))
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
            info = self.db_usi.get("%s#%s"%(usr, rid))
            if info == b'null' or not info: info = {}
            else:info = json.loads(info.decode())
            info.update({"usr":si.get("usr", ""), "pwd":si.get("pwd", "")})
            self.db_usi.set("%s#%s"%(usr, rid) , json.dumps(info))
            return 'success'
        except Exception as e:
            Log(LOG_ERROR, "DbEngine", "failed to update server(%s) info for: %s"%(rid, e))
            return 'failed for %s'%e        


def constructUser( usr, pwd):
    import hashlib
    sha = hashlib.sha256()
    sha.update(pwd.encode())
    db_cli = redis.Redis(host="192.168.66.118", port = int(65379), db = 5, password='pd3@a^,.)992') # user info
    db_cli.set(usr, json.dumps({"pwd":sha.hexdigest(), "isAdmin":1}))
    print(db_cli.get(usr)) 


def constructPublish():
    addr, port, pwd = "192.168.66.118", 65379, 'pd3@a^,.)992'
    db_pub = redis.Redis(host=addr, port = int(port), db = 2, password=pwd) # publish collection
    for gid in ('01', '02', '03'):
        db_pub.set('%s#aods-test#aods-test-win-x64'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103")}))

        db_pub.set('%s#aods-test#aods-test-win-x86'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103")}))

        db_pub.set('%s#aods-test#aods-test-linux-x64'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103"), "0001.0001.0131":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103")}))
        db_pub.set('%s#aods-test#test-linux-x64'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103"), "0001.0001.0131":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103")}))
        db_pub.set('%s#aods-test#test-win-x64'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103"), "0001.0001.0131":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0132":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0133":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0135":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0136":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0138":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0140":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103")}))
        db_pub.set('%s#aods-test#test-win-x86'%gid, json.dumps({"0001.0001.0120":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced.dat", "2020-12-11 10:00:01#fixed bug 001\r\nfixed bug 002\r\nadd a new function 003"), "0001.0001.0121":("1bcd22342234111", "$dafa32", "http://192.168.66.124/abced1.dat", "2020-12-11 10:00:02#fixed bug 002\r\nfixed bug 003\r\nadd a new function 004"), "0001.0001.0130":("1bcd22342234112", "$dafa33", "http://192.168.66.124/abced123.dat", "2020-12-11 10:00:11#fixed bug 011\r\nfixed bug 012\r\nadd a new function 103"), "0001.0001.0131":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0132":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0133":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0135":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0136":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0138":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103"), "0001.0001.0140":("1bcd22342234113", "$dafa34", "http://192.168.66.124/abced1234.dat", "2020-12-11 10:00:41#fixed bug 041\r\nfixed bug 042\r\nadd a new function 103")}))


def constructServer():
    addr, port, pwd = "192.168.66.118", 65379, 'pd3@a^,.)992'
    db_mgr = redis.Redis(host=addr, port = int(port), db = 1, password=pwd)
    for gid in ('01', '02', '03'):
        for i in range(100):
            db_mgr.set('%sabcdef00%02x'%(gid, i), json.dumps({"ip":"%s.1.1.%s"%(gid, i), 'flag':"%s.1.1.%s"%(gid, i), "active_code" : "3123123123abfad23f3", "deploy" : {}}))


def constructPublishLog():
    addr, port, pwd = "192.168.66.118", 65379, 'pd3@a^,.)992'
    db_log = redis.Redis(host=addr, port = int(port), db = 3, password=pwd) # publish log

    for module in ["aods-test-win-x64", "aods-test-win-x86", "aods-test-linux-x64", "test-linux-x64", "test-win-x64", "test-win-x86"]:
        for gid in ('01', '02', '03'):
            for i in range(100):
                rlt = ['更新成功', '下载失败', "保存失败", "校验失败", "解压失败", "执行失败", "未知错误"][random.Random().randint(0, 6)]                
                db_log.set("%s#%s#%s#%s"%('%sabcdef00%02x'%(gid, i), "aods-test", module, "0001.0001.0120"), "%s#%s"%(rlt, datetime.datetime.now()))


def clearRubbishData():
    addr, port, pwd = "192.168.66.118", 65379, 'pd3@a^,.)992'
    db_mgr = redis.Redis(host=addr, port = int(port), db = 1, password=pwd)    
    for key in db_mgr.keys("*"):
        if db_mgr.get(key) == b'null':db_mgr.delete(key)
    db_usi = redis.Redis(host=addr, port = port, db = 7, password=pwd)
    for key in db_usi.keys("*"):
        data = db_usi.get(key)
        if not data or data==b'null':db_usi.delete(key)



if __name__ == "__main__":
    clearRubbishData()
    # construct a usr
    #constructUser('PrQiang', "`12K<34bac*1)")

    # construct publish info
    #constructPublish()

    # construct server
    #constructServer()

    # constuct publish log
    #constructPublishLog()
    
    # test update detail
    #print(DbEngine.Instance().queryModuleInfo("aods-test", "test-win-x64"))

    # test print all     
    #rlt = DbEngine.Instance().queryAllServer('raoqiang')
    #print(json.dumps(rlt, indent=2))
    pass