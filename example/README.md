# example

## 架构模型: 
![pic/framework.png](../pic/framework.png)

## 部署模型：
![pic/model.png](../pic/model.png)


## 扩展性: 
管理服务器很大时，可考虑在每个机房部署一个aodc服务,http下载服务；

发布时，可采用sftp等协议同时上转至不同的http服务上

## 健壮性：
建议部署一个备份aodc服务节点，当主节点故障时，aods可以自动切换至备份服务器;

根据需要可以考虑建kafka集群，单点故障时可自动平滑切db
