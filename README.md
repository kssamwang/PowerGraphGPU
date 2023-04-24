# PowerGraph-GPU

This project is used to provide a generalizable GPU computation acceleration framework for distributed graph processing system.

This project is forked by [cave-g-f](https://github.com/cave-g-f/PowerGraph-GPU)

## Building Environment

Tencent Cloud Server Ubuntu18.04

NVIDIA ARCH ：Volt (V100)

NVIDIA-SMI 450.102.04

CUDA Version: 10.0

CUDNN Version: 7.5.0

## Dependencies

To simplify installation, GraphLab PowerGraph currently downloads and builds most of its required dependencies using CMake’s External Project feature. This also means the first build could take a long time.

There are however, a few dependencies which must be manually satisfied.

These are dependencies on Ubuntu 18.04.

If you use 20.04 or higher version,please downgrade version of these software dependencies.

```sh
$ cat /etc/apt/sources.list
deb http://mirrors.tencentyun.com/ubuntu/ bionic main restricted universe multiverse
deb http://mirrors.tencentyun.com/ubuntu/ bionic-security main restricted universe multiverse
deb http://mirrors.tencentyun.com/ubuntu/ bionic-updates main restricted universe multiverse
#deb http://mirrors.tencentyun.com/ubuntu/ bionic-proposed main restricted universe multiverse
#deb http://mirrors.tencentyun.com/ubuntu/ bionic-backports main restricted universe multiverse
deb-src http://mirrors.tencentyun.com/ubuntu/ bionic main restricted universe multiverse
deb-src http://mirrors.tencentyun.com/ubuntu/ bionic-security main restricted universe multiverse
deb-src http://mirrors.tencentyun.com/ubuntu/ bionic-updates main restricted universe multiverse
#deb-src http://mirrors.tencentyun.com/ubuntu/ bionic-proposed main restricted universe multiverse
#deb-src http://mirrors.tencentyun.com/ubuntu/ bionic-backports main restricted universe multiverse
```

To install them:

```sh
$ sudo apt-get update
$ sudo apt-get install gcc g++ build-essential libopenmpi-dev openmpi-bin default-jdk cmake zlib1g-dev git
```

Version details:

```sh
$ apt list --installed | grep gcc
gcc/bionic-security,bionic-updates,now 4:7.4.0-1ubuntu2.3 amd64 [installed]
gcc-7/bionic-security,bionic-updates,now 7.5.0-3ubuntu1~18.04 amd64 [installed,automatic]
gcc-7-base/bionic-security,bionic-updates,now 7.5.0-3ubuntu1~18.04 amd64 [installed,automatic]
gcc-8-base/bionic-security,bionic-updates,now 8.4.0-1ubuntu1~18.04 amd64 [installed]
libgcc-7-dev/bionic-security,bionic-updates,now 7.5.0-3ubuntu1~18.04 amd64 [installed,automatic]
libgcc1/bionic-security,bionic-updates,now 1:8.4.0-1ubuntu1~18.04 amd64 [installed]
$ apt list --installed | grep g++
g++/bionic-security,bionic-updates,now 4:7.4.0-1ubuntu2.3 amd64 [installed]
g++-7/bionic-security,bionic-updates,now 7.5.0-3ubuntu1~18.04 amd64 [installed,automatic]
$ apt list --installed | grep build-essential
build-essential/bionic,now 12.4ubuntu1 amd64 [installed]
$ apt list --installed | grep libopenmpi-dev
libopenmpi-dev/bionic,now 2.1.1-8 amd64 [installed]
$ apt list --installed | grep openmpi-bin
openmpi-bin/bionic,now 2.1.1-8 amd64 [installed]
$ apt list --installed | grep default-jdk
default-jdk/bionic-security,bionic-updates,now 2:1.11-68ubuntu1~18.04.1 amd64 [installed]
default-jdk-headless/bionic-security,bionic-updates,now 2:1.11-68ubuntu1~18.04.1 amd64 [installed,automatic]
$ apt list --installed | grep cmake
cmake/bionic-updates,now 3.10.2-1ubuntu2.18.04.2 amd64 [installed]
cmake-data/bionic-updates,bionic-updates,now 3.10.2-1ubuntu2.18.04.2 all [installed,automatic]
$ apt list --installed | grep zlib1g-dev
zlib1g-dev/bionic-security,bionic-updates,now 1:1.2.11.dfsg-0ubuntu2.2 amd64 [installed]
```


## Compiling and Running

### Build once

```sh
./configure && ./autoBuild.sh
```

### Build step by step

```sh
./configure
```

In the graphlabapi directory, will create two sub-directories, release/ and debug/ . cd into either of these directories and running make will build the release or the debug versions respectively. Note that this will compile all of GraphLab, including all toolkits. Since some toolkits require additional dependencies (for instance, the Computer Vision toolkit needs OpenCV), this will also download and build all optional dependencies.

We recommend using make’s parallel build feature to accelerate the compilation process. For instance:

```sh
make -j4
```

will perform up to 4 build tasks in parallel. When building in release/ mode, GraphLab does require a large amount of memory to compile with the heaviest toolkit requiring 1GB of RAM.

Alternatively, if you know exactly which toolkit you want to build, cd into the toolkit’s sub-directory and running make, will be significantly faster as it will only download the minimal set of dependencies for that toolkit. For instance:

```sh
cd release/toolkits/graph_analytics
make -j4
```

will build only the Graph Analytics toolkit and will not need to obtain OpenCV, Eigen, etc used by the other toolkits.
