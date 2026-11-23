---
layout: post
title: DDD 实战（三）：把当前 TypeORM 博客逐步分层
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/DDD架构
description: DDD 实战（三）：把当前 TypeORM 博客逐步分层：结合真实模板与现有 NextBlog 源码逐步解释。
keywords: Express, TypeScript, DDD, TypeORM, Prisma, node_modules
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 迁移目标与当前事实

当前 server/app.ts 集中处理 Session、CSRF、输入校验、权限、文章和评论查询。server/entities.ts 是 TypeORM EntitySchema，db.ts 创建 DataSource，manage.ts 管理迁移与管理员授权。这是可运行的应用，但还没有独立的领域层与应用用例。

如果团队要求 Express + TypeScript + DDD，而并不强制原模板的 Prisma/Cognito，可以保留现有 TypeORM 和 Session，借鉴其组织方式。这能让架构调整与业务行为变化分别验收。以下目录是建议目标，不声称当前后端已经完成迁移。


{% raw %}
````text
server/
├── bootstrap.ts                 # 连接、装配、监听、退出清理
├── modules/
│   └── publishing/
│       ├── domain/              # Article、值对象、仓库接口
│       ├── application/         # CreateArticle、UpdateArticle
│       └── infrastructure/
│           ├── http/            # Express 路由、控制器、DTO
│           └── persistence/     # TypeORM 实体、仓库实现、映射
├── identity/                    # 认证与可信身份适配
└── shared/infrastructure/       # 数据源、Session 池、日志、错误映射
````
{% endraw %}


## 逐项把现有代码放到合适位置

| 当前文件或逻辑 | 建议位置 | 应保留的约束 |
| --- | --- | --- |
| req/res、路由和 Zod DTO | infrastructure/http | 参数校验与状态码 |
| 创建、修改、删除的业务步骤 | application | 管理员授权、冲突处理 |
| 标题、slug 等稳定业务约束 | domain | 不导入 Express 或 TypeORM |
| EntitySchema 与 QueryBuilder | infrastructure/persistence | 唯一约束、原子版本比较 |
| Session 与可信 userId 查询 | 身份适配与中间件 | 不接受 req.body 伪造角色 |
| DataSource、pg.Pool、关闭钩子 | bootstrap/shared | 两组池正确复用和关闭 |
| 数据库迁移 | persistence/migrations | 不丢失现有迁移历史 |

## 先切一个纵向功能，不要先搬全部文件

先选择“创建文章”：定义领域 Article 与仓库接口 → 编写 CreateArticle 用例 → 编写 TypeORM 实现 → 在装配入口连接它们 → 让原 POST /api/articles 委托给用例。其他接口先保持原样。

这样每次可以用原前端验证同一请求，不必同时修改所有路由、返回字段和数据库。接下来再处理更新和删除，最后拆评论、查询与认证。提取文件时修改相对 import 并检查迁移注册，尤其 manage.ts 原本引用 src/lib/posts.ts，独立后端包不能直接假定这个跨目录路径仍存在。

## 保持安全边界

控制器从已经验证的服务器会话获取 actor，不能从 req.body 读取 role。中间件继续承担 Cookie、Origin 和 CSRF 验证；应用用例也可检查调用者是否有权执行操作，保证命令行等非 HTTP 入口不会绕过业务授权。

DTO 校验处理外部 JSON 是否符合接口要求；领域构造函数维护业务不变量。两层可能都有检查，但职责不同。并发唯一性最终交给数据库唯一约束，先 find 再 save 不能替代它。

## 可运行的最小示例

本次附上 docs/ddd-example 四个文件。它只实现三字段文章创建，用内存仓库验证边界，没有接入真实 Express、TypeORM、Cookie 或数据库，也不是可直接替换现有文章 API 的完整代码。标题清理、slug 规则与管理员条件贴近现有博客，便于理解依赖方向。


文件：`docs/ddd-example/domain.ts`。

