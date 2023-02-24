## c-lfs-server

基于c写的[git-lfs-server](https://github.com/git-lfs/git-lfs/)，web框架使用[facil.io](facil.io)，存储使用[腾讯云cos](https://github.com/tencentyun/cos-c-sdk-v5)


### 开发环境

安装以下环境


```shell
apt install -y libapr1-dev libaprutil1-dev libcurl4-openssl-dev curl
```

此外还需要编译 [腾讯云sdk](https://github.com/tencentyun/cos-c-sdk-v5)


可以参照 [Dockerfile](Dockerfile) 内容
