# Jekyll 博客深度优化实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 Jekyll 博客实现全自动 WebP 转换、树状分类、可折叠归档及同类文章导航优化。

**Architecture:** 
1. 使用 GitHub Actions + Node.js (Sharp) 在构建时进行图片转换与 HTML 路径重写。
2. 通过 Liquid 模板逻辑重构分类和归档页面。
3. 使用 Vanilla JS 和 CSS 实现“回到顶部”及树状导航的交互。

**Tech Stack:** Jekyll, Liquid, Node.js (Sharp, JSDOM), GitHub Actions, Vanilla JS, CSS.

---

### Task 1: 自动化构建与 WebP 转换环境搭建

**Files:**
- Create: `package.json`
- Create: `scripts/convert-webp.js`
- Create: `scripts/post-build.js`
- Create: `.github/workflows/deploy.yml`

- [ ] **Step 1: 创建 `package.json` 并安装依赖**

```json
{
  "name": "dink-optimization",
  "version": "1.0.0",
  "scripts": {
    "convert-webp": "node scripts/convert-webp.js",
    "post-build": "node scripts/post-build.js"
  },
  "dependencies": {
    "sharp": "^0.33.0",
    "glob": "^10.3.0",
    "jsdom": "^22.1.0"
  }
}
```

- [ ] **Step 2: 编写 WebP 转换脚本 `scripts/convert-webp.js`**

```javascript
const sharp = require('sharp');
const { globSync } = require('glob');
const fs = require('fs');
const path = require('path');

const imageDirs = ['assets/images', 'images/post', 'assets/screenshots'];

imageDirs.forEach(dir => {
  const files = globSync(`${dir}/**/*.{png,jpg,jpeg}`);
  files.forEach(async (file) => {
    const webpFile = file.replace(/\.(png|jpg|jpeg)$/i, '.webp');
    if (!fs.existsSync(webpFile)) {
      await sharp(file).toFile(webpFile);
      console.log(`Converted: ${file} -> ${webpFile}`);
    }
  });
});
```

- [ ] **Step 3: 编写 HTML 后处理脚本 `scripts/post-build.js`**

```javascript
const { globSync } = require('glob');
const fs = require('fs');
const { JSDOM } = require('jsdom');

const htmlFiles = globSync('_site/**/*.html');

htmlFiles.forEach(file => {
  const content = fs.readFileSync(file, 'utf8');
  const dom = new JSDOM(content);
  const document = dom.window.document;
  const images = document.querySelectorAll('img');

  images.forEach(img => {
    const src = img.getAttribute('src');
    if (src && (src.startsWith('/assets/') || src.startsWith('/images/')) && src.match(/\.(png|jpg|jpeg)$/i)) {
      img.setAttribute('src', src.replace(/\.(png|jpg|jpeg)$/i, '.webp'));
    }
  });

  fs.writeFileSync(file, dom.serialize());
});
```

- [ ] **Step 4: 配置 GitHub Actions `deploy.yml`**

```yaml
name: Deploy Jekyll with WebP
on:
  push:
    branches: ["main"]
permissions:
  contents: read
  pages: write
  id-token: write
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: ruby/setup-ruby@v1
        with:
          ruby-version: '3.1'
          bundle-cache: true
      - uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'
      - run: npm install
      - run: npm run convert-webp
      - run: bundle exec jekyll build
      - run: npm run post-build
      - uses: actions/upload-pages-artifact@v3
  deploy:
    environment:
      name: github-pages
      url: ${{ steps.deployment.outputs.page_url }}
    runs-on: ubuntu-latest
    needs: build
    steps:
      - id: deployment
        uses: actions/deploy-pages@v4
```

---

### Task 2: 树状分类逻辑实现

**Files:**
- Modify: `_layouts/categories.html`
- Modify: `assets/css/shuimo.css`

- [ ] **Step 1: 重写 `_layouts/categories.html` 使用 Liquid 解析 `category_path`**

