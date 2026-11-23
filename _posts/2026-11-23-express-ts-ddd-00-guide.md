---
layout: post
title: Express + TypeScript + DDD：教学与构建文档导读
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/DDD架构
description: Express + TypeScript + DDD：教学与构建文档导读：结合真实模板与现有 NextBlog 源码逐步解释。
keywords: Express, TypeScript, DDD, TypeORM, Prisma, node_modules
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


这组文档包含四篇：先理解同名模板的真实架构，再按 Windows 环境搭建，随后规划现有博客的 TypeORM + Session 迁移，最后解释 node_modules。

同名参考仓库为 [gonzaloplaza/express-ts-ddd](https://github.com/gonzaloplaza/express-ts-ddd)，核对提交 0de3752。若团队指的是其他脚手架，应按实际仓库重新核对。DDD 是设计思路，不强制使用这个模板或某一种 ORM。

当前博客仍按原接口运行，本次交付没有替换数据库和认证。自编四文件示例位于 NextBlog 的 docs/ddd-example，已完成严格类型编译和四项行为测试；原模板则仅完成源码核对，没有宣称完整安装、Cognito 或生产部署验收。

建议先读第一篇，再读第三篇理解现有代码如何对应；要实际克隆原模板时再按第二篇操作。关于依赖目录的疑问可以直接从第四篇开始。

## 文档目录

- [express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入]({% post_url 2026-11-23-express-ts-ddd-01-architecture %})
- [express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收]({% post_url 2026-11-23-express-ts-ddd-02-build %})
- [DDD 实战（三）：把当前 TypeORM 博客逐步分层]({% post_url 2026-11-23-express-ts-ddd-03-nextblog %})
- [依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有]({% post_url 2026-11-23-express-ts-ddd-04-node-modules %})
