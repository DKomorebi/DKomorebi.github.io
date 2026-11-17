---
layout: post
title: "实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件"
date: 2026-11-17 09:03:00 +0800
categories: ["实习项目"]
category_path: "开发/实习项目/博客系统"
description: "拆解文章卡片的工具类、响应式断点、主题变量和六种纯 CSS 封面，附样式与组件完整源码。"
keywords: "Tailwind CSS, CSS, 响应式, Grid, 主题变量"
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

这一篇不背工具类列表，而是从界面效果回到 CSS 原理：空间怎么分配，卡片为什么能自适应，颜色又如何保持统一。

> 本文是「实习博客实战」系列第 4 / 8 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [实习博客实战（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

## Tailwind CSS：用组合表达样式

### 本项目使用 v4 配置

`postcss.config.mjs` 注册 `@tailwindcss/postcss`，`src/app/globals.css` 首行是：

{% raw %}
```css
@import "tailwindcss";
```
{% endraw %}

Tailwind 在构建时扫描源代码中的类名，生成对应 CSS。因此它不是运行时逐条翻译 className。v4 的配置方式与很多旧版教程不同，不要混用 v3 的安装命令和配置。[官方 Next.js 安装指南](https://tailwindcss.com/docs/installation/framework-guides/nextjs)

### 拆开一张卡片

`post-browser.tsx` 中卡片的类名可以按作用阅读：

{% raw %}
```tsx
className="group overflow-hidden rounded-2xl border border-line
           bg-white/60 transition-colors hover:border-accent/50"
```
{% endraw %}

| 工具类 | 对应的视觉作用 |
| --- | --- |
| `overflow-hidden` | 裁切超出卡片的封面，让圆角生效 |
| `rounded-2xl` | 较大的边框圆角 |
| `border border-line` | 细边框，使用项目主题颜色 |
| `bg-white/60` | 60% 不透明度的白色背景 |
| `transition-colors` | 颜色变化使用过渡 |
| `hover:border-accent/50` | 悬停时改变边框颜色 |
| `group` | 让子元素可以响应父级的悬停状态 |
| `group-hover:text-accent` | 父卡片悬停时，子标题改变颜色 |

这些类可以组合；仍然需要理解 box model、flex、grid、overflow 等 CSS 概念。Tailwind 让你少起很多类名，但不会替你决定合理布局。

### 响应式从小屏幕开始

文章列表：

{% raw %}
```tsx
className="grid grid-cols-1 gap-x-6 gap-y-8 md:grid-cols-2"
```
{% endraw %}

基础是一列；到默认 md 断点（48rem）及以上变成两列。首页外层用了 `lg:grid-cols-[minmax(0,1fr)_260px]`：宽屏时左侧占剩余空间，右侧固定 260px；更窄时回到单列。`minmax(0,1fr)` 允许主内容缩小，避免长内容强行撑宽网格。

`hidden md:flex` 表示小屏隐藏装饰，中等及以上屏幕显示 flex。注意 md 不是“只在平板”，而是“达到该宽度以及更宽”。

### 间距、字号与任意值

默认间距基于 0.25rem：`p-6` 通常是 1.5rem 内边距，`mt-5` 是 1.25rem 上边距。`text-sm` 同时设定字号和行高；正文使用 `leading-8` 增加阅读间距。

方括号允许任意值，例如 `text-[11px]`、`min-h-[65vh]`。应优先使用统一的设计尺度，再在确实需要时使用任意值，避免每个元素都有不同的数值。

### 主题变量与自定义 CSS

在 `globals.css` 中：

{% raw %}
```css
@theme {
  --color-paper: #f8f7f3;
  --color-ink: #272b27;
  --color-accent: #526545;
}
```
{% endraw %}

Tailwind 据此提供 `bg-paper`、`text-ink`、`border-accent` 等类。修改一个变量即可调整整个网站的对应颜色。

封面的轨道装饰、网格纹理属于重复而具体的视觉样式，所以放进 `.cover`、`.cover-orbit` 和 `.hero-grid` 等普通 CSS 类。Tailwind 可以与普通 CSS 共存，不需要强迫一切都写成工具类。

### 不要拼接无法扫描的类名

不建议写 `bg-${color}-500`。构建器不执行你的 JavaScript 来枚举所有可能字符串，因此可能漏掉样式。使用完整类名映射，例如 `{ green: "bg-green-500", blue: "bg-blue-500" }`。

本项目 `cover-${kind}` 对应的样式是 globals.css 中明确写出的普通 CSS 规则；不是期待 Tailwind 自动生成这些动态类。分类按钮的三元表达式则包含两组完整的工具类字符串，可被扫描。

## 样式与封面源码

### src/app/globals.css

Tailwind 入口、项目主题、公共焦点样式以及封面装饰都在这里。普通 CSS 与工具类各自负责适合的部分。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/components/cover.tsx

kind 使用 Post["cover"] 保持类型一致；themes 把封面名称与图标集中管理；large 控制精选封面和列表封面的尺寸。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

## 本篇练习

把 --color-accent 改为另一种深色，观察标题、边框和按钮的变化。再把文章网格的 md:grid-cols-2 暂时移除，比较 375px、768px 和 1440px 下的布局。记得恢复样式后再构建。

## 继续阅读

[← 上一篇：实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})

[下一篇：实习博客实战（五）：用 React 实现搜索、分类与排序 →]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
