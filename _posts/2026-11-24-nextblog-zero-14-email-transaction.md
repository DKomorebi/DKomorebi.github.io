---
layout: post
title: 邮箱业务（十四）：SMTP、验证码摘要、限流 SQL 与注册事务逐方法拆解
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 邮箱业务（十四）：SMTP、验证码摘要、限流 SQL 与注册事务逐方法拆解：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. createRegistrationMailer(env) 为什么返回函数

它接受环境变量对象，默认process.env；检查HOST/USER/PASS/FROM是否齐全，不齐返回undefined，由业务发送时明确返回503。端口默认587，转Number并检查整数范围1..65535，不合法抛配置错误。

nodemailer.createTransport建立发送配置：465使用隐式TLS，其他端口要求STARTTLS；auth使用SMTP账号与授权码；连接/问候/通信都有超时；禁止消息通过路径或URL读取任意附件内容。配置含密钥，只能在server里使用。[Nodemailer SMTP文档](https://nodemailer.com/smtp)解释secure与STARTTLS的区别。

返回的`async (email, code)`才真正执行sendMail，组装from/to/subject/text。检查accepted非空、rejected为空，否则抛错。SMTP接收不保证邮件进收件箱，可能进入垃圾邮件或后续退信，所以UI提示查收。运输层测试认证成功不等于真实投递测试通过。

类型`RegistrationMailer=(email:string,code:string)=>Promise<void>`表达能力契约。测试注入把code放Map的函数，生产入口注入真实SMTP函数；业务本身不必知道密码。未配置邮件时没有“万能验证码”或自动放行。

## 2. RegistrationError.constructor(status,message)

继承Error保存业务HTTP状态和提示。app.ts识别此类再输出，避免把SMTP底层异常字符串直接传给用户。它在业务层包含HTTP数字，是当前简化设计；严格DDD改造可以用独立业务错误码，再由控制器映射HTTP。

## 3. createRegistration(db,secret,mailer?,now) 与 digest

工厂接收DataSource、密钥、发送能力和时钟，返回sendCode/register两个方法。now默认new Date，测试可以推进虚拟时间而不真实睡五分钟。这是可测试性来自依赖注入的具体例子。

digest(email,nonce,code)用HMAC-SHA256处理JSON数组[registration,email,nonce,code]并输出hex。用途字符串避免混用其他验证码场景；邮箱绑定收件人；nonce绑定本次发送；JSON数组避免简单拼接时边界混淆；HMAC密钥阻止仅拿到数据库的人轻易枚举百万种数字。secret旋转会使未用验证码失效。

## 4. sendCode(email)：清理、预留、发送、激活

**第一段：配置和清理。** 无mailer抛503。获取at=now与新nonce；删除sent_at早于24小时前的记录。每行按email主键存储，不按每次发送无限追加。

**第二段：原子名额。** INSERT新邮箱默认sends=1；冲突时，仅当距sent_at至少60秒且当前小时发送未达5次或窗口已过期，才执行UPDATE。更新nonce，清digest/expires_at，attempts归0，sent_at改为现在。小时窗口过期则window_at重置且sends=1，否则sends+1。RETURNING没有行表示条件不满足，抛429。

不要改成“SELECT计数→在JS判断→UPDATE”，两个并发请求可能都看到旧计数。条件写进同一SQL，PostgreSQL在竞争同一主键时串行决定更新资格。

**第三段：已注册与生成。** 查询用户存在则直接返回，外层仍返回统一提示，不实际发注册邮件。否则randomInt(0,1000000)产生0..999999，padStart(6,"0")保留前导零。这比Math.random更适合凭证。

**第四段：外部邮件。** await mailer(email,code)，失败统一503；名额保留，digest仍空，所以没有可用验证码。外部SMTP不放在长数据库事务里，避免等待邮件网络时持续持锁。

**第五段：激活。** UPDATE同邮箱且nonce仍匹配的记录，写HMAC摘要与发送完成后五分钟的截止时间。nonce条件避免过时发送任务覆盖更新一轮记录。旧码在名额预留时已失效；若发送成功但激活数据库失败，邮件中的码不可用，需要稍后重发。这一简单两阶段流程不是跨SMTP和数据库的分布式事务。

## 5. register(input)：每一个判断为什么存在

输入是规范email、username、password、verificationCode；HTTP层已经验证confirmPassword。开始db.transaction，使用事务提供的manager，后续不要偷换成全局db.getRepository，否则可能跳出事务连接。

`SELECT ... FOR UPDATE`锁住email行。没digest、attempts>=5、无过期时间或当前已到截止时间，返回null。只有有效候选记录才计算actual摘要，与stored digest按相同hex编码转Buffer，再检查等长并timingSafeEqual。

错误码：事务内UPDATE attempts=attempts+1，然后返回null。**不能在这里直接throw业务错误**，否则事务回滚会把刚加的错误次数撤销，攻击者就能一直猜。当前实现让transaction正常提交，然后在外层见null才抛400。

正确码：manager.getRepository(UserEntity)，repo.create组装对象，bcrypt.hash(password,12)，role固定user，await repo.save写入；再UPDATE清digest/expires_at消费验证码，返回created。两个写入都成功才提交，任何异常都回滚。

为什么保留行而不DELETE？要保留发送窗口计数。为什么锁行？两个注册请求用同一验证码时，只能第一个看到有效摘要并创建用户；第二个等锁后看到摘要已空，失败。email UNIQUE仍是用户表最后保障。

## 6. 用时间线理解所有限制

00:00发第一封；00:20重发被60秒限制拒绝；01:00可重发并使第一封失效；该轮连续错5次后正确码也不能使用；激活后5分钟边界到达即失效；同一固定小时窗口最多预留5次。刷新页面、重启API不清数据库限额；IP计数当前在内存，重启会重置，所以二者需要区分。

邮件发送失败仍占名额，是防止通过触发失败不断发送/尝试的取舍。当前没有验证码管理员查询接口，不把code写入日志，不把SMTP_PASS发到浏览器。

## 7. 对照 SSM_demo 学习，而不是原样照搬

参考项目使用JavaMailSender和内存Map；NextBlog使用Nodemailer、PostgreSQL和注入时钟。相同的是“先发6位码→5分钟内验证→创建账号”；变化的是重启后可验证、HMAC存储、错误次数、并发消费和频率控制。不能把示例内存Map直接称为分布式可靠存储。

练习：为什么事务内更新attempts后throw会破坏防猜码？为什么邮件发送不应在锁住验证码行的注册事务中调用？能用自己的话回答这两点，再读源码里的SQL会容易得多。

## 对照源码：server/mail.ts（第 1–36 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/mail.ts start: 1 end: 36 sha256: e2693a63532858a40f3bb7feb9b3af43bbfa30b76d8666d85ac0e868890df008 -->

{% raw %}
````ts
import nodemailer from "nodemailer";

export type RegistrationMailer = (email: string, code: string) => Promise<void>;

// 没有 SMTP 配置时不伪造成功，也不向浏览器或日志泄露验证码。
export function createRegistrationMailer(
  env: NodeJS.ProcessEnv = process.env,
): RegistrationMailer | undefined {
  if (!env.SMTP_HOST || !env.SMTP_USER || !env.SMTP_PASS || !env.SMTP_FROM)
    return undefined;
  const port = Number(env.SMTP_PORT || 587);
  if (!Number.isInteger(port) || port < 1 || port > 65535)
    throw new Error("SMTP_PORT 必须是有效端口");
  const transport = nodemailer.createTransport({
    host: env.SMTP_HOST,
    port,
    secure: port === 465,
    requireTLS: port !== 465,
    auth: { user: env.SMTP_USER, pass: env.SMTP_PASS },
    connectionTimeout: 10000,
    greetingTimeout: 10000,
    socketTimeout: 15000,
    disableFileAccess: true,
    disableUrlAccess: true,
  });
  return async (email, code) => {
    const result = await transport.sendMail({
      from: env.SMTP_FROM,
      to: email,
      subject: "拾页 · 注册邮箱验证码",
      text: `你的注册验证码是：${code}\n\n5 分钟内有效，仅可使用一次。请勿向他人提供验证码。\n如果不是你本人操作，请忽略此邮件。`,
    });
    if (!result.accepted.length || result.rejected.length)
      throw new Error("SMTP recipient rejected");
  };
}
````
{% endraw %}

## 对照源码：server/registration.ts（第 1–94 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/registration.ts start: 1 end: 94 sha256: a118cb010f68399f01fba7af7d01c94096e7a0257731ce5434c14804e76a81ee -->

{% raw %}
````ts
import { createHmac, randomInt, randomUUID, timingSafeEqual } from "node:crypto";
import bcrypt from "bcryptjs";
import type { DataSource } from "typeorm";
import { UserEntity } from "./entities";
import type { RegistrationMailer } from "./mail";

export class RegistrationError extends Error {
  constructor(public status: number, message: string) {
    super(message);
  }
}

export function createRegistration(
  db: DataSource,
  secret: string,
  mailer?: RegistrationMailer,
  now = () => new Date(),
) {
  const digest = (email: string, nonce: string, code: string) =>
    createHmac("sha256", secret)
      .update(JSON.stringify(["registration", email, nonce, code]))
      .digest("hex");

  return {
    async sendCode(email: string) {
      if (!mailer)
        throw new RegistrationError(503, "注册邮件服务尚未配置，请联系站点管理员");
      const at = now();
      const nonce = randomUUID();
      // 一条 SQL 原子预留发送名额：同邮箱并发请求也不能绕过冷却和小时上限。
      await db.query("DELETE FROM registration_codes WHERE sent_at < $1", [
        new Date(at.getTime() - 24 * 60 * 60 * 1000),
      ]);
      const rows = await db.query(
        `INSERT INTO registration_codes(email, nonce, sent_at, window_at)
         VALUES ($1, $2, $3, $3)
         ON CONFLICT(email) DO UPDATE SET
           nonce = $2, digest = NULL, expires_at = NULL, attempts = 0, sent_at = $3,
           window_at = CASE WHEN registration_codes.window_at <= $3::timestamptz - interval '1 hour'
             THEN $3 ELSE registration_codes.window_at END,
           sends = CASE WHEN registration_codes.window_at <= $3::timestamptz - interval '1 hour'
             THEN 1 ELSE registration_codes.sends + 1 END
         WHERE registration_codes.sent_at <= $3::timestamptz - interval '60 seconds'
           AND (registration_codes.sends < 5 OR registration_codes.window_at <= $3::timestamptz - interval '1 hour')
         RETURNING email`,
        [email, nonce, at],
      );
      if (!rows.length)
        throw new RegistrationError(429, "请勿频繁发送：间隔至少 60 秒，每个邮箱每小时最多 5 次");
      // 已注册邮箱也返回相同提示，避免发送接口直接暴露账号是否存在。
      if (await db.getRepository(UserEntity).existsBy({ email })) return;
      const code = randomInt(0, 1000000).toString().padStart(6, "0");
      try {
        await mailer(email, code);
      } catch {
        // 保留发送配额，失败时无有效验证码。不要把 SMTP 异常（可能含凭据）返回前端。
        throw new RegistrationError(503, "邮件发送失败，请稍后重试或联系站点管理员");
      }
      await db.query(
        `UPDATE registration_codes SET digest = $3, expires_at = $4
         WHERE email = $1 AND nonce = $2`,
        [email, nonce, digest(email, nonce, code), new Date(now().getTime() + 5 * 60 * 1000)],
      );
    },

    async register(input: { email: string; username: string; password: string; verificationCode: string }) {
      // 行锁使校验、创建用户、消费验证码成为一个事务；错误次数必须提交，不能抛异常回滚。
      const user = await db.transaction(async (manager) => {
        const [record] = await manager.query(
          "SELECT * FROM registration_codes WHERE email = $1 FOR UPDATE",
          [input.email],
        );
        if (!record?.digest || record.attempts >= 5 || !record.expires_at ||
          new Date(record.expires_at).getTime() <= now().getTime()) return null;
        const expected = Buffer.from(record.digest, "hex");
        const actual = Buffer.from(digest(input.email, record.nonce, input.verificationCode), "hex");
        if (expected.length !== actual.length || !timingSafeEqual(expected, actual)) {
          await manager.query("UPDATE registration_codes SET attempts = attempts + 1 WHERE email = $1", [input.email]);
          return null;
        }
        const repo = manager.getRepository(UserEntity);
        const created = await repo.save(repo.create({
          email: input.email, username: input.username,
          passwordHash: await bcrypt.hash(input.password, 12), role: "user",
        }));
        // 保留发送计数，但销毁有效凭证；任何重放都会失败。
        await manager.query("UPDATE registration_codes SET digest = NULL, expires_at = NULL WHERE email = $1", [input.email]);
        return created;
      });
      if (!user) throw new RegistrationError(400, "验证码错误、已过期或已失效，请检查邮箱或重新获取");
      return user;
    },
  };
}
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-15-editor %})
