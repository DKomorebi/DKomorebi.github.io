---
layout: post
title: 依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/DDD架构
description: 依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有：结合真实模板与现有 NextBlog 源码逐步解释。
keywords: Express, TypeScript, DDD, TypeORM, Prisma, node_modules
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


## 先纠正一个观察

当前博客并不是没有 node_modules。本机根目录 E:/文档/BSH/blog/node_modules 存在，server/node_modules 不存在；GitHub 仓库里看不到它，是因为 .gitignore 忽略安装产物。这是两个不同问题：依赖装在哪里，以及依赖是否提交。

本次从 server/index.ts 的位置实际解析 Express，得到根目录下 node_modules/.pnpm/express@5.3.0/node_modules/express/index.js。不是靠猜测说明“可能装在外面”。


{% raw %}
````text
blog/
├── package.json             # 同时声明 Next、Express 和 TypeORM
├── pnpm-lock.yaml
├── node_modules/            # 本机安装结果，被 Git 忽略
│   ├── express              # pnpm 管理的链接
│   └── .pnpm/
├── src/                     # Next.js 页面和 React 组件
└── server/                  # 后端源码，没有独立 package.json
````
{% endraw %}


## 为什么 server 能使用父目录的依赖

当前只有根 package.json，同时声明 React、Next、Express、TypeORM 等包。server 是源码子目录，不是一个拥有独立 manifest 的后端项目。Node 的模块解析可以向父级寻找 node_modules，因此 server 内部 import express 能使用根安装。

这与 .ts/.js 无关，也与 DDD 无关。源码目录名叫 server、backend 或 api，不会自动决定安装位置。运行工具和模块解析模式还有各自规则；这里用 createRequire 实测的是从 server 文件位置解析 CommonJS 包 Express，不能把这个命令当成所有 ESM/打包器解析规则的完整模拟。

## 你的目录为什么可能不同

如果你的后端有自己的 package.json，并在后端目录安装依赖，就通常会看到 backend/node_modules。没有读取到你另一个后端的目录，所以不能把这个常见原因当成已经核实的事实。可以先找它的 package.json 和锁文件，再看启动命令从哪里执行。

| 组织方式 | package.json | 依赖布局与特点 |
| --- | --- | --- |
| 当前单包项目 | 根目录一份 | 前后端共享安装，简单，部署依赖边界较粗 |
| 前后端独立项目 | web、api 各一份 | 各自声明、安装、构建与部署 |
| workspace 多包 | 根与子包各自清晰声明 | 通过包管理器组织共享与链接，不能依赖偶然提升 |

当前 pnpm-workspace.yaml 只配置 allowBuilds，没有声明 packages 列表，不能仅看到这个文件就宣称现有项目已经是前后端独立的 workspace 多包架构。

## pnpm 下为何还有 .pnpm

pnpm 使用虚拟存储目录和链接组织包依赖，所以顶层 express 可能是指向 .pnpm 内实际包目录的链接。这不是缺少包，也不建议手动复制 .pnpm 内部文件给 server 使用。包管理器应负责重建这些关系。

不同包管理器和安装选项可能使用不同布局，不能认为每个项目都应该出现相同层级。最可靠的是查看 manifest、锁文件、实际解析结果和执行位置。

## 你可以自己验证

在当前 NextBlog 根目录执行：


{% raw %}
````powershell
node -e "const {createRequire}=require('node:module');const path=require('node:path');const r=createRequire(path.resolve('server/index.ts'));console.log(r.resolve('express'));"
git check-ignore node_modules/express
pnpm why express
````
{% endraw %}


第一条显示 server 位置的 Express 解析结果；第二条说明该路径被 Git 忽略；第三条查看为什么安装了 Express。不要打印 process.env 或真实 .env 来解决依赖路径问题，依赖查找不需要暴露数据库连接串。

## 为什么不把 node_modules 推送到 GitHub

应提交 package.json、锁文件和必要安装配置，让另一台机器重建依赖。node_modules 通常体积大，可能包含平台相关产物与链接，不适合当作源码归档。克隆后看不到它是正常的，但仍必须按文档安装才能运行。

.gitignore 也忽略 .env，是为了不提交真实密钥；这和依赖忽略的原因不同。迁移到独立后端时，两个项目都要有相应忽略规则与占位 .env.example。

## 如果要让后端拥有自己的 node_modules

先在新的练习目录建立独立 api 包，整理后端直接使用的 dependencies：express、typeorm、pg、session 相关库、dotenv、bcryptjs、helmet、rate-limit、zod 等；devDependencies 包含 TypeScript、运行工具、对应类型与测试工具。不要把根 package.json 全复制后又声称已经缩小依赖范围。

建立后端专用 tsconfig：明确源码范围、输出目录、模块系统与运行方式。当前根配置针对 Next.js，moduleResolution=bundler、noEmit=true，不能当作独立 node dist 后端编译配置原封不动使用。服务端也不要依赖前端 @/* 别名或跨包 seed 源码。

随后选择一种包管理器安装，提交该包锁文件，验证独立目录里不借助父项目依赖仍能 typecheck、测试、构建和启动。网络上继续通过 Next.js rewrite 连接 API；分包并不会自动改变 Cookie 与 CSRF 配置。

## build 后是否还需要 node_modules

一般 TypeScript 编译只是把 TS 变成 JS，不会把 Express、ORM 和数据库驱动全部打包进去，所以运行 dist 通常仍需运行时依赖。只有明确使用打包器并验证外部依赖、原生库与生成客户端处理方式，才讨论不同部署产物。

原 express-ts-ddd 的 start 还依赖 ts-node；当前 NextBlog 的 start:api 依赖 tsx。若按这两个源码启动方式部署，就不能盲目删掉启动所需开发依赖。切换为编译产物启动是另一项明确配置工作。

自测：server 里没有 node_modules 能否说明 Express 没安装？不能。GitHub 看不到 node_modules 能否直接运行克隆的项目？通常不能，先安装。DDD 是否要求每一层一个 node_modules？完全不要求，架构职责与安装边界是两回事。

## 本组导航

[文档导读]({% post_url 2026-11-23-express-ts-ddd-00-guide %})

- [express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入]({% post_url 2026-11-23-express-ts-ddd-01-architecture %})
- [express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收]({% post_url 2026-11-23-express-ts-ddd-02-build %})
- [DDD 实战（三）：把当前 TypeORM 博客逐步分层]({% post_url 2026-11-23-express-ts-ddd-03-nextblog %})
- [依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有]({% post_url 2026-11-23-express-ts-ddd-04-node-modules %})

## 官方参考

[Node.js 从 node_modules 加载模块](https://nodejs.org/api/modules.html#loading-from-node_modules-folders)、[pnpm 的依赖链接结构](https://pnpm.io/symlinked-node-modules-structure)。本文目录结论另由本机实际解析验证。
