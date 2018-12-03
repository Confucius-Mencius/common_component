#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"

int main()
{
    // 定义Logger
    log4cplus::Logger test_logger = log4cplus::Logger::getInstance("LoggerName");

    // 定义一个控制台的Appender
    log4cplus::SharedAppenderPtr console_appender(new log4cplus::ConsoleAppender());

    // 将需要关联Logger的Appender添加到Logger上
    test_logger.addAppender(console_appender);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG4CPLUS_ERROR(test_logger, "failed to create socket: " << strerror(errno));
        return -1;
    }

    struct sockaddr_in remote_addr;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(12345);

    if (connect(fd, (struct sockaddr*) &remote_addr, sizeof(struct sockaddr)) != 0)
    {
        LOG4CPLUS_ERROR(test_logger, "failed to connect to remote server: " << strerror(errno));
        close(fd);
        return -1;
    }

    std::string str = "I am Michael";

    while (true)
    {
        send(fd, str.c_str(), str.size(), 0);
        sleep(5);
    }

    return 0;
}
