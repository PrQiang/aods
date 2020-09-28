# aods 自动化运维部署系统
## 简介：
aods是一款可以帮助运维实现软件自动化部署的轻量级软件系统, 配合脚本和发布策略可以实现服务器自动化、批量化运维. 目前支持windows(服务器: windows server 2003 sp2及高版本, 个人版: windows 7以及高版本), centos, ubuntu.

根据aods架构设计理论上应可以轻松支持万级别服务器的自动化部署，尚未测试验证过，不过根据笔者所在企业使用情况，600+服务器是没有任何压力的。

笔者开源的目的集和大家的智慧一起改进系统，如果企业运维需要，也欢迎使用。

## 项目构成
本项目由客户端aods(windows为aods.exe, linux为aods), 服务端aodc(windows为aodc.exe, linux为aodc),以及python脚本控制端DeployCtrl，Python打包发布工具Publish，如下图:

```flow
st=>start: aods/aods.exe
op1=>operation: aodc/aodc.exe
op2=>operation: kafka
op3=>operation: DeployCtrl
st(right)->op1(right)->op2(right)->op3
```


## 建设中。。。
