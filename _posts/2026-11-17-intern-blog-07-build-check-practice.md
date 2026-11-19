---
layout: post
title: NextBlog（七）：运行验收、排错与生产边界
date: 2026-11-17 09:06:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 建立类型检查、数据库集成测试、浏览器验收和配置排错的方法。
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

建立类型检查、数据库集成测试、浏览器验收和配置排错的方法。

> 本文是 NextBlog 全栈博客系列第 7 / 15 篇，内容对应当前全栈版本。

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

## 三条命令分别证明什么


{% raw %}
```powershell
pnpm typecheck
pnpm test:api
pnpm build
```
{% endraw %}


typecheck 执行 tsc --noEmit，发现类型使用不一致。test:api 用 Supertest 请求真实 Express，并连接真实 PostgreSQL。build 检查 Next.js 生产编译及路由生成。三者不能互相替代：能编译不代表数据库权限正确，接口通过也不代表手机按钮布局合适。

server/app.test.ts 创建随机命名的独立测试数据库，完成后删除自己创建的数据库，不清空你的 fieldnotes。测试使用的数据库账号需要创建数据库权限。不要把生产数据复制到临时测试里，也不要为了跑测试改成清空业务表。

## 接口测试覆盖的业务边界

| 场景 | 应观察的结果 |
| --- | --- |
| 游客修改文章 | 即使通过 CSRF 验证也会被身份检查拒绝 |
| 普通用户新增、修改或删除文章 | 403 |
| 注册时额外提交 role | 输入校验拒绝，不会创建管理员 |
| 缺少 token 或来源不匹配 | 403 |
| 重启应用后使用已有会话 | PostgreSQL 中的登录状态仍可读取 |
| 退出后再次使用旧 Cookie | 无法恢复原登录身份 |
| 重复邮箱或 slug | 409 |
| 旧 version 保存 | 409，避免覆盖较新的编辑 |
| 删除他人评论 | 普通用户被拒绝，管理员允许 |
| 删除文章 | 关联评论一起删除 |

## 手动验收按真实业务顺序走

先作为游客搜索、分类、分页和阅读 Markdown；再注册普通账号并发表评论；最后通过服务器命令授予自己的测试账号管理员权限，验证新建、修改和删除文章。删除操作只对自己新建的验收内容进行。

正文应包含标题、列表、表格和代码块，确认预览与阅读一致。尝试输入原始 HTML 脚本，当前阅读器应忽略它而不是执行。主题检查包括刷新、系统偏好、键盘切换、跨标签同步、减少动态效果和手机宽度。

## 按请求链路排查错误

| 现象 | 先检查 |
| --- | --- |
| 首页能开，文章加载失败 | 4000 端口服务、数据库连接、/api/health |
| 提示表不存在 | .env 指向的数据库是否执行过迁移 |
| 保存时提示来源不允许 | APP_ORIGIN 与浏览器协议、主机、端口是否一致 |
| 提示页面验证过期 | 会话是否变化；刷新会话后再提交 |
| 登录后仍不能管理 | 注册邮箱是否通过 pnpm admin 授权；刷新状态 |
| 编辑提示地址不匹配 | 从管理列表进入，确认 id 与 slug 属于同一文章 |
| 保存提示文章被修改 | 发生版本冲突，先保留自己的正文，再重新加载比较 |
| 端口已占用 | 关闭自己启动的旧进程，或同步调整端口配置 |

读取 Network 中的 HTTP 状态和 JSON 错误信息，比看到页面异常就清缓存更有用。500 是服务器内部问题；400 是输入不符合要求；401 与 403 分别代表身份或权限问题，但 403 也可能来自 CSRF，要结合 message 判断。

## 本地开发与生产启动

本地学习用 pnpm dev。生产先 pnpm build，再用 pnpm start 同时启动两个服务；server/production.ts 设置生产模式，此时 Cookie 启用 Secure，需要 HTTPS。服务器必须准备数据库、迁移、随机 SESSION_SECRET、正确 APP_ORIGIN 和受控代理转发。

当前启动命令依赖 tsx 与 concurrently 等工具，不能只删除开发依赖后假设启动仍可用。当前限流存储在进程内存，多实例部署要重新规划共享限流与代理信任边界。数据库备份、监控、邮件验证、密码找回和上传功能也不属于当前已实现的范围。

## 提交与验收记录

真实 .env、数据库密码、Session 密钥、node_modules 和 .next-fullstack 不提交。提交占位 .env.example、源文件与 pnpm-lock.yaml。新版本修改了什么，就验证对应行为；纯文档调整检查源码一致性、导航与 Jekyll 构建，不需要伪造一次业务测试结果。

练习：用自己的语言说明“build 成功为什么不能证明普通用户不能删文章”，再从 server/app.test.ts 找到对应权限断言。第十四篇给出完整测试与配置。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})

[下一篇]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
