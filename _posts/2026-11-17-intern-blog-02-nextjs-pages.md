---
layout: post
title: Next.js博客（二）：Next.js 路由、静态导出与页面完整源码
date: 2026-11-17 09:01:00 +0800
categories:
  - 实习项目
category_path: 开发/实习项目/博客系统
description: 用首页、详情页、归档和关于页讲解 App Router、动态参数、服务端组件与静态路径生成。
keywords: Next.js, App Router, 静态导出, 动态路由, 页面源码
cover:
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

这一篇把目录、URL 和页面代码串起来。先读路由原理，再沿着首页进入一篇文章，最后查看每个页面的完整实现。

> 本文是「实习博客实战」系列第 2 / 14 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

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

## Next.js：目录如何变成网站

### App Router 的文件约定

| 文件 | 浏览器地址 / 作用 |
| --- | --- |
| `src/app/page.tsx` | `/` |
| `src/app/about/page.tsx` | `/about/` |
| `src/app/archive/page.tsx` | `/archive/` |
| `src/app/posts/[slug]/page.tsx` | `/posts/nextjs-first-page/` 等 |
| `src/app/layout.tsx` | 包裹所有页面，不是单独地址 |
| `src/app/not-found.tsx` | 找不到页面时的展示 |

目录不一定就是页面：通常需要其中存在 `page.tsx`。`components` 下的组件不会自动变成 URL。`page.tsx` 需要默认导出页面组件。

### layout 与 children

`src/app/layout.tsx` 的结构可概括为：

{% raw %}
```tsx
<html lang="zh-CN">
  <body>
    <Header />
    {children}
    <footer>...</footer>
  </body>
</html>
```
{% endraw %}

`children` 是当前路由对应的内容。打开首页时放入 Home，打开归档时放入 Archive。这样每个页面不必重复写导航和页脚。`lang` 告诉辅助技术页面语言；`metadata` 提供浏览器标题与描述，不是界面上的可见标题。

### Link 与普通 a

内部页面导航用 `next/link` 的 `Link`，例如 `<Link href="/archive/">归档</Link>`。Next.js 能据此优化内部导航。跳转同页目录锚点使用普通 `<a href="#section-0">` 就足够。

### 服务端组件与客户端组件

App Router 中组件默认是服务端组件。首页、详情页、归档不需要处理本地交互，所以没有 `"use client"`。

`src/components/post-browser.tsx` 顶部写了：

{% raw %}
```tsx
"use client";
import { useState } from "react";
```
{% endraw %}

它需要 `useState`、输入事件和点击事件，因此声明为客户端入口。这个边界下的代码会进入浏览器包。不要在其中导入数据库连接或服务器密钥。

**“服务端组件”不表示本项目上线后必须一直运行 Node.js 服务。** 当前启用了静态导出，这些组件在 build 时执行。客户端组件也可以参与初始 HTML 预渲染，随后浏览器加载 JavaScript 并接管事件，这个过程称为 hydration。并不是写了 `use client` 就意味着初始页面完全没有 HTML。[官方说明](https://nextjs.org/docs/app/getting-started/server-and-client-components)

为什么不把整个首页都标记为客户端组件？因为只有列表需要交互。把边界放在较小的组件上，更容易看清哪些逻辑属于浏览器。

### 动态路由：slug 是什么

打开 `src/app/posts/[slug]/page.tsx`：

{% raw %}
```tsx
type ArticleProps = { params: Promise<{ slug: string }> };

export default async function Article({ params }: ArticleProps) {
  const { slug } = await params;
  const post = getPost(slug);
  if (!post) notFound();
  // 后续返回文章 JSX
}
```
{% endraw %}

访问 `/posts/typescript-data-model/` 时，slug 对应 `typescript-data-model`。当前 Next.js 的 params 是异步参数，因此先 `await`。`Promise<T>` 表示将来会得到一个 T 类型的结果；包含 await 的函数要声明为 async。

`getPost` 使用 `find` 查询，结果可能是 `Post`，也可能是 `undefined`。不存在时调用 `notFound()` 结束当前页面渲染；检查之后 TypeScript 知道 post 已存在，后面的 `post.title` 才能安全访问。

### 静态导出为什么需要 generateStaticParams

`next.config.ts`：

{% raw %}
```ts
const nextConfig: NextConfig = {
  output: "export",
  trailingSlash: true,
};
```
{% endraw %}

详情路由：

{% raw %}
```tsx
export function generateStaticParams() {
  return posts.map((post) => ({ slug: post.slug }));
}
```
{% endraw %}

构建器必须知道所有文章地址，才能逐个生成 HTML。`trailingSlash` 让文章输出为类似 `out/posts/typescript-data-model/index.html`，适合按目录提供静态文件。新增文章后重新构建，就会生成新路径。

没有 Node.js 请求处理服务，就不能依赖请求时读数据库、Server Actions 或运行时动态生成未知文章。静态页面仍可以通过浏览器请求独立 API，但新增详情地址仍需重建，或者改变路由与部署方案。[静态导出文档](https://nextjs.org/docs/app/guides/static-exports)

## 阅读代码时先关注数据流

首页读取 posts，把数组交给 PostBrowser；详情页读取 URL 中的 slug，再由 getPost 找到文章。Cover 只负责封面呈现。数据类型和模拟数据见第三篇，封面样式见第四篇，交互列表见第五篇。这里的 `@/` 指向 Demo 的 src 目录。

## 页面源码

### src/app/page.tsx

首页包括介绍区、精选文章、文章列表和作者侧栏。它本身没有本地状态，交互由 PostBrowser 负责。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/app/posts/[slug]/page.tsx

generateStaticParams 枚举所有文章地址；await params 读取参数；notFound 处理查询不到的文章。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/app/archive/page.tsx

归档复用同一份 posts。当前年月标题是示例数据对应的固定值，扩展不同月份时需要改成分组计算。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/app/about/page.tsx

关于页展示作者说明与学习路线。learning 是页面内锚点，不是额外路由。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/app/not-found.tsx

不存在页面时显示友好提示。部署时还需要静态服务器返回正确的 HTTP 404 状态。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

## 本篇练习

新增 `src/app/notes/page.tsx`，默认导出一个返回 JSX 的组件，再在 Header 添加 `/notes/` 链接。解释为什么只创建 notes 文件夹而不添加 page.tsx 不会得到一个可访问页面。

## 继续阅读

[← 上一篇：实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})

[下一篇：实习博客实战（三）：TypeScript 类型设计与文章数据建模 →]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
