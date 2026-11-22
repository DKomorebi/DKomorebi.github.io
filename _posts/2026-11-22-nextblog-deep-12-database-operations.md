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
title: 数据库连接（三）：迁移、初始化、健康检查与故障定位
date: 2026-11-22 00:12:00 +0800
description: D · 数据库与会话连接：结合项目源码、逐段讲解和练习答案深入学习。
---

> D · 数据库与会话连接 · 第 12 / 12 篇。源码基准：[NextBlog 99b069b](https://github.com/DKomorebi/NextBlog/tree/99b069b66012e2000fa25201c72bd455003dfb07)。

## 三个管理命令共用同一个入口


代码位置：`server/manage.ts`（完整文件）。

{% raw %}
````typescript
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


process.argv[2] 决定 migrate、seed 或 admin；argv[3] 是授权时的邮箱。先 initialize，业务放在 try 中，finally 关闭 DataSource。命令不是持续提供 HTTP 服务，所以无论成功失败都要结束连接。

## migrate、seed、admin 不能互相替代

migrate 执行结构变化；seed 导入示例内容；admin 修改一个已注册账号的角色。运行 seed 不能弥补缺少的表，运行 admin 不能凭空注册账号，运行 migrate 也不会自动产生管理员密码。

seed 按 slug 检查是否存在，存在就跳过，目的是不要覆盖你已经编辑的同名文章。但“先查再插”并不是并发锁；不要同时运行多个 seed 并假定绝无竞争，最终仍依赖数据库唯一约束。

admin 把邮箱标准化后 update role，并检查 affected。邮箱未注册时应该报错；命令入口的授权来自你能操作服务器终端，而不是网站提供了一个允许任意访客提权的接口。

## health 能证明什么

server/app.ts 的 GET /api/health 执行 SELECT 1。它能说明此次数据库查询成功，但不证明每张业务表存在，也不证明注册、权限、Markdown 和主题功能正确。健康检查只是整体验收的一部分。

| 故障表现 | 可能层级 | 建议核对 |
| --- | --- | --- |
| 连接被拒绝 | 网络或监听 | 服务是否启动，主机端口是否正确 |
| 密码认证失败 | 数据库凭据 | 账号和密码，URL 编码 |
| database 不存在 | 库名 | 是否先创建目标库 |
| relation 不存在 | 表结构 | 是否对当前目标库执行迁移 |
| health 成功但登录失败 | 会话或认证 | sessions 表、密钥、账号、Cookie |
| GET 成功而 POST 403 | 来源或权限 | APP_ORIGIN、token、角色 |
| 生产 HTTP 登录不保持 | Cookie 传输条件 | HTTPS 与 Secure Cookie |

不要将完整连接串复制到公开日志。错误码与脱敏的目标信息通常足以区分连接、认证、结构与权限问题。

## 集成测试为什么创建独立数据库

server/app.test.ts 使用随机名称建立测试库、执行迁移、驱动 Express 请求并清理。它需要创建数据库权限，因此普通运行账号能读写业务表，并不保证有权运行这套测试。测试失败要区分环境权限失败与业务断言失败。

DataSource 与 Session Pool 都应指向同一个测试库，否则文章在临时库而会话落进真实库，会污染环境并让结果失真。createApp 接受 pool 参数，就是为了把会话连接也注入到测试环境。

## 避免把排错变成数据重置

遇到表不存在先确认 URL 的库名，不要立即删除数据目录；迁移失败先看失败语句与迁移记录，不要反复修改已执行迁移。需要结构演进时新增迁移；需要重置测试环境时只使用明确属于测试的数据库。

这些判断比记住一个“万能重装命令”更有价值。日常阅读项目不需要实际执行 down 或 DROP DATABASE。

## 最后串成一条可复述的流程

配置环境变量 → 创建 DataSource → initialize → createApp 创建 Session Pool → 安装中间件 → 监听 HTTP → 接收请求 → 权限与输入检查 → Repository 查询 → JSON 返回 → React 更新界面。

请不看代码复述这个流程，再打开 index、db、app、entities、migration、manage 对照。自测：两种数据库账号是哪两种？PostgreSQL 连接账号和 users 表中的网站账号。首次空库运行为什么不能跳过迁移？连接可用并不意味着应用表与 Session 表已经存在。

## 继续学习

[专题导读]({% post_url 2026-11-22-nextblog-deep-00-guide %})

[上一篇]({% post_url 2026-11-22-nextblog-deep-11-pools-session %})

## 官方资料

本文的具体行为以所列项目文件为准；以下官方资料用于查阅相应概念与 API。项目锁定 TypeORM 0.3.31 所在版本线；官网可能默认展示更新的大版本，复制 API 前应核对项目版本。

- [TypeORM DataSource](https://typeorm.io/docs/data-source/data-source/)
- [node-postgres 连接池](https://node-postgres.com/features/pooling)
- [Express Session](https://expressjs.com/en/resources/middleware/session.html)
