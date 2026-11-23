---
layout: post
title: Express 语法（五）：把 req、res、next 和整条中间件链写明白
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: Express 语法（五）：把 req、res、next 和整条中间件链写明白：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 最小可运行示例先于项目大文件

下面是独立学习示例，不要直接替换现有 app.ts：

{% raw %}
```ts
import express, { type Request, type Response, type NextFunction } from "express";
const app = express();
app.use(express.json());
app.use((req: Request, _res: Response, next: NextFunction) => {
  console.log(req.method, req.path); // 不要记录密码、Cookie 或完整请求体
  next();
});
app.get("/hello", (_req, res) => res.json({ message: "hello" }));
app.listen(4001, "127.0.0.1");
```
{% endraw %}

`express()` 创建应用对象。`app.use` 注册顺序执行的中间件；`app.get` 指定 HTTP 方法与路径；`res.json` 序列化 JSON 并结束响应；`listen` 开始监听端口。注册路由时回调还没执行，等请求匹配才会执行。

## 2. 参数名不是魔法，参数位置有约定

req 提供 method、params、query、body、headers；res 提供 status、json、end、clearCookie；next 把处理交给下一层。名字可以改，但一般按这套约定写，便于阅读。

`/articles/:id` 的 id 在 req.params；`/articles?page=2` 的 page 在 req.query；POST JSON 在 req.body。`req.query.page` 本质是外部输入，不能因为 TypeScript 写了 number 就当成数字，所以项目用 `z.coerce.number().int().min(1)`。

`res.status(403).json(...)` 是链式调用：status 返回 res，json 再发响应。响应后 `return` 是为了退出当前 JS 函数，避免继续执行数据库写入或再次响应。`next()` 不是 return，也不是把函数返回值交给浏览器；调用之后如仍有代码，JS 仍会继续执行它。

## 3. createApp(db, options) 的每一步

它接受已经 initialize 的 DataSource 和依赖配置，返回 `{ app, close }`。将创建应用与监听端口分开，Supertest 才能直接测试 app，不需要抢占固定 4000 端口。

| 顺序 | 代码动作 | 若顺序错误会怎样 |
| --- | --- | --- |
| 1 | 校验 SESSION_SECRET 长度、创建注册服务和 Session Store | 密钥不足会拒绝启动 |
| 2 | disable x-powered-by；生产配置 trust proxy | 代理影响协议/IP 判断，必须与实际拓扑一致 |
| 3 | helmet 与 express.json({ limit: "256kb" }) | JSON 解析必须先于读取 req.body |
| 4 | Cache-Control no-store 中间件 | 统一避免 API 身份响应被缓存 |
| 5 | express-session | 后续才能读 req.session |
| 6 | `/api` 中间件：写请求 Origin/CSRF 校验、按 userId 装载 account | 没有身份装载，权限中间件无可靠依据 |
| 7 | 业务路由及其限流、权限中间件 | 按路径和方法执行具体业务 |
| 8 | 未匹配接口的 404 | 放在业务路由前会拦住所有接口 |
| 9 | 四参数错误处理中间件 | 统一将异常变成 JSON |

