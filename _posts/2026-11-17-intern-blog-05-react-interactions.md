---
layout: post
title: Next.js博客（五）：用 React 实现搜索、分类与排序
date: 2026-11-17 09:04:00 +0800
categories:
  - 实习项目
category_path: 开发/实习项目/博客系统
description: 逐步跟踪受控输入、useState、filter、sort 与 map，完成可组合筛选和可恢复的空状态。
keywords: React, useState, 搜索, 分类筛选, 排序, 客户端组件
cover:
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

静态站也可以有交互：浏览器已经拿到文章数组，用户搜索时不必请求服务器。这一篇跟踪一次输入如何变成页面更新。

> 本文是「实习博客实战」系列第 5 / 8 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [实习博客实战（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

## 把一次搜索从头跟到尾

阅读 `src/components/post-browser.tsx`，按下列顺序跟踪：

1. `useState("")` 保存搜索文本 query，初始为空。
2. input 的 `value={query}` 让 React 状态成为输入内容的来源，这叫受控输入。
3. 用户键入内容，onChange 用 `event.target.value` 读取新文本。
4. `setQuery(...)` 请求 React 更新状态并重新渲染组件；不是立刻修改当前函数中的变量。
5. 组件重新执行 filter，生成新的 visible。
6. map 将 visible 变成卡片，React 更新页面中变化的部分。

实际筛选逻辑：

{% raw %}
```tsx
const matchesCategory = category === "全部文章" || post.category === category;
const matchesQuery = `${post.title} ${post.excerpt} ${post.tags.join(" ")}`
  .toLowerCase()
  .includes(query.trim().toLowerCase());
return matchesCategory && matchesQuery;
```
{% endraw %}

`||` 表示任一条件成立；`&&` 表示两个条件都成立。分类和搜索必须同时匹配。join 将标签数组拼成可搜索文本，trim 去掉搜索首尾空格，toLowerCase 统一大小写。

filter 返回新数组，因此后面的 sort 不会修改原始 posts。若直接对 props 中的 posts 调用 sort，就会原地改变它。本项目日期统一使用补零的 `YYYY-MM-DD` 格式，所以字符串比较也能按日期排序。

visible 是派生结果，不需要再存进一个 useState；否则每次更新 query 和 category 都要同步它，容易出现状态不一致。只有六条数据，也没有必要先引入 useMemo 或搜索库。真实大量数据则应在服务端分页和搜索。

## 为什么只保存三个状态

query 保存搜索文本，category 保存当前分类，sort 保存排序方式。visible 是由这三个状态和 posts 计算出的结果，因此不用再保存为第四个状态。每次渲染重新计算六条文章的筛选结果，逻辑清楚且成本很低。

## 交互组件完整源码

### src/components/post-browser.tsx

先阅读顶部状态与筛选逻辑，再阅读输入框、分类按钮、排序框，最后阅读文章列表和无结果分支。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

## 检查组合条件

先选择「后端探索」，应看到两篇文章；再搜索 TypeScript，应看到无结果提示。点击重置后恢复全部文章。搜索首尾空格会被 trim 去掉，英文大小写会被统一处理。

aria-pressed 表达分类按钮的选中状态，aria-live 提示筛选后的数量，sr-only 让文字对视觉隐藏但仍提供给辅助技术。它们不能代替完整的无障碍测试，但能让组件语义更明确。

## 从本地筛选走向服务端筛选

本地搜索只处理已经下载的数据；文章数量很大时，不应该先下载全部正文再筛选。后续 API 可以支持 q、category、page、pageSize，并补充加载、错误、重试与过期请求处理。此时 TypeScript 类型仍然需要配合运行时 JSON 校验。

## 继续阅读

[← 上一篇：实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})

[下一篇：实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线 →]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
