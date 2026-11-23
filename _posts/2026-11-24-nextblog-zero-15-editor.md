---
layout: post
title: 管理编辑（十五）：加载、change、save、Markdown 导入与离开提示
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 管理编辑（十五）：加载、change、save、Markdown 导入与离开提示：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 两个入口共用 Editor({ id? })

新建页不传id，使用blank默认对象；编辑页传路径id并带query.slug。Editor读取Context确定身份，useRouter负责保存后导航，useSearchParams读取slug。页面权限提示只是体验，POST/PUT仍由后端requireAdmin保护。

form状态保存title/slug/excerpt/category/content；version初始1，编辑加载时换成服务器当前版本；ready初始!id，新建立即可输入，编辑要等加载；error/busy用于反馈。dirty用Ref保存是否有未保存修改，因为它不需要直接显示在页面。

## 2. 加载Effect的每个分支

依赖[id,slug]。无id立即返回，说明新建；有id但无slug则提示“从管理列表进入编辑”；否则按slugGET文章。返回后首先验证a.id===id，防止路径id与query.slug不对应。

alive为true时逐个字段复制到form，同时保存a.version，设ready=true。显式挑字段避免把createdAt/id等元数据塞入编辑请求。catch写错误，清理alive=false避免旧加载覆盖新页面。

当前ready只在初始值与成功加载处设置，没有在每次id变更时先reset，dirty也没有在所有导航路径上重置。正常从管理列表进入和保存后跳转能工作；扩展“编辑器内直接切换文章”时应额外处理这个状态生命周期，不能宣称已覆盖任意切换。

## 3. change(key,value)

输入key受`keyof typeof blank`限制，value为字符串。先dirty.current=true，再`setForm(f=>({...f,[key]:value}))`。展开保留其他字段，[key]是计算属性名。若写成setForm({title:value})，其余字段会丢失且类型不匹配。

title/slug/category三个input、excerpt/content两个textarea的onChange都调用它。各字段并非各写一套业务方法，而是通过key复用统一更新函数；表单状态改变使右侧Markdown即时重新渲染。

## 4. beforeunload Effect与listener(e)

挂载时注册window beforeunload；监听器只在dirty.current为true时preventDefault；卸载移除同一函数引用。它主要覆盖刷新、关闭标签页等离开文档行为，具体提示由浏览器决定，不能自定义任意提示文案。

它不是完整的Next客户端路由导航拦截器，点击站内Link可能不会出现离开提示。文档与功能说明必须明确这个边界，不能把一段beforeunload称为“保证永不丢失编辑”。

## 5. 文件输入onChange：为什么它是async

从e.target.files?.[0]取第一个文件，没选择直接return；超过200000字节设错误并return；否则await file.text()读取文本，change("content",文本)写入表单，最后把input.value清空，便于再次选同一个文件触发change。

accept只影响文件选择器建议，不是服务器上传或安全验证。此处不会将原文件发到后端，只读取本地文本；真正保存是在点击发布时，把Markdown字符串放JSON。正文maxLength是字符数，文件size是字节数，HTTP又有256kb请求上限，三者不能当作同一个单位。非ASCII正文较大时可能先触发413。

当前文件读取没有独立try/catch，因此罕见的读文件失败需要进一步完善错误反馈；不要把未来改进写成当前已处理。

## 6. save(e) 的请求选择

onSubmit触发，preventDefault，busy=true。根据id选择`/articles/id`+PUT或`/articles`+POST；body展开form，只有修改时附version，csrf取Session。await api<Article>拿到数据库结果，dirty=false，再router.push到返回的slug。

创建不附version，因为后端创建schema是strict，乱加字段会400。修改必须附加载时的version，后端条件更新才知道是否有人已经改过。出现409时catch显示错误并保留用户输入；当前不会自动合并差异，用户应先复制自己的修改，再重新加载比较，避免直接刷新丢失内容。

finally解除busy；保存按钮在busy或缺csrf时禁用。按钮文字随模式变“发布文章/保存修改”。成功跳转用服务端返回slug，避免前端预测与实际保存结果不一致。

## 7. 管理列表、编辑页、阅读页的闭环

管理卡片点击编辑 → Link携id与slug → Editor加载并核对 → change标脏 → save发送PUT+version → 后端原子更新 → 返回新文章 → router进入阅读页 → ArticleReader重新GET。删除则留在管理列表，用revision刷新，不走编辑器。

练习：同时开两个标签编辑同一篇文章，先保存A再保存B。B收到409才是正确行为；如果B覆盖A，检查后端where条件和前端发送的version，而不是先去改CSS。

## 对照源码：src/components/full-blog.tsx（第 824–1000 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 824 end: 1000 sha256: 77c436087bbbb754a771fecdbfa3025de4a794a7404a448eb9a9de9d3f2a0a64 -->

{% raw %}
````tsx
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-14-email-transaction %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-16-ddd %})
