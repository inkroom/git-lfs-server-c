## c-lfs-server

基于c写的[git-lfs-server](https://github.com/git-lfs/git-lfs/)，web框架使用[facil.io](https://facil.io)，存储使用[腾讯云cos](https://github.com/tencentyun/cos-c-sdk-v5)


### 开发环境

安装以下环境


```shell
apt install -y libapr1-dev libaprutil1-dev libcurl4-openssl-dev curl
```

此外还需要编译 [腾讯云sdk](https://github.com/tencentyun/cos-c-sdk-v5)


可以参照 [Dockerfile](Dockerfile) 内容


### 使用方法

#### 初始化

```shell
git init 
git config lfs.url http://host:port/
git lfs install 
git lfs track "*.png" 
git lfs track "*.jpg" 
git config lfs.http://host:port/.locksverify false
git config credential.http://host:port.username 用户名
git config credential.helper 'store' 
echo "http://账号:密码@host%3aport" > ~/.git-credentials ## 这是为了避免push时要求输入密码，具体内容可以在执行了前两条命令的情况下，手动push一次，再去指定文件获取内容 如 http://username:password@10.0.90.95%3a48013
```


对应自建的gitlab仓库，最好把仓库里的**LFS**支持给**关了**，否则有可能无法正常**push**

#### 已有仓库克隆

```shell
GIT_LFS_SKIP_SMUDGE=1 git clone 仓库地址 # 跳过LFS拉取
git config lfs.url http://host:port/
git lfs pull # 重新拉取文件
```

