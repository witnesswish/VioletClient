 <div align="center"><b><a href="README.md">简体中文</a> | <a href="README_EN.md">English</a></b></div>

# VioletClient 🚀

[![C++11](https://img.shields.io/badge/C++-11-blue.svg)](https://en.cppreference.com/)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/)
[![Qt6](https://img.shields.io/badge/Qt-6-blue.svg)](https://www.qt.io/)

This soft is based on Qt 6.9, and it's a client of Violet server, Server project： [Violet](https://github.com/witnesswish/Violet.git)

    A c++ chat soft client

    A practice porject, server has been deployed, try it on

## compile
1. downlad the source code
2. compile with Qt Creator, dev version is based on Qt 6.9

## structure
version：
```mermaid
graph LR
    v1.0[basic] --> v1.1[+messageDispatcher+windowRegister]
    v1.1 --> v1.2[recv buffer accumulation resolve]
```
