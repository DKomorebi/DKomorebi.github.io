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
title: React 与 Next.js（一）：它们各自解决什么问题
date: 2026-11-22 00:01:00 +0800
description: A · React 与 Next.js：结合项目源码、逐段讲解和练习答案深入学习。
---

> A · React 与 Next.js · 第 1 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 先回答你最关心的问题

React 是用组件描述界面及交互的库；Next.js 是使用 React 的应用框架。你不是在两种互斥的页面技术中二选一：项目里的 Next.js 页面本身就在使用 React。Express 是另一个进程中的 HTTP 业务服务，TypeScript 则是这些文件使用的语言。

先把四个层次分开：语言描述代码怎样写，UI 库描述界面怎样组织，应用框架组织路由和构建，业务服务处理请求和数据库。一个 .tsx 页面可以同时属于 TypeScript、React 和 Next.js，不能只给它贴一个标签。

## 从四行首页代码开始


代码位置：`src/app/page.tsx`（完整文件）。

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
````
{% endraw %}


import 从业务组件文件引入 ArticleList。默认导出的 Page 是函数组件；return 后的 JSX 表达要渲染哪个组件。这部分是 React 的写法。src/app/page.tsx 这个特定位置会被 Next.js 识别为首页入口，这部分是 Next.js 的约定。

假如将同一个函数放到普通 React 项目里，它仍然可以作为组件使用，但不会仅因为文件叫 page.tsx 就自动对应 /。反过来，保留 Next.js 路由却删除 ArticleList 的实现，也不会自动得到搜索和文章卡片。

| 项目中的具体能力 | 主要负责者 | 文件线索 |
| --- | --- | --- |
| 点击后改变搜索词，重新显示列表 | React state 与 effect | full-blog.tsx |
| /posts/xxx 找到对应页面 | Next.js 文件路由 | app/posts/[slug]/page.tsx |
| Header 与页面共用登录快照 | React Context | SessionProvider |
| 所有页面使用相同布局 | Next.js layout 约定与 React 组合 | app/layout.tsx |
| /api 转发到 4000 | Next.js rewrite | next.config.ts |
| 判断谁能删除文章 | Express 中间件和业务代码 | server/app.ts |

## JSX、组件与 props 怎样连接

JSX 是描述元素树的语法，不是后端 HTML 模板。大写名称通常引用组件，小写名称通常代表 HTML 元素。ArticleList 内部会返回 section、input 和 article 等元素；这些组件再嵌套起来形成页面。

归档页面传入 archive 属性，管理页面传入 manage 属性。属性就是父组件提供给子组件的数据。是否处于管理模式控制界面，但并不授予权限：服务器依然检查账号角色。这个例子同时说明组件复用和业务授权不能混为一谈。

## 框架为什么还需要自己的业务组件

Next.js 可以提供路由、构建和运行环境，却不知道你的博客需要怎样搜索、文章有哪些字段、哪些按钮只有管理员能看。full-blog.tsx 就填补了这些业务内容。当前集中在一个文件是教学版组织方式，并不是 React 或 Next.js 要求所有组件写在一起。

Next.js 本身也能编写服务端接口；本项目选择独立 Express，是为了满足技术栈训练和明确的业务服务边界。因此“用了 Next.js 就必须再用 Express”和“Next.js 只能做纯前端”都不准确。

## 跟着文件做一次阅读练习

先打开 src/app/page.tsx，进入 ArticleList 的定义，找到 q、category、page 三个状态，再回到 archive/page.tsx 看传入的属性。你应该能解释：网址由 Next.js 决定，列表内容和交互由 React 组件实现。

自测：把 ArticleList 改名为 ArticleBrowser，会自动改变网址吗？不会；路由位置没变。把页面目录 archive 改成 library，会怎样？对应文件路由会变化，但组件本身仍可以叫 ArticleList。这里只做阅读推演，不需要为了练习破坏现有链接。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-02-rendering %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [React：描述用户界面](https://react.dev/learn/describing-the-ui)
- [Next.js 服务端与客户端组件](https://nextjs.org/docs/app/getting-started/server-and-client-components)
