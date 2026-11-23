---
layout: post
title: NextBlog（三）：用真实业务理解 TypeScript
date: 2026-11-17 09:02:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 从联合类型、可空值和泛型，到表单键约束与运行时输入验证。
keywords: NextBlog, Next.js, TypeScript, Tailwind CSS, DaisyUI, Express, PostgreSQL
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
last_modified_at: 2026-11-20 07:31:36 +0800
---

> **2026-11-24 教程更新：** 本文保留为历史阶段资料。请优先阅读[《NextBlog 从零到逐方法：详细教程》]({% post_url 2026-11-24-nextblog-zero-00-guide %})，新版逐个解释页面方法、React 事件、Express/TypeORM、DDD 的实际边界，以及确认密码和邮箱验证码。本文早期源码不代表最新注册接口，完整当前源码见新版第二十、二十一篇。


从联合类型、可空值和泛型，到表单键约束与运行时输入验证。

> 本文是 NextBlog 全栈博客系列第 3 / 15 篇，内容对应当前全栈版本。

## 系列导航

1. [NextBlog（一）：全栈项目搭建与 React 入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [NextBlog（二）：Next.js 路由、布局与前后端边界]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [NextBlog（三）：用真实业务理解 TypeScript]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [NextBlog（四）：Tailwind CSS、DaisyUI 与简约页面]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [NextBlog（五）：React 状态、搜索请求与表单交互]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [NextBlog（六）：Express、TypeORM 与真实数据库请求]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [NextBlog（七）：运行验收、排错与生产边界]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [NextBlog（八）：串起登录、文章编辑与评论业务]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})
9. [NextBlog（九）：PostgreSQL 数据建模与迁移]({% post_url 2026-11-20-intern-blog-09-database %})
10. [NextBlog（十）：Express、Cookie、Session 与权限边界]({% post_url 2026-11-20-intern-blog-10-auth-api %})
11. [NextBlog（十一）：全栈路由与每个页面完整源码]({% post_url 2026-11-20-intern-blog-11-pages %})
12. [NextBlog（十二）：搜索、编辑器、登录与评论组件详解]({% post_url 2026-11-20-intern-blog-12-components %})
13. [NextBlog（十三）：DaisyUI 卡片、Lucide 图标与深色主题]({% post_url 2026-11-20-intern-blog-13-theme %})
14. [NextBlog（十四）：接口测试、运行配置与发布检查]({% post_url 2026-11-20-intern-blog-14-tests %})
15. [NextBlog（十五）：项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})

## 类型解决什么问题

JavaScript 在运行时执行请求与渲染；TypeScript 在开发、检查和构建阶段帮助发现不一致。它不会自动证明接口返回的数据正确，也不会代替登录权限检查。先从真实业务对象学习，比单独背语法更容易理解。

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
type User = {
  id: string;
  username: string;
  email: string;
  role: "user" | "admin";
};
type Session = { user: User | null; csrf: string };
type Article = {
  id: string;
  slug: string;
  title: string;
  excerpt: string;
  content: string;
  category: string;
  createdAt: string;
  version: number;
};
type Comment = {
  id: string;
  userId: string;
  username: string;
  content: string;
  createdAt: string;
};
```
{% endraw %}


User 的 role 是联合类型，只允许 user 或 admin；Session.user 是 User 与 null 的联合，因为游客也有会话。Article.id 和 slug 都是字符串，但业务含义不同。createdAt 在浏览器 API 类型中是字符串，而服务器实体使用 Date，因为 HTTP JSON 传输会将日期序列化。

这里的 Article 类型在列表和详情中复用，但列表接口为了节省流量没有返回 content。当前列表不读取正文，因此界面可以工作；若继续拆分类型，应定义只含列表字段的 ArticleSummary，而不是依靠一个宽泛类型假设每次都有正文。这也是阅读现有代码时要认识的改进空间。

## 可空值、可选参数与安全访问

`session.user?.role` 先检查 user 是否存在，游客不会因为直接取 null.role 而报错。`register?: boolean` 表示调用方可以不传；AuthForm 的默认值 false 将它解释为登录页。`Editor({ id }: { id?: string })` 用 id 是否存在区分新建和编辑。

可选链只是取值语法，不提供权限保障。浏览器可以修改自己的状态，后端必须再次读取登录身份与角色。后端的 `req.account!` 是非空断言，表示开发者相信前置中间件已保证登录；这个符号不会在运行时创建账号或检查权限。

## 泛型让请求返回值有类型

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
async function api<T>(
  path: string,
  method = "GET",
  body?: unknown,
  csrf?: string,
): Promise<T> {
  const r = await fetch("/api" + path, {
    method,
    credentials: "same-origin",
    cache: "no-store",
    headers: {
      "Content-Type": "application/json",
      ...(csrf ? { "X-CSRF-Token": csrf } : {}),
    },
    ...(body ? { body: JSON.stringify(body) } : {}),
  });
  if (r.status === 204) return undefined as T;
  const data = await r.json().catch(() => ({ message: "服务响应异常" }));
  if (!r.ok)
    throw new Error(data.details?.join("；") || data.message || "请求失败");
  return data;
}
```
{% endraw %}


