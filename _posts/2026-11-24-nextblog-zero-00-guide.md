---
layout: post
title: NextBlog 从零到逐方法：详细教程总导航
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: NextBlog 从零到逐方法：详细教程总导航：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

本系列以2026年11月24日的NextBlog源码快照为基准，包含确认密码、邮箱验证码、Session和权限。目标是从“先决定做什么”讲到“每个事件与方法为什么这样写”，不是只给框架介绍或粘贴源码。

## 阅读顺序

第一遍按01→18顺序搭建理解；遇到某个函数看19的索引，再打开20/21源码卷。每篇先解释动机、输入、执行顺序、成功/失败与边界，再附对应原文。首次学习可一天只走一个闭环，写完就用浏览器Network验证。

- 想重新搭建：01、03、05、06、07，再接页面。
- 不懂React/Next/TS：02、03，随后09、10。
- 不懂TypeORM：06、07、08、14，特别注意create与save、事务manager和行锁。
- 想逐方法学新注册登录：12→13→14→09，覆盖按钮、表单、计时器、SMTP、数据库、Cookie和Session。
- 想使用Express+TS+DDD：05→16，明确当前分层雏形与完整改造方案的区别。

## 版本与范围说明

这是新的主学习路线。旧“实习博客”“专题报告”“DDD”系列保留作历史资料，入口处链接到这里；它们早期的无邮箱验证注册示例不应继续复制到新版本。当前后台仍是Express+TypeORM的轻量组织，尚未完成全量DDD改造。第16篇的迁移示意与docs/ddd-example明确标注为教学方案。

本次完整源码附在20/21篇，来源清单、SHA-256和方法索引保存在NextBlog的docs/from-zero。敏感.env和node_modules不归档。文章里的框架事实以当前安装版本与官方资料核对，远程文档出现新主版本不表示项目已经升级。

## 全部章节

- [从零搭建（一）：先想清业务，再按可验证的小步骤落地]({% post_url 2026-11-24-nextblog-zero-01-plan-build %})
- [基础拆解（二）：TypeScript、React 与表单事件的写法]({% post_url 2026-11-24-nextblog-zero-02-typescript-react %})
- [页面入口（三）：Next.js 路由、布局与每个 page 方法]({% post_url 2026-11-24-nextblog-zero-03-next-routes %})
- [样式基础（四）：Tailwind、DaisyUI、布局和无障碍如何配合]({% post_url 2026-11-24-nextblog-zero-04-styles %})
- [Express 语法（五）：把 req、res、next 和整条中间件链写明白]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %})
- [数据库连接（六）：从 .env、DataSource 到启动和关闭]({% post_url 2026-11-24-nextblog-zero-06-database-start %})
- [TypeORM 建模（七）：实体、字段、迁移和数据库约束逐项解释]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %})
- [TypeORM 查询（八）：把文章和评论的每一个 API 翻译成数据库操作]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %})
- [共享前端（九）：api、SessionProvider、Header 的每个方法]({% post_url 2026-11-24-nextblog-zero-09-session-ui %})
- [列表页面（十）：首页、归档、管理列表的所有状态和事件]({% post_url 2026-11-24-nextblog-zero-10-list-pages %})
- [阅读与评论（十一）：ArticleReader、Markdown、Comments 的每个方法]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %})
- [注册登录前端（十二）：逐个输入框、事件、计时器和请求追踪]({% post_url 2026-11-24-nextblog-zero-12-auth-events %})
- [认证后端（十三）：Session、CSRF、注册、登录、退出逐方法讲解]({% post_url 2026-11-24-nextblog-zero-13-auth-server %})
- [邮箱业务（十四）：SMTP、验证码摘要、限流 SQL 与注册事务逐方法拆解]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %})
- [管理编辑（十五）：加载、change、save、Markdown 导入与离开提示]({% post_url 2026-11-24-nextblog-zero-15-editor %})
- [DDD 应用（十六）：现有代码的分层体现、完整示例与迁移步骤]({% post_url 2026-11-24-nextblog-zero-16-ddd %})
- [主题与首屏（十七）：HomeHero 和 ThemeToggle 的每个方法与事件]({% post_url 2026-11-24-nextblog-zero-17-theme %})
- [验收与构建（十八）：从手工检查到接口测试、发布和故障定位]({% post_url 2026-11-24-nextblog-zero-18-tests-build %})
- [方法与结构索引（十九）：从文件、方法和事件反查教学章节]({% post_url 2026-11-24-nextblog-zero-19-method-index %})
- [前端源码卷（二十）：全部页面、组件、样式与种子数据]({% post_url 2026-11-24-nextblog-zero-20-source-frontend %})
- [后端源码卷（二十一）：API、邮箱、事务、迁移、测试和配置]({% post_url 2026-11-24-nextblog-zero-21-source-backend %})

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-01-plan-build %})
