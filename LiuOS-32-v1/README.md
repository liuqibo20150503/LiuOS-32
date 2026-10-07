# LiuOS-32 Minimal

12文件最小可用内核，12个真命令。

## 命令

HELP      显示帮助
CLS       清屏
VER       版本
ECHO      回显
DIR       列文件
CREATE    创建文件
WRITE     写文件
TYPE      读文件
DELETE    删除文件
TASKS     任务列表
REBOOT    重启
SHUTDOWN  关机

## 编译运行

chmod +x build.sh
./build.sh
qemu-system-i386 -fda disk.img