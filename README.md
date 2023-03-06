## rust-lfs-server

基于rust写的[git-lfs-server](https://github.com/git-lfs/git-lfs/)，存储使用**腾讯云cos**

---

相比c版，构建更加简单，最终生成的docker镜像更小


### 开发环境


我是基于**docker**镜像+ **vscode** 开发的


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

