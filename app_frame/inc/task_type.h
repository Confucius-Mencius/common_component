#ifndef APP_FRAME_INC_TASK_TYPE_H_
#define APP_FRAME_INC_TASK_TYPE_H_

enum
{
    TASK_TYPE_MIN = 0,
    TASK_TYPE_NORMAL = TASK_TYPE_MIN,

    TASK_TYPE_TCP_CONN_CONNECTED, // listen thread通知tcp thread有新的连接建立了
    TASK_TYPE_TCP_CONN_CLOSED, // 通知listen thread减少连接数
    TASK_TYPE_TCP_CONN_CLOSED_NET_STORM, // 通知listen thread减少连接数，原因是网络风暴

    TASK_TYPE_SEND_TO_CLIENT, // io thread发送数据给客户端
    TASK_TYPE_CLOSE_CONN, // io thread关闭客户端连接

    TASK_TYPE_MAX,
};

#endif // APP_FRAME_INC_TASK_TYPE_H_
