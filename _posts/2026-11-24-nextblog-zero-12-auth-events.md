---
layout: post
title: 注册登录前端（十二）：逐个输入框、事件、计时器和请求追踪
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 注册登录前端（十二）：逐个输入框、事件、计时器和请求追踪：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 先在页面入口定位 AuthForm

login/page返回`<AuthForm />`，register/page返回`<AuthForm register />`。AuthForm的register默认false，所以同一函数通过布尔参数选择两种模式。注册显示昵称、邮箱、密码、确认密码、验证码；登录只保留邮箱密码。调用接口的路径也由register决定。

不要在page.tsx里找注册点击代码，它只是页面入口。真正的事件位于full-blog.tsx中的AuthForm。本章最后附完整AuthForm片段，可以按这里的方法名逐行对照。

## 2. Context、State、Ref逐个对应

| 名称 | 类型/来源 | 谁读取或改变 | 理由 |
| --- | --- | --- | --- |
| session | Context | sendCode/submit读取csrf | 写请求要携当前令牌 |
| setSession | Context方法 | 登录或注册成功调用 | Header/评论立即反映登录身份 |
| loading/connection/refresh | Context | 连接失败重试、禁用操作 | 身份和csrf未准备时不提交 |
| error | State string | catch和前端校验 | 表单错误，优先于connection显示 |
| busy | State boolean | submit前后 | 禁用提交/发送、显示等待 |
| sending | State boolean | sendCode前后 | 展示发送中、锁定邮箱编辑 |
| notice | State string | 发送成功、邮箱变化 | 展示发送提示，不用错误色 |
| remaining | State number | sendCode与定时器 | 页面显示剩余秒数 |
| formRef | Ref<HTMLFormElement> | form.ref、sendCode、邮箱onChange | 非受控表单字段的DOM访问 |
| sendingRef | Ref<boolean> | sendCode检查、设置、finally清理 | 在React重渲染前也能挡住连续调用 |
| retryAt | Ref<number> | 发送后写入、每秒读取 | 保存绝对截止毫秒时间 |

busy与sending分开，因为“验证邮件正在发送”和“账号正在注册”是不同操作。Ref不触发渲染，所以remaining不能只用Ref；State更新不是同步锁，所以仅靠sending也不如sendingRef明确。

## 3. 每个输入框的行为

nickname输入name=username，minLength=2，maxLength=40，required；邮箱type=email、max255，浏览器做基本格式检查；密码min10/max72，注册autocomplete=new-password，登录current-password。前端maxLength按字符计，后端还限制UTF-8最多72字节，中文等多字节密码可能通过字符检查却被后端拒绝，错误会返回给用户。

确认密码只在register模式渲染，name=confirmPassword，required；验证码name=verificationCode，type=text而非number，避免丢前导零；inputMode=numeric只是给手机键盘提示，真正格式约束是pattern="[0-9]{6}"、长度与后端Zod。

邮箱readOnly={sending||busy}，避免请求执行期间用户把目标邮箱改掉。邮箱onChange清notice，并通过formRef找到verificationCode输入将value清空；它不重置后端发送次数，也不证明新邮箱已验证。

## 4. useEffect：建立与清理倒计时

Effect依赖[register]。登录模式直接return，不创建定时器。注册模式每秒计算`Math.max(0, Math.ceil((retryAt.current-Date.now())/1000))`并setRemaining。Math.ceil让不足一秒仍显示1秒，Math.max不显示负数。

清理函数clearInterval，组件离开或模式改变时不会留下后台计时器。使用截止时间而非每秒固定减1，能在后台标签页计时器被延迟时校正剩余时间。刷新后前端倒计时会消失，服务器数据库仍拒绝冷却期内重发，前端倒计时只是体验。

## 5. sendCode()，按执行顺序读每一段

触发点：`button type="button" onClick={sendCode}`。按钮不能用默认submit，否则用户未填写密码时，浏览器会拦住发送验证码。

1. 若sendingRef.current为true或当前时间早于retryAt，return，避免重复执行。
2. 从formRef.current?.elements.namedItem("email")拿字段。问号处理ref尚为空的情况。
3. 用`instanceof HTMLInputElement`缩小DOM类型；调用email.reportValidity()，邮箱无效时浏览器直接提示并停止。这里不检查其他输入框。
4. 设置同步发送锁sendingRef=true，再setSending(true)、清error和notice。
5. 调api POST `/auth/register/code`，JSON只有email，令牌为session.csrf。密码不会随发送验证码请求传输。
6. 成功读取message/retryAfter；retryAt设为当前时间+秒数*1000，remaining立即设秒数，notice显示提示。
7. catch取业务错误，await refresh更新可能失效的Session/CSRF。刷新身份不表示邮件已发送，界面仍保留错误。
8. finally无论成功失败都清发送锁与sending状态，避免一次异常后按钮永久锁住。

disabled同时检查sending、busy、loading、csrf是否存在、remaining>0。后端返回429时页面会显示限流原因；当前失败响应没有新的retryAfter倒计时，不能声称前端自动准确知道小时配额何时恢复。

## 6. submit(e)：不是只有按按钮才触发

`form onSubmit={submit}`覆盖提交按钮和Enter。浏览器通常先执行required/pattern等原生约束，未通过时submit甚至不会被调用。通过后e.preventDefault取消整页跳转，FormData(e.currentTarget)按name读取值。

取password，再比较register模式下的confirmPassword。不同则setError，找到确认字段focus，然后return：既不设置busy，也不调用API。相同则清error并setBusy(true)。

