---
layout: post
title: 样式基础（四）：Tailwind、DaisyUI、布局和无障碍如何配合
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 样式基础（四）：Tailwind、DaisyUI、布局和无障碍如何配合：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. 这三个层次不是重复造轮子

Tailwind 给出 `flex`、`gap-2`、`min-w-0` 等原子类；DaisyUI 给出 `btn`、`input`、`card` 等组件样式；globals.css 定义本站版式、主题色、阅读排版与动画。DaisyUI 这里主要是 CSS 组件类，不是需要 `import { Button }` 的 React 组件库。

`postcss.config.mjs` 使用 `@tailwindcss/postcss`。`globals.css` 开头 `@import "tailwindcss"` 与 `@plugin "daisyui"` 属于当前 Tailwind 4/DaisyUI 5 的接入方式，不应再额外套用旧教程的 Tailwind 3 初始化命令。[DaisyUI 安装说明](https://daisyui.com/docs/install/)提供当前插件入口；复现本项目应使用锁文件里的版本。

## 2. 从一个注册按钮逐层读类名

`className="btn btn-primary"`：btn 提供基础尺寸、交互和布局，btn-primary 使用主题 primary 颜色。`disabled={busy}` 是原生 HTML 属性和 React 表达式，与 DaisyUI 样式配合，但“禁止重复发送”仍由 JS 和后端负责，CSS 不执行请求。

发送验证码的外层 `flex flex-wrap gap-2` 表示横向排列、窄屏允许换行、子项间距；输入框 `min-w-0 flex-1` 允许弹性缩小，避免默认最小内容宽度把手机页面撑开。`grid gap-3` 排列标签、输入行和说明。不能只缩小字号来掩盖布局溢出。

文章的 `card article-card` 同时应用库的卡片基础和本站选择器。分类使用 badge，删除用 text-error，导航按钮用 btn-ghost。遇到样式冲突先看浏览器 computed styles：最终值来自哪个选择器、是否在后面覆盖，而不是不停叠加随机类名。

## 3. 主题数据怎样变成 CSS

两个 `@plugin "daisyui/theme"` 分别注册 fieldnotes 与 fieldnotes-dark。`data-theme` 写在 html 根元素上，决定 `--color-base-100`、`--color-base-content`、`--color-primary` 等语义变量。`:root` 进一步把 `--bg`、`--ink`、`--accent` 映射到这些语义变量。

切换主题不是遍历每个按钮手动换颜色；只改根属性，CSS 变量重新计算即可。单独写死的颜色未必自动适应主题，例如早期 not-found 里部分 ink/white 工具类仍是固定色，学习时要区别“语义主题变量”和“固定颜色”两种写法。

## 4. 页面各部分对应什么样式责任

| 选择器或类 | 对应页面 | 负责内容 |
| --- | --- | --- |
| `.site-main`、`.reading` | 页面与阅读详情 | 内容宽度、居中、留白 |
| `.home-hero`、`.hero-line` | 首页 | 大标题区域、文字进入动画 |
| `.article-list`、`.archive-list` | 首页/归档 | 卡片排布或紧凑列表 |
| `.auth`、`.panel` | 登录注册 | 表单容器、边框与间距 |
| `.editor-columns`、`.preview` | 编辑 | Markdown 输入与预览两栏，窄屏调整 |
| `.markdown` | 阅读与编辑预览 | 标题、代码块、表格、图片的文章排版 |
| `.comments`、`.comment` | 文章详情 | 评论输入和列表 |
| `::view-transition-new(root)` | 主题切换 | 新主题截图的圆形展开动画 |

完整 CSS 在源码卷。读它时按上述组件定位相关选择器，而不是要求记住整张样式表。

## 5. 图标、标签和动效不是纯装饰

Lucide 的 `<Trash2 size={15} aria-hidden="true" />` 是 SVG 图标组件；旁边有“删除”文字时隐藏图标朗读，避免重复。只有图标的 ThemeToggle 需要 aria-label 明确说明动作。搜索框用 sr-only 标签给辅助技术名称，视觉上仍保持简洁。

错误使用 role=alert，发送结果使用 role=status，分页读数使用 aria-live。它们不会帮你校验数据，但能让不依赖视觉的用户知道请求结果。focus-visible 保留键盘焦点；不要为了简洁把 outline 一律去掉。

`prefers-reduced-motion` 代表用户希望减少动画。Hero 在该模式下直接显示，主题切换也直接应用颜色。主题动画的事件与几何计算在专题第十七篇逐行展开。

练习：给发送按钮增加 Mail 图标时，保留文字与 type="button"，给图标 aria-hidden，不要改成无语义的 div。检查 390px 和桌面宽度、深浅色以及键盘 Tab 顺序。

## 对照源码：src/app/globals.css（第 1–85 行）

这是本次版本的原文节选，与上文解释按方法名对照。

<!-- source: src/app/globals.css start: 1 end: 85 sha256: 4b890cca9d7ed59a89e9f282c46fadc1a6836927f9e700da2423245be6b1c55b -->

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
````
{% endraw %}

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-03-next-routes %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-05-express-syntax %})
