const puppeteer = require('puppeteer');

class CDP {
    constructor(cdp, enables = [], listeners = []) {
        this.cdp = cdp;
        this.enable = this.enable.bind(this);
        this.listen = this.listen.bind(this);


        this.enable(enables);
        listeners.forEach(({event, cb}) => this.listen(event, cb));
        cdp.removeAllListeners('Target.attachedToTarget');
        cdp.send('Target.setAutoAttach', {
            autoAttach: true,
            waitForDebuggerOnStart: true, // override default setup in puppeteer
            flatten: true,
        })
    }

    enable(enables) {

        for (const enable of enables) {
            if (this.cdp._targetType === 'service_worker')
                continue;
            this.cdp.send(enable);
        }
    }

    listen(event, cb) {
        this.cdp.on(event, cb);

    }

    async exec(event, params) {
        return await this.cdp.send(event, params);
    }
}

class DevTools {
    constructor(enables = [], listeners = []) {
        this.targetMap = new Map();
        this.enables = enables;
        this.listens = listeners;
        this._init = this._init.bind(this);
        this.launch = this.launch.bind(this);
        this.close = this.close.bind(this);
    }

    async _init() {
        // handle current opened pages
        const pages = await this.browser.pages();
        pages.forEach(page => {this.targetMap.set(page.target(), new CDP(page._client, this.enables, this.listens));})
        // handle newly created tabs
        this.browser.on('targetcreated', async target => {
            // we skip devtools
            if (target._targetInfo.url.startsWith('devtools://')) return;
            const page = await target.page();

            if(page) {
                this.targetMap.set(page.target(), new CDP(page._client, this.enables, this.listens));
                page._client.on('Target.attachedToTarget', async event=> {
                    const {sessionId, waitingForDebugger, targetInfo} = event;
                    if (waitingForDebugger) {
                        // find the session
                        const _cdp = this.browser._connection.session(sessionId);
                        const cdp = new CDP(_cdp, this.enables, this.listens);
                        _cdp.send('Runtime.runIfWaitingForDebugger');
                        this.targetMap.set(target, cdp);
                    }
                })
            }
        });

        this.browser.on('targetdestroyed', target => {
            // logger.info('Deleted target ' + target._targetInfo.url);
            this.targetMap.delete(target);
        });
    }

    async launch(options, customized=false) {
        const {browserURL} = options;
        this.browser = browserURL ? await puppeteer.connect(options) : await puppeteer.launch(options);
        if (customized) {
            await this._init();
        }

        return this.browser;
    }

    async close() {
        await this.browser.close();
    }

}

module.exports.DevTools = DevTools;