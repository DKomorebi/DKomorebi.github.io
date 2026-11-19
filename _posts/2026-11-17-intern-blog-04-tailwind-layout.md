---
layout: post
title: NextBlog（四）：Tailwind CSS、DaisyUI 与简约页面
date: 2026-11-17 09:03:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统
description: 结合当前文章卡片、Hero、图标和双主题学习样式组织。
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

结合当前文章卡片、Hero、图标和双主题学习样式组织。

> 本文是 NextBlog 全栈博客系列第 4 / 15 篇，内容对应当前全栈版本。

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

## 三种样式工具各自负责什么

Tailwind CSS 提供原子工具类，例如 sr-only 和 whitespace-pre-wrap。DaisyUI 在 Tailwind 上提供 btn、input、select、card 等组件类。globals.css 中的 article-card、home-hero 和 header-inner 则控制本站的排版与布局。

读到 `className="btn btn-primary"` 时，btn 表示按钮基础规则，btn-primary 使用主题主色。读到 article-card 时，应到 globals.css 找项目自定义样式。当前项目使用了不少语义 CSS 类，不是把每条样式都写成 Tailwind 工具类。

## Tailwind 4 与 DaisyUI 的入口

代码位置：`src/app/globals.css`（节选）。

{% raw %}
```css
@import "tailwindcss";
@plugin "daisyui" {
  themes: false;
  logs: false;
}
```
{% endraw %}


CSS 文件先导入 Tailwind，再加载 DaisyUI 插件。themes: false 关闭内置主题选择，项目随后声明自己的两套主题。这里采用当前仓库的 Tailwind 4 写法，复制旧版本 tailwind.config.js 教程前需要核对版本。

代码位置：`src/app/globals.css`（节选）。

{% raw %}
```css
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
```
{% endraw %}


base-100 是页面底色，base-content 是正文颜色，primary 是强调色，primary-content 是强调色表面上的文字色。圆角、边框和控件尺寸也由变量统一。深色 fieldnotes-dark 采用同名变量，因此按钮、表单和卡片可以一起改变，不必为每个按钮单独写黑色样式。

## Hero 为什么既有留白又有层次

代码位置：`src/components/home-hero.tsx`（完整文件）。

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


这个组件负责语义内容：一个主标题、一句介绍、一个阅读锚点。排版宽度、间距、渐显时机在 globals.css 中控制。标题分成两行并只强调部分文字，可以在少量内容下建立视觉层级。

`href="#articles"` 跳到文章区域，不触发新的后端业务。CSS 在 prefers-reduced-motion 条件下关闭渐显并显示内容，避免读者因为减少动态效果设置而看不到文字。动画应作为补充，不能承担内容是否可读的职责。

## 文章卡片与响应式布局

ArticleList 渲染每篇文章时使用 article-card，同时保留标题链接、摘要、分类与日期。卡片用细边框、圆角和轻微悬停反馈区分内容，不再需要大段装饰性说明文字。分类标签使用 DaisyUI badge，辅助图标来自 lucide-react。

桌面文章区域使用网格，700px 以下切成单列。手机导航允许换行，主题按钮放在品牌右侧；搜索框与分类控件要共同适应可用宽度。仅观察桌面缩略图不够，还要实际检查 375px 下文档宽度是否超出视口。

## 图标、标签与可访问名称

Search、Folder、CalendarDays、Pencil、Trash2 等图标对应搜索、分类、日期、编辑和删除。旁边已有文字的图标设置 aria-hidden，避免屏幕阅读器重复朗读。主题按钮只有图标，所以由 aria-label 提供“切换深色主题”或“切换浅色主题”。

搜索框的说明使用 sr-only：视觉上隐藏，但仍为输入框提供名称。不要将所有小字一律删除；重复装饰文案可以精简，密码要求、错误提示与控件标签仍然影响用户完成操作。

## 深浅主题不只是 CSS

theme.ts 在首帧读取主题偏好，ThemeToggle 处理点击、系统偏好、跨标签同步及 localStorage。支持 View Transition 的浏览器使用圆形展开；不支持或开启减少动态效果时直接切换。第十三篇逐段解释这一实现。

练习：调整主题 primary 后检查按钮、图标和标签；再切到深色检查正文、边框、输入框、代码块和错误状态。调整卡片间距时同时检查桌面与手机，不要只扩大固定宽度。

## 继续阅读

[上一篇]({% post_url 2026-11-17-intern-blog-03-typescript-data %})

[下一篇]({% post_url 2026-11-17-intern-blog-05-react-interactions %})

[项目结构与完整源码汇总]({% post_url 2026-11-20-intern-blog-15-project-source %})
