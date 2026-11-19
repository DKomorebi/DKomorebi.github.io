---
layout: post
title: NextBlog（九）：PostgreSQL 数据建模与迁移
date: 2026-11-20 00:09:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 数据库迁移、实体建模、文章持久化与管理员初始化。
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

数据库迁移、实体建模、文章持久化与管理员初始化。

> 本文是 NextBlog 全栈博客系列第 9 / 15 篇，内容对应当前全栈版本。

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

## 数据持久化的请求链路

页面通过 /api/articles 读取数据库，管理员保存文章后即可被读者看到。浏览器 → Next.js 同源代理 → Express → TypeORM → PostgreSQL，这是当前项目的数据流。

数据库包含 users、articles、comments、sessions 四张业务表。用户邮箱和文章 slug 有唯一约束；评论通过外键关联文章和用户，删除文章会级联删除评论。不要仅靠前端先查一次来防止重复，因为两个并发请求仍可能同时写入；数据库约束才是最后一道保证。

## EntitySchema 与迁移

entities.ts 用 EntitySchema 描述对象与表的映射，避免在初学阶段引入装饰器配置。db.ts 创建 DataSource，明确关闭 synchronize。migration.ts 才是创建表和索引的版本化操作，执行 pnpm db:migrate 后会记录迁移版本，不会每次启动重新建表。

## 初始化与授权

先创建自己的空数据库，根据 .env.example 创建本地 .env，生成至少 32 字符的随机 SESSION_SECRET。执行 pnpm db:migrate，再执行 pnpm db:seed 导入示例文章。seed 按 slug 检查是否存在，不会覆盖已经编辑过的同名文章。密码与真实连接信息不能提交到仓库。

在网页注册账号后运行 pnpm admin 你的注册邮箱，再刷新页面。注册接口始终创建 user，管理员由服务器终端明确授予；不会让第一个访客自动成为管理员。示例数据只在 seed 时导入；文章列表与正文在运行时从数据库读取。

## 练习

给文章增加一个可选字段时，分别思考实体、迁移、API 校验、前端类型和编辑器需要如何变化。不要只改实体并依赖 synchronize 自动修改现有数据。


## 本篇文件索引

- [server/entities.ts](#source-1)
- [server/db.ts](#source-2)
- [server/migration.ts](#source-3)
- [server/manage.ts](#source-4)
- [compose.yaml](#source-5)
- [.env.example](#source-6)

<a id="source-1"></a>

### server/entities.ts

以下为该文件的完整源码。

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

<a id="source-2"></a>

### server/db.ts

以下为该文件的完整源码。

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

<a id="source-3"></a>

### server/migration.ts

以下为该文件的完整源码。

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

<a id="source-4"></a>

### server/manage.ts

以下为该文件的完整源码。

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

<a id="source-5"></a>

### compose.yaml

以下为该文件的完整源码。

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

<a id="source-6"></a>

### .env.example

以下为该文件的完整源码。

{% raw %}
```text
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

[下一篇]({% post_url 2026-11-20-intern-blog-10-auth-api %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
