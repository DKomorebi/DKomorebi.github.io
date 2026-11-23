---
layout: post
title: DDD 应用（十六）：现有代码的分层体现、完整示例与迁移步骤
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: DDD 应用（十六）：现有代码的分层体现、完整示例与迁移步骤：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 先回答“这个项目已经是 ts-express-ddd 吗”

当前运行的NextBlog不是完整DDD工程，也没有安装一个名为ts-express-ddd的运行时框架。此前参考的是gonzaloplaza/express-ts-ddd模板，核对过本地克隆的0de3752版本；它使用的Express/ORM/认证方案与当前项目不同，不能把模板依赖直接覆盖本项目。

DDD是围绕业务含义划定边界、表达规则与依赖方向的设计方法。Express处理HTTP，TypeScript描述契约，TypeORM访问数据库，它们都可以与DDD配合，但“用了这三个工具”不自动等于DDD。

## 2. 当前已经能看见的分离

| 当前位置 | 职责 | 与分层思想的关系 | 仍欠缺什么 |
| --- | --- | --- | --- |
| server/app.ts中的注册路由 | 解析JSON、Zod校验、返回HTTP | 接口适配层的部分工作 | 多数文章评论数据库逻辑仍在同文件 |
| createRegistration | 协调发码、校验、创建用户 | 应用服务的雏形 | 直接依赖DataSource和SQL，非持久化无关 |
| RegistrationMailer类型 | 声明发送能力 | 端口/依赖倒置思想 | 其他数据库依赖未抽成端口 |
| createRegistrationMailer | SMTP实现 | 基础设施适配器 | 还没有独立模块的组合根目录 |
| server/index.ts | 创建真实依赖并交给app | 组合根、手工依赖注入 | 不需要为了分层必须引入容器库 |
| EntitySchema | 数据库映射 | 基础设施模型 | 不等同于承载业务规则的领域实体 |

最具体的体现是：测试把mailer换成内存Map函数，把now换成可推进的时钟，而业务sendCode/register不用改。这说明部分依赖已经可以替换；但db仍在服务内部，所以不能声称业务已经完全不依赖TypeORM。

## 3. 先选一个有业务规则的切口

以“管理员发布文章”为例，规则包括：只有管理员可以创建；标题不能为空且最长160；slug格式合法且唯一；创建返回有稳定id的文章。HTTP请求格式、PostgreSQL连接、状态码不是文章本身的业务含义，应分别放在边缘层。

建议的迁移目录（**方案，尚未替换server运行入口**）：

{% raw %}
```text
server/modules/articles/
  domain/Article.ts
  domain/ArticleRepository.ts
  application/CreateArticle.ts
  infrastructure/TypeOrmArticleRepository.ts
  interfaces/http/articleRouter.ts
server/bootstrap.ts
```
{% endraw %}

依赖方向：interfaces调用application；application依赖domain的规则与端口；infrastructure实现domain端口；bootstrap认识所有具体实现并组装。domain不import express、typeorm或nodemailer。不要只是把app.ts按文件夹拆开却让domain反过来引用控制器。

## 4. 已存在的四文件教学例子，每个方法都做什么

`docs/ddd-example/domain.ts`是独立缩小模型，不替代真实ArticleEntity。InvalidArticle、DuplicateSlug是业务错误类别；Article是Readonly对象类型；createArticle(id,title,slug)先trim标题，再检查id/标题长度/slug长度和正则，失败抛InvalidArticle，成功Object.freeze返回对象。freeze是运行时浅冻结，Readonly是类型层限制，两个层面不同。

ArticleRepository接口只声明insert(article):Promise<void>，并要求实现方在并发场景下原子保证slug唯一。不要在应用层先exists再insert当成唯一性保障。

`application.ts`的Actor来自已验证服务端身份。CreateArticle.constructor接收repository和nextId函数，不自己new数据库连接。execute(actor,input)先确认admin，调用createArticle表达实体规则，await repository.insert，最后返回article。应用服务负责步骤编排，领域函数负责文章自身合法性。

`memory.ts`的MemoryArticleRepository用Map按slug存储。insert先has，重复抛DuplicateSlug，否则set；这段检查与插入之间没有await，在单个JS进程里不会在其中让出执行，但不能把这点推广到多个进程或数据库。count getter返回Map大小，供测试检查是否写入。

