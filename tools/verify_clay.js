'use strict';

// Exercise the built phone bundle, then emit actual Clay pages for browser tests.
// Run after `pebble build`: node tools/verify_clay.js
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const vm = require('vm');
const root = path.resolve(__dirname, '..');
const bundle = fs.readFileSync(path.join(root, 'build/pebble-js-app.js'), 'utf8');
const output = '/tmp/neon-clay';
fs.mkdirSync(output, {recursive: true});

function browserCheck() {
  var clay = this;
  window.neonTestClay = clay;
  clay.on(clay.EVENTS.AFTER_BUILD, function() {
    setTimeout(function() {
      var platform = clay.meta.activeWatchInfo.platform;
      var bw = ['aplite', 'diorite', 'flint'].indexOf(platform) !== -1;
      function check(value, message) {
        if (!value) { throw new Error(message); }
      }
      var result;
      try {
        var digit = clay.getItemByMessageKey('DIGIT_COLOR');
        var background = clay.getItemByMessageKey('BACKGROUND_COLOR');
        var glow = clay.getItemByMessageKey('GLOW_COLOR');
        check(!!digit && !!background, 'Missing color controls');
        check(!!glow === !bw, 'Glow must be available only on color watches');
        var choices = [];
        for (var r = 0; r < 4; r++) {
          for (var g = 0; g < 4; g++) {
            for (var b = 0; b < 4; b++) {
              var color = r*85*65536 + g*85*256 + b*85;
              digit.set(color);
              var rounded = digit.get();
              if (choices.indexOf(rounded) === -1) { choices.push(rounded); }
            }
          }
        }
        check(choices.length === (bw ? 2 : 64), 'Unexpected number of selectable colors');
        if (bw) {
          check(choices.indexOf(0) !== -1 && choices.indexOf(16777215) !== -1,
                'Monochrome choices must be black and white');
        }
        digit.set(bw ? 0 : 0xAAFFFF);
        background.set(bw ? 0xFFFFFF : 0x000055);
        if (glow) { glow.set(0x00AAFF); }
        var settings = clay.serialize();
        check(settings.DIGIT_COLOR.value === (bw ? 0 : 0xAAFFFF), 'Digit value');
        check(settings.BACKGROUND_COLOR.value === (bw ? 0xFFFFFF : 0x000055), 'Background value');
        check(Object.keys(settings).length === (bw ? 2 : 3), 'Hidden settings leaked');
        result = {platform: platform, pass: true, choices: choices.length, settings: settings};
      } catch (error) {
        result = {platform: platform, pass: false, error: error.message};
      }
      document.documentElement.innerHTML = '<head><title>Neon Clay verification</title></head>' +
        '<body><pre id="result">' + JSON.stringify(result) + '</pre></body>';
    }, 0);
  });
}

for (const platform of ['aplite', 'basalt', 'diorite', 'flint', 'emery']) {
  const events = {};
  let url;
  let sent;
  const saved = {};
  const context = {
    console,
    localStorage: {
      getItem: key => saved[key] || null,
      setItem: (key, value) => { saved[key] = value; }
    },
    Pebble: {
      platform: 'android',
      addEventListener: (event, fn) => { events[event] = fn; },
      getActiveWatchInfo: () => ({platform, firmware: {major: 4, minor: 9, patch: 0}}),
      getAccountToken: () => '', getWatchToken: () => '',
      openURL: value => { url = value; },
      sendAppMessage: (message, success) => { sent = message; success(); }
    }
  };
  vm.runInNewContext(bundle, context);
  events.showConfiguration();
  assert(url.startsWith('data:text/html'));
  const html = decodeURIComponent(url.slice(url.indexOf(',')+1));
  fs.writeFileSync(path.join(output, platform+'-preview.html'), html);
  assert(html.includes('window.customFn=function(){}'));
  fs.writeFileSync(path.join(output, platform+'.html'),
    html.replace('window.customFn=function(){}', 'window.customFn='+browserCheck.toString()));

  events.webviewclosed({response: ''});
  assert.strictEqual(sent, undefined, 'Cancel must not send settings');
  const values = {DIGIT_COLOR: {value: 0xAAFFFF}, GLOW_COLOR: {value: 0x00AAFF},
                  BACKGROUND_COLOR: {value: 0x000055}};
  events.webviewclosed({response: encodeURIComponent(JSON.stringify(values))});
  assert.deepStrictEqual(JSON.parse(JSON.stringify(sent)),
    {'0': 0xAAFFFF, '1': 0x00AAFF, '2': 0x000055});
  assert(saved['clay-settings']);
  events.showConfiguration();
  assert(decodeURIComponent(url).includes('11206655'), 'Saved color must be restored');
  console.log(platform + ': bundled configuration open/save/cancel/restore passed');
}
console.log('Browser test pages: ' + output + '/{platform}.html');
