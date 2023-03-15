## rust-lfs-server

基于rust写的[git-lfs-server](https://github.com/git-lfs/git-lfs/)，存储使用**腾讯云cos**，要求存储库为公有读，私有写

---

相比c版，构建更加简单，~~最终生成的docker镜像更小~~


### 开发环境


我是基于[**docker**](https://gist.github.com/inkroom/501548078a930c6f3bd98ea257409648#file-dockerfile-rust)镜像+ **vscode** 开发的


### features

- **plog**
>  使用日志组件，否则使用普通的输出
- **bucket** 默认不启动
> 自动创建存储桶(要求能联网)，关闭能减小构建体积，不启用的情况下最好提前创建存储桶
- **thread** 默认不启动
> 多线程server，单个请求会降低性能


### 使用方法


#### 存储桶

lfs-server将从url中获取存储桶，如 `http://localhost:8898/examples`  此处 **examples** 就是存储桶，注意其命名规范，cos文档说明如下:

> - 仅支持小写英文字母和数字，即[a-z，0-9]、中划线“-”及其组合。
> - 存储桶名称的最大允许字符受到 地域简称 和 APPID 的字符数影响，组成的完整请求域名字符数总计最多60个字符。例如请求域名 123456789012345678901-1250000000.cos.ap-beijing.myqcloud.com 总和为60个字符。
> - 存储桶命名不能以“-”开头或结尾。




#### 初始化

```shell
git init 
git config lfs.url http://host:port/{bucket}
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
git config lfs.url http://host:port/{bucket}
git lfs install
git lfs pull # 重新拉取文件
```

