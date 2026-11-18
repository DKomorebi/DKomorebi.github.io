---
layout: post
title: Next.js博客（六）：Express、PostgreSQL 与 TypeORM 接入路线
date: 2026-11-17 09:05:00 +0800
categories:
  - 实习项目
category_path: 开发/实习项目/博客系统
description: 规划静态 Demo 到完整博客的演进，说明请求链路、目录分层、接口契约、数据库迁移和部署选择。
keywords: Express, PostgreSQL, TypeORM, API, 数据库迁移, 全栈
cover:
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

前五篇完成了前端基础。这一篇讨论下一阶段如何接真实数据，示意代码不属于当前已运行的静态 Demo，也不是完整的后端服务。

> 本文是「实习博客实战」系列第 6 / 8 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})

本章是下一阶段的设计与示意，不是当前已运行的后端。当前项目只安装了前端和构建依赖，不需要数据库账号。
8. [实习博客实战（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

## 请求链路

{% raw %}
```text
浏览器里的 Next.js 界面
  → HTTP GET /api/posts
  → Express 路由（参数校验、错误处理）
  → 业务服务 / TypeORM Repository
  → PostgreSQL 表
  → JSON 响应
  → 前端更新文章卡片
```
{% endraw %}

数据库连接字符串只应出现在服务端环境变量中。带有 NEXT_PUBLIC_ 前缀的变量可能进入前端包，不能存放数据库密码。Next.js 自身也有后端能力，但公司指定 Express 时，可以使用独立 Express API，不需要把两套路由混为一谈。

## 建议拆分职责

后续可以新增独立 `server/` 工程，使用自己的 package.json 和 tsconfig，避免把数据库模块编译到浏览器。

{% raw %}
```text
server/src/
  app.ts                  Express 中间件和路由
  data-source.ts          TypeORM DataSource
  entities/PostEntity.ts  数据库映射
  routes/posts.ts         HTTP 参数、状态码、响应
  services/posts.ts       文章查询与业务规则
  migrations/             可审查的数据库结构变更
```
{% endraw %}

不要一开始加入登录、文件上传、评论、富文本编辑等所有功能。先完成“数据库中存在一篇文章，前端能读出来”这个闭环。

## 数据模型与接口约定

建议文章表字段：id（UUID 主键）、slug（唯一且非空）、title、excerpt、category、published_at、reading_minutes、tags、sections。当前结构化的 sections 可以先考虑 JSONB，后面再根据内容编辑需求决定是否改为 Markdown 或拆表。

API 响应保持前端 Post 结构稳定，例如：

{% raw %}
```json
{
  "data": [{ "slug": "nextjs-first-page", "title": "示例标题" }],
  "total": 1
}
```
{% endraw %}

上面仅展示响应外壳，真正每个 data 项还需要所有 Post 字段。数据库实体不一定等于前端 DTO：例如数据库使用 reading_minutes，API 可以映射成 minutes；日期也需要约定格式。

| 接口 | 成功响应 | 需要处理的失败 |
| --- | --- | --- |
| GET /api/posts | 200，列表、total | 400 参数错误；500 查询失败 |
| GET /api/posts/:slug | 200，单篇文章 | 404 不存在；500 查询失败 |
| POST /api/posts（更后面） | 201，新文章 | 401 未登录；403 无权发布；400 校验失败；409 slug 冲突 |

列表参数可以使用 `q`、`category`、`page`、`pageSize`，需要校验类型、范围和枚举。排序字段应使用白名单；查询使用 ORM 参数化能力，不拼接用户输入到 SQL 中。

## Express 的只读路由示意

以下代码只解释各层的配合，省略初始化与 schema 定义，**不能直接作为完整服务运行**：

{% raw %}
```ts
app.get("/api/posts/:slug", async (req, res, next) => {
  try {
    const post = await postRepository.findOneBy({ slug: req.params.slug });
    if (!post) {
      res.status(404).json({ error: "POST_NOT_FOUND" });
      return;
    }
    res.json({ data: toPostDto(post) });
  } catch (error) {
    next(error);
  }
});
```
{% endraw %}

req 提供 URL 参数和请求内容；res 控制状态码与响应；next 把错误交给统一错误中间件。toPostDto 是需要自己实现的转换函数。错误日志可以留在服务端，但不要把堆栈和数据库连接信息直接返回给用户。[Express 官方路由说明](https://expressjs.com/en/5x/guide/routing/)

开发时前端和 Express 的端口不同，属于不同 origin，需要配置允许的 CORS 来源；生产也可以通过反向代理把 /api 指向 Express。CORS 不是身份认证，不应把它当作阻止未授权写入的机制。

## TypeORM 与迁移

TypeORM 的 DataSource 负责数据库连接与实体配置。实体描述表结构，Repository 用于查询与保存。装饰器写法需要后端 tsconfig 的相应配置，不要直接沿用 Next.js 的前端配置。

结构变更建议采用 migration：生成或编写迁移，检查 SQL，在测试库执行，确认后提交。生产配置保持 `synchronize: false`，避免启动时自动修改既有表。迁移需要版本记录与回滚策略。[迁移配置参考](https://typeorm.io/docs/migrations/setup/)

## 前端应该怎么换数据源

首先把数据访问从页面中抽离，例如设计 `listPosts()` 与 `getPostBySlug()`。组件继续接收 Post[] 或 Post，不关心数据库是什么。

不要只添加一行 fetch 就宣布接入完成，还需要：检查 response.ok、校验 JSON、处理加载和错误、处理空结果、避免过期请求覆盖新结果、验证直接访问详情地址。

有两种部署路线，需要和导师明确选一种：

| 路线 | 文章变更时 | 取舍 |
| --- | --- | --- |
| 保留静态导出，构建时读取 API | 重新构建和发布 | 部署简单，但内容不是即时更新 |
| 使用 Next.js Node.js 服务，移除 output: export | 请求时或按缓存策略读取 API | 支持运行时新文章地址，需要部署服务 |

静态页面也可以在浏览器请求 API，但当前 /posts/[slug] 的路径是在构建时枚举的。只把列表改成客户端 fetch，并不能自动让新 slug 获得详情页，这是迁移时特别容易忽略的边界。

## 完成标准

重启服务后文章仍在；直接访问详情地址正常；不存在的文章得到明确 404；接口异常能在页面上显示可恢复错误；网络返回值经过校验；没有数据库密码进入客户端包；测试库迁移成功。达到这些标准之后，再开始登录和发布流程。

## 继续阅读

[← 上一篇：实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})

[下一篇：实习博客实战（七）：编译、验收、问题排查与实习练习 →]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
