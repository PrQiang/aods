import logging
import logging.handlers
import time, datetime
import os
import threading
import sys
import traceback


# 定义日志等级，对应 logging 等级数值
LOG_FATAL = logging.FATAL
LOG_ERROR = logging.ERROR
LOG_WARN  = logging.WARN
LOG_INFO  = logging.INFO
LOG_DEBUG = logging.DEBUG

class Logger:
    FMT_DIRT = {
        logging.DEBUG:  "%(asctime)s %(message)s",
        logging.INFO:   "%(asctime)s %(message)s",
        logging.WARN:   "%(asctime)s %(message)s",
        logging.ERROR:  "%(asctime)s %(message)s",
        logging.FATAL:  "%(asctime)s %(message)s"}

    LEVEL_DIRT = {
        logging.DEBUG:  "调试",
        logging.INFO:   "信息",
        logging.WARN:   "警告",
        logging.ERROR:  "错误",
        logging.FATAL:  "崩溃"}

    FILE_LEN = 2 * 1024 * 1024      # 单个日志文件的大小，超过后分割为多个文件
    FILE_CNT = 10                   # 分割文件的数量

    # 模块名字典，名称对应分文件数，{"name":count}
    modules = {}

    def __init__(self, modulename, loglevel):
        # 记录模块名
        if modulename in Logger.modules.keys():
            Logger.modules[modulename] = Logger.modules[modulename] + 1
        else:
            Logger.modules[modulename] = 1

        # 多同名模块时，模块名后面加上123..加以区别
        cnt = Logger.modules[modulename]

        # 创建 logger
        self.logger = logging.getLogger("%s:%s:%s"%(modulename ,datetime.datetime.now().strftime('%Y%m%d'), str(cnt)))
        self.logger.setLevel(loglevel)

        # 日志文件名称：模块名(日期).log
        datename = datetime.datetime.now().strftime('%Y%m%d%H%M%S')        
        self.date = datetime.datetime.now().strftime('%Y%m%d')
        logfilename = 'log/' + modulename + '(' + datename + ').log'

        # 如果当前不存在"log"目录则创建出来
        if not os.path.exists('log'):
            os.makedirs('log')

        # 回滚方式创建写入文件（按大小分割为多文件）
        #rthandler = logging.handlers.RotatingFileHandler(logfilename, mode='a', 
        #    maxBytes=Logger.FILE_LEN, backupCount=Logger.FILE_CNT, encoding=None, delay=0)
        #rthandler.setLevel(loglevel)
        # 创建写入日志文件 handler
        filehd = logging.FileHandler(logfilename)
        filehd.setLevel(loglevel)
        # 创建控制台输出 handler
        console = logging.StreamHandler()
        console.setLevel(loglevel)

        # 定义输出格式
        formatter = logging.Formatter(Logger.FMT_DIRT[loglevel])
        #rthandler.setFormatter(formatter)
        filehd.setFormatter(formatter)
        console.setFormatter(formatter)

        # 给 logger 添加 handler
        #self.logger.addHandler(rthandler)
        self.logger.addHandler(filehd)
        self.logger.addHandler(console)


    def InitModules(*modulenames):
        """初始化各日志模块
        modulenames=(name, level), (name, level)...
        """
        for module, level in modulenames:
            log = Logger(module, level)

        # 设置自动删除旧日志
        Logger.AutoDelLog()


    def GetLogModule(modulename):
        "创建单个日志模块"
        # 不存在则创建，存在但大小超过最大值，也创建新的模块（分文件）
        # 创建日志模块，默认等级为最低级
        try:
            log = None;
            if modulename not in Logger.modules.keys():
                return Logger(modulename, logging.DEBUG).logger
            else:
                cnt = Logger.modules[modulename]
                log = logging.getLogger("%s:%s:%s"%(modulename ,datetime.datetime.now().strftime('%Y%m%d'), str(cnt)))
                if log is None or (len(log.handlers) > 0 and log.handlers[0].stream != None \
                    and log.handlers[0].stream.tell() > Logger.FILE_LEN):
                    return Logger(modulename, logging.DEBUG).logger
            return log

        except Exception as err:
            print(traceback.format_exc())
            print('Logger.GetLogModule 异常: %s'%(err))


    def DelOldLog(dir='log/', day=2):
        """搜索并清理旧日志，默认保留2天日志
        dir(目录), day(相差天数)"""
        # 当前时间
        curdate = datetime.datetime.now()
        # 获得该目录下所有文件
        filelist = os.listdir(dir)
        for file in filelist:
            # 过滤非日志文件
            if len(file) < 20 or file[-4:] != '.log':
                continue
            # 定位日期数据位置
            idx = file.find('(')
            if idx == -1:
                continue
            datestr = file[idx+1:-5]
            # 截取取必须为数字
            if not datestr.isdigit():
                continue
            # 获得该文件建立时间
            filetime = datetime.datetime.strptime(datestr, "%Y%m%d%H%M%S")
            # 清理日志
            if (curdate - filetime).days >= day:
                os.remove(dir + file)


    def AutoDelLog(time=3600):
        """设置定时清理日志，默认1小时检查一次
        time(秒)"""
        timer = threading.Timer(time, Logger.DelOldLog)
        timer.start()


def Log(level, module, msg):
    """各模块统一调用日志方法
    level(日志等级), modeule(模块名), msg(日志消息)
    """
    # 本函数封装了系统logging调用，导致“文件名、调用模块、行号信息”恒显示为本函数信息
    # 所以需要重新获取这三个数据（ %(filename)s %(module)s %(lineno)d ）
    try:
        raise Exception
    except:
        f = sys.exc_info()[2].tb_frame.f_back

    filename = os.path.basename(f.f_code.co_filename)
    callfuncname = f.f_code.co_name
    levelname = Logger.LEVEL_DIRT[level]
    try:
        errStr = traceback.format_exc()
        if errStr is None or len(errStr) < 1 or errStr == 'NoneType: None\n':
            msg = '[%s line:%d][%s][%s] %s' % (filename, f.f_lineno, callfuncname, levelname, msg)
        else:
            msg = '[%s line:%d][%s][%s] %s\r\n%s' % (filename, f.f_lineno, callfuncname, levelname, msg, errStr)
    except Exception as e: # 屏蔽python 3版本之间差异造成启动异常bug
        msg = '[%s line:%d][%s][%s] %s' % (filename, f.f_lineno, callfuncname, levelname, msg)

    # 获取日志模块
    lgr = Logger.GetLogModule(module)
    if lgr == None:
        return
    # 统一调用方式
    if level == logging.DEBUG:
        lgr.debug(msg)
    elif level == logging.INFO:
        lgr.info(msg)
    elif level == logging.WARN:
        lgr.warn(msg)
    elif level == logging.ERROR:
        lgr.error(msg)
    elif level == logging.FATAL:
        lgr.fatal(msg)
    else:
        lgr.debug(msg)

    return lgr


def TestLog():
    Log(LOG_ERROR, "Log", "this is a test")


if __name__ == '__main__':
    Logger.InitModules()
    while 1:
        TestLog()
        time.sleep(10.0)