这里 Express 中间件有三种出口：成功处理并发送响应；调用 next() 继续；调用 next(error) 或抛错进入错误链。什么都不做会让请求悬挂。[Express 官方中间件文档](https://expressjs.com/en/guide/writing-middleware/)说明了这套请求处理约定及 Express 5 的 Promise 错误转发行为。

## 4. TypeScript 在 Express 中具体写在哪里

`declare module "express-session"` 扩展 SessionData，声明 userId? 与 csrf?。`declare global { namespace Express { interface Request { account?: User } } }` 扩展请求对象。这些只让编译器认识属性；真正赋值的是 Session 中间件和查询用户的 await 代码。

权限函数签名 `(req: Request, res: Response, next: NextFunction)` 明确三个参数类型。许多路由回调没有再写一遍类型，是因为 `app.get` 等方法已经为回调提供上下文类型推断，不代表它们不受 TypeScript 检查。

`credentials.extend(...).strict().refine(...).parse(req.body)`：extend 添加字段；strict 拒绝多余字段（例如 role）；refine 检查跨字段规则；parse 成功返回校验后的值，失败抛 ZodError。trim、toLowerCase 还会规范化邮箱，所以后续使用 input.email，而不是原始 req.body.email。

## 5. requireUser 与 requireAdmin 逐步执行

requireUser：没有 req.account 就 401 并 return；有账号就 next。requireAdmin：先判登录（401），再判 role（403），满足才 next。路由写成 `app.post(path, requireAdmin, async handler)` 时，这些函数顺序运行。

请求身份每次按 Session 的 userId 查询数据库，所以后台修改角色后，下一次请求可以读到新角色。前端导航条件只能隐藏按钮；真正防止普通用户增删改的是 requireAdmin。

## 6. async 错误和回调错误为何两种写法

当前 Express 5 对路由返回 Promise 的拒绝和 async 抛错会进入错误中间件。但 `session.destroy(callback)` 的异步回调不是外层路由返回的 Promise，所以退出登录显式写 `if (e) return next(e)`。不要机械地认为“用了 Express 5 就任何回调异常都自动捕获”。

最后的错误方法固定四参数 `(error, req, res, next)`：RegistrationError 使用业务状态码；ZodError 返回400和字段问题；PostgreSQL唯一约束23505返回409；格式错误/超体积返回400/413；未知错误返回500且不回传敏感详情。GET health 的 `SELECT 1` 失败也经过这条错误链。

`close()` 关闭 Session Store 定时器与 pg Pool；主入口另外销毁 TypeORM DataSource。它不是每次请求调用的方法，而是进程退出与测试清理时调用。

## 对照源码：server/app.ts（第 1–160 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: server/app.ts start: 1 end: 160 sha256: 18d6a57064fc355627fead287a4b15641b846a020840690637fded9b723a5c1f -->

{% raw %}
````ts
import express, {
  type Request,
  type Response,
  type NextFunction,
} from "express";
import session from "express-session";
import connectPgSimple from "connect-pg-simple";
import { Pool } from "pg";
import { randomBytes, timingSafeEqual } from "node:crypto";
import bcrypt from "bcryptjs";
import helmet from "helmet";
import { rateLimit } from "express-rate-limit";
import { z } from "zod";
import type { DataSource } from "typeorm";
import { createRegistration, RegistrationError } from "./registration";
import type { RegistrationMailer } from "./mail";
import {
  UserEntity,
  ArticleEntity,
  CommentEntity,
  type User,
} from "./entities";
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
const credentials = z.object({
  email: z.string().trim().toLowerCase().email().max(255),
  password: z
    .string()
    .min(10)
    .max(72)
    .refine((s) => Buffer.byteLength(s, "utf8") <= 72, "密码不能超过 72 字节"),
});
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
const uuid = z.string().uuid();
const publicUser = (u: User) => ({
  id: u.id,
  username: u.username,
  email: u.email,
  role: u.role,
});
export function createApp(
  db: DataSource,
  options: {
    secret: string;
    origin: string;
    production?: boolean;
    pool?: Pool;
    mailer?: RegistrationMailer;
    now?: () => Date;
  },
) {
  if (options.secret.length < 32)
    throw new Error("SESSION_SECRET 至少需要 32 个字符");
  const app = express();
  const registration = createRegistration(db, options.secret, options.mailer, options.now);
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
  app.use("/api", async (req, res, next) => {
    if (!["GET", "HEAD", "OPTIONS"].includes(req.method)) {
      if (req.get("origin") !== options.origin) {
        res.status(403).json({ message: "请求来源不被允许" });
        return;
      }
      const token = req.get("x-csrf-token") ?? "",
        expected = req.session.csrf ?? "";
      if (
        !expected ||
        Buffer.byteLength(token) !== Buffer.byteLength(expected) ||
        !timingSafeEqual(Buffer.from(token), Buffer.from(expected))
      ) {
        res.status(403).json({ message: "页面验证已过期，请刷新后重试" });
        return;
      }
    }
    if (req.session.userId)
      req.account =
        (await db
          .getRepository(UserEntity)
          .findOneBy({ id: req.session.userId })) ?? undefined;
    next();
  });
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
  const authLimit = rateLimit({
    windowMs: 15 * 60 * 1000,
    limit: 30,
    standardHeaders: "draft-7",
    legacyHeaders: false,
    message: { message: "尝试过于频繁，请稍后重试" },
  });
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-04-styles %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %})
