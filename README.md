node-ctp
========

高性能CTP交易框架

## Why?

Shif发布的CTP接口是基于C++语言开发的，我们使用CTP开发交易策略软件时，一般也使用C++语言。
我本人觉得这样不是很方便，封装成Node.js模块，我考虑基于以下两点：

    1. 使用Javascript极大的方便了交易策略的编写。
    2. 提供了一个高性能的并发框架，并支持多账户交易。


## 运行环境

我分别给出了Linux和Windows平台下的工程，由于Shif发布接口文件平台的缘故，Linux版本的运行
在Node.js x64下，Windows运行在Node.js x86下。注意下载不同版本的Node.js,下载地址：[Download Node.js](http://www.nodejs.org/download/).
相应的Shif发布的Linux平台下CTP包选用x64的，而Windows平台的选用x86的。下载地址：[Download tradeapi](http://www.sfit.com.cn/5_2_DocumentDown.htm)

    linux:centos 6.4 x64;
    gcc:v4.4.7 20120313;
    node-gyp:v1.0.2;
    node.js:v0.10.26;
    tradeapi:6.3.0_20140811_traderapi_linux64

    windows:win7 x64;
    visual studio 2013;
    node-gyp:v1.0.2;
    node.js:v0.10.26 x86;
    tradeapi:6.2.5_20140811_traderapi_win32


## 编译

1. `npm install -g node-gyp`
1. `npm install`
1. 打开binding.gyp文件，把libraries节点的路径修改成自己开发环境的真实路径；
1. `node-gyp rebuild`

注意：windows环境下，node-gyp默认生成vs2010版本的工程，如果没有安装此版本的vs，需要设置环境变量
如：GYP_MSVS_VERSION=2013

## Run Demo

1. `cp demo/config.example.js demo/config.js`
1. edit demo/config.js
1. `node demo/mduser`

[More Demo](./demo)

## 介绍

待续