```liquid
{% assign all_paths = "" | split: "," %}
{% for post in site.posts %}
  {% if post.category_path %}
    {% assign all_paths = all_paths | push: post.category_path %}
  {% endif %}
{% endfor %}
{% assign unique_paths = all_paths | uniq | sort %}

<div class="tree-categories">
  <!-- 简单的逻辑：先按路径字符串显示，稍后通过 JS 或更复杂的 Liquid 嵌套实现树形 -->
  {% for path in unique_paths %}
    <details class="tree-node">
      <summary>{{ path }}</summary>
      <ul>
        {% for post in site.posts %}
          {% if post.category_path == path %}
            <li><a href="{{ post.url }}">{{ post.title }}</a></li>
          {% endif %}
        {% endfor %}
      </ul>
    </details>
  {% endfor %}
</div>
```

- [ ] **Step 2: 添加树状分类样式到 `assets/css/shuimo.css`**

```css
.tree-node { margin-left: 1rem; border-left: 1px solid var(--border-color); padding-left: 0.5rem; }
.tree-node summary { cursor: pointer; padding: 0.5rem; list-style: none; }
.tree-node summary::before { content: '▶ '; font-size: 0.8em; }
.tree-node[open] > summary::before { content: '▼ '; }
```

---

### Task 3: 同类邻近导航与归档折叠

**Files:**
- Modify: `_layouts/post.html`
- Modify: `_layouts/archives.html`

- [ ] **Step 1: 修改 `_layouts/post.html` 优化上一篇/下一篇逻辑**

```liquid
{% if page.category_path %}
  {% assign posts_in_cat = site.posts | where: "category_path", page.category_path | reverse %}
  {% for post in posts_in_cat %}
    {% if post.url == page.url %}
      {% assign prev_idx = forloop.index0 | minus: 1 %}
      {% assign next_idx = forloop.index0 | plus: 1 %}
      {% if prev_idx >= 0 %}{% assign prev_post = posts_in_cat[prev_idx] %}{% endif %}
      {% if next_idx < posts_in_cat.size %}{% assign next_post = posts_in_cat[next_idx] %}{% endif %}
      {% break %}
    {% endif %}
  {% endfor %}
{% endif %}

<!-- 原有的导航 UI 代码，将链接替换为 prev_post 和 next_post -->
```

- [ ] **Step 2: 修改 `_layouts/archives.html` 实现年份折叠**

```liquid
<div class="timeline">
  {% assign last_year = '' %}
  {% for post in site.posts %}
    {% capture y %}{{ post.date | date: '%Y' }}{% endcapture %}
    {% if y != last_year %}
      {% unless forloop.first %}</details>{% endunless %}
      <details class="tl-group" {% if forloop.first %}open{% endif %}>
        <summary class="tl-year">{{ y }}</summary>
      {% assign last_year = y %}
    {% endif %}
    <a class="tl-item" href="{{ post.url }}">...</a>
  {% endfor %}
</div>
```

---

### Task 4: “回到顶部”按钮与最终样式调整

**Files:**
- Modify: `_layouts/default.html`
- Modify: `assets/js/shuimo.js`
- Modify: `assets/css/shuimo.css`

- [ ] **Step 1: 在 `_layouts/default.html` 尾部添加按钮 HTML**

```html
<button id="back-to-top" class="back-to-top" title="回到顶部">
  <svg viewBox="0 0 24 24"><path d="M7.41 15.41L12 10.83l4.59 4.58L18 14l-6-6-6 6z"/></svg>
</button>
```

- [ ] **Step 2: 在 `assets/js/shuimo.js` 添加平滑滚动逻辑**

```javascript
const btt = document.getElementById('back-to-top');
window.onscroll = () => {
  if (document.body.scrollTop > 200 || document.documentElement.scrollTop > 200) {
    btt.classList.add('show');
  } else {
    btt.classList.remove('show');
  }
};
btt.onclick = () => {
  window.scrollTo({ top: 0, behavior: 'smooth' });
};
```

- [ ] **Step 3: 在 `assets/css/shuimo.css` 添加按钮的水墨风格样式**

```css
.back-to-top {
  position: fixed; bottom: 2rem; right: 2rem;
  width: 3rem; height: 3rem;
  background: var(--bg-soft); border: 1px solid var(--border-color);
  border-radius: 50%; display: none; cursor: pointer;
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}
.back-to-top.show { display: flex; align-items: center; justify-content: center; }
/* 添加水墨晕染动画效果 */
```
