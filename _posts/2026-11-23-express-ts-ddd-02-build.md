---
layout: post
title: express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收
date: 2026-11-23 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/DDD架构
description: express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收：结合真实模板与现有 NextBlog 源码逐步解释。
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


## 明确两条路线的区别

本篇是原模板路线，使用 Prisma 与 Cognito，在独立练习目录和独立数据库中操作。第三篇是当前 NextBlog 的 TypeORM + Session 分层路线。本次交付是教学文档和最小示例，没有把运行中的博客后端替换成原模板。

已完成的验证是：原仓库源码检查、脚本与路径核对，以及自编最小示例编译测试。没有替你配置真实 Cognito、安装原模板全部依赖或对其进行端到端数据库验收。以下步骤给出实际执行顺序和验收点，不声称旧模板在所有新环境下一键可用。

## 第一步：固定代码版本和包管理器

准备 Git、Node.js、Yarn Classic 1.x、PostgreSQL，以及有权使用的 Cognito 配置。模板 CI 和 Dockerfile 使用 Node 18，说明其历史验证基线；这不是让你把旧基线直接作为新生产环境方案。组织要求的 Node 版本应单独验证依赖兼容性。


{% raw %}
````powershell
git clone https://github.com/gonzaloplaza/express-ts-ddd.git express-ts-ddd-learning
cd express-ts-ddd-learning
git checkout 0de3752d9c5bce62691bcd5fbd721cd02df9603b
git switch -c study/ddd-learning
yarn --version
yarn install --frozen-lockfile
Copy-Item .env.dist .env
# 编辑 .env 后生成 Prisma Client
yarn prisma generate
````
{% endraw %}


checkout 固定提交，随后建立自己的学习分支。模板包含 yarn.lock，先沿用 Yarn；不要在同一目录交替执行 npm install、pnpm install 后还假定锁定结果相同。先确认 yarn --version 是预期的 Classic 版本。

## 第二步：配置独立环境

先创建空数据库 ddd_learning。不要把 Prisma 迁移直接指向现有博客 fieldnotes。将 .env 中的值换成自己的配置；PORT=4001 避开当前 Next.js 3000 和 Express 4000。


{% raw %}
````dotenv
PORT=4001
APP_NAME="express-ts-ddd-learning"
APP_DATABASE_URL="postgresql://YOUR_USER:YOUR_ENCODED_PASSWORD@localhost:5432/ddd_learning?schema=public"
APP_LOG_LEVEL="debug"
COGNITO_USER_POOL="YOUR_REAL_USER_POOL_ID"
COGNITO_CLIENT_ID="YOUR_REAL_CLIENT_ID"
COGNITO_REGION="YOUR_REGION"
````
{% endraw %}


模板读取 APP_DATABASE_URL，当前博客读取 DATABASE_URL，名字不同。prisma/schema.prisma 使用 env("APP_DATABASE_URL")；config/index.ts 通过 dotenv 加载配置，并按 NODE_ENV 选择环境配置。变量名只写对一半依然会连接失败。

模板的 CognitoClient 在构造时就建立用户池对象，容器解析整套路由时可能连带构造认证服务。因此不能保证只访问公开 health_check 就可以完全不配置 Cognito。占位字符串仅说明格式，不是可用身份服务。

如果暂时没有 Cognito，应另做一个仅健康检查的学习分支：从 ApiRouter 移除认证与受保护业务路由，从它的参数及容器中去除对应认证依赖，再验证启动。不能通过删除 authorizer 却保留写接口的方式当作可部署版本。保留 Session 的完整博客迁移见第三篇。

## 第三步：生成客户端不等于创建表

yarn prisma generate 根据 schema 生成数据库客户端；它不等于已经建立 Activity 表。当前核对的仓库有 schema，但没有已提交迁移目录。在自己的空练习库中可创建初始迁移：


{% raw %}
````powershell
yarn prisma migrate dev --name init
yarn dev
# 在另一个 PowerShell 终端访问
Invoke-RestMethod http://localhost:4001/health_check
````
{% endraw %}


migrate dev 是开发命令，可能使用 shadow database，需要相应数据库权限。不要因为权限不足就在业务库里执行重置。应使用专门的开发账号或按团队规范配置 shadow database。生成的迁移目录需要提交，之后部署使用已审查的迁移与 migrate deploy，不在生产运行开发重置流程。

health_check 返回 id、success、date，只证明应用响应；此模板的 HealthCheckService 没有查数据库。数据库验收还需要执行实际持久化读写，不能把 success=true 当成数据库已连通。

## 第四步：阅读并修正模板的具体问题

