const fs = require('fs');
const { JSDOM } = require('jsdom');

const dom = new JSDOM(`<!DOCTYPE html><html><body></body></html>`, { url: "http://localhost/" });
global.window = dom.window;
global.document = dom.window.document;
global.navigator = dom.window.navigator;
global.DOMParser = dom.window.DOMParser;

async function test() {
  let m;
  try {
    m = (await import('mermaid')).default;
  } catch (e) {
    console.error("Failed to load mermaid:", e);
    return;
  }

  m.initialize({ startOnLoad: false });

  const content = fs.readFileSync('_posts/2026-06-17-UML合集.md', 'utf8');
  const blocks = [];
  const regex = /```mermaid([\s\S]*?)```/g;
  let match;
  while ((match = regex.exec(content)) !== null) {
    blocks.push(match[1]);
  }

  for (let i = 0; i < blocks.length; i++) {
    try {
      await m.parse(blocks[i]);
      console.log(`Block ${i + 1} is VALID`);
    } catch (e) {
      console.error(`Block ${i + 1} is INVALID!`);
      // console.error(e.message || e);
    }
  }
}

test();