## build
编译脚本。可以编译debug和release两种版本。编译完成会安装到/opt/common_component目录下，可以其它开发机上mount使用。

build.sh -h
=>

usage: /home/hgc/Confucius-Mencius/common_component/build/build.sh
                 -h show this help info.
                 -b debug|release build projects.
                 -r debug|release rebuild projects.
                 -c debug|release clear tmp files.
