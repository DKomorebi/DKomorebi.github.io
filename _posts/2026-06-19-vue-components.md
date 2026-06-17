---
layout: post
title: "Vue组件传值技巧"
categories: [vue教学]
category_path: "vue教学/第二章 进阶"
description: "掌握 Props, Emit, Provide/Inject 等多种 Vue 组件间通信方式。"
keywords: "Vue, 组件, 通信, Props"
---

在掌握了基础概念后，我们进入第二章：组件化开发。

组件是 Vue.js 最强大的功能之一。组件系统让我们可以用独立可复用的小组件来构建大型应用。而组件之间的通信则是不可避免的话题。

## 1. 父传子：Props

最基础也是最常用的方式，父组件通过属性向下传递数据。

```html
<!-- 父组件 -->
<child-component :message="parentMsg"></child-component>
```

## 2. 子传父：Events (Emit)

子组件通过触发事件来向父组件发送消息。

```javascript
// 子组件
this.$emit('update', newValue);
```

## 3. 跨级传递：Provide / Inject

当我们需要在祖先组件和子孙组件之间传递数据时，如果使用 Props 逐层传递会非常繁琐。这时就可以使用 `provide` 和 `inject`。

```javascript
// 祖先组件
provide() {
  return {
    theme: 'dark'
  }
}

// 子孙组件
inject: ['theme']
```

熟练掌握这些传值技巧