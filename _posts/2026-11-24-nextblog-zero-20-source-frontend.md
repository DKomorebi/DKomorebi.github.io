---
layout: post
title: 前端源码卷（二十）：全部页面、组件、样式与种子数据
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 前端源码卷（二十）：全部页面、组件、样式与种子数据：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

本篇用于逐行查代码，不替代前面的方法解释。所有src文件来自这次更新的同一工作区快照，包含新的确认密码与邮箱验证码表单。第十九篇索引中的行号对应本篇原文。

其中src/lib/posts.ts和src/types/post.ts保留静态阶段的数据结构，当前仅作为seed等历史辅助代码；网页实际读取Express API。字符串里描述旧阶段的示例文章是种子正文，不是本教程对当前架构的判断。

## 对照源码：src/app/about/page.tsx（第 1–25 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/about/page.tsx start: 1 end: 25 sha256: 4f76f4b49789f8018d907d24b05f07c0d6bf4d20c9f1c80c923c6017e0ea444c -->

{% raw %}
````tsx
export const metadata = { title: "关于拾页" };
export default function About() {
  return (
    <div className="reading">
      <section className="intro">
        <h1>让想法，慢慢生长。</h1>
        <p>拾页，是一个安放文字的小地方。</p>
      </section>
      <div className="markdown">
        <p>
          在这里记录技术探索，也记录日常里微小而真实的发现。写作是整理思绪的过程，阅读是与另一种经验相遇。
        </p>
        <h2>关于这个小站</h2>
        <p>
          用 Markdown
          写作，分享值得记录的想法。欢迎注册账号，在文章下留下你的思考。
        </p>
        <h2>留言约定</h2>
        <p>
          尊重不同的观点，讨论具体的问题。请勿发布他人的隐私或无关广告。你可以删除自己的留言，管理员可以维护讨论秩序。
        </p>
      </div>
    </div>
  );
}
````
{% endraw %}

## 对照源码：src/app/admin/[id]/page.tsx（第 1–13 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/[id]/page.tsx start: 1 end: 13 sha256: 3f6f925c6632f894c52a87224e92bd47c148263ee7d909cf659f55719a0094cc -->

{% raw %}
````tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ id: string }>;
}) {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor id={(await params).id} />
    </Suspense>
  );
}
````
{% endraw %}

## 对照源码：src/app/admin/new/page.tsx（第 1–9 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/new/page.tsx start: 1 end: 9 sha256: 93536324a61c89e0e8940aa165f4b1596ea1eae6048db39dbf57d07573d2ccdf -->

{% raw %}
````tsx
import { Suspense } from "react";
import { Editor } from "@/components/full-blog";
export default function Page() {
  return (
    <Suspense fallback={<p>正在打开编辑器…</p>}>
      <Editor />
    </Suspense>
  );
}
````
{% endraw %}

## 对照源码：src/app/admin/page.tsx（第 1–4 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/admin/page.tsx start: 1 end: 4 sha256: 631bf8cdc05478b190ed32ffe351d0c8666e860eb7a9e53320283b66c4a9a6b3 -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export default function Page() {
  return <ArticleList manage />;
}
````
{% endraw %}

## 对照源码：src/app/archive/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/archive/page.tsx start: 1 end: 5 sha256: 8888f2a2b9b56527b3a4f2fe9e162b4458f89211a38efeb468b391b2030c18c6 -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export const metadata = { title: "文章归档" };
export default function Archive() {
  return <ArticleList archive />;
}
````
{% endraw %}

## 对照源码：src/app/globals.css（第 1–792 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/globals.css start: 1 end: 792 sha256: 9168a622124977504fe6f95bb80d1c379b6141cf4d069201e49d1cc0a02b9aea -->

{% raw %}
````css
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
````
{% endraw %}

## 对照源码：src/app/layout.tsx（第 1–30 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/layout.tsx start: 1 end: 30 sha256: 2c420ea29526fe04f3f5ee48ab37a71f2d3dd1268e299190abe526598c0fce40 -->

{% raw %}
````tsx
import type { Metadata } from "next";
import { Header, SessionProvider } from "@/components/full-blog";
import "./globals.css";
import { themeInitScript } from "@/lib/theme";
export const metadata: Metadata = {
  title: { default: "拾页 · 落笔成文，拾页成光", template: "%s · 拾页" },
  description: "关于技术、生活与每一个值得记录的想法。",
};
export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="zh-CN" data-theme="fieldnotes" suppressHydrationWarning>
      <head>
        <script dangerouslySetInnerHTML={{ __html: themeInitScript }} />
      </head>
      <body>
        <SessionProvider>
          <Header />
          <main className="site-main">{children}</main>
          <footer className="site-footer">
            <span>拾页</span>
          </footer>
        </SessionProvider>
      </body>
    </html>
  );
}
````
{% endraw %}

## 对照源码：src/app/login/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/login/page.tsx start: 1 end: 5 sha256: 1cad94120696bfb7a62ae48364ebb26b7355d5e08eb600526505f70028b2c1d8 -->

