# CCS Reviewers

1. The location of the major components of the record and replay engine: `third_party/blink/renderer/core/inspector/forensics`
2. The location of the networking record and replay components: `third_party/blink/renderer/platform/forensics`
3. Directions on how to setup are below.

# ![Logo](chrome/app/theme/chromium/product_logo_64.png) Chromium

Chromium is an open-source browser project that aims to build a safer, faster,
and more stable way for all users to experience the web.

The project's web site is https://www.chromium.org.

Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory Structure
](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).

For historical reasons, there are some small top level directories. Now the
guidance is that new top level directories are for product (e.g. Chrome,
Android WebView, Ash). Even if these products have multiple executables, the
code should be in subdirectories of the product.

# JSCapsule

## Chromium Version

```
Chromium=83.0.4103.97
V8=8.3.110.9
```

## Code Repo

We now have two branches. One is `master` which only contains the instrumented files.
The other is `dev` which is an orphan branch of Chromium version stated at the beginning.

### Master
The structure of the **master** is as below. The `./src` directory is exactly the same
structure of `path/to/chromium/src` but only has instrumented files.

```
.
|-- src/
|-- utils/
```


### Dev

We don't keep the git history of Chromium in this branch, so we need a procedure to
work with this branch, AKA, the working branch.
1. Fetch the Chromium code by following the [instruction](https://www.chromium.org/developers/how-tos/get-the-code).
2. Checkout to the correct version by `git checkout tags/83.0.4103.97` and run `gclient sync -D` afterwards.
3. Then add one more remote to Chromium's repo by `git remote add github git@github.com:jallen89/JSCapsule.git`.
4. Do a `git fetch github dev` to get the information of the added repo.
5. Now you can checkout to the `dev` branch by doing `git checkout -b dev github/dev`.
6. When you need to pull/push changes, remember to do `git pull/push dev github/dev`.
7. If you want to work in a different branch, just branch from the `github/dev`. When you finish the work, please submit a Pull Request
   and merge the changes into `github/dev`.

**It's important that every time you need to run `gclient sync -D` in `tags/83.0.4103.97` when you switch to other Chromium tags or master**

## Misc

1. The log now writes to file. `chromium/src/out/[Debug|Release|Default]/path/to/chromium --no-sandbox --forensic-log-file=/path/to/logfile` to start recording. `/path/to/logfile` should be accessible from chromium. If `--forensic-log-file` is not provided, it writes to `/tmp/jscapsule.log` by default.

2. JSCapsule cannot be compiled using component builds. You need to update the  `args.gn` file to disable component builds. In `args.gn` set `is_component_build=false`. 



## Recording 

The script `record.sh` is used for recording a website: 

`./record.sh https://www.example.com example` 

This would record `https://www.example.com` (NOTE: always used the entire URL) and store the logs in `example<pid>`. 

## Replaying

To replay a website, open two terminals. 

In the first terminal, run the following command: 

`./replay.sh example.replay &> example.logs`

In the second terminal, run the following command in the __forensic_utils__ directory:

`./run.py load-logs ../example<pid>`

__NOTE__: If the recording used multiple process (e.g., multiple tabs or iframes in w/ different origins), you will need to run the following command

`./run.py load-logs --merge ../example<pid>*`






