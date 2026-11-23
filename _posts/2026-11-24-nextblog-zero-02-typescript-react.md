---
layout: post
title: 基础拆解（二）：TypeScript、React 与表单事件的写法
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 基础拆解（二）：TypeScript、React 与表单事件的写法：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 先分清 .ts、.tsx 与 tsx 命令

`.ts` 是 TypeScript 文件；`.tsx` 允许 TypeScript 里出现 JSX 标记；`tsx` 命令是本项目运行 server/*.ts 的工具。三个名称相似，职责不同。后端没有返回 `<button>` 这样的 JSX，所以使用 `.ts` 完全正常。浏览器不能直接拿服务器 TS 源文件执行；Next 会处理前端，tsx 工具负责本项目的后端开发/运行。`tsx` 执行不等于完成严格类型检查，仍需 `pnpm typecheck`。

`import type { Request } from "express"` 只导入类型；`import express from "express"` 导入运行时函数。前者在转译后不保留，后者运行时确实需要 node_modules 里的包。TypeScript 的 `interface User` 也不会自动建表，建表属于迁移。

## 2. 从项目中的类型读语法

`type Session = { user: User | null; csrf: string }`：Session 必须有 user、csrf；user 要么是对象，要么是 null。`role: "user" | "admin"` 限定两种字面量。`body?: unknown` 表示参数可省略；unknown 要经过检查才能安全使用。

`useState<Session>({ user: null, csrf: "" })` 用泛型指定状态结构。`api<Article>(...)` 表示希望结果按 Article 使用，**不会在运行时证明网络返回符合 Article**。真正检查外部输入的是后端 Zod。当前前端没有给每个响应加运行时 schema，应知道这个边界。

`(e as Error).message` 是告诉编译器如何看待 e，不是把任意值变成 Error。`req.account!.id` 的感叹号是非空断言，运行时不会帮你检查；它在当前写接口中依赖前面的 requireUser 已完成校验。把中间件删掉，断言不会保护接口。

`keyof typeof blank` 先获得编辑表单对象的类型，再得到其键名联合，所以 `change("title", "新标题")` 合法，`change("titlle", "新标题")` 会被类型检查发现拼写错误。

## 3. 函数组件并不是 HTML 文件

{% raw %}
```tsx
function Greeting({ name }: { name: string }) {
  return <h1>你好，{name}</h1>;
}
```
{% endraw %}

这是教学最小例子。组件接受 props 并返回 React 元素。`{name}` 表示在 JSX 中插入 JS 表达式。实际的 `ArticleList({ archive = false, manage = false })` 同时使用解构、默认参数与 props 类型，根据参数控制显示形式。

`items.map(a => <article key={a.id}>...</article>)` 把数组转换为元素数组。key 用于 React 区分列表项，不是传给 DOM 的业务字段；不要用标题当稳定唯一键。条件渲染 `session.user && ...` 在登录时显示内容，但它只是界面条件，不能替代服务端权限。

## 4. 事件究竟在哪里

| 实际写法 | 执行时机 | 返回什么 |
| --- | --- | --- |
| `onClick={logout}` | 点击退出 | 一个 Promise，React 不把它当作页面 |
| `onClick={() => remove(a)}` | 点击当前文章删除按钮 | 先用闭包捕获当前 a，再执行删除 |
| `onChange={(e) => setQ(e.target.value)}` | 搜索输入变化 | 更新 q 状态，引起渲染和 Effect |
| `onSubmit={submit}` | 表单有效时点击提交或按 Enter | 业务请求与状态更新 |
| `useEffect(() => ..., [page])` | 渲染提交后且 page 变化 | 可返回清理函数，不是点击事件 |

不能写 `onClick={logout()}`，它会在渲染时调用函数并把结果交给事件属性。`onClick={logout}` 是传函数，`onClick={() => logout()}` 是传一个稍后调用它的函数。[React 官方事件说明](https://react.dev/learn/responding-to-events)也采用“先定义函数，再将函数传入 JSX”的方式。

`e.target` 是产生事件的元素，`e.currentTarget` 是当前绑定处理器的元素。注册 submit 绑在 form 上，所以 `new FormData(e.currentTarget)` 读取整张表单。`e.preventDefault()` 取消浏览器默认跳转；它不会阻止其他监听器传播，和 stopPropagation 不同。

## 5. useState、useRef、useEffect 和 Context 各自解决什么

useState 的值属于本次渲染快照。`setPage(n => n + 1)` 根据最新值更新，适合前后页和 revision；直接改 `page++` 不会让 React 按预期更新。

useRef 提供跨渲染保存的可变容器。编辑器 dirty.current 表示有未保存修改；主题切换 inFlight.current 表示动画进行中；表单 formRef.current 指向真实 DOM。改变 ref.current 本身不会重新渲染，所以倒计时显示必须放在 State，不能只存在 ref。

useEffect 管理渲染后的外部同步，例如 fetch、事件监听器、计时器。清理函数清掉上一轮计时器或监听器；异步请求返回的 alive 标志避免旧响应覆盖当前状态，但它不等于取消网络请求。开发 Strict Mode 可能额外运行一次 Effect 的建立/清理，代码需要能正确清理。

Context 让 Header、评论、编辑器共享 Session 状态。它不负责将用户写入数据库，也不保证刷新后状态自动恢复。恢复是 SessionProvider.refresh 请求 `/auth/session` 的结果。三者分开记：React 状态在页面运行中，Cookie 在浏览器，Session 数据在服务器数据库。

## 6. 受控与非受控表单

ArticleList 搜索框用 `value={q}` 与 onChange，是受控输入；Editor 每个字段由 form 状态控制，才能即时 Markdown 预览。AuthForm 只在提交时读取 name 字段，主要是非受控输入，减少每次按键的状态同步。

两种都是正常 React 写法。需要实时预览或联动时选择受控；只在提交时收集值时可以用 FormData。确认密码的即时提交校验不要求把整张表单改为受控。若要“输入过程中就提示不一致”，可加 onChange 和状态，但仍必须保留后端校验。

练习：为什么 setRemaining 后文字变化，而 retryAt.current 改变后文字不直接变化？答案：State 请求重渲染，Ref 仅保存数据；定时器读取 Ref 并更新 State 才完成界面同步。

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-01-plan-build %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %})
