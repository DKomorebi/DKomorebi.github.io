---
layout: page
title: 友链
subtitle: 没有链接的博客是孤独的
keywords: 友情链接
comments: false
menu: 友链
permalink: /links/
---

{% if site.data.links and site.data.links.size > 0 %}
<ul class="links-grid">
{% for link in site.data.links %}
  <li>
    <a class="link-card" href="{{ link.url }}" target="_blank" rel="noopener" data-ripple>
      {% if link.avatar %}<img src="{{ link.avatar }}" alt="{{ link.name }}">{% endif %}
      <span>
        <span class="n">{{ link.name }}</span>
        {% if link.desc %}<span class="desc">{{ link.desc }}</span>{% endif %}
      </span>
    </a>
  </li>
{% endfor %}
</ul>
{% else %}
<p style="color:var(--text-soft)">还没有友链。想交换链接的朋友，欢迎在 GitHub 提 issue 申请。</p>
{% endif %}

