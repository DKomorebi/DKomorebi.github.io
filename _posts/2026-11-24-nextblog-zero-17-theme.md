---
layout: post
title: 主题与首屏（十七）：HomeHero 和 ThemeToggle 的每个方法与事件
date: 2026-11-24 00:00:00 +0800
categories:
- 实习项目
category_path: 开发/实习项目/博客系统/从零到逐方法
description: 主题与首屏（十七）：HomeHero 和 ThemeToggle 的每个方法与事件：结合当前文件，解释搭建思路、输入输出、事件、状态与错误分支。
keywords: NextBlog, Next.js, React, TypeScript, Express, TypeORM, DDD, 邮箱验证
cover: null
mermaid: false
sequence: false
flow: false
mathjax: false
mindmap: false
mindmap2: false
---

## 1. HomeHero() 为什么没有 click 方法

它返回section、标题、说明和href="#articles"链接。标题分两行，内层span配合CSS动画渐显；“开始阅读”使用浏览器原生锚点导航，目标是ArticleList里的id=articles。没有业务状态，也没有网络请求，所以无需useState或onClick。

注释写明纯CSS动效。组件虽然没有use client，但由客户端ArticleList导入时属于该客户端模块图的一部分；不能只看这个文件没有指令就断言它绝不进入客户端。

## 2. theme.ts 三个常量与立即执行脚本

THEME_KEY是localStorage键，LIGHT_THEME/DARK_THEME是DaisyUI主题名。themeInitScript在head中执行：尝试读localStorage；若保存值不是两种合法值，读取系统prefers-color-scheme；设置document.documentElement.dataset.theme。

try/catch处理存储不可用，例如受限浏览器环境。脚本只拼接项目常量，不能插入用户输入；在React挂载前设置主题是为减少深色页面刷新时先闪白的现象。这段是立即执行函数，不等待用户点击。

## 3. ThemeToggle的状态和初始化Effect

dark控制图标/aria标签；busy控制动画期间按钮disabled；inFlight是立即生效的Ref锁。Effect建立matchMedia对象，然后定义apply(theme)、sync()、storage(e)，挂载时sync一次，注册系统主题change与window.storage监听器，卸载时一一移除。

这里有两个名为apply的局部函数，它们作用域不同：Effect中的apply接收theme并更新根属性和dark；toggle内部的apply闭包捕获next，还负责写localStorage。不要看见同名就以为前者被后者覆盖。

## 4. sync()、apply(theme)、storage(e)分别做什么

sync尝试读取保存值。合法用户偏好优先于系统设置；没有保存偏好才根据media.matches选择深浅。apply把选择落在html的data-theme并同步dark。

storage仅当e.key等于本站主题键或为null（存储被clear）时sync。storage事件通常通知同源其他标签页，当前标签页点击时由toggle自己更新。所以这是跨标签同步，不是每秒轮询localStorage。

系统change触发sync而不是直接使用系统值，是为了尊重用户手动保存的主题；否则系统变化会把用户明确选择覆盖掉。

## 5. toggle(event)前半段：决定主题与退化方案

onClick传入React MouseEvent<HTMLButtonElement>。如果inFlight为true立即return，避免过渡竞争。读取当前根主题决定next，定义内部apply：设置根data-theme、setDark，再try写localStorage。

如果浏览器没有startViewTransition，或prefers-reduced-motion为reduce，直接apply并return。功能不依赖动画API才成立，因此旧浏览器仍能切换深浅色。

## 6. 坐标、半径和键盘操作

getBoundingClientRect获取按钮矩形；event.detail===0时通常来自键盘激活，使用按钮中心；鼠标点击则用clientX/clientY。半径Math.hypot(max(x,width-x),max(y,height-y))覆盖到离点击点最远的视口角，保证圆形展开最终覆盖整屏。

如果始终使用鼠标坐标，键盘用户可能从左上角或错误位置触发动画。这里显式处理了两种输入方式。

## 7. toggle后半段：Promise、动画与清理

先inFlight=true、busy=true；document.startViewTransition(apply)让浏览器捕获新旧页面视觉状态。尽早给transition.finished挂catch，避免页面隐藏或其他过渡抢占时产生未处理拒绝。await transition.ready后，对`::view-transition-new(root)`做clipPath从半径0到目标半径的580ms动画。

等待root.animate(...).finished再等待transition.finished；出现异常catch调用apply，保证主题最终仍应用；finally释放inFlight/busy，即使动画失败也不永久锁住按钮。这种“体验可退化，功能必须完成”的处理比假定动画永远成功可靠。

## 8. JSX和CSS配合

button有type=button、btn-circle、aria-label、aria-pressed与disabled。dark=true显示Sun，表示下一步切浅色；false显示Moon，表示下一步切深色。图标aria-hidden避免与标签重复朗读。

globals.css的view-transition相关规则关闭不需要的默认动画并提供截图层级；reduced-motion规则处理Hero等其他动效。主题组件不直接改每张卡片颜色，颜色跟随语义CSS变量。

验收：刷新记住主题；另开标签同步；系统主题变化在无手选偏好时生效；键盘Enter/Space能操作；减少动画模式直接切色；快速点击不会卡住。

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

## 本系列导航

[总导航]({% post_url 2026-11-24-nextblog-zero-00-guide %}) · [上一篇]({% post_url 2026-11-24-nextblog-zero-16-ddd %}) · [下一篇]({% post_url 2026-11-24-nextblog-zero-18-tests-build %})
