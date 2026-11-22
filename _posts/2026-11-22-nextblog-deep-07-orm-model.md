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
title: TypeORM（一）：实体、类型、表与迁移到底是什么
date: 2026-11-22 00:07:00 +0800
description: C · TypeORM：结合项目源码、逐段讲解和练习答案深入学习。
---

> C · TypeORM · 第 7 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## ORM 处在什么位置

PostgreSQL 保存行和列，业务代码操作 JavaScript 对象。TypeORM 在两者之间提供映射、查询与持久化 API；它不是数据库本身，也不会取代 PostgreSQL 服务。底层仍需要驱动连接数据库并执行 SQL。


代码位置：`server/entities.ts`（完整文件）。

{% raw %}
````typescript
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


先看 interface Article：它列出 TypeScript 代码中的字段与类型。它不会创建表，因为接口在运行时不承担表结构定义。再看 ArticleEntity：new EntitySchema 构造一个真实运行时映射对象，告诉 TypeORM 哪个表、哪些列以及如何处理值。

## 一行一行理解映射

name 是实体标识，tableName 是数据库表名。columns 中 id 对应 UUID 主键，slug 标注唯一性，content 显式使用 text，createdAt 与 updatedAt 映射为带时区时间，version 初始为 1。

`type: String` 中 String 是 JavaScript 构造函数值；interface 的 `title: string` 中 string 是 TypeScript 类型。它们处在两个不同层面。`as const` 帮助保留字面量类型，便于复用 id 定义，不是数据库 SQL 的 const。

EntitySchema 是 TypeORM 支持的定义方式，另一类教程常见 @Entity、@Column 装饰器。本项目采用前者，所以没有装饰器不代表没有 ORM。阅读别人的装饰器示例时，先找它与这里的 tableName、columns、primary 的对应关系。

## 有实体，为什么还要 migration.ts

实体描述代码怎样使用表，迁移描述数据库怎样被修改。当前 synchronize 为 false，修改实体不会自动新增列。实际 varchar 长度、CHECK、外键与索引由 migration.ts 中的 SQL 创建，所以审查数据库约束时要同时看迁移，不能只看 EntitySchema。

以评论为例：实体保存 articleId 和 userId 这两个标量 UUID 字段，代码没有通过关系装饰器加载关联对象；外键和级联删除写在迁移 SQL 中。关系在数据库存在，不意味着 ORM 一定配置了对象形式的关系属性。

| 概念 | 实际文件 | 生效时机 |
| --- | --- | --- |
| interface Article | entities.ts | 类型检查 |
| ArticleEntity | entities.ts | ORM 初始化与查询 |
| articles 表 | PostgreSQL | 数据持久化 |
| CREATE TABLE | migration.ts | 执行迁移时 |
| 列表 Article 类型 | full-blog.tsx | 浏览器代码的类型检查 |

## 前后端同名类型为什么不完全相同

服务器实体的日期是 Date，HTTP JSON 中日期变成字符串，前端类型因此使用 string。列表查询还省略正文，详情查询返回完整文章。当前前端复用 Article 类型是较宽泛的实现，改进时可拆 ArticleSummary 和 ArticleDetail；不能因为名称相同就认为网络数据自动验证过。

## 增加字段时的推演

假设新增阅读次数，应考虑：数据库默认值与迁移、实体字段、接口是否允许客户端修改、返回字段、前端类型和展示。只往 interface 加一个 number 不会创建列；只写 SQL 加列也不会自动出现页面标签。

练习：在纸上写出以上六处需要改变的职责，不必现在修改数据库。参考答案重点是让服务器决定计数逻辑，不能把所有客户端提交的对象不加校验交给 ORM 保存。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-06-express-lifecycle %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-08-orm-crud %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [TypeORM Repository API](https://typeorm.io/docs/working-with-entity-manager/repository-api/)
