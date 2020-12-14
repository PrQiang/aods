import threading

class SafeMon:
    _lock = threading.Lock()

    def __init__(self):
        pass


    @classmethod
    def Instance(cls):
        if not hasattr(SafeMon, "_instance"):
            with SafeMon._lock:
                if not hasattr(SafeMon, "_instance"):
                    SafeMon._instance = SafeMon()
        return SafeMon._instance


    def isHack(self, cliAddr, msgCode, msgValue):
        return False