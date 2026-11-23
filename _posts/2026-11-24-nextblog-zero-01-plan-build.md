---
layout: post
title: 从零搭建（一）：先想清业务，再按可验证的小步骤落地
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 从零搭建（一）：先想清业务，再按可验证的小步骤落地：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 先画出“谁对什么做什么”

不要从安装六个框架开始。先把需求写成可验证的行为：访客可以搜索和阅读文章；注册读者可以登录、退出、评论、删除自己的评论；管理员还可以创建、修改和删除文章。文章正文保存 Markdown 字符串，读页面负责渲染。注册前必须证明能收到指定邮箱的邮件。

这会导出三种核心数据：用户 User、文章 Article、评论 Comment；再导出两种支持数据：登录会话 sessions、临时注册验证码 registration_codes。评论依赖一个用户和一篇文章。角色只能由后台授予，注册表单不能让用户自己选 admin。

用一个贯穿例子检查设计：管理员写《学习 TypeORM》，读者搜索 TypeORM，打开文章，登录后留言。每一步都对应一页界面、一组状态或一个 HTTP 请求。先把这条主线做通，再加主题动画和视觉细节。

## 2. 技术如何分工

| 技术 | 本项目中实际负责 | 不要误认为 |
| --- | --- | --- |
| React | 组件、状态、事件、条件渲染 | 数据库访问框架 |
| Next.js | app 目录路由、布局、打包、同源 API 转发 | 已自动生成 Express 后端 |
| TypeScript | 开发时检查值的结构和调用方式 | 运行时自动过滤恶意 JSON |
| Tailwind + DaisyUI | 原子样式、按钮卡片表单、主题变量 | 已经实现登录业务的 React 组件 |
| Express | 路由、中间件、校验、返回 HTTP 响应 | 自动处理所有权限的框架 |
| TypeORM + pg | 对象映射、业务连接池、查询、事务、迁移 | 无需理解 SQL 的替代品 |
| PostgreSQL | 持久化、唯一约束、外键、行锁 | 浏览器里的临时状态 |

浏览器只请求 Next 所在域的 `/api/...`；Next 的 rewrites 将请求转给 127.0.0.1:4000 的 Express；Express 再访问数据库或 SMTP。Next 和 Express 是两个进程，但当前仓库只有一套根 package.json，这是 server 没有独立 node_modules 的原因。

## 3. 在新电脑运行现有版本：准确复现路线

在 Windows 安装满足根 package.json 中 Next 版本要求的 Node.js、pnpm、PostgreSQL，以及 Git。先执行 `node -v`、`pnpm -v`、`git --version`、`psql --version`，确认命令可用。数据库命令不在 PATH 时，可以使用 PostgreSQL 安装目录下的 psql.exe。

{% raw %}
```powershell
git clone https://github.com/DKomorebi/NextBlog.git
Set-Location NextBlog
pnpm install --frozen-lockfile
Copy-Item .env.example .env
```
{% endraw %}

`--frozen-lockfile` 用仓库的锁文件复现依赖，不在学习途中随意升级主版本。不要把仓库克隆到现有项目目录中覆盖旧文件。

连接 PostgreSQL 的管理员数据库：

{% raw %}
```powershell
psql -h localhost -p 5432 -U postgres -d postgres
```
{% endraw %}

在 psql 中执行（全新学习环境示例，不是让你重新创建已有数据库）：

{% raw %}
```sql
CREATE ROLE nextblog_app LOGIN;
\password nextblog_app
CREATE DATABASE fieldnotes OWNER nextblog_app;
\q
```
{% endraw %}

`\password` 交互输入密码，避免把真实密码写进教程或 shell 历史。应用账号不必拥有超级用户权限。测试会创建临时数据库，需要另备有 CREATEDB 权限的测试连接；不要为了跑测试给生产账号永久授予超级用户权限。

