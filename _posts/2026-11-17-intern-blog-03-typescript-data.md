---
layout: post
title: Next.js博客（三）：TypeScript 类型设计与文章数据建模
date: 2026-11-17 09:02:00 +0800
categories:
  - 实习项目
category_path: 开发/实习项目/博客系统
description: 结合 Post、Section、Category 和查询函数，理解接口、联合类型、可选属性、泛型及静态检查的边界。
keywords: TypeScript, interface, 联合类型, 泛型, 数据建模
cover:
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

类型的价值是让数据约定更明确。这一篇从真实的文章结构出发，解释编译器为什么能发现漏写字段、错误分类和可能不存在的文章。

> 本文是「实习博客实战」系列第 3 / 8 篇。文中的 src、docs、package.json 等路径均相对于拾页 Demo 项目根目录。

## 系列导航

1. [实习博客实战（一）：项目搭建与 React 组件入门]({% post_url 2026-11-17-intern-blog-01-setup-react %})
2. [实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})
3. [实习博客实战（三）：TypeScript 类型设计与文章数据建模]({% post_url 2026-11-17-intern-blog-03-typescript-data %})
4. [实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
5. [实习博客实战（五）：用 React 实现搜索、分类与排序]({% post_url 2026-11-17-intern-blog-05-react-interactions %})
6. [实习博客实战（六）：Express、PostgreSQL 与 TypeORM 接入路线]({% post_url 2026-11-17-intern-blog-06-express-postgresql-typeorm %})
7. [实习博客实战（七）：编译、验收、问题排查与实习练习]({% post_url 2026-11-17-intern-blog-07-build-check-practice %})
8. [实习博客实战（八）：全部页面与组件源码归档]({% post_url 2026-11-17-intern-blog-08-pages-components-source %})

## TypeScript：给数据和函数写约定

### 基础类型与数组

`src/types/post.ts` 里的 `title: string` 是字符串，`minutes: number` 是数字，`tags: string[]` 是字符串数组。属性类型不是默认值，它只描述允许出现的值。

{% raw %}
```ts
const minutes: number = 6;
// const minutes: number = "6"; // 故意写错：字符串不能赋给数字
```
{% endraw %}

不是每个变量都要显式标注。`const [query, setQuery] = useState("")` 可以从空字符串推断 query 是 string。[基础类型参考](https://www.typescriptlang.org/docs/handbook/2/everyday-types.html)

### interface 描述对象形状

当前项目的实际定义：

{% raw %}
```ts
export interface Section {
  heading: string;
  paragraphs: string[];
  code?: string;
}
```
{% endraw %}

每一节都必须有 heading 和 paragraphs；`code?` 可以不存在。可选属性读取后可能是 undefined，因此展示时先判断。`Post` 则把标题、摘要、分类、日期、标签和正文小节组合成完整文章。

`export const posts: Post[] = [...]` 会检查数组中的每一个对象。如果漏写 title，或者 minutes 写成文字，编辑器和 `pnpm typecheck` 会提示具体位置。这是我们把模拟数据也写上类型的原因。

### 联合类型限制可选值

{% raw %}
```ts
export type Category = "前端开发" | "后端探索" | "成长手记";
```
{% endraw %}

竖线表示“或者”。分类不是任意字符串，而是这三个值之一。筛选器还需要“全部文章”，所以使用：

{% raw %}
```tsx
const [category, setCategory] =
  useState<Category | "全部文章">("全部文章");
```
{% endraw %}

`<...>` 是泛型参数，告诉 useState 允许保存什么值。“全部文章”是筛选器状态，不是某一篇文章的真实分类，所以没有加入 Category 本身。

### 函数返回值与 undefined

{% raw %}
```ts
export function getPost(slug: string): Post | undefined {
  return posts.find((post) => post.slug === slug);
}
```
{% endraw %}

括号里的 `slug: string` 限制输入，括号后的 `: Post | undefined` 描述输出。find 只返回第一条匹配数据，找不到时返回 undefined。不要用 `as Post` 强行抹掉“找不到”的可能性，应该处理这个分支。

### 读懂几种常见语法

| 项目写法 | 含义 |
| --- | --- |
| `import type { Post }` | 只导入类型，运行时不需要这个值 |
| `Post["cover"]` | 从 Post 中取出 cover 属性的类型，避免重复定义 |
| `post?.title` | post 不存在时返回 undefined，不继续读取 title |
| `post?.title ?? "文章不存在"` | 左边是 null 或 undefined 时使用默认值 |
| `Readonly<{ children: React.ReactNode }>` | 限制参数对象的属性赋值，children 可包含 React 可渲染内容；它不是深层冻结 |
| `@/types/post` | 路径别名；由 tsconfig 的 paths 映射到 src/types/post |

### strict 与编译的边界

`tsconfig.json` 的 `strict: true` 打开严格检查；`noEmit: true` 表示 TypeScript 检查过程不输出文件，实际编译交给 Next.js。不要为了消除报错随意关掉 strict 或添加 any。

TypeScript 的类型在运行时会消失。将来写 `const data: Post[] = await response.json()` 并不能证明服务器返回的 JSON 符合 Post。API 输入和输出需要运行时校验，这是后端接入阶段的新工作。

## 类型与数据源码

### src/types/post.ts

Post 是每篇文章的约定；Section 组织正文；cover 和 category 限制允许使用的选项。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```typescript
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
```
{% endraw %}

### src/lib/posts.ts

这里保留完整六篇示例数据，方便重建 Demo。理解结构后，不必逐字记住正文；重点阅读 posts 的类型声明、getPost 和 formatDate。

以下为当前 Demo 中该文件的完整源码。路径相对于 Demo 项目根目录，不是本博客的目录。

{% raw %}
```typescript
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
```
{% endraw %}

## 如何增加第七篇文章

1. 打开 `src/lib/posts.ts`，在 posts 数组中复制一篇结构完整的文章。
2. 使用新的、唯一的 slug，例如 `my-first-component`。
3. 修改 title、excerpt、date、tags 和 sections。category 必须使用已有三个值之一；cover 必须使用 Post 中约定的封面键。
4. 运行 `pnpm typecheck`。如果类型报错，按提示补齐字段。
5. 在 dev 中查看首页，再直接访问 `/posts/my-first-component/`。
6. 运行 `pnpm build`，确认 out 中出现新路径，再用 preview 检查。

扩展思考：首页“小站数字”和归档年月目前是为六篇 2026 年 11 月示例文章写的展示内容。添加其他日期的文章时，应将统计和归档分组改为数据驱动，而不是只改数字。相关练习见本系列第七篇。

## 本篇练习

临时把一篇文章的 minutes 改成字符串，再运行 `pnpm typecheck`，观察错误信息；随后恢复正确值。接着解释为什么接口中的 date 是 string，并不自动保证字符串一定是合法日期。

## 继续阅读

[← 上一篇：实习博客实战（二）：Next.js 路由、静态导出与页面完整源码]({% post_url 2026-11-17-intern-blog-02-nextjs-pages %})

[下一篇：实习博客实战（四）：Tailwind CSS 布局、响应式与封面组件 →]({% post_url 2026-11-17-intern-blog-04-tailwind-layout %})
