import kafka
from Logger import *

# kafka生产者类
class KafkaProducer:
    def __init__(self, brokers):
        """
        broker: 分发者地址
        """
        self.producer, self.brokers = kafka.KafkaProducer(bootstrap_servers = brokers), brokers


    def Produce(self, topic, value):
        """ 生产消息
        topic:  主题
        value:  值, 二进制值, 如：b'12'
        """
        try:
            if isinstance(value, str):
                print('Produce invalid data: %s:%s'%(topic, value))
                value = value.encode('utf8')
            elif isinstance(value, bytes) is not True:
                print('Produce invalid data: %s:%s'%(topic, value))
                return
            err = None
            for i in range(3):
                try:
                   self.producer.send(topic, value)
                   return True
                except Exception as e:
                    err = e
                    self.producer = kafka.KafkaProducer(bootstrap_servers = self.brokers)
            Log(LOG_ERROR, "KafkaProducer", '向kafka(%s)发送消息%s异常: %s'%(topic, value, err))
            return False
        except Exception as err:
            Log(LOG_ERROR, "KafkaProducer", '向kafka(%s)发送消息%s异常: %s'%(topic, value, err))
            return False