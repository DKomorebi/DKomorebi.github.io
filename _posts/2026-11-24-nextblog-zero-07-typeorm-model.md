---
layout: post
title: TypeORM 建模（七）：实体、字段、迁移和数据库约束逐项解释
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: TypeORM 建模（七）：实体、字段、迁移和数据库约束逐项解释：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 同一个 User 为什么出现几种表示

前端 User 只有可展示的 id、username、email、role；后端 interface User 还包括 passwordHash、createdAt；UserEntity 是 TypeORM 运行时映射；users 是 PostgreSQL 物理表。四者职责不同，不应该把含密码哈希的后端对象直接当作前端 DTO。

`new EntitySchema<User>({...})` 中 `<User>` 是类型约束，花括号中的 name/tableName/columns 是实际运行时配置。当前项目用 EntitySchema，没有采用 @Entity/@Column 装饰器写法，因此不能从“看不到装饰器”判断没使用 TypeORM。

## 2. 公用 id 配置的每个键

`const id = { type: "uuid" as const, primary: true, generated: "uuid" as const }` 被三个 EntitySchema 复用。type 是 PostgreSQL UUID；primary 指定主键；generated 指定生成策略；as const 避免 TypeScript 把 "uuid" 扩大为普通 string。它不是固定 UUID 值，也不会让所有用户共用一个 id。

## 3. UserEntity、ArticleEntity、CommentEntity 字段

| 对象 | 关键字段 | 含义与约束位置 |
| --- | --- | --- |
| User | username、email、passwordHash、role、createdAt | email 唯一；role 默认 user；迁移另加 role CHECK；createDate 自动插入创建时间 |
| Article | slug、title、excerpt、content、category | slug 唯一；content 为 text；标题/摘要/分类的长度由请求 schema 与迁移分别约束 |
| Article | createdAt、updatedAt、version | createDate/updateDate 管时间；version 普通数值列，本项目显式条件更新，不是隐含的锁 |
| Comment | articleId、userId、content、createdAt | 两个 UUID 引用；外键写在 SQL 迁移里，实体未声明 relations |

String 通常映射为字符串列，但迁移有更明确长度。synchronize=false 时实体与真实表必须人工保持一致；改 interface 不会扩表。当前 CommentEntity 没有 `article: Article` 或 `user: User` 关系配置，所以不能直接要求 `relations: { user: true }` 就自动工作，评论作者现用额外查询得到。

## 4. Initial.up(q) 逐张表做了什么

up 接收 QueryRunner，通过 q.query 执行建表 SQL。users 用 UUID 主键、email UNIQUE、role CHECK；articles 约束 slug 唯一并建立 version 默认值；comments 的 articleId/userId 是 REFERENCES，ON DELETE CASCADE 让文章或用户删除时连同其评论删除；sessions 是 connect-pg-simple 所需结构，包含 sid、sess JSON、expire。

comments_article_idx 支持按文章和创建时间查评论；sessions_expire_idx 帮助过期会话清理。表名与列名要和使用它们的库一致；SQL 中带双引号的 "createdAt" 保留大小写，别误写成不带引号的 createdAt 后再惊讶 PostgreSQL 找不到列。

Initial.down 逆序删除依赖表，是回滚实现，**不要在有数据的博客随意执行**。本教程不会为试验而删除现有库。

## 5. 新增验证码时为什么再建一个迁移类

数据库已经记录执行过 Initial。若只修改它的 up，旧库不会再次运行，别人新建库和你的旧库就会结构不一致。因此新增 RegistrationCodes1795392000000，并在 db.ts migrations 数组登记。

这个 up 创建 registration_codes：email 主键、nonce、digest、expires_at、attempts、sent_at、window_at、sends；下篇注册业务会用 SQL 原子地修改这些字段。其 down 只删除自己新建的表。验证码表没有 EntitySchema，业务通过 db.query/事务 manager.query 操作，这仍属于使用 TypeORM 提供的连接与事务能力，并不要求所有表必须注册为实体。

## 6. 三层约束各有什么用

浏览器 required/pattern 提升体验；Zod 检查来自网络的输入；数据库唯一/外键/CHECK 保证并发与其他入口下的数据完整性。只用“先 SELECT 看 slug 有没有重复，再 INSERT”会有竞态：两个请求都查到没有，然后同时插入。最终唯一索引必须兜底，23505 再转409。

练习：想给文章加 published 字段，应依次考虑 TypeScript 数据类型、EntitySchema、增量迁移、Zod 写入 schema、查询过滤、前端表单和测试。只改 JSX 不能完成数据模型修改。当前项目没有这个字段，练习方案不要当成已完成功能。

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

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-06-database-start %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %})
