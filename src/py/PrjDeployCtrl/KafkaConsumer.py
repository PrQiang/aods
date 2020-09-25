import kafka, threading, time, traceback
from Logger import *

# kafka消费者类
class KafkaConsumer(threading.Thread):
    def __init__(self, brokers, topics, ob, gid, autoStart = True, desc = None):
        """
        broker: 分发者地址
        """
        for i in range(3):
            try:
                self.consumer =  kafka.KafkaConsumer(bootstrap_servers = brokers, group_id = gid, auto_offset_reset = 'latest')
                break
            except Exception as e:
                pass
        else:
            self.consumer =  kafka.KafkaConsumer(bootstrap_servers = brokers, group_id = gid, auto_offset_reset = 'latest')
        self.ob, self.desc, self.running =ob, desc or topics, True
        self.consumer.subscribe(topics)
        super().__init__(daemon = True)
        if autoStart:
            self.start()


    def run(self):
        try:
            preTime, totalTime, maxDif, maxSpanValue, totalNum = time.time() * 1000, 0, 0, None, 0
            for m in self.consumer:
                try:
                    if self.running is False:
                        break
                    t1 = time.time() * 1000
                    self.ob.OnConsume(m.topic, m.value)
                    t2 = time.time() * 1000

                    # 计算有效打印消息
                    tDif = t2 - t1
                    if maxDif < tDif:
                        maxDif, maxSpanValue = tDif, m.value
                    totalTime += tDif
                    totalNum += 1
                    if preTime + 60000 <= t2:
                        if (t2 - preTime) * 0.8 < totalTime: # 当且仅当消费线程CPU使用率(非标准CPU使用率)超过80%才打印
                            Log(LOG_INFO, "KafkaConsumer", "%s在%.2f分钟内处理%d条消息, 总共用时: %.2f秒,平均耗时: %.2fms, 最大耗时%dms消息: \n%s"%(self.desc, (t2 - preTime)/60000.0, totalNum, totalTime/1000.0, totalTime / totalNum, maxDif, maxSpanValue))
                        preTime, totalTime, maxDif, maxSpanValue, totalNum = time.time() * 1000, 0, 0, None, 0
                except Exception as e:
                    traceback.print_exc()
                    Log(LOG_ERROR, 'KafkaConsumer', '消息(%s)执行异常,异常描述为:%s'%(m.value, e))

        except Exception as err:
            traceback.print_exc()
            Log(LOG_ERROR, "KafkaConsumer", "kafka消费线程异常: %s"%err)


    def Stop(self):
        self.running = False
