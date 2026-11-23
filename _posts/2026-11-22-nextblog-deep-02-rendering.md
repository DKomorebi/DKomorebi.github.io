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
title: React 与 Next.js（二）：服务端组件、客户端组件与页面渲染
date: 2026-11-22 00:02:00 +0800
description: A · React 与 Next.js：结合项目源码、逐段讲解和练习答案深入学习。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


> A · React 与 Next.js · 第 2 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## “服务端”在这个项目里有两个含义

Next.js 服务端负责页面路由、渲染和代理；Express 服务端负责认证、文章、评论和数据库业务。Server Component 中的 Server 指 Next.js 的渲染环境，不是说组件放进了 server 目录。目录名只是项目组织，实际执行环境由入口和引用关系决定。


代码位置：`src/app/layout.tsx`（完整文件）。

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


RootLayout 没有 use client，是服务端组件入口。它建立 html、body、页眉、主要内容和页脚。children 是当前路由内容；SessionProvider 包在外面，使导航和页面交互能共享会话状态。Provider 是客户端组件，并不会因为包住 children 就把所有服务端 children 的源码强制转换成浏览器代码。

## 为什么 full-blog.tsx 写 use client

它使用 useState、useEffect、useContext、点击事件与表单事件，需要浏览器交互能力。use client 声明的是模块边界，不是“这个文件永远不会参与首屏服务端输出”。客户端组件仍可能参与初始 HTML 的预渲染，然后在浏览器恢复交互。

也正因为如此，不能在任意渲染语句中直接读取 window、document 或 localStorage。ThemeToggle 在 effect 和事件里读浏览器状态；主题首帧还有一段明确插入 head 的初始化脚本。这些都是针对执行时机的设计。

## 打开文章时正文在哪里加载


代码位置：`src/app/posts/[slug]/page.tsx`（完整文件）。

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


代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
````tsx
export function ArticleReader({ slug }: { slug: string }) {
  const [article, setArticle] = useState<Article | null>(null),
    [error, setError] = useState(""),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    setArticle(null);
    api<Article>("/articles/" + encodeURIComponent(slug))
      .then((a) => {
        if (alive) {
          setArticle(a);
          setError("");
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [slug, revision]);
````
{% endraw %}


第一段是 Next.js 动态路由：等待 params 后将 slug 交给 ArticleReader。第二段是 React 客户端 effect：请求 /api/articles/slug，再调用 setArticle。它说明当前文章正文在浏览器请求，而不是由那个 async 页面函数直接查询数据库。

第一次渲染 article 为 null，显示加载提示。effect 在客户端运行，返回的数据更新状态，组件重新渲染标题、Markdown 和评论。清理函数把 alive 改成 false，阻止已过期请求在换页之后继续更新该次结果；它不会取消已经发出的网络请求。

## 预渲染、hydration 与异步请求的区别

预渲染解决首屏 HTML 怎样产生；hydration 将客户端交互逻辑与已有界面对应；effect 获取数据是浏览器运行后的另一个步骤。因此看到初始 HTML 并不代表数据库正文已加载，看到 use client 也不意味着页面最初完全没有 HTML。

本项目的 build 输出里，首页等路径能生成外框，动态详情由动态路由处理，但全文仍走客户端 API。不能将 .next-fullstack 当成一包纯静态页面直接搬到任意静态主机。

## 当前实现的边界

文章不存在时 API 返回 404，ArticleReader 展示错误；页面文档本身不一定因此变成 HTTP 404。文章标题也没有通过动态 metadata 自动写入每篇文档的 head。以后如果目标是更好的文章 SEO，可以重构为服务端获取正文，再同时处理 notFound、metadata 与缓存。

这些是后续优化方向，不是本报告已经完成的代码变更。学习时先把当前的真实执行路径理解清楚，再讨论如何重构。

## 练习与参考答案

在浏览器 Network 中区分页面文档请求与 /api/articles/... 请求，观察后者返回 JSON。禁用 JavaScript 后再打开文章，可以帮助理解当前正文依赖客户端请求；做完恢复设置。

为什么服务器文件不能随便 import 到客户端组件？因为后端模块可能包含数据库驱动、进程环境变量与 Node API；浏览器既不具备相同运行环境，也不应该获得数据库凭据。正确的连接点是 HTTP 接口。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-01-react-next %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-03-request-path %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [React：描述用户界面](https://react.dev/learn/describing-the-ui)
- [Next.js 服务端与客户端组件](https://nextjs.org/docs/app/getting-started/server-and-client-components)
