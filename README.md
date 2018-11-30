# common_component
common components used in Linux server programming, including framework components

verified in CentOS7/LinuxMint18/Ubuntu16.04 and above

公共组件，包括基础库、配置文件读取、日志、网络通信、异步管理、定时器、线程管理、脚本引擎、随机数引擎、时间服务、app launcher、app frame等，每个都是一个独立的工程。

## app_frame
app框架，根据配置启动tcp、udp、http、work、burden线程组。

## app_launcher
统一的app框架启动器，通过dlopen的方式加载app_frame，启动进程。监听reload和stop信号，并通知app_frame。只要实现了app_frame的接口，都可以通过app_launcher来启动。

## base
基础库，一些常用的函数和类。通过-l链接的方式使用。

## build
编译脚本。可以编译debug和release两种版本。编译完成会安装到/opt目录下，也可以mount使用。

build.sh -h
                 -h show this help info.
                 -b debug|release build projects.
                 -r debug|release rebuild projects.
                 -c debug|release clear tmp files.

## burden_threads
burden线程组，是第三级线程组。app_frame通过dlopen的方式加载burden_threads。burden_theads对thread_center进行了包装，是一种类型的thread_center，其他线程组也是这样。可以调度任务给所有的线程组，包括burden_threads。

## client_center_mgr
client指的是连接其他服务的一方。提供了通过tcp、udp、http/https异步请求其他服务的接口。thread_center通过dlopen的方式加载client_center_mgr。

## conf_center
配置文件读取。提供配置访问接口。

## conn_center_mgr
conn指的是作为服务器接收客户端的连接。tcp_threads使用其中的tcp_conn_center,udp_threads使用其中的udp_conn_center,http_threads使用其中的http_conn_center。分别管理各自类型的连接和io。

## doc
文档目录。

## event_center
事件订阅和发布。降低模块之间的耦合。

## global_thread
全局线程，其中执行一些全局唯一的逻辑。可以调度任务给io(tcp、udp、http)线程组、work_threads、burden_threads。广播或者制定线程均可。

## http_threads
http线程组，监听http连接并处理io，是第一级线程组，也是一种io线程组。可以调度任务给work_threads、global_threads。

## log_engine
全局日志宏。是对log4cplus的封装，支持日志配置（指定日志文件个数，每个文件大小限制，日志格式等）。也可以将日志发往socket。

## msg_codec_center
消息编解码器。实现了自定义tcp协议的编解码。

## random_engine
随机数引擎。

## script_engine
脚本引擎。实现了对lua脚本的调用。

## tcp_threads
tcp线程组，监听tcp连接并处理io，是第一级线程组，也是一种io线程组。可以调度任务给work_threads、global_threads。

## third_party
公共cmake文件，定义的是第三方库的inc、link路径和lib名。

## thread_center
底层的线程组封装。

## timer_axis
定时器封装。

## time_service
时间服务接口。

## trans_center
通用的异步事务接口。一个线程使用一个trans_center管理异步事务。

## udp_threads
udp线程组，监听udp请求并处理io，是第一级线程组，也是一种io线程组。可以调度任务给work_threads、global_threads。

## work_threads
work线程组，是第二级线程组。可以调度任务给所有的线程组，包括work_threads。

线程总数=main thread(1) + global thread(1) + listen thread(1) + io threads count + work threads count + burden threads count + http threads count + udp threads count
