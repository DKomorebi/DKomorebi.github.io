---
layout: post
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/专题报告
keywords: NextBlog, React, Next.js, Express, TypeScript, TypeORM, PostgreSQL
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
title: NextBlog 专题报告导读：四组十二篇学习路线
date: 2026-11-22 00:00:00 +0800
description: 四组十二篇详细报告，从具体文件理解 React、Next.js、Express、TypeORM 和数据库连接。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


这组报告专门解决“代码能运行，但不知道每一层为什么这样写”的问题。共四组、十二篇，每组按概念、源码和业务链路递进。建议每次读一篇，同时打开文中具体文件，用自己的话回答文末自测，再继续下一篇。

原来的十五篇仍是完整项目教程与源码归档，本专题补充更细的解释，不改变已有实现。下面的代码节选均从当前项目文件提取；说明性 SQL 和未来改进建议不会作为新功能写入项目。

| 学习组 | 报告 | 读完应该能说明 |
| --- | --- | --- |
| A：React 与 Next.js | 01–03 | 组件与路由的区别，渲染位置，一次阅读请求的路径 |
| B：Express 与 TypeScript | 04–06 | .ts/.tsx/tsx 的区别，类型与运行时校验，服务启动顺序 |
| C：TypeORM | 07–09 | 实体与迁移、Repository CRUD、并发更新与查询 |
| D：数据库连接 | 10–12 | 连接串、两个连接池、Session、迁移与排错 |

最直接回答当前疑问的顺序是 **01 → 04 → 07 → 10**，然后按每组顺序深入。学习数据库时先读配置，不需要展示或复制真实密码；代码中的环境变量示例只使用占位值。

## 报告目录

1. [React 与 Next.js（一）：它们各自解决什么问题]({% post_url 2026-11-22-nextblog-deep-01-react-next %})
2. [React 与 Next.js（二）：服务端组件、客户端组件与页面渲染]({% post_url 2026-11-22-nextblog-deep-02-rendering %})
3. [React 与 Next.js（三）：从文章地址追踪一次完整请求]({% post_url 2026-11-22-nextblog-deep-03-request-path %})
4. [Express 与 TypeScript（一）：为什么 server 里都是 .ts]({% post_url 2026-11-22-nextblog-deep-04-ts-files %})
5. [Express 与 TypeScript（二）：req、res、next 与类型扩展]({% post_url 2026-11-22-nextblog-deep-05-typed-express %})
6. [Express 与 TypeScript（三）：启动、中间件顺序与异步错误]({% post_url 2026-11-22-nextblog-deep-06-express-lifecycle %})
7. [TypeORM（一）：实体、类型、表与迁移到底是什么]({% post_url 2026-11-22-nextblog-deep-07-orm-model %})
8. [TypeORM（二）：Repository 怎样完成文章增删改查]({% post_url 2026-11-22-nextblog-deep-08-orm-crud %})
9. [TypeORM（三）：搜索、并发编辑、迁移与事务思维]({% post_url 2026-11-22-nextblog-deep-09-query-migration %})
10. [数据库连接（一）：从 .env 到 PostgreSQL 的每一步]({% post_url 2026-11-22-nextblog-deep-10-database-config %})
11. [数据库连接（二）：连接池、Session 存储与登录持久化]({% post_url 2026-11-22-nextblog-deep-11-pools-session %})
12. [数据库连接（三）：迁移、初始化、健康检查与故障定位]({% post_url 2026-11-22-nextblog-deep-12-database-operations %})

## 从哪里开始动手

[从零搭建的思路、实施步骤与验收路线]({% post_url 2026-11-23-nextblog-build-roadmap %})按页面、接口、数据库、权限和验收安排实际动手顺序。

## DDD 与独立后端构建

[Express + TypeScript + DDD 教学与构建文档]({% post_url 2026-11-23-express-ts-ddd-00-guide %})解释原模板、现有博客分层与 node_modules 的区别。
