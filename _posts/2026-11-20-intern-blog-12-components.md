---
layout: post
title: 实习博客实战（十二）：搜索、编辑器、登录与评论组件详解
date: 2026-11-20 00:12:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 完整交互组件、Markdown 阅读器、共享类型和种子数据。
keywords: Next.js, TypeScript, Express, PostgreSQL, DaisyUI, 全栈博客
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

完整交互组件、Markdown 阅读器、共享类型和种子数据。

> 本文是「实习博客实战」系列第 12 / 14 篇。源码路径相对于 NextBlog 项目根目录。

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

## 从 api 函数开始读

api 封装 JSON 请求、同源 Cookie、CSRF header 和错误处理。204 没有响应体，不能继续调用 response.json。失败时抛出 Error，由表单和列表组件显示；不要把所有异常当作空数组，否则用户无法区分没有数据和网络故障。

SessionProvider 保存公开用户信息和 CSRF token。它不是权限来源，真正身份由后端 Session 决定。AuthForm 成功登录后更新 Context 并跳转首页，退出重新获取匿名会话。

## 搜索与分页

ArticleList 保存关键词、分类、页码和结果。输入变化后延迟 200ms 查询，effect 清理通过 alive 标记忽略过期响应。关键词和分类改变时回到第一页，避免停留在已经不存在的后续页。只有多页数据才显示分页，屏幕阅读器仍能收到结果数量更新。

## Markdown 编辑与阅读

Editor 保存表单字段，导入 .md 文件时检查大小，用同一个 Markdown 组件实时预览。正文使用 react-markdown、remark-gfm 和 rehype-highlight，不执行原始 HTML。Markdown 图片只允许 HTTP(S) 与本站绝对路径。文章保存带 version，冲突时保留当前表单内容并提示重新加载。

页面刷新或关闭时有未保存提示；当前站内 Link 导航不会自动拦截，因此编辑前离开仍应先保存。当前实现不提供草稿自动保存、富文本 HTML、图片上传和协同编辑。

## 评论区

登录用户可提交纯文本评论；用户 ID 由服务器取得。前端根据角色显示删除按钮，但每次删除都会再次经过后端归属校验。React 转义昵称与评论文本，不需要将评论转换为 HTML。

## 源码组织

full-blog.tsx 是当前集中组织的教学版业务组件，先读类型和请求函数，再读 SessionProvider、Header、ArticleList、AuthForm、ArticleReader、Comments、Editor。types/post.ts 和 lib/posts.ts 保留用于初始化示例文章，运行时列表已经读取数据库，并非从它们直接展示。


## 本篇文件索引

