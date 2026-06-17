---
layout: post
title: "Obsidian 结合 GitHub Pages：打造完美的水墨风博客工作流"
categories: [指南]
category_path: "指南/教程/写作"
cover: "https://placehold.co/1200x600/2f7068/f5f4ef.png?text=Obsidian+Workflow"
description: "一篇专为本博客定制的 Obsidian 使用教程，教你如何优雅地管理图片、使用模板，并实现一键发布。"
keywords: "Obsidian, 教程, 博客, 工作流, Jekyll, GitHub Pages"
---

既然你已经拥有了这款高颜值的「滴墨」主题博客，那么接下来最重要的事情就是：**怎样才能写得爽？**

如果你还在用记事本或者 VSCode 艰难地敲 Markdown，那就太痛苦了。今天，我将向你强烈推荐并详细演示如何使用 **Obsidian** 来作为这款博客的专属写作利器。

Obsidian 不仅仅是一个笔记软件，只要经过几步简单的调教，它就能化身为一个完美契合 Jekyll 博客的本地 CMS（内容管理系统）。

---

## 1. 为什么选择 Obsidian？

相比于传统的编辑器，Obsidian 在写博客时有三个杀手锏：
1. **所见即所得**：极佳的 Markdown 实时预览体验。
2. **极速贴图**：Ctrl+V 粘贴截图，图片直接保存在本地，不用去折腾图床。
3. **强大的生态**：配合插件，可以在 Obsidian 里一键 Push 到 GitHub，全自动发布。

## 2. 第一步：正确地打开仓库 (Vault)

**千万不要只把 `_posts` 文件夹作为仓库！**
你必须把整个博客的**根目录**（比如 `E:\study\Web\dink`）作为 Obsidian 的仓库打开。

**为什么？**
因为你的文章存在 `_posts` 里，但文章里引用的图片必须存放在 `assets/images` 里。只有把整个根目录作为仓库，Obsidian 才能正确理解这其中的相对路径。

---

## 3. 第二步：调教图片与链接设置（最关键！）

Obsidian 默认的链接语法是给它自己用的，Jekyll 根本不认识。所以我们必须修改设置，让它吐出标准的网页 Markdown。

点击 Obsidian 左下角的齿轮 ⚙️ 打开**设置**，找到 **「文件与链接」**：

1. **关闭「使用 Wiki 链接」** ❌：让 `[[图片.png]]` 变成 `![image](图片.png)`。
2. **打开「使用相对路径」** ✅：确保路径跨设备依然有效。
3. **附件默认存放路径**：选择 **「指定的文件夹」**。
4. **附件文件夹路径**：填入 `assets/images/post`。

> 💡 **小贴士**：完成这一步后，你用微信或 Snipaste 截的图，直接在 Obsidian 文章里 `Ctrl + V` 粘贴。图片会自动乖乖保存到 `assets/images/post` 目录下，并且文章里会自动生成像 `![image](../assets/images/post/Pasted-image.png)` 这样的标准链接，直接推送到线上就能完美显示！

*(更棒的是，我们的 GitHub Actions 会在云端自动把这些图片压成 WebP！)*

---

## 4. 第三步：配置一键模板

写博客最烦的就是每次都要手敲开头的 YAML 描述（Front Matter）。我们可以利用 Obsidian 的模板功能解决这个痛点。

### ① 开启模板插件
在设置 -> **「核心插件」** 中，找到并打开 **「模板 (Templates)」**。

### ② 创建模板文件
在根目录下（或者你专门建一个 `template` 文件夹），新建一个名为 `Post-Template.md` 的文件，填入以下内容：

```markdown
---
layout: post
title: "{{title}}"
categories: []
category_path: ""
description: ""
keywords: ""
---

# {{title}}

开始正文...
```

### ③ 设置模板路径
回到设置 -> **「模板」**：
- **模板文件夹位置**：选择你存放模板的文件夹。
- **日期格式**：设为 `YYYY-MM-DD`（这对 Jekyll 很重要）。

### ④ 愉快地使用
在 `_posts` 里新建文件，按照 Jekyll 规范命名为 `2026-06-17-我的文章.md`。
然后点击左侧边栏的**「插入模板」**按钮（或使用你配置的快捷键），刚才的架子就自动填好了，连文件名都会自动读取为标题！

---

## 5. 第四步：玩转树状分类 (category_path)

本博客的一大特色就是左侧分类页的**“树状折叠”**功能。在 Obsidian 里使用它非常直观。

应用模板后，找到 `category_path` 字段：
- 如果这是一篇散文：留空，或者只填 `categories: [随笔]`。
- 如果这是一篇系列教程：填写 `category_path: "开发/Obsidian教程/第一章"`。

推送上线后，系统就会自动为你生成一个带有下拉箭头的“第一章”文件夹，这篇文章会被优雅地收纳在里面。

---

## 6. 终极奥义：一键发布 (Obsidian Git)

如果你不想每次写完文章都去开黑黑的命令行输入 `git push`，你可以安装一个社区插件。

1. 在设置 -> **「社区插件」** 中关闭安全限制。
2. 浏览并搜索安装 **「Obsidian Git」**。
3. 启用后，在插件设置中开启 **「Auto Backup」**（比如每 30 分钟），或者设置一个**快捷键**触发 Commit 和 Push。

**至此，你的终极工作流就达成了：**
打开 Obsidian -> 快捷键新建文章 -> Ctrl+V 贴图 -> 疯狂输出 -> 快捷键一键 Push。

去倒杯水，当你回到电脑前，GitHub Actions 已经帮你转好了 WebP 图片，你的大作也带着水墨风的动效，静静地躺在线上博客里了。
