---
layout: post
title: 共享前端（九）：api、SessionProvider、Header 的每个方法
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 共享前端（九）：api、SessionProvider、Header 的每个方法：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 为什么先读这些公共方法

全站各页面都依赖 api 请求包装器；Header、评论、编辑器依赖 Context；注册/登录成功调用 setSession。先理解公共部分，就不会在每页重复猜 Cookie 从哪里来。本章对应 full-blog.tsx 开头直到 Markdown 前的部分。

## 2. api<T>(path, method="GET", body?, csrf?)

输入 path 是不带 `/api` 的路径；method默认GET；body是可选JSON数据；csrf是写请求令牌。方法先 fetch `/api`+path，设置 credentials="same-origin" 让同源 Cookie 随请求发送，cache="no-store" 不使用浏览器缓存。

headers 始终带 JSON Content-Type，只有传了csrf才添加 X-CSRF-Token；只有 body 为真值才序列化请求体。本项目传递的是对象，所以条件成立；若未来要发送 false/0/null 作为合法 JSON，应调整这里的判断，不能直接拿此包装器当任意请求客户端。

await fetch 不会因404/500自动reject，所以还要检查 r.ok。204没有内容，直接返回undefined as T；其他响应尝试 r.json，解析失败回退message。若响应非2xx，优先拼接details字段问题，其次message，最后通用错误；调用者的catch负责显示。

泛型T仅帮助后续代码获得类型提示；它不验证返回的JSON。不应把返回值未经验证就用于需要高可信数据的逻辑。

## 3. Context 默认值和 Provider

Context 包含 session/loading/error/refresh/setSession。默认 session.user=null、csrf为空，loading=true。默认refresh/setSession空函数仅是类型与未包裹组件的后备值，不会真的请求服务器。因此页面必须处在 RootLayout 的 SessionProvider 下。

Provider声明三组State：session存用户与csrf；loading表示初次身份查询未结束；error表示连接或API失败。孩子通过 useContext(Context) 订阅更新，不用一路传props。

## 4. refresh() 和首次挂载 Effect

refresh 调用 GET auth/session，成功 setSession 并清空错误；失败保留错误消息；finally设loading=false。返回Promise<void>，调用者可以await它，例如退出后等身份刷新完再更新提示。

挂载Effect依赖空数组，内部 `void refresh()` 表达“主动启动异步操作，不使用返回值”。刷新浏览器时React内存状态丢失，但浏览器仍带Cookie，服务器按sid找到userId，所以刷新请求可恢复用户。当前refresh不会在每次调用开始都把loading改回true，也没有定时后台刷新，这些是现状而不是隐含能力。

## 5. Header() 与 logout()

Header读session/loading，初次查询显示加载中；匿名显示登录注册链接；已登录显示用户名和退出；role=admin时显示管理入口。ThemeToggle是独立组件，不属于身份逻辑。

logout 的触发点是按钮 `onClick={logout}`。先 POST auth/logout 并附当前csrf，成功后 await refresh() 重新获得匿名Session和新的csrf，再清空Header错误。失败不假装退出，显示错误。只在前端setSession(null)不够，因为服务器旧Session仍可能有效，所以必须先调用销毁接口。

## 6. date(s) 与 ErrorMessage({ message })

date把服务器日期字符串转Date，再按zh-CN显示年月日；它只格式化不修改数据。服务端Date序列化成JSON字符串，前端Article.createdAt因此声明string。显示按浏览器时区，临近午夜可能与UTC日期不同。

ErrorMessage在message为空时返回null，否则返回role=alert的错误段落。它只负责展示，不捕获异常；每个async方法仍要catch并setError。集中组件可让注册、评论、列表统一样式和朗读行为。

## 7. 请求演练

退出时的链：点击Header退出 → logout → api(POST) → Express销毁Session/清Cookie →204 → refresh(GET) → 服务器产生匿名csrf → setSession → Header变成登录入口，Comments变成登录提示。

练习：移除await refresh会怎样？请求仍可能执行，但后续步骤不再等待身份恢复，页面反馈时序会更难判断。再练习断开API，观察error而不是误显示“密码错误”。

## 对照源码：src/components/full-blog.tsx（第 1–188 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 1 end: 188 sha256: 8d74ee9aedcd95d00998707e0cf6c2d4b40305c7f81286a7a1b5064d5efaffa7 -->

{% raw %}
````tsx
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-08-typeorm-crud %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %})
