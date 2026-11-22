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
title: Express 与 TypeScript（二）：req、res、next 与类型扩展
date: 2026-11-22 00:05:00 +0800
description: B · Express 与 TypeScript：结合项目源码、逐段讲解和练习答案深入学习。
---

> B · Express 与 TypeScript · 第 5 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## Express 仍然是 Express

TypeScript 没有给 Express 换一套请求协议。请求仍包含 URL、方法、头和正文，响应仍包含状态码和数据。变化是代码能声明 Request、Response、NextFunction 等类型，让编辑器了解对象提供哪些属性和函数。

express 提供运行时实现，@types/express 提供本项目所用的类型声明。安装类型声明不会启动服务器；删除类型标注也不会替你完成权限检查。理解运行时对象与类型信息的区别，才能读懂下面的代码。


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
  const requireUser = (req: Request, res: Response, next: NextFunction) => {
    if (!req.account) {
      res.status(401).json({ message: "请先登录" });
      return;
    }
    next();
  };
  const requireAdmin = (req: Request, res: Response, next: NextFunction) => {
    if (!req.account) {
      res.status(401).json({ message: "请先登录" });
      return;
    }
    if (req.account.role !== "admin") {
      res.status(403).json({ message: "仅管理员可管理文章" });
      return;
    }
    next();
  };
````
{% endraw %}


req 是当前请求，res 是当前响应，next 让 Express 继续执行后续匹配的处理器。401 和 403 分支发送 JSON 后 return，不再 next；通过检查才继续。res.status 设状态码，res.json 发送 JSON，return 则结束这个 JavaScript 函数的后续执行。

如果既不响应又不 next，请求可能一直等待；如果响应之后继续进入另一段响应逻辑，可能发生重复发送。这是控制流问题，不是加上 Request 类型就能自动避免的问题。

## 为什么 req 上能写 account


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
declare module "express-session" {
  interface SessionData {
    userId?: string;
    csrf?: string;
  }
}
declare global {
  namespace Express {
    interface Request {
      account?: User;
    }
  }
}
````
{% endraw %}


Express 默认并不知道你要附加账号对象。declare global 中的 Request 接口扩展让 TypeScript 接受 req.account。account? 表示这个字段可能没有值，因为游客请求同样进入应用。

另一个 declare module 扩展 express-session 的 SessionData，使 userId 与 csrf 成为项目能使用的字段。两段声明只描述形状，运行时必须由真实代码为 session 和 account 赋值。

每次 API 请求，如果 Session 中存在 userId，应用从数据库读取用户再赋给 req.account。这使角色变更能够在后续请求生效，也避免直接相信浏览器提交的 role。

## 三种类型语法不要混淆

可选属性 account? 表示可能缺失；可选链 account?.role 表示缺失时安全停止取值；非空断言 account! 表示开发者声明此处相信它存在。感叹号不会检查登录，也不会构造对象，必须依赖前置 requireUser 或 requireAdmin 的正确顺序。

同理，`error as { code?: string }` 是给检查器的类型提示，不会把任何异常真的转换成带 code 的对象。运行时分支仍需处理缺失值和未知异常。

## TS 检查过了，为何还要 Zod


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
const articleInput = z
  .object({
    title: z.string().trim().min(1).max(160),
    slug: z
      .string()
      .regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/)
      .max(120),
    excerpt: z.string().trim().max(500),
    category: z.string().trim().min(1).max(40),
    content: z.string().trim().min(1).max(200000),
  })
  .strict();
````
{% endraw %}


网络客户端不需要通过你项目的 TypeScript 编译，所以 req.body 必须在运行时检查。parse 执行规则：去除两端空格、非空、长度、slug 格式和 strict 额外字段限制。失败时错误处理中间件返回 400。

可以把边界理解成三层：TypeScript 约束你写代码时的用法，Zod 检查客户端实际送来的数据，数据库约束保护最终保存的记录。任何一层都不能完全替代其他两层。

## 练习与参考答案

对照源码找到 req.account 的声明、赋值和读取三处。声明不会访问数据库，赋值发生在 API 中间件，读取发生在权限与评论业务。若只保留 declare global 而删除赋值，程序会有类型提示，但用户仍无法获得正确登录身份。

再回答：前端把 role 字符串写成 admin，会改变数据库角色吗？不会；注册 schema 拒绝多余字段，创建用户时角色固定为 user，后续授权通过服务端命令完成。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-04-ts-files %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-06-express-lifecycle %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [TypeScript JSX](https://www.typescriptlang.org/docs/handbook/jsx.html)
- [Express 中间件](https://expressjs.com/en/guide/using-middleware/)
- [tsx 执行工具](https://github.com/privatenumber/tsx)
