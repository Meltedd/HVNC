# HVNC - Tinynuke (Fixed)
This HVNC (Hidden Virtual Network Computing) Client and Server is based off of Tinynuke's HVNC (C++).

Credits to Tinynuke: https://github.com/rossja/TinyNuke

# Features:
- Hidden Desktop (resize in accurate proportions for the best results)
- Open "Run"
- Start Chrome
- Start Firefox
- Start Internet Explorer

# Usage:
- In the Client's "Main.cpp" file, edit the ip and port variables.
- In the Server's "Server.cpp" file, find "BOOL StartServer(int port)", and replace the port within "addr.sin_port = htons(4043);" with the port you want the Server to listen on.
- Compile the Server & Client, and run the Server. Now, when the Client is executed, it will open a new "Hidden Desktop" window. If you right-click on the white bar at the top of the "Hidden Desktop" window, you can view the available commands that you can run on the target machine.

# Demo of HVNC Window:
![Image1](https://i.ibb.co/JxMn3j4/image.png)

# Contact Me:
Discord: Melted#0001
Telegram: t.me/Melteddd
XMPP: melted@xmpp.jp
