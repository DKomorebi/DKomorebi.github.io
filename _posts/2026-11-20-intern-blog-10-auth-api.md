---
layout: post
title: NextBlog（十）：Express、Cookie、Session 与权限边界
date: 2026-11-20 00:10:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 完整 API 代码，解释认证、CSRF、文章权限和评论归属。
keywords: Next.js, TypeScript, Express, PostgreSQL, DaisyUI, 全栈博客
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
last_modified_at: 2026-11-20 07:31:36 +0800
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


完整 API 代码，解释认证、CSRF、文章权限和评论归属。

> 本文是 NextBlog 全栈博客系列第 10 / 15 篇，内容对应当前全栈版本。

## 系列导航

1. [NextBlog（一）：全栈项目搭建与 React 入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [NextBlog（二）：Next.js 路由、布局与前后端边界]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [NextBlog（三）：用真实业务理解 TypeScript]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [NextBlog（四）：Tailwind CSS、DaisyUI 与简约页面]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [NextBlog（五）：React 状态、搜索请求与表单交互]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [NextBlog（六）：Express、TypeORM 与真实数据库请求]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [NextBlog（七）：运行验收、排错与生产边界]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [NextBlog（八）：串起登录、文章编辑与评论业务]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})
9. [NextBlog（九）：PostgreSQL 数据建模与迁移]({% post_url 2026-11-20-intern-blog-09-database %})
10. [NextBlog（十）：Express、Cookie、Session 与权限边界]({% post_url 2026-11-20-intern-blog-10-auth-api %})
11. [NextBlog（十一）：全栈路由与每个页面完整源码]({% post_url 2026-11-20-intern-blog-11-pages %})
12. [NextBlog（十二）：搜索、编辑器、登录与评论组件详解]({% post_url 2026-11-20-intern-blog-12-components %})
13. [NextBlog（十三）：DaisyUI 卡片、Lucide 图标与深色主题]({% post_url 2026-11-20-intern-blog-13-theme %})
14. [NextBlog（十四）：接口测试、运行配置与发布检查]({% post_url 2026-11-20-intern-blog-14-tests %})
15. [NextBlog（十五）：项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})

## 身份与权限是两次不同的判断

requireUser 判断当前请求是否对应一个已登录用户；requireAdmin 在此基础上判断数据库中的 role 是否为 admin。前端隐藏按钮只改善体验，不能替代这两个中间件。普通用户直接调用 POST、PUT、DELETE 文章接口仍然收到 403；没有登录则返回 401。

Session 只保存 userId 和 CSRF token，每次请求再查用户角色。因此服务端角色变化会在下一次请求生效，用户无法通过修改浏览器 localStorage 或请求中的 role 提权。注册使用严格 schema，额外 role 字段会被拒绝。

## Cookie 与 Session 的分工

Cookie fieldnotes.sid 保存签名后的会话标识，用户数据在 PostgreSQL sessions 表。HttpOnly 限制脚本读取 Cookie，SameSite=Lax 限制部分跨站携带，生产 Secure 要求 HTTPS。七天滚动有效期并不是永久登录。登录和注册调用 regenerate 更换会话 ID，退出调用 destroy 并清除 Cookie；服务重启依然可以从数据库恢复会话。

## 为什么还需要 CSRF

浏览器会自动携带 Cookie，攻击者可能诱导用户发起写请求。项目同时校验 Origin 和 X-CSRF-Token，注册、登录、退出也受保护。前端先请求 /api/auth/session 获取 token，登录后使用新返回的 token。APP_ORIGIN 的主机和端口必须与页面地址完全一致。

## 密码与错误响应

密码经 bcrypt 哈希存储，成本参数 12；最大 72 字节用于避免 bcrypt 的截断行为。错误密码统一返回邮箱或密码错误，不区分账号是否存在。接口使用 Zod 校验输入，数据库唯一冲突返回 409，异常只向前端返回通用提示，不输出连接密码或堆栈。

## 文章与评论规则

列表搜索标题、摘要与正文，每页 10 篇，SQL 使用参数绑定；编辑要求 version，通过原子条件更新避免覆盖另一个编辑者的修改。评论每页 30 条，用户 ID 从 Session 取值，普通用户只能删除自己的评论，管理员可以删除任何评论。删除文章由数据库级联清理评论。

生产入口显式设置 NODE_ENV，开启 Secure Cookie。Express 只监听回环地址，通过受控代理访问。当前限流存储在单进程内存，多实例部署需改共享存储；生产 HTTPS 代理需要单独验收。

