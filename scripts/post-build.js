const { globSync } = require('glob');
const fs = require('fs');
const { JSDOM } = require('jsdom');
const path = require('path');

const htmlFiles = globSync('_site/**/*.html');

htmlFiles.forEach(file => {
  const content = fs.readFileSync(file, 'utf8');
  const dom = new JSDOM(content);
  const document = dom.window.document;
  const images = document.querySelectorAll('img');

  let changed = false;
  images.forEach(img => {
    const src = img.getAttribute('src');
    if (src && (src.startsWith('/assets/') || src.startsWith('/images/')) && src.match(/\.(png|jpg|jpeg)$/i)) {
      // Check if .webp version exists
      const webpSrc = src.replace(/\.(png|jpg|jpeg)$/i, '.webp');
      
      // We assume if we are running this, the webp should exist or we want to point to it
      // In a real scenario, we might want to check file existence on disk
      // but src might be absolute to the site root, so we need to map it to _site path
      const filePath = path.join('_site', src);
      const webpFilePath = filePath.replace(/\.(png|jpg|jpeg)$/i, '.webp');
      
      if (fs.existsSync(webpFilePath)) {
        img.setAttribute('src', webpSrc);
        changed = true;
      }
    }
  });

  if (changed) {
    fs.writeFileSync(file, dom.serialize());
    console.log(`Updated images in: ${file}`);
  }
});
