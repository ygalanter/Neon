'use strict';

module.exports = [
  {type: 'heading', defaultValue: 'Neon'},
  {type: 'text', defaultValue: 'Hours above, minutes below. Neon follows your watch’s 12- or 24-hour time setting.'},
  {
    type: 'section',
    items: [
      {type: 'heading', defaultValue: 'Colors'},
      {
        type: 'color', messageKey: 'DIGIT_COLOR', label: 'Digits',
        defaultValue: 'FFFF55', allowGray: false
      },
      {
        type: 'color', messageKey: 'GLOW_COLOR', label: 'Neon glow',
        defaultValue: 'FFFF00', capabilities: ['COLOR']
      },
      {
        type: 'color', messageKey: 'BACKGROUND_COLOR', label: 'Background',
        defaultValue: '000055', allowGray: false
      },
      {
        type: 'text', capabilities: ['BW'],
        defaultValue: 'Choose black or white. If both colors match, the digits automatically use the opposite color so the time stays visible.'
      }
    ]
  },
  {type: 'submit', defaultValue: 'Save settings'}
];
