---
layout: post
title: TypeORM 查询（八）：把文章和评论的每一个 API 翻译成数据库操作
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: TypeORM 查询（八）：把文章和评论的每一个 API 翻译成数据库操作：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. Repository 方法先逐个认识

`getRepository(ArticleEntity)` 获取针对 articles 的操作对象，不是马上执行 SELECT。`create(input)` 生成实体对象，不入库；`save(input)` 才持久化，有主键等条件时可能更新，因此创建接口用 strict schema 拒绝 id 等多余字段。`findOneBy` 返回一个实体或 null；`existsBy` 返回布尔值；`update`/`delete` 返回操作结果，用 affected 判断是否命中，不能误以为它们返回完整文章。

QueryBuilder 用来逐步拼查询。命名参数 `:id`、`:q` 与 `{ id, q }` 分开传，避免拼接用户输入为 SQL。原始 db.query 用 `$1`、`$2` 占位，与 QueryBuilder 的命名占位是两种不同接口，不要混用。

## 2. GET /api/articles：搜索、分页、总数

先解析 query：q 默认空、最多100字符；category默认空、最多40；page强制转数值、整数、至少1。创建别名 a，q 非空时在 title/excerpt/content 上用 ILIKE；category 非空时增加精确条件。多个条件用 andWhere，所以分类和搜索同时满足。

搜索值先把反斜杠、百分号和下划线转义，再包 `%...%`。参数化解决 SQL 注入，LIKE 通配符转义解决“用户输入 % 却匹配全部文章”的语义问题，二者不是一回事。

select 只取卡片需要字段，不含大段 content，但搜索仍可以使用 content。createdAt DESC 再 id ASC 保持同时间文章的稳定顺序。skip=(page-1)*10，take=10；getManyAndCount 得到当前页实体和全部匹配总数。最后 res.json 返回 items/total/page/pageSize。

当前前端的列表仍复用含 content 的 Article 类型，尽管响应不含 content；界面没有读取该字段才暂不出错。严谨扩展时应增加 ArticleSummary 类型，本文如实说明这一类型边界，不把泛型当作运行时证明。

## 3. GET /api/categories 与 GET /api/articles/:slug

分类查询 select category as name、COUNT(*)::int as count、groupBy(category)、orderBy(category)，用 getRawMany 返回聚合行，非完整 Article 实体。前端目前只使用 name，count 是可扩展信息。

详情按 slug findOneBy，null 返回404，有值返回包含 Markdown 正文的文章。id 是内部稳定身份，slug 是可读地址，两者不要替换使用；评论接口接受 id。

## 4. POST /api/articles：建立新文章

requireAdmin 先确认权限；articleInput.parse 校验 title/slug/excerpt/category/content；Repository.save 写入；返回201和新实体。前端从返回的 slug 跳转阅读页。slug 重复由唯一约束触发23505，错误中间件映射409，不能仅靠网页上检查是否重名。

## 5. PUT /api/articles/:id：为什么传 version

id 必须为 UUID；请求 schema 在文章字段外增加正整数 version。解构 `{ version, ...input }` 将用于条件的版本与要更新的内容分开。QueryBuilder 生成等价操作：

{% raw %}
```sql
UPDATE articles
SET title = $1, version = version + 1
WHERE id = $2 AND version = $3
RETURNING *;
```
{% endraw %}

这是缩减示意，实际代码还更新其他表单字段。关键是“检查版本”和“递增版本”在一条 SQL 内完成。用户甲乙都打开 version=1，甲先保存变2；乙再提交 version=1 时 affected=0，收到409，而不是覆盖甲的修改。UPDATE 无命中也可能是文章已删，当前响应合并为“已被修改或删除”。

## 6. DELETE /api/articles/:id

权限后解析 UUID，delete(id)，affected=0返回404；成功204无响应体。评论由数据库外键级联删除，所以前端确认文案明确包含评论。前端 api 对204直接返回，不能再强制调用 json()。

## 7. GET /api/articles/:id/comments

解析 articleId、page，按文章过滤，createdAt DESC，skip/take 每页30，getManyAndCount。随后 Promise.all 并行查询每条评论用户的 username，再合成返回对象；用户不存在时用“已注销用户”。当前实现存在 N+1 查询，最多一页30次额外查询；这适合讲清流程，后续优化可用 JOIN 或批量查用户，不能声称当前已使用联表优化。

该 GET 没有先验证文章是否存在，不存在文章通常得到空评论列表。POST 评论则明确检查文章存在。边界行为应以实际代码为准。

## 8. POST 评论与 DELETE 评论

POST：requireUser → 每分钟10次限流 → UUID/正文trim与长度1..2000校验 → existsBy文章 → save({articleId,userId:req.account.id,content}) → 201。用户id必须来自Session装载的账号，不能让浏览器提交userId冒充别人。

DELETE：requireUser → 根据评论id读取 → 不存在404 → 判断“管理员 或 作者本人” → delete →204。条件写成 `role !== admin && comment.userId !== req.account.id` 表示两种允许情况都不满足才拒绝。别把 && 随意改成 ||，否则会错误拒绝合法作者或管理员。

本章所有匿名路由回调都是方法，虽然没有 function 名字。通过“HTTP方法+路径”定位它们，比硬记匿名回调行号更可靠；源码卷提供完整 app.ts。

## 对照源码：server/app.ts（第 238–395 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: server/app.ts start: 238 end: 395 sha256: e0195ecd1c208d9467e437a4a534f4dc7902481a8ac906a3895cca6314ca2518 -->

{% raw %}
````ts
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %})
