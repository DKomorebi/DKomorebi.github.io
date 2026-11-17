---
layout: post
title: "实习博客实战（一）：项目搭建与 React 组件入门"
date: 2026-11-17 09:00:00 +0800
categories: ["实习项目"]
category_path: "开发/实习项目/博客系统"
description: "从六种技术的分工入手，运行静态博客，理解 JSX、组件、props 与公共布局，并附完整配置源码。"
keywords: "实习, 博客, Next.js, React, JSX, props, 项目搭建"
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

这组文章记录一个从零开始的实习培训项目：先完成可运行的静态博客，再逐步接入后端。每篇先解释概念，再给出实际项目源码，适合边读边改。

> 本文是「实习博客实战」系列第 1 / 7 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})

## 本系列的范围

Demo 名为「拾页 Fieldnotes」，包含首页、六篇详情、搜索、分类、排序、归档和关于页。当前只运行 Next.js + React + TypeScript + Tailwind CSS；Express、PostgreSQL、TypeORM 尚未接入。文章数据是本地模拟数据，页面可以交互，但不能在线发布或保存文章。

本文的代码快照来自 2026 年 11 月 17 日的项目。Next.js 16.4.6、React 19.3.1、TypeScript 5.9.2、Tailwind CSS 4.3.5 为本次已验证版本。依赖范围以 package.json 为准，精确版本由原项目的 pnpm-lock.yaml 固定；复制文章重新安装而未使用锁文件时，版本可能变化。

## 先理解各自的工作

| 技术 | 在项目中的职责 | 类比 |
| --- | --- | --- |
| HTML / JSX | 表达标题、段落、链接等内容结构 | 房子的骨架 |
| CSS / Tailwind CSS | 布局、颜色、间距、响应式 | 房间的摆放与装修 |
| JavaScript | 筛选、排序、点击事件等逻辑 | 电路和开关 |
| TypeScript | 在开发和编译阶段检查数据的形状 | 施工图中的规格 |
| React | 用组件描述 UI，状态变化后更新界面 | 可重复使用的房间模块 |
| Next.js | 基于 React 提供路由、构建、渲染和资源组织 | 把模块组织成完整建筑 |
| Express（下一阶段） | 接收 HTTP 请求、返回数据 | 服务窗口 |
| PostgreSQL（下一阶段） | 持久化存储文章等数据 | 档案库 |
| TypeORM（下一阶段） | 在后端将实体与表映射，帮助执行查询 | 数据访问工具 |

TypeScript 不是另一个 UI 框架；Tailwind 不是组件库；Next.js 也不是 Express 的别名。分清职责，才知道遇到问题应该检查哪里。

## 如何启动，命令分别做什么

先阅读根目录 `package.json` 的 scripts：

{% raw %}
```json
{
  "dev": "next dev --webpack",
  "build": "next build --webpack",
  "typecheck": "tsc --noEmit",
  "preview": "serve out -l 3000"
}
```
{% endraw %}

- `pnpm install`：根据依赖声明下载工具和库。已有锁文件时使用 `--frozen-lockfile`，避免无意改变依赖版本。
- `pnpm dev`：开发服务器。保存代码后通常自动刷新，适合修改和调试。
- `pnpm typecheck`：运行 TypeScript 检查，不输出 JavaScript。通过不等于页面交互一定正确。
- `pnpm build`：检查并构建生产版本。本项目还会生成静态页面到 `out/`。
- `pnpm preview`：提供 `out/` 中的已构建文件。修改源代码后，要重新 build 才能在 preview 看到变化。

