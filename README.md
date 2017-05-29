# FTP-Nick-Peter

We have created two programs -  FTPclient.c and FTPserver.c. With these files, we have simulated a basic FTP client/server model, using a TCP connection. 

##Commands

USER - username with this command the client gets to identify which user is trying to login
to the FTP server. This is mainly used for authentication, since there might be different
access controls specified for each user.

PASS - The client needs to authenticate
with the user password by issuing a PASS command followed by the user password.

PUT - this command is used to upload a file named filename from the current
client directory to the current server directory. 

GET - this command is used to download a file named filename from the current
server directory to the current client directory. 

LS - this command is used to list all the files under the current server directory

!LS - this command is used to list all the files under the current client directory

CD - this command is used to change the current server directory

!CD - this command is used to change the current client directory

PWD - this command displays the current server directory

!PWD - this command displays the current client directory

QUIT - this command quits the FTP session and closes the control TCP connection

##More Information About Our Programs
 
To run the FTPclient use: ./FTPclient ftp-server-ip-address ftp-server-port-number

The FTPserver uses select() to accept multiple clients and server also checks to validate the USER and PASS to run commands for each individual client. 

##Issues
We are still working on refining how the commands work.

Some specific issues include: how to send the port number and hostname to the server; the PUT command somehow gets blocked by the server when creating a new socket with a new port number, it only works when the port number is the same as the initial connection.
