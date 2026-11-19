---
layout: post
title: NextBlog（十五）：项目结构与完整源码汇总
date: 2026-11-20 00:15:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 集中查看项目结构、文件职责、业务调用链与全部页面、组件、后端和配置源码。
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

集中查看项目结构、文件职责、业务调用链与全部页面、组件、后端和配置源码。

> 本文是 NextBlog 全栈博客系列第 15 / 15 篇，内容对应当前全栈版本。

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

## 本篇怎样使用

这是当前全栈版本的统一索引与完整源码快照。前八篇从基础解释业务，第九至十四篇按模块深入；需要快速定位文件时从本篇进入。下面同时提供文件职责、模块调用关系与逐文件完整代码，避免拿到不同阶段的页面和后端混用。

源码基准为 [NextBlog 提交 e0f8b52](https://github.com/DKomorebi/NextBlog/commit/e0f8b5239b13ca8efeb13a88c87bb42ecc10a8e0)。后续仓库可能继续变化，本篇保留该版本代码；安装必须同时使用该提交的 package.json 与 pnpm-lock.yaml。真实 .env、依赖目录、构建产物和数据库内容不属于可公开复制的源码。

## 项目结构


{% raw %}
```text
NextBlog/
├── docs/
├── server/
│   ├── app.test.ts
│   ├── app.ts
│   ├── db.ts
│   ├── entities.ts
│   ├── index.ts
│   ├── manage.ts
│   ├── migration.ts
│   └── production.ts
├── src/
│   ├── app/
│   │   ├── about/
│   │   │   └── page.tsx
│   │   ├── admin/
│   │   │   ├── [id]/
│   │   │   │   └── page.tsx
│   │   │   ├── new/
│   │   │   │   └── page.tsx
│   │   │   └── page.tsx
│   │   ├── archive/
│   │   │   └── page.tsx
│   │   ├── login/
│   │   │   └── page.tsx
│   │   ├── posts/
│   │   │   └── [slug]/
│   │   │       └── page.tsx
│   │   ├── register/
│   │   │   └── page.tsx
│   │   ├── globals.css
│   │   ├── layout.tsx
│   │   ├── not-found.tsx
│   │   └── page.tsx
│   ├── components/
│   │   ├── full-blog.tsx
│   │   ├── home-hero.tsx
│   │   └── theme-toggle.tsx
│   ├── lib/
│   │   ├── posts.ts
│   │   └── theme.ts
│   └── types/
│       └── post.ts
├── .env  # 仅本地配置，不提交
├── .env.example
├── .gitignore
├── .npmrc
├── README.md
├── compose.yaml
├── next-env.d.ts
├── next.config.ts
├── package.json
├── pnpm-lock.yaml
├── pnpm-workspace.yaml
├── postcss.config.mjs
└── tsconfig.json
```
{% endraw %}

src/app 只组织页面和公共布局，src/components 承担浏览器交互，server 处理数据库与 HTTP 业务。src/lib/posts.ts 和 src/types/post.ts 是 seed 的数据与类型，不能把它们当成运行时文章存储。完整源码中不存在旧的封面或静态搜索组件。

## 按业务读代码

| 业务 | 页面入口 | 前端组件 | 后端入口 |
| --- | --- | --- | --- |
| 首页、分类、搜索、分页 | /、/archive | ArticleList | GET /api/articles、/api/categories |
| 文章阅读 | /posts/[slug] | ArticleReader、Markdown | GET /api/articles/:slug |
| 登录注册与退出 | /login、/register、Header | SessionProvider、AuthForm | /api/auth/* |
| 文章新建与编辑 | /admin/new、/admin/[id] | Editor | POST /api/articles、PUT /api/articles/:id |
| 文章删除 | /admin | ArticleList manage | DELETE /api/articles/:id |
| 评论与归属检查 | 文章阅读页 | Comments | /api/articles/:id/comments、/api/comments/:id |
| 主题 | 公共布局与页眉 | ThemeToggle | 无后端接口，主题存于 localStorage |

所有 API 都先经过 Next.js 的 rewrite，再由 Express 执行会话、来源、输入和权限检查。文章与评论进入 PostgreSQL；主题偏好保留在浏览器。这样可以区分哪些数据只影响外观，哪些行为必须被服务器授权。

## 文件职责与源码跳转

| 文件 | 职责 | 完整源码 |
| --- | --- | --- |
| `server/app.test.ts` | 真实 PostgreSQL 集成测试与临时数据库清理 | [展开阅读](#file-1) |
| `server/app.ts` | Express 中间件、会话、权限、认证、文章与评论 API | [展开阅读](#file-2) |
| `server/db.ts` | 环境变量、DataSource 与迁移注册 | [展开阅读](#file-3) |
| `server/entities.ts` | 用户、文章和评论的实体映射 | [展开阅读](#file-4) |
| `server/index.ts` | 开发 API 启动、监听与退出清理 | [展开阅读](#file-5) |
| `server/manage.ts` | 迁移、seed 与管理员授权命令 | [展开阅读](#file-6) |
| `server/migration.ts` | 业务表、会话表、约束与索引 | [展开阅读](#file-7) |
| `server/production.ts` | 设置生产模式并启动 API | [展开阅读](#file-8) |
| `src/app/about/page.tsx` | 关于页面 | [展开阅读](#file-9) |
| `src/app/admin/[id]/page.tsx` | 编辑文章动态路由与 Suspense 边界 | [展开阅读](#file-10) |
| `src/app/admin/new/page.tsx` | 新建文章与 Suspense 边界 | [展开阅读](#file-11) |
| `src/app/admin/page.tsx` | 管理员文章列表 | [展开阅读](#file-12) |
| `src/app/archive/page.tsx` | 归档路由，复用文章列表 | [展开阅读](#file-13) |
| `src/app/globals.css` | Tailwind、DaisyUI 主题、卡片和响应式布局 | [展开阅读](#file-14) |
| `src/app/layout.tsx` | 公共布局、会话 Provider、页眉和页脚 | [展开阅读](#file-15) |
| `src/app/login/page.tsx` | 登录路由，复用 AuthForm | [展开阅读](#file-16) |
| `src/app/not-found.tsx` | Next.js 未找到路由时的页面 | [展开阅读](#file-17) |
| `src/app/page.tsx` | 首页路由，渲染文章列表 | [展开阅读](#file-18) |
| `src/app/posts/[slug]/page.tsx` | 按 slug 打开阅读组件 | [展开阅读](#file-19) |
| `src/app/register/page.tsx` | 注册路由，复用 AuthForm | [展开阅读](#file-20) |
| `src/components/full-blog.tsx` | api、Context、导航、列表、登录、阅读、评论与编辑 | [展开阅读](#file-21) |
| `src/components/home-hero.tsx` | 首页 Hero 文案与阅读锚点 | [展开阅读](#file-22) |
| `src/components/theme-toggle.tsx` | 主题偏好同步与圆形切换动画 | [展开阅读](#file-23) |
| `src/lib/posts.ts` | 仅供 seed 导入的示例文章 | [展开阅读](#file-24) |
| `src/lib/theme.ts` | 主题常量与首帧初始化脚本 | [展开阅读](#file-25) |
| `src/types/post.ts` | seed 示例文章的结构类型 | [展开阅读](#file-26) |
| `package.json` | 依赖与运行、检查、数据库管理命令 | [展开阅读](#file-27) |
| `tsconfig.json` | TypeScript 严格检查、模块解析与路径别名 | [展开阅读](#file-28) |
| `next.config.ts` | 构建目录与 Express 同源代理 | [展开阅读](#file-29) |
| `next-env.d.ts` | Next.js 生成的类型入口；通常由框架维护 | [展开阅读](#file-30) |
| `postcss.config.mjs` | Tailwind PostCSS 插件 | [展开阅读](#file-31) |
| `pnpm-workspace.yaml` | pnpm 原生构建依赖配置 | [展开阅读](#file-32) |
| `.npmrc` | 依赖仓库地址 | [展开阅读](#file-33) |
| `.gitignore` | 本地密钥、依赖、构建产物与导出副本忽略规则 | [展开阅读](#file-34) |
| `.env.example` | 环境变量占位示例，无真实凭据 | [展开阅读](#file-35) |
| `compose.yaml` | 可选的 PostgreSQL 18 容器与持久卷 | [展开阅读](#file-36) |

锁文件由包管理器生成，避免在文章中重复数千行依赖元数据：[pnpm-lock.yaml 完整文件](https://github.com/DKomorebi/NextBlog/blob/e0f8b5239b13ca8efeb13a88c87bb42ecc10a8e0/pnpm-lock.yaml)。启动说明见 [README.md](https://github.com/DKomorebi/NextBlog/blob/e0f8b5239b13ca8efeb13a88c87bb42ecc10a8e0/README.md)。以下 36 个源代码与配置文件均完整收录，不以省略号替代业务逻辑。

## 建议阅读顺序与修改入口

先读 package.json、next.config.ts、layout.tsx 和首页，理解两个服务的边界；再读 full-blog.tsx 中的类型、api 与 SessionProvider；随后沿 ArticleList 和 Editor 找到 server/app.ts 对应路由；最后读 entities、migration 与测试。

修改页面外观主要看 globals.css、home-hero.tsx、theme-toggle.tsx；新增文章字段会同时影响迁移、实体、接口校验和前端类型/表单。修改角色规则必须先调整后端权限和测试，再调整按钮显示。当前 full-blog.tsx 集中承载业务，后续可按认证、文章、评论拆分，拆分时保持 Context 与请求封装的共同入口。

恢复运行时，先按第一篇安装依赖并创建本地数据库，再配置 .env、执行迁移、可选导入 seed，然后 pnpm dev。不要用这篇文章覆盖自己的 .env，也不要将生产数据库当作测试数据库。

## 完整源码

<a id="file-1"></a>

### server/app.test.ts

真实 PostgreSQL 集成测试与临时数据库清理。

代码位置：`server/app.test.ts`（完整文件）。

{% raw %}
```typescript
import { test } from "node:test";
import assert from "node:assert/strict";
import { randomBytes } from "node:crypto";
import { Client, Pool } from "pg";
import request from "supertest";
import { createDatabase } from "./db";
import { createApp } from "./app";
import { UserEntity } from "./entities";

test("PostgreSQL integration: authentication, session, CSRF, RBAC, CRUD and comments", async () => {
  const base = new URL(process.env.DATABASE_URL!);
  const database = "fieldnotes_test_" + randomBytes(6).toString("hex");
  const adminUrl = new URL(base);
  adminUrl.pathname = "/postgres";
  const controller = new Client({ connectionString: adminUrl.toString() });
  await controller.connect();
  await controller.query(`CREATE DATABASE "${database}"`);
  base.pathname = "/" + database;
  const db = await createDatabase(base.toString()).initialize();
  await db.runMigrations();
  const origin = "http://localhost:3000";
  const secret = randomBytes(48).toString("hex");
  let service = createApp(db, {
    secret,
    origin,
    pool: new Pool({ connectionString: base.toString() }),
  });
  try {
    const anonymous = request.agent(service.app);
    let r = await anonymous.get("/api/auth/session").expect(200);
    const csrf = r.body.csrf;
    assert.match(r.headers["set-cookie"][0], /HttpOnly/);
    assert.match(r.headers["set-cookie"][0], /SameSite=Lax/);
    const payload = {
      title: "测试文章",
      slug: "integration-post",
      category: "测试",
      excerpt: "摘要",
      content: "## Markdown\n\n**hello**",
    };
    await anonymous
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", csrf)
      .send(payload)
      .expect(401);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", origin)
      .send({})
      .expect(403);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", "https://evil.example")
      .set("X-CSRF-Token", csrf)
      .send({})
      .expect(403);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", csrf)
      .send({
        email: "bad@example.com",
        username: "bad",
        password: "Testing-password-123",
        role: "admin",
      })
      .expect(400);
    const user = request.agent(service.app);
    r = await user.get("/api/auth/session");
    const before = r.headers["set-cookie"][0].split(";")[0];
    const init = r.body.csrf;
    r = await user
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", init)
      .send({
        email: "reader@example.com",
        username: "读者",
        password: "Testing-password-123",
      })
      .expect(201);
    let token = r.body.csrf;
    assert.equal(r.body.user.role, "user");
    assert.notEqual(before, r.headers["set-cookie"][0].split(";")[0]);
    assert.equal(r.body.user.passwordHash, undefined);
    await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(403);
    await db
      .getRepository(UserEntity)
      .update({ email: "reader@example.com" }, { role: "admin" });
    r = await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(201);
    const article = r.body;
    assert.ok(article.id);
    await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(409);
    r = await anonymous.get("/api/articles?q=Markdown").expect(200);
    assert.equal(r.body.total, 1);
    assert.equal(r.body.items[0].content, undefined);
    await user
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ ...payload, title: "已修改", version: 1 })
      .expect(200);
    await user
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ ...payload, version: 1 })
      .expect(409);
    const other = request.agent(service.app);
    const otherCsrf = (await other.get("/api/auth/session")).body.csrf;
    r = await other
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", otherCsrf)
      .send({
        email: "other@example.com",
        username: "另一位读者",
        password: "Testing-password-456",
      })
      .expect(201);
    const otherToken = r.body.csrf;
    await other
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .send({ ...payload, version: 2 })
      .expect(403);
    await other
      .delete("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(403);
    r = await user
      .post(`/api/articles/${article.id}/comments`)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ content: "管理员留言" })
      .expect(201);
    const commentId = r.body.id;
    await other
      .delete("/api/comments/" + commentId)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(403);
    r = await other
      .post(`/api/articles/${article.id}/comments`)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .send({ content: "普通用户留言" })
      .expect(201);
    await other
      .delete("/api/comments/" + r.body.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(204);
    r = await user.get("/api/auth/session");
    const savedCookie = r.headers["set-cookie"][0].split(";")[0];
    await service.close();
    service = createApp(db, {
      secret,
      origin,
      pool: new Pool({ connectionString: base.toString() }),
    });
    r = await request(service.app)
      .get("/api/auth/session")
      .set("Cookie", savedCookie)
      .expect(200);
    assert.equal(r.body.user.email, "reader@example.com");
    token = r.body.csrf;
    await request(service.app)
      .delete("/api/articles/" + article.id)
      .set("Cookie", savedCookie)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .expect(204);
    assert.equal(
      (await request(service.app).get(`/api/articles/${article.id}/comments`))
        .body.total,
      0,
    );
    await request(service.app)
      .get("/api/articles/integration-post")
      .expect(404);
    await request(service.app)
      .post("/api/auth/logout")
      .set("Cookie", savedCookie)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .expect(204);
    assert.equal(
      (
        await request(service.app)
          .get("/api/auth/session")
          .set("Cookie", savedCookie)
      ).body.user,
      null,
    );
    const login = request.agent(service.app);
    const loginCsrf = (await login.get("/api/auth/session")).body.csrf;
    await login
      .post("/api/auth/login")
      .set("Origin", origin)
      .set("X-CSRF-Token", loginCsrf)
      .send({ email: "reader@example.com", password: "wrong-password" })
      .expect(401);
    await login
      .post("/api/auth/login")
      .set("Origin", origin)
      .set("X-CSRF-Token", loginCsrf)
      .send({ email: "reader@example.com", password: "Testing-password-123" })
      .expect(200);
  } finally {
    await service.close();
    await db.destroy();
    await controller.query(`DROP DATABASE "${database}"`);
    await controller.end();
  }
});
```
{% endraw %}

<a id="file-2"></a>

### server/app.ts

Express 中间件、会话、权限、认证、文章与评论 API。

代码位置：`server/app.ts`（完整文件）。

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

<a id="file-3"></a>

### server/db.ts

环境变量、DataSource 与迁移注册。

代码位置：`server/db.ts`（完整文件）。

{% raw %}
```typescript
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
```
{% endraw %}

<a id="file-4"></a>

### server/entities.ts

用户、文章和评论的实体映射。

代码位置：`server/entities.ts`（完整文件）。

{% raw %}
```typescript
import { EntitySchema } from "typeorm";
export interface User {
  id: string;
  username: string;
  email: string;
  passwordHash: string;
  role: "user" | "admin";
  createdAt: Date;
}
export interface Article {
  id: string;
  slug: string;
  title: string;
  excerpt: string;
  content: string;
  category: string;
  createdAt: Date;
  updatedAt: Date;
  version: number;
}
export interface Comment {
  id: string;
  articleId: string;
  userId: string;
  content: string;
  createdAt: Date;
}
const id = { type: "uuid" as const, primary: true, generated: "uuid" as const };
export const UserEntity = new EntitySchema<User>({
  name: "User",
  tableName: "users",
  columns: {
    id,
    username: { type: String, length: 40 },
    email: { type: String, unique: true },
    passwordHash: { type: String },
    role: { type: String, default: "user" },
    createdAt: { type: "timestamptz", createDate: true },
  },
});
export const ArticleEntity = new EntitySchema<Article>({
  name: "Article",
  tableName: "articles",
  columns: {
    id,
    slug: { type: String, unique: true },
    title: { type: String },
    excerpt: { type: String },
    content: { type: "text" },
    category: { type: String },
    createdAt: { type: "timestamptz", createDate: true },
    updatedAt: { type: "timestamptz", updateDate: true },
    version: { type: Number, default: 1 },
  },
});
export const CommentEntity = new EntitySchema<Comment>({
  name: "Comment",
  tableName: "comments",
  columns: {
    id,
    articleId: { type: "uuid" },
    userId: { type: "uuid" },
    content: { type: "text" },
    createdAt: { type: "timestamptz", createDate: true },
  },
});
```
{% endraw %}

<a id="file-5"></a>

### server/index.ts

开发 API 启动、监听与退出清理。

代码位置：`server/index.ts`（完整文件）。

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

<a id="file-6"></a>

### server/manage.ts

迁移、seed 与管理员授权命令。

代码位置：`server/manage.ts`（完整文件）。

{% raw %}
```typescript
import { createDatabase } from "./db";
import { UserEntity, ArticleEntity } from "./entities";
import { posts } from "../src/lib/posts";
async function main() {
  const db = await createDatabase().initialize();
  try {
    const command = process.argv[2];
    if (command === "migrate") {
      await db.runMigrations();
      console.log("Migrations applied");
    } else if (command === "admin") {
      const email = process.argv[3]?.trim().toLowerCase();
      if (!email)
        throw new Error("用法：pnpm admin 用户邮箱（该用户须先注册）");
      const result = await db
        .getRepository(UserEntity)
        .update({ email }, { role: "admin" });
      if (!result.affected) throw new Error("该邮箱未注册");
      console.log("Admin role granted");
    } else if (command === "seed") {
      const repo = db.getRepository(ArticleEntity);
      for (const p of posts) {
        if (await repo.existsBy({ slug: p.slug })) continue;
        await repo.save({
          slug: p.slug,
          title: p.title,
          excerpt: p.excerpt,
          category: p.category,
          content: p.sections
            .map(
              (s) =>
                `## ${s.heading}\n\n${s.paragraphs.join("\n\n")}${s.code ? "\n\n```tsx\n" + s.code + "\n```" : ""}`,
            )
            .join("\n\n"),
        });
      }
      console.log("Sample articles imported");
    } else throw new Error("使用 migrate、admin 或 seed");
  } finally {
    await db.destroy();
  }
}
main().catch((e) => {
  console.error(e.message);
  process.exit(1);
});
```
{% endraw %}

<a id="file-7"></a>

### server/migration.ts

业务表、会话表、约束与索引。

代码位置：`server/migration.ts`（完整文件）。

{% raw %}
```typescript
import type { MigrationInterface, QueryRunner } from "typeorm";
export class Initial1795000000000 implements MigrationInterface {
  async up(q: QueryRunner) {
    await q.query(`CREATE TABLE users (id uuid PRIMARY KEY DEFAULT gen_random_uuid(), username varchar(40) NOT NULL, email varchar(255) NOT NULL UNIQUE, "passwordHash" varchar(255) NOT NULL, role varchar(10) NOT NULL DEFAULT 'user' CHECK (role IN ('user','admin')), "createdAt" timestamptz NOT NULL DEFAULT now());
CREATE TABLE articles (id uuid PRIMARY KEY DEFAULT gen_random_uuid(), slug varchar(120) NOT NULL UNIQUE, title varchar(160) NOT NULL, excerpt varchar(500) NOT NULL, content text NOT NULL, category varchar(40) NOT NULL, "createdAt" timestamptz NOT NULL DEFAULT now(), "updatedAt" timestamptz NOT NULL DEFAULT now(), version integer NOT NULL DEFAULT 1);
CREATE TABLE comments (id uuid PRIMARY KEY DEFAULT gen_random_uuid(), "articleId" uuid NOT NULL REFERENCES articles(id) ON DELETE CASCADE, "userId" uuid NOT NULL REFERENCES users(id) ON DELETE CASCADE, content text NOT NULL, "createdAt" timestamptz NOT NULL DEFAULT now());
CREATE INDEX comments_article_idx ON comments("articleId", "createdAt");
CREATE TABLE sessions (sid varchar NOT NULL PRIMARY KEY, sess json NOT NULL, expire timestamp(6) NOT NULL);
CREATE INDEX sessions_expire_idx ON sessions(expire);`);
  }
  async down(q: QueryRunner) {
    await q.query(
      "DROP TABLE sessions; DROP TABLE comments; DROP TABLE articles; DROP TABLE users;",
    );
  }
}
```
{% endraw %}

<a id="file-8"></a>

### server/production.ts

设置生产模式并启动 API。

代码位置：`server/production.ts`（完整文件）。

{% raw %}
```typescript
Object.assign(process.env, { NODE_ENV: 'production' });
void import('./index');
```
{% endraw %}

<a id="file-9"></a>

### src/app/about/page.tsx

关于页面。

代码位置：`src/app/about/page.tsx`（完整文件）。

{% raw %}
```tsx
export const metadata = { title: "关于拾页" };
export default function About() {
  return (
    <div className="reading">
      <section className="intro">
        <h1>让想法，慢慢生长。</h1>
        <p>拾页，是一个安放文字的小地方。</p>
      </section>
      <div className="markdown">
        <p>
          在这里记录技术探索，也记录日常里微小而真实的发现。写作是整理思绪的过程，阅读是与另一种经验相遇。
        </p>
        <h2>关于这个小站</h2>
        <p>
          用 Markdown
          写作，分享值得记录的想法。欢迎注册账号，在文章下留下你的思考。
        </p>
        <h2>留言约定</h2>
        <p>
          尊重不同的观点，讨论具体的问题。请勿发布他人的隐私或无关广告。你可以删除自己的留言，管理员可以维护讨论秩序。
        </p>
      </div>
    </div>
  );
}
```
{% endraw %}

<a id="file-10"></a>

### src/app/admin/[id]/page.tsx

编辑文章动态路由与 Suspense 边界。

代码位置：`src/app/admin/[id]/page.tsx`（完整文件）。

{% raw %}
```tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ id: string }>;
}) {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor id={(await params).id} />
    </Suspense>
  );
}
```
{% endraw %}

<a id="file-11"></a>

### src/app/admin/new/page.tsx

新建文章与 Suspense 边界。

代码位置：`src/app/admin/new/page.tsx`（完整文件）。

{% raw %}
```tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default function Page() {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor />
    </Suspense>
  );
}
```
{% endraw %}

<a id="file-12"></a>

### src/app/admin/page.tsx

管理员文章列表。

代码位置：`src/app/admin/page.tsx`（完整文件）。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export default function Page() {
  return <ArticleList manage />;
}
```
{% endraw %}

<a id="file-13"></a>

### src/app/archive/page.tsx

归档路由，复用文章列表。

代码位置：`src/app/archive/page.tsx`（完整文件）。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export const metadata = { title: "文章归档" };
export default function Archive() {
  return <ArticleList archive />;
}
```
{% endraw %}

<a id="file-14"></a>

### src/app/globals.css

Tailwind、DaisyUI 主题、卡片和响应式布局。

代码位置：`src/app/globals.css`（完整文件）。

{% raw %}
```css
@import "tailwindcss";
@plugin "daisyui" {
  themes: false;
  logs: false;
}
@plugin "daisyui/theme" {
  name: "fieldnotes";
  default: true;
  color-scheme: light;
  --color-base-100: #faf9f6;
  --color-base-200: #efeee8;
  --color-base-300: #dedfd7;
  --color-base-content: #252c2a;
  --color-primary: #2f7068;
  --color-primary-content: #ffffff;
  --color-secondary: #62756d;
  --color-secondary-content: #ffffff;
  --color-accent: #2f7068;
  --color-accent-content: #ffffff;
  --color-neutral: #283a35;
  --color-neutral-content: #ffffff;
  --color-info: #326f93;
  --color-info-content: #ffffff;
  --color-success: #357257;
  --color-success-content: #ffffff;
  --color-warning: #d8ac53;
  --color-warning-content: #252c2a;
  --color-error: #a33131;
  --color-error-content: #ffffff;
  --radius-selector: 0.5rem;
  --radius-field: 0.65rem;
  --radius-box: 1rem;
  --size-selector: 0.25rem;
  --size-field: 0.25rem;
  --border: 1px;
  --depth: 0;
  --noise: 0;
}
@plugin "daisyui/theme" {
  name: "fieldnotes-dark";
  default: false;
  color-scheme: dark;
  --color-base-100: #111a19;
  --color-base-200: #192423;
  --color-base-300: #30403c;
  --color-base-content: #e3eae5;
  --color-primary: #83c5b3;
  --color-primary-content: #10231d;
  --color-secondary: #a4bcb1;
  --color-secondary-content: #10231d;
  --color-accent: #83c5b3;
  --color-accent-content: #10231d;
  --color-neutral: #cadbd3;
  --color-neutral-content: #10231d;
  --color-info: #82b9dd;
  --color-info-content: #10231d;
  --color-success: #8cc9a3;
  --color-success-content: #10231d;
  --color-warning: #d8ac53;
  --color-warning-content: #e3eae5;
  --color-error: #f0a4a4;
  --color-error-content: #10231d;
  --radius-selector: 0.5rem;
  --radius-field: 0.65rem;
  --radius-box: 1rem;
  --size-selector: 0.25rem;
  --size-field: 0.25rem;
  --border: 1px;
  --depth: 0;
  --noise: 0;
}
@theme {
  --color-paper: #faf9f6;
  --color-ink: #252c2a;
  --color-muted: #626b67;
  --color-line: #dedfd7;
  --font-sans: Arial, "Microsoft YaHei", sans-serif;
}
:root {
  --serif: "Noto Serif SC", "Songti SC", "STSong", serif;
  --bg: var(--color-base-100);
  --ink: var(--color-base-content);
  --accent: var(--color-primary);
  --line: var(--color-base-300);
}
body {
  margin: 0;
  background: var(--bg);
  color: var(--ink);
  font:
    16px/1.85 Arial,
    "Microsoft YaHei",
    sans-serif;
  -webkit-font-smoothing: antialiased;
}
a {
  color: var(--accent);
  text-decoration: none;
}
a:hover {
  text-decoration: underline;
}
.btn:hover {
  text-decoration: none;
}
:focus-visible {
  outline: 2px solid var(--accent);
  outline-offset: 4px;
}
h1,
h2,
h3 {
  font-family: var(--serif);
  font-weight: 600;
  line-height: 1.5;
}
h1 {
  font-size: clamp(28px, 4vw, 40px);
}
h2 {
  font-size: 24px;
}
.site-header {
  background: var(--bg);
}
.header-inner {
  max-width: 1120px;
  margin: auto;
  padding: 28px 32px;
  justify-content: space-between;
  gap: 24px;
  min-height: 100px;
}
.brand {
  font-family: var(--serif);
  font-size: 30px;
  color: var(--ink);
  letter-spacing: 0.12em;
}
.brand:hover {
  text-decoration: none;
}
nav {
  display: flex;
  align-items: center;
  gap: 28px;
  font-size: 14px;
  flex-wrap: wrap;
}
nav > a {
  color: var(--muted);
}
.username {
  max-width: 110px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
.site-main {
  max-width: 1040px;
  min-height: 72vh;
  margin: auto;
  padding: 0 30px 70px;
}
.home-hero {
  min-height: 570px;
  place-items: center start;
  position: relative;
  isolation: isolate;
  padding: 64px 0 96px;
}
.home-hero:before {
  content: "";
  position: absolute;
  width: min(640px, 95vw);
  height: 440px;
  left: -160px;
  top: -70px;
  background: radial-gradient(ellipse, #2f70680b, transparent 68%);
  z-index: -1;
  pointer-events: none;
}
.home-hero-content {
  max-width: none;
  display: block;
  padding: 0;
  text-align: left;
}
.home-hero h1 {
  font-size: clamp(42px, 5.6vw, 76px);
  line-height: 1.35;
  letter-spacing: 0.035em;
  margin: 0;
  font-weight: 500;
}
.hero-line {
  display: block;
  overflow: hidden;
  padding-bottom: 0.08em;
}
.hero-line > span {
  display: block;
  animation: hero-reveal 1.1s cubic-bezier(0.2, 0.7, 0.25, 1) both;
}
.hero-line:nth-child(2) > span {
  animation-delay: 160ms;
}
.hero-accent {
  color: var(--accent);
}
.hero-description {
  font-size: 17px;
  color: var(--muted);
  margin: 30px 0 0;
  animation: hero-reveal 1s 320ms both;
}
.hero-read {
  margin-top: 32px;
  min-height: 42px;
  padding-inline: 22px;
  font-weight: 400;
  animation: hero-reveal 1s 420ms both;
  border-color: #abbab0;
  color: var(--accent);
}
@keyframes hero-reveal {
  from {
    opacity: 0;
    transform: translateY(24px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}
.intro {
  padding: 60px 0 44px;
}
.intro h1 {
  margin: 0 0 24px;
}
.section-heading {
  scroll-margin-top: 24px;
  margin-bottom: 24px;
}
.section-heading h2 {
  font-size: 24px;
  margin: 0;
}
.list-toolbar {
  display: flex;
  gap: 14px;
  margin-bottom: 30px;
  align-items: center;
}
.search {
  flex: 1;
}
.list-toolbar .input {
  margin: 0;
  width: 100%;
  background: transparent;
}
.list-toolbar .select {
  min-width: 140px;
  background-color: transparent;
}
.input,
.textarea,
.select,
.file-input {
  width: 100%;
  font-size: 15px;
}
.input,
.select {
  height: 46px;
}
.textarea {
  min-height: 110px;
}
.file-input {
  height: auto;
  min-height: 46px;
}
.btn {
  font-weight: 500;
  letter-spacing: 0;
}
.btn:disabled {
  cursor: not-allowed;
}
label {
  display: block;
  font-size: 14px;
}
label .input,
label .textarea {
  margin-top: 7px;
}
.muted {
  color: var(--muted);
  font-size: 14px;
}
.article-list {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 24px;
}
.article-card {
  padding: 28px;
  border-radius: var(--radius-box);
  border: 1px solid var(--line);
  box-shadow: none;
  background: var(--surface);
  gap: 0;
  transition:
    border-color 0.2s,
    transform 0.2s,
    box-shadow 0.2s;
}
.article-card:hover {
  border-color: var(--color-primary);
  transform: translateY(-3px);
  box-shadow: 0 12px 32px #00000008;
}
.article-card h2 {
  font-size: 24px;
  margin: 18px 0 12px;
  line-height: 1.6;
}
.article-card h2 a {
  color: var(--ink);
}
.article-card h2 a:hover {
  color: var(--accent);
  text-decoration: none;
}
.article-card p {
  font-size: 16px;
  color: var(--muted);
  margin: 0;
  line-height: 1.95;
}
.meta {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 16px;
  font-size: 13px;
  color: var(--muted);
}
.meta span:first-child {
  color: var(--accent);
}
.card-foot {
  margin-top: 20px;
}
.actions {
  display: flex;
  gap: 16px;
  align-items: center;
  font-size: 14px;
}
.pagination {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 20px;
  margin: 40px 0;
  font-size: 14px;
}
.site-footer {
  max-width: 980px;
  margin: 0 auto;
  padding: 26px 0 40px;
  border-top: 1px solid var(--line);
  color: var(--muted);
  font: 14px var(--serif);
}
.reading {
  max-width: 740px;
  margin: 15px auto;
}
.article-head {
  padding: 40px 0 30px;
  margin-bottom: 32px;
  border-bottom: 1px solid var(--line);
}
.article-head h1 {
  margin: 18px 0;
}
.article-head > p {
  color: var(--muted);
}
.markdown {
  overflow-wrap: anywhere;
  line-height: 2;
}
.markdown h2 {
  margin: 40px 0 18px;
}
.markdown h3 {
  margin: 28px 0 15px;
}
.markdown p {
  margin: 18px 0;
}
.markdown ul {
  list-style: disc;
  padding-left: 26px;
}
.markdown ol {
  list-style: decimal;
  padding-left: 26px;
}
.markdown pre {
  background: #202c2b;
  color: #e3ede8;
  overflow: auto;
  padding: 24px;
  border-radius: 12px;
  font-size: 14px;
  line-height: 1.8;
}
.markdown code {
  font-family: Consolas, monospace;
}
.markdown :not(pre) > code {
  background: var(--color-base-200);
  padding: 3px 6px;
  border-radius: 4px;
  font-size: 0.9em;
}
.markdown blockquote {
  border-left: 3px solid var(--accent);
  padding: 4px 22px;
  background: #2f706807;
  color: var(--muted);
}
.markdown table {
  display: block;
  overflow: auto;
  border-collapse: collapse;
  max-width: 100%;
}
.markdown td,
.markdown th {
  border: 1px solid var(--line);
  padding: 8px 14px;
}
.markdown hr {
  border: 0;
  border-top: 1px solid var(--line);
  margin: 30px 0;
}
.hljs-keyword,
.hljs-selector-tag {
  color: #9bd3c4;
}
.hljs-string {
  color: #c8d6a2;
}
.hljs-comment {
  color: #94a6a2;
}
.image-placeholder {
  display: block;
  border: 1px dashed var(--line);
  padding: 16px;
  font-size: 14px;
}
.comments {
  margin-top: 60px;
  border-top: 1px solid var(--line);
  padding-top: 25px;
}
.comments form .btn {
  margin-top: 12px;
}
.comment {
  padding: 22px 0;
  border-bottom: 1px solid var(--line);
}
.comment p {
  margin: 10px 0;
  overflow-wrap: anywhere;
}
.notice {
  padding: 14px 18px;
  background: #e9eee7;
  border-radius: 9px;
  font-size: 14px;
}
.error {
  background: #faeae7;
  color: var(--color-error);
}
.panel {
  padding: 36px;
  background: var(--surface);
  border: 1px solid var(--line);
  border-radius: 18px;
}
.auth {
  max-width: 470px;
  margin: 45px auto;
}
.auth h1 {
  font-size: 32px;
  margin: 12px 0;
}
.auth form {
  margin-top: 25px;
}
.auth label {
  margin-top: 18px;
}
.auth form > .btn {
  width: 100%;
  margin-top: 15px;
}
.empty {
  text-align: center;
  padding: 60px 15px;
}
.page-title {
  margin: 25px 0;
}
.editor-fields {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}
.editor-columns {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 24px;
  margin: 25px 0;
}
.code-editor {
  min-height: 600px;
  font:
    14px/1.9 Consolas,
    monospace;
}
.preview {
  padding: 24px;
  background: var(--surface);
  border: 1px solid var(--line);
  border-radius: 12px;
  min-width: 0;
  max-height: 720px;
  overflow: auto;
}
.preview h2 {
  font-size: 18px;
}
.file-import {
  margin: 25px 0;
  max-width: 400px;
}
.file-import input {
  font-size: 14px;
}

.markdown img {
  max-width: 100%;
  height: auto;
  border-radius: 12px;
  margin: 20px 0;
}
.alert {
  margin: 18px 0;
  font-size: 14px;
  box-shadow: none;
}
.panel {
  background: var(--color-base-100);
  box-shadow: none;
}
.auth {
  margin-top: 60px;
}
.preview {
  border-radius: var(--radius-box);
}
@media (max-width: 700px) {
  .header-inner {
    padding: 22px;
    min-height: 84px;
    flex-wrap: wrap;
    gap: 14px;
  }
  nav {
    gap: 20px;
  }
  .brand {
    font-size: 27px;
  }
  .site-main {
    padding: 0 22px 50px;
  }
  .home-hero {
    min-height: 460px;
    padding: 60px 0 64px;
  }
  .home-hero h1 {
    font-size: clamp(42px, 10vw, 58px);
  }
  .hero-description {
    font-size: 16px;
    margin-top: 24px;
  }
  .hero-read {
    margin-top: 26px;
  }
  .article-list,
  .editor-fields,
  .editor-columns {
    grid-template-columns: 1fr;
  }
  .article-card {
    padding: 24px;
  }
  .article-card h2 {
    font-size: 23px;
  }
  .list-toolbar {
    flex-wrap: wrap;
  }
  .search {
    min-width: 170px;
  }
  .list-toolbar > label:last-child {
    flex: 1;
  }
  .section-heading {
    margin-bottom: 20px;
  }
  .intro {
    padding: 40px 0 24px;
  }
  .auth {
    padding: 24px;
    margin: 25px 0;
  }
  .site-footer {
    margin: 0 22px;
  }
  .code-editor {
    min-height: 400px;
  }
  .reading {
    margin-top: 25px;
  }
  .meta {
    gap: 12px;
  }
}
@media (prefers-reduced-motion: reduce) {
  .hero-line > span,
  .hero-description,
  .hero-read {
    animation: none;
    opacity: 1;
    transform: none;
  }
  * {
    scroll-behavior: auto !important;
    transition: none !important;
  }
}

:root {
  --surface: #ffffff;
  --muted: #64716a;
}
[data-theme="fieldnotes-dark"] {
  --surface: #192423;
  --muted: #a7bab0;
  --color-paper: #111a19;
  --color-ink: #e3eae5;
  --color-muted: #a7bab0;
  --color-line: #30403c;
}
.nav-item,
.meta time,
.actions > a,
.comments > h2 {
  display: inline-flex;
  align-items: center;
  gap: 8px;
}
.nav-item svg {
  opacity: 0.75;
}
.meta .badge {
  font-size: 13px;
  padding: 5px 10px;
  height: auto;
  font-weight: 400;
}
.meta time {
  gap: 6px;
}
.search {
  position: relative;
}
.search-icon {
  position: absolute;
  left: 14px;
  top: 50%;
  transform: translateY(-50%);
  color: var(--muted);
  pointer-events: none;
  z-index: 1;
}
.search .input {
  padding-left: 42px;
}
.theme-toggle {
  color: var(--ink);
  flex-shrink: 0;
}
.theme-toggle svg {
  stroke-width: 1.6;
}
.article-card p {
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}
.article-card .card-foot {
  margin-top: auto;
  padding-top: 24px;
}
.archive-list {
  display: grid;
  gap: 18px;
}
.alert {
  display: block;
}
.alert a {
  margin-inline: 3px;
}
.comments > h2 {
  display: flex;
}
.markdown blockquote {
  background: color-mix(in srgb, var(--accent) 7%, transparent);
}
::view-transition-old(root),
::view-transition-new(root) {
  animation: none;
  mix-blend-mode: normal;
}
::view-transition-old(root) {
  z-index: 1;
}
::view-transition-new(root) {
  z-index: 2;
}
@media (max-width: 700px) {
  .header-inner {
    position: relative;
    gap: 16px;
    flex-wrap: wrap;
  }
  nav {
    width: 100%;
    gap: 14px;
  }
  .theme-toggle {
    position: absolute;
    top: 22px;
    right: 22px;
    width: 36px;
    height: 36px;
    min-height: 36px;
  }
  .nav-item {
    gap: 5px;
  }
}
@media (prefers-reduced-motion: reduce) {
  .article-card:hover {
    transform: none;
  }
}
```
{% endraw %}

<a id="file-15"></a>

### src/app/layout.tsx

公共布局、会话 Provider、页眉和页脚。

代码位置：`src/app/layout.tsx`（完整文件）。

{% raw %}
```tsx
import type { Metadata } from "next";
import { Header, SessionProvider } from "@/components/full-blog";
import "./globals.css";
import { themeInitScript } from "@/lib/theme";
export const metadata: Metadata = {
  title: { default: "拾页 · 落笔成文，拾页成光", template: "%s · 拾页" },
  description: "关于技术、生活与每一个值得记录的想法。",
};
export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="zh-CN" data-theme="fieldnotes" suppressHydrationWarning>
      <head>
        <script dangerouslySetInnerHTML={{ __html: themeInitScript }} />
      </head>
      <body>
        <SessionProvider>
          <Header />
          <main className="site-main">{children}</main>
          <footer className="site-footer">
            <span>拾页</span>
          </footer>
        </SessionProvider>
      </body>
    </html>
  );
}
```
{% endraw %}

<a id="file-16"></a>

### src/app/login/page.tsx

登录路由，复用 AuthForm。

代码位置：`src/app/login/page.tsx`（完整文件）。

{% raw %}
```tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "登录" };
export default function Page() {
  return <AuthForm />;
}
```
{% endraw %}

<a id="file-17"></a>

### src/app/not-found.tsx

Next.js 未找到路由时的页面。

代码位置：`src/app/not-found.tsx`（完整文件）。

{% raw %}
```tsx
import Link from "next/link";

export default function NotFound() {
  return (
    <main className="px-6 py-28 text-center">
      <p className="font-mono text-sm text-accent">404 / PAGE NOT FOUND</p>
      <h1 className="mt-5 text-3xl font-bold">这一页，还没有写下。</h1>
      <p className="mt-4 text-sm text-muted">
        地址可能有误，回到首页继续翻阅吧。
      </p>
      <Link
        href="/"
        className="mt-8 inline-block rounded-full bg-ink px-6 py-3 text-sm text-white"
      >
        返回首页
      </Link>
    </main>
  );
}
```
{% endraw %}

<a id="file-18"></a>

### src/app/page.tsx

首页路由，渲染文章列表。

代码位置：`src/app/page.tsx`（完整文件）。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
```
{% endraw %}

<a id="file-19"></a>

### src/app/posts/[slug]/page.tsx

按 slug 打开阅读组件。

代码位置：`src/app/posts/[slug]/page.tsx`（完整文件）。

{% raw %}
```tsx
import { ArticleReader } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ slug: string }>;
}) {
  return <ArticleReader slug={(await params).slug} />;
}
```
{% endraw %}

<a id="file-20"></a>

### src/app/register/page.tsx

注册路由，复用 AuthForm。

代码位置：`src/app/register/page.tsx`（完整文件）。

{% raw %}
```tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "注册" };
export default function Page() {
  return <AuthForm register />;
}
```
{% endraw %}

<a id="file-21"></a>

### src/components/full-blog.tsx

api、Context、导航、列表、登录、阅读、评论与编辑。

代码位置：`src/components/full-blog.tsx`（完整文件）。

{% raw %}
```tsx
"use client";
import {
  createContext,
  useContext,
  useEffect,
  useState,
  useRef,
  type ReactNode,
  type FormEvent,
} from "react";
import Link from "next/link";
import { HomeHero } from "./home-hero";
import { ThemeToggle } from "./theme-toggle";
import {
  BookOpen,
  Archive,
  UserRound,
  Settings2,
  LogOut,
  Search,
  Folder,
  CalendarDays,
  Plus,
  Pencil,
  Trash2,
  MessageCircle,
  Send,
  Save,
} from "lucide-react";
import { useRouter, useSearchParams } from "next/navigation";
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";
import rehypeHighlight from "rehype-highlight";
type User = {
  id: string;
  username: string;
  email: string;
  role: "user" | "admin";
};
type Session = { user: User | null; csrf: string };
type Article = {
  id: string;
  slug: string;
  title: string;
  excerpt: string;
  content: string;
  category: string;
  createdAt: string;
  version: number;
};
type Comment = {
  id: string;
  userId: string;
  username: string;
  content: string;
  createdAt: string;
};
async function api<T>(
  path: string,
  method = "GET",
  body?: unknown,
  csrf?: string,
): Promise<T> {
  const r = await fetch("/api" + path, {
    method,
    credentials: "same-origin",
    cache: "no-store",
    headers: {
      "Content-Type": "application/json",
      ...(csrf ? { "X-CSRF-Token": csrf } : {}),
    },
    ...(body ? { body: JSON.stringify(body) } : {}),
  });
  if (r.status === 204) return undefined as T;
  const data = await r.json().catch(() => ({ message: "服务响应异常" }));
  if (!r.ok)
    throw new Error(data.details?.join("；") || data.message || "请求失败");
  return data;
}
const Context = createContext<{
  session: Session;
  loading: boolean;
  error: string;
  refresh: () => Promise<void>;
  setSession: (s: Session) => void;
}>({
  session: { user: null, csrf: "" },
  loading: true,
  error: "",
  refresh: async () => {},
  setSession: () => {},
});
export function SessionProvider({ children }: { children: ReactNode }) {
  const [session, setSession] = useState<Session>({ user: null, csrf: "" }),
    [loading, setLoading] = useState(true),
    [error, setError] = useState("");
  async function refresh() {
    try {
      setSession(await api<Session>("/auth/session"));
      setError("");
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setLoading(false);
    }
  }
  useEffect(() => {
    void refresh();
  }, []);
  return (
    <Context.Provider value={{ session, loading, error, refresh, setSession }}>
      {children}
    </Context.Provider>
  );
}
const date = (s: string) =>
  new Date(s).toLocaleDateString("zh-CN", {
    year: "numeric",
    month: "long",
    day: "numeric",
  });
function ErrorMessage({ message }: { message: string }) {
  return message ? (
    <p className="alert alert-error" role="alert">
      {message}
    </p>
  ) : null;
}
export function Header() {
  const { session, loading, refresh } = useContext(Context);
  const [error, setError] = useState("");
  async function logout() {
    try {
      await api("/auth/logout", "POST", undefined, session.csrf);
      await refresh();
      setError("");
    } catch (e) {
      setError((e as Error).message);
    }
  }
  return (
    <header className="site-header">
      <div className="navbar header-inner">
        <Link className="brand" href="/">
          拾页
        </Link>
        <nav aria-label="主导航">
          <Link href="/" className="nav-item">
            <BookOpen size={16} aria-hidden="true" />
            文章
          </Link>
          <Link href="/archive" className="nav-item">
            <Archive size={16} aria-hidden="true" />
            归档
          </Link>
          <Link href="/about" className="nav-item">
            <UserRound size={16} aria-hidden="true" />
            关于
          </Link>
          {session.user?.role === "admin" && (
            <Link href="/admin" className="nav-item">
              <Settings2 size={16} aria-hidden="true" />
              管理
            </Link>
          )}
          {loading ? (
            <span>加载中</span>
          ) : session.user ? (
            <>
              <span className="username">{session.user.username}</span>
              <button className="btn btn-ghost btn-sm" onClick={logout}>
                <LogOut size={16} aria-hidden="true" />
                退出
              </button>
            </>
          ) : (
            <Link className="btn btn-ghost btn-sm nav-login" href="/login">
              <UserRound size={16} aria-hidden="true" />
              登录 / 注册
            </Link>
          )}
          <ThemeToggle />
        </nav>
      </div>
      <ErrorMessage message={error} />
    </header>
  );
}
export function Markdown({ content }: { content: string }) {
  return (
    <div className="markdown">
      <ReactMarkdown
        remarkPlugins={[remarkGfm]}
        rehypePlugins={[rehypeHighlight]}
        skipHtml
        components={{
          a: ({ children, href }) => (
            <a href={href} rel="nofollow noopener noreferrer">
              {children}
            </a>
          ),
          img: ({ src, alt }) =>
            typeof src === "string" && /^(https?:\/\/|\/(?!\/))/.test(src) ? (
              <img
                src={src}
                alt={alt || ""}
                loading="lazy"
                referrerPolicy="no-referrer"
              />
            ) : (
              <span>[图片地址不支持]</span>
            ),
        }}
      >
        {content}
      </ReactMarkdown>
    </div>
  );
}
export function ArticleList({
  archive = false,
  manage = false,
}: {
  archive?: boolean;
  manage?: boolean;
}) {
  const { session, loading } = useContext(Context);
  const [items, setItems] = useState<Article[]>([]),
    [q, setQ] = useState(""),
    [category, setCategory] = useState(""),
    [categories, setCategories] = useState<{ name: string }[]>([]),
    [page, setPage] = useState(1),
    [total, setTotal] = useState(0),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(true),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    const timer = setTimeout(() => {
      setBusy(true);
      api<{ items: Article[]; total: number }>(
        `/articles?q=${encodeURIComponent(q)}&category=${encodeURIComponent(category)}&page=${page}`,
      )
        .then((r) => {
          if (alive) {
            setItems(r.items);
            setTotal(r.total);
            setError("");
          }
        })
        .catch((e) => {
          if (alive) setError(e.message);
        })
        .finally(() => {
          if (alive) setBusy(false);
        });
    }, 200);
    return () => {
      alive = false;
      clearTimeout(timer);
    };
  }, [q, category, page, revision]);
  useEffect(() => {
    api<{ name: string }[]>("/categories")
      .then(setCategories)
      .catch(() => {});
  }, [revision]);
  async function remove(a: Article) {
    if (!confirm(`确定删除「${a.title}」及其评论？`)) return;
    try {
      await api("/articles/" + a.id, "DELETE", undefined, session.csrf);
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    }
  }
  if (manage && loading) return <p>正在验证身份…</p>;
  if (manage && session.user?.role !== "admin")
    return (
      <section className="card panel">
        <h1>仅管理员可管理文章</h1>
        <Link href="/login">登录账号</Link>
      </section>
    );
  return (
    <>
      {!manage && !archive ? (
        <HomeHero />
      ) : (
        <section className="intro">
          <h1>{manage ? "文章管理" : "文章归档"}</h1>
          {manage && (
            <Link className="btn btn-primary" href="/admin/new">
              <Plus size={17} aria-hidden="true" />
              写一篇文章
            </Link>
          )}
        </section>
      )}
      <div id="articles" className="section-heading">
        <h2>{manage ? "所有文章" : archive ? "全部文章" : "最新文章"}</h2>
      </div>
      <div className="list-toolbar">
        <label className="search">
          <Search className="search-icon" size={18} aria-hidden="true" />
          <span className="sr-only">搜索文章</span>
          <input
            className="input"
            placeholder="搜索文章或正文…"
            value={q}
            onChange={(e) => {
              setQ(e.target.value);
              setPage(1);
            }}
          />
        </label>
        <label>
          <span className="sr-only">分类</span>
          <select
            className="select"
            value={category}
            onChange={(e) => {
              setCategory(e.target.value);
              setPage(1);
            }}
          >
            <option value="">全部分类</option>
            {categories.map((c) => (
              <option key={c.name}>{c.name}</option>
            ))}
          </select>
        </label>
      </div>
      <ErrorMessage message={error} />
      {error && (
        <button
          className="btn btn-ghost btn-sm"
          onClick={() => setRevision((n) => n + 1)}
        >
          重新加载
        </button>
      )}
      <p className="sr-only" aria-live="polite">
        {busy ? "正在翻开书页…" : `${total} 篇文章`}
      </p>
      {!busy && !error && !items.length && (
        <div className="empty">
          <h2>这里还没有文章</h2>
          <p>试试其他关键词，或等待下一次更新。</p>
          <button
            className="btn btn-ghost btn-sm"
            onClick={() => {
              setQ("");
              setCategory("");
              setPage(1);
            }}
          >
            清空筛选
          </button>
        </div>
      )}
      <div className={archive ? "archive-list" : "article-list"}>
        {!error &&
          items.map((a) => (
            <article className="card article-card" key={a.id}>
              <div className="meta">
                <span className="badge badge-soft badge-primary">
                  <Folder size={13} aria-hidden="true" />
                  {a.category}
                </span>
                <time>
                  <CalendarDays size={13} aria-hidden="true" />
                  {date(a.createdAt)}
                </time>
              </div>
              <h2>
                <Link href={"/posts/" + a.slug}>{a.title}</Link>
              </h2>
              {!archive && <p>{a.excerpt}</p>}
              {manage && (
                <div className="card-foot">
                  {manage && (
                    <div className="actions">
                      <Link
                        href={`/admin/${a.id}?slug=${encodeURIComponent(a.slug)}`}
                      >
                        <Pencil size={15} aria-hidden="true" />
                        编辑
                      </Link>
                      <button
                        className="btn btn-ghost btn-sm text-error"
                        onClick={() => remove(a)}
                      >
                        <Trash2 size={15} aria-hidden="true" />
                        删除
                      </button>
                    </div>
                  )}
                </div>
              )}
            </article>
          ))}
      </div>
      {total > 10 && (
        <div className="pagination">
          <button
            className="btn btn-ghost btn-sm"
            disabled={busy || page === 1}
            onClick={() => setPage((n) => n - 1)}
          >
            上一页
          </button>
          <span>
            第 {page} / {Math.max(1, Math.ceil(total / 10))} 页
          </span>
          <button
            className="btn btn-ghost btn-sm"
            disabled={busy || page * 10 >= total}
            onClick={() => setPage((n) => n + 1)}
          >
            下一页
          </button>
        </div>
      )}
    </>
  );
}
export function AuthForm({ register = false }: { register?: boolean }) {
  const {
    session,
    setSession,
    loading,
    error: connection,
    refresh,
  } = useContext(Context);
  const [error, setError] = useState(""),
    [busy, setBusy] = useState(false);
  const router = useRouter();
  async function submit(e: FormEvent<HTMLFormElement>) {
    e.preventDefault();
    const f = new FormData(e.currentTarget);
    setBusy(true);
    try {
      setSession(
        await api<Session>(
          "/auth/" + (register ? "register" : "login"),
          "POST",
          {
            email: f.get("email"),
            password: f.get("password"),
            ...(register ? { username: f.get("username") } : {}),
          },
          session.csrf,
        ),
      );
      router.push("/");
    } catch (e) {
      setError((e as Error).message);
      await refresh();
    } finally {
      setBusy(false);
    }
  }
  return (
    <section className="card panel auth">
      <h1>{register ? "初次见面，欢迎你" : "欢迎回来"}</h1>
      <ErrorMessage message={error || connection} />
      {connection && (
        <button className="btn btn-ghost btn-sm" onClick={refresh}>
          重新连接
        </button>
      )}
      {session.user ? (
        <p>
          你已登录为 {session.user.username}。<Link href="/">返回首页</Link>
        </p>
      ) : (
        <form onSubmit={submit}>
          {register && (
            <label>
              昵称
              <input
                className="input"
                name="username"
                minLength={2}
                maxLength={40}
                autoComplete="nickname"
                required
              />
            </label>
          )}
          <label>
            邮箱
            <input
              className="input"
              name="email"
              type="email"
              autoComplete="email"
              maxLength={255}
              required
            />
          </label>
          <label>
            密码
            <input
              className="input"
              name="password"
              type="password"
              minLength={10}
              maxLength={72}
              autoComplete={register ? "new-password" : "current-password"}
              required
            />
          </label>
          <p className="muted">至少 10 个字符，最多 72 字节。</p>
          <button
            className="btn btn-primary"
            disabled={busy || loading || !session.csrf}
          >
            {busy ? "请稍候…" : register ? "注册并登录" : "登录"}
          </button>
          <p>
            {register ? "已有账号？" : "还没有账号？"}{" "}
            <Link href={register ? "/login" : "/register"}>
              {register ? "去登录" : "创建账号"}
            </Link>
          </p>
        </form>
      )}
    </section>
  );
}
export function ArticleReader({ slug }: { slug: string }) {
  const [article, setArticle] = useState<Article | null>(null),
    [error, setError] = useState(""),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    setArticle(null);
    api<Article>("/articles/" + encodeURIComponent(slug))
      .then((a) => {
        if (alive) {
          setArticle(a);
          setError("");
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [slug, revision]);
  return (
    <div className="reading">
      <Link href="/">返回文章列表</Link>
      <ErrorMessage message={error} />
      {error && (
        <button
          className="btn btn-ghost btn-sm"
          onClick={() => setRevision((n) => n + 1)}
        >
          重试
        </button>
      )}
      {!article && !error && <p>正在打开文章…</p>}
      {article && (
        <>
          <article>
            <header className="article-head">
              <div className="meta">
                <span>{article.category}</span>
                <time>{date(article.createdAt)}</time>
              </div>
              <h1>{article.title}</h1>
              <p>{article.excerpt}</p>
            </header>
            <Markdown content={article.content} />
          </article>
          <Comments articleId={article.id} />
        </>
      )}
    </div>
  );
}
function Comments({ articleId }: { articleId: string }) {
  const { session } = useContext(Context);
  const [items, setItems] = useState<Comment[]>([]),
    [total, setTotal] = useState(0),
    [page, setPage] = useState(1),
    [content, setContent] = useState(""),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(false),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    api<{ items: Comment[]; total: number }>(
      `/articles/${articleId}/comments?page=${page}`,
    )
      .then((r) => {
        if (alive) {
          setItems(r.items);
          setTotal(r.total);
          setError("");
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [articleId, page, revision]);
  async function submit(e: FormEvent) {
    e.preventDefault();
    setBusy(true);
    try {
      await api(
        `/articles/${articleId}/comments`,
        "POST",
        { content },
        session.csrf,
      );
      setContent("");
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }
  async function remove(id: string) {
    if (!confirm("确定删除这条评论？")) return;
    try {
      await api("/comments/" + id, "DELETE", undefined, session.csrf);
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    }
  }
  return (
    <section className="comments">
      <h2>
        <MessageCircle size={21} aria-hidden="true" />
        页边留言 <span className="muted">{total}</span>
      </h2>
      <ErrorMessage message={error} />
      {session.user ? (
        <form onSubmit={submit}>
          <label htmlFor="comment">以 {session.user.username} 的身份留言</label>
          <textarea
            className="textarea"
            id="comment"
            required
            maxLength={2000}
            value={content}
            onChange={(e) => setContent(e.target.value)}
            placeholder="写下你的想法…"
          />
          <button
            className="btn btn-primary"
            disabled={busy || !content.trim()}
          >
            <Send size={16} aria-hidden="true" />
            {busy ? "正在发表…" : "发表留言"}
          </button>
        </form>
      ) : (
        <p className="alert">
          <Link href="/login">登录</Link> 后参与讨论，也可以{" "}
          <Link href="/register">注册账号</Link>。
        </p>
      )}
      {items.map((c) => (
        <article key={c.id} className="comment">
          <div className="meta">
            <strong>{c.username}</strong>
            <time>{date(c.createdAt)}</time>
            {(session.user?.id === c.userId ||
              session.user?.role === "admin") && (
              <button
                className="btn btn-ghost btn-sm text-error"
                onClick={() => remove(c.id)}
              >
                <Trash2 size={15} aria-hidden="true" />
                删除
              </button>
            )}
          </div>
          <p className="whitespace-pre-wrap">{c.content}</p>
        </article>
      ))}
      {!items.length && !error && (
        <p className="muted">还没有留言，来写下第一句吧。</p>
      )}
      {total > 30 && (
        <div className="pagination">
          <button
            className="btn btn-ghost btn-sm"
            disabled={page === 1}
            onClick={() => setPage((n) => n - 1)}
          >
            上一页
          </button>
          <span>
            {page} / {Math.max(1, Math.ceil(total / 30))}
          </span>
          <button
            className="btn btn-ghost btn-sm"
            disabled={page * 30 >= total}
            onClick={() => setPage((n) => n + 1)}
          >
            下一页
          </button>
        </div>
      )}
    </section>
  );
}
const blank = {
  title: "",
  slug: "",
  excerpt: "",
  category: "随笔",
  content: "",
};
export function Editor({ id }: { id?: string }) {
  const { session, loading } = useContext(Context);
  const slug = useSearchParams().get("slug");
  const router = useRouter();
  const [form, setForm] = useState(blank),
    [version, setVersion] = useState(1),
    [ready, setReady] = useState(!id),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(false);
  const dirty = useRef(false);
  useEffect(() => {
    let alive = true;
    if (!id) return;
    if (!slug) {
      setError("请从管理列表进入编辑");
      return;
    }
    api<Article>("/articles/" + encodeURIComponent(slug))
      .then((a) => {
        if (a.id !== id) throw new Error("文章地址不匹配");
        if (alive) {
          setForm({
            title: a.title,
            slug: a.slug,
            excerpt: a.excerpt,
            category: a.category,
            content: a.content,
          });
          setVersion(a.version);
          setReady(true);
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [id, slug]);
  useEffect(() => {
    const listener = (e: BeforeUnloadEvent) => {
      if (dirty.current) e.preventDefault();
    };
    window.addEventListener("beforeunload", listener);
    return () => window.removeEventListener("beforeunload", listener);
  }, []);
  function change(key: keyof typeof blank, value: string) {
    dirty.current = true;
    setForm((f) => ({ ...f, [key]: value }));
  }
  async function save(e: FormEvent) {
    e.preventDefault();
    setBusy(true);
    try {
      const a = await api<Article>(
        id ? "/articles/" + id : "/articles",
        id ? "PUT" : "POST",
        { ...form, ...(id ? { version } : {}) },
        session.csrf,
      );
      dirty.current = false;
      router.push("/posts/" + a.slug);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }
  if (loading) return <p>正在验证身份…</p>;
  if (session.user?.role !== "admin")
    return (
      <section className="card panel">
        <h1>仅管理员可编辑文章</h1>
        <Link href="/login">登录账号</Link>
      </section>
    );
  return (
    <>
      <Link href="/admin">返回管理列表</Link>
      <h1 className="page-title">{id ? "编辑文章" : "写一篇文章"}</h1>
      <ErrorMessage message={error} />
      {ready && (
        <form className="editor" onSubmit={save}>
          <div className="editor-fields">
            <label>
              文章标题
              <input
                className="input"
                value={form.title}
                required
                maxLength={160}
                onChange={(e) => change("title", e.target.value)}
              />
            </label>
            <label>
              地址标识（小写英文、数字、连字符）
              <input
                className="input"
                value={form.slug}
                required
                maxLength={120}
                pattern="[a-z0-9]+(-[a-z0-9]+)*"
                onChange={(e) => change("slug", e.target.value)}
              />
            </label>
            <label>
              分类
              <input
                className="input"
                value={form.category}
                required
                maxLength={40}
                onChange={(e) => change("category", e.target.value)}
              />
            </label>
            <label>
              摘要
              <textarea
                className="textarea"
                value={form.excerpt}
                maxLength={500}
                onChange={(e) => change("excerpt", e.target.value)}
              />
            </label>
          </div>
          <label className="file-import">
            导入 Markdown 文件
            <input
              className="file-input"
              type="file"
              accept=".md,.markdown,text/markdown,text/plain"
              onChange={async (e) => {
                const file = e.target.files?.[0];
                if (!file) return;
                if (file.size > 200000) {
                  setError("文件不能超过 200KB");
                  return;
                }
                change("content", await file.text());
                e.target.value = "";
              }}
            />
          </label>
          <div className="editor-columns">
            <label>
              Markdown 正文
              <textarea
                className="textarea code-editor"
                value={form.content}
                required
                maxLength={200000}
                onChange={(e) => change("content", e.target.value)}
                placeholder={"## 从这里开始\n\n记录值得分享的想法。"}
              />
            </label>
            <section className="preview">
              <h2>阅读预览</h2>
              <Markdown content={form.content || "正文预览将显示在这里。"} />
            </section>
          </div>
          <button className="btn btn-primary" disabled={busy || !session.csrf}>
            <Save size={16} aria-hidden="true" />
            {busy ? "正在保存…" : id ? "保存修改" : "发布文章"}
          </button>
          <p className="muted">离开前请保存。原始 HTML 不会执行。</p>
        </form>
      )}
    </>
  );
}
```
{% endraw %}

<a id="file-22"></a>

### src/components/home-hero.tsx

首页 Hero 文案与阅读锚点。

代码位置：`src/components/home-hero.tsx`（完整文件）。

{% raw %}
```tsx
/** 纯 CSS 入场动效，无需额外动画库；减少动态效果时直接显示完整文字。 */
export function HomeHero() {
  return (
    <section className="hero home-hero" aria-labelledby="home-title">
      <div className="hero-content home-hero-content">
        <h1 id="home-title" aria-label="落笔成文，拾页成光。">
          <span className="hero-line">
            <span>落笔成文，</span>
          </span>
          <span className="hero-line">
            <span>
              拾页<span className="hero-accent">成光。</span>
            </span>
          </span>
        </h1>
        <p className="hero-description">记录技术与生活。</p>
        <a className="btn btn-outline hero-read" href="#articles">
          开始阅读
        </a>
      </div>
    </section>
  );
}
```
{% endraw %}

<a id="file-23"></a>

### src/components/theme-toggle.tsx

主题偏好同步与圆形切换动画。

代码位置：`src/components/theme-toggle.tsx`（完整文件）。

{% raw %}
```tsx
"use client";
import { useEffect, useRef, useState, type MouseEvent } from "react";
import { Sun, Moon } from "lucide-react";
import { THEME_KEY, LIGHT_THEME, DARK_THEME } from "@/lib/theme";

export function ThemeToggle() {
  const [dark, setDark] = useState(false),
    [busy, setBusy] = useState(false);
  const inFlight = useRef(false);
  useEffect(() => {
    const media = matchMedia("(prefers-color-scheme: dark)");
    const apply = (theme: string) => {
      document.documentElement.dataset.theme = theme;
      setDark(theme === DARK_THEME);
    };
    const sync = () => {
      let saved: string | null = null;
      try {
        saved = localStorage.getItem(THEME_KEY);
      } catch {}
      apply(
        saved === LIGHT_THEME || saved === DARK_THEME
          ? saved
          : media.matches
            ? DARK_THEME
            : LIGHT_THEME,
      );
    };
    sync();
    media.addEventListener("change", sync);
    const storage = (e: StorageEvent) => {
      if (e.key === THEME_KEY || e.key === null) sync();
    };
    window.addEventListener("storage", storage);
    return () => {
      media.removeEventListener("change", sync);
      window.removeEventListener("storage", storage);
    };
  }, []);
  async function toggle(event: MouseEvent<HTMLButtonElement>) {
    if (inFlight.current) return;
    const root = document.documentElement;
    const next = root.dataset.theme === DARK_THEME ? LIGHT_THEME : DARK_THEME;
    const apply = () => {
      root.dataset.theme = next;
      setDark(next === DARK_THEME);
      try {
        localStorage.setItem(THEME_KEY, next);
      } catch {}
    };
    if (
      !document.startViewTransition ||
      matchMedia("(prefers-reduced-motion: reduce)").matches
    ) {
      apply();
      return;
    }
    const rect = event.currentTarget.getBoundingClientRect();
    const x = event.detail === 0 ? rect.left + rect.width / 2 : event.clientX;
    const y = event.detail === 0 ? rect.top + rect.height / 2 : event.clientY;
    const radius = Math.hypot(
      Math.max(x, innerWidth - x),
      Math.max(y, innerHeight - y),
    );
    inFlight.current = true;
    setBusy(true);
    try {
      const transition = document.startViewTransition(apply);
      // 页面隐藏或其他过渡抢占时 finished 也可能拒绝，及时处理。
      const finished = transition.finished.catch(() => {});
      await transition.ready;
      await root.animate(
        {
          clipPath: [
            `circle(0px at ${x}px ${y}px)`,
            `circle(${radius}px at ${x}px ${y}px)`,
          ],
        },
        {
          duration: 580,
          easing: "cubic-bezier(.4,0,.2,1)",
          pseudoElement: "::view-transition-new(root)",
        },
      ).finished;
      await finished;
    } catch {
      apply();
    } finally {
      inFlight.current = false;
      setBusy(false);
    }
  }
  return (
    <button
      type="button"
      className="btn btn-ghost btn-circle theme-toggle"
      aria-label={dark ? "切换浅色主题" : "切换深色主题"}
      aria-pressed={dark}
      disabled={busy}
      onClick={toggle}
    >
      {dark ? (
        <Sun size={20} aria-hidden="true" />
      ) : (
        <Moon size={20} aria-hidden="true" />
      )}
    </button>
  );
}
```
{% endraw %}

<a id="file-24"></a>

### src/lib/posts.ts

仅供 seed 导入的示例文章。

代码位置：`src/lib/posts.ts`（完整文件）。

{% raw %}
```typescript
import type { Post } from "@/types/post";

// 当前阶段的唯一数据源。未来可用 API 返回的数据替换。
export const posts: Post[] = [
  {
    slug: "nextjs-first-page",
    title: "从一张白纸开始，搭建我的第一个 Next.js 博客",
    excerpt: "理解路由、组件和渲染，把零散的知识串成一个真正能运行的小项目。",
    category: "前端开发",
    date: "2026-11-17",
    minutes: 8,
    tags: ["Next.js", "React", "入门"],
    cover: "next",
    sections: [
      {
        heading: "为什么从博客开始？",
        paragraphs: [
          "博客虽然小，却包含列表、详情、导航和数据展示这些常见需求。我们先用静态数据把页面搭起来，再逐步接入 API 和数据库。每一步都有可以看见的结果。",
          "React 负责把数据变成界面；Next.js 在 React 之上提供路由、构建和渲染能力。本项目使用 App Router，页面放在 src/app 目录下。",
        ],
      },
      {
        heading: "一个文件，就是一个页面",
        paragraphs: [
          "src/app/page.tsx 对应首页。组件是返回 JSX 的函数，JSX 看起来像 HTML，但允许你用花括号嵌入 JavaScript 表达式。",
        ],
        code: "export default function Home() {\n  return <h1>你好，我的博客</h1>;\n}",
      },
      {
        heading: "让文章拥有自己的地址",
        paragraphs: [
          "posts/[slug]/page.tsx 中的方括号表示动态路由。slug 是文章的可读标识，例如 nextjs-first-page。因为使用静态导出，我们通过 generateStaticParams 告诉 Next.js 需要提前生成哪些文章。",
          "本项目的文章正文在构建时生成 HTML。搜索和分类切换则由浏览器里的客户端组件完成。这两种方式可以在同一个页面中组合。",
        ],
        code: "export function generateStaticParams() {\n  return posts.map((post) => ({ slug: post.slug }));\n}",
      },
      {
        heading: "把第一步走扎实",
        paragraphs: [
          "试着修改文章标题，然后新增一条数据。你会发现首页列表和文章详情都跟着变化了：数据与展示分离，就是组件化带来的第一个好处。",
          "下一步阅读项目 docs 目录下的教学文档，跟着实际文件逐段理解，再尝试自己添加一个页面。",
        ],
      },
    ],
  },
  {
    slug: "typescript-data-model",
    title: "给数据一份说明书：TypeScript 入门",
    excerpt: "从一篇文章的数据结构出发，理解类型、接口与可选属性。",
    category: "前端开发",
    date: "2026-11-16",
    minutes: 6,
    tags: ["TypeScript", "类型系统"],
    cover: "typescript",
    sections: [
      {
        heading: "类型是开发时的约定",
        paragraphs: [
          "JavaScript 允许变量持有不同类型的值。TypeScript 为它增加静态类型检查，让你在运行之前发现常见错误，例如把字符串传给需要数字的地方。类型会在编译时被移除，不会替你校验网络返回的数据。",
        ],
        code: 'const minutes: number = 6;\nconst tags: string[] = ["TypeScript", "入门"];',
      },
      {
        heading: "用接口描述文章",
        paragraphs: [
          "Post 接口描述所有文章必须提供的字段。组件拿到 Post 后，编辑器就知道有哪些属性，也能提示拼写错误。Section 中的 code 是可选属性，因为不是每一节都有示例代码。",
        ],
        code: "interface Section {\n  heading: string;\n  paragraphs: string[];\n  code?: string;\n}",
      },
      {
        heading: "缩小可能性",
        paragraphs: [
          "Category 使用字符串联合类型限制分类。如果把前端开发写成前端开法，TypeScript 会直接报错。联合类型尤其适合分类、状态和固定选项。",
          "练习：给 Post 添加一个可选的 description 字段，并在卡片里处理它不存在的情况。",
        ],
      },
    ],
  },
  {
    slug: "tailwind-layout",
    title: "少写一点 CSS，多理解一点布局",
    excerpt: "用 Tailwind CSS 搭一张文章卡片，让响应式布局变得直观。",
    category: "前端开发",
    date: "2026-11-15",
    minutes: 5,
    tags: ["Tailwind CSS", "CSS"],
    cover: "tailwind",
    sections: [
      {
        heading: "从一个工具类开始",
        paragraphs: [
          "Tailwind 提供小而专一的工具类。p-6 控制内边距，rounded-2xl 控制圆角，flex 启用弹性布局。它没有替代 CSS 的原理，而是提供一种组合 CSS 的方式。",
        ],
        code: '<article className="rounded-2xl border p-6">\n  <h2 className="text-xl font-bold">文章标题</h2>\n</article>',
      },
      {
        heading: "从手机开始设计",
        paragraphs: [
          "grid-cols-1 是基础布局；md:grid-cols-2 只在中等及以上屏幕应用。先让小屏幕可读，再在更宽的空间增加列数。",
          "本项目的文章网格在手机上显示一列，在中等屏幕显示两列，宽屏下与右侧栏共同构成阅读布局。",
        ],
      },
      {
        heading: "交互状态也是样式",
        paragraphs: [
          "hover: 表示鼠标悬停状态，focus-visible: 适合键盘焦点反馈。不要仅凭颜色表达选中状态，还应使用 aria-pressed 等语义属性。",
          "练习：调整卡片圆角、间距和标题字号，然后观察不同宽度下的变化。",
        ],
      },
    ],
  },
  {
    slug: "database-first-look",
    title: "文章最后存在哪里？认识 PostgreSQL",
    excerpt: "从一张文章表开始，建立对数据库和持久化的第一印象。",
    category: "后端探索",
    date: "2026-11-14",
    minutes: 7,
    tags: ["PostgreSQL", "TypeORM"],
    cover: "database",
    sections: [
      {
        heading: "从内存到持久化",
        paragraphs: [
          "当前文章写在 TypeScript 文件中，修改后需要重新构建。数据库可以让系统在运行时保存内容，服务重启后数据依然存在。",
          "PostgreSQL 是关系型数据库。可以把文章理解为一张表，每篇文章是一行，标题、正文和发布时间是列。",
        ],
      },
      {
        heading: "TypeORM 的位置",
        paragraphs: [
          "TypeORM 帮助后端把 TypeScript 实体与数据库表建立映射。Repository 提供读取和保存实体的接口。数据库操作仍然发生在服务端，连接密码不应放进浏览器代码。",
          "当前 Demo 没有连接数据库。正式接入时，先设计表结构与迁移，再实现 Express API。",
        ],
      },
      {
        heading: "一步一步接入",
        paragraphs: [
          "先实现只读文章列表和详情接口，验证错误和空状态，再实现发布功能。发布涉及身份认证、输入校验和权限控制，不宜直接开放写入接口。",
        ],
      },
    ],
  },
  {
    slug: "express-request",
    title: "一次请求的旅程：浏览器到 Express",
    excerpt: "拆开请求与响应，看看前端和后端是如何对话的。",
    category: "后端探索",
    date: "2026-11-13",
    minutes: 6,
    tags: ["Express", "HTTP"],
    cover: "express",
    sections: [
      {
        heading: "前后端如何对话",
        paragraphs: [
          "浏览器通过 HTTP 请求访问后端接口，后端返回 JSON。前端读取这些数据，再用组件展示。路由 /api/posts 通常用于文章列表，/api/posts/:slug 用于单篇文章。",
        ],
      },
      {
        heading: "最小只读接口",
        paragraphs: [
          "下面是后续阶段的示意代码，并未集成进当前静态 Demo。Express 接收请求后，将查询结果作为 JSON 返回。",
        ],
        code: 'app.get("/api/posts", (_req, res) => {\n  res.json({ data: posts });\n});',
      },
      {
        heading: "别忘了失败的情况",
        paragraphs: [
          "请求可能失败，文章也可能不存在。前端需要加载中、空列表和错误状态；后端需要明确的状态码与统一的错误响应。",
          "本项目已经实现搜索无结果提示和文章不存在页面，可以用这些界面作为下一阶段的基础。",
        ],
      },
    ],
  },
  {
    slug: "intern-first-week",
    title: "实习第一周：把问题拆小，把进步记下来",
    excerpt: "不必一次掌握所有技术。让每一天都有一个可以验证的小目标。",
    category: "成长手记",
    date: "2026-11-12",
    minutes: 4,
    tags: ["实习", "学习方法"],
    cover: "notes",
    sections: [
      {
        heading: "先跑起来，再问为什么",
        paragraphs: [
          "刚接触新项目时，先找到启动命令、入口文件和数据来源。成功修改一个标题，就已经走通从源代码到页面的完整过程。",
        ],
      },
      {
        heading: "让问题更具体",
        paragraphs: [
          "遇到问题时，记录你期望的行为、实际结果、报错信息和尝试过的方法。具体的问题更容易得到帮助，也更容易被自己解决。",
        ],
      },
      {
        heading: "给每一天一个小目标",
        paragraphs: [
          "第一天学会组件和 props；第二天理解类型；第三天完成响应式卡片；第四天实现筛选；第五天复盘整个数据流。",
          "能解释自己写下的代码，比一次堆出很多功能更重要。",
        ],
      },
    ],
  },
];

export function getPost(slug: string): Post | undefined {
  return posts.find((post) => post.slug === slug);
}

export function formatDate(date: string): string {
  return date.replaceAll("-", ".");
}
```
{% endraw %}

<a id="file-25"></a>

### src/lib/theme.ts

主题常量与首帧初始化脚本。

代码位置：`src/lib/theme.ts`（完整文件）。

{% raw %}
```typescript
export const THEME_KEY = "fieldnotes-theme";
export const LIGHT_THEME = "fieldnotes";
export const DARK_THEME = "fieldnotes-dark";
// 在首帧绘制前应用偏好，避免刷新深色页面时先闪现浅色背景。
// 只包含项目常量，不插入用户输入。
export const themeInitScript = `(()=>{let theme;try{theme=localStorage.getItem('${THEME_KEY}')}catch{}if(theme!=='${LIGHT_THEME}'&&theme!=='${DARK_THEME}')theme=matchMedia('(prefers-color-scheme: dark)').matches?'${DARK_THEME}':'${LIGHT_THEME}';document.documentElement.dataset.theme=theme})()`;
```
{% endraw %}

<a id="file-26"></a>

### src/types/post.ts

seed 示例文章的结构类型。

代码位置：`src/types/post.ts`（完整文件）。

{% raw %}
```typescript
// 联合类型：分类只能取下面三个值，拼错时编译器会提醒。
export type Category = "前端开发" | "后端探索" | "成长手记";

export interface Section {
  heading: string;
  paragraphs: string[];
  code?: string; // ? 表示这个字段可以省略。
}

export interface Post {
  slug: string;
  title: string;
  excerpt: string;
  category: Category;
  date: string;
  minutes: number;
  tags: string[];
  cover: "next" | "typescript" | "tailwind" | "database" | "express" | "notes";
  sections: Section[];
}
```
{% endraw %}

<a id="file-27"></a>

### package.json

依赖与运行、检查、数据库管理命令。

代码位置：`package.json`（完整文件）。

{% raw %}
```json
{
  "name": "fieldnotes-blog",
  "version": "1.0.0",
  "private": true,
  "scripts": {
    "dev": "concurrently -k \"pnpm dev:api\" \"pnpm dev:web\"",
    "build": "next build --webpack",
    "typecheck": "tsc --noEmit",
    "dev:web": "next dev --webpack",
    "dev:api": "tsx watch server/index.ts",
    "start": "concurrently -k \"pnpm start:api\" \"pnpm start:web\"",
    "start:api": "tsx server/production.ts",
    "start:web": "next start",
    "db:migrate": "tsx server/manage.ts migrate",
    "db:seed": "tsx server/manage.ts seed",
    "admin": "tsx server/manage.ts admin",
    "test:api": "tsx --test server/app.test.ts"
  },
  "dependencies": {
    "bcryptjs": "^3.0.3",
    "connect-pg-simple": "^10.0.0",
    "dotenv": "^18.0.10",
    "express": "^5.3.0",
    "express-rate-limit": "^8.7.4",
    "express-session": "^1.19.0",
    "helmet": "^8.3.0",
    "lucide-react": "^0.468.0",
    "next": "^16.0.0",
    "pg": "^8.23.1",
    "react": "^19.2.0",
    "react-dom": "^19.2.0",
    "react-markdown": "^10.1.0",
    "reflect-metadata": "^0.2.2",
    "rehype-highlight": "^7.0.2",
    "remark-gfm": "^4.0.1",
    "typeorm": "~0.3.31",
    "zod": "^4.6.7"
  },
  "devDependencies": {
    "@tailwindcss/postcss": "^4.1.0",
    "@types/connect-pg-simple": "^7.0.3",
    "@types/express": "^5.0.6",
    "@types/express-session": "^1.19.0",
    "@types/node": "^22.0.0",
    "@types/pg": "^8.23.1",
    "@types/react": "^19.2.0",
    "@types/react-dom": "^19.2.0",
    "@types/supertest": "^7.2.1",
    "concurrently": "^10.0.7",
    "daisyui": "^5.8.1",
    "postcss": "^8.5.0",
    "serve": "^14.2.4",
    "supertest": "^7.3.2",
    "tailwindcss": "^4.1.0",
    "tsx": "^4.23.15",
    "typescript": "^5.9.0"
  }
}
```
{% endraw %}

<a id="file-28"></a>

### tsconfig.json

TypeScript 严格检查、模块解析与路径别名。

代码位置：`tsconfig.json`（完整文件）。

{% raw %}
```json
{
  "compilerOptions": {
    "target": "ES2017",
    "lib": [
      "dom",
      "dom.iterable",
      "esnext"
    ],
    "allowJs": true,
    "skipLibCheck": true,
    "strict": true,
    "noEmit": true,
    "esModuleInterop": true,
    "module": "esnext",
    "moduleResolution": "bundler",
    "resolveJsonModule": true,
    "isolatedModules": true,
    "jsx": "react-jsx",
    "incremental": true,
    "plugins": [
      {
        "name": "next"
      }
    ],
    "paths": {
      "@/*": [
        "./src/*"
      ]
    }
  },
  "include": [
    "next-env.d.ts",
    "**/*.ts",
    "**/*.tsx",
    ".next/types/**/*.ts",
    ".next/dev/types/**/*.ts",
    ".next-fullstack/types/**/*.ts",
    ".next-fullstack/dev/types/**/*.ts"
  ],
  "exclude": [
    "node_modules"
  ]
}
```
{% endraw %}

<a id="file-29"></a>

### next.config.ts

构建目录与 Express 同源代理。

代码位置：`next.config.ts`（完整文件）。

{% raw %}
```typescript
import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  distDir: ".next-fullstack",
  async rewrites() {
    return [
      {
        source: "/api/:path*",
        destination: `${process.env.API_ORIGIN || "http://127.0.0.1:4000"}/api/:path*`,
      },
    ];
  },
};

