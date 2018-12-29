## app_launcher
统一的app框架启动器，通过dlopen的方式加载app_frame，启动进程。监听reload和stop信号，并通知app_frame。

只要实现了app_frame的接口，都可以通过app_launcher来启动。
