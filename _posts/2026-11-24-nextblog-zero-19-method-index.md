---
layout: post
title: 方法与结构索引（十九）：从文件、方法和事件反查教学章节
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 方法与结构索引（十九）：从文件、方法和事件反查教学章节：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 目录结构就是职责地图

{% raw %}
```text
NextBlog/
  src/app/                  Next路由、布局、全局CSS
  src/components/           共享业务组件、Hero和主题切换
  src/lib/                  主题常量与旧示例文章（seed来源）
  src/types/                种子文章类型
  server/index.ts           API启动和退出
  server/app.ts             HTTP中间件、路由和错误处理
  server/db.ts              TypeORM连接配置
  server/entities.ts        用户、文章、评论映射
  server/migration.ts       初始表结构
  server/registration-migration.ts  验证码表增量迁移
  server/registration.ts    验证码与注册事务
  server/mail.ts            SMTP适配
  server/manage.ts          迁移、种子和管理员命令
  server/*.test.ts          PostgreSQL集成测试
  docs/ddd-example/         独立教学模型，不是运行后端入口
  docs/from-zero/           本套详细教程与源码快照
  next.config.ts            同源API转发
  package.json              前后端共同的依赖和脚本
  pnpm-lock.yaml            确切依赖锁定
```
{% endraw %}

## 2. 怎样读下面的函数表

索引用TypeScript编译器AST从src和server提取，包括命名函数、箭头函数、类方法、Effect与JSX内联回调，避免只列导出组件而漏掉真正触发业务的代码。行号指本次源码快照；修改源码后应重新生成，不能认为固定行号永久不变。

“callback”不是缺失了函数，而是源码采用匿名回调。例如.then/.catch负责请求完成反馈，Promise构造器回调把Session旧式回调转成await，useEffect返回的函数是清理。表里给出所属作用域和对应解释章；完整代码在第二十、二十一篇。

React库函数setState不是本站定义的新方法，故不单独展开其内部源码；重点是本站如何调用它们。DDD教学模型的方法在第十六篇逐个解释和完整归档。

### server/app.test.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 10 | `test callback` | test回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 24 | `test callback / mailer` | 函数调用 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |

### server/app.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 42 | `z .string() .min(10) .max(72) .refine callback` | z .string() .min(10) .max(72) .refine回调 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 57 | `publicUser` | 函数调用 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 63 | `createApp` | 函数调用 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 91 | `createApp / app.use callback` | app.use回调 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 112 | `createApp / app.use /api callback` | HTTP请求 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 136 | `createApp / requireUser` | 函数调用 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 143 | `createApp / requireAdmin` | 函数调用 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 161 | `createApp / app.get /api/health callback` | HTTP请求 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 165 | `createApp / app.get /api/auth/session callback` | HTTP请求 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 172 | `createApp / loginSession` | 函数调用 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 173 | `createApp / loginSession / callback` | 闭包/Promise回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 174 | `createApp / loginSession / callback / req.session.regenerate callback` | req.session.regenerate回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 178 | `createApp / loginSession / callback` | 闭包/Promise回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 179 | `createApp / loginSession / callback / req.session.save callback` | req.session.save回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 188 | `createApp / app.post /api/auth/register/code callback` | HTTP请求 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 193 | `createApp / app.post /api/auth/register callback` | HTTP请求 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 201 | `createApp / app.post /api/auth/register callback / ….refine callback` | ….refine回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 209 | `createApp / app.post /api/auth/login callback` | HTTP请求 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 226 | `createApp / app.post /api/auth/logout callback` | HTTP请求 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 227 | `createApp / app.post /api/auth/logout callback / req.session.destroy callback` | req.session.destroy回调 | [第 13 篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) |
| 238 | `createApp / app.get /api/articles callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 272 | `createApp / app.get /api/categories callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 284 | `createApp / app.get /api/articles/:slug callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 294 | `createApp / app.post /api/articles callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 298 | `createApp / app.put /api/articles/:id callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 307 | `createApp / app.put /api/articles/:id callback / version` | 函数调用 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 317 | `createApp / app.delete /api/articles/:id callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 327 | `createApp / app.get /api/articles/:id/comments callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 344 | `createApp / app.get /api/articles/:id/comments callback / items.map callback` | items.map回调 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 361 | `createApp / app.post /api/articles/:id/comments callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 382 | `createApp / app.delete /api/comments/:id callback` | HTTP请求 | [第 08 篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) |
| 396 | `createApp / app.use callback` | app.use回调 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 398 | `createApp / app.use callback` | app.use回调 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 409 | `createApp / app.use callback / error.issues.map callback` | error.issues.map回调 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |
| 429 | `createApp / close` | 函数调用 | [第 05 篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) |