export default nextConfig;
```
{% endraw %}

<a id="file-30"></a>

### next-env.d.ts

Next.js 生成的类型入口；通常由框架维护。

代码位置：`next-env.d.ts`（完整文件）。

{% raw %}
```typescript
/// <reference types="next" />
/// <reference types="next/image-types/global" />
import "./.next-fullstack/types/routes.d.ts";
import "./.next-fullstack/types/root-params.d.ts";

// NOTE: This file should not be edited
// see https://nextjs.org/docs/app/api-reference/config/typescript for more information.
```
{% endraw %}

<a id="file-31"></a>

### postcss.config.mjs

Tailwind PostCSS 插件。

代码位置：`postcss.config.mjs`（完整文件）。

{% raw %}
```javascript
export default { plugins: { "@tailwindcss/postcss": {} } };
```
{% endraw %}

<a id="file-32"></a>

### pnpm-workspace.yaml

pnpm 原生构建依赖配置。

代码位置：`pnpm-workspace.yaml`（完整文件）。

{% raw %}
```yaml
allowBuilds:
  esbuild: true
```
{% endraw %}

<a id="file-33"></a>

### .npmrc

依赖仓库地址。

代码位置：`.npmrc`（完整文件）。

{% raw %}
```text
registry=https://registry.npmjs.org/
```
{% endraw %}

<a id="file-34"></a>

### .gitignore

本地密钥、依赖、构建产物与导出副本忽略规则。

代码位置：`.gitignore`（完整文件）。

{% raw %}
```text
node_modules/
.next/
out/
*.tsbuildinfo
.env*
!.env.example
.tools/

.pnpm-store/
.idea/

.next-fullstack/

# Generated copies published in the separate teaching blog
docs/dink-*/
```
{% endraw %}

<a id="file-35"></a>

### .env.example

环境变量占位示例，无真实凭据。

代码位置：`.env.example`（完整文件）。

{% raw %}
```text
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000
```
{% endraw %}

<a id="file-36"></a>

### compose.yaml

可选的 PostgreSQL 18 容器与持久卷。

代码位置：`compose.yaml`（完整文件）。

{% raw %}
```yaml
services:
  db:
    image: postgres:18
    environment:
      POSTGRES_DB: fieldnotes
      POSTGRES_USER: blog
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD:?Set POSTGRES_PASSWORD in .env}
    ports:
      - "127.0.0.1:5432:5432"
    volumes:
      - blog_pg:/var/lib/postgresql
volumes:
  blog_pg:
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-14-tests %})