{% raw %}
````typescript
// Teaching model only: this is not a replacement for server/entities.ts.
export class InvalidArticle extends Error {}
export class DuplicateSlug extends Error {}
export type Article = Readonly<{ id: string; title: string; slug: string }>;
export function createArticle(id: string, title: string, slug: string): Article {
  const cleanTitle = title.trim();
  if (!id || !cleanTitle || cleanTitle.length > 160 ||
      slug.length > 120 || !/^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(slug)) {
    throw new InvalidArticle("Invalid article");
  }
  return Object.freeze({ id, title: cleanTitle, slug });
}
export interface ArticleRepository {
  // Must enforce uniqueness atomically, including concurrent requests.
  insert(article: Article): Promise<void>;
}
````
{% endraw %}


domain.ts 定义运行时校验与领域对象，以及仓库接口。接口声明要求 insert 原子保证 slug 唯一，而不是把“先检查是否存在”作为唯一性保障。Readonly 和 freeze 用于这个浅层三字段对象，不代表任意嵌套对象自动深度不可变。


文件：`docs/ddd-example/application.ts`。

{% raw %}
````typescript
import { ArticleRepository, createArticle } from "./domain";
export class Forbidden extends Error {}
// Actor must come from a verified server-side session, never req.body.
export type Actor = { id: string; role: "user" | "admin" } | null;
export class CreateArticle {
  constructor(
    private readonly repository: ArticleRepository,
    private readonly nextId: () => string,
  ) {}
  async execute(actor: Actor, input: { title: string; slug: string }) {
    if (!actor || actor.role !== "admin") throw new Forbidden("Admin required");
    const article = createArticle(this.nextId(), input.title, input.slug);
    await this.repository.insert(article);
    return article;
  }
}
````
{% endraw %}


CreateArticle 只依赖仓库接口和 ID 生成函数，不认识 HTTP、PostgreSQL 或 Awilix。actor 必须由可信边界提供。示例用一个 Forbidden 统一表示拒绝，实际 HTTP 适配应按游客与已登录无权分别映射 401/403，不机械照搬一个状态码。


文件：`docs/ddd-example/memory.ts`。

{% raw %}
````typescript
import { Article, ArticleRepository, DuplicateSlug } from "./domain";
export class MemoryArticleRepository implements ArticleRepository {
  private readonly bySlug = new Map<string, Article>();
  async insert(article: Article): Promise<void> {
    // No await between check and set in this single-process teaching adapter.
    if (this.bySlug.has(article.slug)) throw new DuplicateSlug("Slug exists");
    this.bySlug.set(article.slug, article);
  }
  get count() { return this.bySlug.size; }
}
````
{% endraw %}


内存实现只用于教学和单进程测试。它的检查与 set 之间没有 await，可在此示例中保证一次插入成功；真实多进程数据库必须使用唯一约束，并将 PostgreSQL 的重复键错误映射为 DuplicateSlug。


文件：`docs/ddd-example/example.test.ts`。