T 是调用方传入的类型参数。例如 `api<Session>("/auth/session")` 让后续代码知道应读取 user 和 csrf。Promise<T> 表示异步完成后得到 T。body 使用 unknown，意味着封装允许接收不同业务参数，而不会在函数内擅自假设某个字段。

204 响应没有 JSON 正文，函数直接返回；其余响应先解析 JSON，再根据 r.ok 决定返回还是抛错。泛型本身没有验证 JSON 的形状，`undefined as T` 也只是断言。因此调用删除接口的地方不能继续读取返回对象；更严格的封装可以区分有正文与无正文的响应。

## keyof 与 typeof 如何约束表单

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
  function change(key: keyof typeof blank, value: string) {
    dirty.current = true;
    setForm((f) => ({ ...f, [key]: value }));
  }
```
{% endraw %}


typeof blank 取得初始表单对象的类型，keyof 再取出 title、slug、excerpt、category、content 这些键的联合。因此 change("title", value) 合法，拼错成 change("titel", value) 会在类型检查时报错。

`[key]` 是计算属性名，`{ ...f, [key]: value }` 创建新对象并替换一个字段。传函数给 setForm 可以基于最近的状态更新，而不是依赖某次渲染捕获的旧对象。dirty 是 ref，标记未保存变化；它的变化不直接触发界面重新渲染。

## 输入验证要在运行时再做一次

代码位置：`server/app.ts`（节选）。

{% raw %}
```typescript
const articleInput = z
  .object({
    title: z.string().trim().min(1).max(160),
    slug: z
      .string()
      .regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/)
      .max(120),
    excerpt: z.string().trim().max(500),
    category: z.string().trim().min(1).max(40),
    content: z.string().trim().min(1).max(200000),
  })
  .strict();
```
{% endraw %}


Zod 的 parse 在接口收到请求时实际执行。title 去掉两侧空格后不能为空，slug 只能由小写英文、数字和连字符构成，content 有长度上限，strict 拒绝额外字段。前端的类型、input 的 required 与后端的 parse 分别承担不同层面的检查。

数据库还有唯一约束、非空约束和外键，即使并发请求通过了前面的检查，也不能保存重复 slug 或无效关联。完整的数据保障横跨 TypeScript、Zod 和数据库约束。

## 工程配置与练习

tsconfig.json 开启 strict，noEmit 表示类型检查不额外生成 JavaScript；Next.js 负责应用构建。paths 将 @/* 映射到 src/*，所以 @/components/full-blog 指向 src/components/full-blog.tsx。带 JSX 的文件使用 .tsx，普通逻辑和类型文件通常使用 .ts。

练习时在临时编辑中把 role 写成不存在的值、把表单键拼错，然后运行 pnpm typecheck 观察错误并撤销。再思考：直接通过 HTTP 发出非法 role 时为什么仍必须由后端拒绝？答案是 TypeScript 不运行在外部请求上。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})

[下一篇]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
