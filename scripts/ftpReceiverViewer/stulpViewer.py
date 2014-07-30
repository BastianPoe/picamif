from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer
import subprocess
import os

#SCRIPT="START /MIN ftpDisplay.cmd"
#SCRIPT="START /B CMD /C CALL \"%ProgramFiles(x86)%\IrfanView\i_view32.exe\" \"%1%\" /one /fs"
SCRIPT="./ftpDisplay.sh"

if not os.path.exists("incoming"):
    os.makedirs("incoming")

def onUpload(self, file):
    print("Uploaded "+str(file))
    cmd=SCRIPT+" "+file
    #cmd=[]
    #cmd.append("START")
    #cmd.append("C:\Program Files (x86)\IrfanView\i_view32.exe")
    
    #cmd.append(file)
    #cmd.append("/one")
    #cmd.append("/fs")
    #cmd="%ProgramFiles(x86)%\IrfanView\i_view32.exe\" \"" +file+ "\" /one /fs"
    print("Execute "+str(cmd))
    #res=os.popen(cmd)
    res=subprocess.call(cmd, shell=True)
    print ("Result "+str(res))
    
def main():
    # Instantiate a dummy authorizer for managing 'virtual' users
    authorizer = DummyAuthorizer()

    # Define a new user having full r/w permissions and a read-only
    # anonymous user
    authorizer.add_user('user', '12345', 'incoming', perm='elradfmwM')
    authorizer.add_anonymous(os.getcwd())

    # Instantiate FTP handler class
    handler = FTPHandler
    handler.authorizer = authorizer

    handler.on_file_received=onUpload

    # Define a customized banner (string returned when client connects)
    handler.banner = "pyftpdlib based ftpd ready."

    # Specify a masquerade address and the range of ports to use for
    # passive connections.  Decomment in case you're behind a NAT.
    #handler.masquerade_address = '151.25.42.11'
    #handler.passive_ports = range(60000, 65535)

    # Instantiate FTP server class and listen on 0.0.0.0:2121
    address = ('0.0.0.0', 2121)
    server = FTPServer(address, handler)

    # set a limit for connections
    server.max_cons = 256
    server.max_cons_per_ip = 5

    # start ftp server
    server.serve_forever()

if __name__ == '__main__':
    main()
