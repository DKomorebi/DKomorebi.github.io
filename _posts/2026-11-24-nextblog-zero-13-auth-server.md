---
layout: post
title: 认证后端（十三）：Session、CSRF、注册、登录、退出逐方法讲解
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 认证后端（十三）：Session、CSRF、注册、登录、退出逐方法讲解：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 两种令牌和一段身份不要混淆

fieldnotes.sid Cookie承载签名的会话标识，浏览器自动保存和发送；sessions表保存会话内容，其中有userId和csrf；前端收到的csrf放在后续写请求头。Cookie不是包含完整用户密码的JSON，CSRF也不是邮箱验证码。

HttpOnly使页面JS不能直接读取Cookie；SameSite=Lax限制部分跨站携带；生产secure=true要求HTTPS；maxAge=7天、rolling=true在符合会话中间件行为的响应中延长有效期。resave=false减少无修改时强制保存，saveUninitialized=false避免无内容的新会话直接保存；auth/session会写csrf，所以会产生需要保存的匿名Session。

## 2. `/api` 中间件：先保护写请求，再装载用户

GET/HEAD/OPTIONS跳过写请求校验，其余方法先比较Origin是否等于APP_ORIGIN，再从请求头取x-csrf-token并与Session里csrf比较。先比较字节长度，再timingSafeEqual，避免长度不同导致比较函数抛错；无预期token同样拒绝。

之后若session.userId存在，查询UserEntity并赋req.account，用户已删除就得到undefined。身份取自受保护Session，不从req.body.userId或role读取。next把请求交给路由。

本项目同源转发不等于所有CSRF风险自动消失，仍保留Origin和令牌检查。APP_ORIGIN包含协议和端口，localhost与127.0.0.1、3000与3001都不相同。

## 3. publicUser(u)

它创建只含id/username/email/role的新对象。这是输出白名单：数据库用户还有passwordHash、createdAt，但认证响应不直接展开用户全部属性。把`{...user}`当成便捷返回可能意外暴露密码哈希或未来新增敏感字段。

## 4. GET /auth/session

`req.session.csrf ??= randomBytes(32).toString("hex")`只在csrf为空或undefined时创建，不是每次GET都换令牌。返回publicUser或null，加当前csrf。匿名用户也需要它，因为注册、发送验证码和登录都是受CSRF保护的POST。

前端Provider首次调用此接口后按钮才启用；没有Cookie但手工拼个随机请求头，服务器没有对应Session token仍会403。

## 5. loginSession(req,user)

内部有两个Promise包装。第一个把session.regenerate回调转成await，创建新会话标识以减少会话固定风险；完成后写userId并重新生成csrf；第二个包装session.save，确保持久化完成再向浏览器返回成功。

包装回调的形式`e ? reject(e) : resolve()`让错误进入外层async异常链。如果不等待save，刚跳转首页的下一请求可能先于会话写入完成。登录后必须把新csrf回传前端，旧csrf不应继续使用。

## 6. POST /auth/register/code 与 POST /auth/register

/code专用每小时10次IP限流；从credentials.pick({email:true}).strict().parse取得规范邮箱；调用registration.sendCode；返回统一提示和60秒建议冷却，不返回代码。

/register复用每15分钟30次认证限流；extend昵称、确认密码、6位验证码；strict拒绝多余字段；refine对比两次密码。parse通过后调用registration.register创建用户，随后loginSession并返回201。register服务不收Express req/res，便于测试；但目前仍直接依赖TypeORM，属于职责抽取而非完整DDD。

校验不通过不会调用注册事务，更不会先存一条“临时用户”。验证记录与用户是分开的表。旧账号没有补验证流程，仍可登录，不应在介绍里写所有历史账号都已验证。

## 7. POST /auth/login 的每一步

credentials.strict.parse只允许email/password并执行规范化与长度约束；按email找用户；用bcrypt.compare比较明文输入与存储哈希。不存在用户时仍与固定的无关哈希做比较，减少明显的计算分支差异，但这不等于保证全接口恒定时间。

若无用户或比较失败，返回同一401“邮箱或密码错误”，不分别暴露是哪项错误；成功调用loginSession，返回publicUser和csrf。哈希不是可逆加密，不能从passwordHash“解密”出密码。确认密码只在注册防误输入，不是登录所需凭证。

