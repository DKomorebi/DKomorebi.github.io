---
layout: post
title: Next.js博客（八）：全部页面与组件源码归档
date: 2026-11-17 09:07:00 +0800
categories:
  - 实习项目
category_path: 开发/实习项目/博客系统
description: 集中归档拾页博客的全部页面、组件、全局样式与依赖数据，附文件索引和完整源码，方便对照学习与复建。
keywords: Next.js, React, 页面源码, 组件源码, TypeScript, Tailwind CSS
cover:
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

前面的章节按知识点讲解代码，这一篇把全部页面和组件集中归档。每段代码都从当前 Demo 的真实文件读取，保留完整实现，方便按文件查找、对照和复制。

> 本文是「实习博客实战」系列第 8 / 14 篇。以下路径都相对于拾页 Demo 项目根目录。这些代码在本篇中作为代码块展示，不会在 Jekyll 博客中直接执行。

> 版本说明：本篇保留静态 Demo 阶段的代码。当前全栈版本请从[第九篇]({% post_url 2026-11-20-intern-blog-09-database %})开始。

## 系列导航

1. [Next.js博客（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [Next.js博客（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [Next.js博客（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [Next.js博客（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [Next.js博客（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [Next.js博客（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [Next.js博客（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [Next.js博客（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})
9. [实习博客实战（九）：从静态页面走向 PostgreSQL 全栈博客]({% post_url 2026-11-20-intern-blog-09-database %})
10. [实习博客实战（十）：Express、Cookie、Session 与权限边界]({% post_url 2026-11-20-intern-blog-10-auth-api %})
11. [实习博客实战（十一）：全栈路由与每个页面完整源码]({% post_url 2026-11-20-intern-blog-11-pages %})
12. [实习博客实战（十二）：搜索、编辑器、登录与评论组件详解]({% post_url 2026-11-20-intern-blog-12-components %})
13. [实习博客实战（十三）：DaisyUI 卡片、Lucide 图标与深色主题]({% post_url 2026-11-20-intern-blog-13-theme %})
14. [实习博客实战（十四）：接口测试、运行配置与发布检查]({% post_url 2026-11-20-intern-blog-14-tests %})

## 使用说明

源码快照来自 2026-11-17 的静态 Demo，使用 Next.js App Router、TypeScript 和 Tailwind CSS v4。请先按第一篇配置 package.json、tsconfig.json、next.config.ts、postcss.config.mjs 与 next-env.d.ts，再按下列路径放入文件。本文同时收录组件依赖的类型、模拟数据和 CSS，覆盖当前 src 目录中的全部文件。

不要把这些 TSX 文件放进 Jekyll 的 _posts 中期待它直接执行。_posts 负责渲染本篇教学 Markdown；真正运行 Demo 时，需要在独立的 Next.js 项目里创建对应文件并安装依赖。

## 文件索引

| 文件 | 用途 | 完整代码 |
| --- | --- | --- |
| `src/app/layout.tsx` | 公共布局 | [查看](#source-1) |
| `src/app/page.tsx` | 首页 | [查看](#source-2) |
| `src/app/posts/[slug]/page.tsx` | 文章详情 | [查看](#source-3) |
| `src/app/archive/page.tsx` | 文章归档 | [查看](#source-4) |
| `src/app/about/page.tsx` | 关于页面 | [查看](#source-5) |
| `src/app/not-found.tsx` | 404 页面 | [查看](#source-6) |
| `src/components/header.tsx` | 导航组件 | [查看](#source-7) |
| `src/components/cover.tsx` | 文章封面组件 | [查看](#source-8) |
| `src/components/post-browser.tsx` | 文章浏览组件 | [查看](#source-9) |
| `src/app/globals.css` | 全局样式 | [查看](#source-10) |
| `src/types/post.ts` | 共享类型 | [查看](#source-11) |
| `src/lib/posts.ts` | 完整模拟数据 | [查看](#source-12) |

## 组件之间如何配合

{% raw %}
```text
RootLayout
├─ Header
├─ 当前路由页面
│  ├─ Home → Cover + PostBrowser → Cover
│  ├─ Article → Cover
│  ├─ Archive
│  ├─ About
│  └─ NotFound
└─ Footer

PostBrowser：浏览器状态与交互
posts.ts：文章数组、查询函数、日期格式化
post.ts：共享类型
globals.css：全局主题与封面样式
```
{% endraw %}

<a id="source-1"></a>

## 1. 公共布局

文件：`src/app/layout.tsx`

所有页面共用的 HTML 外壳，包含导航、页脚、元数据和全局样式入口。children 表示当前路由的页面。

{% raw %}
```tsx
import type { Metadata } from "next";
import Link from "next/link";
import { Header } from "@/components/header";
import "./globals.css";

export const metadata: Metadata = {
  title: {
    default: "拾页 Fieldnotes · 记录每一个小小的进步",
    template: "%s · 拾页",
  },
  description: "一份关于代码、思考与成长的开发者学习手记。",
};

export default function RootLayout({
  children,
}: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="zh-CN">
      <body>
        <Header />
        {children}
        <footer className="mx-auto mt-20 flex max-w-7xl flex-wrap justify-between gap-4 border-t border-line px-6 py-8 text-xs text-muted lg:px-10">
          <p>© 2026 拾页 Fieldnotes · 在记录中，慢慢生长。</p>
          <div className="flex gap-5">
            <span>用 Next.js 构建</span>
            <Link href="/about/">关于这个小站 ↗</Link>
          </div>
        </footer>
      </body>
    </html>
  );
}
```
{% endraw %}

<a id="source-2"></a>

## 2. 首页

文件：`src/app/page.tsx`

路径为 /。组织介绍区、精选文章、列表和作者侧栏，通过 props 把文章数组交给 PostBrowser。

{% raw %}
```tsx
import Link from "next/link";
import { ArrowDown, ArrowUpRight, BookOpen, Code2, Sprout } from "lucide-react";
import { Cover } from "@/components/cover";
import { PostBrowser } from "@/components/post-browser";
import { posts } from "@/lib/posts";

export default function Home() {
  const featured = posts[0];
  return (
    <main className="mx-auto max-w-7xl px-6 lg:px-10">
      <section className="relative flex items-center justify-between gap-6 border-b border-line py-14 md:py-18">
        <div className="relative z-10">
          <div className="mb-5 flex items-center gap-2 text-xs text-accent">
            <span className="size-1.5 rounded-full bg-accent" />{" "}
            一个开发者的学习手记
          </div>
          <h1 className="text-4xl leading-[1.5] font-bold tracking-tight md:text-5xl">
            在代码里探索，
            <br />
            在记录中
            <span className="relative text-accent">
              生长
              <span className="absolute -bottom-2 left-0 h-1 w-full -rotate-2 rounded-full bg-[#cbd4bc]" />
            </span>
            。
          </h1>
          <p className="mt-6 text-sm leading-7 text-muted">
            关于技术、思考，以及每一个小小的进步。
            <br />
            把学到的写下来，把走过的路分享给你。
          </p>
          <a
            href="#articles"
            className="mt-7 inline-flex items-center gap-3 text-xs font-semibold"
          >
            翻开最新一页 <ArrowDown size={15} />
          </a>
        </div>
        <div
          aria-hidden="true"
          className="hero-grid relative hidden h-64 w-80 items-center justify-center md:flex"
        >
          <div className="absolute h-56 w-48 rotate-12 rounded-lg border border-[#d0d5c6] bg-[#e9ecdf]" />
          <div className="relative h-56 w-48 -rotate-6 rounded-lg border border-line bg-white p-6 shadow-lg shadow-stone-200/40">
            <span className="font-mono text-[9px] text-muted">
              A NOTE TO MYSELF
            </span>
            <Sprout size={48} strokeWidth={1.1} className="my-5 text-accent" />
            <p className="text-xs leading-6">
              保持好奇。
              <br />
              慢慢来，也是在前进。
            </p>
            <div className="mt-3 h-px w-8 bg-accent" />
          </div>
          <span className="absolute right-1 bottom-4 rotate-6 font-mono text-[10px] text-muted">
            one page at a time.
          </span>
        </div>
      </section>
      <section
        aria-label="精选文章"
        className="my-10 overflow-hidden rounded-2xl border border-line bg-[#eeeee5]"
      >
        <Link
          href={`/posts/${featured.slug}/`}
          className="group grid md:grid-cols-2"
        >
          <div className="flex flex-col justify-center p-7 md:p-9">
            <span className="mb-5 flex items-center gap-2 text-[10px] font-semibold tracking-widest text-accent">
              <span className="size-1.5 rounded-full bg-accent" /> 本期精选 /
              EDITOR’S PICK
            </span>
            <h2 className="max-w-md text-2xl leading-relaxed font-bold group-hover:text-accent">
              {featured.title}
            </h2>
            <p className="mt-3 max-w-sm text-xs leading-6 text-muted">
              {featured.excerpt}
            </p>
            <span className="mt-6 flex items-center gap-3 text-xs font-semibold">
              开始阅读 <ArrowUpRight size={16} />
            </span>
          </div>
          <Cover kind="next" large />
        </Link>
      </section>
      <div className="grid items-start gap-10 lg:grid-cols-[minmax(0,1fr)_260px] xl:gap-14">
        <PostBrowser posts={posts} />
        <aside className="space-y-7 lg:pt-1">
          <section className="rounded-2xl border border-line bg-[#efefe7] p-6">
            <span className="mb-5 inline-flex size-12 items-center justify-center rounded-full bg-[#dce3d1] text-accent">
              <Code2 size={24} />
            </span>
            <p className="font-mono text-[10px] tracking-widest text-muted">
              HELLO, WORLD.
            </p>
            <h2 className="mt-2 text-lg font-bold">
              你好，我是小拾 <span className="text-sm">✳</span>
            </h2>
            <p className="mt-3 text-xs leading-7 text-muted">
              一名正在升级打怪的开发实习生。在这里记录从「不太懂」到「原来如此」的每一步。
            </p>
            <Link
              href="/about/"
              className="mt-5 inline-flex items-center gap-2 text-xs font-semibold"
            >
              多了解我一点 <ArrowUpRight size={14} />
            </Link>
          </section>
          <section className="px-1">
            <h2 className="mb-5 flex items-center gap-2 text-sm font-bold">
              <BookOpen size={16} /> 正在探索
            </h2>
            <div className="flex flex-wrap gap-2">
              {[
                "Next.js",
                "TypeScript",
                "Tailwind CSS",
                "Express",
                "PostgreSQL",
                "TypeORM",
              ].map((tag) => (
                <span
                  key={tag}
                  className="rounded-md border border-line bg-white/50 px-3 py-1.5 font-mono text-[10px] text-muted"
                >
                  {tag}
                </span>
              ))}
            </div>
          </section>
          <section className="border-t border-line px-1 pt-6">
            <h2 className="mb-4 text-sm font-bold">小站数字</h2>
            <div className="flex gap-10">
              <div>
                <p className="font-mono text-2xl">06</p>
                <p className="mt-1 text-[11px] text-muted">篇学习记录</p>
              </div>
              <div>
                <p className="font-mono text-2xl">03</p>
                <p className="mt-1 text-[11px] text-muted">个探索方向</p>
              </div>
            </div>
          </section>
          <div className="border-t border-line pt-6">
            <p className="font-serif text-3xl text-[#bbc4ac]">“</p>
            <p className="text-xs leading-7 text-muted">
              学习不是填满水桶，
              <br />
              而是点燃火焰。
            </p>
            <p className="mt-3 font-mono text-[9px] tracking-wider text-muted">
              STAY CURIOUS. KEEP BUILDING.
            </p>
          </div>
        </aside>
      </div>
    </main>
  );
}
```
{% endraw %}

<a id="source-3"></a>

## 3. 文章详情

文件：`src/app/posts/[slug]/page.tsx`

路径为 /posts/文章标识/。根据 slug 查询文章，通过 generateStaticParams 为六篇文章生成静态路径。

{% raw %}
```tsx
import type { Metadata } from "next";
import Link from "next/link";
import { notFound } from "next/navigation";
import { ArrowLeft, ArrowUpRight } from "lucide-react";
import { Cover } from "@/components/cover";
import { formatDate, getPost, posts } from "@/lib/posts";

type ArticleProps = { params: Promise<{ slug: string }> };

// 静态导出必须提前列出所有动态地址。
export function generateStaticParams() {
  return posts.map((post) => ({ slug: post.slug }));
}

export async function generateMetadata({
  params,
}: ArticleProps): Promise<Metadata> {
  const post = getPost((await params).slug);
  return { title: post?.title ?? "文章不存在", description: post?.excerpt };
}

export default async function Article({ params }: ArticleProps) {
  const { slug } = await params;
  const post = getPost(slug);
  if (!post) notFound();
  const related = posts
    .filter((item) => item.category === post.category && item.slug !== slug)
    .slice(0, 2);
  return (
    <main className="mx-auto max-w-4xl px-6 pt-10">
      <Link
        href="/#articles"
        className="inline-flex items-center gap-2 text-xs text-muted hover:text-ink"
      >
        <ArrowLeft size={14} /> 返回文章列表
      </Link>
      <article className="mt-10">
        <div className="mb-5 flex gap-4 text-xs text-muted">
          <span className="font-semibold text-accent">{post.category}</span>
          <time dateTime={post.date}>{formatDate(post.date)}</time>
          <span>{post.minutes} 分钟阅读</span>
        </div>
        <h1 className="text-3xl leading-relaxed font-bold tracking-tight md:text-4xl">
          {post.title}
        </h1>
        <p className="mt-5 text-sm leading-8 text-muted">{post.excerpt}</p>
        <div className="mt-6 flex gap-2">
          {post.tags.map((tag) => (
            <span
              key={tag}
              className="rounded-full bg-[#e9ecdf] px-3 py-1 text-xs text-accent"
            >
              {tag}
            </span>
          ))}
        </div>
        <div className="my-10 overflow-hidden rounded-2xl">
          <Cover kind={post.cover} large />
        </div>
        <nav
          aria-label="文章目录"
          className="mb-10 rounded-xl border border-line p-6"
        >
          <h2 className="mb-3 text-sm font-bold">这一页的内容</h2>
          <ol className="space-y-3 text-sm text-muted">
            {post.sections.map((section, index) => (
              <li key={section.heading}>
                <a className="hover:text-accent" href={`#section-${index}`}>
                  {String(index + 1).padStart(2, "0")} / {section.heading}
                </a>
              </li>
            ))}
          </ol>
        </nav>
        <div className="mx-auto max-w-2xl">
          {post.sections.map((section, index) => (
            <section
              key={section.heading}
              id={`section-${index}`}
              className="mb-10 scroll-mt-8"
            >
              <h2 className="mb-5 text-xl font-bold">{section.heading}</h2>
              {section.paragraphs.map((paragraph) => (
                <p
                  key={paragraph}
                  className="mb-4 text-[15px] leading-8 text-[#555b52]"
                >
                  {paragraph}
                </p>
              ))}
              {section.code && (
                <pre className="mt-5 overflow-x-auto rounded-xl bg-[#252d29] p-5 text-xs leading-7 text-[#e5edda]">
                  <code>{section.code}</code>
                </pre>
              )}
            </section>
          ))}
        </div>
      </article>
      <section className="mt-14 border-t border-line pt-8">
        <h2 className="mb-5 text-lg font-bold">接着翻一页</h2>
        {related.length ? (
          related.map((item) => (
            <Link
              className="mb-3 flex items-center justify-between gap-3 rounded-xl border border-line p-5 text-sm hover:bg-white"
              key={item.slug}
              href={`/posts/${item.slug}/`}
            >
              {item.title}
              <ArrowUpRight size={16} className="shrink-0" />
            </Link>
          ))
        ) : (
          <Link href="/archive/" className="text-sm text-accent">
            去归档看看更多文章 ↗
          </Link>
        )}
      </section>
    </main>
  );
}
```
{% endraw %}

<a id="source-4"></a>

## 4. 文章归档

文件：`src/app/archive/page.tsx`

路径为 /archive/。复用文章数据生成时间线；年月标题当前对应示例数据，需要时可改为按年月分组。

{% raw %}
```tsx
import Link from "next/link";
import { ArrowUpRight } from "lucide-react";
import { posts, formatDate } from "@/lib/posts";

export const metadata = { title: "文章归档" };

export default function Archive() {
  return (
    <main className="mx-auto min-h-[65vh] max-w-4xl px-6 py-14">
      <p className="font-mono text-xs tracking-widest text-accent">
        THE ARCHIVE
      </p>
      <h1 className="mt-5 text-4xl font-bold">走过的每一页</h1>
      <p className="mt-5 text-sm text-muted">
        共 {posts.length} 篇记录，把零散的进步连成一条线。
      </p>
      <h2 className="mt-14 mb-5 font-mono text-xl">
        2026 <span className="text-sm text-muted">/ 11 月</span>
      </h2>
      <div className="border-t border-line">
        {posts.map((post) => (
          <Link
            key={post.slug}
            href={`/posts/${post.slug}/`}
            className="group flex flex-wrap items-center gap-4 border-b border-line py-6"
          >
            <time dateTime={post.date} className="font-mono text-xs text-muted">
              {formatDate(post.date)}
            </time>
            <h3 className="min-w-48 flex-1 text-sm font-semibold group-hover:text-accent">
              {post.title}
            </h3>
            <span className="text-xs text-muted">{post.category}</span>
            <ArrowUpRight size={16} />
          </Link>
        ))}
      </div>
    </main>
  );
}
```
{% endraw %}

<a id="source-5"></a>

## 5. 关于页面

文件：`src/app/about/page.tsx`

路径为 /about/。展示作者说明与学习路线，learning 锚点供导航里的“一起学习”链接使用。

{% raw %}
```tsx
import { Sprout } from "lucide-react";

export const metadata = { title: "关于小站" };

export default function About() {
  return (
    <main className="mx-auto max-w-3xl px-6 py-14">
      <Sprout size={40} strokeWidth={1.3} className="text-accent" />
      <p className="mt-7 font-mono text-xs tracking-widest text-muted">
        A LITTLE ABOUT ME
      </p>
      <h1 className="mt-4 text-4xl font-bold">每一个高手，都曾是新手。</h1>
      <div className="mt-8 space-y-5 text-sm leading-8 text-muted">
        <p>
          你好，欢迎来到拾页。这是一个开发实习生的示例博客，也是一份可以边看、边改、边学习的小项目。「小拾」是
          Demo 中的虚构作者，你可以把名字、介绍和文章替换成自己的内容。
        </p>
        <p>
          我相信，把学到的东西用自己的话讲出来，是检验理解的好办法。这里记录技术探索，也记录卡住时的困惑和解决问题后的那一点开心。
        </p>
      </div>
      <section
        id="learning"
        className="mt-12 scroll-mt-8 rounded-2xl border border-line bg-[#efefe7] p-7"
      >
        <h2 className="text-xl font-bold">一起学习，从这条路线开始</h2>
        <ol className="mt-6 space-y-6">
          {[
            ["01", "先把页面搭起来", "认识 React 组件、Next.js 路由与 JSX。"],
            ["02", "给数据定义形状", "用 TypeScript 接口描述文章与组件参数。"],
            [
              "03",
              "让界面适应不同屏幕",
              "理解 Tailwind CSS 的工具类与响应式前缀。",
            ],
            [
              "04",
              "走向完整的博客系统",
              "下一阶段用 Express、PostgreSQL 和 TypeORM 接入真实数据。",
            ],
          ].map(([step, title, description]) => (
            <li key={step} className="flex gap-5">
              <span className="font-mono text-sm text-accent">{step}</span>
              <div>
                <h3 className="text-sm font-bold">{title}</h3>
                <p className="mt-2 text-xs leading-6 text-muted">
                  {description}
                </p>
              </div>
            </li>
          ))}
        </ol>
      </section>
      <p className="mt-8 text-sm leading-7 text-muted">
        本项目附带详细的中文教学文档，请从项目根目录的 README.md
        开始阅读。当前版本是静态 Demo，暂未接入登录、文章发布和数据库。
      </p>
    </main>
  );
}
```
{% endraw %}

<a id="source-6"></a>

## 6. 404 页面

文件：`src/app/not-found.tsx`

展示找不到页面时的提示和首页入口；静态托管仍需正确配置 404 状态。

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

<a id="source-7"></a>

## 7. 导航组件

文件：`src/components/header.tsx`

供 RootLayout 复用，通过 Next.js Link 跳转首页、归档和关于页面。

{% raw %}
```tsx
import Link from "next/link";
import { ArrowUpRight, Feather } from "lucide-react";

export function Header() {
  return (
    <header className="border-b border-line bg-paper">
      <div className="mx-auto flex max-w-7xl flex-wrap items-center justify-between gap-4 px-6 py-6 lg:px-10">
        <Link
          href="/"
          className="flex items-center gap-3"
          aria-label="拾页首页"
        >
          <span className="flex size-9 items-center justify-center rounded-xl bg-ink text-white">
            <Feather size={20} />
          </span>
          <span className="text-xl font-bold tracking-tight">
            拾页
            <span className="ml-3 font-mono text-xs font-normal tracking-[.15em] text-muted">
              FIELDNOTES
            </span>
          </span>
        </Link>
        <nav
          aria-label="主导航"
          className="flex items-center gap-5 text-sm md:gap-8"
        >
          <Link className="nav-link" href="/">
            文章
          </Link>
          <Link className="nav-link" href="/archive/">
            归档
          </Link>
          <Link className="nav-link" href="/about/">
            关于
          </Link>
          <Link
            href="/about/#learning"
            className="flex items-center gap-2 rounded-full border border-line px-4 py-2 text-xs hover:bg-white"
          >
            一起学习 <ArrowUpRight size={14} />
          </Link>
        </nav>
      </div>
    </header>
  );
}
```
{% endraw %}

<a id="source-8"></a>

## 8. 文章封面组件

文件：`src/components/cover.tsx`

根据 kind 选择标题、图标和主题样式。large 区分精选大封面和列表小封面，不需要远程图片。

{% raw %}
```tsx
import {
  ArrowUpRight,
  Braces,
  Database,
  Feather,
  Layers,
  Wind,
} from "lucide-react";
import type { Post } from "@/types/post";

const themes = {
  next: {
    label: "NEXT.JS",
    subtitle: "Build something worth sharing.",
    icon: Layers,
  },
  typescript: {
    label: "TypeScript",
    subtitle: "A little structure. A lot of clarity.",
    icon: Braces,
  },
  tailwind: {
    label: "tailwindcss",
    subtitle: "Small utilities. Endless possibilities.",
    icon: Wind,
  },
  database: {
    label: "PostgreSQL",
    subtitle: "A place for every idea.",
    icon: Database,
  },
  express: {
    label: "express",
    subtitle: "Connect the dots.",
    icon: ArrowUpRight,
  },
  notes: {
    label: "Small steps.",
    subtitle: "Keep learning. Keep building.",
    icon: Feather,
  },
};

export function Cover({
  kind,
  large = false,
}: {
  kind: Post["cover"];
  large?: boolean;
}) {
  const { label, subtitle, icon: Icon } = themes[kind];
  return (
    <div
      aria-hidden="true"
      className={`cover cover-${kind} ${large ? "min-h-72 md:min-h-96" : "h-44"}`}
    >
      <span className="cover-orbit" />
      <span className="cover-orbit orbit-two" />
      <div className="relative z-10">
        <Icon
          size={large ? 48 : 32}
          strokeWidth={1.2}
          className="mb-5 opacity-80"
        />
        <p
          className={`${large ? "text-4xl md:text-5xl" : "text-3xl"} font-semibold tracking-tight`}
        >
          {label}
          <span className="text-orange-400">
            {kind === "next" ? " ↗" : ""}
          </span>
        </p>
        <p className="mt-3 font-mono text-[10px] tracking-wide opacity-60">
          {subtitle}
        </p>
      </div>
      <span className="absolute right-5 bottom-4 font-mono text-[9px] opacity-40">
        FIELDNOTES / {kind.toUpperCase()}
      </span>
    </div>
  );
}
```
{% endraw %}

<a id="source-9"></a>

## 9. 文章浏览组件

文件：`src/components/post-browser.tsx`

唯一显式声明 use client 的业务组件。使用状态、受控输入、过滤和排序实现交互，包含空状态与重置操作。

{% raw %}
```tsx
"use client";

import { useState } from "react";
import Link from "next/link";
import { ArrowUpRight, Search, SlidersHorizontal, X } from "lucide-react";
import type { Category, Post } from "@/types/post";
import { Cover } from "@/components/cover";
import { formatDate } from "@/lib/posts";

const categories: (Category | "全部文章")[] = [
  "全部文章",
  "前端开发",
  "后端探索",
  "成长手记",
];

export function PostBrowser({ posts }: { posts: Post[] }) {
  const [query, setQuery] = useState("");
  const [category, setCategory] = useState<Category | "全部文章">("全部文章");
  const [sort, setSort] = useState("newest");
  // 从原数据派生展示结果，不直接修改 props。
  const visible = posts
    .filter((post) => {
      const matchesCategory =
        category === "全部文章" || post.category === category;
      const matchesQuery =
        `${post.title} ${post.excerpt} ${post.tags.join(" ")}`
          .toLowerCase()
          .includes(query.trim().toLowerCase());
      return matchesCategory && matchesQuery;
    })
    .sort((a, b) =>
      sort === "newest"
        ? b.date.localeCompare(a.date)
        : a.date.localeCompare(b.date),
    );

  return (
    <section id="articles" className="scroll-mt-6">
      <div className="mb-6 flex items-end justify-between">
        <h2 className="text-2xl font-bold tracking-tight">
          最近写下的
          <span className="ml-3 align-middle font-mono text-xs font-normal text-muted">
            / {posts.length.toString().padStart(2, "0")}
          </span>
        </h2>
        <span className="hidden text-xs text-muted sm:block">
          一点积累，一点进步
        </span>
      </div>
      <div className="mb-5 flex items-center gap-3 rounded-xl border border-line bg-white/60 px-4">
        <Search size={17} className="shrink-0 text-muted" />
        <label htmlFor="post-search" className="sr-only">
          搜索文章、技术或关键词
        </label>
        <input
          id="post-search"
          value={query}
          onChange={(event) => setQuery(event.target.value)}
          placeholder="搜索文章、技术或关键词…"
          className="min-w-0 flex-1 bg-transparent py-3 text-sm outline-none"
        />
        {query && (
          <button
            aria-label="清空搜索"
            onClick={() => setQuery("")}
            className="p-2"
          >
            <X size={16} />
          </button>
        )}
      </div>
      <div className="mb-7 flex flex-wrap items-center justify-between gap-4 border-b border-line pb-4">
        <div className="flex flex-wrap gap-2">
          {categories.map((item) => (
            <button
              key={item}
              aria-pressed={category === item}
              onClick={() => setCategory(item)}
              className={`rounded-full px-3.5 py-2 text-xs transition-colors ${category === item ? "bg-ink text-white" : "text-muted hover:bg-white hover:text-ink"}`}
            >
              {item}
            </button>
          ))}
        </div>
        <label className="flex items-center gap-1 text-xs text-muted">
          <SlidersHorizontal size={14} />
          <span className="sr-only">文章排序</span>
          <select
            value={sort}
            onChange={(event) => setSort(event.target.value)}
            className="bg-transparent p-1"
          >
            <option value="newest">最新发布</option>
            <option value="oldest">最早发布</option>
          </select>
        </label>
      </div>
      <p aria-live="polite" className="sr-only">
        找到 {visible.length} 篇文章
      </p>
      {visible.length ? (
        <div className="grid grid-cols-1 gap-x-6 gap-y-8 md:grid-cols-2">
          {visible.map((post) => (
            <article
              key={post.slug}
              className="group overflow-hidden rounded-2xl border border-line bg-white/60 transition-colors hover:border-accent/50"
            >
              <Link href={`/posts/${post.slug}/`} className="block">
                <Cover kind={post.cover} />
                <div className="p-5">
                  <div className="mb-3 flex items-center gap-3 text-[11px]">
                    <span className="font-semibold text-accent">
                      {post.category}
                    </span>
                    <span className="text-muted">{post.minutes} 分钟阅读</span>
                  </div>
                  <h3 className="text-lg leading-relaxed font-bold group-hover:text-accent">
                    {post.title}
                  </h3>
                  <p className="mt-2 text-xs leading-6 text-muted">
                    {post.excerpt}
                  </p>
                  <div className="mt-5 flex items-center justify-between border-t border-line pt-4 text-[11px] text-muted">
                    <time dateTime={post.date}>{formatDate(post.date)}</time>
                    <span className="flex items-center gap-1">
                      阅读全文 <ArrowUpRight size={14} />
                    </span>
                  </div>
                </div>
              </Link>
            </article>
          ))}
        </div>
      ) : (
        <div className="rounded-2xl border border-dashed border-line px-5 py-16 text-center">
          <Search className="mx-auto mb-4 text-muted" />
          <h3 className="font-bold">暂时没有找到相关文章</h3>
          <p className="mt-2 text-sm text-muted">
            换个关键词，或者看看全部文章吧。
          </p>
          <button
            onClick={() => {
              setQuery("");
              setCategory("全部文章");
            }}
            className="mt-5 rounded-full bg-ink px-5 py-2 text-sm text-white"
          >
            重置筛选
          </button>
        </div>
      )}
      {visible.length > 0 && (
        <p className="mt-9 text-center text-xs text-muted">
          — 已展示全部 {visible.length} 篇文章，新的故事正在路上 —
        </p>
      )}
    </section>
  );
}
```
{% endraw %}

<a id="source-10"></a>

## 10. 全局样式

文件：`src/app/globals.css`

Tailwind v4 入口、主题颜色、字体、键盘焦点和封面装饰。应与页面和封面组件一起复制。

{% raw %}
```css
@import "tailwindcss";

@theme {
  --color-paper: #f8f7f3;
  --color-ink: #272b27;
  --color-muted: #777b73;
  --color-line: #e4e5dc;
  --color-accent: #526545;
  --font-sans: Arial, "Microsoft YaHei", "PingFang SC", sans-serif;
}

html {
  scroll-behavior: smooth;
}
body {
  background: var(--color-paper);
  color: var(--color-ink);
}
button,
a,
input,
select {
  -webkit-tap-highlight-color: transparent;
}
button,
select {
  cursor: pointer;
}
:focus-visible {
  outline: 2px solid #526545;
  outline-offset: 5px;
}
::selection {
  background: #dce6ce;
}
.nav-link {
  color: #777b73;
  transition: color 0.2s;
}
.nav-link:hover {
  color: #272b27;
}
.cover {
  position: relative;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem;
}
.cover-next {
  background: #252d29;
  color: #f5f4e9;
}
.cover-typescript {
  background: #e2eaf0;
  color: #356584;
}
.cover-tailwind {
  background: #dfebe6;
  color: #3d7a72;
}
.cover-database {
  background: #e5e4ed;
  color: #656080;
}
.cover-express {
  background: #e8e7dd;
  color: #696b45;
}
.cover-notes {
  background: #eee0d2;
  color: #936d50;
}
.cover-orbit {
  position: absolute;
  width: 310px;
  height: 310px;
  border: 1px solid currentColor;
  opacity: 0.09;
  border-radius: 50%;
  right: -100px;
  top: -130px;
}
.orbit-two {
  width: 400px;
  height: 400px;
  right: -145px;
  top: -175px;
}
.hero-grid {
  background-image: radial-gradient(#8c967c40 1px, transparent 1px);
  background-size: 17px 17px;
}
@media (prefers-reduced-motion: reduce) {
  html {
    scroll-behavior: auto;
  }
  * {
    transition-duration: 0s !important;
  }
}
```
{% endraw %}

<a id="source-11"></a>

## 11. 共享类型

文件：`src/types/post.ts`

Post、Section 与 Category 定义文章字段、可选代码块和允许的分类，供数据与组件共同使用。

{% raw %}
```typescript
// 联合类型：分类只能取下面三个值，拼错时编译器会提醒。
export type Category = "前端开发" | "后端探索" | "成长手记";

export interface Section {
  heading: string;
  paragraphs: string[];
  code?: string; // ? 表示这个字段可以省略。
}

export interface Post {
  slug: string;
  title: string;
  excerpt: string;
  category: Category;
  date: string;
  minutes: number;
  tags: string[];
  cover: "next" | "typescript" | "tailwind" | "database" | "express" | "notes";
  sections: Section[];
}
```
{% endraw %}

<a id="source-12"></a>

## 12. 完整模拟数据

文件：`src/lib/posts.ts`

提供六篇文章及 getPost、formatDate。页面源码依赖这些数据与函数，复制时不要遗漏。

{% raw %}
```typescript
import type { Post } from "@/types/post";

// 当前阶段的唯一数据源。未来可用 API 返回的数据替换。
export const posts: Post[] = [
  {
    slug: "nextjs-first-page",
    title: "从一张白纸开始，搭建我的第一个 Next.js 博客",
    excerpt: "理解路由、组件和渲染，把零散的知识串成一个真正能运行的小项目。",
    category: "前端开发",
    date: "2026-11-17",
    minutes: 8,
    tags: ["Next.js", "React", "入门"],
    cover: "next",
    sections: [
      {
        heading: "为什么从博客开始？",
        paragraphs: [
          "博客虽然小，却包含列表、详情、导航和数据展示这些常见需求。我们先用静态数据把页面搭起来，再逐步接入 API 和数据库。每一步都有可以看见的结果。",
          "React 负责把数据变成界面；Next.js 在 React 之上提供路由、构建和渲染能力。本项目使用 App Router，页面放在 src/app 目录下。",
        ],
      },
      {
        heading: "一个文件，就是一个页面",
        paragraphs: [
          "src/app/page.tsx 对应首页。组件是返回 JSX 的函数，JSX 看起来像 HTML，但允许你用花括号嵌入 JavaScript 表达式。",
        ],
        code: "export default function Home() {\n  return <h1>你好，我的博客</h1>;\n}",
      },
      {
        heading: "让文章拥有自己的地址",
        paragraphs: [
          "posts/[slug]/page.tsx 中的方括号表示动态路由。slug 是文章的可读标识，例如 nextjs-first-page。因为使用静态导出，我们通过 generateStaticParams 告诉 Next.js 需要提前生成哪些文章。",
          "本项目的文章正文在构建时生成 HTML。搜索和分类切换则由浏览器里的客户端组件完成。这两种方式可以在同一个页面中组合。",
        ],
        code: "export function generateStaticParams() {\n  return posts.map((post) => ({ slug: post.slug }));\n}",
      },
      {
        heading: "把第一步走扎实",
        paragraphs: [
          "试着修改文章标题，然后新增一条数据。你会发现首页列表和文章详情都跟着变化了：数据与展示分离，就是组件化带来的第一个好处。",
          "下一步阅读项目 docs 目录下的教学文档，跟着实际文件逐段理解，再尝试自己添加一个页面。",
        ],
      },
    ],
  },
  {
    slug: "typescript-data-model",
    title: "给数据一份说明书：TypeScript 入门",
    excerpt: "从一篇文章的数据结构出发，理解类型、接口与可选属性。",
    category: "前端开发",
    date: "2026-11-16",
    minutes: 6,
    tags: ["TypeScript", "类型系统"],
    cover: "typescript",
    sections: [
      {
        heading: "类型是开发时的约定",
        paragraphs: [
          "JavaScript 允许变量持有不同类型的值。TypeScript 为它增加静态类型检查，让你在运行之前发现常见错误，例如把字符串传给需要数字的地方。类型会在编译时被移除，不会替你校验网络返回的数据。",
        ],
        code: 'const minutes: number = 6;\nconst tags: string[] = ["TypeScript", "入门"];',
      },
      {
        heading: "用接口描述文章",
        paragraphs: [
          "Post 接口描述所有文章必须提供的字段。组件拿到 Post 后，编辑器就知道有哪些属性，也能提示拼写错误。Section 中的 code 是可选属性，因为不是每一节都有示例代码。",
        ],
        code: "interface Section {\n  heading: string;\n  paragraphs: string[];\n  code?: string;\n}",
      },
      {
        heading: "缩小可能性",
        paragraphs: [
          "Category 使用字符串联合类型限制分类。如果把前端开发写成前端开法，TypeScript 会直接报错。联合类型尤其适合分类、状态和固定选项。",
          "练习：给 Post 添加一个可选的 description 字段，并在卡片里处理它不存在的情况。",
        ],
      },
    ],
  },
  {
    slug: "tailwind-layout",
    title: "少写一点 CSS，多理解一点布局",
    excerpt: "用 Tailwind CSS 搭一张文章卡片，让响应式布局变得直观。",
    category: "前端开发",
    date: "2026-11-15",
    minutes: 5,
    tags: ["Tailwind CSS", "CSS"],
    cover: "tailwind",
    sections: [
      {
        heading: "从一个工具类开始",
        paragraphs: [
          "Tailwind 提供小而专一的工具类。p-6 控制内边距，rounded-2xl 控制圆角，flex 启用弹性布局。它没有替代 CSS 的原理，而是提供一种组合 CSS 的方式。",
        ],
        code: '<article className="rounded-2xl border p-6">\n  <h2 className="text-xl font-bold">文章标题</h2>\n</article>',
      },
      {
        heading: "从手机开始设计",
        paragraphs: [
          "grid-cols-1 是基础布局；md:grid-cols-2 只在中等及以上屏幕应用。先让小屏幕可读，再在更宽的空间增加列数。",
          "本项目的文章网格在手机上显示一列，在中等屏幕显示两列，宽屏下与右侧栏共同构成阅读布局。",
        ],
      },
      {
        heading: "交互状态也是样式",
        paragraphs: [
          "hover: 表示鼠标悬停状态，focus-visible: 适合键盘焦点反馈。不要仅凭颜色表达选中状态，还应使用 aria-pressed 等语义属性。",
          "练习：调整卡片圆角、间距和标题字号，然后观察不同宽度下的变化。",
        ],
      },
    ],
  },
  {
    slug: "database-first-look",
    title: "文章最后存在哪里？认识 PostgreSQL",
    excerpt: "从一张文章表开始，建立对数据库和持久化的第一印象。",
    category: "后端探索",
    date: "2026-11-14",
    minutes: 7,
    tags: ["PostgreSQL", "TypeORM"],
    cover: "database",
    sections: [
      {
        heading: "从内存到持久化",
        paragraphs: [
          "当前文章写在 TypeScript 文件中，修改后需要重新构建。数据库可以让系统在运行时保存内容，服务重启后数据依然存在。",
          "PostgreSQL 是关系型数据库。可以把文章理解为一张表，每篇文章是一行，标题、正文和发布时间是列。",
        ],
      },
      {
        heading: "TypeORM 的位置",
        paragraphs: [
          "TypeORM 帮助后端把 TypeScript 实体与数据库表建立映射。Repository 提供读取和保存实体的接口。数据库操作仍然发生在服务端，连接密码不应放进浏览器代码。",
          "当前 Demo 没有连接数据库。正式接入时，先设计表结构与迁移，再实现 Express API。",
        ],
      },
      {
        heading: "一步一步接入",
        paragraphs: [
          "先实现只读文章列表和详情接口，验证错误和空状态，再实现发布功能。发布涉及身份认证、输入校验和权限控制，不宜直接开放写入接口。",
        ],
      },
    ],
  },
  {
    slug: "express-request",
    title: "一次请求的旅程：浏览器到 Express",
    excerpt: "拆开请求与响应，看看前端和后端是如何对话的。",
    category: "后端探索",
    date: "2026-11-13",
    minutes: 6,
    tags: ["Express", "HTTP"],
    cover: "express",
    sections: [
      {
        heading: "前后端如何对话",
        paragraphs: [
          "浏览器通过 HTTP 请求访问后端接口，后端返回 JSON。前端读取这些数据，再用组件展示。路由 /api/posts 通常用于文章列表，/api/posts/:slug 用于单篇文章。",
        ],
      },
      {
        heading: "最小只读接口",
        paragraphs: [
          "下面是后续阶段的示意代码，并未集成进当前静态 Demo。Express 接收请求后，将查询结果作为 JSON 返回。",
        ],
        code: 'app.get("/api/posts", (_req, res) => {\n  res.json({ data: posts });\n});',
      },
      {
        heading: "别忘了失败的情况",
        paragraphs: [
          "请求可能失败，文章也可能不存在。前端需要加载中、空列表和错误状态；后端需要明确的状态码与统一的错误响应。",
          "本项目已经实现搜索无结果提示和文章不存在页面，可以用这些界面作为下一阶段的基础。",
        ],
      },
    ],
  },
  {
    slug: "intern-first-week",
    title: "实习第一周：把问题拆小，把进步记下来",
    excerpt: "不必一次掌握所有技术。让每一天都有一个可以验证的小目标。",
    category: "成长手记",
    date: "2026-11-12",
    minutes: 4,
    tags: ["实习", "学习方法"],
    cover: "notes",
    sections: [
      {
        heading: "先跑起来，再问为什么",
        paragraphs: [
          "刚接触新项目时，先找到启动命令、入口文件和数据来源。成功修改一个标题，就已经走通从源代码到页面的完整过程。",
        ],
      },
      {
        heading: "让问题更具体",
        paragraphs: [
          "遇到问题时，记录你期望的行为、实际结果、报错信息和尝试过的方法。具体的问题更容易得到帮助，也更容易被自己解决。",
        ],
      },
      {
        heading: "给每一天一个小目标",
        paragraphs: [
          "第一天学会组件和 props；第二天理解类型；第三天完成响应式卡片；第四天实现筛选；第五天复盘整个数据流。",
          "能解释自己写下的代码，比一次堆出很多功能更重要。",
        ],
      },
    ],
  },
];

export function getPost(slug: string): Post | undefined {
  return posts.find((post) => post.slug === slug);
}

export function formatDate(date: string): string {
  return date.replaceAll("-", ".");
}
```
{% endraw %}

## 复制后的运行与检查

{% raw %}
```powershell
pnpm install
pnpm typecheck
pnpm build
pnpm preview
```
{% endraw %}

有原项目锁文件时，首次安装使用 `pnpm install --frozen-lockfile`。构建结果在 out 目录。先检查首页与全部文章详情，再测试搜索、分类叠加、排序和无结果重置。当前项目已使用 Webpack 构建；本次归档只新增文档，没有修改 Demo 运行代码。

源码中的文章和作者是示例内容，Express、PostgreSQL、TypeORM 尚未接入。后续扩展路线见第六篇，构建结果和已知静态预取限制见第七篇。

## 继续阅读

[← 上一篇：编译、验收与练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})

[回到第一篇：项目搭建与 React 入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})


[下一篇：走向全栈]({% post_url 2026-11-20-intern-blog-09-database %})
