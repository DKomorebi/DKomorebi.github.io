---
layout: post
title: express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/DDD架构
description: express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入：结合真实模板与现有 NextBlog 源码逐步解释。
keywords: Express, TypeScript, DDD, TypeORM, Prisma, node_modules
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 本文说的 express-ts-ddd 是什么

这里以 gonzaloplaza/express-ts-ddd 仓库作为同名模板的具体参考。如果公司要求的是另一个仓库，需要重新核对其约定。Express + TypeScript + DDD 也可以泛指一种组合方式，不能仅凭名称断言它是某个必须 npm install 的框架包。

本文核对的模板提交为 0de3752，使用 Express 4、Prisma、Awilix 和 Cognito。当前 NextBlog 则是 Express 5、TypeORM、PostgreSQL Session。它们都可以按 DDD 思路组织，但认证、ORM、启动方式不能直接互换。本文先解释模板，第三篇再讨论保留现有博客技术栈的方案。

## DDD 首先是业务建模，不是创建三个文件夹

Domain-Driven Design 的核心是围绕业务概念和规则组织模型，与业务人员保持一致的术语。比如 Article 是文章，slug 是公开地址标识，只有管理员可发布，版本冲突不能默默覆盖。这些术语与规则应能对应到代码，而不是全部淹没在 req.body 和 SQL 中。

把 Controller、Service、Repository 分开是结构手段，不自动证明已经建立了有意义的领域模型。简单 CRUD 用清晰分层已经可以受益，无需为了“看起来 DDD”给每一个字符串包装十个类。

## 先分清几个概念

| 概念 | 博客例子 | 不要混淆 |
| --- | --- | --- |
| 实体 | 一篇有稳定 id 的文章 | 不是必须等同 ORM 表对象 |
| 值对象 | 经校验的 slug | 重点是值与约束，不是独立身份 |
| 聚合 | 需要一起维护不变量的一组对象 | 不等于数据库里所有关联表 |
| 仓库接口 | 保存、读取文章的抽象能力 | 不等于 TypeORM Repository 本身 |
| 应用用例 | 管理员创建文章 | 编排步骤，不直接接收 Express req |
| 限界上下文 | 内容发布、身份认证等各自的术语边界 | 不要求拆成微服务 |

文章与评论有外键，不代表一定要把全部评论加载成一个巨大 Article 聚合。应根据一致性要求决定边界。当前项目没有完整的草稿工作流，不能在文档中把它当作已有领域规则。

## 原模板的真实调用链

bin/index.ts 创建容器，解析 server 和 config，然后启动 HTTP 服务。Container.ts 注册控制器、应用服务、仓库、PrismaClient、日志与认证实现。ApiRouter 把路径绑定到控制器的 invoke 方法。

以创建 Activity 为例：POST /api/v1/activities → Cognito 授权 → 请求验证 → CreateActivityController → CreateActivityService → IActivityRepository → PrismaActivityRepository → PrismaClient → PostgreSQL。

- [`bin/index.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/bin/index.ts)
- [`src/shared/infrastructure/Container.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/shared/infrastructure/Container.ts)
- [`src/api/infrastructure/express/router/index.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/api/infrastructure/express/router/index.ts)
- [`src/api/infrastructure/express/controllers/activity/CreateActivityController.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/api/infrastructure/express/controllers/activity/CreateActivityController.ts)
- [`src/api/application/activity/create/CreateActivityService.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/api/application/activity/create/CreateActivityService.ts)
- [`src/api/domain/model/activity/IActivityRepository.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/api/domain/model/activity/IActivityRepository.ts)
- [`src/api/infrastructure/persistence/prisma/PrismaActivityRepository.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/api/infrastructure/persistence/prisma/PrismaActivityRepository.ts)
- [`src/shared/infrastructure/prisma/index.ts`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/src/shared/infrastructure/prisma/index.ts)
- [`prisma/schema.prisma`](https://github.com/gonzaloplaza/express-ts-ddd/blob/0de3752d9c5bce62691bcd5fbd721cd02df9603b/prisma/schema.prisma)

逐个文件读时，先问它知道哪些外部概念。控制器知道 req/res 和 HTTP 校验；应用服务知道创建 Activity 的输入与仓库能力；仓库实现知道 Prisma。不要一开始从最外层 index 一口气追完所有 Cognito 文件。

## 三层职责与依赖方向

domain 表达业务模型、规则和必要接口。application 编排一个操作需要哪些步骤，处理业务授权及跨对象协调。infrastructure 连接 Express、ORM、外部认证、日志等具体技术。最外面的装配入口负责 new 或注册具体实现。

运行时会从控制器走到数据库，但源码依赖可以让应用层依赖仓库接口，再由基础设施实现接口。这就是依赖倒置：用例不必直接 import PrismaActivityRepository 才能保存数据。

严格边界的领域层应避免 import Express 的 Request、Response 或 ORM 生成类型。实际模板的 IActivityRepository 引用了 @prisma/client 的 Activity，说明它仍有 ORM 类型向内泄漏。它是可学习的模板，不是无需思考的完美架构。若保留 TypeORM，应自行定义领域对象及映射。

## IoC 与 DI 到底做了什么

依赖注入就是把用例需要的仓库从外部传入，而不是在用例内部固定创建数据库仓库。手动 new CreateArticle(repository, nextId) 也属于 DI，不需要容器才能做到。

Awilix 负责对象装配。该模板使用 CLASSIC 注入模式，构造函数参数名需要与注册名匹配，例如 activityRepository。TypeScript interface 在运行时不会替你注册对象，拼错名字仍可能在解析时失败。

模板把很多服务注册成 singleton。它们可以保存无请求差异的依赖，却不应该把当前用户或当前请求事务存在实例字段里，否则多个并发请求可能串用身份。用户信息应作为调用参数，事务依赖则按请求或用例范围处理。

## 如何把框架学成自己的能力

先手工串起一个只读用例，再换一个内存仓库验证不依赖数据库，最后理解容器只是把手工装配集中化。不要一开始同时引入 ORM、更换认证、修改目录和升级所有依赖，这会让每个失败都失去定位线索。

自测：把 PrismaActivityRepository 换成内存实现，控制器是否必须跟着重写？良好边界下不用。把 domain 文件夹搬到别处但仍 import req.body，是否已经实现 DDD？没有，边界由职责与依赖决定，不由目录名决定。

## 本组导航

[文档导读]({% post_url 2026-11-23-express-ts-ddd-00-guide %})

- [express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入]({% post_url 2026-11-23-express-ts-ddd-01-architecture %})
- [express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收]({% post_url 2026-11-23-express-ts-ddd-02-build %})
- [DDD 实战（三）：把当前 TypeORM 博客逐步分层]({% post_url 2026-11-23-express-ts-ddd-03-nextblog %})
- [依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有]({% post_url 2026-11-23-express-ts-ddd-04-node-modules %})

## 核对来源

[原模板固定提交](https://github.com/gonzaloplaza/express-ts-ddd/tree/0de3752d9c5bce62691bcd5fbd721cd02df9603b)；具体文件链接与名称见正文。文档中的修正建议与示例代码是本次教学补充，不是原仓库已实现的功能。