## 8. POST /auth/logout

调用req.session.destroy回调：失败next(e)，不假装成功；成功res.clearCookie，选项的path/secure/sameSite等与设置时匹配，再204.end。只清Cookie而不删服务端Session，会让旧会话标识仍有机会使用；只删服务器记录不清浏览器Cookie，也会留下无效标识和不清晰体验，所以两边都做。

前端logout接着refresh获取新的匿名csrf。登录成功后的Header显示用户名不是因为服务器直接改了DOM，而是setSession触发React重渲染。

## 9. 权限矩阵与接口验收

| 行为 | 匿名 | 普通账号 | 管理员 |
| --- | --- | --- | --- |
| 搜索/阅读/读评论 | 可 | 可 | 可 |
| 写评论 | 401 | 可 | 可 |
| 删自己评论 | 401 | 可 | 可 |
| 删他人评论 | 401 | 403 | 可 |
| 新建/编辑/删除文章 | 401 | 403 | 可 |

错误格式与细节见Express章。测试还验证重新创建应用后同一个保存的Cookie仍能恢复身份，证明Session持久化，而不是偶然保存在某个JS变量里。

## 对照源码：server/app.ts（第 165–237 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: server/app.ts start: 165 end: 237 sha256: 7b3bfcaccc2cbb855ee42d403099af8162d907d6d8116dac75f30c6da47e0f1c -->

{% raw %}
````ts
  app.get("/api/auth/session", (req, res) => {
    req.session.csrf ??= randomBytes(32).toString("hex");
    res.json({
      user: req.account ? publicUser(req.account) : null,
      csrf: req.session.csrf,
    });
  });
  async function loginSession(req: Request, user: User) {
    await new Promise<void>((resolve, reject) =>
      req.session.regenerate((e) => (e ? reject(e) : resolve())),
    );
    req.session.userId = user.id;
    req.session.csrf = randomBytes(32).toString("hex");
    await new Promise<void>((resolve, reject) =>
      req.session.save((e) => (e ? reject(e) : resolve())),
    );
  }
  app.post("/api/auth/register/code", rateLimit({
    windowMs: 60 * 60 * 1000,
    limit: 10,
    standardHeaders: "draft-7",
    legacyHeaders: false,
    message: { message: "验证码发送过于频繁，请稍后重试" },
  }), async (req, res) => {
    const { email } = credentials.pick({ email: true }).strict().parse(req.body);
    await registration.sendCode(email);
    res.json({ message: "若邮箱尚未注册，验证码将发送到该邮箱，5 分钟内有效；已有账号请直接登录。", retryAfter: 60 });
  });
  app.post("/api/auth/register", authLimit, async (req, res) => {
    const input = credentials
      .extend({
        username: z.string().trim().min(2).max(40),
        confirmPassword: z.string().min(10).max(72),
        verificationCode: z.string().regex(/^\d{6}$/, "请输入 6 位邮箱验证码"),
      })
      .strict()
      .refine((v) => v.password === v.confirmPassword, {
        message: "两次输入的密码不一致", path: ["confirmPassword"],
      })
      .parse(req.body);
    const user = await registration.register(input);
    await loginSession(req, user);
    res.status(201).json({ user: publicUser(user), csrf: req.session.csrf });
  });
  app.post("/api/auth/login", authLimit, async (req, res) => {
    const input = credentials.strict().parse(req.body);
    const user = await db
      .getRepository(UserEntity)
      .findOneBy({ email: input.email });
    const valid = await bcrypt.compare(
      input.password,
      user?.passwordHash ??
        "$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxlS2MWV.y1WHJt9HmPj7G3ON1e",
    );
    if (!user || !valid) {
      res.status(401).json({ message: "邮箱或密码错误" });
      return;
    }
    await loginSession(req, user);
    res.json({ user: publicUser(user), csrf: req.session.csrf });
  });
  app.post("/api/auth/logout", (req, res, next) => {
    req.session.destroy((e) => {
      if (e) return next(e);
      res.clearCookie("fieldnotes.sid", {
        httpOnly: true,
        sameSite: "lax",
        secure: !!options.production,
        path: "/",
      });
      res.status(204).end();
    });
  });
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %})
