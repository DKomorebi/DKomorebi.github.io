---
layout: post
title: NextBlog（五）：React 状态、搜索请求与表单交互
date: 2026-11-17 09:04:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 理解 useState、useEffect、Context、异步竞态与请求状态。
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

理解 useState、useEffect、Context、异步竞态与请求状态。

> 本文是 NextBlog 全栈博客系列第 5 / 15 篇，内容对应当前全栈版本。

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

## 从一次输入理解状态更新

ArticleList 用 q 保存搜索词，用 category 保存分类，用 page 保存页码，用 items 和 total 保存接口结果。输入框的 value 来自 q，onChange 调用 setQ；这种由 React 状态决定输入值的方式称为受控输入。

修改搜索词或分类时同时 setPage(1)，因为原来第十页在新的筛选条件下可能不存在。不要先修改数组或 DOM 再试图让 React 猜出变化；应改变状态，让渲染从状态生成界面。

## useEffect 把状态变化连接到请求

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
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
```
{% endraw %}


依赖数组中的 q、category、page 或 revision 变化时，React 清理上次 effect，再启动本次 effect。200ms 定时器减少连续输入时的请求频率。清理函数取消尚未触发的定时器，并将本次闭包中的 alive 改为 false。

alive 解决的是响应先后顺序：旧搜索即使较晚返回，也不能覆盖新搜索。它不会中止已经发出的 HTTP 请求；如果需要节省请求传输，还应使用 AbortController。这段代码区分了“忽略过期结果”和“取消网络任务”。

encodeURIComponent 将用户输入编码到 URL 参数中；后端再用 Zod 检查长度，用参数绑定查询数据库。浏览器编码并不等于 SQL 注入防护，两个环节各自解决不同问题。

## loading、empty 和 error 是不同状态

busy 控制加载反馈，items 为空且没有错误时显示空列表，error 非空时显示错误与重试按钮。不要在请求尚未完成时立即显示“没有文章”，也不要把数据库错误解释成搜索结果为空。

revision 是重新加载的触发值。删除成功或点击重试时递增它，effect 因依赖改变而重新读取结果。分页按钮依据 total 和每页 10 条计算边界；当前排序由后端固定为创建时间倒序，没有独立的前端排序选择器。

## 多个页面如何共享登录状态

代码位置：`src/components/full-blog.tsx`（节选）。

{% raw %}
```tsx
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
```
{% endraw %}


SessionProvider 只负责读取和共享当前会话，Header、AuthForm、Comments 与 Editor 通过 useContext 获取状态。loading 表示首次验证尚未完成，error 表示连接失败；游客的 user 为 null，不等于网络请求失败。

登录或注册成功后，AuthForm 用 setSession 接收服务器返回的公开用户信息与新 CSRF token，再通过 router.push 返回首页。退出后 Header 调用 refresh 重新读取游客会话。Cookie 由浏览器管理，React 状态只是用于显示的当前快照。

## 表单提交为什么不直接刷新页面

AuthForm 在 onSubmit 中调用 preventDefault，阻止浏览器按传统表单导航；然后用 FormData 读取 email、password 和可选昵称，交给 api() 发送 JSON。Editor 则使用受控对象保存文章字段，因为它还需要预览和导入正文。

busy 在请求期间禁用提交，finally 恢复按钮。错误时保留输入并展示说明，不能先清空正文再发送请求。服务端会独立检查数据和权限，前端禁用按钮不能阻止他人直接调用接口。

## useRef 与 useState 的差异

Editor.dirty 和 ThemeToggle.inFlight 不直接决定一段文字内容，而是供事件处理读取的标志，使用 ref 保存。useRef 的 current 改变不会触发渲染；dark、busy 等会改变按钮外观的值则使用 state。

练习：将 Network 设置为较慢速度，快速输入两次不同关键词，观察较旧请求的结果不会覆盖最新结果。再临时停止自己启动的后端，检查“连接失败”和“没有文章”是否能被区分，恢复后使用重试。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})

[下一篇]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
