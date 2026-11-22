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
title: React 与 Next.js（三）：从文章地址追踪一次完整请求
date: 2026-11-22 00:03:00 +0800
description: A · React 与 Next.js：结合项目源码、逐段讲解和练习答案深入学习。
---

> A · React 与 Next.js · 第 3 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 选定一个具体案例

以 /posts/intern-first-week 为例。这个地址表示打开一篇文章，不是数据库连接地址，也不是 Express 的接口地址。阅读时准备四个文件：posts/[slug]/page.tsx、full-blog.tsx、next.config.ts、server/app.ts。

## 第一站：Next.js 匹配页面

目录 [slug] 是动态路径段，接收 intern-first-week。页面 await params 后渲染 ArticleReader。这里 slug 是字符串，UUID id 还没有从数据库取得。slug 适合阅读链接，id 用于修改、删除和关联评论，两者不能交换使用。

## 第二站：React 发出 JSON 请求

ArticleReader 调用 api<Article>。api 封装将路径前加 /api，使用 fetch 获取 JSON，响应成功则返回数据，失败则抛出错误。泛型 Article 让编辑器检查使用方式，并没有在运行时验证响应 JSON。

浏览器实际访问 localhost:3000/api/articles/intern-first-week。credentials: same-origin 允许带上该源的 Cookie，但读取文章不要求登录；同一个接口可以服务游客和登录用户。

## 第三站：代理到 Express


代码位置：`next.config.ts`（完整文件）。

{% raw %}
````typescript
import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  distDir: ".next-fullstack",
  async rewrites() {
    return [
      {
        source: "/api/:path*",
        destination: `${process.env.API_ORIGIN || "http://127.0.0.1:4000"}/api/:path*`,
      },
    ];
  },
};

export default nextConfig;
````
{% endraw %}


source 匹配外部请求，:path* 收集后续路径。destination 读取 API_ORIGIN，默认 127.0.0.1:4000。rewrite 保留浏览器看到的源，不会要求浏览器跳转到 4000。浏览器不需要知道数据库在哪里。

APP_ORIGIN 和 API_ORIGIN 不能互换。前者是写操作允许的浏览器来源，后者是 Next.js 到 Express 的转发目标。网页是 localhost，后台目标是 127.0.0.1 是允许的；关键是浏览器来源校验配置必须与网页实际地址一致。

## 第四站：路由和数据库查询


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


req.params.slug 由 Express 路由匹配得到。getRepository 取得文章实体的数据访问对象，findOneBy 根据 slug 查记录。不存在时发送 404 并 return，避免再继续发送成功响应。存在时 res.json 序列化对象，Date 等数据会转成 JSON 可表达的形式。

## 第五站：返回 React 渲染

浏览器收到对象，setArticle 触发新一轮渲染。Markdown 组件解析 content，Comments 使用 article.id 请求评论。因此同一个页面会出现多个 API 请求：会话、正文、评论。这不是“同一请求重复执行”的充分证据，需要先看路径和用途。

| 地址或值 | 属于哪一层 | 用途 |
| --- | --- | --- |
| /posts/intern-first-week | 页面路由 | 打开阅读界面 |
| /api/articles/intern-first-week | HTTP API | 获取文章 JSON |
| 127.0.0.1:4000 | 业务服务监听地址 | 接收代理请求 |
| localhost:5432/fieldnotes | 数据库连接目标的一部分 | PostgreSQL 协议连接 |
| articles 表的 slug 列 | 数据层 | 根据标识找到记录 |

## 排错时怎样切断问题范围

页面完全打不开，先查 Next.js；页面有外框但 API 失败，查 Express 和代理；health 也失败，继续查连接；接口 200 但内容显示错误，查前端字段和 Markdown。不要把所有错误都归结为 React。

练习：记录文档与正文 API 的 URL、状态码和返回类型。自测：把数据库端口改成 3000 能解决页面访问吗？不能，它们使用的协议和进程不同。正文 API 返回 404 时应先检查 slug 对应记录，而不是添加一个同名 .tsx 文件。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-02-rendering %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-04-ts-files %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [React：描述用户界面](https://react.dev/learn/describing-the-ui)
- [Next.js 服务端与客户端组件](https://nextjs.org/docs/app/getting-started/server-and-client-components)
