---
layout: post
title: NextBlog（二）：Next.js 路由、布局与前后端边界
date: 2026-11-17 09:01:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 理解 App Router、动态参数、Server/Client Component 与同源代理。
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

理解 App Router、动态参数、Server/Client Component 与同源代理。

> 本文是 NextBlog 全栈博客系列第 2 / 15 篇，内容对应当前全栈版本。

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

## 文件路径怎样变成网址

项目使用 App Router。src/app 下的文件夹代表路径段，page.tsx 是页面入口，layout.tsx 包裹子页面。组件文件放进 src/components 不会自动产生网址。

| 网址 | 页面文件 | 主要组件 |
| --- | --- | --- |
| / | src/app/page.tsx | ArticleList |
| /archive | src/app/archive/page.tsx | ArticleList archive |
| /about | src/app/about/page.tsx | 关于页正文 |
| /posts/文章标识 | src/app/posts/[slug]/page.tsx | ArticleReader |
| /login、/register | 对应目录的 page.tsx | AuthForm |
| /admin | src/app/admin/page.tsx | ArticleList manage |
| /admin/new | src/app/admin/new/page.tsx | Editor |
| /admin/文章ID | src/app/admin/[id]/page.tsx | Editor id |

slug 是便于阅读的唯一文章标识，例如 my-first-post；id 是数据库 UUID。阅读接口按 slug 查询，修改和删除按 id 操作。管理列表生成编辑链接时还带 slug 查询参数，Editor 加载后会验证返回的文章 id 与路径一致。

## 动态参数为什么需要 await

代码位置：`src/app/posts/[slug]/page.tsx`（完整文件）。

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


方括号表示动态路径段。本项目采用的 Next.js 版本把 params 声明为 Promise，因此页面是 async 函数，先 await 再取 slug。这里仅将标识传入 ArticleReader；真正加载文章正文的请求在客户端组件中执行。不能看到 async 页面就认为正文已经在服务端读取了。

## 公共布局负责稳定的页面外框

代码位置：`src/app/layout.tsx`（完整文件）。

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


children 是当前路由渲染出来的内容。Header、页脚和 SessionProvider 放在布局中，让页面共享外框和登录状态。metadata 提供基础标题与描述；当前文章标题没有在这个布局中自动变成每篇文章的独立 SEO 元信息。

head 中的脚本来自本地固定字符串 themeInitScript，用于绘制前确定深浅主题。这里没有插入用户文章。suppressHydrationWarning 只用于 html 上预先改变的主题属性，不是忽略所有界面错误的开关。

## Server Component 与 Client Component

app 中没有标记的组件默认处于服务端组件模型。本项目 full-blog.tsx 和 theme-toggle.tsx 顶部的 `"use client"` 建立客户端边界，使它们能使用 state、effect、点击事件等能力。它们仍可能参与首屏预渲染，浏览器对象应在合适的 effect 或事件中访问。

服务端布局可以把 children 传给客户端 Provider；这不意味着所有 children 都被改写成客户端组件。应沿 import 边界和组件职责判断。数据库模块只在 server 目录的后端入口使用，不导入浏览器业务组件。

## 为什么浏览器只请求 /api

代码位置：`next.config.ts`（完整文件）。

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


浏览器请求 localhost:3000/api/articles，Next.js 将请求转发至 Express 的 4000 端口。地址栏保持网站的同一源，api() 使用 same-origin 携带 Cookie。代理不会替代身份检查；真正的权限判断仍在 Express 中。

API_ORIGIN 是服务器连接目标，APP_ORIGIN 则参与写请求的来源校验。改变访问端口后，要同步调整配置并重启服务；生产环境应配好 HTTPS、转发头与受控代理链。

## 构建结果要怎样理解

pnpm build 输出 .next-fullstack。部分页面可以预渲染出外框，然后在浏览器请求数据库内容；这不代表整个项目能够仅靠静态文件运行。当前应用需要 Next.js 服务、Express 和 PostgreSQL。

未找到文章时 API 返回 404，ArticleReader 显示错误；当前实现没有将这个客户端结果转换成详情文档本身的 HTTP 404。src/app/not-found.tsx 用于 Next.js 未匹配路由等情况。若以后重构正文服务端获取，应一起处理 notFound、标题与缓存策略。

## 练习

对照路由表打开每个页面，在 Network 面板观察页面导航与 /api 请求的区别。然后从首页进入文章，沿 page.tsx → ArticleReader → api() → Express 依次找到执行位置。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-01-setup-react %})

[下一篇]({% post_url 2026-11-17-intern-blog-03-typescript-data %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
