# Simple Scene

## 快速开始

### 构建工具

- Nix
- just (可选)

### 脚本

justfile 存放自定义脚本

首先启动一个 nix 环境

```sh
just shell
# 或 nix-shell nix/shell.nix
```

构建

```sh
just build
```

开发（构建并运行）

```sh
just dev
```

构建一个容器镜像

```sh
just image
```

## 运行容器

### Linux

#### 环境

- Intel 集显
- Ubuntu Noble LTS，原生环境非 WSL，虚拟机未测试过
- 容器运行时 `docker` 或 `podman` (rootless 模式)
- 桌面环境（X11 + Wayland）
  - 我 build 出来的东西应该用了 X11 兼容模式
  - 如果使用 `docker`，需要 `xhost` 命令

#### 运行

如果本地安装了 Docker

```sh
bash ./run.sh
```

如果安装了 Podman

```sh
CONTAINER_RUNTIME=podman bash ./run.sh
```

### Windows

#### 环境

- Intel 集显
- Podman (我选择的是 Hyper-V 安装)，Docker 没有测试过
- X Server for Windows: [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
  - Win 启动菜单点击 "XLaunch"
  - Display settings: 选择 "Multiple windows"，设置 Display number 为 0.
  - Client startup: 选择 "Start no client".
  - Extra settings:
    - 勾选 "Clipboard".
    - 勾选 "Native opengl".
    - 勾选 "Disable access control".
- 本机 IPv4 地址: `ipconfig` 获取，假设为 `172.x.x.x`

#### 启动 Podman VM 及程序

在脚本 `./run.ps1` 中替换为真实 IPv4 地址 `DISPLAY=172.x.x.x` 并运行

```ps1
./run.ps1
```

## 引用与依赖

美术素材

- [地面木板纹理](https://polyhaven.com/a/dark_wooden_planks)
- [墙纸纹理](https://polyhaven.com/a/decrepit_wallpaper)
- [中式窗模型-Weiting Ke](https://sketchfab.com/3d-models/chinese-windows-b-fb645ed041f947c4918fa1d9020d24e2)
- [搪瓷水杯-Tejay21](https://sketchfab.com/3d-models/chinese-vintage-iron-cup-1a722222f69d49f59759293a107822bd)
