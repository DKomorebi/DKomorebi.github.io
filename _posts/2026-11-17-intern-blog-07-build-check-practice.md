---
layout: post
title: "实习博客实战（七）：编译、验收、问题排查与实习练习"
date: 2026-11-17 09:06:00 +0800
categories: ["实习项目"]
category_path: "开发/实习项目/博客系统"
description: "记录静态构建与浏览器检查结果，给出排错表和六组练习，验证自己真正理解了项目。"
keywords: "Next.js, 构建, 验收, 调试, 实习, 学习路线"
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

这一篇把「能运行」变成可复现的验收过程。类型检查、编译和浏览器检查分别验证不同问题，不能相互替代。

> 本文是「实习博客实战」系列第 7 / 8 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [实习博客实战（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

## 构建与预览

在 Demo 根目录运行：

{% raw %}
```powershell
pnpm typecheck
pnpm build
pnpm preview
```
{% endraw %}

生产文件输出到 out 目录。用 HTTP 服务预览，不要双击 HTML，也不要对静态导出运行 next start。修改源代码后必须重新构建，preview 才会展示最新结果。

## 如何调试常见问题

| 症状 | 优先检查 |
| --- | --- |
| 找不到 pnpm | 确认包管理器安装和终端 PATH；也可按 README 使用 npm |
| 安装报 EACCES / 网络超时 | 检查工作环境网络权限、代理和项目 .npmrc，不要盲目改业务代码 |
| useState 相关的组件报错 | 文件是否需要声明 `"use client"`，是否错误地在服务端使用浏览器功能 |
| 新文章直接打开 404 | slug 是否正确、是否新增到 posts、是否重新 build、静态服务是否支持目录 index.html |
| 修改后 preview 不变 | preview 展示 out，需要重新 build |
| Tailwind 类无效果 | globals.css 是否导入、PostCSS 插件是否正确、类名是否完整可扫描 |
| 浏览器出现横向滚动 | 检查固定宽度、长代码和 grid 子项最小宽度 |
| port 3000 已占用 | 关闭自己的旧服务，或开发时使用 `pnpm dev --port 3001` |

开发时先读终端报错，再看浏览器控制台。一次只改一个原因明确的点。不要同时升级依赖、修改配置和改业务代码，否则难以知道哪一项解决了问题。

## 推荐阅读顺序与自测

第一轮只读 `types/post.ts → lib/posts.ts → components/cover.tsx → app/page.tsx`，理解数据如何展示。第二轮读 `post-browser.tsx`，理解状态与事件。第三轮读动态详情路由和构建配置，理解 URL 与静态导出。

当你能不用看文档回答这些问题，就具备继续接后端的基础了：为什么 slug 不能重复？为什么搜索组件需要 use client？为什么 TypeScript 不能校验未知 JSON？为什么静态站新增文章要重建？为什么 filter 后再 sort 不会修改原数组？为什么 md 不是设备类型？

本系列的 API 说明参考各篇附带的官方文档。具体项目行为以本仓库代码、锁文件与实际构建结果为准，后续升级版本时应重新检查迁移说明。

## 手动验收清单

先执行 `pnpm typecheck` 和 `pnpm build`，再用 `pnpm preview` 检查生产结果。以下是可重复的检查步骤，不代表每一项都已执行；本次实际结果见本篇后面的验证记录。

| 操作 | 预期结果 |
| --- | --- |
| 打开首页 | 六张文章卡片、精选、作者栏正常显示 |
| 搜索 `typescript` | 只显示匹配标题、摘要或标签的文章 |
| 搜索 ` TypeScript ` | 与无首尾空格搜索一致 |
| 选择后端探索 | 显示 PostgreSQL、Express 两篇文章 |
| 在后端分类下搜索 TypeScript | 显示无结果状态 |
| 点击重置筛选 | 搜索清空，恢复全部六篇文章 |
| 选择最早发布 | 11 月 12 日文章在前，17 日在后 |
| 点击任意文章 | 地址变化、标题正文对应、目录跳转正常 |
| 刷新详情页 | 仍能打开，不依赖先访问首页 |
| 打开不存在的地址 | 静态服务器返回 404 页面与状态 |
| 点击归档、关于、一起学习 | 导航有效，对应内容可见 |
| 使用 Tab、Enter、空格操作 | 搜索框、按钮、链接可操作，焦点清晰 |
| 375px、768px、1440px 宽度查看 | 没有整体横向溢出；卡片和侧栏按空间调整 |

本 Demo 没有自动化测试框架依赖。对于六条静态数据，先用类型检查、生产构建和交互检查验证行为；后续接入数据库后，再为接口和关键业务流程添加针对性测试。

## 练习一：修改个人信息（15 分钟）

在首页与关于页替换作者名字、介绍。保持布局在手机上可读。完成标准：导航、作者卡片、关于页没有相互矛盾的信息。

## 练习二：新增文章（30 分钟）

新增一篇 slug 唯一的文章，至少两个小节，其中一个带代码。完成标准：类型检查通过；首页出现新卡片；构建后直接访问新地址成功。

## 练习三：让统计与归档由数据生成（45 分钟）

把首页固定的 06 改为 posts.length；使用 Set 计算分类数；按年月将归档分组。完成标准：新增一篇不同月份文章后，正确出现新分组，无需手动改统计。

## 练习四：新增一个分类（30 分钟）

扩展 Category，补齐筛选器数组，新增该分类的文章。完成标准：分类按钮可选中，搜索与分类可以同时使用，无结果可恢复。

## 练习五：解释代码而不是背代码（20 分钟）

向导师演示一次搜索，并解释 useState、受控输入、filter、sort、map 分别在哪一步执行。说明为什么本项目可以静态部署，却仍然有交互。

## 练习六：模拟加载失败（后续阶段）

接入 API 后，手动停止后端，检查页面是否显示错误并支持重试，而不是一直空白。把网络请求的失败与“成功返回空数组”区分开。

## 2026-11-17 实际验证记录

日期：2026-11-17。环境：Windows、Node.js 24.22.3、pnpm；浏览器检查使用本机 Edge 的无头模式。

## 编译

- `pnpm typecheck`：通过。
- `pnpm build`（Next.js 16.4.6，Webpack）：通过，静态文件生成至 `out/`。
- 构建包含首页、关于、归档、自定义 404 和六篇文章详情。
- React 19.3.1、TypeScript 5.9.2、Tailwind CSS 4.3.5；完整版本以 `pnpm-lock.yaml` 为准。

默认 Turbopack 在本机报告 PostCSS 子进程拒绝访问。使用 Next.js 提供的 `--webpack` 选项后构建通过，已将 dev 和 build 脚本统一为这个选项，没有跳过类型检查。

依赖下载时，原环境配置的旧镜像无法访问，npm 官方源的大包下载超时，最终使用 npmmirror 完成安装。项目 `.npmrc` 保留官方源。如遇相同网络问题，可在公司允许的前提下执行 `pnpm install --frozen-lockfile --registry=https://registry.npmmirror.com`。

## 已执行的浏览器检查

基于生产目录 `out/`，使用 `pnpm preview` 提供静态服务。用 Playwright 驱动本机 Edge，执行断言：

- 首页状态码 200，六张文章卡片。
- 后端探索分类显示两篇；叠加 TypeScript 搜索显示无结果。
- 重置后恢复六篇；首尾空格搜索 ` TypeScript ` 找到一篇。
- 切换最早发布后，实习第一周文章排在第一。
- 375px、768px、1440px 三个视口，页面宽度未超出视口。
- 六个文章详情地址均可直接访问并返回 200；目录链接更新锚点。
- 关于、归档返回 200；不存在地址返回 404 并显示自定义文案。
- 实际点击首页“归档”，再点击文章链接，均能进入正确页面。
- 检查过程没有捕获到页面 JavaScript 异常。

已知限制：该 Next.js 版本在 Windows 静态导出下，部分分段预取 `.txt` 请求在预览服务器日志中返回 404。直接访问和实际点击导航已验证成功，但这不等同于所有资源请求都无错误；后续上线时应在目标静态托管环境复核分段资源路径和预取行为。

桌面与手机长截图已进行视觉检查，截图保存在原 Demo 的 docs 目录。

这不等于所有浏览器兼容性或完整无障碍审计。键盘和屏幕阅读器的完整流程、真实移动设备触摸操作尚未专项验证，可继续按验收文档检查。数据库与 API 不在当前静态阶段的验证范围。

## 完成这个系列之后

先独立新增一篇文章和一个页面，再向导师演示一次搜索的数据流。能够解释后，再按第六篇实现最小的只读 API。学习目标是能够修改、定位问题和说明理由，而不仅是复制一份能运行的代码。

## 继续阅读

[← 上一篇：实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})

[下一篇：实习博客实战（八）：全部页面与组件源码归档 →]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})