{% raw %}
````tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "登录" };
export default function Page() {
  return <AuthForm />;
}
````
{% endraw %}

## 对照源码：src/app/not-found.tsx（第 1–19 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/not-found.tsx start: 1 end: 19 sha256: 8ef3b086583d7df499e91d0aa56a3aa07eb432e225c93a127e117a0e8cb1c64d -->

{% raw %}
````tsx
import Link from "next/link";

export default function NotFound() {
  return (
    <main className="px-6 py-28 text-center">
      <p className="font-mono text-sm text-accent">404 / PAGE NOT FOUND</p>
      <h1 className="mt-5 text-3xl font-bold">这一页，还没有写下。</h1>
      <p className="mt-4 text-sm text-muted">
        地址可能有误，回到首页继续翻阅吧。
      </p>
      <Link
        href="/"
        className="mt-8 inline-block rounded-full bg-ink px-6 py-3 text-sm text-white"
      >
        返回首页
      </Link>
    </main>
  );
}
````
{% endraw %}

## 对照源码：src/app/page.tsx（第 1–4 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/page.tsx start: 1 end: 4 sha256: 033ac29ce9c3b3ccba54f1f5832923a6e4d4820c1b14c1c5281a2aaa305085ef -->

{% raw %}
````tsx
import { ArticleList } from "@/components/full-blog";
export default function Home() {
  return <ArticleList />;
}
````
{% endraw %}

## 对照源码：src/app/posts/[slug]/page.tsx（第 1–8 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/posts/[slug]/page.tsx start: 1 end: 8 sha256: 1b3c17a6a4f41beb68c27c7abfb163dc2c9888b736c636752d80c56add88ab78 -->

{% raw %}
````tsx
import { ArticleReader } from "@/components/full-blog";
export default async function Page({
  params,
}: {
  params: Promise<{ slug: string }>;
}) {
  return <ArticleReader slug={(await params).slug} />;
}
````
{% endraw %}

## 对照源码：src/app/register/page.tsx（第 1–5 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/app/register/page.tsx start: 1 end: 5 sha256: c439d49590283c273d832ae19466d65b9c6b767b84d2d846e06dda2683c6f249 -->

{% raw %}
````tsx
import { AuthForm } from "@/components/full-blog";
export const metadata = { title: "注册" };
export default function Page() {
  return <AuthForm register />;
}
````
{% endraw %}

## 对照源码：src/components/full-blog.tsx（第 1–1000 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/components/full-blog.tsx start: 1 end: 1000 sha256: c9196feccce47835b38f94b236f78bd903c76c946dee6111c5bd7a2f279b87c2 -->

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
````
{% endraw %}

## 对照源码：src/components/home-hero.tsx（第 1–23 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/components/home-hero.tsx start: 1 end: 23 sha256: 9940168990bc1e765e9fd15b937c8eaac03621cafff7c6f3c0a06b689f85ce5c -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：src/components/theme-toggle.tsx（第 1–109 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/components/theme-toggle.tsx start: 1 end: 109 sha256: b1f7472a040500549b6b7abf7ad3cb970e3afc77a39ef9bc6f008c1b2ce8b5b9 -->

{% raw %}
````tsx
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
````
{% endraw %}

## 对照源码：src/lib/posts.ts（第 1–215 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/lib/posts.ts start: 1 end: 215 sha256: e72f6a5e6fe085b79110d239f5b63c10fe69d61c5bfda36b17917243047ad3d2 -->

{% raw %}
````ts
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
````
{% endraw %}

## 对照源码：src/lib/theme.ts（第 1–6 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/lib/theme.ts start: 1 end: 6 sha256: 3cfb791e3a3084f8632e4a360cb42b3e2b63f50786a8e435816c5f9b343f5679 -->

{% raw %}
````ts
export const THEME_KEY = "fieldnotes-theme";
export const LIGHT_THEME = "fieldnotes";
export const DARK_THEME = "fieldnotes-dark";
// 在首帧绘制前应用偏好，避免刷新深色页面时先闪现浅色背景。
// 只包含项目常量，不插入用户输入。
export const themeInitScript = `(()=>{let theme;try{theme=localStorage.getItem('${THEME_KEY}')}catch{}if(theme!=='${LIGHT_THEME}'&&theme!=='${DARK_THEME}')theme=matchMedia('(prefers-color-scheme: dark)').matches?'${DARK_THEME}':'${LIGHT_THEME}';document.documentElement.dataset.theme=theme})()`;
````
{% endraw %}

## 对照源码：src/types/post.ts（第 1–20 行）

这是本次版本的原文完整文件，与上文解释按方法名对照。

<!-- source: src/types/post.ts start: 1 end: 20 sha256: 267fd3381470fd00dd73f9f672b783eddfbb200d6a3174ae8ded3fca315087f3 -->

{% raw %}
````ts
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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-19-method-index %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-21-source-backend %})