### server/db.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 7 | `createDatabase` | 函数调用 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |

### server/index.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 4 | `main` | 函数调用 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 15 | `main / app.listen callback` | app.listen回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 21 | `main / process.once callback` | process.once回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 22 | `main / process.once callback / server.close callback` | server.close回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 29 | `main().catch callback` | main().catch回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |

### server/mail.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 6 | `createRegistrationMailer` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 26 | `createRegistrationMailer / callback` | 清理回调 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |

### server/manage.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 4 | `main` | 函数调用 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 31 | `main / p.sections .map callback` | p.sections .map回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 43 | `main().catch callback` | main().catch回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |

### server/migration.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `up` | 函数调用 | [第 07 篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %}) |
| 11 | `down` | 函数调用 | [第 07 篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %}) |

### server/registration-migration.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 4 | `up` | 函数调用 | [第 07 篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %}) |
| 17 | `down` | 函数调用 | [第 07 篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %}) |

### server/registration.test.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 11 | `test callback` | test回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 23 | `test callback / now` | 函数调用 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 27 | `test callback / mailer` | 函数调用 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 39 | `test callback / post` | 函数调用 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 41 | `test callback / payload` | 函数调用 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 95 | `test callback / concurrent.filter callback` | concurrent.filter回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 99 | `test callback / signups.filter callback` | signups.filter回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 102 | `test callback / assert.rejects callback` | assert.rejects回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |
| 114 | `test callback / assert.rejects callback` | assert.rejects回调 | [第 18 篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) |

### server/registration.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 8 | `constructor` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 13 | `createRegistration` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 17 | `createRegistration / callback` | 闭包/Promise回调 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 19 | `createRegistration / digest` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 25 | `createRegistration / sendCode` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 66 | `createRegistration / register` | 函数调用 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |
| 68 | `createRegistration / register / db.transaction callback` | db.transaction回调 | [第 14 篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) |

### src/app/about/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 2 | `About` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/admin/[id]/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/admin/new/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/admin/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 2 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/archive/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `Archive` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/layout.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 9 | `RootLayout` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/login/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/not-found.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `NotFound` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 2 | `Home` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/posts/[slug]/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 2 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/app/register/page.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 3 | `Page` | 函数调用 | [第 03 篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) |

