---
layout: post
title: 列表页面（十）：首页、归档、管理列表的所有状态和事件
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 列表页面（十）：首页、归档、管理列表的所有状态和事件：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 三个页面共用 ArticleList 的理由

Home返回默认ArticleList，Archive传archive，管理页传manage。获取文章、搜索分类、分页和删除后刷新是相同数据逻辑，区别只在外观与可见操作。组件参数默认false；首页显示HomeHero，归档隐藏摘要，管理显示新增/编辑/删除按钮。

## 2. 全部状态先对应具体问题

| 状态 | 初始值 | 谁修改 | 为什么需要 |
| --- | --- | --- | --- |
| items | [] | 列表请求成功 | 当前页文章 |
| q | 空字符串 | 搜索onChange、清空筛选 | 请求关键词 |
| category | 空字符串 | select onChange、清空筛选 | 分类条件 |
| categories | [] | 分类请求 | 下拉框选项 |
| page | 1 | 上下页、过滤变化、删除 | 请求页码 |
| total | 0 | 列表响应 | 总页数与是否显示分页 |
| error | 空 | catch/成功清空 | 展示失败及重试 |
| busy | true | 列表请求前后 | 读屏加载提示、禁用分页 |
| revision | 0 | 删除、重试 | 参数没变时主动刷新 |

State描述当前界面，不是永久数据库。比如删除后直接setItems过滤数组只能改变画面，真正删除必须先成功调用DELETE。

## 3. 第一个 useEffect：200ms等待与旧响应保护

依赖[q,category,page,revision]中的任何值变化，就为当前渲染建立alive=true和timer。200ms后设busy=true并请求拼接好的URL，q/category使用encodeURIComponent，不让空格、中文、&混入URL结构。

then判断alive再写items/total并清error；catch只在alive时写错误；finally只在alive时结束busy。清理函数将alive=false并清timer。如果200ms内继续输入，上一轮timer被取消，实现防抖；如果上轮请求已经发出，清理不会取消fetch，但会阻止其结果覆盖新搜索。

例：先搜React，再很快搜TypeORM。假设React响应更晚返回，alive守卫让它不能把TypeORM卡片覆盖掉。这是异步竞态保护，不能理解成固定延迟使网络按顺序返回。

## 4. 第二个 useEffect：分类选项

依赖只有revision：首次和删除/手动刷新后GET categories，then直接setCategories。catch当前是空处理，表示分类加载失败不会覆盖主列表错误；代价是用户可能只看到“全部分类”而不知道选项加载失败。后续可以给分类单独错误提示，但本文不虚构已实现。

## 5. 搜索、分类、清空、重试逐个事件

搜索onChange读取e.target.value，setQ后setPage(1)，避免在原来第十页搜索导致看不到其实存在的第一页结果。分类onChange同理更新category并归1。

“清空筛选”onClick把q、category清空并归1；它不直接请求，状态变化让Effect执行。“重新加载”只让revision+1，保留用户筛选条件。若直接写setPage(1)而当前已经1，React不会因此保证产生新请求，所以revision单独存在。

## 6. remove(a) 删除方法

输入完整Article是为了确认框显示标题，真正请求只使用id。confirm返回false立即结束；确认后await api(DELETE,csrf)；成功归page=1并revision+1，重新取列表和分类；失败setError。它不在请求完成前乐观移除卡片，因此错误时仍可看到原文章。

点击绑定为 `onClick={() => remove(a)}`，闭包选中当前卡片，而不是渲染时执行remove。删除权限由后端检查；在控制台改session.user.role不能让API绕过403。

## 7. map 回调和分页回调

categories.map产生option，key用分类名；items.map产生article，key用UUID。文章标题Link到`/posts/slug`，编辑Link到`/admin/id?slug=...`，构成下一页的输入。归档的className换成archive-list且不输出excerpt。

上一页setPage(n=>n-1)，在busy或page=1禁用；下一页setPage(n=>n+1)，在busy或page*10>=total禁用。总页数Math.max(1,Math.ceil(total/10))避免显示0页。接口每页10和前端计算必须一起维护。

## 8. 页面渲染分支与限制

manage且loading先提示验证；manage且非admin返回权限提示。但Hooks在这些return之前，所以管理页等待身份时仍可能请求公开文章列表，这不泄露私有数据，不能宣称前端完全不发请求。

非忙碌、无错误且items空时显示空态；发生错误隐藏卡片并显示重试；加载时保留已有内容并向读屏报告状态。HomeHero只有默认首页显示，它的“开始阅读”是href="#articles"的原生页内锚点，不需要JS点击方法。

验收：连续输入、换分类后页码归1、最后页删文后回第一页、网络失败重试保留筛选、普通用户直接访问/admin不能写入。

## 对照源码：src/components/full-blog.tsx（第 220–428 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 220 end: 428 sha256: 95989d61879fecf887585399f128b3aaec0cf81fff1bbc27f1353e40f94a91ba -->

{% raw %}
````tsx
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-09-session-ui %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %})
