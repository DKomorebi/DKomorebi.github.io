---
layout: post
title: NextBlog（十三）：DaisyUI 卡片、Lucide 图标与深色主题
date: 2026-11-20 00:13:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 双主题语义色、主题持久化、圆形切换过渡与无障碍降级。
keywords: Next.js, TypeScript, Express, PostgreSQL, DaisyUI, 全栈博客
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
last_modified_at: 2026-11-20 07:31:36 +0800
---

双主题语义色、主题持久化、圆形切换过渡与无障碍降级。

> 本文是 NextBlog 全栈博客系列第 13 / 15 篇，内容对应当前全栈版本。

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

## 组件库负责基础控件，项目负责排版

DaisyUI 5 通过 Tailwind CSS 4 的 @plugin 加载。btn、input、textarea、select、file-input、alert、navbar、card、badge 负责基础组件；globals.css 控制阅读宽度、留白、文章卡片和主题色。卡片恢复独立背景、细边框和轻微悬停反馈，首页仍保留两行 Hero 与渐显。

图标库是 lucide-react。导航、搜索、分类、日期、编辑、删除、保存和评论使用语义匹配的图标。辅助图标使用 aria-hidden，按钮保留文字；主题按钮没有可见文字，因此必须提供明确的 aria-label。

## 两套主题共享语义

浅色 fieldnotes 与深色 fieldnotes-dark 使用相同的语义颜色变量。背景、卡片、正文、次要文字和边框都应跟随变量变化，而不是只将 body 改成黑色。代码块、表单、错误提示和弹出控件同样需要检查对比度。

## 首帧与偏好

theme.ts 中的初始化脚本在 head 执行：先读 localStorage，没有有效保存值则读取 prefers-color-scheme。这样深色用户刷新时不会先看到浅色页面。存储只包含主题名称，没有身份或密码。localStorage 不可用时仍能切换当前页。

ThemeToggle 监听系统偏好和 storage 事件，支持跨标签同步；用户明确选择后，该选择优先于系统。首屏脚本和客户端组件使用同一组常量，避免主题名称不一致。

## 圆形展开动画

调用 document.startViewTransition 在切换前后捕获页面快照，对新快照的 clipPath 从零半径扩展到覆盖整个视口。点击时使用鼠标坐标，键盘激活时使用按钮中心。inFlight 防止快速重复点击产生重叠过渡。

API 不可用、用户开启减少动态效果或浏览器中断动画时，都直接完成主题切换。动画不能成为按钮是否可用的前提。Hero 同样使用 prefers-reduced-motion 关闭渐显。