`node_modules` 是下载的依赖；`.next` 是 Next.js 生成的中间产物；`out` 是可部署的静态结果。不要直接编辑这些目录，应该编辑 `src`。Next.js 的最低 Node.js 要求可以查阅[安装文档](https://nextjs.org/docs/app/getting-started/installation)。

## 先补上 React / JSX 基础

### 组件就是返回界面的函数

打开 `src/components/header.tsx`，你会看到 `export function Header()`。函数返回 JSX：

{% raw %}
```tsx
// 简化示例
export function Header() {
  return <header><h1>拾页</h1></header>;
}
```
{% endraw %}

组件名首字母大写；`<Header />` 使用组件，小写的 `<header>` 是原生 HTML 元素。`export` 允许其他文件导入它。JSX 里使用 `className` 而非 HTML 的 `class`，因为这是 JavaScript 中的属性名。

`.ts` 文件存放普通 TypeScript；`.tsx` 文件允许写 JSX。它们都需要工具处理后，浏览器才能使用。

### 花括号把数据带进界面

首页 `src/app/page.tsx`：

{% raw %}
```tsx
const featured = posts[0];
// 以下 JSX 是首页中的片段
<h2>{featured.title}</h2>
```
{% endraw %}

`posts[0]` 取第一篇文章；`{featured.title}` 将字符串渲染出来。React 默认会转义文本，因此正文里出现 `<script>` 不会因为普通文本渲染而执行。不要为了显示文章随意改为 `dangerouslySetInnerHTML`；将来接入富文本需要另行设计清洗策略。

### props 是传给组件的输入

首页使用：

{% raw %}
```tsx
<PostBrowser posts={posts} />
```
{% endraw %}

子组件接收：

{% raw %}
```tsx
export function PostBrowser({ posts }: { posts: Post[] }) {
  // ...
}
```
{% endraw %}

左边的 `posts` 是属性名，花括号中的 `posts` 是父组件变量。`{ posts }` 是 JavaScript 解构：从参数对象里取出这个属性；后面的 `{ posts: Post[] }` 是 TypeScript 注解，要求它是文章数组。子组件应该读取 props，不应直接修改父组件传来的数据。

### 列表与条件渲染

`post-browser.tsx` 中，`visible.map(...)` 把文章数组映射为卡片数组。每个卡片有 `key={post.slug}`，React 用稳定且唯一的 key 识别条目。不要在会排序的列表中用数组下标作为 key。

详情页的 `section.code && <pre>...</pre>` 表示有代码时才展示代码块；文章列表使用三元表达式 `visible.length ? 列表 : 空状态`。条件渲染就是让数据决定界面。

## 在自己的电脑上运行

在 Demo 项目根目录运行下列命令，不要在存放这篇文章的 Jekyll 博客目录运行。首次安装建议使用原项目锁文件。

{% raw %}
```powershell
pnpm install --frozen-lockfile
pnpm dev
```
{% endraw %}

浏览器打开终端显示的本地地址，通常为 `http://localhost:3000`。如果手动逐文件重建、尚无锁文件，第一次执行 `pnpm install` 生成锁文件。本文采用 `--webpack`，因为当前 Windows 环境的 Turbopack 创建 PostCSS 子进程时失败，Webpack 已成功构建相同代码。

## 项目配置与公共布局源码

### package.json

scripts 声明启动命令；dependencies 是应用依赖，devDependencies 是构建和类型检查工具。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```json
{
  "name": "fieldnotes-blog",
  "version": "1.0.0",
  "private": true,
  "scripts": {
    "dev": "next dev --webpack",
    "build": "next build --webpack",
    "typecheck": "tsc --noEmit",
    "preview": "serve out -l 3000"
  },
  "dependencies": {
    "next": "^16.0.0",
    "react": "^19.2.0",
    "react-dom": "^19.2.0",
    "lucide-react": "^0.468.0"
  },
  "devDependencies": {
    "@tailwindcss/postcss": "^4.1.0",
    "tailwindcss": "^4.1.0",
    "postcss": "^8.5.0",
    "typescript": "^5.9.0",
    "@types/node": "^22.0.0",
    "@types/react": "^19.2.0",
    "@types/react-dom": "^19.2.0",
    "serve": "^14.2.4"
  }
}
```
{% endraw %}

### tsconfig.json

strict 启用严格类型检查；@/* 映射到 src/*；不要通过关闭类型检查来掩盖错误。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```json
{
  "compilerOptions": {
    "target": "ES2017",
    "lib": ["dom", "dom.iterable", "esnext"],
    "allowJs": true,
    "skipLibCheck": true,
    "strict": true,
    "noEmit": true,
    "esModuleInterop": true,
    "module": "esnext",
    "moduleResolution": "bundler",
    "resolveJsonModule": true,
    "isolatedModules": true,
    "jsx": "react-jsx",
    "incremental": true,
    "plugins": [{ "name": "next" }],
    "paths": { "@/*": ["./src/*"] }
  },
  "include": [
    "next-env.d.ts",
    "**/*.ts",
    "**/*.tsx",
    ".next/types/**/*.ts",
    ".next/dev/types/**/*.ts"
  ],
  "exclude": ["node_modules"]
}
```
{% endraw %}

### next.config.ts

output: export 开启静态导出，trailingSlash 生成适合目录访问的文件结构。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```typescript
import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  output: "export",
  trailingSlash: true,
};

export default nextConfig;
```
{% endraw %}

### postcss.config.mjs

Tailwind v4 通过专用 PostCSS 插件参与样式构建。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```javascript
export default { plugins: { "@tailwindcss/postcss": {} } };
```
{% endraw %}

### next-env.d.ts

Next.js 的环境类型声明；通常由框架维护。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```typescript
/// <reference types="next" />
/// <reference types="next/image-types/global" />
import "./.next/types/routes.d.ts";
import "./.next/types/root-params.d.ts";

// NOTE: This file should not be edited
// see https://nextjs.org/docs/app/api-reference/config/typescript for more information.
```
{% endraw %}

### .gitignore

生成文件和依赖不需要提交到版本控制。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```text
node_modules/
.next/
out/
*.tsbuildinfo
.env*
!.env.example
.tools/
```
{% endraw %}

### src/app/layout.tsx

layout 负责页面外壳，children 是当前路由的内容。globals.css 和 Header 分别在下文及第四篇展开。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

### src/components/header.tsx

Header 演示组件复用与 Link 导航；图标来自 lucide-react。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

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

## 本篇自测

尝试只改 Header 中的站点名称，然后访问首页、归档和关于页。解释为什么只修改一个文件，三个页面的导航都会更新。

## 继续阅读

[下一篇：实习博客实战（二）：Next.js 路由、静态导出与页面完整源码 →]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