{% raw %}
````typescript
import test from "node:test";
import assert from "node:assert/strict";
import { CreateArticle, Forbidden } from "./application";
import { DuplicateSlug, InvalidArticle } from "./domain";
import { MemoryArticleRepository } from "./memory";
const admin = { id: "admin-id", role: "admin" as const };
test("verified admin creates a normalized article", async () => {
  const repo = new MemoryArticleRepository();
  const useCase = new CreateArticle(repo, () => "article-id");
  const result = await useCase.execute(admin, { title: " Hello ", slug: "hello" });
  assert.equal(result.title, "Hello"); assert.equal(result.id, "article-id");
  assert.equal(repo.count, 1); assert.ok(Object.isFrozen(result));
});
test("guest and ordinary user cannot write", async () => {
  const repo = new MemoryArticleRepository();
  const useCase = new CreateArticle(repo, () => "id");
  for (const actor of [null, { id: "u", role: "user" as const }]) {
    await assert.rejects(useCase.execute(actor, { title: "T", slug: "t" }), Forbidden);
  }
  assert.equal(repo.count, 0);
});
test("invalid domain values never reach storage", async () => {
  const repo = new MemoryArticleRepository();
  const useCase = new CreateArticle(repo, () => "id");
  for (const input of [{ title: " ", slug: "ok" }, { title: "T", slug: "Bad Slug" }]) {
    await assert.rejects(useCase.execute(admin, input), InvalidArticle);
  }
  assert.equal(repo.count, 0);
});
test("duplicate slug is rejected; exactly one concurrent attempt succeeds", async () => {
  const repo = new MemoryArticleRepository();
  const useCase = new CreateArticle(repo, () => "id");
  const results = await Promise.allSettled([
    useCase.execute(admin, { title: "A", slug: "same" }),
    useCase.execute(admin, { title: "B", slug: "same" }),
  ]);
  assert.equal(results.filter(r => r.status === "fulfilled").length, 1);
  const failure = results.find(r => r.status === "rejected");
  assert.ok(failure && failure.status === "rejected" && failure.reason instanceof DuplicateSlug);
  assert.equal(repo.count, 1);
});
````
{% endraw %}


在当前 NextBlog 根目录安装既有依赖后执行：


{% raw %}
````powershell
node node_modules/typescript/bin/tsc docs/ddd-example/domain.ts docs/ddd-example/application.ts docs/ddd-example/memory.ts docs/ddd-example/example.test.ts --outDir .tools/ddd-example-build --module commonjs --target es2020 --esModuleInterop --strict --skipLibCheck --types node
node --test .tools/ddd-example-build/example.test.js
````
{% endraw %}


四项测试已实际通过：管理员创建、普通用户与游客被拒绝、无效领域字段不入库、重复 slug 仅一次成功。这证明最小用例与内存适配的行为，不证明 TypeORM 适配或 HTTP 身份校验已经完成。

## TypeORM 适配该怎样写

TypeOrmArticleRepository 实现领域接口，在内部用 db.getRepository(ArticleEntity) 写入，并显式映射领域字段与表字段。不要把 ORM EntitySchema 类型直接作为领域对象接口，以免领域层被数据库生成方式绑住。

当前文章更新必须保留 WHERE id AND version 与 version+1 的原子更新。新增“先 load、改对象、save”如果丢掉版本条件，就会使原本存在的并发保护失效。删除文章的评论级联同样来自数据库外键，迁移文件不能遗漏。

若未来一个用例同时写文章和审计日志，需要一个事务边界，让相关仓库在同一个事务 manager 上运行。不能每个仓库自行创建独立 DataSource，然后认为它们天然处于同一事务。

## 手工装配还是 Awilix

学习时手工 new 最清楚：先建立 DataSource，再建立 TypeORM 仓库，再建立用例，最后建立控制器。确认依赖方向后，可在 Awilix 中注册 repository、useCase、controller。容器不是业务规则的存放地点。

数据库连接可以应用级复用，actor 和事务上下文不要保存在共享单例字段。测试时注入内存仓库或受控测试数据库，不需要为了测试标题规则先启动整套 Web 服务。

## 迁移完成的标准

原 API 契约保持兼容；域模型与用例有独立测试；TypeORM 唯一冲突、版本冲突与回滚有数据库测试；Session、CSRF、普通用户越权、评论归属仍通过原接口测试；typecheck 与 Next 构建继续通过。只有目录变化而权限退化，不能视为成功迁移。

## 本组导航

[文档导读]({% post_url 2026-11-23-express-ts-ddd-00-guide %})

- [express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入]({% post_url 2026-11-23-express-ts-ddd-01-architecture %})
- [express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收]({% post_url 2026-11-23-express-ts-ddd-02-build %})
- [DDD 实战（三）：把当前 TypeORM 博客逐步分层]({% post_url 2026-11-23-express-ts-ddd-03-nextblog %})
- [依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有]({% post_url 2026-11-23-express-ts-ddd-04-node-modules %})
