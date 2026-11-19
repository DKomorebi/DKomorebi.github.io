---
layout: post
title: NextBlog（一）：全栈项目搭建与 React 入门
date: 2026-11-17 09:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 从环境配置、数据库初始化到 React 组件，运行当前完整博客。
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

从环境配置、数据库初始化到 React 组件，运行当前完整博客。

> 本文是 NextBlog 全栈博客系列第 1 / 15 篇，内容对应当前全栈版本。

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

## 本系列围绕哪个项目学习

NextBlog 的网站名是「拾页」。它使用 Next.js 展示页面，Express 提供接口，PostgreSQL 保存文章、用户、评论与会话。管理员可以在线发布和修改 Markdown 文章；普通用户可以注册、登录和评论。首页还有文章卡片、搜索分类、分页、Hero 渐显与深浅主题。

本系列直接讲解这套正在运行的全栈代码。第一至八篇按学习顺序建立基础，第九至十四篇深入数据库、认证、页面、组件、主题与测试，第十五篇汇总项目结构和完整源码。文中的路径均相对于 NextBlog 根目录；复制代码时请参考同一版本的配置与锁文件。

| 技术 | 在本项目中做什么 | 先看哪里 |
| --- | --- | --- |
| React | 将文章列表、表单和评论组织成组件 | src/components/full-blog.tsx |
| Next.js | 文件路由、公共布局、构建与接口代理 | src/app、next.config.ts |
| TypeScript | 检查变量、参数、返回值与组件属性 | .ts 与 .tsx 文件 |
| Tailwind CSS / DaisyUI | 工具类、基础控件、主题 | src/app/globals.css |
| Express | 接收请求，检查身份并处理业务 | server/app.ts |
| TypeORM / PostgreSQL | 将对象映射到数据库并持久化 | server/entities.ts、server/db.ts |

## 第一步：安装项目依赖

本项目使用 Node.js 运行 Next.js 和 Express，使用 pnpm 安装依赖。请准备与仓库 Next.js 版本兼容的 Node.js 环境，以及 PostgreSQL。本机开发验证使用 PostgreSQL 18。先确认终端能执行 node、pnpm；数据库可以用已有本地实例，也可以按 compose.yaml 启动一个实例，不要让两个实例争用 5432。


{% raw %}
```powershell
git clone https://github.com/DKomorebi/NextBlog.git
cd NextBlog
pnpm install --frozen-lockfile
# 仅首次配置时复制；已有 .env 请保留
Copy-Item .env.example .env
```
{% endraw %}


package.json 声明依赖范围，pnpm-lock.yaml 固定实际解析结果。--frozen-lockfile 要求安装遵守锁文件；安装不匹配时应查明原因，不要为了绕过错误随意删除锁文件。node_modules 是安装结果，不需要提交。

## 第二步：连接自己的数据库

先在 PostgreSQL 中创建一个空数据库，例如 fieldnotes。连接信息填写到本地 .env；下面只是仓库的占位示例，必须换成自己的数据库账号和密码。

代码位置：`.env.example`（完整文件）。

{% raw %}
```text
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000
```
{% endraw %}


DATABASE_URL 同时表达数据库协议、用户名、密码、主机、端口和数据库名。密码包含 @、:、/ 等 URL 特殊字符时需要百分号编码。APP_ORIGIN 是浏览器访问的源；API_ORIGIN 是 Next.js 转发的目标，二者用途不同。不要将数据库密码写成 NEXT_PUBLIC_ 开头的环境变量。

SESSION_SECRET 用于会话签名，至少 32 个字符。下面的命令生成随机值，将输出保存到你自己的 .env，不要提交这个值。


{% raw %}
```powershell
node -e "console.log(require('node:crypto').randomBytes(32).toString('hex'))"
```
{% endraw %}


## 第三步：建表并启动


{% raw %}
```powershell
pnpm db:migrate
pnpm db:seed
pnpm dev
```
{% endraw %}


迁移负责创建表和索引；seed 可选，导入六篇示例文章，遇到已有 slug 会跳过。dev 同时启动 3000 端口的 Next.js 和 4000 端口的 Express。打开 http://localhost:3000 阅读文章，不要把前端端口换成 127.0.0.1 却仍保留不同的 APP_ORIGIN。

注册后默认是普通用户。要学习文章管理，先注册自己的邮箱，再在项目终端执行 `pnpm admin 你的注册邮箱`，刷新浏览器进入“管理”。没有默认管理员密码。

## 从首页认识 React 组件

代码位置：`src/app/page.tsx`（完整文件）。

{% raw %}
```tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
```
{% endraw %}


Page 是一个返回 JSX 的函数。JSX 看起来像 HTML，但允许用大括号嵌入 JavaScript 表达式；首字母大写的 ArticleList 表示另一个组件。import 引入的是函数定义，`<ArticleList />` 表示在页面中渲染它。

ArticleList 也用于归档页和管理页，通过 archive、manage 属性选择界面。这些传入组件的参数称为 props。列表内部的搜索词和页码则使用 state 保存，用户输入后由 React 更新界面。后面会分别学习 props、state 和异步请求。

## 练习与检查点

依次完成：打开首页、搜索 TypeScript、注册、授权管理员、发布一篇文章、退出登录后阅读。重启前后端后文章仍在，说明数据已经进入 PostgreSQL。若页面出现连接错误，先检查 Express 是否启动和数据库配置，不要立即修改 React 组件。

## 继续阅读

[下一篇]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
