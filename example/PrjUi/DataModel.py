import threading

class DataModel(object):
    _lock = threading.Lock()

    def __init__(self):
        self.usr = {}


    @classmethod
    def Instance(cls):
        if not hasattr(DataModel, "_instance"):
            with DataModel._lock:
                if not hasattr(DataModel, "_instance"):
                    DataModel._instance = DataModel()
        return DataModel._instance


    def UpdateUser(self, ui):
        self.usr.update(ui)


    def GetCookie(self):
        return self.usr.get("cookie")