1. shared/infrastructure/Router.ts import 了 body-parser，但 package.json 未直接声明它。Yarn 的提升布局可能让它暂时可见，严格依赖布局不应依赖这种偶然性。可以显式声明它，也可以改用 Express 内置 json/urlencoded：从 express 导入 Router、json、urlencoded，替换原两个 bodyParser 调用并删除未使用 import。
2. CreateActivityController 先 json 再 status 的顺序不正确。应先设置状态再发送，例如 res.status(201).json({ success: true })；这是建议修正，原模板未自动完成。
3. 此控制器将多类异常统一包成 400，实际业务应区分输入错误、冲突和内部失败，不能把数据库故障误报为用户输入问题。
4. Server.stop 直接 await http.close 并不等于正确等待回调完成；要用 Promise 包住 close 回调，并在退出时正确断开 PrismaClient。

这些问题来自逐文件核对。选择模板意味着接手并维护它，不是只复制目录之后忽略运行行为。

## 第五步：开发、检查和测试

yarn dev 运行 nodemon，nodemon.json 再执行 ts-node ./bin/index.ts。它监听 src 和 bin；修改 config 或 .env 后，应手动重启，不能假定这些文件也被监听。

执行 yarn tsc --noEmit -p . 做类型检查，再按模板配置运行 yarn test。测试可能需要认证与外部环境配置；出现失败应区分编译问题、配置缺失与断言失败。本次没有把未执行的模板测试标成通过。

## 第六步：Windows 下编译 JavaScript

原 yarn build 脚本含 rm -rf，是类 Unix 命令，在原生 PowerShell 环境不能当作可靠跨平台清理方式。新克隆目录首次编译可直接调用编译器：


{% raw %}
````powershell
# 首次构建，跨平台直接调用 TypeScript 编译器
yarn tsc -p .
# 确认 dist/bin/index.js 存在后，在项目根目录运行
$env:NODE_ENV = "production"
node dist/bin/index.js
````
{% endraw %}


tsconfig 的 outDir 是 dist，入口在 bin/index.ts，因此期望输出 dist/bin/index.js，而不是 dist/index.js。config 虽然不显式列在 include 中，但被入口引用，会进入编译依赖图。编译成功后仍需检查输出文件确实存在。

后续构建前需要清理旧 dist 时，先确认当前目录是练习仓库并只删除其 dist。不要在不确定工作目录时复制递归删除命令。生产设置 NODE_ENV 后，config 中的 pro 配置会生效；本地检查完可用 Remove-Item Env:NODE_ENV 撤销当前终端变量。

原 yarn start 仍是 ts-node 运行源码，不会因为你执行过 build 就自动改成运行 dist。选择编译产物部署时，应明确增加自己的启动脚本并保证运行依赖和 Prisma Client 存在。

## 第七步：部署前检查哪些产物

需要 JavaScript 输出、运行时依赖、Prisma 生成客户端、迁移文件、环境配置和日志/关闭策略。模板 Dockerfile 将 dist 内容复制到 /app，所以容器内入口是 /app/bin/index.js；这与本地 node dist/bin/index.js 不矛盾，只是复制层级不同。

模板镜像安装生产依赖时的 Prisma CLI 可用性、生成阶段及最终镜像所需文件应实际验证，不能把 Dockerfile 当作已经通过你环境验收的证明。本篇未构建镜像。

## 验收清单

依赖安装成功 → 配置加载正确 → TypeScript 检查通过 → Prisma 生成成功 → 独立开发库迁移成功 → health 响应 → 使用真实授权验证活动读写 → 错误状态正确 → 编译产物启动 → 正常退出释放连接。每一步记录命令、结果和失败位置，比反复删除 node_modules 更能定位问题。

## 本组导航

[文档导读]({% post_url 2026-11-23-express-ts-ddd-00-guide %})

- [express-ts-ddd 教学（一）：从真实模板理解 DDD、分层与依赖注入]({% post_url 2026-11-23-express-ts-ddd-01-architecture %})
- [express-ts-ddd 构建（二）：Windows 搭建、Prisma 初始化与运行验收]({% post_url 2026-11-23-express-ts-ddd-02-build %})
- [DDD 实战（三）：把当前 TypeORM 博客逐步分层]({% post_url 2026-11-23-express-ts-ddd-03-nextblog %})
- [依赖管理（四）：为什么你的 Express 有 node_modules，而 server 没有]({% post_url 2026-11-23-express-ts-ddd-04-node-modules %})

## 核对来源

[原模板固定提交](https://github.com/gonzaloplaza/express-ts-ddd/tree/0de3752d9c5bce62691bcd5fbd721cd02df9603b)；具体文件链接与名称见正文。文档中的修正建议与示例代码是本次教学补充，不是原仓库已实现的功能。
