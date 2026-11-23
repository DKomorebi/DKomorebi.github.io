---
layout: post
title: NextBlog（十一）：全栈路由与每个页面完整源码
date: 2026-11-20 00:11:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 首页、归档、登录注册、文章详情、后台编辑与公共布局。
keywords: Next.js, TypeScript, Express, PostgreSQL, DaisyUI, 全栈博客
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


首页、归档、登录注册、文章详情、后台编辑与公共布局。

> 本文是 NextBlog 全栈博客系列第 11 / 15 篇，内容对应当前全栈版本。

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

## 路由文件保持简单

App Router 通过目录组织 URL。首页和归档复用 ArticleList；登录和注册复用 AuthForm；管理列表仍用 ArticleList，但启用 manage。两个编辑入口共享 Editor，新建不带 id，修改时携带 id 与 slug。职责明确后，页面文件可以非常短。

动态详情页通过 await params 获取 slug，交给 ArticleReader 查询真实 API；不再枚举 generateStaticParams，也不再输出静态 out 目录。未知文章由 API 返回 404 并在阅读器中显示错误，当前前端路由本身不实现文章级服务端 404 或独立 SEO 元数据。

## 公共布局

layout.tsx 放置导航、SessionProvider、正文和页脚。首帧主题脚本在页面绘制前读取偏好，html 的 suppressHydrationWarning 仅用于处理这个预期的 data-theme 差异。脚本内容是项目常量，不能将用户输入拼入其中。

next.config.ts 使用 rewrite 将 /api 转发到 Express。浏览器仍然访问同一个 origin，Cookie 不需要跨域开放。运行端口变化时要同步调整 APP_ORIGIN，否则写操作会被来源校验拒绝。

## 阅读顺序

先对照下面的文件路径访问页面，再阅读第十二篇中的交互组件。页面是否能显示与用户是否有权执行操作是两回事：后台页面有前端提示，后端仍逐请求检查权限。


## 本篇文件索引

- [src/app/about/page.tsx](#source-1)
- [src/app/admin/[id]/page.tsx](#source-2)
- [src/app/admin/new/page.tsx](#source-3)
- [src/app/admin/page.tsx](#source-4)
- [src/app/archive/page.tsx](#source-5)
- [src/app/layout.tsx](#source-6)
- [src/app/login/page.tsx](#source-7)
- [src/app/not-found.tsx](#source-8)
- [src/app/page.tsx](#source-9)
- [src/app/posts/[slug]/page.tsx](#source-10)
- [src/app/register/page.tsx](#source-11)
- [next.config.ts](#source-12)
- [next-env.d.ts](#source-13)

<a id="source-1"></a>

### src/app/about/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
export const metadata = { title: "关于拾页" };
export default function About() {
  return (
    <div className="reading">
      <section className="intro">
        <h1>让想法，慢慢生长。</h1>
        <p>拾页，是一个安放文字的小地方。</p>
      </section>
      <div className="markdown">
        <p>
          在这里记录技术探索，也记录日常里微小而真实的发现。写作是整理思绪的过程，阅读是与另一种经验相遇。
        </p>
        <h2>关于这个小站</h2>
        <p>
          用 Markdown
          写作，分享值得记录的想法。欢迎注册账号，在文章下留下你的思考。
        </p>
        <h2>留言约定</h2>
        <p>
          尊重不同的观点，讨论具体的问题。请勿发布他人的隐私或无关广告。你可以删除自己的留言，管理员可以维护讨论秩序。
        </p>
      </div>
    </div>
  );
}
```
{% endraw %}

<a id="source-2"></a>

### src/app/admin/[id]/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ id: string }>;
}) {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor id={(await params).id} />
    </Suspense>
  );
}
```
{% endraw %}

<a id="source-3"></a>

### src/app/admin/new/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default function Page() {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor />
    </Suspense>
  );
}
```
{% endraw %}

<a id="source-4"></a>

### src/app/admin/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export default function Page() {
  return <ArticleList manage />;
}
```
{% endraw %}

<a id="source-5"></a>

### src/app/archive/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export const metadata = { title: "文章归档" };
export default function Archive() {
  return <ArticleList archive />;
}
```
{% endraw %}

<a id="source-6"></a>

### src/app/layout.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import type { Metadata } from "next";
import { Header, SessionProvider } from "@/components/full-blog";
import "./globals.css";
import { themeInitScript } from "@/lib/theme";
export const metadata: Metadata = {
  title: { default: "拾页 · 落笔成文，拾页成光", template: "%s · 拾页" },
  description: "关于技术、生活与每一个值得记录的想法。",
};
export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="zh-CN" data-theme="fieldnotes" suppressHydrationWarning>
      <head>
        <script dangerouslySetInnerHTML={{ __html: themeInitScript }} />
      </head>
      <body>
        <SessionProvider>
          <Header />
          <main className="site-main">{children}</main>
          <footer className="site-footer">
            <span>拾页</span>
          </footer>
        </SessionProvider>
      </body>
    </html>
  );
}
```
{% endraw %}

<a id="source-7"></a>

### src/app/login/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "登录" };
export default function Page() {
  return <AuthForm />;
}
```
{% endraw %}

<a id="source-8"></a>

### src/app/not-found.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import Link from "next/link";

export default function NotFound() {
  return (
    <main className="px-6 py-28 text-center">
      <p className="font-mono text-sm text-accent">404 / PAGE NOT FOUND</p>
      <h1 className="mt-5 text-3xl font-bold">这一页，还没有写下。</h1>
      <p className="mt-4 text-sm text-muted">
        地址可能有误，回到首页继续翻阅吧。
      </p>
      <Link
        href="/"
        className="mt-8 inline-block rounded-full bg-ink px-6 py-3 text-sm text-white"
      >
        返回首页
      </Link>
    </main>
  );
}
```
{% endraw %}

<a id="source-9"></a>

### src/app/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
```
{% endraw %}

<a id="source-10"></a>

### src/app/posts/[slug]/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { ArticleReader } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ slug: string }>;
}) {
  return <ArticleReader slug={(await params).slug} />;
}
```
{% endraw %}

<a id="source-11"></a>

### src/app/register/page.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "注册" };
export default function Page() {
  return <AuthForm register />;
}
```
{% endraw %}

<a id="source-12"></a>

### next.config.ts

以下为该文件的完整源码。

{% raw %}
```typescript
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
```
{% endraw %}

<a id="source-13"></a>

### next-env.d.ts

以下为该文件的完整源码。

{% raw %}
```typescript
/// <reference types="next" />
/// <reference types="next/image-types/global" />
import "./.next-fullstack/types/routes.d.ts";
import "./.next-fullstack/types/root-params.d.ts";

// NOTE: This file should not be edited
// see https://nextjs.org/docs/app/api-reference/config/typescript for more information.
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-10-auth-api %})

[下一篇]({% post_url 2026-11-20-intern-blog-12-components %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
