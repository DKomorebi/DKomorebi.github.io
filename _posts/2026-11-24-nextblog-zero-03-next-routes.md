---
layout: post
title: 页面入口（三）：Next.js 路由、布局与每个 page 方法
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 页面入口（三）：Next.js 路由、布局与每个 page 方法：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. Next 与 React 的关系落在什么文件

React 提供组件模型，Next 用约定把组件组织成网站。`src/app/page.tsx` 对应 `/`；目录里的 `page.tsx` 是可访问页面；`layout.tsx` 提供共享布局。普通 `src/components/full-blog.tsx` 不会因为文件存在就自动成为网址。

App Router 的页面入口默认是服务端组件。当前页面把复杂交互交给声明 `"use client"` 的组件。客户端组件仍可能参与首次服务端 HTML 生成，“use client”不等于“整个文件永远只在浏览器渲染”。其事件处理、Effect、浏览器 API 在客户端工作。

## 2. RootLayout({ children }) 逐步读

文件 `src/app/layout.tsx`：导入全局 CSS；导出 metadata 作为全站标题/描述默认值；返回 html/head/body；把 children 放进 main。children 是 Next 根据当前 URL 选中的页面内容，不是我们手工解析路径得到的。

SessionProvider 包住 Header 和 main，所以两者共享身份。页脚也在其内部。`themeInitScript` 是本站常量构造的首帧主题脚本；dangerouslySetInnerHTML 不能随意拿来插入文章正文或用户输入。这里的 suppressHydrationWarning 仅针对 html 上客户端提前改变 data-theme 的差异，不是通用错误消除器。

## 3. 每个页面导出的函数逐个说明

| 入口文件 | 函数/输入 | 输出与调用链 | 本页独有注意点 |
| --- | --- | --- | --- |
| `src/app/page.tsx` | `Home()`，无参数 | 返回 `<ArticleList />` | 默认模式显示 Hero 与文章卡片 |
| `src/app/archive/page.tsx` | `Archive()` | `<ArticleList archive />` | archive 是布尔 true 的简写；显示归档样式、不显示摘要 |
| `src/app/login/page.tsx` | `Page()` | `<AuthForm />` | register 默认 false，只显示登录字段 |
| `src/app/register/page.tsx` | `Page()` | `<AuthForm register />` | 注册开启昵称、确认密码和验证码 |
| `src/app/posts/[slug]/page.tsx` | `async Page({ params })` | await params，再传 slug 给 ArticleReader | slug 来自 URL 路径，不来自数据库 id |
| `src/app/admin/page.tsx` | `Page()` | `<ArticleList manage />` | 客户端身份提示不替代 API 权限检查 |
| `src/app/admin/new/page.tsx` | `Page()` | Suspense 内 `<Editor />` | 没有 id，因此初始 ready=true，保存使用 POST |
| `src/app/admin/[id]/page.tsx` | `async Page({ params })` | Suspense 内 `<Editor id={(await params).id} />` | id 来自路径，slug 来自查询字符串，二者需对应 |
| `src/app/about/page.tsx` | `About()` | 返回介绍和留言约定 JSX | 没有请求、Effect 或表单事件，metadata 设置页标题 |
| `src/app/not-found.tsx` | `NotFound()` | 返回 404 文案和首页 Link | Link 自带客户端导航，不需要另写 click 方法 |

动态路径当前使用 `params: Promise<{ slug: string }>` 或 Promise<{ id: string }>，所以先 await。不要照搬老版本示例把 params 一律当成同步对象。页面函数本身没有手写事件，是因为交互在其子组件，不是 Next 禁止事件。

`Suspense` 给 Editor 内使用 useSearchParams 的渲染提供回退边界，尤其是静态预渲染与查询参数读取。它不会自动捕获 fetch 的 HTTP 错误；编辑器仍用 error 状态显示业务失败。

## 4. 动态路由和查询参数是两件事

访问 `/admin/某个UUID?slug=learning-typeorm`：Next 从目录 `[id]` 得到 id，Editor 通过 `useSearchParams().get("slug")` 得到 slug。现有读取接口按 slug 查文章，写接口按 id 更新，所以编辑页加载后还要检查返回的 a.id 是否等于路径 id。

访问 `/posts/learning-typeorm`：阅读页只需 slug，API 查到文章之后拿返回的 article.id 加载评论。若请求到不存在的 slug，当前 ArticleReader 会显示 API 的“文章不存在”错误；它没有调用 Next 的 notFound()，因此不要把业务 404 提示等同于 Next 全局 not-found 页面。

