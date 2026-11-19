---
layout: post
title: NextBlog（八）：串起登录、文章编辑与评论业务
date: 2026-11-17 09:07:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 以发布一篇文章为主线，将会话、权限、Markdown 和评论连接起来。
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

以发布一篇文章为主线，将会话、权限、Markdown 和评论连接起来。

> 本文是 NextBlog 全栈博客系列第 8 / 15 篇，内容对应当前全栈版本。

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

## 从页面集合走向完整业务

读完前七篇后，可以沿“注册 → 登录 → 授权 → 发布 → 阅读 → 评论 → 修改 → 删除”理解整个应用。每一步都对应前端状态、HTTP 请求和数据库变化。本篇把它们连接起来；文件总表与全部源码统一放在第十五篇。

## 注册和登录究竟保存了什么

SessionProvider 首先读取 /api/auth/session，获得公开用户信息和 CSRF token。注册表单发送昵称、邮箱、密码，后端验证并用 bcrypt 保存密码哈希，角色固定为 user。登录成功时重建 Session ID，防止沿用旧会话标识。

浏览器 Cookie fieldnotes.sid 保存签名的会话标识；PostgreSQL sessions 保存 userId、csrf 等会话数据。HttpOnly 阻止页面脚本读取登录 Cookie，SameSite=Lax 限制部分跨站携带行为，生产额外使用 Secure。写请求仍需匹配 Origin 与 X-CSRF-Token，不能只依靠其中一个设置。

React Context 中的 user 用于显示导航与按钮，localStorage 只保存主题偏好，不保存登录密码或身份凭证。退出删除服务端会话并清除 Cookie，再刷新客户端状态。

## 管理员权限从服务器授予

先用自己的邮箱注册，再在终端运行 pnpm admin 你的注册邮箱。前端出现“管理”后可以进入编辑器；后端每次请求仍从数据库读取账号角色。普通用户不能通过更改浏览器 Context、构造按钮或提交 role 字段获取权限。

| 操作 | 游客 | 普通用户 | 管理员 |
| --- | --- | --- | --- |
| 阅读与搜索 | 可以 | 可以 | 可以 |
| 发表评论 | 需登录 | 可以 | 可以 |
| 删除自己的评论 | 不可以 | 可以 | 可以 |
| 删除他人评论 | 不可以 | 不可以 | 可以 |
| 新增、修改、删除文章 | 不可以 | 不可以 | 可以 |

## 发布和修改共用 Editor

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
  async function save(e: FormEvent) {
    e.preventDefault();
    setBusy(true);
    try {
      const a = await api<Article>(
        id ? "/articles/" + id : "/articles",
        id ? "PUT" : "POST",
        { ...form, ...(id ? { version } : {}) },
        session.csrf,
      );
      dirty.current = false;
      router.push("/posts/" + a.slug);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }
```
{% endraw %}


没有 id 时发送 POST；有 id 时发送 PUT，并附带加载时的 version。服务器返回保存后的文章，页面跳转到它的 slug。只有成功后才将 dirty 设为 false，失败时保留表单。409 发生时应先保留自己的修改，再加载新版本比较，不能直接更换版本号强行覆盖。

Editor 支持导入 .md 文件和即时预览。它对刷新、关闭页面使用 beforeunload 提示，但没有拦截所有站内 Link 导航，也没有自动保存草稿。学习时先保存再离开编辑页面。

## Markdown 怎样安全地显示

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
export function Markdown({ content }: { content: string }) {
  return (
    <div className="markdown">
      <ReactMarkdown
        remarkPlugins={[remarkGfm]}
        rehypePlugins={[rehypeHighlight]}
        skipHtml
        components={{
          a: ({ children, href }) => (
            <a href={href} rel="nofollow noopener noreferrer">
              {children}
            </a>
          ),
          img: ({ src, alt }) =>
            typeof src === "string" && /^(https?:\/\/|\/(?!\/))/.test(src) ? (
              <img
                src={src}
                alt={alt || ""}
                loading="lazy"
                referrerPolicy="no-referrer"
              />
            ) : (
              <span>[图片地址不支持]</span>
            ),
        }}
      >
        {content}
      </ReactMarkdown>
    </div>
  );
}
```
{% endraw %}


react-markdown 把 Markdown 解析成 React 节点，remark-gfm 提供表格等扩展语法，rehype-highlight 负责代码高亮。skipHtml 忽略正文中的原始 HTML，因此它不是支持脚本执行的 MDX 编辑器。

图片只允许 HTTP(S) 地址或本站绝对路径；HTTP 图片在 HTTPS 网站中仍可能受浏览器混合内容规则影响，实际发布优先使用 HTTPS。链接继续经过解析库的 URL 处理。文章中的 Markdown 与普通评论是两种显示方式：评论通过 React 文本输出，不会变成富文本 HTML。

## 评论属于谁由服务端决定

创建评论只提交 content。userId 从登录账号取得，articleId 来自目标文章；不接受浏览器随意填写作者身份。删除时再验证当前用户是否为作者或管理员。

代码位置：`server/app.ts`（节选）。

{% raw %}
```typescript
  app.delete("/api/comments/:id", requireUser, async (req, res) => {
    const repo = db.getRepository(CommentEntity);
    const comment = await repo.findOneBy({ id: uuid.parse(req.params.id) });
    if (!comment) {
      res.status(404).json({ message: "评论不存在" });
      return;
    }
    if (req.account!.role !== "admin" && comment.userId !== req.account!.id) {
      res.status(403).json({ message: "只能删除自己的评论" });
      return;
    }
    await repo.delete(comment.id);
    res.status(204).end();
  });
```
{% endraw %}


存在性检查让重复删除得到 404，归属检查让普通用户无法删除他人评论，成功后返回无正文的 204。前端根据同样的规则控制按钮，但数据库写入仍以这段服务端代码为准。

## 把链路画出来，再看源码

阅读链路：page.tsx → ArticleReader → api() → GET /api/articles/:slug → ArticleEntity → Markdown。

保存链路：Editor → api() 携带 Cookie 与 CSRF → 来源校验 → requireAdmin → Zod → Repository / QueryBuilder → PostgreSQL → 返回文章并跳转。

评论链路：Comments → 登录与内容校验 → CommentEntity → 返回列表；删除文章时外键级联删除评论。

建议先在一篇自己新建的文章上走完这三条链路，再进入第九至十四篇的分模块源码。需要按文件查找或复制整个版本时，从第十五篇的项目结构与源码总览进入。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})

[下一篇]({% post_url 2026-11-20-intern-blog-09-database %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
