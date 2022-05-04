## Procedure
We measure the page load time by the `performance` API.
`page_load_time = performance.timing.loadEventEnd - performance.timing.navigationStart`

For each browsing session, we create a CDP session such that we can turn on and off the recording.

For each website, we first visit it to warm up the network cache. And then reload it without recording turned on.
It sleeps for 1 second. Then it sends a command to turn on recording and visit the same website.

It alternates between recording being on and off for 10 times for each website.

The result is in the below example.

## Postprocessing

For each website, we will have 10+10 data points. We may choose the median/mean of the 10 data points as
a representing page loading time. So, the `overhead = (page_load_time_w_recording - page_load_time_wo_recording) / page_load_time_wo_recording`.

If we crawl 1k websites, we will have 1k data points. Build a box-plot from the data.

## Command
Example: `node index.js --chrome path/to/chrome --list path/to/website --output result.json`

## Result
Example:
```json
[
  {
    "website": "https://www.google.com",
    "result": {
      "withRecording": [
        228,
        241,
        216,
        217,
        216,
        223,
        266,
        215,
        214,
        228
      ],
      "withoutRecording": [
        271,
        233,
        250,
        198,
        210,
        202,
        210,
        198,
        190,
        193
      ]
    }
  },
  {
    "website": "https://www.example.com",
    "result": {
      "withRecording": [
        11,
        12,
        10,
        10,
        11,
        10,
        12
      ],
      "withoutRecording": [
        8,
        8,
        8,
        11,
        9,
        12,
        8,
        8,
        10,
        8,
        9,
        8,
        10
      ]
    }
  }
]
```
