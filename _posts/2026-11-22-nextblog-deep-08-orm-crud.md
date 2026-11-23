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
title: TypeORM（二）：Repository 怎样完成文章增删改查
date: 2026-11-22 00:08:00 +0800
description: C · TypeORM：结合项目源码、逐段讲解和练习答案深入学习。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


> C · TypeORM · 第 8 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## Repository 是针对一个实体的数据访问入口

db.getRepository(ArticleEntity) 根据已经初始化的 DataSource 取得文章仓库。它不会为每篇文章新建一个数据库，也不是 Git 仓库。可以把它理解成提供文章查询与写入操作的对象，底层复用 DataSource 管理的连接。

## 查询：findOneBy 的条件对象


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
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
````
{% endraw %}


findOneBy({ slug }) 表达“查找 slug 匹配的一条文章”。SQL 意图类似 SELECT ... FROM articles WHERE slug = 参数；这里的解释不是逐字生成 SQL。slug 在数据库有唯一约束，所以业务上最多对应一篇文章。

查询不到返回空结果，路由显式处理 404。await 之后才能拿到记录，不能对尚未完成的 Promise 读取 title。res.json 将结果发给前端，绝不应把用户密码哈希对象直接照样返回；认证路由使用 publicUser 筛选字段。

## 新增：save 之前必须校验


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
  app.post("/api/articles", requireAdmin, async (req, res) => {
    const input = articleInput.parse(req.body);
    res.status(201).json(await db.getRepository(ArticleEntity).save(input));
  });
````
{% endraw %}


这里 parse 只留下允许的文章字段，用户没有提供主键，所以 save 按新增路径保存并返回结果。save 是兼具插入与更新语义的高层 API，不能简单记成“永远 INSERT”。判断时要结合对象标识与已有记录。本项目更新文章使用专门的条件更新逻辑。

server/app.ts 注册路由中先 repo.create 再 repo.save 的模式也容易混淆：create 只创建符合实体形状的对象，并不单独写入数据库；save 才进行持久化。admin 命令使用 update 按邮箱修改角色，随后检查 affected 判断是否找到目标。

## 修改与删除：不能忽略结果


代码位置：`server/app.ts`（节选）。

{% raw %}
````typescript
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
````
{% endraw %}


修改路由使用 QueryBuilder，WHERE 同时匹配 id 和 version。返回的 affected 为零时，可能是记录已删除，也可能是版本已经变化，当前统一返回 409。成功时 returning 取得更新后的记录用于页面跳转。

删除路由按 UUID 删除文章，检查 affected，没找到就 404，成功则 204。204 没有 JSON 正文，所以前端 api 封装提前返回；如果无条件调用 response.json，成功删除反而可能被当成解析错误。

## 为什么删除文章会影响评论

不是 JavaScript 循环逐条删评论，而是数据库外键 ON DELETE CASCADE。关联约束在 migration.ts。这样即使删除不是来自网页按钮，数据库也会维护相同的关联规则。操作前的确认提示应准确说明文章及评论一起删除。

## 安全使用 ORM 的三个边界

先验证身份，再校验允许字段，最后执行数据库操作。ORM 不知道当前用户是不是管理员，save 不会自动拒绝越权请求，唯一约束也不替你决定谁能编辑文章。

数据库错误需要转换为业务结果：重复 slug 对应 409，不是让前端显示完整 SQL 堆栈；找不到记录对应 404；未知内部错误不直接暴露连接信息。

## 练习与答案

只对自己新增的练习文章操作，记录创建后的 id、slug、version，更新一次观察 version 增加，再删除观察 204。自测：将查询结果对象在内存中改 title 就保存了吗？没有；需要明确执行写入方法。调用 repo.create 保存了吗？也没有。

另一个问题：两个管理员同时修改时，用普通 save 是否自然实现本项目版本保护？不能这样假定；保护来自这里明确的 WHERE version 条件和原子递增。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-07-orm-model %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-09-query-migration %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [TypeORM Repository API](https://typeorm.io/docs/working-with-entity-manager/repository-api/)