## 5. rewrites() 为什么使 Cookie 更容易处理

`next.config.ts` 的异步 rewrites() 返回 source `/api/:path*` 到 API_ORIGIN 的转发表。浏览器调用 `fetch("/api/articles")`，看到的仍是 Next 页面同源地址；内部 Express 端口不直接暴露给前端代码。

这里是转发，不是把 Express 方法编译成 Next Route Handler。运行 `pnpm dev:web` 只启动网页服务，API 不会随之自动启动；根 `pnpm dev` 用 concurrently 同时启动两个命令。API_ORIGIN 是服务端配置，不使用 NEXT_PUBLIC_ 前缀。

复习路径：页面函数 → 客户端组件 → api<T> → `/api` → rewrites → Express。打开浏览器 Network，找到响应 JSON 后再看组件如何 setState，比盯着 page.tsx 找数据库代码更有效。

## 对照源码：src/app/layout.tsx（第 1–30 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/layout.tsx start: 1 end: 30 sha256: 2c420ea29526fe04f3f5ee48ab37a71f2d3dd1268e299190abe526598c0fce40 -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：next.config.ts（第 1–15 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: next.config.ts start: 1 end: 15 sha256: 79c7e1719755b639ca410a20d4e88993711b6a1a76ee58c86ab9b138eec086f3 -->

{% raw %}
````ts
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

## 对照源码：src/app/not-found.tsx（第 1–19 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/not-found.tsx start: 1 end: 19 sha256: 8ef3b086583d7df499e91d0aa56a3aa07eb432e225c93a127e117a0e8cb1c64d -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：src/app/page.tsx（第 1–4 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/page.tsx start: 1 end: 4 sha256: 033ac29ce9c3b3ccba54f1f5832923a6e4d4820c1b14c1c5281a2aaa305085ef -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
````
{% endraw %}

## 对照源码：src/app/about/page.tsx（第 1–25 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/about/page.tsx start: 1 end: 25 sha256: 4f76f4b49789f8018d907d24b05f07c0d6bf4d20c9f1c80c923c6017e0ea444c -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：src/app/admin/page.tsx（第 1–4 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/page.tsx start: 1 end: 4 sha256: 631bf8cdc05478b190ed32ffe351d0c8666e860eb7a9e53320283b66c4a9a6b3 -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export default function Page() {
  return <ArticleList manage />;
}
````
{% endraw %}

## 对照源码：src/app/archive/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/archive/page.tsx start: 1 end: 5 sha256: 8888f2a2b9b56527b3a4f2fe9e162b4458f89211a38efeb468b391b2030c18c6 -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export const metadata = { title: "文章归档" };
export default function Archive() {
  return <ArticleList archive />;
}
````
{% endraw %}

## 对照源码：src/app/login/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/login/page.tsx start: 1 end: 5 sha256: 1cad94120696bfb7a62ae48364ebb26b7355d5e08eb600526505f70028b2c1d8 -->

{% raw %}
````tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "登录" };
export default function Page() {
  return <AuthForm />;
}
````
{% endraw %}

## 对照源码：src/app/register/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/register/page.tsx start: 1 end: 5 sha256: c439d49590283c273d832ae19466d65b9c6b767b84d2d846e06dda2683c6f249 -->

{% raw %}
````tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "注册" };
export default function Page() {
  return <AuthForm register />;
}
````
{% endraw %}

## 对照源码：src/app/posts/[slug]/page.tsx（第 1–8 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/posts/[slug]/page.tsx start: 1 end: 8 sha256: 1b3c17a6a4f41beb68c27c7abfb163dc2c9888b736c636752d80c56add88ab78 -->

{% raw %}
````tsx
import { ArticleReader } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ slug: string }>;
}) {
  return <ArticleReader slug={(await params).slug} />;
}
````
{% endraw %}

## 对照源码：src/app/admin/new/page.tsx（第 1–9 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/new/page.tsx start: 1 end: 9 sha256: 93536324a61c89e0e8940aa165f4b1596ea1eae6048db39dbf57d07573d2ccdf -->

{% raw %}
````tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default function Page() {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor />
    </Suspense>
  );
}
````
{% endraw %}

## 对照源码：src/app/admin/[id]/page.tsx（第 1–13 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/[id]/page.tsx start: 1 end: 13 sha256: 3f6f925c6632f894c52a87224e92bd47c148263ee7d909cf659f55719a0094cc -->

{% raw %}
````tsx
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-02-typescript-react %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-04-styles %})
