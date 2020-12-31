import json, redis, random, datetime
from Logger import *
from KafkaConsumer import KafkaConsumer
from KafkaProducer import KafkaProducer

class DeployCtrl:
    def __init__(self, brokers, ra, gid, index):
        self.brokers, self.ra, self.gid, self.index = brokers, ra, gid, bytes([int(index)%255]).hex()


    def Run(self):
        try:
            self.InitDb()
            self.producer = KafkaProducer(self.brokers)
            self.consumer = KafkaConsumer(self.brokers, ["deploy_u", "publish"], self, self.gid)
            self.consumer.join()
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "Run Failed: %s"%e)


    def InitDb(self):
        try:
            (addr, port) = self.ra.split(":")
            self.db_mgr = redis.Redis(host=addr, port = int(port), db = 1, password='pd3@a^,.)992') # 服务器管理集合
            self.srvs = dict([(key.decode(), json.loads(self.db_mgr.get(key))) for key in self.db_mgr.keys("%s*"%(self.index))])
            self.db_pub = redis.Redis(host=addr, port = int(port), db = 2, password='pd3@a^,.)992') # 发布管理集合
            self.publish = dict([(key.decode(), json.loads(self.db_pub.get(key))) for key in self.db_pub.keys("%s*"%(self.index))])
            self.db_log = redis.Redis(host=addr, port = int(port), db = 3, password='pd3@a^,.)992') # 发布日志集合
            self.db_cli = redis.Redis(host=addr, port = int(port), db = 4, password='pd3@a^,.)992') # 客户端版本信息
        except Exception as e:
            Log(LOG_ERROR,"DeployCtrl", "InitDb Failed: %s"%e)


    def OnConsume(self, topic, value):
        try:
            for k, v in json.loads(value.decode("utf8")).items():
                # topic deploy_u
                if k == "active": self.__dealActive(v)# 激活
                elif k == 'active_suc': self.__dealActiveSuc(v)# 激活成功
                elif k =='auth': self.__dealAuthen(v)# 认证
                elif k == 'check_update': self.__dealUpdateCheck(v) # 检测更新
                elif k == 'update_result': self.__dealUpdateResult(v) # 更新结果
                # topic publish
                elif k == "publish": self.__dealPublish(v)# 发布
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "OnConsume(%s, %s) failed: %s"%(topic, value, e))


    def __dealActive(self, value):
        try:            
            if self.index != bytes([value.get("index", 0)]).hex(): return
            rid = self.__generalRid() # 生成唯一标识,并保存至内存中
            self.srvs[rid] = {}
            self.producer.Produce("deploy_d", json.dumps({"active_ack":{"uuid":value.get("uuid",""), "rid":rid, "ack":0}}).encode("utf8"))
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "__dealActive(%s) failed: %s"%(value, e))


    def __generalRid(self):
        while 1:
            rid = self.index + bytes([random.randint(1, 255) for i in range(5)]).hex()
            if rid not in self.srvs:
                return rid


    def __dealActiveSuc(self, value):
        try:
            rid, active_code = value.get("rid"), value.get("active_code")
            if rid is None or active_code is None or rid not in self.srvs: return
            value = self.srvs[rid] = {"active_code" : active_code, "deploy" : {}}
            self.db_mgr.set(rid, json.dumps(value))
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "__dealActiveSuc(%s) failed: %s"%(value, e))


    def __dealAuthen(self, value):
        try:
            rid, active_code = value.get("rid"), value.get("active_code")
            if type(rid) is not str or rid[:2] != self.index: return# 不是该服务端管理服务
            if rid not in self.srvs or active_code != self.srvs[rid]["active_code"]: # 认证非法
                self.producer.Produce("deploy_d", json.dumps({"auth_ack":{"rid":rid, "ack":1}}).encode("utf8"))
            else:self.producer.Produce("deploy_d", json.dumps({"auth_ack":{"rid":rid, "ack":0}}).encode("utf8"))
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "__dealAuthen(%s) failed: %s"%(value, e))


    def __dealUpdateCheck(self, value):
        try:
            if self.srvs.get(value.get("rid")) is None: return
            print("__dealUpdateCheck(%s)"%(value))
            pmKey = self.__prjModule2Key(value.get("project"), value.get("module"))
            (ver, hash, url, code) = self.__getNewestVer(pmKey)
            if ver is None:return
            if ver != value.get("version") or hash != value.get("hash"):
                self.producer.Produce("deploy_d",json.dumps({"update":{"rid":value.get("rid"),"project":value.get("project"),"module":value.get("module"),"version":ver,"hash":hash,"url":url,"code":code,"force":1}}).encode("utf8"))
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "__dealUpdateCheck(%s) failed: %s"%(value, e))


    def __dealUpdateResult(self, value):
        try:
            if self.srvs.get(value.get("rid")) is None: return
            print("__dealUpdateResult(%s)"%(value))
            prj, module, version, hash, result = value.get("project"), value.get("module"), value.get("version"), value.get("hash"), value.get("result")
            result = {0:"更新成功", 1:"下载失败", 2:"保存失败", 3:"校验失败", 4:"解压失败", 5:"执行失败"}.get(result, "未知错误")
            self.db_log.set(self.__ridPrjModuleKeyVersion(value.get("rid"), prj, module, version), "%s#%s"%(result, datetime.datetime.now()))
            if result == "更新成功": pass # 记录设备当前版本号
        except Exception as e:
            Log(LOG_ERROR, "DeployCtrl", "__dealUpdateResult(%s) failed: %s"%(value, e))


    def __dealPublish(self, value):
        try:
            if self.index not in [bytes([index%255]).hex() for index in value.get('indexes', [])]:return
            prj, module, version, hash, code, url = value.get("project"), value.get("module"), value.get("version"), value.get("hash"), value.get("code"), value.get("url")
            for rid in self.srvs.keys():
                self.producer.Produce("deploy_d", json.dumps({"update":{"rid":rid, "project":prj,"module":module,"version":version,"hash":hash,"url":url, "code":code,"force":1}}).encode("utf8"))
            pmKey = self.__prjModule2Key(prj, module)
            if pmKey not in self.publish: self.publish[pmKey] = {version:(hash, code, url, "%s"%datetime.datetime.now())}
            else: self.publish[pmKey][version] = (hash, code, url, "%s"%datetime.datetime.now())
            self.db_pub.set(pmKey, json.dumps(self.publish[pmKey]))
        except Exception as e:
            Log(LOG_ERROR, "__dealPublish(%s) failed: %s"%(value, e))


    def __prjModule2Key(self, prj, module):
        return '%s#%s#%s'%(self.index, prj, module)


    def __ridPrjModuleKeyVersion(self, rid, prj, module, version):
        return "%s#%s#%s#%s"%(rid,prj,module,version)


    def __getNewestVer(self, pmKey):
        info = self.publish.get(pmKey)
        if not info: return (None, None, None, None)
        rv, rh, ru, rc, rt = None, None, None, None, None
        for ver, (hash, code, url, pubTime) in info.items():
            if rv is None or datetime.datetime.fromisoformat(rt) < datetime.datetime.fromisoformat(pubTime): 
                rv, rh, ru, rc, rt = ver, hash, url, code, pubTime
        return rv, rh, ru, rc


if __name__ == "__main__":
    DeployCtrl("192.168.66.124:9092", "192.168.66.124:65379", "deploy-1", 1).Run()