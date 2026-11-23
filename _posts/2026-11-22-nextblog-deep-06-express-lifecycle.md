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
title: Express 与 TypeScript（三）：启动、中间件顺序与异步错误
date: 2026-11-22 00:06:00 +0800
description: B · Express 与 TypeScript：结合项目源码、逐段讲解和练习答案深入学习。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


> B · Express 与 TypeScript · 第 6 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## server/index.ts 为什么不直接写所有路由


代码位置：`server/index.ts`（完整文件）。

{% raw %}
````typescript
import { createDatabase } from "./db";
import { createApp } from "./app";
async function main() {
  const db = await createDatabase().initialize();
  const { app, close } = createApp(db, {
    secret: process.env.SESSION_SECRET ?? "",
    origin: process.env.APP_ORIGIN ?? "http://localhost:3000",
    production: process.env.NODE_ENV === "production",
  });
  const server = app.listen(
    Number(process.env.API_PORT ?? 4000),
    "127.0.0.1",
    () =>
      console.log(
        "API ready: http://127.0.0.1:" + (process.env.API_PORT ?? 4000),
      ),
  );
  for (const signal of ["SIGINT", "SIGTERM"])
    process.once(signal, () =>
      server.close(async () => {
        await close();
        await db.destroy();
        process.exit(0);
      }),
    );
}
main().catch((e) => {
  console.error("API startup failed:", e.message);
  process.exit(1);
});
````
{% endraw %}


main 负责启动顺序：初始化数据库，创建应用，最后监听端口。createApp 集中配置业务；拆开入口与应用创建可以让测试构造自己的 DataSource 和会话池，不必先在固定端口启动完整服务。

createDatabase().initialize() 使用 await，所以连接失败时不会继续打印 API ready。listen 的回调代表 HTTP 监听准备好，并不等于所有业务数据已经存在；迁移仍应提前运行。默认只监听回环地址，由 Next.js 转发。

## 中间件像按顺序通过的检查站

在 server/app.ts 中，JSON 解析器先将请求正文转成对象并限制大小；Session 中间件读取 Cookie 并取回会话；/api 中间件检查写请求 Origin、CSRF，再查询账号；随后匹配具体路由并验证角色和输入。

顺序很重要：若访问 req.session 的代码放在 Session 初始化之前，它拿不到需要的会话；若普通用户可以在 requireAdmin 前进入数据库写入，后面的判断已经太晚。不要只看有没有这几个函数，还要看它们怎样注册。


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
  app.post("/api/articles", requireAdmin, async (req, res) => {
    const input = articleInput.parse(req.body);
    res.status(201).json(await db.getRepository(ArticleEntity).save(input));
  });
````
{% endraw %}


这里 requireAdmin 是路由级前置处理器。后面的 async 函数只在前者调用 next 后执行。Zod 解析通过后，await save 等待写入结果，201 响应才返回保存后的对象。

## Promise 与 await 在这里做什么

数据库查询是异步操作，函数先返回 Promise；await 等待成功值或抛出错误。它让当前函数暂停等待，并不等于整个 Node 进程无法处理其他请求。不要把等待数据库理解为浏览器线程被后端锁住。

当前使用 Express 5，返回的 Promise 拒绝会进入错误处理流程。但脱离处理器返回链的异步任务，例如随意启动且未 await 的 Promise，不能假定框架会自动接住。代码应保证业务完成或失败被纳入请求的执行链。

## 错误怎样变成 HTTP 响应

app.ts 末尾的四参数错误处理器区分 Zod 错误、唯一约束错误、格式错误和内部错误。四个参数中的 error 是特殊签名的一部分。没有匹配任何路由时，前面的兜底处理器返回接口 404。

| 结果 | 当前常见原因 | 排查重点 |
| --- | --- | --- |
| 400 | 输入校验或 JSON 格式失败 | 字段、长度、类型 |
| 401 | 未登录或账号不可用 | 会话与账号 |
| 403 | 角色不符或 CSRF 失败 | message 对应分支 |
| 409 | 唯一冲突或编辑版本变化 | slug、邮箱、version |
| 500 | 未预期的服务端错误 | 服务日志与连接 |

## 关闭服务为什么还要清理数据库

index 注册 SIGINT 和 SIGTERM，收到信号后关闭 HTTP 服务，然后 close 清理 Session 存储和池，再 destroy TypeORM DataSource。连接池会占用资源，脚本执行完不清理可能导致进程挂着不退出。

这里描述的是代码注册的退出流程；实际系统强杀进程不保证所有清理回调都执行。长时间运行服务在正常退出时清理，manage.ts 这种一次性命令则用 finally 清理。

## 跟踪练习

从文章创建路由逆向列出它依赖的条件：JSON 已解析、Session 可用、来源和 token 正确、账号存在、角色为 admin、字段通过校验、数据库可写。任意一项失败都不应该被解释成“React 保存按钮没反应”。

参考答案：前端只负责发请求和显示反馈，最终应在 Network 中找到状态码，再定位对应后端分支。测试代码能直接构建 createApp，正是入口与应用拆开的实际收益。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-05-typed-express %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-07-orm-model %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [TypeScript JSX](https://www.typescriptlang.org/docs/handbook/jsx.html)
- [Express 中间件](https://expressjs.com/en/guide/using-middleware/)
- [tsx 执行工具](https://github.com/privatenumber/tsx)

- [Express 异步错误处理](https://expressjs.com/en/guide/error-handling/)