请求体由公共email/password加注册专属username/confirmPassword/verificationCode组成。`...(register ? {...} : {})`是对象展开；登录不能带注册字段，因为后端credentials.strict拒绝额外字段。

await api返回Session后立即setSession，随后router.push("/")。若发生错误，catch显示错误并refresh；finally关闭busy。如果网络在后端已注册完成但响应回传前断开，可能显示失败但账号已存在；此时尝试登录，不应反复创建同一个邮箱。

## 7. JSX里其余事件和条件

连接错误时“重新连接”按钮onClick={refresh}；session.user存在时显示“已登录为…”并隐藏表单；error||connection选择最相关提示；发送结果p用role=status；底部Link在/login和/register之间导航，无需手写click。

提交按钮未指定type，表单内默认是submit。发送按钮明确type=button，因此两种按钮不会互相串业务。再强调一次：这里有完整React事件，Next.js并未要求去掉onClick/onSubmit。

## 8. 以一次失败再成功的注册手工调试

打开DevTools Network：邮箱空点发送，应没有POST；输入有效邮箱点发送，只出现/code请求，响应无验证码；填错确认密码后提交，应在前端停止；两次密码一致但验证码错误，会POST/register得到400；正确验证码返回201与新csrf，随后首页加载文章。

后端也要求confirmPassword相同，不能用修改页面绕过。验证码有效期/错误次数/重复使用都由后端决定。浏览器只负责更及时地提示，不决定用户是否真的可以注册。

## 对照源码：src/components/full-blog.tsx（第 429–634 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 429 end: 634 sha256: b745735d37c427bec3e47c8da5d22acc0b336e4260c28f5f1d7b9542939a7509 -->

{% raw %}
````tsx
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
  const [sending, setSending] = useState(false);
  const [notice, setNotice] = useState("");
  const [remaining, setRemaining] = useState(0);
  const formRef = useRef<HTMLFormElement>(null);
  const sendingRef = useRef(false);
  const retryAt = useRef(0);
  useEffect(() => {
    if (!register) return;
    const timer = window.setInterval(() => {
      setRemaining(Math.max(0, Math.ceil((retryAt.current - Date.now()) / 1000)));
    }, 1000);
    return () => window.clearInterval(timer);
  }, [register]);
  const router = useRouter();
  async function sendCode() {
    if (sendingRef.current || Date.now() < retryAt.current) return;
    const email = formRef.current?.elements.namedItem("email");
    if (!(email instanceof HTMLInputElement) || !email.reportValidity()) return;
    sendingRef.current = true;
    setSending(true);
    setError("");
    setNotice("");
    try {
      const result = await api<{ message: string; retryAfter: number }>(
        "/auth/register/code", "POST", { email: email.value }, session.csrf,
      );
      retryAt.current = Date.now() + result.retryAfter * 1000;
      setRemaining(result.retryAfter);
      setNotice(result.message);
    } catch (e) {
      setError((e as Error).message);
      await refresh();
    } finally {
      sendingRef.current = false;
      setSending(false);
    }
  }
  async function submit(e: FormEvent<HTMLFormElement>) {
    e.preventDefault();
    const f = new FormData(e.currentTarget);
    const password = f.get("password");
    // 前端即时提示；后端仍会独立校验，不能仅信任网页。
    if (register && password !== f.get("confirmPassword")) {
      setError("两次输入的密码不一致，请重新确认");
      const confirmation = e.currentTarget.elements.namedItem("confirmPassword");
      if (confirmation instanceof HTMLInputElement) confirmation.focus();
      return;
    }
    setError("");
    setBusy(true);
    try {
      setSession(
        await api<Session>(
          "/auth/" + (register ? "register" : "login"),
          "POST",
          {
            email: f.get("email"),
            password,
            ...(register ? {
              username: f.get("username"),
              confirmPassword: f.get("confirmPassword"),
              verificationCode: f.get("verificationCode"),
            } : {}),
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
        <form ref={formRef} onSubmit={submit}>
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
              readOnly={sending || busy}
              onChange={() => {
                setNotice("");
                const code = formRef.current?.elements.namedItem("verificationCode");
                if (code instanceof HTMLInputElement) code.value = "";
              }}
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
          {register && (
            <label>
              确认密码
              <input
                className="input"
                name="confirmPassword"
                type="password"
                minLength={10}
                maxLength={72}
                autoComplete="new-password"
                required
              />
            </label>
          )}
          {register && (
            <div className="grid gap-3">
              <label htmlFor="registration-code">邮箱验证码</label>
              <div className="flex flex-wrap gap-2">
                <input
                  className="input min-w-0 flex-1"
                  id="registration-code"
                  name="verificationCode"
                  type="text"
                  inputMode="numeric"
                  autoComplete="one-time-code"
                  pattern="[0-9]{6}"
                  minLength={6}
                  maxLength={6}
                  placeholder="6 位数字"
                  aria-describedby="registration-code-notice"
                  required
                />
                <button
                  className="btn btn-outline"
                  type="button"
                  onClick={sendCode}
                  disabled={sending || busy || loading || !session.csrf || remaining > 0}
                >
                  {sending ? "正在发送…" : remaining > 0 ? `${remaining} 秒后重发` : "发送验证码"}
                </button>
              </div>
              <p id="registration-code-notice" className="muted" role="status">
                {notice || "验证码 5 分钟内有效，请查收收件箱或垃圾邮件。"}
              </p>
            </div>
          )}
          <button
            className="btn btn-primary"
            disabled={busy || sending || loading || !session.csrf}
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-11-reader-comments %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-13-auth-server %})
