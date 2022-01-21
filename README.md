# Information-transmission-between-client-and-client-realized-by-socket-programming
Information transmission between client and client realized by socket programming(Computer network)  
1.功能要求  
实现TCP客户端之间通信，实现能够在局域网中两台主机之间进行socket通信。  
原理简述：不同于本机内部传输，实验二要求在两台主机之间进行相互传输。一台电脑作为服务器端，另一台电脑作为客户端，服务器端开启自己电脑的一个端口，监听其它电脑对这一端口的请求，一旦发现有接入，则建立连接。  
  
2.简要步骤：
1. 客户端：创建一个socket； 设置socket属性；绑定IP地址、端口等信息到socket上；设置要连接的对方的IP地址和端口等属性；连接服务器，用函数connect()；收发数据，用函数send()和recv()，或者read()和write()；关闭网络连接；  
2. 服务器端：1、创建一个socket；设置socket属性；绑定IP地址、端口等信息到socket上；开启监听，用函数listen()；接收客户端上来的连接，用函数accept()；收发数据，用函数send()和recv()，或者read()和write(); 关闭网络连接；关闭监听；  
3. 实现互相对话：在服务器上显示出界面，客户端都同服务器端进行通信，服务器端每当接收到一个客户端的请求就建立一个socket，并存入列表中，最终形成一个简易聊天室。  
注：此任务要求使用局域网上两台不同主机（不使用回环地址）。  
