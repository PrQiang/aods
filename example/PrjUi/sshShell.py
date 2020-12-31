import traceback, os, stat, time
from paramiko.client import SSHClient, AutoAddPolicy
from paramiko import AuthenticationException
from paramiko.ssh_exception import NoValidConnectionsError

class sshShell:
    def __init__(self, name, addr, usr, pwd, pk = None, kf = None, encoding= 'utf-8'):
        self.name, self.usr, self.pwd, self.pk, self.kf, self.encoding = name, usr, pwd, pk, kf, encoding
        self.host, port = addr.split(":")
        self.port = int(port)
        self.cacheStat = {}

    def Name(self):
        return self.name


    def __createShell(self):
        self.ssh = SSHClient()
        self.ssh.set_missing_host_key_policy(AutoAddPolicy())
        self.ssh.connect(self.host, port=self.port, username=self.usr, password=self.pwd, pkey = self.pk, key_filename=self.kf, banner_timeout = 3.0, auth_timeout=3.0)
        self.shell = self.ssh.invoke_shell()
        self.shell.settimeout(2.0)


    def __recv(self):
        try:
            return (self.shell.recv(999) or b'').decode(self.encoding)
        except Exception as e:
            return ''

    def __send(self, cmd):
        self.shell.send(cmd + '\r')


    def __close(self):
        try:
            self.shell.shutdown(2)
        except Exception as e:
            pass

        try:
            self.ssh.close()
        except Exception as e:
            pass


    def Excute(self, cmd):
        try:
            self.__createShell()
            print("shell hallo words: %s"%(self.__recv()))
            self.__send(cmd)
            result = ''
            while 1:
                rsp = self.__recv()
                if len(rsp) < 1:break
                result += rsp
            self.__close()
            return result
        except Exception as e:
            return traceback.format_exc()


    def Upload(self, files, dsDir):
        try:
            self.ssh = SSHClient()
            self.ssh.set_missing_host_key_policy(AutoAddPolicy())
            self.ssh.connect(self.host, port=self.port, username=self.usr, password=self.pwd, pkey = self.pk, key_filename=self.kf, banner_timeout = 3.0, auth_timeout=3.0)
            self.stftp = self.ssh.open_sftp()
            result = []
            for f in files:
                if os.path.isfile(f):
                    dir, fn = os.path.split(f)
                    result.append((f,self.UploadFile(self.stftp, f, os.path.join(dsDir, fn))))
                elif os.path.isdir(f):
                    for root, dirs, fns in os.walk(f):
                        for fn in fns:
                            result.append((os.path.join(root, fn), self.UploadFile(self.stftp, os.path.join(root, fn), os.path.join(dsDir, root.replace(f, ''), fn))))
            self.__close()
            return True, result
        except Exception as e:
            self.__close()
            return False, traceback.format_exc()


    def UploadFile(self, sftp, local, remote):
        t1 = time.time() * 1000
        try:
            # check the dir is exists
            remote = remote.replace("\\", "/")
            dir, fn = os.path.split(remote)
            self.check2CreateDir(sftp, dir)
            sftp.put(local, remote)
            return 'ok', time.time() * 1000 - t1
        except Exception as e:
            return traceback.format_exc(), time.time() * 1000 - t1


    def check2CreateDir(self, sftp, dir):
        try:
            dirSections = dir.split('/')
            if len(dir) > 0 and dir[0] =='/':dirSections[0] = '/'
            for i in range(len(dirSections)):
                pdir = '/'.join(dirSections[:i+1])
                if stat.S_ISDIR(self.cacheStat.get(pdir, 0)):continue
                try:
                    a = sftp.stat(pdir)
                    self.cacheStat[pdir] = a.st_mode
                    if not stat.S_ISDIR(a.st_mode): return False
                except IOError as e:
                    if e.errno == 2:
                        sftp.mkdir(pdir)
                    else:
                        traceback.print_exc()
                        return False                     
        except Exception as e:
            traceback.print_exc()
            return False