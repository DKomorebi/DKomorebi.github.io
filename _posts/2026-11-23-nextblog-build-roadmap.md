---
layout: post
title: NextBlog 从零搭建：开始思路、实施顺序与每步验收
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: NextBlog 从零搭建：开始思路、实施顺序与每步验收：结合实际文件、步骤和练习解析。
keywords: NextBlog, 项目搭建
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 开始时先确定要完成的业务

不要先把所有技术各看一遍，再期待它们自动拼成网站。先确定一个最小结果：游客能看到数据库里的一篇 Markdown 文章，管理员能保存修改，刷新后仍然存在。它把页面、接口、数据库串成闭环，后面的搜索、评论、主题都可以在这个基础上增量加入。

当前项目还包含注册登录、Session、CSRF、权限、评论、搜索分类分页、主题和示例导入。本篇回答“如果自己重新动手，应该先想什么、按什么顺序做”。它不要求清空你已经能运行的项目；重建练习请另建目录。

## 先画四层关系，再列出数据

浏览器中的 React 组件 → Next.js 页面与同源 /api 代理 → Express 业务接口 → TypeORM → PostgreSQL。

React 负责交互，Next.js 组织页面和构建；Express 决定谁能做什么，TypeORM 执行对象与表的访问。前端不直接连接 PostgreSQL，也不保存数据库密码。先定四张业务表：users、articles、comments、sessions；再定文章的 id、slug、title、excerpt、content、category 和 version 等字段。

把需求写成权限表：游客阅读，登录用户评论，管理员管理文章；普通用户只能删除自己的评论。这个决定应在写按钮之前做，因为它影响接口、会话与测试。

## 两种开始方式：熟悉现有项目与重新搭建

如果目的是先跑通，在新的练习目录克隆 NextBlog，安装锁定依赖，创建自己的空数据库，再按 .env.example 建立 .env。已有本项目配置不要覆盖。命令顺序为：


{% raw %}
````powershell
git clone https://github.com/DKomorebi/NextBlog.git NextBlog-learning
cd NextBlog-learning
pnpm install --frozen-lockfile
# 仅首次配置：创建空库后复制示例并填写自己的连接与随机密钥
Copy-Item .env.example .env
pnpm db:migrate
pnpm db:seed
pnpm dev
````
{% endraw %}


如果目的是从空目录逐步重建，先建一个单独的 NextBlog-learning 目录。为了避免依赖版本与教程混用，可从同一源码提交复制 package.json、pnpm-lock.yaml、pnpm-workspace.yaml、.npmrc、tsconfig.json、next-env.d.ts、postcss.config.mjs 和 next.config.ts，再执行 pnpm install --frozen-lockfile。这些是复用工程配置，不是已经复制完整业务。

此时 package.json 里的命令已存在，但入口文件尚未补齐；不要立即运行 pnpm dev 并把“找不到 server/index.ts”误认为依赖错误。下面每个阶段只运行已经具备入口的部分。参考快照见文末仓库链接。

## 第一步：只让一个页面出现

建立 src/app/layout.tsx 和 src/app/page.tsx，布局先只包含 html、body、children，首页先返回一个 h1。先不要 import 尚未写好的 SessionProvider 和 Header。运行 pnpm dev:web，确认 localhost:3000 显示标题。

通过标准：页面路径可访问，终端没有编译错误。此时可以再把标题提取成一个 React 组件，用 props 传标题，观察组件与页面路由是不同的概念。完成后提交一次小的 Git 记录。

## 第二步：独立跑通 Express

建立 server/app.ts 的 createApp 和 server/index.ts。学习阶段先写一个不查数据库的 /api/health 返回 JSON，监听 127.0.0.1:4000；随后再演进到当前项目中查询 SELECT 1 的健康检查。不要把暂时省略数据库的练习接口当成最终实现。

运行 pnpm dev:api，直接打开 4000 端口的 health；再通过 next.config.ts 的 rewrite 打开 localhost:3000/api/health。前者失败检查 Express，前者成功而后者失败检查代理。两条都成功以后再加页面请求。

## 第三步：建库、连接与迁移

在 PostgreSQL 中准备一个新的空库。配置 DATABASE_URL、SESSION_SECRET、APP_ORIGIN、API_PORT 和 API_ORIGIN；密钥使用自己随机生成的值，真实 .env 不提交。字段格式参考已有 .env.example，不复制他人的账号密码。

