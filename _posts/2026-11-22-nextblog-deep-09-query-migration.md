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
title: TypeORM（三）：搜索、并发编辑、迁移与事务思维
date: 2026-11-22 00:09:00 +0800
description: C · TypeORM：结合项目源码、逐段讲解和练习答案深入学习。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


> C · TypeORM · 第 9 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 简单仓库方法不够时用 QueryBuilder


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
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
````
{% endraw %}


q 是搜索词，category 是可选分类，page 经 z.coerce 转成整数并要求至少为 1。qb 是查询构建器，a 是文章表别名。andWhere 根据是否有输入添加条件，最后 getManyAndCount 同时提供当前页记录与总数。

## 参数绑定和通配符转义不是一回事

`:q` 配合参数对象表达绑定值，避免将输入直接拼成 SQL 代码。ILIKE 的 % 和 _ 又有匹配语义，所以代码将输入中的这些字符及反斜杠转义，再在外侧加 %，表示按字面关键词做包含匹配。这两层分别处理 SQL 结构与搜索语义。

category 使用等值匹配，正文、标题、摘要使用不区分大小写的包含查询。当前不是全文搜索排名系统；文章量大时，应测量查询计划，再考虑合适的索引或专用搜索方案。

## 为什么列表没返回 content

select 只选择卡片需要的数据，减少正文传输。skip 计算跳过 (page-1)*10 条，take 限制当前页 10 条。排序先按创建时间倒序，再按 id 提供稳定次序，避免同一时间的记录顺序完全不确定。

这是基于 offset 的分页。数据在请求之间新增或删除时，跨页内容仍可能变化；不能把稳定排序等同于整个浏览过程的数据快照。当前教学项目没有实现游标分页。

## version 怎样防止覆盖

两个编辑器都读取 version=1，A 保存时 SQL 条件匹配成功并改成 2。B 再用 version=1 保存，affected 为零，因此返回 409。比较与写入在同一个数据库 UPDATE 中完成，避免先查版本再写入之间被插入另一次修改。

这叫乐观并发控制：允许同时编辑，在提交时检测冲突。它没有自动合并两份正文，前端要保留自己的输入，让用户重新加载比较。单纯把版本字段显示在页面上没有保护作用。

## 迁移怎样成为可重复管理的结构历史


代码位置：`server/migration.ts`（完整文件）。

{% raw %}
````typescript
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


up 创建表、外键、CHECK 与索引，down 描述逆向删除。manage.ts 的 runMigrations 执行未应用的迁移，并由 TypeORM 记录执行状态。正常修改结构应新增迁移，不要反复改已执行过的初始文件并期待数据库自动跟随。

当前 down 会删除业务表，属于破坏性回退逻辑，理解代码即可，不应当作普通“重试启动”的命令。增加字段时需要设计已有数据默认值、非空约束的实施顺序和回滚方案。

## 事务在这里怎样理解

单条 UPDATE 的原子性不等于多条业务操作天然同一事务。例如未来同时写文章与审计记录，如果要求同时成功或失败，应显式建立事务，并在回调内使用该事务提供的 manager/repository。当前代码并没有实现审计表事务，不能把这个建议当成现有功能。

## 练习与答案

用两页同时打开自己创建的练习文章，先后保存，确认后一页显示冲突且正文仍保留。然后阅读迁移，找出评论外键、邮箱唯一性和角色 CHECK。

自测：synchronize=false 时修改 EntitySchema 会建列吗？不会。绑定参数能自动把搜索中的 % 当普通字符吗？不一定，所以代码另做匹配语义转义。版本冲突应无条件自动重试吗？不能，否则可能掩盖用户尚未比较的内容差异。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-08-orm-crud %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-10-database-config %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [TypeORM Repository API](https://typeorm.io/docs/working-with-entity-manager/repository-api/)
