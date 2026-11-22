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
title: 数据库连接（二）：连接池、Session 存储与登录持久化
date: 2026-11-22 00:11:00 +0800
description: D · 数据库与会话连接：结合项目源码、逐段讲解和练习答案深入学习。
---

> D · 数据库与会话连接 · 第 11 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 为什么项目里有两处连接数据库

一处是 TypeORM DataSource，为用户、文章、评论提供 ORM 查询；另一处是 pg.Pool，提供给 connect-pg-simple 保存 Session。它们可以指向同一个 PostgreSQL 数据库，但属于两组连接管理，不是同一个对象的两个名字。


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
  const pool =
    options.pool ?? new Pool({ connectionString: process.env.DATABASE_URL });
  const Store = connectPgSimple(session);
  const store = new Store({
    pool,
    tableName: "sessions",
    createTableIfMissing: false,
  });
  app.disable("x-powered-by");
  // API 只绑定回环地址，由 Next 同源转发；生产仅信任这一级受控代理。
  if (options.production) app.set("trust proxy", 1);
  app.use(helmet());
  app.use(express.json({ limit: "256kb" }));
  app.use((_req, res, next) => {
    res.setHeader("Cache-Control", "no-store");
    next();
  });
  app.use(
    session({
      name: "fieldnotes.sid",
      secret: options.secret,
      store,
      resave: false,
      saveUninitialized: false,
      rolling: true,
      cookie: {
        httpOnly: true,
        sameSite: "lax",
        secure: !!options.production,
        maxAge: 7 * 24 * 60 * 60 * 1000,
        path: "/",
      },
    }),
  );
````
{% endraw %}


options.pool 允许测试传入独立的池，否则按环境变量创建新池。Store 使用 sessions 表，createTableIfMissing=false 表示不在这里隐式建表，必须由迁移提前建立。显式表结构管理让生产启动职责更清楚。

## 连接池不是每次请求重新登录数据库

连接池管理可复用的连接，业务操作借用可用连接执行查询，完成后交还。若每次 HTTP 请求都 new DataSource 并 initialize，会增加连接成本并使资源更难清理。当前代码在应用启动时创建和复用连接。

两组池意味着容量规划要把它们相加；并发进程数增加时总连接也会增加。不要认为“一台 Express 只有一个连接”，也不要在不了解负载时盲目放大池上限。此处解释结构，没有修改当前连接池参数。

## Cookie 与数据库 sessions 表怎样对应

Cookie fieldnotes.sid 保存签名后的会话标识，服务器根据标识读取 sessions 表中的数据。sess 包含 userId、csrf 等字段，expire 用于会话过期管理。Cookie 本身不是完整用户档案，Session 也不保存用户明文密码。

HttpOnly 限制页面脚本直接读取 Cookie，SameSite=Lax 约束跨站携带行为，maxAge 是有效期配置，rolling 会在适用响应中滚动更新有效期，生产 secure 要求 HTTPS。它们与写请求的 Origin/CSRF 校验一起工作，各自解决不同问题。

## 登录时为什么更换 Session ID


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
  async function loginSession(req: Request, user: User) {
    await new Promise<void>((resolve, reject) =>
      req.session.regenerate((e) => (e ? reject(e) : resolve())),
    );
    req.session.userId = user.id;
    req.session.csrf = randomBytes(32).toString("hex");
    await new Promise<void>((resolve, reject) =>
      req.session.save((e) => (e ? reject(e) : resolve())),
    );
  }
````
{% endraw %}


regenerate 先重建会话，再写入 userId 和随机 csrf，save 等待持久化完成。这既更新身份边界，也避免响应跳转时会话尚未保存。重新登录得到新的 token，前端应接收服务器返回的会话状态，不应继续缓存旧 csrf。

每次后续请求，服务端再根据 userId 查询 users 表取得角色。因此 Session 保存的是身份关联，权限仍以数据库账号当前状态判断。退出会销毁会话并清 Cookie，而不是只让页面显示“登录”两个字。

## 重启后还能登录的条件

因为会话放在 PostgreSQL，正常重启 Express 不会像内存会话那样丢掉所有记录。但还需要 Cookie 没过期、签名密钥一致、数据库可访问、账号有效，以及浏览器满足 Cookie 条件。随意更换 SESSION_SECRET 可能让已有签名无法验证。

## 连接的生命周期

长期服务在启动时初始化，在正常退出时关闭两组池。manage 命令执行完必须 finally destroy；集成测试也要关闭应用存储和数据库后再删除测试数据库。仍有连接占用时清理数据库容易失败。

练习：用正常账号登录，重启自己启动的开发 API，再刷新查看会话；不要删除 sessions 表来“测试重启”。若失效，按 Cookie、密钥、过期时间和连接逐项检查，而不是直接认定 PostgreSQL 没保存。

自测：两个池必须连两个不同数据库吗？不必。保存主题偏好的 localStorage 是 Session 吗？不是，当前主题完全是浏览器外观偏好，不代表账号身份。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-10-database-config %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-12-database-operations %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [node-postgres 连接池](https://node-postgres.com/features/pooling)
- [Express Session](https://expressjs.com/en/resources/middleware/session.html)
