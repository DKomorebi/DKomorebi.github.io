---
layout: post
title: 数据库连接（六）：从 .env、DataSource 到启动和关闭
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 数据库连接（六）：从 .env、DataSource 到启动和关闭：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 一次启动究竟发生什么

`pnpm dev:api` 执行 `tsx watch server/index.ts`。index 导入 db，db 顶部加载 reflect-metadata 和 dotenv/config，然后 main 调用 `createDatabase().initialize()`。只有连接初始化成功后才调用 createApp 并监听端口。因此“网页能打开”不代表“数据库已连接”：网页和 API 是两个独立进程。

`dotenv/config` 读取当前项目根目录的 `.env`，将值放入 process.env。它不会创建数据库或执行 SQL。process.env 中的值通常是 string 或 undefined，所以端口需要 Number(...)，缺失的 DATABASE_URL 则直接抛出有意义的错误。

## 2. createDatabase(url = process.env.DATABASE_URL)

这是一个工厂函数，输入连接字符串，输出尚未初始化的 TypeORM DataSource。默认参数仅在调用者省略或传 undefined 时读取环境变量。测试传入另一条连接字符串，因而能使用独立数据库，而不用临时改全局配置。

逐项读构造参数：type=postgres 选驱动；url 给主机、端口、库名与认证；entities 注册对象映射；migrations 注册建表变更类；synchronize=false 表示启动时不自动按实体修改表；logging=false 避免普通查询日志输出。修改这些选项只改变连接配置，并不意味着数据库已随之迁移。

{% raw %}
```ts
const db = createDatabase();       // 只是配置对象
await db.initialize();             // 建立连接/连接池
await db.query("SELECT 1");       // 真正执行数据库查询
await db.destroy();                // 脚本或退出时释放资源
```
{% endraw %}