按 server/entities.ts 建立实体，按 server/db.ts 创建 DataSource，按 server/migration.ts 创建表及约束，按 server/manage.ts 接入 migrate 命令。执行 pnpm db:migrate 后再让 index 初始化数据库。不要开启 synchronize 来掩盖迁移缺失。

通过标准：健康检查的 SELECT 1 成功，业务表和 sessions 表存在；修改配置指向错误库时能得到明确失败，而不是假装启动成功。seed 是可选的内容导入，与建表不是同一步。

## 第四步：只读文章闭环

先实现 GET /api/articles 和 GET /api/articles/:slug，用 Repository 读取数据。通过 seed 插入示例，检查 JSON 字段。再在 full-blog.tsx 的 api、ArticleList 和 ArticleReader 中接入请求，并用 posts/[slug]/page.tsx 接收 slug。

这一步先解决加载、空结果、错误和重试。阅读页面使用 Markdown 组件显示正文，禁用任意原始 HTML。不要一开始就把登录、编辑、主题一起做，否则请求失败时很难分辨是哪一层的问题。

## 第五步：会话、注册与登录

加入 express-session 与 connect-pg-simple，让 Cookie 关联 PostgreSQL sessions。先写 GET /api/auth/session，再写注册、登录和退出。密码哈希保存，注册角色固定 user；登录时 regenerate 会话并保存，再响应。

同时接入 Origin 和 CSRF 校验，前端 SessionProvider 保存公开会话快照。通过标准：注册能登录，刷新与正常重启后状态可读取，退出后旧 Cookie 不再恢复身份。不是仅仅把导航文字从“登录”改成昵称。

## 第六步：先有授权，再开放文章写入

实现 requireUser、requireAdmin，并提供服务器端 pnpm admin 邮箱授权命令。随后实现新增、修改、删除文章。字段用 Zod 校验，修改时比较 version，删除使用数据库外键处理评论。

按 src/app/admin 下的页面入口组织 Editor 和管理列表。游客与普通用户不能绕过前端直接写入；保存失败保留正文；重复 slug 返回冲突。通过标准应同时包括“管理员成功”和“普通用户被拒绝”。

## 第七步：评论、搜索与分页

评论作者来自 Session，不接收客户端任意 authorId。删除检查作者或管理员。文章列表的关键词、分类和页码交给后端，响应包含 items 与 total；前端输入变化重置页码并防止旧请求覆盖新结果。

先做正常路径，再补空列表、末页删除、非法页码、重复提交等情况。功能能演示不代表边界已经可靠。

## 第八步：最后完善布局与主题

再引入 globals.css 中的 Tailwind/DaisyUI 主题、文章卡片、HomeHero 和 Lucide 图标，最后完成 ThemeToggle。主题偏好放 localStorage；登录身份仍用 Session。手机端、键盘切换、减少动画和刷新保持偏好都应实际查看。

这一步后置，是为了让样式问题与业务问题能分开定位。布局也可以早期用基础控件搭好，但无需在数据库还没连通时反复雕琢动画。

## 第九步：把验收固定下来

执行 pnpm typecheck、pnpm test:api 和 pnpm build。集成测试需要创建独立数据库的权限；不要为测试清空真实业务库。浏览器再验证从注册到发布、评论、修改、删除的完整流程。

发布前确认 .env 被忽略，提交锁文件与占位配置。当前生产启动使用 pnpm start，需要 HTTPS、正确的 APP_ORIGIN 和代理配置；Next.js 构建成功不代表 Express 已编译成单独 dist，也不能直接把旧静态目录拿去部署。

## 每一步遇到问题先看哪里

| 问题 | 优先文件 | 暂时不要做 |
| --- | --- | --- |
| 页面无法编译 | page.tsx、layout.tsx、终端错误 | 重置数据库 |
| health 不通 | index.ts、db.ts、环境变量 | 重写 React |
| GET 能用而 POST 403 | Origin、CSRF、requireAdmin | 删除权限中间件 |
| 保存 409 | slug 唯一约束、version | 强行覆盖版本 |
| 页面有数据但显示错 | API 字段、组件类型、Markdown | 更换数据库 |

## 建议的学习交付记录

每完成一步记录四件事：改了哪些文件、输入和输出是什么、如何证明成功、失败时出现什么信息。学习笔记不必抄满所有源码，先能解释一篇文章从 URL 到 SQL 再回到页面的路径。前面的十五篇教程和十二篇专题报告可按这里的阶段查阅，而不是必须先全部背完再开始动手。

项目源码参考：[NextBlog 77ffe17](https://github.com/DKomorebi/NextBlog/tree/77ffe17131b9a9524498e6e6bee536516bad82b4)。
