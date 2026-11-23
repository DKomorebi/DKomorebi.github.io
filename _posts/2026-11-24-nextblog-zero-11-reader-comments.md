---
layout: post
title: 阅读与评论（十一）：ArticleReader、Markdown、Comments 的每个方法
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 阅读与评论（十一）：ArticleReader、Markdown、Comments 的每个方法：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. ArticleReader({ slug }) 的输入与状态

路由Page把URL的slug传进来；article初始null，error为空，revision=0。Effect依赖slug和revision，开始时setArticle(null)清掉上一文章，调用GET articles/编码后的slug。成功写article并清错误，失败写error；清理设alive=false，避免旧文章响应覆盖新页面。

重试按钮只revision+1，Effect会重走请求。文章未返回且无错误时显示加载；有错误显示ErrorMessage；成功后展示标题、分类、日期、摘要，再把article.content交给Markdown，把article.id交给Comments。拿slug去请求评论会因UUID校验失败，这正是slug与id需要分清的地方。

## 2. Markdown({ content }) 及两个渲染回调

ReactMarkdown接收字符串作为children，把Markdown转成React元素；remarkGfm支持GFM语法，如表格和任务列表；rehypeHighlight为代码块加高亮标记。`skipHtml`禁止把正文原始HTML直接当页面HTML执行。

components.a回调接收children和href，返回链接并加nofollow/noopener/noreferrer。它没有强制target=_blank，所以不能在文档中写“所有链接新窗口打开”。

components.img回调先判断src是字符串且匹配http(s)或单斜杠根相对路径；有效时显示lazy图片并设置no-referrer，否则显示“不支持”。`/(?!/)`排除以双斜杠开头的协议相对地址；这不是上传功能，文件仍来自给定地址。

Markdown只是展示，不写数据库，不负责图片存储。不要为了“支持所有HTML”直接换成dangerouslySetInnerHTML；本项目阅读和编辑预览共用同一限制。

## 3. Comments 的全部状态

items=当前页评论，total=评论总数，page=1起，content=受控输入文本，error=反馈，busy=发表中，revision=刷新编号。Context给session。Effect依赖articleId/page/revision，GET评论列表后写items/total并清error；清理用alive防过期响应。

当前初始空列表和请求中的空列表没有独立loading状态，所以加载早期可能短暂显示“还没有留言”；这不是数据库肯定没有评论的证明。评论按新到旧排序，每页30。

## 4. textarea.onChange 与 submit(e)

textarea的value来自content；每次输入setContent(e.target.value)。按钮disabled条件为busy或!content.trim()，防止只输入空格；后端仍再次trim并限制1..2000字符，浏览器maxLength不是安全边界。

submit由form.onSubmit触发：preventDefault；setBusy(true)；POST `/articles/id/comments`，body只有content，csrf来自session；成功清空输入、归第一页、revision+1；catch写error；finally解除busy。返回的评论对象当前没直接插入列表，而是重新获取，保证作者名和列表结构一致。

匿名用户不渲染表单，只显示登录注册链接；不过有人手工POST仍由requireUser返回401，不能只靠隐藏表单。

## 5. remove(id) 与删除按钮

用户确认后DELETE comments/id，成功归第一页并revision+1，失败保留并显示错误。按钮仅当自己是作者或admin时出现。每个评论的onClick闭包传c.id；评论文字用普通JSX文本和whitespace-pre-wrap保留换行，不调用Markdown，也不执行HTML。

后端再次判断同样权限。客户端控件是交互提示，服务端检查是授权执行。两者都写，但不要混淆其安全职责。

## 6. 评论分页与作者渲染

items.map产生文章块，用comment.id作为key；显示username和格式化日期。上一页回调递减，page=1禁用；下一页递增，page*30>=total禁用；只有total>30才显示分页。与文章列表不同，当前评论分页没有busy请求禁用，所以快速连点可能发多个请求，alive确保过期响应不覆盖最新页。

练习：发表评论成功后为什么同时setPage(1)和setRevision？如果已经在第一页，setPage(1)不会改变依赖；revision保证仍然重新加载。若在第二页发表，新评论按最新排序应出现在第一页。

## 7. 一次评论请求的全链追踪

按下发表 → React submit拿content → api携Cookie/CSRF → Next转发 → Express检查Origin、Session、CSRF → requireUser → 限流与正文校验 → TypeORM确认文章存在并save →201 → 前端清空输入 → Effect GET列表 → 查询作者名 → React渲染新评论。

出问题按链路定位：没请求看浏览器必填/disabled；403看Origin/CSRF；401看Session；400看正文；404看文章id；500看服务端及数据库。不要一看到失败就重写整个组件。

## 对照源码：src/components/full-blog.tsx（第 189–219 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 189 end: 219 sha256: 5b5770d7d44c3bfecc66d09bbb43e0ebcb985f45de2daa5050c84b2861ffc111 -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：src/components/full-blog.tsx（第 635–823 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 635 end: 823 sha256: cfbac2f436dbc60730b6a3b3a07da0df3e886eb709d65f0f6d60c10988d358ea -->

{% raw %}
````tsx
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-10-list-pages %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-12-auth-events %})
