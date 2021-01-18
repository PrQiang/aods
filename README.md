![license](https://img.shields.io/badge/license-MIT-brightgreen.svg)
# aods 自动化运维部署系统
## 简介：
aods是一款可以帮助运维实现软件自动化部署的简单且轻量级软件系统, 配合脚本和发布策略可以实现服务器自动化、批量化运维. 目前支持windows(服务器: windows server 2003 sp2及高版本, 客户端: windows 7以及高版本), centos, ubuntu.

根据笔者所在企业使用情况，600+服务器是没有任何压力的，不过根据aods架构设计理论上应可以轻松支持万级别服务器的自动化部署，目前尚未测试验证过。

笔者开源的目的分享使用的同时改进系统。交流方式: 邮箱: 2114647743@qq.com, qq:2114647743。

## 安装使用：
    系统通过安装在各个服务器上的客户端aods服务接收服务端的更新控制指令来实现其管理的模块的部署更新。
### 服务端  
* redis安装: 请参阅<https://redis.io/download>
   
* kafka安装: 请参阅<http://kafka.apache.org/quickstart>
   
* http下载服务安装: 可根据个人或企业情况安装http下载服务，windows可以使用iis, centos可以使用httpd
   
* aodc安装: 使用命令aodc -i
  - 配置文件[./config/aodccfg.json](config/aodccfg.json "aodc服务配置文件")

* DeployCtrl: 
   - 采用python3编写，可直接使用脚本运行，也可使用pyinstaller打包为执行文件部署

   - 使用前请修改DeployCtrl.py最后一行DeployCtrl("192.168.66.124:9092", "192.168.66.124:65379", "deploy-1", 1).Run() 参数
      + 第一个参数为: kafka broker
      + 第二个参数为redis地址
      + 第三个参数为kafka消费组ID
      + 第四个参数表示维护的段ID，这个和客户端aods的段ID一致
   - 多个段时，需要启动多份该脚本程序，且不同脚本程序采用不同的kafka消费组ID和维护段ID，
      
    
* Publish: 采用python3编写，在需要发布的时候执行下即可

### 客户端
* aods安装: install -i 段号 将会在本地安装为aods服务运行

  - 段号用来规划部署环境定义，比如笔者这里定义1为测试环境, 2为预发布环境，3-255为正式环境或运营环境
  - 配置文件: 
    + [./config/installcfg.json](config/installcfg.json "安装程序配置文件")
    + [./config/aodscfg.json](config/aodscfg.json "部署服务客户端配置文件")
    + [./data/aodsumi.db](data/aodsumi.db "部署服务管理项目数据文件")
    
## 应用举例:
* 服务端业务模块自动部署更新

* 运维环境安全策略初始化以及更新调整

* 服务器脚本自动部署实现服务器批量运维
## 关于安全性：
* 下载文件安全： 由于每个下载更新包独立密钥加密，可防止更新包信息泄露；客户端aods对每个下载包均执行hash校验，可防止更新包文件被篡改.

* 增加aodc中间层，可避免kafka暴漏在公网，因此强烈建议在防火墙中添加kafka的访问端口仅对aodc和脚本服务开放.

* redis: redis服务端口仅对脚本服务开放.

* 安全无绝对，对于有条件的童鞋可重构c++中CAoEncrypt以及python中Publish模块中加密算法

## at last
 感谢束大师对该项目开源支持

 小伙伴们若在使用过程遇到什么问题或bug，请反馈至： 2114647743@qq.com, qq:2114647743，谢谢~