### src/components/full-blog.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 58 | `api` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 75 | `api / r.json().catch callback` | r.json().catch回调 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 90 | `refresh` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 91 | `setSession` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 93 | `SessionProvider` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 97 | `SessionProvider / refresh` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 107 | `SessionProvider / useEffect callback` | Effect建立或清理 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 116 | `date` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 122 | `ErrorMessage` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 129 | `Header` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 132 | `Header / logout` | 函数调用 | [第 09 篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) |
| 189 | `Markdown` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 197 | `Markdown / a` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 202 | `Markdown / img` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 220 | `ArticleList` | 函数调用 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 237 | `ArticleList / useEffect callback` | Effect建立或清理 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 239 | `ArticleList / useEffect callback / setTimeout callback` | setTimeout回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 244 | `ArticleList / useEffect callback / setTimeout callback / ….then callback` | ….then回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 251 | `ArticleList / useEffect callback / setTimeout callback / ….catch callback` | ….catch回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 254 | `ArticleList / useEffect callback / setTimeout callback / ….finally callback` | ….finally回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 258 | `ArticleList / useEffect callback / callback` | 清理回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 263 | `ArticleList / useEffect callback` | Effect建立或清理 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 266 | `ArticleList / useEffect callback / api<{ name: string }[]>("/categories") .then(setCategories) .catch callback` | api<{ name: string }[]>("/categories") .then(setCategories) .catch回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 268 | `ArticleList / remove` | 函数调用 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 273 | `ArticleList / remove / setRevision callback` | setRevision回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 312 | `ArticleList / onChange` | JSX onChange | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 323 | `ArticleList / onChange` | JSX onChange | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 329 | `ArticleList / categories.map callback` | categories.map回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 339 | `ArticleList / onClick` | JSX onClick | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 339 | `ArticleList / onClick / setRevision callback` | setRevision回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 353 | `ArticleList / onClick` | JSX onClick | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 365 | `ArticleList / items.map callback` | items.map回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 393 | `ArticleList / items.map callback / onClick` | JSX onClick | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 410 | `ArticleList / onClick` | JSX onClick | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 410 | `ArticleList / onClick / setPage callback` | setPage回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 420 | `ArticleList / onClick` | JSX onClick | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 420 | `ArticleList / onClick / setPage callback` | setPage回调 | [第 10 篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) |
| 429 | `AuthForm` | 函数调用 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 445 | `AuthForm / useEffect callback` | Effect建立或清理 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 447 | `AuthForm / useEffect callback / window.setInterval callback` | window.setInterval回调 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 450 | `AuthForm / useEffect callback / callback` | 清理回调 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 453 | `AuthForm / sendCode` | 函数调用 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 476 | `AuthForm / submit` | 函数调用 | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 550 | `AuthForm / onChange` | JSX onChange | [第 12 篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) |
| 635 | `ArticleReader` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 639 | `ArticleReader / useEffect callback` | Effect建立或清理 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 643 | `ArticleReader / useEffect callback / api<Article>("/articles/" + encodeURIComponent(slug)) .then callback` | api<Article>("/articles/" + encodeURIComponent(slug)) .then回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 649 | `ArticleReader / useEffect callback / ….catch callback` | ….catch回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 652 | `ArticleReader / useEffect callback / callback` | 清理回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 663 | `ArticleReader / onClick` | JSX onClick | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 663 | `ArticleReader / onClick / setRevision callback` | setRevision回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 688 | `Comments` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 697 | `Comments / useEffect callback` | Effect建立或清理 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 702 | `Comments / useEffect callback / ….then callback` | ….then回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 709 | `Comments / useEffect callback / ….catch callback` | ….catch回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 712 | `Comments / useEffect callback / callback` | 清理回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 716 | `Comments / submit` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 728 | `Comments / submit / setRevision callback` | setRevision回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 735 | `Comments / remove` | 函数调用 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 740 | `Comments / remove / setRevision callback` | setRevision回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 761 | `Comments / onChange` | JSX onChange | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 778 | `Comments / items.map callback` | items.map回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 787 | `Comments / items.map callback / onClick` | JSX onClick | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 805 | `Comments / onClick` | JSX onClick | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 805 | `Comments / onClick / setPage callback` | setPage回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 815 | `Comments / onClick` | JSX onClick | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 815 | `Comments / onClick / setPage callback` | setPage回调 | [第 11 篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) |
| 831 | `Editor` | 函数调用 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 841 | `Editor / useEffect callback` | Effect建立或清理 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 849 | `Editor / useEffect callback / api<Article>("/articles/" + encodeURIComponent(slug)) .then callback` | api<Article>("/articles/" + encodeURIComponent(slug)) .then回调 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 863 | `Editor / useEffect callback / ….catch callback` | ….catch回调 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 866 | `Editor / useEffect callback / callback` | 清理回调 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 870 | `Editor / useEffect callback` | Effect建立或清理 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 871 | `Editor / useEffect callback / listener` | 函数调用 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 875 | `Editor / useEffect callback / callback` | 清理回调 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 877 | `Editor / change` | 函数调用 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 879 | `Editor / change / setForm callback` | setForm回调 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 881 | `Editor / save` | 函数调用 | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 922 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 933 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 943 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 952 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 962 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |
| 982 | `Editor / onChange` | JSX onChange | [第 15 篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) |

### src/components/home-hero.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 2 | `HomeHero` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |

### src/components/theme-toggle.tsx

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 6 | `ThemeToggle` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 10 | `ThemeToggle / useEffect callback` | Effect建立或清理 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 12 | `ThemeToggle / useEffect callback / apply` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 16 | `ThemeToggle / useEffect callback / sync` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 31 | `ThemeToggle / useEffect callback / storage` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 35 | `ThemeToggle / useEffect callback / callback` | 清理回调 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 40 | `ThemeToggle / toggle` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 44 | `ThemeToggle / toggle / apply` | 函数调用 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |
| 70 | `ThemeToggle / toggle / transition.finished.catch callback` | transition.finished.catch回调 | [第 17 篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) |

### src/lib/posts.ts

| 起始行 | 作用域 / 方法 | 触发方式 | 解释章节 |
| --- | --- | --- | --- |
| 209 | `getPost` | 函数调用 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 210 | `getPost / posts.find callback` | posts.find回调 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |
| 213 | `formatDate` | 函数调用 | [第 06 篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) |

本次共识别 **176** 个函数、方法或回调。按前述章节阅读业务方法，再通过此表定位对应源码。

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-20-source-frontend %})
