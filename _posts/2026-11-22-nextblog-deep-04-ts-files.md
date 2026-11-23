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
title: Express 与 TypeScript（一）：为什么 server 里都是 .ts
date: 2026-11-22 00:04:00 +0800
description: B · Express 与 TypeScript：结合项目源码、逐段讲解和练习答案深入学习。
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


> B · Express 与 TypeScript · 第 4 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 直接解释三个容易混淆的名字

.ts 是 TypeScript 源文件扩展名；.tsx 是包含 JSX 的 TypeScript 源文件扩展名；tsx 还是本项目用于执行 TypeScript 的一个命令行工具名称。第三个名字相同，并不要求它执行的文件必须以 .tsx 结尾。

Express 接收 HTTP 请求、执行业务并返回 JSON，当前 server 文件没有 JSX 元素，所以使用 .ts。React 页面返回 `<ArticleList />` 等 JSX，因此用 .tsx。前后端不是靠扩展名划分的：前端的 lib/theme.ts 也使用 .ts，后端如果有需要也可能处理模板或其他输出。

## 没有 .js 文件，Node 是怎样执行的


{% raw %}
```json
{
  "dev": "concurrently -k \"pnpm dev:api\" \"pnpm dev:web\"",
  "build": "next build --webpack",
  "typecheck": "tsc --noEmit",
  "dev:web": "next dev --webpack",
  "dev:api": "tsx watch server/index.ts",
  "start": "concurrently -k \"pnpm start:api\" \"pnpm start:web\"",
  "start:api": "tsx server/production.ts",
  "start:web": "next start",
  "db:migrate": "tsx server/manage.ts migrate",
  "db:seed": "tsx server/manage.ts seed",
  "admin": "tsx server/manage.ts admin",
  "test:api": "tsx --test server/app.test.ts"
}
```
{% endraw %}


dev:api 是 tsx watch server/index.ts。执行工具处理 TypeScript 源码并交给 Node 运行，watch 在文件变化时重新运行。这个流程不要求在 server 同目录生成一套 .js 文件，所以你看到目录只有 .ts 是正常的。

本项目不是用 `node server/index.ts` 作为约定入口，也没有配置 tsc 先输出后端 dist 再启动。某些 Node 版本具备有限原生类型剥离能力，但不能由此推断本项目可以任意更换启动工具；应按 package.json 的实际脚本运行。

## 运行、类型检查、构建是三件事

| 命令 | 做什么 | 不应误以为 |
| --- | --- | --- |
| pnpm dev:api | 执行并监听后端 TS | 已完成严格类型检查 |
| pnpm typecheck | tsc --noEmit 检查类型 | 会生成 server/*.js |
| pnpm build | Next.js 生产构建 | 会把 Express 编译成独立 dist 服务 |
| pnpm start:api | 通过 tsx 运行生产入口 | 不再需要 tsx 依赖 |

tsx 运行不能代替 tsc 的静态检查。tsconfig 中 noEmit 为 true，表示类型检查不额外写出 JavaScript。Next.js 自己会生成它的运行产物，但这不等于旁边的 Express 自动拥有同样的编译输出。

## 生产入口也很短


代码位置：`server/production.ts`（完整文件）。

{% raw %}
````typescript
Object.assign(process.env, { NODE_ENV: 'production' });
void import('./index');
````
{% endraw %}


生产入口设置模式后导入 index。业务仍使用相同应用创建逻辑；模式影响 Secure Cookie 和代理信任等配置。当前部署方式必须保留 tsx 与 concurrently 等启动工具，不能照搬“删掉全部 devDependencies 后直接 node dist”的其他教程。

如果将来希望独立编译 Express，可以新增后端专用 tsconfig、输出目录和 node 启动脚本，并检查 ESM、路径、迁移及运行依赖。这是另一套工程方案，不是把 .ts 文件手动改名 .js 就完成了。

## TypeScript 最终留下什么

interface、类型参数和参数标注主要服务于开发检查，不会成为数据库表，也不会自动生成输入验证代码。普通变量、函数调用、if、await 和对象值会参与运行。后面的 EntitySchema 是运行时对象，而 interface Article 是静态类型，这个差别会贯穿 TypeORM 学习。

## 练习与答案

先只阅读 tsconfig 的 noEmit 和 package.json 的 dev:api，再回答：为何执行 typecheck 后看不到 JS？因为明确禁止输出。为何 backend 不用 .tsx？因为没有 JSX。为何能使用名为 tsx 的工具运行 .ts？工具支持 TypeScript 文件，工具名并不是扩展名限制。

建议在临时练习文件里写一次“类型不匹配但逻辑仍可运行”的例子，分别运行类型检查和执行工具观察差异；不要把故意错误留在正式项目里。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-03-request-path %})

[下一篇]({% post_url 2026-11-22-nextblog-deep-05-typed-express %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。

- [TypeScript JSX](https://www.typescriptlang.org/docs/handbook/jsx.html)
- [Express 中间件](https://expressjs.com/en/guide/using-middleware/)
- [tsx 执行工具](https://github.com/privatenumber/tsx)
