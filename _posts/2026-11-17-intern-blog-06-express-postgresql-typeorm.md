---
layout: post
title: NextBlog（六）：Express、TypeORM 与真实数据库请求
date: 2026-11-17 09:05:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 沿一次文章发布理解中间件、Repository、迁移、校验与错误处理。
keywords: NextBlog, Next.js, TypeScript, Tailwind CSS, DaisyUI, Express, PostgreSQL
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
last_modified_at: 2026-11-20 07:31:36 +0800
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


沿一次文章发布理解中间件、Repository、迁移、校验与错误处理。

> 本文是 NextBlog 全栈博客系列第 6 / 15 篇，内容对应当前全栈版本。

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

## 一次请求经过哪些文件

浏览器中的 api() 请求 /api/articles，next.config.ts 转发到 Express。server/index.ts 初始化数据库并启动监听；server/app.ts 创建应用、安装中间件并注册路由；路由通过 TypeORM 使用 PostgreSQL。服务端不是在 Next.js 页面中直接运行浏览器组件的数据库代码。

## DataSource 表示一组数据库配置与连接

代码位置：`server/db.ts`（完整文件）。

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


dotenv/config 在后端加载 .env。DataSource 包含连接地址、实体和迁移列表，initialize 后才可以执行数据库操作。synchronize: false 表示不会按实体定义自动改表；创建表必须明确执行 pnpm db:migrate。

server/entities.ts 用 EntitySchema 描述 users、articles、comments 的字段映射。sessions 由 connect-pg-simple 使用，建表 SQL 也在迁移中。数据库还会维护迁移执行记录，所以业务表数量不等于数据库中所有表的数量。

## Entity 与迁移要一起理解

实体告诉 TypeORM 应如何读取和写入对象，迁移说明数据库结构怎样从一个版本变化到下一个版本。给文章增加字段时，要同时考虑迁移、实体、输入校验、返回数据和前端表单。已经应用过的迁移应通过新增迁移演进，不应依赖反复修改旧迁移重建用户数据。

users.email 与 articles.slug 有唯一约束；评论有外键，删除文章时关联评论会被级联删除。seed 是可选的数据初始化命令，读取 src/lib/posts.ts，将示例段落转换成 Markdown 后保存。运行时首页始终读 API，不直接读取这个示例数组。

## 文章创建路由只有几行，但依赖前置边界

代码位置：`server/app.ts`（节选）。

{% raw %}
```typescript
  app.post("/api/articles", requireAdmin, async (req, res) => {
    const input = articleInput.parse(req.body);
    res.status(201).json(await db.getRepository(ArticleEntity).save(input));
  });
```
{% endraw %}


POST 表示创建。requireAdmin 先验证身份和角色，articleInput.parse 检查实际请求数据，Repository.save 保存对象，201 响应表示创建成功。异步操作必须 await，否则响应可能在持久化完成前返回。

代码位置：`server/app.ts`（节选）。

{% raw %}
```typescript
  const requireAdmin = (req: Request, res: Response, next: NextFunction) => {
    if (!req.account) {
      res.status(401).json({ message: "请先登录" });
      return;
    }
    if (req.account.role !== "admin") {
      res.status(403).json({ message: "仅管理员可管理文章" });
      return;
    }
    next();
  };
```
{% endraw %}


req.account 不是浏览器传入的用户对象。前面的会话中间件从 Session.userId 查询数据库，取得当前账号。未登录返回 401，普通用户返回 403。将按钮隐藏在管理页之外是界面优化，服务端的判断才决定写入是否被允许。

## 中间件顺序决定可用信息

应用先设置基础安全响应头和 JSON 大小限制，再加载 PostgreSQL Session；/api 写请求检查 Origin 和 CSRF token，然后读取登录账号，最后进入路由。错误处理中间件位于路由之后，将 Zod 错误映射为 400，将唯一约束冲突映射为 409，其余内部错误返回通用 500 提示。

这段顺序也解释了为什么注册前仍要读取 /api/auth/session：它生成游客会话的 CSRF token，登录和注册同样需要来源和 token 校验。不能只在文章保存时才考虑 CSRF。

## 查询与更新的约束

搜索在 title、excerpt、content 上执行 ILIKE，参数绑定防止把用户输入拼成 SQL 代码，通配符另外进行转义。列表只选择卡片需要的字段，每页 10 条；详情返回正文。大规模数据可再考虑专门的搜索索引，当前实现没有声称提供全文检索排名。

修改文章提交 version，SQL 同时比较 id 与 version，成功时递增版本。两个编辑页同时打开，先保存的一方成功，另一方持旧版本保存会得到 409。删除文章返回 204，没有 JSON 正文。

## 练习

在管理员页面新建文章，观察 POST 请求、201 响应和返回的 id、slug、version。然后在数据库只读查询中确认保存结果。再用普通账号访问管理页，理解前端提示与后端 403 是两层独立检查。第九、十篇提供数据库和 API 的完整源码。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-05-react-interactions %})

[下一篇]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