## 练习

用两个不同账号访问同一篇文章，尝试删除对方评论；比较页面显示与实际 HTTP 状态码。再用两个编辑页面保存同一篇文章，观察第二次保存的版本冲突提示。


## 本篇文件索引

- [server/app.ts](#source-1)
- [server/index.ts](#source-2)
- [server/production.ts](#source-3)

<a id="source-1"></a>

### server/app.ts

以下为该文件的完整源码。

{% raw %}
```typescript
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
  },
) {
  if (options.secret.length < 32)
    throw new Error("SESSION_SECRET 至少需要 32 个字符");
  const app = express();
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
  app.get("/api/health", async (_req, res) => {
    await db.query("SELECT 1");
    res.json({ ok: true });
  });
  app.get("/api/auth/session", (req, res) => {
    req.session.csrf ??= randomBytes(32).toString("hex");
    res.json({
      user: req.account ? publicUser(req.account) : null,
      csrf: req.session.csrf,
    });
  });
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
  app.post("/api/auth/register", authLimit, async (req, res) => {
    const input = credentials
      .extend({ username: z.string().trim().min(2).max(40) })
      .strict()
      .parse(req.body);
    const repo = db.getRepository(UserEntity);
    const user = await repo.save(
      repo.create({
        email: input.email,
        username: input.username,
        passwordHash: await bcrypt.hash(input.password, 12),
        role: "user",
      }),
    );
    await loginSession(req, user);
    res.status(201).json({ user: publicUser(user), csrf: req.session.csrf });
  });
  app.post("/api/auth/login", authLimit, async (req, res) => {
    const input = credentials.strict().parse(req.body);
    const user = await db
      .getRepository(UserEntity)
      .findOneBy({ email: input.email });
    const valid = await bcrypt.compare(
      input.password,
      user?.passwordHash ??
        "$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxlS2MWV.y1WHJt9HmPj7G3ON1e",
    );
    if (!user || !valid) {
      res.status(401).json({ message: "邮箱或密码错误" });
      return;
    }
    await loginSession(req, user);
    res.json({ user: publicUser(user), csrf: req.session.csrf });
  });
  app.post("/api/auth/logout", (req, res, next) => {
    req.session.destroy((e) => {
      if (e) return next(e);
      res.clearCookie("fieldnotes.sid", {
        httpOnly: true,
        sameSite: "lax",
        secure: !!options.production,
        path: "/",
      });
      res.status(204).end();
    });
  });
  app.get("/api/articles", async (req, res) => {
    const input = z
      .object({
        q: z.string().max(100).default(""),
        category: z.string().max(40).default(""),
        page: z.coerce.number().int().min(1).default(1),
      })
      .parse(req.query);
    const qb = db.getRepository(ArticleEntity).createQueryBuilder("a");
    if (input.q)
      qb.andWhere(
        "(a.title ILIKE :q OR a.excerpt ILIKE :q OR a.content ILIKE :q)",
        { q: `%${input.q.replace(/[\\%_]/g, "\\$&")}%` },
      );
    if (input.category)
      qb.andWhere("a.category = :category", { category: input.category });
    qb.select([
      "a.id",
      "a.slug",
      "a.title",
      "a.excerpt",
      "a.category",
      "a.createdAt",
      "a.updatedAt",
      "a.version",
    ]);
    const [items, total] = await qb
      .orderBy("a.createdAt", "DESC")
      .addOrderBy("a.id", "ASC")
      .skip((input.page - 1) * 10)
      .take(10)
      .getManyAndCount();
    res.json({ items, total, page: input.page, pageSize: 10 });
  });
  app.get("/api/categories", async (_req, res) => {
    res.json(
      await db
        .getRepository(ArticleEntity)
        .createQueryBuilder("a")
        .select("a.category", "name")
        .addSelect("COUNT(*)::int", "count")
        .groupBy("a.category")
        .orderBy("a.category")
        .getRawMany(),
    );
  });
  app.get("/api/articles/:slug", async (req, res) => {
    const article = await db
      .getRepository(ArticleEntity)
      .findOneBy({ slug: String(req.params.slug) });
    if (!article) {
      res.status(404).json({ message: "文章不存在" });
      return;
    }
    res.json(article);
  });
  app.post("/api/articles", requireAdmin, async (req, res) => {
    const input = articleInput.parse(req.body);
    res.status(201).json(await db.getRepository(ArticleEntity).save(input));
  });
  app.put("/api/articles/:id", requireAdmin, async (req, res) => {
    const id = uuid.parse(req.params.id);
    const { version, ...input } = articleInput
      .extend({ version: z.number().int().positive() })
      .parse(req.body);
    const result = await db
      .getRepository(ArticleEntity)
      .createQueryBuilder()
      .update()
      .set({ ...input, version: () => "version + 1" })
      .where("id = :id AND version = :version", { id, version })
      .returning("*")
      .execute();
    if (!result.affected) {
      res.status(409).json({ message: "文章已被修改或删除，请重新加载后编辑" });
      return;
    }
    res.json(result.raw[0]);
  });
  app.delete("/api/articles/:id", requireAdmin, async (req, res) => {
    const result = await db
      .getRepository(ArticleEntity)
      .delete(uuid.parse(req.params.id));
    if (!result.affected) {
      res.status(404).json({ message: "文章不存在" });
      return;
    }
    res.status(204).end();
  });
  app.get("/api/articles/:id/comments", async (req, res) => {
    const id = uuid.parse(req.params.id);
    const page = z.coerce
      .number()
      .int()
      .min(1)
      .default(1)
      .parse(req.query.page);
    const [items, total] = await db
      .getRepository(CommentEntity)
      .createQueryBuilder("c")
      .where("c.articleId = :id", { id })
      .orderBy("c.createdAt", "DESC")
      .skip((page - 1) * 30)
      .take(30)
      .getManyAndCount();
    const comments = await Promise.all(
      items.map(async (c) => ({
        ...c,
        username:
          (await db.getRepository(UserEntity).findOneBy({ id: c.userId }))
            ?.username ?? "已注销用户",
      })),
    );
    res.json({ items: comments, total, page, pageSize: 30 });
  });
  app.post(
    "/api/articles/:id/comments",
    requireUser,
    rateLimit({
      windowMs: 60000,
      limit: 10,
      message: { message: "评论过于频繁，请稍后重试" },
    }),
    async (req, res) => {
      const articleId = uuid.parse(req.params.id);
      const { content } = z
        .object({ content: z.string().trim().min(1).max(2000) })
        .strict()
        .parse(req.body);
      if (
        !(await db.getRepository(ArticleEntity).existsBy({ id: articleId }))
      ) {
        res.status(404).json({ message: "文章不存在" });
        return;
      }
      res
        .status(201)
        .json(
          await db
            .getRepository(CommentEntity)
            .save({ articleId, userId: req.account!.id, content }),
        );
    },
  );
  app.delete("/api/comments/:id", requireUser, async (req, res) => {
    const repo = db.getRepository(CommentEntity);
    const comment = await repo.findOneBy({ id: uuid.parse(req.params.id) });
    if (!comment) {
      res.status(404).json({ message: "评论不存在" });
      return;
    }
    if (req.account!.role !== "admin" && comment.userId !== req.account!.id) {
      res.status(403).json({ message: "只能删除自己的评论" });
      return;
    }
    await repo.delete(comment.id);
    res.status(204).end();
  });
  app.use((_req, res) => res.status(404).json({ message: "接口不存在" }));
  app.use(
    (error: unknown, _req: Request, res: Response, _next: NextFunction) => {
      if (error instanceof z.ZodError) {
        res
          .status(400)
          .json({
            message: "输入不符合要求",
            details: error.issues.map(
              (i) => `${i.path.join(".")}: ${i.message}`,
            ),
          });
        return;
      }
      const e = error as { code?: string; status?: number };
      if (e.code === "23505") {
        res.status(409).json({ message: "邮箱或文章标识已存在" });
        return;
      }
      if (e.status === 400 || e.status === 413) {
        res.status(e.status).json({ message: "请求格式错误或内容过大" });
        return;
      }
      console.error("API request failed", e.code ?? "internal");
      res.status(500).json({ message: "服务暂时不可用，请稍后重试" });
    },
  );
  return {
    app,
    close: async () => {
      store.close();
      await pool.end();
    },
  };
}
```
{% endraw %}

<a id="source-2"></a>

### server/index.ts

以下为该文件的完整源码。

{% raw %}
```typescript
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
```
{% endraw %}

<a id="source-3"></a>

### server/production.ts

以下为该文件的完整源码。

{% raw %}
```typescript
Object.assign(process.env, { NODE_ENV: 'production' });
void import('./index');
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-09-database %})

[下一篇]({% post_url 2026-11-20-intern-blog-11-pages %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