打开 `.env`：设置 DATABASE_URL、32 字符以上随机 SESSION_SECRET、APP_ORIGIN=http://localhost:3000、API_PORT=4000、API_ORIGIN=http://127.0.0.1:4000，以及 SMTP 五个配置项。数据库密码含 `@`、`:`、`/` 等字符时，需要按 URL 组件编码，不能随意直接拼接。可以用以下命令产生 Session 密钥，再只粘贴到本地 `.env`：

{% raw %}
```powershell
node -e "console.log(require('node:crypto').randomBytes(32).toString('hex'))"
pnpm db:migrate
pnpm db:seed
pnpm dev
```
{% endraw %}

在浏览器打开首页。Network 中 `/api/articles` 应返回 items、total；接口故障先访问 `/api/health` 判断数据库是否可用。注册自己可收信的邮箱，验证后再执行 `pnpm admin 你的邮箱`，刷新页面出现管理入口。不要把示例占位 SMTP 授权码当成真实配置。

## 4. 如果从空目录亲手重写，按什么顺序

这是学习重建路线，实际完整文件在本系列源码卷。每一阶段写出最小可验证结果，不必第一天就实现所有业务。

| 阶段 | 写哪些文件 | 为什么在这里写 | 如何确认走通 |
| --- | --- | --- | --- |
| 1 | package.json、tsconfig.json、postcss.config.mjs | 建立编译与样式入口 | tsc 可执行，Next 能渲染最小页面 |
| 2 | src/app/layout.tsx、page.tsx、globals.css | 所有页面共享骨架 | 首页显示标题和主内容 |
| 3 | ArticleList 的固定数组版本 | 先理解 JSX、map、props | 两张卡片可点击，无数据库依赖 |
| 4 | server/db.ts、entities.ts、migration.ts、manage.ts | 明确表结构与连接生命周期 | migrate 创建表，seed 导入示例 |
| 5 | server/index.ts、app.ts 的 health 与文章 GET | 完成第一次端到端数据链 | 浏览器显示数据库文章 |
| 6 | api<T>、列表 Effect、搜索分页 | 替换固定数组，理解请求状态 | 搜索改变请求参数与卡片 |
| 7 | 动态阅读页、Markdown、Comments 的 GET | 打通 slug 与 id 的关系 | 标题、正文、评论分别出现 |
| 8 | Session 中间件、auth/session、登录退出 | 建立可靠身份来源 | 刷新后保持登录，退出后失效 |
| 9 | 注册验证码表、mail、registration、AuthForm | 完整注册必须跨邮件与事务 | 验证前不创建用户，成功后自动登录 |
| 10 | requireAdmin、编辑器、文章写接口 | 在开放写操作前先保护权限 | 普通用户直接调接口仍得到 403 |
| 11 | 评论写入删除、版本冲突、错误处理 | 处理边界与多人使用 | 他人评论不能删，旧版本不能覆盖新版本 |
| 12 | DaisyUI、Hero、ThemeToggle、测试 | 在业务稳定后整理视觉与回归 | 深浅色、手机宽度、构建均通过 |

空目录练习可以使用 `pnpm create next-app@16 nextblog-lab --ts --tailwind --app --src-dir` 生成基础工程，再将依赖与配置调整到本仓库的锁定版本。这个命令只建立前端骨架，不会自动创建 server、数据库或本文的认证代码。它的交互选项会随工具版本变化，最终以实际生成文件和本教程源码卷逐项对照。

## 5. 每写一个方法先回答五个问题

例如要写“删除文章”：谁调用它（管理员点击按钮）？传入什么（文章 id）？先检查什么（确认框、Session、后端管理员权限）？改变什么（删除文章和外键级联评论）？怎样反馈（204 后刷新列表，404/403 显示错误）？

把这五个答案写出，再写 `ArticleList.remove(a)` 与后端 DELETE 处理函数，就不会只会照抄语法。后文每个页面都按事件、状态、请求和结果展开。

练习：不用看代码，画出“注册按钮按下后”经过的层。答案至少包括浏览器原生校验、submit、api、Next rewrite、Express 的 JSON/Session/CSRF/Zod、registration 事务、loginSession、前端 setSession/router.push。缺少其中一层，就先去该章补上。

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-02-typescript-react %})
