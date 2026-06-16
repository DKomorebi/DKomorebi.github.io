const sharp = require('sharp');
const { globSync } = require('glob');
const fs = require('fs');
const path = require('path');

const imageDirs = ['assets/images', 'images/post', 'assets/screenshots'];

async function convertImages() {
  for (const dir of imageDirs) {
    const files = globSync(`${dir}/**/*.{png,jpg,jpeg}`);
    for (const file of files) {
      const webpFile = file.replace(/\.(png|jpg|jpeg)$/i, '.webp');
      if (!fs.existsSync(webpFile)) {
        try {
          await sharp(file).toFile(webpFile);
          console.log(`Converted: ${file} -> ${webpFile}`);
        } catch (err) {
          console.error(`Error converting ${file}:`, err);
        }
      }
    }
  }
}

convertImages().catch(err => {
  console.error('Fatal error during conversion:', err);
  process.exit(1);
});
