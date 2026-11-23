---
layout: post
title: 后端源码卷（二十一）：API、邮箱、事务、迁移、测试和配置
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 后端源码卷（二十一）：API、邮箱、事务、迁移、测试和配置：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

本篇完整归档server目录与核心配置。测试中的内存mailer与可控时钟是测试注入，不是网页可调用的验证码后门。环境文件只归档.env.example占位值，不公开本机数据库密码、SMTP授权码或Session密钥。

依赖精确版本还应以仓库pnpm-lock.yaml为准；不要通过手工复制node_modules安装。next-env.d.ts由Next生成，未作为手写业务代码归档，开发与构建过程可能使其生成路径变化。

## 对照源码：server/app.test.ts（第 1–247 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/app.test.ts start: 1 end: 247 sha256: 3896956fc7fad2bfe4ca9674d2c8087e38d7a6562cf091ee7b257c32bb988491 -->

{% raw %}
````ts
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
  const codes = new Map<string, string>();
  const mailer = async (email: string, code: string) => { codes.set(email, code); };
  let service = createApp(db, {
    mailer,
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
    await user.post("/api/auth/register/code")
      .set("Origin", origin).set("X-CSRF-Token", init)
      .send({ email: "reader@example.com" }).expect(200);
    r = await user
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", init)
      .send({
        email: "reader@example.com",
        username: "读者",
        password: "Testing-password-123",
        confirmPassword: "Testing-password-123",
        verificationCode: codes.get("reader@example.com"),
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
    await other.post("/api/auth/register/code")
      .set("Origin", origin).set("X-CSRF-Token", otherCsrf)
      .send({ email: "other@example.com" }).expect(200);
    r = await other
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", otherCsrf)
      .send({
        email: "other@example.com",
        username: "另一位读者",
        password: "Testing-password-456",
        confirmPassword: "Testing-password-456",
        verificationCode: codes.get("other@example.com"),
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
````
{% endraw %}

## 对照源码：server/app.ts（第 1–434 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/app.ts start: 1 end: 434 sha256: 6d6bdecec7db577a0a989a92a071b1485bd2b57608efe84d7a4df55634f0d3e4 -->

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
  app.post("/api/auth/register/code", rateLimit({
    windowMs: 60 * 60 * 1000,
    limit: 10,
    standardHeaders: "draft-7",
    legacyHeaders: false,
    message: { message: "验证码发送过于频繁，请稍后重试" },
  }), async (req, res) => {
    const { email } = credentials.pick({ email: true }).strict().parse(req.body);
    await registration.sendCode(email);
    res.json({ message: "若邮箱尚未注册，验证码将发送到该邮箱，5 分钟内有效；已有账号请直接登录。", retryAfter: 60 });
  });
  app.post("/api/auth/register", authLimit, async (req, res) => {
    const input = credentials
      .extend({
        username: z.string().trim().min(2).max(40),
        confirmPassword: z.string().min(10).max(72),
        verificationCode: z.string().regex(/^\d{6}$/, "请输入 6 位邮箱验证码"),
      })
      .strict()
      .refine((v) => v.password === v.confirmPassword, {
        message: "两次输入的密码不一致", path: ["confirmPassword"],
      })
      .parse(req.body);
    const user = await registration.register(input);
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
      if (error instanceof RegistrationError) {
        res.status(error.status).json({ message: error.message });
        return;
      }
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
````
{% endraw %}

## 对照源码：server/db.ts（第 1–17 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/db.ts start: 1 end: 17 sha256: 180bf05893c262e8a3a136930db853f257690b21921e366017b72439c46c60f9 -->

{% raw %}
````ts
import "reflect-metadata";
import "dotenv/config";
import { DataSource } from "typeorm";
import { UserEntity, ArticleEntity, CommentEntity } from "./entities";
import { Initial1795000000000 } from "./migration";
import { RegistrationCodes1795392000000 } from "./registration-migration";
export function createDatabase(url = process.env.DATABASE_URL) {
  if (!url) throw new Error("请在 .env 中设置 DATABASE_URL");
  return new DataSource({
    type: "postgres",
    url,
    entities: [UserEntity, ArticleEntity, CommentEntity],
    migrations: [Initial1795000000000, RegistrationCodes1795392000000],
    synchronize: false,
    logging: false,
  });
}
````
{% endraw %}

## 对照源码：server/entities.ts（第 1–66 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/entities.ts start: 1 end: 66 sha256: 6530a94ab9a22c766a8200e35f27fb8e571c9efe56b44251d8d2448b6373c705 -->

{% raw %}
````ts
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
````
{% endraw %}

## 对照源码：server/index.ts（第 1–32 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/index.ts start: 1 end: 32 sha256: 8b68e5bc0d580d13409dec0b9c0ab8c21a91ceecf1f1f0bd4d8eed25940248e3 -->

{% raw %}
````ts
import { createDatabase } from "./db";
import { createApp } from "./app";
import { createRegistrationMailer } from "./mail";
async function main() {
  const db = await createDatabase().initialize();
  const { app, close } = createApp(db, {
    secret: process.env.SESSION_SECRET ?? "",
    origin: process.env.APP_ORIGIN ?? "http://localhost:3000",
    production: process.env.NODE_ENV === "production",
    mailer: createRegistrationMailer(),
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

## 对照源码：server/mail.ts（第 1–36 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/mail.ts start: 1 end: 36 sha256: e2693a63532858a40f3bb7feb9b3af43bbfa30b76d8666d85ac0e868890df008 -->

{% raw %}
````ts
import nodemailer from "nodemailer";

export type RegistrationMailer = (email: string, code: string) => Promise<void>;

// 没有 SMTP 配置时不伪造成功，也不向浏览器或日志泄露验证码。
export function createRegistrationMailer(
  env: NodeJS.ProcessEnv = process.env,
): RegistrationMailer | undefined {
  if (!env.SMTP_HOST || !env.SMTP_USER || !env.SMTP_PASS || !env.SMTP_FROM)
    return undefined;
  const port = Number(env.SMTP_PORT || 587);
  if (!Number.isInteger(port) || port < 1 || port > 65535)
    throw new Error("SMTP_PORT 必须是有效端口");
  const transport = nodemailer.createTransport({
    host: env.SMTP_HOST,
    port,
    secure: port === 465,
    requireTLS: port !== 465,
    auth: { user: env.SMTP_USER, pass: env.SMTP_PASS },
    connectionTimeout: 10000,
    greetingTimeout: 10000,
    socketTimeout: 15000,
    disableFileAccess: true,
    disableUrlAccess: true,
  });
  return async (email, code) => {
    const result = await transport.sendMail({
      from: env.SMTP_FROM,
      to: email,
      subject: "拾页 · 注册邮箱验证码",
      text: `你的注册验证码是：${code}\n\n5 分钟内有效，仅可使用一次。请勿向他人提供验证码。\n如果不是你本人操作，请忽略此邮件。`,
    });
    if (!result.accepted.length || result.rejected.length)
      throw new Error("SMTP recipient rejected");
  };
}
````
{% endraw %}

## 对照源码：server/manage.ts（第 1–46 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/manage.ts start: 1 end: 46 sha256: 1444c35233c0a842eba85f5cda781712ead73afdaf18c3feb80237ebb24bb8db -->

{% raw %}
````ts
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
````
{% endraw %}

## 对照源码：server/migration.ts（第 1–16 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/migration.ts start: 1 end: 16 sha256: ef42cf9a9deaacffbd79b9b02c2db414352fd5cee7ada930f45aab40c052e642 -->

{% raw %}
````ts
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
````
{% endraw %}

## 对照源码：server/production.ts（第 1–2 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/production.ts start: 1 end: 2 sha256: dea4b6b9f83f9156fc251fb66d06bf5f8353291121d4392037536ac2a6e1079f -->

{% raw %}
````ts
Object.assign(process.env, { NODE_ENV: 'production' });
void import('./index');
````
{% endraw %}

## 对照源码：server/registration-migration.ts（第 1–20 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/registration-migration.ts start: 1 end: 20 sha256: 08f4c00f59f441fcb82cd19c5ad4805b81f068d10e4264413563cdd1942be2f1 -->

{% raw %}
````ts
import type { MigrationInterface, QueryRunner } from "typeorm";

export class RegistrationCodes1795392000000 implements MigrationInterface {
  async up(q: QueryRunner) {
    await q.query(`CREATE TABLE registration_codes (
      email varchar(255) PRIMARY KEY,
      nonce uuid NOT NULL,
      digest varchar(64),
      expires_at timestamptz,
      attempts integer NOT NULL DEFAULT 0,
      sent_at timestamptz NOT NULL,
      window_at timestamptz NOT NULL,
      sends integer NOT NULL DEFAULT 1
    );
    CREATE INDEX registration_codes_cleanup_idx ON registration_codes(sent_at);`);
  }
  async down(q: QueryRunner) {
    await q.query("DROP TABLE registration_codes");
  }
}
````
{% endraw %}

## 对照源码：server/registration.test.ts（第 1–125 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/registration.test.ts start: 1 end: 125 sha256: 08bc57c6f23d35faa559c89e2a1b157c6c3620c6b1049e1bb4b4ed1abb5c382e -->

{% raw %}
````ts
import { test } from "node:test";
import assert from "node:assert/strict";
import { randomBytes } from "node:crypto";
import { Client, Pool } from "pg";
import request from "supertest";
import { createDatabase } from "./db";
import { createApp } from "./app";
import { createRegistration, RegistrationError } from "./registration";
import { UserEntity } from "./entities";

test("registration: email ownership, expiry, replay, limits, concurrency and SMTP failure", async () => {
  const base = new URL(process.env.DATABASE_URL!);
  const database = "registration_test_" + randomBytes(6).toString("hex");
  const adminUrl = new URL(base);
  adminUrl.pathname = "/postgres";
  const controller = new Client({ connectionString: adminUrl.toString() });
  await controller.connect();
  await controller.query(`CREATE DATABASE "${database}"`);
  base.pathname = "/" + database;
  const db = await createDatabase(base.toString()).initialize();
  await db.runMigrations();
  let at = Date.now();
  const now = () => new Date(at);
  const codes = new Map<string, string>();
  let failMail = false;
  let sends = 0;
  const mailer = async (email: string, code: string) => {
    if (failMail) throw new Error("sensitive SMTP diagnostic");
    sends++;
    codes.set(email, code);
  };
  const secret = randomBytes(48).toString("hex");
  const origin = "http://localhost:3000";
  const service = createApp(db, { secret, origin, now, mailer, pool: new Pool({ connectionString: base.toString() }) });
  const registration = createRegistration(db, secret, mailer, now);
  try {
    const agent = request.agent(service.app);
    let csrf = (await agent.get("/api/auth/session")).body.csrf;
    const post = (path: string, body: object) => agent.post("/api/auth/" + path)
      .set("Origin", origin).set("X-CSRF-Token", csrf).send(body);
    const payload = (email: string, code = codes.get(email) ?? "000000") => ({
      email, username: "注册读者", password: "Testing-password-123",
      confirmPassword: "Testing-password-123", verificationCode: code,
    });
    await agent.post("/api/auth/register/code").send({ email: "reader@example.com" }).expect(403);
    await post("register/code", { email: "not-an-email" }).expect(400);
    await post("register", payload("reader@example.com")).expect(400);
    const sent = await post("register/code", { email: " Reader@Example.com " }).expect(200);
    assert.equal(sent.body.retryAfter, 60);
    assert.equal(sent.body.code, undefined);
    assert.equal(sent.body.verificationCode, undefined);
    const code = codes.get("reader@example.com")!;
    assert.match(code, /^\d{6}$/);
    const [record] = await db.query("SELECT * FROM registration_codes WHERE email = $1", ["reader@example.com"]);
    assert.equal(record.digest.length, 64);
    assert.notEqual(record.digest, code);
    await post("register/code", { email: "READER@example.com" }).expect(429);
    await post("register", { ...payload("reader@example.com"), confirmPassword: "different-password" }).expect(400);
    await post("register", { ...payload("reader@example.com"), confirmPassword: undefined }).expect(400);
    await post("register", { ...payload("reader@example.com"), verificationCode: undefined }).expect(400);
    await post("register", { ...payload("reader@example.com"), role: "admin" }).expect(400);
    await post("register", payload("different@example.com", code)).expect(400);
    assert.equal(await db.getRepository(UserEntity).count(), 0);
    const result = await post("register", payload("READER@EXAMPLE.COM", code)).expect(201);
    assert.equal(result.body.user.role, "user");
    assert.equal(result.body.user.passwordHash, undefined);
    csrf = result.body.csrf;
    await post("register", payload("reader@example.com", code)).expect(400);
    assert.equal((await agent.get("/api/auth/session")).body.user.email, "reader@example.com");

    // 5 次错误即失效；不能用第 6 次正确输入绕过。
    await registration.sendCode("locked@example.com");
    const wrong = codes.get("locked@example.com") === "000000" ? "000001" : "000000";
    for (let i = 0; i < 5; i++) await post("register", payload("locked@example.com", wrong)).expect(400);
    await post("register", payload("locked@example.com")).expect(400);
    assert.equal((await db.query("SELECT attempts FROM registration_codes WHERE email=$1", ["locked@example.com"]))[0].attempts, 5);

    await registration.sendCode("expired@example.com");
    at += 5 * 60 * 1000;
    await post("register", payload("expired@example.com")).expect(400);

    // 重发使之前的 nonce 失效；极小概率生成相同数字时重新发送，避免随机性造成假失败。
    await registration.sendCode("resend@example.com");
    const oldCode = codes.get("resend@example.com")!;
    do { at += 61000; await registration.sendCode("resend@example.com"); }
    while (codes.get("resend@example.com") === oldCode);
    await post("register", payload("resend@example.com", oldCode)).expect(400);
    const freshService = createRegistration(db, secret, mailer, now);
    await freshService.register(payload("resend@example.com")); // 重建服务仍可使用数据库里的验证码。

    // 同邮箱只能有一个并发发送成功。
    const concurrent = await Promise.allSettled([
      registration.sendCode("parallel@example.com"), registration.sendCode("parallel@example.com"),
    ]);
    assert.equal(concurrent.filter((v) => v.status === "fulfilled").length, 1);
    const signups = await Promise.allSettled([
      registration.register(payload("parallel@example.com")), registration.register(payload("parallel@example.com")),
    ]);
    assert.equal(signups.filter((v) => v.status === "fulfilled").length, 1);

    for (let i = 0; i < 5; i++) { await registration.sendCode("quota@example.com"); at += 61000; }
    await assert.rejects(registration.sendCode("quota@example.com"), (e) => e instanceof RegistrationError && e.status === 429);
    at += 3600000;
    await registration.sendCode("quota@example.com");
    const before = sends;
    await post("register/code", { email: "reader@example.com" }).expect(200);
    assert.equal(sends, before); // 已注册邮箱不发新验证码。

    failMail = true;
    const failed = await post("register/code", { email: "failed@example.com" }).expect(503);
    assert.ok(!JSON.stringify(failed.body).includes("sensitive"));
    assert.equal((await db.query("SELECT digest FROM registration_codes WHERE email=$1", ["failed@example.com"]))[0].digest, null);
    await post("register", payload("failed@example.com")).expect(400);
    await assert.rejects(createRegistration(db, secret).sendCode("no-config@example.com"), (e) => e instanceof RegistrationError && e.status === 503);
    // IP 级限流不可被轮换邮箱绕过（每次失败也计入）。
    for (let i = 0; i < 11; i++) await post("register/code", { email: `limit${i}@example.com` });
    await post("register/code", { email: "last@example.com" }).expect(429);
    assert.equal(await db.getRepository(UserEntity).count(), 3);
  } finally {
    await service.close();
    await db.destroy();
    await controller.query(`DROP DATABASE "${database}"`);
    await controller.end();
  }
});
````
{% endraw %}

## 对照源码：server/registration.ts（第 1–94 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/registration.ts start: 1 end: 94 sha256: a118cb010f68399f01fba7af7d01c94096e7a0257731ce5434c14804e76a81ee -->

{% raw %}
````ts
import { createHmac, randomInt, randomUUID, timingSafeEqual } from "node:crypto";
import bcrypt from "bcryptjs";
import type { DataSource } from "typeorm";
import { UserEntity } from "./entities";
import type { RegistrationMailer } from "./mail";

export class RegistrationError extends Error {
  constructor(public status: number, message: string) {
    super(message);
  }
}

export function createRegistration(
  db: DataSource,
  secret: string,
  mailer?: RegistrationMailer,
  now = () => new Date(),
) {
  const digest = (email: string, nonce: string, code: string) =>
    createHmac("sha256", secret)
      .update(JSON.stringify(["registration", email, nonce, code]))
      .digest("hex");

  return {
    async sendCode(email: string) {
      if (!mailer)
        throw new RegistrationError(503, "注册邮件服务尚未配置，请联系站点管理员");
      const at = now();
      const nonce = randomUUID();
      // 一条 SQL 原子预留发送名额：同邮箱并发请求也不能绕过冷却和小时上限。
      await db.query("DELETE FROM registration_codes WHERE sent_at < $1", [
        new Date(at.getTime() - 24 * 60 * 60 * 1000),
      ]);
      const rows = await db.query(
        `INSERT INTO registration_codes(email, nonce, sent_at, window_at)
         VALUES ($1, $2, $3, $3)
         ON CONFLICT(email) DO UPDATE SET
           nonce = $2, digest = NULL, expires_at = NULL, attempts = 0, sent_at = $3,
           window_at = CASE WHEN registration_codes.window_at <= $3::timestamptz - interval '1 hour'
             THEN $3 ELSE registration_codes.window_at END,
           sends = CASE WHEN registration_codes.window_at <= $3::timestamptz - interval '1 hour'
             THEN 1 ELSE registration_codes.sends + 1 END
         WHERE registration_codes.sent_at <= $3::timestamptz - interval '60 seconds'
           AND (registration_codes.sends < 5 OR registration_codes.window_at <= $3::timestamptz - interval '1 hour')
         RETURNING email`,
        [email, nonce, at],
      );
      if (!rows.length)
        throw new RegistrationError(429, "请勿频繁发送：间隔至少 60 秒，每个邮箱每小时最多 5 次");
      // 已注册邮箱也返回相同提示，避免发送接口直接暴露账号是否存在。
      if (await db.getRepository(UserEntity).existsBy({ email })) return;
      const code = randomInt(0, 1000000).toString().padStart(6, "0");
      try {
        await mailer(email, code);
      } catch {
        // 保留发送配额，失败时无有效验证码。不要把 SMTP 异常（可能含凭据）返回前端。
        throw new RegistrationError(503, "邮件发送失败，请稍后重试或联系站点管理员");
      }
      await db.query(
        `UPDATE registration_codes SET digest = $3, expires_at = $4
         WHERE email = $1 AND nonce = $2`,
        [email, nonce, digest(email, nonce, code), new Date(now().getTime() + 5 * 60 * 1000)],
      );
    },

    async register(input: { email: string; username: string; password: string; verificationCode: string }) {
      // 行锁使校验、创建用户、消费验证码成为一个事务；错误次数必须提交，不能抛异常回滚。
      const user = await db.transaction(async (manager) => {
        const [record] = await manager.query(
          "SELECT * FROM registration_codes WHERE email = $1 FOR UPDATE",
          [input.email],
        );
        if (!record?.digest || record.attempts >= 5 || !record.expires_at ||
          new Date(record.expires_at).getTime() <= now().getTime()) return null;
        const expected = Buffer.from(record.digest, "hex");
        const actual = Buffer.from(digest(input.email, record.nonce, input.verificationCode), "hex");
        if (expected.length !== actual.length || !timingSafeEqual(expected, actual)) {
          await manager.query("UPDATE registration_codes SET attempts = attempts + 1 WHERE email = $1", [input.email]);
          return null;
        }
        const repo = manager.getRepository(UserEntity);
        const created = await repo.save(repo.create({
          email: input.email, username: input.username,
          passwordHash: await bcrypt.hash(input.password, 12), role: "user",
        }));
        // 保留发送计数，但销毁有效凭证；任何重放都会失败。
        await manager.query("UPDATE registration_codes SET digest = NULL, expires_at = NULL WHERE email = $1", [input.email]);
        return created;
      });
      if (!user) throw new RegistrationError(400, "验证码错误、已过期或已失效，请检查邮箱或重新获取");
      return user;
    },
  };
}
````
{% endraw %}

## 对照源码：package.json（第 1–60 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: package.json start: 1 end: 60 sha256: 2d5bbf2228e6ede0175173a48e60a4237c35614ad43712cc60373e04d726e94d -->

{% raw %}
````json
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
    "test:api": "tsx --test server/app.test.ts server/registration.test.ts"
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
    "nodemailer": "^10.6.1",
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
    "@types/nodemailer": "^8.0.2",
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
````
{% endraw %}

## 对照源码：tsconfig.json（第 1–43 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: tsconfig.json start: 1 end: 43 sha256: 337bb4102ae158e18e87963ab723729a5ef1c6d054ad205ca509cae2d4fc38f6 -->

{% raw %}
````json
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
````
{% endraw %}

## 对照源码：next.config.ts（第 1–15 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: next.config.ts start: 1 end: 15 sha256: 79c7e1719755b639ca410a20d4e88993711b6a1a76ee58c86ab9b138eec086f3 -->

{% raw %}
````ts
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
````
{% endraw %}

## 对照源码：postcss.config.mjs（第 1–1 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: postcss.config.mjs start: 1 end: 1 sha256: 69aa80a4682c0d665069ef0a4109ec9e0c8618bef2bcaf967b9ec076f1a425db -->

{% raw %}
````js
export default { plugins: { "@tailwindcss/postcss": {} } };
````
{% endraw %}

## 对照源码：.env.example（第 1–13 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: .env.example start: 1 end: 13 sha256: 27a1658833e30aec2cd264ab753c5d0777b52fa432ab3d4cf7fdaaf90169218f -->

{% raw %}
````text
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000

# SMTP 注册邮件：587 使用 STARTTLS，465 使用隐式 TLS。
# SMTP_PASS 使用邮箱服务提供的 SMTP 授权码，不是网页登录密码。
SMTP_HOST=smtp.example.com
SMTP_PORT=587
SMTP_USER=sender@example.com
SMTP_PASS=replace-with-smtp-authorization-code
SMTP_FROM=sender@example.com
````
{% endraw %}

## 对照源码：.gitignore（第 1–15 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: .gitignore start: 1 end: 15 sha256: 091e26493cdce733e822849c562f10c75b9db38699a5f7302bf633622f7d71d8 -->

{% raw %}
````text
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-20-source-frontend %})