官方参考：[DaisyUI 主题](https://daisyui.com/docs/themes/)、[View Transition](https://developer.mozilla.org/en-US/docs/Web/API/Document/startViewTransition)。

## 练习

设置系统深色偏好后清空主题存储，刷新并检查首帧。随后手动切浅色，再刷新，确认手动选择优先。用键盘 Tab / Enter 切换，再开启减少动态效果测试即时切换。


## 本篇文件索引

- [src/components/home-hero.tsx](#source-1)
- [src/components/theme-toggle.tsx](#source-2)
- [src/lib/theme.ts](#source-3)
- [src/app/globals.css](#source-4)

<a id="source-1"></a>

### src/components/home-hero.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
/** 纯 CSS 入场动效，无需额外动画库；减少动态效果时直接显示完整文字。 */
export function HomeHero() {
  return (
    <section className="hero home-hero" aria-labelledby="home-title">
      <div className="hero-content home-hero-content">
        <h1 id="home-title" aria-label="落笔成文，拾页成光。">
          <span className="hero-line">
            <span>落笔成文，</span>
          </span>
          <span className="hero-line">
            <span>
              拾页<span className="hero-accent">成光。</span>
            </span>
          </span>
        </h1>
        <p className="hero-description">记录技术与生活。</p>
        <a className="btn btn-outline hero-read" href="#articles">
          开始阅读
        </a>
      </div>
    </section>
  );
}
```
{% endraw %}

<a id="source-2"></a>

### src/components/theme-toggle.tsx

以下为该文件的完整源码。

{% raw %}
```tsx
"use client";
import { useEffect, useRef, useState, type MouseEvent } from "react";
import { Sun, Moon } from "lucide-react";
import { THEME_KEY, LIGHT_THEME, DARK_THEME } from "@/lib/theme";

export function ThemeToggle() {
  const [dark, setDark] = useState(false),
    [busy, setBusy] = useState(false);
  const inFlight = useRef(false);
  useEffect(() => {
    const media = matchMedia("(prefers-color-scheme: dark)");
    const apply = (theme: string) => {
      document.documentElement.dataset.theme = theme;
      setDark(theme === DARK_THEME);
    };
    const sync = () => {
      let saved: string | null = null;
      try {
        saved = localStorage.getItem(THEME_KEY);
      } catch {}
      apply(
        saved === LIGHT_THEME || saved === DARK_THEME
          ? saved
          : media.matches
            ? DARK_THEME
            : LIGHT_THEME,
      );
    };
    sync();
    media.addEventListener("change", sync);
    const storage = (e: StorageEvent) => {
      if (e.key === THEME_KEY || e.key === null) sync();
    };
    window.addEventListener("storage", storage);
    return () => {
      media.removeEventListener("change", sync);
      window.removeEventListener("storage", storage);
    };
  }, []);
  async function toggle(event: MouseEvent<HTMLButtonElement>) {
    if (inFlight.current) return;
    const root = document.documentElement;
    const next = root.dataset.theme === DARK_THEME ? LIGHT_THEME : DARK_THEME;
    const apply = () => {
      root.dataset.theme = next;
      setDark(next === DARK_THEME);
      try {
        localStorage.setItem(THEME_KEY, next);
      } catch {}
    };
    if (
      !document.startViewTransition ||
      matchMedia("(prefers-reduced-motion: reduce)").matches
    ) {
      apply();
      return;
    }
    const rect = event.currentTarget.getBoundingClientRect();
    const x = event.detail === 0 ? rect.left + rect.width / 2 : event.clientX;
    const y = event.detail === 0 ? rect.top + rect.height / 2 : event.clientY;
    const radius = Math.hypot(
      Math.max(x, innerWidth - x),
      Math.max(y, innerHeight - y),
    );
    inFlight.current = true;
    setBusy(true);
    try {
      const transition = document.startViewTransition(apply);
      // 页面隐藏或其他过渡抢占时 finished 也可能拒绝，及时处理。
      const finished = transition.finished.catch(() => {});
      await transition.ready;
      await root.animate(
        {
          clipPath: [
            `circle(0px at ${x}px ${y}px)`,
            `circle(${radius}px at ${x}px ${y}px)`,
          ],
        },
        {
          duration: 580,
          easing: "cubic-bezier(.4,0,.2,1)",
          pseudoElement: "::view-transition-new(root)",
        },
      ).finished;
      await finished;
    } catch {
      apply();
    } finally {
      inFlight.current = false;
      setBusy(false);
    }
  }
  return (
    <button
      type="button"
      className="btn btn-ghost btn-circle theme-toggle"
      aria-label={dark ? "切换浅色主题" : "切换深色主题"}
      aria-pressed={dark}
      disabled={busy}
      onClick={toggle}
    >
      {dark ? (
        <Sun size={20} aria-hidden="true" />
      ) : (
        <Moon size={20} aria-hidden="true" />
      )}
    </button>
  );
}
```
{% endraw %}

<a id="source-3"></a>

### src/lib/theme.ts

以下为该文件的完整源码。

{% raw %}
```typescript
export const THEME_KEY = "fieldnotes-theme";
export const LIGHT_THEME = "fieldnotes";
export const DARK_THEME = "fieldnotes-dark";
// 在首帧绘制前应用偏好，避免刷新深色页面时先闪现浅色背景。
// 只包含项目常量，不插入用户输入。
export const themeInitScript = `(()=>{let theme;try{theme=localStorage.getItem('${THEME_KEY}')}catch{}if(theme!=='${LIGHT_THEME}'&&theme!=='${DARK_THEME}')theme=matchMedia('(prefers-color-scheme: dark)').matches?'${DARK_THEME}':'${LIGHT_THEME}';document.documentElement.dataset.theme=theme})()`;
```
{% endraw %}

<a id="source-4"></a>

### src/app/globals.css

以下为该文件的完整源码。

{% raw %}
```css
@import "tailwindcss";
@plugin "daisyui" {
  themes: false;
  logs: false;
}
@plugin "daisyui/theme" {
  name: "fieldnotes";
  default: true;
  color-scheme: light;
  --color-base-100: #faf9f6;
  --color-base-200: #efeee8;
  --color-base-300: #dedfd7;
  --color-base-content: #252c2a;
  --color-primary: #2f7068;
  --color-primary-content: #ffffff;
  --color-secondary: #62756d;
  --color-secondary-content: #ffffff;
  --color-accent: #2f7068;
  --color-accent-content: #ffffff;
  --color-neutral: #283a35;
  --color-neutral-content: #ffffff;
  --color-info: #326f93;
  --color-info-content: #ffffff;
  --color-success: #357257;
  --color-success-content: #ffffff;
  --color-warning: #d8ac53;
  --color-warning-content: #252c2a;
  --color-error: #a33131;
  --color-error-content: #ffffff;
  --radius-selector: 0.5rem;
  --radius-field: 0.65rem;
  --radius-box: 1rem;
  --size-selector: 0.25rem;
  --size-field: 0.25rem;
  --border: 1px;
  --depth: 0;
  --noise: 0;
}
@plugin "daisyui/theme" {
  name: "fieldnotes-dark";
  default: false;
  color-scheme: dark;
  --color-base-100: #111a19;
  --color-base-200: #192423;
  --color-base-300: #30403c;
  --color-base-content: #e3eae5;
  --color-primary: #83c5b3;
  --color-primary-content: #10231d;
  --color-secondary: #a4bcb1;
  --color-secondary-content: #10231d;
  --color-accent: #83c5b3;
  --color-accent-content: #10231d;
  --color-neutral: #cadbd3;
  --color-neutral-content: #10231d;
  --color-info: #82b9dd;
  --color-info-content: #10231d;
  --color-success: #8cc9a3;
  --color-success-content: #10231d;
  --color-warning: #d8ac53;
  --color-warning-content: #e3eae5;
  --color-error: #f0a4a4;
  --color-error-content: #10231d;
  --radius-selector: 0.5rem;
  --radius-field: 0.65rem;
  --radius-box: 1rem;
  --size-selector: 0.25rem;
  --size-field: 0.25rem;
  --border: 1px;
  --depth: 0;
  --noise: 0;
}
@theme {
  --color-paper: #faf9f6;
  --color-ink: #252c2a;
  --color-muted: #626b67;
  --color-line: #dedfd7;
  --font-sans: Arial, "Microsoft YaHei", sans-serif;
}
:root {
  --serif: "Noto Serif SC", "Songti SC", "STSong", serif;
  --bg: var(--color-base-100);
  --ink: var(--color-base-content);
  --accent: var(--color-primary);
  --line: var(--color-base-300);
}
body {
  margin: 0;
  background: var(--bg);
  color: var(--ink);
  font:
    16px/1.85 Arial,
    "Microsoft YaHei",
    sans-serif;
  -webkit-font-smoothing: antialiased;
}
a {
  color: var(--accent);
  text-decoration: none;
}
a:hover {
  text-decoration: underline;
}
.btn:hover {
  text-decoration: none;
}
:focus-visible {
  outline: 2px solid var(--accent);
  outline-offset: 4px;
}
h1,
h2,
h3 {
  font-family: var(--serif);
  font-weight: 600;
  line-height: 1.5;
}
h1 {
  font-size: clamp(28px, 4vw, 40px);
}
h2 {
  font-size: 24px;
}
.site-header {
  background: var(--bg);
}
.header-inner {
  max-width: 1120px;
  margin: auto;
  padding: 28px 32px;
  justify-content: space-between;
  gap: 24px;
  min-height: 100px;
}
.brand {
  font-family: var(--serif);
  font-size: 30px;
  color: var(--ink);
  letter-spacing: 0.12em;
}
.brand:hover {
  text-decoration: none;
}
nav {
  display: flex;
  align-items: center;
  gap: 28px;
  font-size: 14px;
  flex-wrap: wrap;
}
nav > a {
  color: var(--muted);
}
.username {
  max-width: 110px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
.site-main {
  max-width: 1040px;
  min-height: 72vh;
  margin: auto;
  padding: 0 30px 70px;
}
.home-hero {
  min-height: 570px;
  place-items: center start;
  position: relative;
  isolation: isolate;
  padding: 64px 0 96px;
}
.home-hero:before {
  content: "";
  position: absolute;
  width: min(640px, 95vw);
  height: 440px;
  left: -160px;
  top: -70px;
  background: radial-gradient(ellipse, #2f70680b, transparent 68%);
  z-index: -1;
  pointer-events: none;
}
.home-hero-content {
  max-width: none;
  display: block;
  padding: 0;
  text-align: left;
}
.home-hero h1 {
  font-size: clamp(42px, 5.6vw, 76px);
  line-height: 1.35;
  letter-spacing: 0.035em;
  margin: 0;
  font-weight: 500;
}
.hero-line {
  display: block;
  overflow: hidden;
  padding-bottom: 0.08em;
}
.hero-line > span {
  display: block;
  animation: hero-reveal 1.1s cubic-bezier(0.2, 0.7, 0.25, 1) both;
}
.hero-line:nth-child(2) > span {
  animation-delay: 160ms;
}
.hero-accent {
  color: var(--accent);
}
.hero-description {
  font-size: 17px;
  color: var(--muted);
  margin: 30px 0 0;
  animation: hero-reveal 1s 320ms both;
}
.hero-read {
  margin-top: 32px;
  min-height: 42px;
  padding-inline: 22px;
  font-weight: 400;
  animation: hero-reveal 1s 420ms both;
  border-color: #abbab0;
  color: var(--accent);
}
@keyframes hero-reveal {
  from {
    opacity: 0;
    transform: translateY(24px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}
.intro {
  padding: 60px 0 44px;
}
.intro h1 {
  margin: 0 0 24px;
}
.section-heading {
  scroll-margin-top: 24px;
  margin-bottom: 24px;
}
.section-heading h2 {
  font-size: 24px;
  margin: 0;
}
.list-toolbar {
  display: flex;
  gap: 14px;
  margin-bottom: 30px;
  align-items: center;
}
.search {
  flex: 1;
}
.list-toolbar .input {
  margin: 0;
  width: 100%;
  background: transparent;
}
.list-toolbar .select {
  min-width: 140px;
  background-color: transparent;
}
.input,
.textarea,
.select,
.file-input {
  width: 100%;
  font-size: 15px;
}
.input,
.select {
  height: 46px;
}
.textarea {
  min-height: 110px;
}
.file-input {
  height: auto;
  min-height: 46px;
}
.btn {
  font-weight: 500;
  letter-spacing: 0;
}
.btn:disabled {
  cursor: not-allowed;
}
label {
  display: block;
  font-size: 14px;
}
label .input,
label .textarea {
  margin-top: 7px;
}
.muted {
  color: var(--muted);
  font-size: 14px;
}
.article-list {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 24px;
}
.article-card {
  padding: 28px;
  border-radius: var(--radius-box);
  border: 1px solid var(--line);
  box-shadow: none;
  background: var(--surface);
  gap: 0;
  transition:
    border-color 0.2s,
    transform 0.2s,
    box-shadow 0.2s;
}
.article-card:hover {
  border-color: var(--color-primary);
  transform: translateY(-3px);
  box-shadow: 0 12px 32px #00000008;
}
.article-card h2 {
  font-size: 24px;
  margin: 18px 0 12px;
  line-height: 1.6;
}
.article-card h2 a {
  color: var(--ink);
}
.article-card h2 a:hover {
  color: var(--accent);
  text-decoration: none;
}
.article-card p {
  font-size: 16px;
  color: var(--muted);
  margin: 0;
  line-height: 1.95;
}
.meta {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 16px;
  font-size: 13px;
  color: var(--muted);
}
.meta span:first-child {
  color: var(--accent);
}
.card-foot {
  margin-top: 20px;
}
.actions {
  display: flex;
  gap: 16px;
  align-items: center;
  font-size: 14px;
}
.pagination {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 20px;
  margin: 40px 0;
  font-size: 14px;
}
.site-footer {
  max-width: 980px;
  margin: 0 auto;
  padding: 26px 0 40px;
  border-top: 1px solid var(--line);
  color: var(--muted);
  font: 14px var(--serif);
}
.reading {
  max-width: 740px;
  margin: 15px auto;
}
.article-head {
  padding: 40px 0 30px;
  margin-bottom: 32px;
  border-bottom: 1px solid var(--line);
}
.article-head h1 {
  margin: 18px 0;
}
.article-head > p {
  color: var(--muted);
}
.markdown {
  overflow-wrap: anywhere;
  line-height: 2;
}
.markdown h2 {
  margin: 40px 0 18px;
}
.markdown h3 {
  margin: 28px 0 15px;
}
.markdown p {
  margin: 18px 0;
}
.markdown ul {
  list-style: disc;
  padding-left: 26px;
}
.markdown ol {
  list-style: decimal;
  padding-left: 26px;
}
.markdown pre {
  background: #202c2b;
  color: #e3ede8;
  overflow: auto;
  padding: 24px;
  border-radius: 12px;
  font-size: 14px;
  line-height: 1.8;
}
.markdown code {
  font-family: Consolas, monospace;
}
.markdown :not(pre) > code {
  background: var(--color-base-200);
  padding: 3px 6px;
  border-radius: 4px;
  font-size: 0.9em;
}
.markdown blockquote {
  border-left: 3px solid var(--accent);
  padding: 4px 22px;
  background: #2f706807;
  color: var(--muted);
}
.markdown table {
  display: block;
  overflow: auto;
  border-collapse: collapse;
  max-width: 100%;
}
.markdown td,
.markdown th {
  border: 1px solid var(--line);
  padding: 8px 14px;
}
.markdown hr {
  border: 0;
  border-top: 1px solid var(--line);
  margin: 30px 0;
}
.hljs-keyword,
.hljs-selector-tag {
  color: #9bd3c4;
}
.hljs-string {
  color: #c8d6a2;
}
.hljs-comment {
  color: #94a6a2;
}
.image-placeholder {
  display: block;
  border: 1px dashed var(--line);
  padding: 16px;
  font-size: 14px;
}
.comments {
  margin-top: 60px;
  border-top: 1px solid var(--line);
  padding-top: 25px;
}
.comments form .btn {
  margin-top: 12px;
}
.comment {
  padding: 22px 0;
  border-bottom: 1px solid var(--line);
}
.comment p {
  margin: 10px 0;
  overflow-wrap: anywhere;
}
.notice {
  padding: 14px 18px;
  background: #e9eee7;
  border-radius: 9px;
  font-size: 14px;
}
.error {
  background: #faeae7;
  color: var(--color-error);
}
.panel {
  padding: 36px;
  background: var(--surface);
  border: 1px solid var(--line);
  border-radius: 18px;
}
.auth {
  max-width: 470px;
  margin: 45px auto;
}
.auth h1 {
  font-size: 32px;
  margin: 12px 0;
}
.auth form {
  margin-top: 25px;
}
.auth label {
  margin-top: 18px;
}
.auth form > .btn {
  width: 100%;
  margin-top: 15px;
}
.empty {
  text-align: center;
  padding: 60px 15px;
}
.page-title {
  margin: 25px 0;
}
.editor-fields {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}
.editor-columns {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 24px;
  margin: 25px 0;
}
.code-editor {
  min-height: 600px;
  font:
    14px/1.9 Consolas,
    monospace;
}
.preview {
  padding: 24px;
  background: var(--surface);
  border: 1px solid var(--line);
  border-radius: 12px;
  min-width: 0;
  max-height: 720px;
  overflow: auto;
}
.preview h2 {
  font-size: 18px;
}
.file-import {
  margin: 25px 0;
  max-width: 400px;
}
.file-import input {
  font-size: 14px;
}

.markdown img {
  max-width: 100%;
  height: auto;
  border-radius: 12px;
  margin: 20px 0;
}
.alert {
  margin: 18px 0;
  font-size: 14px;
  box-shadow: none;
}
.panel {
  background: var(--color-base-100);
  box-shadow: none;
}
.auth {
  margin-top: 60px;
}
.preview {
  border-radius: var(--radius-box);
}
@media (max-width: 700px) {
  .header-inner {
    padding: 22px;
    min-height: 84px;
    flex-wrap: wrap;
    gap: 14px;
  }
  nav {
    gap: 20px;
  }
  .brand {
    font-size: 27px;
  }
  .site-main {
    padding: 0 22px 50px;
  }
  .home-hero {
    min-height: 460px;
    padding: 60px 0 64px;
  }
  .home-hero h1 {
    font-size: clamp(42px, 10vw, 58px);
  }
  .hero-description {
    font-size: 16px;
    margin-top: 24px;
  }
  .hero-read {
    margin-top: 26px;
  }
  .article-list,
  .editor-fields,
  .editor-columns {
    grid-template-columns: 1fr;
  }
  .article-card {
    padding: 24px;
  }
  .article-card h2 {
    font-size: 23px;
  }
  .list-toolbar {
    flex-wrap: wrap;
  }
  .search {
    min-width: 170px;
  }
  .list-toolbar > label:last-child {
    flex: 1;
  }
  .section-heading {
    margin-bottom: 20px;
  }
  .intro {
    padding: 40px 0 24px;
  }
  .auth {
    padding: 24px;
    margin: 25px 0;
  }
  .site-footer {
    margin: 0 22px;
  }
  .code-editor {
    min-height: 400px;
  }
  .reading {
    margin-top: 25px;
  }
  .meta {
    gap: 12px;
  }
}
@media (prefers-reduced-motion: reduce) {
  .hero-line > span,
  .hero-description,
  .hero-read {
    animation: none;
    opacity: 1;
    transform: none;
  }
  * {
    scroll-behavior: auto !important;
    transition: none !important;
  }
}

:root {
  --surface: #ffffff;
  --muted: #64716a;
}
[data-theme="fieldnotes-dark"] {
  --surface: #192423;
  --muted: #a7bab0;
  --color-paper: #111a19;
  --color-ink: #e3eae5;
  --color-muted: #a7bab0;
  --color-line: #30403c;
}
.nav-item,
.meta time,
.actions > a,
.comments > h2 {
  display: inline-flex;
  align-items: center;
  gap: 8px;
}
.nav-item svg {
  opacity: 0.75;
}
.meta .badge {
  font-size: 13px;
  padding: 5px 10px;
  height: auto;
  font-weight: 400;
}
.meta time {
  gap: 6px;
}
.search {
  position: relative;
}
.search-icon {
  position: absolute;
  left: 14px;
  top: 50%;
  transform: translateY(-50%);
  color: var(--muted);
  pointer-events: none;
  z-index: 1;
}
.search .input {
  padding-left: 42px;
}
.theme-toggle {
  color: var(--ink);
  flex-shrink: 0;
}
.theme-toggle svg {
  stroke-width: 1.6;
}
.article-card p {
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}
.article-card .card-foot {
  margin-top: auto;
  padding-top: 24px;
}
.archive-list {
  display: grid;
  gap: 18px;
}
.alert {
  display: block;
}
.alert a {
  margin-inline: 3px;
}
.comments > h2 {
  display: flex;
}
.markdown blockquote {
  background: color-mix(in srgb, var(--accent) 7%, transparent);
}
::view-transition-old(root),
::view-transition-new(root) {
  animation: none;
  mix-blend-mode: normal;
}
::view-transition-old(root) {
  z-index: 1;
}
::view-transition-new(root) {
  z-index: 2;
}
@media (max-width: 700px) {
  .header-inner {
    position: relative;
    gap: 16px;
    flex-wrap: wrap;
  }
  nav {
    width: 100%;
    gap: 14px;
  }
  .theme-toggle {
    position: absolute;
    top: 22px;
    right: 22px;
    width: 36px;
    height: 36px;
    min-height: 36px;
  }
  .nav-item {
    gap: 5px;
  }
}
@media (prefers-reduced-motion: reduce) {
  .article-card:hover {
    transform: none;
  }
}
```
{% endraw %}

## 继续阅读

[上一篇]({% post_url 2026-11-20-intern-blog-12-components %})

[下一篇]({% post_url 2026-11-20-intern-blog-14-tests %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
