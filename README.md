# SHC (Software and Hardware Collaboration)

## 运行环境说明
容器环境为腾讯基于锐捷可编程交换机而创造的，为了适配学校实验室的交换机环境，对工程进行了部分修改。
1. 对cmake文件夹中，p4.cmake文件中的-DP4PPFLAGS="-Xp4c=--disable-parse-depth-limit" 进行了注释，sde9.7.0不支持此条命令

### 实验室环境
两台不同厂商的交换机，需要找厂商要bsp，可以找厂商要onl和内核以及内核头文件
1. 

实验室tofino交换机环境配置
物理机系统：3.16.39-OpenNetworkLinux
容器系统："Debian GNU/Linux 9 (stretch)"

指令：./p4studio configure --bsp-path ../../bf-platforms-ingrasys-bsp-9.7.0.tgz

/home/buildsde/bf-sde-9.7.1/pkgsrc/bf-platforms 会更新

/home/buildsde/bf-sde-9.7.1/install/lib 下 libpltfm.so 

bsp更新依赖宿主机操作系统