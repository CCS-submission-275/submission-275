const {DevTools} = require('./devtools');
const jStat = require('jstat');

const devToolsOptions = {
    headless: false,
    userDataDir: './userData',
    executablePath: `${process.env.JSCAPSULE_CHROMIUM}`,
    defaultViewport: {
        width: 1920,
        height: 1080,
    },
    args: ['--no-sandbox'],
    // browserURL: 'http://127.0.0.1:9222/'
};

const loadingTime = (perf) => {
    return perf.domContentLoadedEventEnd - perf.navigationStart;
};


async function run(page, url, withRecordOn = false) {
    if (withRecordOn) {
        await page._client.send('Forensics.enable');
        await page._client.send('Forensics.startRecord');
    }
    await page.goto(url, {timeout: 0});
    return await page.evaluate('performance.timing.toJSON()');

};

function getLoadingTime(dataPoints) {
    return dataPoints.map(dp => loadingTime(dp));
}

function getOverhead(baseline, measured) {
    return (measured - baseline) / baseline;
}

function getOverallOverhead(baseline, measured) {
    const baseLineLoadingTime = jStat(getLoadingTime(baseline));
    const measuredLoadingTime = jStat(getLoadingTime(measured));
    console.log('Overhead by mean', getOverhead(baseLineLoadingTime.mean(), measuredLoadingTime.mean()));
    console.log('Overhead by median', getOverhead(baseLineLoadingTime.median(), measuredLoadingTime.median()));
    console.log('Overhead by min', getOverhead(baseLineLoadingTime.min(), measuredLoadingTime.min()));
}

const round = async (url ,N) => {
    console.log(`Running on ${url} for ${N} times...`)
    const d = new DevTools([], []);
    await d.launch(devToolsOptions); // launch the browser

    let page = await d.browser.newPage();
    const withoutRecording = [];
    await page.goto(url); // warm the cache
    console.log(`Running without recording...`)
    for (let i = 0; i < N; i++) {
        withoutRecording.push(await run(page, url));
    }
    await page.close();

    page = await d.browser.newPage();
    const withRecording = [];
    await page.goto(url); // warm the cache too
    console.log(`Running with recording...`)
    for (let i = 0; i < N; i++) {
        withRecording.push(await run(page, url, true));
    }
    await page.close();

    getOverallOverhead(withoutRecording, withRecording);
    console.log(`Running for ${url} completed`)
    await d.close();

};

const openWithRecordingOn = async () => {
    const d = new DevTools([], []);
    await d.launch(devToolsOptions); // launch the browser
    let page = await d.browser.newPage();
    await run(page, 'about:blank', true)
    // await page.close();
}

openWithRecordingOn();