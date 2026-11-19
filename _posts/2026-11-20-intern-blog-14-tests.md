---
layout: post
title: 实习博客实战（十四）：接口测试、运行配置与发布检查
date: 2026-11-20 00:14:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 完整测试和工程配置，验证 Session、越权、文章与评论业务。
keywords: Next.js, TypeScript, Express, PostgreSQL, DaisyUI, 全栈博客
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

完整测试和工程配置，验证 Session、越权、文章与评论业务。

> 本文是「实习博客实战」系列第 14 / 14 篇。源码路径相对于 NextBlog 项目根目录。

## 系列导航

1. [Next.js博客（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [Next.js博客（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [Next.js博客（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [Next.js博客（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [Next.js博客（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [Next.js博客（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [Next.js博客（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [Next.js博客（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})
9. [实习博客实战（九）：从静态页面走向 PostgreSQL 全栈博客]({% post_url 2026-11-20-intern-blog-09-database %})
10. [实习博客实战（十）：Express、Cookie、Session 与权限边界]({% post_url 2026-11-20-intern-blog-10-auth-api %})
11. [实习博客实战（十一）：全栈路由与每个页面完整源码]({% post_url 2026-11-20-intern-blog-11-pages %})
12. [实习博客实战（十二）：搜索、编辑器、登录与评论组件详解]({% post_url 2026-11-20-intern-blog-12-components %})
13. [实习博客实战（十三）：DaisyUI 卡片、Lucide 图标与深色主题]({% post_url 2026-11-20-intern-blog-13-theme %})
14. [实习博客实战（十四）：接口测试、运行配置与发布检查]({% post_url 2026-11-20-intern-blog-14-tests %})

## 测试要检查不可绕过的边界

server/app.test.ts 使用 Supertest 驱动真实 Express，并连接真实 PostgreSQL。测试创建随机命名的独立数据库，迁移完成后执行场景，最后关闭连接并删除自己创建的数据库，不清空 fieldnotes。运行账号需要 CREATE DATABASE 权限。

覆盖未登录 401、普通用户越权 403、伪造注册角色、CSRF 来源与 token、登录后 Session 轮换、密码错误、服务重启保持登录、退出后旧 Cookie 失效、文章 CRUD、重复 slug、版本冲突、搜索、评论归属与级联删除。

## 命令的用途

pnpm typecheck 做静态类型检查；pnpm test:api 做数据库集成测试；pnpm build 验证 Next.js 生产编译。浏览器检查还需要验证真实按钮、响应式布局和 Markdown 显示，这些不会由 TypeScript 自动保证。

pnpm dev 同时启动前后端。本地登录使用 HTTP 开发模式；生产 pnpm start 会启用 Secure Cookie，必须配好 HTTPS 与受控反向代理。不要把旧 out 目录部署为当前全栈网站，也不要在生产遗漏 tsx 等启动所需依赖。

## 提交前检查

.env、node_modules、构建目录和工具缓存必须被 Git 忽略。提交 .env.example 占位配置，不提交真实数据库密码和 SESSION_SECRET。第一次克隆请使用锁文件安装，随后执行迁移和可选的 seed。管理员账号在网页注册后通过服务端命令授予，没有默认公开密码。

项目源码仓库：[DKomorebi/NextBlog](https://github.com/DKomorebi/NextBlog)。本文代码为 2026-11-20 快照，具体依赖解析以仓库 pnpm-lock.yaml 为准。

## 验证边界

本地接口、浏览器主要流程、主题偏好与移动布局已经验证；公网 HTTPS 代理、多实例限流和完整跨浏览器兼容性仍需要在部署环境验收。当前不包含密码找回、邮箱验证、上传与嵌套评论。


## 本篇文件索引

- [server/app.test.ts](#source-1)
- [package.json](#source-2)
- [tsconfig.json](#source-3)
- [postcss.config.mjs](#source-4)
- [pnpm-workspace.yaml](#source-5)
- [.gitignore](#source-6)

<a id="source-1"></a>

### server/app.test.ts

以下为该文件的完整源码。

{% raw %}
```typescript
import { test } from "node:test";
import assert from "node:assert/strict";
import { randomBytes } from "node:crypto";
import { Client, Pool } from "pg";
import request from "supertest";
import { createDatabase } from "./db";
import { createApp } from "./app";
import { UserEntity } from "./entities";

test("PostgreSQL integration: authentication, session, CSRF, RBAC, CRUD and comments", async () => {
  const base = new URL(process.env.DATABASE_URL!);
  const database = "fieldnotes_test_" + randomBytes(6).toString("hex");
  const adminUrl = new URL(base);
  adminUrl.pathname = "/postgres";
  const controller = new Client({ connectionString: adminUrl.toString() });
  await controller.connect();
  await controller.query(`CREATE DATABASE "${database}"`);
  base.pathname = "/" + database;
  const db = await createDatabase(base.toString()).initialize();
  await db.runMigrations();
  const origin = "http://localhost:3000";
  const secret = randomBytes(48).toString("hex");
  let service = createApp(db, {
    secret,
    origin,
    pool: new Pool({ connectionString: base.toString() }),
  });
  try {
    const anonymous = request.agent(service.app);
    let r = await anonymous.get("/api/auth/session").expect(200);
    const csrf = r.body.csrf;
    assert.match(r.headers["set-cookie"][0], /HttpOnly/);
    assert.match(r.headers["set-cookie"][0], /SameSite=Lax/);
    const payload = {
      title: "测试文章",
      slug: "integration-post",
      category: "测试",
      excerpt: "摘要",
      content: "## Markdown\n\n**hello**",
    };
    await anonymous
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", csrf)
      .send(payload)
      .expect(401);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", origin)
      .send({})
      .expect(403);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", "https://evil.example")
      .set("X-CSRF-Token", csrf)
      .send({})
      .expect(403);
    await anonymous
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", csrf)
      .send({
        email: "bad@example.com",
        username: "bad",
        password: "Testing-password-123",
        role: "admin",
      })
      .expect(400);
    const user = request.agent(service.app);
    r = await user.get("/api/auth/session");
    const before = r.headers["set-cookie"][0].split(";")[0];
    const init = r.body.csrf;
    r = await user
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", init)
      .send({
        email: "reader@example.com",
        username: "读者",
        password: "Testing-password-123",
      })
      .expect(201);
    let token = r.body.csrf;
    assert.equal(r.body.user.role, "user");
    assert.notEqual(before, r.headers["set-cookie"][0].split(";")[0]);
    assert.equal(r.body.user.passwordHash, undefined);
    await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(403);
    await db
      .getRepository(UserEntity)
      .update({ email: "reader@example.com" }, { role: "admin" });
    r = await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(201);
    const article = r.body;
    assert.ok(article.id);
    await user
      .post("/api/articles")
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send(payload)
      .expect(409);
    r = await anonymous.get("/api/articles?q=Markdown").expect(200);
    assert.equal(r.body.total, 1);
    assert.equal(r.body.items[0].content, undefined);
    await user
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ ...payload, title: "已修改", version: 1 })
      .expect(200);
    await user
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ ...payload, version: 1 })
      .expect(409);
    const other = request.agent(service.app);
    const otherCsrf = (await other.get("/api/auth/session")).body.csrf;
    r = await other
      .post("/api/auth/register")
      .set("Origin", origin)
      .set("X-CSRF-Token", otherCsrf)
      .send({
        email: "other@example.com",
        username: "另一位读者",
        password: "Testing-password-456",
      })
      .expect(201);
    const otherToken = r.body.csrf;
    await other
      .put("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .send({ ...payload, version: 2 })
      .expect(403);
    await other
      .delete("/api/articles/" + article.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(403);
    r = await user
      .post(`/api/articles/${article.id}/comments`)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .send({ content: "管理员留言" })
      .expect(201);
    const commentId = r.body.id;
    await other
      .delete("/api/comments/" + commentId)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(403);
    r = await other
      .post(`/api/articles/${article.id}/comments`)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .send({ content: "普通用户留言" })
      .expect(201);
    await other
      .delete("/api/comments/" + r.body.id)
      .set("Origin", origin)
      .set("X-CSRF-Token", otherToken)
      .expect(204);
    r = await user.get("/api/auth/session");
    const savedCookie = r.headers["set-cookie"][0].split(";")[0];
    await service.close();
    service = createApp(db, {
      secret,
      origin,
      pool: new Pool({ connectionString: base.toString() }),
    });
    r = await request(service.app)
      .get("/api/auth/session")
      .set("Cookie", savedCookie)
      .expect(200);
    assert.equal(r.body.user.email, "reader@example.com");
    token = r.body.csrf;
    await request(service.app)
      .delete("/api/articles/" + article.id)
      .set("Cookie", savedCookie)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .expect(204);
    assert.equal(
      (await request(service.app).get(`/api/articles/${article.id}/comments`))
        .body.total,
      0,
    );
    await request(service.app)
      .get("/api/articles/integration-post")
      .expect(404);
    await request(service.app)
      .post("/api/auth/logout")
      .set("Cookie", savedCookie)
      .set("Origin", origin)
      .set("X-CSRF-Token", token)
      .expect(204);
    assert.equal(
      (
        await request(service.app)
          .get("/api/auth/session")
          .set("Cookie", savedCookie)
      ).body.user,
      null,
    );
    const login = request.agent(service.app);
    const loginCsrf = (await login.get("/api/auth/session")).body.csrf;
    await login
      .post("/api/auth/login")
      .set("Origin", origin)
      .set("X-CSRF-Token", loginCsrf)
      .send({ email: "reader@example.com", password: "wrong-password" })
      .expect(401);
    await login
      .post("/api/auth/login")
      .set("Origin", origin)
      .set("X-CSRF-Token", loginCsrf)
      .send({ email: "reader@example.com", password: "Testing-password-123" })
      .expect(200);
  } finally {
    await service.close();
    await db.destroy();
    await controller.query(`DROP DATABASE "${database}"`);
    await controller.end();
  }
});
```
{% endraw %}

<a id="source-2"></a>

### package.json

以下为该文件的完整源码。

{% raw %}
```json
{
  "name": "fieldnotes-blog",
  "version": "1.0.0",
  "private": true,
  "scripts": {
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
  },
  "dependencies": {
    "bcryptjs": "^3.0.3",
    "connect-pg-simple": "^10.0.0",
    "dotenv": "^18.0.10",
    "express": "^5.3.0",
    "express-rate-limit": "^8.7.4",
    "express-session": "^1.19.0",
    "helmet": "^8.3.0",
    "lucide-react": "^0.468.0",
    "next": "^16.0.0",
    "pg": "^8.23.1",
    "react": "^19.2.0",
    "react-dom": "^19.2.0",
    "react-markdown": "^10.1.0",
    "reflect-metadata": "^0.2.2",
    "rehype-highlight": "^7.0.2",
    "remark-gfm": "^4.0.1",
    "typeorm": "~0.3.31",
    "zod": "^4.6.7"
  },
  "devDependencies": {
    "@tailwindcss/postcss": "^4.1.0",
    "@types/connect-pg-simple": "^7.0.3",
    "@types/express": "^5.0.6",
    "@types/express-session": "^1.19.0",
    "@types/node": "^22.0.0",
    "@types/pg": "^8.23.1",
    "@types/react": "^19.2.0",
    "@types/react-dom": "^19.2.0",
    "@types/supertest": "^7.2.1",
    "concurrently": "^10.0.7",
    "daisyui": "^5.8.1",
    "postcss": "^8.5.0",
    "serve": "^14.2.4",
    "supertest": "^7.3.2",
    "tailwindcss": "^4.1.0",
    "tsx": "^4.23.15",
    "typescript": "^5.9.0"
  }
}
```
{% endraw %}

<a id="source-3"></a>

### tsconfig.json

以下为该文件的完整源码。

{% raw %}
```json
{
  "compilerOptions": {
    "target": "ES2017",
    "lib": [
      "dom",
      "dom.iterable",
      "esnext"
    ],
    "allowJs": true,
    "skipLibCheck": true,
    "strict": true,
    "noEmit": true,
    "esModuleInterop": true,
    "module": "esnext",
    "moduleResolution": "bundler",
    "resolveJsonModule": true,
    "isolatedModules": true,
    "jsx": "react-jsx",
    "incremental": true,
    "plugins": [
      {
        "name": "next"
      }
    ],
    "paths": {
      "@/*": [
        "./src/*"
      ]
    }
  },
  "include": [
    "next-env.d.ts",
    "**/*.ts",
    "**/*.tsx",
    ".next/types/**/*.ts",
    ".next/dev/types/**/*.ts",
    ".next-fullstack/types/**/*.ts",
    ".next-fullstack/dev/types/**/*.ts"
  ],
  "exclude": [
    "node_modules"
  ]
}
```
{% endraw %}

<a id="source-4"></a>

### postcss.config.mjs

以下为该文件的完整源码。

{% raw %}
```javascript
export default { plugins: { "@tailwindcss/postcss": {} } };
```
{% endraw %}

<a id="source-5"></a>

### pnpm-workspace.yaml

以下为该文件的完整源码。

{% raw %}
```yaml
allowBuilds:
  esbuild: true
```
{% endraw %}

<a id="source-6"></a>

### .gitignore

以下为该文件的完整源码。

{% raw %}
```text
node_modules/
.next/
out/
*.tsbuildinfo
.env*
!.env.example
.tools/

.pnpm-store/
.idea/

.next-fullstack/

# Generated copies published in the separate teaching blog
docs/dink-*/
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-13-theme %})

