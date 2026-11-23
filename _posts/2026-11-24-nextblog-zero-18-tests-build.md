---
layout: post
title: 验收与构建（十八）：从手工检查到接口测试、发布和故障定位
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 验收与构建（十八）：从手工检查到接口测试、发布和故障定位：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 检查分层，别把一种成功当成全部成功

`pnpm typecheck`检查TS，不证明SQL存在；`pnpm test:api`验证真实PostgreSQL接口，不证明页面按钮可点击；`pnpm build`验证Next生产构建，不证明SMTP能投递；浏览器测试验证事件与布局，若API被模拟，也不等于真实注册端到端收信。报告应分别说明证据。

## 2. app.test.ts 的顶层测试回调

测试解析当前DATABASE_URL，复制URL后将管理员连接指向postgres库；生成随机测试库名并CREATE DATABASE；createDatabase指向新库initialize并runMigrations；createApp注入对应Session Pool与内存mailer。随机库名由工具生成，不接受用户输入拼SQL。

Supertest的request.agent保存Cookie，先GET session取得csrf，再发送带Origin/令牌的请求。测试注册时先调用/code，从测试mailer捕获验证码，再POST/register。这是测试替身，不是生产提供获取验证码接口。

断言包括：Cookie的HttpOnly/SameSite；缺CSRF和错误Origin返回403；注册不能指定admin；普通用户不能创建文章；数据库提升role后可创建；slug冲突409；搜索返回精简列表；旧version更新409；作者/管理员评论权限；删除文章级联评论。

中途service.close后用同数据库重新createApp，再用旧Cookie请求session确认身份仍在，验证持久化；退出后旧Cookie不能恢复用户；错误密码401、正确密码200。finally关闭service/db并删除独立测试库，不能把生产库名拿去复用这段清理代码。

## 3. registration.test.ts 的时间、邮件和请求辅助函数

mailer(email,code)把验证码放Map并增加sends，可切换failMail模拟SMTP错误；now()返回可变at表示的Date；payload(email,code)构造合法注册体；post(path,body)统一附Origin/当前csrf。这些小方法让测试重点落在行为，而不是每次重复构造请求。

测试推进at模拟过期/重发/小时配额，不真实等待。验证存储digest不是明文、不在响应返回验证码；缺字段、确认密码不同、串邮箱、过期、5次错码、重放均失败；Promise.allSettled同时发送/注册，检查只能一个成功；重建服务后仍可使用数据库验证码；SMTP失败不能激活记录，且响应不含底层诊断。

注册成功会换csrf，所以测试随后更新csrf变量。忘记更新时后续请求403可能是令牌失效，而不是验证码业务坏了。

## 4. 手工浏览器验收脚本

按匿名访问首页→搜索→归档→阅读→注册发码→密码不一致→正确验证码→登录状态→评论→退出→重新登录的顺序操作。每步记录UI、Network状态码、预期数据库变化。再用普通用户尝试文章POST，应403；管理员编辑冲突应409。

表单单独检查：邮箱空时无发码请求；只填邮箱能发；发送冷却禁用；改邮箱清验证码；Enter和点击提交走同一逻辑；登录页没有确认密码/验证码；手机390px无横向滚动。邮件真实投递需要你控制的收件地址，测试不能擅自向其他人发信。

## 5. 构建和部署顺序

{% raw %}
```powershell
pnpm install --frozen-lockfile
pnpm typecheck
pnpm test:api
pnpm build
```
{% endraw %}

部署前给目标数据库执行迁移，配置目标环境的DATABASE_URL、SESSION_SECRET、APP_ORIGIN、API_ORIGIN、SMTP。生产用`pnpm start`同时启动Next与API，根next.config的distDir是.next-fullstack；旧out静态目录不是当前全栈应用。GitHub Pages只能托管dink教学博客，不能运行Express/PostgreSQL。

反向代理提供HTTPS并只暴露所需网页入口；API当前绑定回环地址，适合Next与API同主机。若拆容器，127.0.0.1只表示当前容器，必须重新设计网络地址与信任代理，不能照抄本机配置。

当前start:api仍使用tsx执行TS，部署包因此需要该运行工具；不能先`pnpm install --prod`去掉devDependencies里的tsx然后期待start:api可用。另一种部署路线是独立后端tsconfig编译到JS再node启动，但那是额外改造，不是当前脚本已做的事。

## 6. 常见状态码应去哪里排查

| 现象 | 优先看 | 不应立即做 |
| --- | --- | --- |
| 页面开了但文章失败 | API进程、rewrite、health | 重装整个Next |
| 所有POST都是403 | Origin端口、Cookie、csrf是否更新 | 放开权限绕过验证 |
| 注册400 | details、确认密码、6位码、有效期 | 直接把测试码写死 |
| 发码503 | SMTP五项、授权码、连接日志类别 | 向网页返回完整SMTP错误 |
| 编辑409 | version是否过期 | 强行忽略版本覆盖他人修改 |
| 生产登录不保持 | HTTPS、Secure Cookie、代理配置 | 将生产Cookie改成不安全来掩盖原因 |
| 连接耗尽 | 两个池、实例数、close/destroy | 每请求再建更多池 |

## 7. 文档也需要验证

本系列的完整源码直接从当前工作区读取并生成哈希清单，不手工复制旧版本。发布前检查所有post_url、YAML模板、Liquid raw与代码围栏匹配；方法索引从TypeScript AST生成，核对命名函数、Effect、JSX事件及API回调。dink最终以GitHub Actions的Jekyll构建与部署结果为准，不把Markdown写入磁盘等同于线上可见。

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-17-theme %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-19-method-index %})
