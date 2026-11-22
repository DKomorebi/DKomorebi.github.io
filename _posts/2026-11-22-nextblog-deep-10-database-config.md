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
title: 数据库连接（一）：从 .env 到 PostgreSQL 的每一步
date: 2026-11-22 00:10:00 +0800
description: D · 数据库与会话连接：结合项目源码、逐段讲解和练习答案深入学习。
---

> D · 数据库与会话连接 · 第 10 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 先区分服务器、数据库、表与账号

PostgreSQL 服务是运行中的数据库进程；fieldnotes 是该服务管理的一个数据库；articles、users 等是库中的表；数据库用户是连接凭据的一部分。网页里注册的用户保存在 users 表，不会自动成为 PostgreSQL 连接账号。

本项目已有本地配置时保留自己的 .env。下面只展示仓库中的占位示例，没有真实密码。


{% raw %}
```dotenv
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000
```
{% endraw %}


## 逐项拆开 DATABASE_URL

postgresql 是协议类型；:// 后、@ 前的 user:password 是用户名和密码；@ 后是主机；5432 是端口；路径 fieldnotes 是库名。主机能连接、密码正确却没有这个数据库时，连接仍会失败。连接成功也不代表 articles 表已经建立。

密码有 URL 特殊字符时要正确编码，不能把 @ 混成分隔符。不要把完整 URL 打到日志里来排错，可以分别核对主机、端口、库名和账号名，避免输出密码。

| 变量 | 谁读取 | 含义 |
| --- | --- | --- |
| DATABASE_URL | 后端数据库模块和 Session 池 | 连接 PostgreSQL |
| SESSION_SECRET | Express Session | 签名会话 Cookie |
| APP_ORIGIN | Express 来源校验 | 允许的浏览器源 |
| API_PORT | Express 启动入口 | HTTP 监听端口 |
| API_ORIGIN | Next.js rewrite | 后端 HTTP 目标 |

SESSION_SECRET 不是数据库密码，API_ORIGIN 也不是数据库 URL。很多启动问题来自把这些变量都当成“后端地址”。

## 环境变量怎样进入代码


代码位置：`server/db.ts`（完整文件）。

{% raw %}
````typescript
import "reflect-metadata";
import "dotenv/config";
import { DataSource } from "typeorm";
import { UserEntity, ArticleEntity, CommentEntity } from "./entities";
import { Initial1795000000000 } from "./migration";
export function createDatabase(url = process.env.DATABASE_URL) {
  if (!url) throw new Error("请在 .env 中设置 DATABASE_URL");
  return new DataSource({
    type: "postgres",
    url,
    entities: [UserEntity, ArticleEntity, CommentEntity],
    migrations: [Initial1795000000000],
    synchronize: false,
    logging: false,
  });
}
````
{% endraw %}


import dotenv/config 在后端读取环境配置，process.env.DATABASE_URL 取到字符串。createDatabase 的默认参数使用它，也允许测试显式传入另一个 URL；没有值就抛出明确错误。

new DataSource 创建配置对象，initialize 才进行初始化并建立数据库访问能力。entities 注册 ORM 映射，migrations 注册迁移类，synchronize=false 关闭按实体自动改表。logging=false 避免默认输出查询日志，不等于所有错误都被隐藏。

## 正确启动顺序

先启动 PostgreSQL 并准备空库、账号；再配置 .env；然后 pnpm db:migrate 建表；可选 pnpm db:seed 导入文章；最后 pnpm dev 启动网页和 API。迁移依赖已经存在的数据库连接，不负责替你创建数据库服务。

compose.yaml 是可选容器方案，使用 POSTGRES_PASSWORD 并映射回环 5432；如果机器已有 PostgreSQL，不要同时争用这个端口。本项目不会因为存在 compose.yaml 就自动运行 Docker。

## localhost 的相对性

本机进程里的 localhost 指本机。将 Express 放进容器后，容器内 localhost 通常指容器自己，而不是宿主机 PostgreSQL；部署时要按网络拓扑修改连接目标。这个项目当前的本地默认值只描述本机运行，不是所有部署环境通用配置。

## 练习与参考答案

不打印密码，写出自己环境的浏览器地址、Express 目标、PostgreSQL 主机/端口/库名。自测：能打开首页是否证明数据库连通？不能，外框可能由 Next.js 返回。修改 .env 后为何需要重启？后端启动时读取配置并创建连接，已有进程不会自动重建连接池。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-09-query-migration %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-11-pools-session %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [node-postgres 连接池](https://node-postgres.com/features/pooling)
- [Express Session](https://expressjs.com/en/resources/middleware/session.html)
