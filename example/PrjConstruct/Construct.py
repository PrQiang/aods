import redis, json

def constructUser( usr, pwd):
    import hashlib
    sha = hashlib.sha256()
    sha.update(pwd.encode())
    db_ui = redis.Redis(host="192.168.221.134", port = int(65379), db = 5, password='pd3@a^,.)992') # user info
    db_ui.set(usr, json.dumps({"pwd":sha.hexdigest(), "isAdmin":1}))
    print(db_ui.get(usr))


if __name__ == "__main__":
    # 创建用户名
    constructUser('master', 'master@er.com')