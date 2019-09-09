// Finalize Nodejs Script
// 1 - Append JS in HTML Document
// 2 - Gzip HTML
// 3 - Covert to Raw Bytes
// 4 - ( Save to File: webpage.h ) in dist Folder

const fs = require('fs')
const zlib = require('zlib')
const path = require('path')

function getByteArray (file) {
  const fileData = file.toString('hex')
  const result = []
  for (let i = 0; i < fileData.length; i += 2) { result.push(`0x${fileData[i]}${fileData[i + 1]}`) }
  return result
}

const js = fs.readFileSync(path.join(__dirname, 'dist/js/app.js'))
const html = `
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width,initial-scale=1.0">
    <meta name="referrer" content="no-referrer">
    <title>ESP8266 Led Driver</title>
  </head>
  <body>
    <noscript>
      <strong>We're sorry but ESP8266 Led Driver doesn't work properly without JavaScript enabled. Please enable it to continue.</strong>
    </noscript>
    <div id="app"></div>
    <script>${js}</script>
  </body>
</html>
`
// Gzip the index.html file with JS Code.
const gzippedIndex = zlib.gzipSync(html, { level: zlib.constants.Z_BEST_COMPRESSION })
const indexHTML = getByteArray(gzippedIndex)

const source = `
const uint32_t HTML_SIZE = ${indexHTML.length};
const uint8_t HTML[] PROGMEM = { ${indexHTML} };
`

fs.writeFileSync(path.join(__dirname, '/../src/webpage.h'), source, 'utf8')
