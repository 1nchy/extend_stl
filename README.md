# ABSTRACT FACTORY

## 简介

[抽象工厂模式](https://zh.wikipedia.org/wiki/%E6%8A%BD%E8%B1%A1%E5%B7%A5%E5%8E%82)（Abstract factory pattern）是一种软件开发设计模式。

它提供了一种方式，将一组具有同一主题的工厂封装起来。使用者不需要知道（或关心）它如何从这些内部的工厂方法中获得对象的具体类型，仅需使用创建这些对象的通用接口。抽象工厂模式将对象的创建与使用分离开来。

## 概述

### 使用

这是一个仅含头文件的库，使用时仅需要引用把 include 文件夹内的头文件即可。

在项目的具体使用方法，详见 [1nchy/project_template](https://github.com/1nchy/project_template)。

### 示例

我们以羽毛球相关的物品为例，详情见 [文档](./doc/badminton/usage.md)。

### 设计

抽象工厂能够根据字符串 key 与类型参数注册创建方法，并获取对象实例。

详情见 [设计文档](./doc/design.md)。

## 项目结构

~~~txt
abstract_factory
├── .gitignore
├── CMakeLists.txt
├── LICENSE.md
├── README.md
├── doc                         # 项目文档
│   ├── design.md               # 设计文档
│   └── badminton
│       └── usage.md            # 使用说明
├── include                     # 头文件（对外使用）
│   └── ...
└── example                     # 示例
    └── badminton               # 抽象工厂示例
        └── ...
~~~