`example.test.ts`验证正常创建、普通用户拒绝、非法文章拒绝、重复slug拒绝。它没有连接PostgreSQL，因此是教学行为测试，不证明真实SQL事务和Session已完成迁移。

## 5. 如何把真实 TypeORM 接到端口

下面是**适配器写法示意**，使用简化模型的三个字段，不是可直接覆盖生产实体的完整代码。真实项目还需要excerpt/content/category及其校验。

{% raw %}
```ts
class TypeOrmArticleRepository implements ArticleRepository {
  constructor(private readonly repository: Repository<ArticleRow>) {}
  async insert(article: Article): Promise<void> {
    try {
      await this.repository.insert(article);
    } catch (error) {
      if ((error as { code?: string }).code === "23505") {
        throw new DuplicateSlug("Slug exists");
      }
      throw error;
    }
  }
}
```
{% endraw %}

constructor把具体Repository保存为依赖；insert只负责把领域值映射到数据库；捕获已知唯一冲突并翻译成业务错误；未知异常继续抛，不能把断网也伪装成“slug重复”。生产更精确时还应按constraint名称区分不同唯一约束。

控制器应完成DTO校验、从req.account构造actor、调用useCase.execute、把结果映射201；Forbidden映射403、InvalidArticle映射400、DuplicateSlug映射409。它不负责生成SQL。组合根用`new CreateArticle(new TypeOrmArticleRepository(...), randomUUID)`注入实现，测试则注入MemoryArticleRepository和固定id函数。

## 6. 注册为什么不能简单拆成三个独立 Repository.save

注册事务要求“检查并锁验证码→计错误次数或创建用户→消费验证码”在同一事务语义内。若VerificationRepository和UserRepository各自偷偷用不同连接，会失去原子性。DDD改造时必须设计同一Unit of Work/事务边界，或者提供原子业务仓储操作；不能为了类图漂亮而拆坏安全规则。

邮件发送与数据库不可能靠一个普通PostgreSQL事务保证共同提交。当前采用预留+发送+激活并说明失败边界；规模更大可评估outbox和任务队列，但那是额外架构，不是本文已实现。

## 7. 渐进迁移的具体顺序

先保留API路径与响应契约，建立当前集成测试作为保护；抽出Article领域规则与端口；实现TypeORM适配器；将一条创建路由改为调用use case；运行管理员/普通用户/重复slug测试；再迁移更新、删除、评论；最后处理注册事务与身份上下文。

每完成一步，都检查“业务规则是否更容易独立测试”“事务是否仍完整”“前端是否无需知道后端重构”。不要同时换Prisma、认证服务和数据库，因为那会使初学者无法判断失败来自分层还是依赖更换。

## 8. DOM事件与领域事件不是一个东西

onClick、onSubmit是浏览器交互事件。DDD的ArticlePublished是“文章已经发布”的业务事实，可供通知或索引任务使用。当前运行项目没有事件总线或领域事件发布器，所以在full-blog中找不到DDD事件是正常的；不能因此推断Next不推荐使用页面事件。当前注册也没有通过领域事件发送邮件，而是应用服务显式调用mailer。

## 对照源码：docs/ddd-example/domain.ts（第 1–16 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: docs/ddd-example/domain.ts start: 1 end: 16 sha256: 34d056cebfa1bb21ea36947df0f659e423b70c746f556b72cdc38e34f4bef80f -->

{% raw %}
````ts
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

## 对照源码：docs/ddd-example/application.ts（第 1–16 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: docs/ddd-example/application.ts start: 1 end: 16 sha256: 0ac3a8d6529bb541c6775b95880c638efccffd80a3ebea2d5a7e072423689beb -->

{% raw %}
````ts
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

## 对照源码：docs/ddd-example/memory.ts（第 1–10 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: docs/ddd-example/memory.ts start: 1 end: 10 sha256: 78d7ccc18486bb26cd5646bbf4236b4211ec0203335f9526890a19a9ac04c795 -->

{% raw %}
````ts
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

## 对照源码：docs/ddd-example/example.test.ts（第 1–41 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: docs/ddd-example/example.test.ts start: 1 end: 41 sha256: 343f27297ddb2f097751f0bca1cae4a5668bec7f69a4a65e1de1f4217c10c585 -->

{% raw %}
````ts
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

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-15-editor %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-17-theme %})