这是生命周期示意，HTTP 服务不会每次请求都 initialize/destroy。连接池可以复用连接；每次请求建连接不仅慢，还可能耗尽数据库连接数。项目采用 TypeORM 0.3 系列，在线官网主版本可能更新，API 细节以锁文件与项目测试为准；[DataSource 官方说明](https://typeorm.io/docs/data-source/data-source/)可用于理解连接的建立和销毁职责。

## 3. 为什么还有 new Pool

TypeORM 管 users/articles/comments 和验证码事务；connect-pg-simple 使用 pg Pool 管 sessions。两个池可以连接同一个 PostgreSQL 数据库，但不会因为数据库 URL 相同就变成同一个池。createApp 的 options.pool 允许测试提供指向测试数据库的 Session 池，避免业务数据在测试库、Cookie 会话却落到正式库。

数据库连接池不是 Cookie，也不是每个用户一个数据库。很多浏览器会话可以共用服务器连接池，Session 表按 sid 区分会话。部署时要为两个池和实例数量预留连接额度，不要只算 TypeORM。

## 4. index.main() 的每个职责

第一步 await initialize；第二步把 secret、origin、production、mailer 传入 createApp；第三步 app.listen 绑定 127.0.0.1 和 API_PORT；第四步注册 SIGINT/SIGTERM 回调。收到退出信号时先 server.close 停止接受新连接，回调内关闭 Session Store/Pool，再 destroy TypeORM，最后 process.exit(0)。

main().catch 捕获启动失败，输出错误信息并以 1 退出，便于进程管理器识别失败。它不是用于处理每个 HTTP 请求的 catch，那是 app.ts 的错误中间件。

`server/production.ts` 先设置 NODE_ENV=production，再动态 import index，确保初始化时已经知道自己在生产环境。这会启用 Secure Cookie，所以本地 HTTP 学习应使用 dev，不要拿生产启动后 Cookie 不回传误判成密码错误。

## 5. manage.main()：三个命令分别做什么

| 分支 | 输入 | 具体执行 | 结果 |
| --- | --- | --- | --- |
| migrate | process.argv[2] | db.runMigrations() | 只执行尚未记录的迁移 |
| seed | 命令名 | 遍历 src/lib/posts 的文章；slug 已存在就跳过；sections.map 拼 Markdown，再 repo.save | 提供可阅读示例，不覆盖已存在文章 |
| admin | process.argv[3] 邮箱 | trim/lowercase；按 email update role=admin；检查 affected | 提升已注册账号，不创建默认管理员 |
| 其他 | 未识别命令 | 抛用法错误 | 非零退出 |

无论哪个分支，finally 都 destroy 连接，否则命令可能因为池仍打开而不退出。seed 中 `s.paragraphs.join("\n\n")` 保留段落，s.code 存在时再加 fenced code；这里读取旧静态数据只用于导入，网站首页目前读取数据库。

## 6. 故障定位顺序

ECONNREFUSED：检查数据库服务、地址和端口。认证失败：确认账号、密码与连接字符串编码。database does not exist：先创建库，迁移不等于 CREATE DATABASE。relation does not exist：检查是否对正确的库执行迁移。API health 成功但浏览器 403：检查 Origin/CSRF/端口，不应继续修改数据库密码。

只记录错误类别，不把完整 DATABASE_URL 贴到公开文章。练习：将测试库 URL 传给 createDatabase，但不传 options.pool，会出现什么问题？答案：Session 仍可能按默认环境变量连接原库，所以测试必须同时注入两个连接入口。

## 7. 静态种子文件保留的方法

`src/lib/posts.ts` 的 `getPost(slug)` 用 posts.find 回调比较 post.slug，找到返回旧 Post 对象，找不到返回undefined；`formatDate(date)` 用 replaceAll 把连字符改成点，仅做字符串显示。当前网页阅读和列表不调用这两个方法，而由API读取数据库。posts数组仍被manage.seed使用。`src/types/post.ts` 的Category/Section/Post描述这套种子结构，与数据库Article类型不同，不能用seed里的sections替换数据库content而不转换。

## 对照源码：server/db.ts（第 1–17 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/db.ts start: 1 end: 17 sha256: 180bf05893c262e8a3a136930db853f257690b21921e366017b72439c46c60f9 -->

{% raw %}
````ts
import "reflect-metadata";
import "dotenv/config";
import { DataSource } from "typeorm";
import { UserEntity, ArticleEntity, CommentEntity } from "./entities";
import { Initial1795000000000 } from "./migration";
import { RegistrationCodes1795392000000 } from "./registration-migration";
export function createDatabase(url = process.env.DATABASE_URL) {
  if (!url) throw new Error("请在 .env 中设置 DATABASE_URL");
  return new DataSource({
    type: "postgres",
    url,
    entities: [UserEntity, ArticleEntity, CommentEntity],
    migrations: [Initial1795000000000, RegistrationCodes1795392000000],
    synchronize: false,
    logging: false,
  });
}
````
{% endraw %}

## 对照源码：server/index.ts（第 1–32 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/index.ts start: 1 end: 32 sha256: 8b68e5bc0d580d13409dec0b9c0ab8c21a91ceecf1f1f0bd4d8eed25940248e3 -->

{% raw %}
````ts
import { createDatabase } from "./db";
import { createApp } from "./app";
import { createRegistrationMailer } from "./mail";
async function main() {
  const db = await createDatabase().initialize();
  const { app, close } = createApp(db, {
    secret: process.env.SESSION_SECRET ?? "",
    origin: process.env.APP_ORIGIN ?? "http://localhost:3000",
    production: process.env.NODE_ENV === "production",
    mailer: createRegistrationMailer(),
  });
  const server = app.listen(
    Number(process.env.API_PORT ?? 4000),
    "127.0.0.1",
    () =>
      console.log(
        "API ready: http://127.0.0.1:" + (process.env.API_PORT ?? 4000),
      ),
  );
  for (const signal of ["SIGINT", "SIGTERM"])
    process.once(signal, () =>
      server.close(async () => {
        await close();
        await db.destroy();
        process.exit(0);
      }),
    );
}
main().catch((e) => {
  console.error("API startup failed:", e.message);
  process.exit(1);
});
````
{% endraw %}

## 对照源码：server/production.ts（第 1–2 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/production.ts start: 1 end: 2 sha256: dea4b6b9f83f9156fc251fb66d06bf5f8353291121d4392037536ac2a6e1079f -->

{% raw %}
````ts
Object.assign(process.env, { NODE_ENV: 'production' });
void import('./index');
````
{% endraw %}

## 对照源码：server/manage.ts（第 1–46 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: server/manage.ts start: 1 end: 46 sha256: 1444c35233c0a842eba85f5cda781712ead73afdaf18c3feb80237ebb24bb8db -->

{% raw %}
````ts
import { createDatabase } from "./db";
import { UserEntity, ArticleEntity } from "./entities";
import { posts } from "../src/lib/posts";
async function main() {
  const db = await createDatabase().initialize();
  try {
    const command = process.argv[2];
    if (command === "migrate") {
      await db.runMigrations();
      console.log("Migrations applied");
    } else if (command === "admin") {
      const email = process.argv[3]?.trim().toLowerCase();
      if (!email)
        throw new Error("用法：pnpm admin 用户邮箱（该用户须先注册）");
      const result = await db
        .getRepository(UserEntity)
        .update({ email }, { role: "admin" });
      if (!result.affected) throw new Error("该邮箱未注册");
      console.log("Admin role granted");
    } else if (command === "seed") {
      const repo = db.getRepository(ArticleEntity);
      for (const p of posts) {
        if (await repo.existsBy({ slug: p.slug })) continue;
        await repo.save({
          slug: p.slug,
          title: p.title,
          excerpt: p.excerpt,
          category: p.category,
          content: p.sections
            .map(
              (s) =>
                `## ${s.heading}\n\n${s.paragraphs.join("\n\n")}${s.code ? "\n\n```tsx\n" + s.code + "\n```" : ""}`,
            )
            .join("\n\n"),
        });
      }
      console.log("Sample articles imported");
    } else throw new Error("使用 migrate、admin 或 seed");
  } finally {
    await db.destroy();
  }
}
main().catch((e) => {
  console.error(e.message);
  process.exit(1);
});
````
{% endraw %}

## 对照源码：.env.example（第 1–13 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: .env.example start: 1 end: 13 sha256: 27a1658833e30aec2cd264ab753c5d0777b52fa432ab3d4cf7fdaaf90169218f -->

{% raw %}
````text
DATABASE_URL=postgresql://blog:change-this-password@127.0.0.1:5432/fieldnotes
SESSION_SECRET=replace-with-a-random-secret-at-least-32-characters
APP_ORIGIN=http://localhost:3000
API_PORT=4000
API_ORIGIN=http://127.0.0.1:4000

# SMTP 注册邮件：587 使用 STARTTLS，465 使用隐式 TLS。
# SMTP_PASS 使用邮箱服务提供的 SMTP 授权码，不是网页登录密码。
SMTP_HOST=smtp.example.com
SMTP_PORT=587
SMTP_USER=sender@example.com
SMTP_PASS=replace-with-smtp-authorization-code
SMTP_FROM=sender@example.com
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-07-typeorm-model %})