- [src/components/full-blog.tsx](#source-1)
- [src/types/post.ts](#source-2)
- [src/lib/posts.ts](#source-3)

<a id="source-1"></a>

### src/components/full-blog.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
"use client";
import {
  createContext,
  useContext,
  useEffect,
  useState,
  useRef,
  type ReactNode,
  type FormEvent,
} from "react";
import Link from "next/link";
import { HomeHero } from "./home-hero";
import { ThemeToggle } from "./theme-toggle";
import {
  BookOpen,
  Archive,
  UserRound,
  Settings2,
  LogOut,
  Search,
  Folder,
  CalendarDays,
  Plus,
  Pencil,
  Trash2,
  MessageCircle,
  Send,
  Save,
} from "lucide-react";
import { useRouter, useSearchParams } from "next/navigation";
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";
import rehypeHighlight from "rehype-highlight";
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
const Context = createContext<{
  session: Session;
  loading: boolean;
  error: string;
  refresh: () => Promise<void>;
  setSession: (s: Session) => void;
}>({
  session: { user: null, csrf: "" },
  loading: true,
  error: "",
  refresh: async () => {},
  setSession: () => {},
});
export function SessionProvider({ children }: { children: ReactNode }) {
  const [session, setSession] = useState<Session>({ user: null, csrf: "" }),
    [loading, setLoading] = useState(true),
    [error, setError] = useState("");
  async function refresh() {
    try {
      setSession(await api<Session>("/auth/session"));
      setError("");
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setLoading(false);
    }
  }
  useEffect(() => {
    void refresh();
  }, []);
  return (
    <Context.Provider value={{ session, loading, error, refresh, setSession }}>
      {children}
    </Context.Provider>
  );
}
const date = (s: string) =>
  new Date(s).toLocaleDateString("zh-CN", {
    year: "numeric",
    month: "long",
    day: "numeric",
  });
function ErrorMessage({ message }: { message: string }) {
  return message ? (
    <p className="alert alert-error" role="alert">
      {message}
    </p>
  ) : null;
}
export function Header() {
  const { session, loading, refresh } = useContext(Context);
  const [error, setError] = useState("");
  async function logout() {
    try {
      await api("/auth/logout", "POST", undefined, session.csrf);
      await refresh();
      setError("");
    } catch (e) {
      setError((e as Error).message);
    }
  }
  return (
    <header className="site-header">
      <div className="navbar header-inner">
        <Link className="brand" href="/">
          拾页
        </Link>
        <nav aria-label="主导航">
          <Link href="/" className="nav-item">
            <BookOpen size={16} aria-hidden="true" />
            文章
          </Link>
          <Link href="/archive" className="nav-item">
            <Archive size={16} aria-hidden="true" />
            归档
          </Link>
          <Link href="/about" className="nav-item">
            <UserRound size={16} aria-hidden="true" />
            关于
          </Link>
          {session.user?.role === "admin" && (
            <Link href="/admin" className="nav-item">
              <Settings2 size={16} aria-hidden="true" />
              管理
            </Link>
          )}
          {loading ? (
            <span>加载中</span>
          ) : session.user ? (
            <>
              <span className="username">{session.user.username}</span>
              <button className="btn btn-ghost btn-sm" onClick={logout}>
                <LogOut size={16} aria-hidden="true" />
                退出
              </button>
            </>
          ) : (
            <Link className="btn btn-ghost btn-sm nav-login" href="/login">
              <UserRound size={16} aria-hidden="true" />
              登录 / 注册
            </Link>
          )}
          <ThemeToggle />
        </nav>
      </div>
      <ErrorMessage message={error} />
    </header>
  );
}
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
export function ArticleList({
  archive = false,
  manage = false,
}: {
  archive?: boolean;
  manage?: boolean;
}) {
  const { session, loading } = useContext(Context);
  const [items, setItems] = useState<Article[]>([]),
    [q, setQ] = useState(""),
    [category, setCategory] = useState(""),
    [categories, setCategories] = useState<{ name: string }[]>([]),
    [page, setPage] = useState(1),
    [total, setTotal] = useState(0),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(true),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    const timer = setTimeout(() => {
      setBusy(true);
      api<{ items: Article[]; total: number }>(
        `/articles?q=${encodeURIComponent(q)}&category=${encodeURIComponent(category)}&page=${page}`,
      )
        .then((r) => {
          if (alive) {
            setItems(r.items);
            setTotal(r.total);
            setError("");
          }
        })
        .catch((e) => {
          if (alive) setError(e.message);
        })
        .finally(() => {
          if (alive) setBusy(false);
        });
    }, 200);
    return () => {
      alive = false;
      clearTimeout(timer);
    };
  }, [q, category, page, revision]);
  useEffect(() => {
    api<{ name: string }[]>("/categories")
      .then(setCategories)
      .catch(() => {});
  }, [revision]);
  async function remove(a: Article) {
    if (!confirm(`确定删除「${a.title}」及其评论？`)) return;
    try {
      await api("/articles/" + a.id, "DELETE", undefined, session.csrf);
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    }
  }
  if (manage && loading) return <p>正在验证身份…</p>;
  if (manage && session.user?.role !== "admin")
    return (
      <section className="card panel">
        <h1>仅管理员可管理文章</h1>
        <Link href="/login">登录账号</Link>
      </section>
    );
  return (
    <>
      {!manage && !archive ? (
        <HomeHero />
      ) : (
        <section className="intro">
          <h1>{manage ? "文章管理" : "文章归档"}</h1>
          {manage && (
            <Link className="btn btn-primary" href="/admin/new">
              <Plus size={17} aria-hidden="true" />
              写一篇文章
            </Link>
          )}
        </section>
      )}
      <div id="articles" className="section-heading">
        <h2>{manage ? "所有文章" : archive ? "全部文章" : "最新文章"}</h2>
      </div>
      <div className="list-toolbar">
        <label className="search">
          <Search className="search-icon" size={18} aria-hidden="true" />
          <span className="sr-only">搜索文章</span>
          <input
            className="input"
            placeholder="搜索文章或正文…"
            value={q}
            onChange={(e) => {
              setQ(e.target.value);
              setPage(1);
            }}
          />
        </label>
        <label>
          <span className="sr-only">分类</span>
          <select
            className="select"
            value={category}
            onChange={(e) => {
              setCategory(e.target.value);
              setPage(1);
            }}
          >
            <option value="">全部分类</option>
            {categories.map((c) => (
              <option key={c.name}>{c.name}</option>
            ))}
          </select>
        </label>
      </div>
      <ErrorMessage message={error} />
      {error && (
        <button
          className="btn btn-ghost btn-sm"
          onClick={() => setRevision((n) => n + 1)}
        >
          重新加载
        </button>
      )}
      <p className="sr-only" aria-live="polite">
        {busy ? "正在翻开书页…" : `${total} 篇文章`}
      </p>
      {!busy && !error && !items.length && (
        <div className="empty">
          <h2>这里还没有文章</h2>
          <p>试试其他关键词，或等待下一次更新。</p>
          <button
            className="btn btn-ghost btn-sm"
            onClick={() => {
              setQ("");
              setCategory("");
              setPage(1);
            }}
          >
            清空筛选
          </button>
        </div>
      )}
      <div className={archive ? "archive-list" : "article-list"}>
        {!error &&
          items.map((a) => (
            <article className="card article-card" key={a.id}>
              <div className="meta">
                <span className="badge badge-soft badge-primary">
                  <Folder size={13} aria-hidden="true" />
                  {a.category}
                </span>
                <time>
                  <CalendarDays size={13} aria-hidden="true" />
                  {date(a.createdAt)}
                </time>
              </div>
              <h2>
                <Link href={"/posts/" + a.slug}>{a.title}</Link>
              </h2>
              {!archive && <p>{a.excerpt}</p>}
              {manage && (
                <div className="card-foot">
                  {manage && (
                    <div className="actions">
                      <Link
                        href={`/admin/${a.id}?slug=${encodeURIComponent(a.slug)}`}
                      >
                        <Pencil size={15} aria-hidden="true" />
                        编辑
                      </Link>
                      <button
                        className="btn btn-ghost btn-sm text-error"
                        onClick={() => remove(a)}
                      >
                        <Trash2 size={15} aria-hidden="true" />
                        删除
                      </button>
                    </div>
                  )}
                </div>
              )}
            </article>
          ))}
      </div>
      {total > 10 && (
        <div className="pagination">
          <button
            className="btn btn-ghost btn-sm"
            disabled={busy || page === 1}
            onClick={() => setPage((n) => n - 1)}
          >
            上一页
          </button>
          <span>
            第 {page} / {Math.max(1, Math.ceil(total / 10))} 页
          </span>
          <button
            className="btn btn-ghost btn-sm"
            disabled={busy || page * 10 >= total}
            onClick={() => setPage((n) => n + 1)}
          >
            下一页
          </button>
        </div>
      )}
    </>
  );
}
export function AuthForm({ register = false }: { register?: boolean }) {
  const {
    session,
    setSession,
    loading,
    error: connection,
    refresh,
  } = useContext(Context);
  const [error, setError] = useState(""),
    [busy, setBusy] = useState(false);
  const router = useRouter();
  async function submit(e: FormEvent<HTMLFormElement>) {
    e.preventDefault();
    const f = new FormData(e.currentTarget);
    setBusy(true);
    try {
      setSession(
        await api<Session>(
          "/auth/" + (register ? "register" : "login"),
          "POST",
          {
            email: f.get("email"),
            password: f.get("password"),
            ...(register ? { username: f.get("username") } : {}),
          },
          session.csrf,
        ),
      );
      router.push("/");
    } catch (e) {
      setError((e as Error).message);
      await refresh();
    } finally {
      setBusy(false);
    }
  }
  return (
    <section className="card panel auth">
      <h1>{register ? "初次见面，欢迎你" : "欢迎回来"}</h1>
      <ErrorMessage message={error || connection} />
      {connection && (
        <button className="btn btn-ghost btn-sm" onClick={refresh}>
          重新连接
        </button>
      )}
      {session.user ? (
        <p>
          你已登录为 {session.user.username}。<Link href="/">返回首页</Link>
        </p>
      ) : (
        <form onSubmit={submit}>
          {register && (
            <label>
              昵称
              <input
                className="input"
                name="username"
                minLength={2}
                maxLength={40}
                autoComplete="nickname"
                required
              />
            </label>
          )}
          <label>
            邮箱
            <input
              className="input"
              name="email"
              type="email"
              autoComplete="email"
              maxLength={255}
              required
            />
          </label>
          <label>
            密码
            <input
              className="input"
              name="password"
              type="password"
              minLength={10}
              maxLength={72}
              autoComplete={register ? "new-password" : "current-password"}
              required
            />
          </label>
          <p className="muted">至少 10 个字符，最多 72 字节。</p>
          <button
            className="btn btn-primary"
            disabled={busy || loading || !session.csrf}
          >
            {busy ? "请稍候…" : register ? "注册并登录" : "登录"}
          </button>
          <p>
            {register ? "已有账号？" : "还没有账号？"}{" "}
            <Link href={register ? "/login" : "/register"}>
              {register ? "去登录" : "创建账号"}
            </Link>
          </p>
        </form>
      )}
    </section>
  );
}
export function ArticleReader({ slug }: { slug: string }) {
  const [article, setArticle] = useState<Article | null>(null),
    [error, setError] = useState(""),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    setArticle(null);
    api<Article>("/articles/" + encodeURIComponent(slug))
      .then((a) => {
        if (alive) {
          setArticle(a);
          setError("");
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [slug, revision]);
  return (
    <div className="reading">
      <Link href="/">返回文章列表</Link>
      <ErrorMessage message={error} />
      {error && (
        <button
          className="btn btn-ghost btn-sm"
          onClick={() => setRevision((n) => n + 1)}
        >
          重试
        </button>
      )}
      {!article && !error && <p>正在打开文章…</p>}
      {article && (
        <>
          <article>
            <header className="article-head">
              <div className="meta">
                <span>{article.category}</span>
                <time>{date(article.createdAt)}</time>
              </div>
              <h1>{article.title}</h1>
              <p>{article.excerpt}</p>
            </header>
            <Markdown content={article.content} />
          </article>
          <Comments articleId={article.id} />
        </>
      )}
    </div>
  );
}
function Comments({ articleId }: { articleId: string }) {
  const { session } = useContext(Context);
  const [items, setItems] = useState<Comment[]>([]),
    [total, setTotal] = useState(0),
    [page, setPage] = useState(1),
    [content, setContent] = useState(""),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(false),
    [revision, setRevision] = useState(0);
  useEffect(() => {
    let alive = true;
    api<{ items: Comment[]; total: number }>(
      `/articles/${articleId}/comments?page=${page}`,
    )
      .then((r) => {
        if (alive) {
          setItems(r.items);
          setTotal(r.total);
          setError("");
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [articleId, page, revision]);
  async function submit(e: FormEvent) {
    e.preventDefault();
    setBusy(true);
    try {
      await api(
        `/articles/${articleId}/comments`,
        "POST",
        { content },
        session.csrf,
      );
      setContent("");
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(false);
    }
  }
  async function remove(id: string) {
    if (!confirm("确定删除这条评论？")) return;
    try {
      await api("/comments/" + id, "DELETE", undefined, session.csrf);
      setPage(1);
      setRevision((n) => n + 1);
    } catch (e) {
      setError((e as Error).message);
    }
  }
  return (
    <section className="comments">
      <h2>
        <MessageCircle size={21} aria-hidden="true" />
        页边留言 <span className="muted">{total}</span>
      </h2>
      <ErrorMessage message={error} />
      {session.user ? (
        <form onSubmit={submit}>
          <label htmlFor="comment">以 {session.user.username} 的身份留言</label>
          <textarea
            className="textarea"
            id="comment"
            required
            maxLength={2000}
            value={content}
            onChange={(e) => setContent(e.target.value)}
            placeholder="写下你的想法…"
          />
          <button
            className="btn btn-primary"
            disabled={busy || !content.trim()}
          >
            <Send size={16} aria-hidden="true" />
            {busy ? "正在发表…" : "发表留言"}
          </button>
        </form>
      ) : (
        <p className="alert">
          <Link href="/login">登录</Link> 后参与讨论，也可以{" "}
          <Link href="/register">注册账号</Link>。
        </p>
      )}
      {items.map((c) => (
        <article key={c.id} className="comment">
          <div className="meta">
            <strong>{c.username}</strong>
            <time>{date(c.createdAt)}</time>
            {(session.user?.id === c.userId ||
              session.user?.role === "admin") && (
              <button
                className="btn btn-ghost btn-sm text-error"
                onClick={() => remove(c.id)}
              >
                <Trash2 size={15} aria-hidden="true" />
                删除
              </button>
            )}
          </div>
          <p className="whitespace-pre-wrap">{c.content}</p>
        </article>
      ))}
      {!items.length && !error && (
        <p className="muted">还没有留言，来写下第一句吧。</p>
      )}
      {total > 30 && (
        <div className="pagination">
          <button
            className="btn btn-ghost btn-sm"
            disabled={page === 1}
            onClick={() => setPage((n) => n - 1)}
          >
            上一页
          </button>
          <span>
            {page} / {Math.max(1, Math.ceil(total / 30))}
          </span>
          <button
            className="btn btn-ghost btn-sm"
            disabled={page * 30 >= total}
            onClick={() => setPage((n) => n + 1)}
          >
            下一页
          </button>
        </div>
      )}
    </section>
  );
}
const blank = {
  title: "",
  slug: "",
  excerpt: "",
  category: "随笔",
  content: "",
};
export function Editor({ id }: { id?: string }) {
  const { session, loading } = useContext(Context);
  const slug = useSearchParams().get("slug");
  const router = useRouter();
  const [form, setForm] = useState(blank),
    [version, setVersion] = useState(1),
    [ready, setReady] = useState(!id),
    [error, setError] = useState(""),
    [busy, setBusy] = useState(false);
  const dirty = useRef(false);
  useEffect(() => {
    let alive = true;
    if (!id) return;
    if (!slug) {
      setError("请从管理列表进入编辑");
      return;
    }
    api<Article>("/articles/" + encodeURIComponent(slug))
      .then((a) => {
        if (a.id !== id) throw new Error("文章地址不匹配");
        if (alive) {
          setForm({
            title: a.title,
            slug: a.slug,
            excerpt: a.excerpt,
            category: a.category,
            content: a.content,
          });
          setVersion(a.version);
          setReady(true);
        }
      })
      .catch((e) => {
        if (alive) setError(e.message);
      });
    return () => {
      alive = false;
    };
  }, [id, slug]);
  useEffect(() => {
    const listener = (e: BeforeUnloadEvent) => {
      if (dirty.current) e.preventDefault();
    };
    window.addEventListener("beforeunload", listener);
    return () => window.removeEventListener("beforeunload", listener);
  }, []);
  function change(key: keyof typeof blank, value: string) {
    dirty.current = true;
    setForm((f) => ({ ...f, [key]: value }));
  }
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
  if (loading) return <p>正在验证身份…</p>;
  if (session.user?.role !== "admin")
    return (
      <section className="card panel">
        <h1>仅管理员可编辑文章</h1>
        <Link href="/login">登录账号</Link>
      </section>
    );
  return (
    <>
      <Link href="/admin">返回管理列表</Link>
      <h1 className="page-title">{id ? "编辑文章" : "写一篇文章"}</h1>
      <ErrorMessage message={error} />
      {ready && (
        <form className="editor" onSubmit={save}>
          <div className="editor-fields">
            <label>
              文章标题
              <input
                className="input"
                value={form.title}
                required
                maxLength={160}
                onChange={(e) => change("title", e.target.value)}
              />
            </label>
            <label>
              地址标识（小写英文、数字、连字符）
              <input
                className="input"
                value={form.slug}
                required
                maxLength={120}
                pattern="[a-z0-9]+(-[a-z0-9]+)*"
                onChange={(e) => change("slug", e.target.value)}
              />
            </label>
            <label>
              分类
              <input
                className="input"
                value={form.category}
                required
                maxLength={40}
                onChange={(e) => change("category", e.target.value)}
              />
            </label>
            <label>
              摘要
              <textarea
                className="textarea"
                value={form.excerpt}
                maxLength={500}
                onChange={(e) => change("excerpt", e.target.value)}
              />
            </label>
          </div>
          <label className="file-import">
            导入 Markdown 文件
            <input
              className="file-input"
              type="file"
              accept=".md,.markdown,text/markdown,text/plain"
              onChange={async (e) => {
                const file = e.target.files?.[0];
                if (!file) return;
                if (file.size > 200000) {
                  setError("文件不能超过 200KB");
                  return;
                }
                change("content", await file.text());
                e.target.value = "";
              }}
            />
          </label>
          <div className="editor-columns">
            <label>
              Markdown 正文
              <textarea
                className="textarea code-editor"
                value={form.content}
                required
                maxLength={200000}
                onChange={(e) => change("content", e.target.value)}
                placeholder={"## 从这里开始\n\n记录值得分享的想法。"}
              />
            </label>
            <section className="preview">
              <h2>阅读预览</h2>
              <Markdown content={form.content || "正文预览将显示在这里。"} />
            </section>
          </div>
          <button className="btn btn-primary" disabled={busy || !session.csrf}>
            <Save size={16} aria-hidden="true" />
            {busy ? "正在保存…" : id ? "保存修改" : "发布文章"}
          </button>
          <p className="muted">离开前请保存。原始 HTML 不会执行。</p>
        </form>
      )}
    </>
  );
}
```
{% endraw %}

<a id="source-2"></a>

### src/types/post.ts

以下为该文件的完整源码。

{% raw %}
```typescript
// 联合类型：分类只能取下面三个值，拼错时编译器会提醒。
export type Category = "前端开发" | "后端探索" | "成长手记";

export interface Section {
  heading: string;
  paragraphs: string[];
  code?: string; // ? 表示这个字段可以省略。
}

export interface Post {
  slug: string;
  title: string;
  excerpt: string;
  category: Category;
  date: string;
  minutes: number;
  tags: string[];
  cover: "next" | "typescript" | "tailwind" | "database" | "express" | "notes";
  sections: Section[];
}
```
{% endraw %}

<a id="source-3"></a>

### src/lib/posts.ts

以下为该文件的完整源码。

{% raw %}
```typescript
import type { Post } from "@/types/post";

// 当前阶段的唯一数据源。未来可用 API 返回的数据替换。
export const posts: Post[] = [
  {
    slug: "nextjs-first-page",
    title: "从一张白纸开始，搭建我的第一个 Next.js 博客",
    excerpt: "理解路由、组件和渲染，把零散的知识串成一个真正能运行的小项目。",
    category: "前端开发",
    date: "2026-11-17",
    minutes: 8,
    tags: ["Next.js", "React", "入门"],
    cover: "next",
    sections: [
      {
        heading: "为什么从博客开始？",
        paragraphs: [
          "博客虽然小，却包含列表、详情、导航和数据展示这些常见需求。我们先用静态数据把页面搭起来，再逐步接入 API 和数据库。每一步都有可以看见的结果。",
          "React 负责把数据变成界面；Next.js 在 React 之上提供路由、构建和渲染能力。本项目使用 App Router，页面放在 src/app 目录下。",
        ],
      },
      {
        heading: "一个文件，就是一个页面",
        paragraphs: [
          "src/app/page.tsx 对应首页。组件是返回 JSX 的函数，JSX 看起来像 HTML，但允许你用花括号嵌入 JavaScript 表达式。",
        ],
        code: "export default function Home() {\n  return <h1>你好，我的博客</h1>;\n}",
      },
      {
        heading: "让文章拥有自己的地址",
        paragraphs: [
          "posts/[slug]/page.tsx 中的方括号表示动态路由。slug 是文章的可读标识，例如 nextjs-first-page。因为使用静态导出，我们通过 generateStaticParams 告诉 Next.js 需要提前生成哪些文章。",
          "本项目的文章正文在构建时生成 HTML。搜索和分类切换则由浏览器里的客户端组件完成。这两种方式可以在同一个页面中组合。",
        ],
        code: "export function generateStaticParams() {\n  return posts.map((post) => ({ slug: post.slug }));\n}",
      },
      {
        heading: "把第一步走扎实",
        paragraphs: [
          "试着修改文章标题，然后新增一条数据。你会发现首页列表和文章详情都跟着变化了：数据与展示分离，就是组件化带来的第一个好处。",
          "下一步阅读项目 docs 目录下的教学文档，跟着实际文件逐段理解，再尝试自己添加一个页面。",
        ],
      },
    ],
  },
  {
    slug: "typescript-data-model",
    title: "给数据一份说明书：TypeScript 入门",
    excerpt: "从一篇文章的数据结构出发，理解类型、接口与可选属性。",
    category: "前端开发",
    date: "2026-11-16",
    minutes: 6,
    tags: ["TypeScript", "类型系统"],
    cover: "typescript",
    sections: [
      {
        heading: "类型是开发时的约定",
        paragraphs: [
          "JavaScript 允许变量持有不同类型的值。TypeScript 为它增加静态类型检查，让你在运行之前发现常见错误，例如把字符串传给需要数字的地方。类型会在编译时被移除，不会替你校验网络返回的数据。",
        ],
        code: 'const minutes: number = 6;\nconst tags: string[] = ["TypeScript", "入门"];',
      },
      {
        heading: "用接口描述文章",
        paragraphs: [
          "Post 接口描述所有文章必须提供的字段。组件拿到 Post 后，编辑器就知道有哪些属性，也能提示拼写错误。Section 中的 code 是可选属性，因为不是每一节都有示例代码。",
        ],
        code: "interface Section {\n  heading: string;\n  paragraphs: string[];\n  code?: string;\n}",
      },
      {
        heading: "缩小可能性",
        paragraphs: [
          "Category 使用字符串联合类型限制分类。如果把前端开发写成前端开法，TypeScript 会直接报错。联合类型尤其适合分类、状态和固定选项。",
          "练习：给 Post 添加一个可选的 description 字段，并在卡片里处理它不存在的情况。",
        ],
      },
    ],
  },
  {
    slug: "tailwind-layout",
    title: "少写一点 CSS，多理解一点布局",
    excerpt: "用 Tailwind CSS 搭一张文章卡片，让响应式布局变得直观。",
    category: "前端开发",
    date: "2026-11-15",
    minutes: 5,
    tags: ["Tailwind CSS", "CSS"],
    cover: "tailwind",
    sections: [
      {
        heading: "从一个工具类开始",
        paragraphs: [
          "Tailwind 提供小而专一的工具类。p-6 控制内边距，rounded-2xl 控制圆角，flex 启用弹性布局。它没有替代 CSS 的原理，而是提供一种组合 CSS 的方式。",
        ],
        code: '<article className="rounded-2xl border p-6">\n  <h2 className="text-xl font-bold">文章标题</h2>\n</article>',
      },
      {
        heading: "从手机开始设计",
        paragraphs: [
          "grid-cols-1 是基础布局；md:grid-cols-2 只在中等及以上屏幕应用。先让小屏幕可读，再在更宽的空间增加列数。",
          "本项目的文章网格在手机上显示一列，在中等屏幕显示两列，宽屏下与右侧栏共同构成阅读布局。",
        ],
      },
      {
        heading: "交互状态也是样式",
        paragraphs: [
          "hover: 表示鼠标悬停状态，focus-visible: 适合键盘焦点反馈。不要仅凭颜色表达选中状态，还应使用 aria-pressed 等语义属性。",
          "练习：调整卡片圆角、间距和标题字号，然后观察不同宽度下的变化。",
        ],
      },
    ],
  },
  {
    slug: "database-first-look",
    title: "文章最后存在哪里？认识 PostgreSQL",
    excerpt: "从一张文章表开始，建立对数据库和持久化的第一印象。",
    category: "后端探索",
    date: "2026-11-14",
    minutes: 7,
    tags: ["PostgreSQL", "TypeORM"],
    cover: "database",
    sections: [
      {
        heading: "从内存到持久化",
        paragraphs: [
          "当前文章写在 TypeScript 文件中，修改后需要重新构建。数据库可以让系统在运行时保存内容，服务重启后数据依然存在。",
          "PostgreSQL 是关系型数据库。可以把文章理解为一张表，每篇文章是一行，标题、正文和发布时间是列。",
        ],
      },
      {
        heading: "TypeORM 的位置",
        paragraphs: [
          "TypeORM 帮助后端把 TypeScript 实体与数据库表建立映射。Repository 提供读取和保存实体的接口。数据库操作仍然发生在服务端，连接密码不应放进浏览器代码。",
          "当前 Demo 没有连接数据库。正式接入时，先设计表结构与迁移，再实现 Express API。",
        ],
      },
      {
        heading: "一步一步接入",
        paragraphs: [
          "先实现只读文章列表和详情接口，验证错误和空状态，再实现发布功能。发布涉及身份认证、输入校验和权限控制，不宜直接开放写入接口。",
        ],
      },
    ],
  },
  {
    slug: "express-request",
    title: "一次请求的旅程：浏览器到 Express",
    excerpt: "拆开请求与响应，看看前端和后端是如何对话的。",
    category: "后端探索",
    date: "2026-11-13",
    minutes: 6,
    tags: ["Express", "HTTP"],
    cover: "express",
    sections: [
      {
        heading: "前后端如何对话",
        paragraphs: [
          "浏览器通过 HTTP 请求访问后端接口，后端返回 JSON。前端读取这些数据，再用组件展示。路由 /api/posts 通常用于文章列表，/api/posts/:slug 用于单篇文章。",
        ],
      },
      {
        heading: "最小只读接口",
        paragraphs: [
          "下面是后续阶段的示意代码，并未集成进当前静态 Demo。Express 接收请求后，将查询结果作为 JSON 返回。",
        ],
        code: 'app.get("/api/posts", (_req, res) => {\n  res.json({ data: posts });\n});',
      },
      {
        heading: "别忘了失败的情况",
        paragraphs: [
          "请求可能失败，文章也可能不存在。前端需要加载中、空列表和错误状态；后端需要明确的状态码与统一的错误响应。",
          "本项目已经实现搜索无结果提示和文章不存在页面，可以用这些界面作为下一阶段的基础。",
        ],
      },
    ],
  },
  {
    slug: "intern-first-week",
    title: "实习第一周：把问题拆小，把进步记下来",
    excerpt: "不必一次掌握所有技术。让每一天都有一个可以验证的小目标。",
    category: "成长手记",
    date: "2026-11-12",
    minutes: 4,
    tags: ["实习", "学习方法"],
    cover: "notes",
    sections: [
      {
        heading: "先跑起来，再问为什么",
        paragraphs: [
          "刚接触新项目时，先找到启动命令、入口文件和数据来源。成功修改一个标题，就已经走通从源代码到页面的完整过程。",
        ],
      },
      {
        heading: "让问题更具体",
        paragraphs: [
          "遇到问题时，记录你期望的行为、实际结果、报错信息和尝试过的方法。具体的问题更容易得到帮助，也更容易被自己解决。",
        ],
      },
      {
        heading: "给每一天一个小目标",
        paragraphs: [
          "第一天学会组件和 props；第二天理解类型；第三天完成响应式卡片；第四天实现筛选；第五天复盘整个数据流。",
          "能解释自己写下的代码，比一次堆出很多功能更重要。",
        ],
      },
    ],
  },
];

export function getPost(slug: string): Post | undefined {
  return posts.find((post) => post.slug === slug);
}

export function formatDate(date: string): string {
  return date.replaceAll("-", ".");
}
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-11-pages %})

[下一篇]({% post_url 2026-11-20-intern-blog-13-theme %})
