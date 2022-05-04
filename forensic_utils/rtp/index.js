const {chrome, list, output} = require('yargs-parser')(process.argv.slice(2));
const puppeteer = require('puppeteer');
const fs = require('fs');

const {getConsoleLogger} = require('./logger');
const assert = require("assert");

const logger = getConsoleLogger('index');

// arg checks
function checkArg(arg) {
  if (!fs.existsSync(arg)) {
    throw Error(`${arg} is not a valid path`);
  }
}

if (chrome) {
  checkArg(chrome);
} else {
  logger.error('argument "--chrome path/to/chrome" is required');
  process.exit(1);
}

if (list) {
  checkArg(list);
} else {
  logger.error('argument "--list path/to/website_list" is required');
  process.exit(1);
}


const REPEAT = 10;

const getLaunchOptions = (website) => {
  const url = new URL(website);
  return {
    headless: true,
    executablePath: chrome,
    args: [
      '--enable-logging=stderr',
      '--no-sandbox',
      `--forensic-log-file=${url.hostname}.log`, // this is not necessary for rtp
      '--vmodule=forensic_file_logger=7',
      '--incognito'],
    // dumpio: true,
    defaultViewport: null,
  };
};


function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function crawl(website) {
  const browser = await puppeteer.launch(getLaunchOptions(website));
  const pages = await browser.pages();
  assert(pages.length > 0);
  const page = pages[0];


  const client = await page.target().createCDPSession();
  // warm up the cache
  await page.goto(website);

  const w_recording = [];
  const wo_recording = [];
  let recordingOn = false;
  let measured = false;
  page.on('load', async () => {
    const pageLoadTime = await page.evaluate(() => {
      const perfData = window.performance.timing;
      return perfData.loadEventEnd - perfData.navigationStart;
    });
    if (recordingOn) {
      w_recording.push(pageLoadTime);
    } else {
      wo_recording.push(pageLoadTime);
    }
    measured = true;
  });


  logger.info(`Warming ${website}`);

  const url = page.url();
  for (let i = 0; i < REPEAT; ++i) {
    logger.info(`Visiting ${url} w/o recording...${i + 1}`);

    await page.goto(url);
    await sleep(2000);

    await client.send('Forensics.startRecord');
    recordingOn = true;
    logger.info(`Visiting ${url} w/ recording...${i + 1}`);
    await page.goto(url);

    await sleep(2000);

    await client.send('Forensics.stopRecord');
    recordingOn = false;
    logger.info(`Done ${url}...${i + 1}`);
  }
  await sleep(1000);
  await browser.close();
  return {
    withRecording: w_recording,
    withoutRecording: wo_recording
  };
}

async function run() {
  const websites = fs.readFileSync(list, {encoding: 'utf-8'});
  const visitedPath = 'visited_' + list;
  let visited = new Set();
  if (fs.existsSync(visitedPath)) {
    const visitedSites = fs.readFileSync(visitedPath, {encoding: 'utf-8'});
    visited = new Set(visitedSites.split('\n'));
  }

  const websitesToVisit = websites.split('\n').filter(w => !visited.has(w));
  const ret = [];
  for (let website of websitesToVisit) {
    try {
      const result = await crawl(website);
      ret.push({website, result});
      fs.writeFileSync(visitedPath, website + '\n', {flag: 'a'});
    } catch (e) {
      logger.error(e.message);
    }
  }

  return ret;

}

run().then((result) => {
  fs.writeFileSync(output, JSON.stringify(result));
});
