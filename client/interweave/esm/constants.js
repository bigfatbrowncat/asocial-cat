function _extends() { _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; }; return _extends.apply(this, arguments); }

export var TYPE_FLOW = 1;
export var TYPE_SECTION = 1 << 1;
export var TYPE_HEADING = 1 << 2;
export var TYPE_PHRASING = 1 << 3;
export var TYPE_EMBEDDED = 1 << 4;
export var TYPE_INTERACTIVE = 1 << 5;
export var TYPE_PALPABLE = 1 << 6;
var tagConfigs = {
  a: {
    content: TYPE_FLOW | TYPE_PHRASING,
    self: false,
    type: TYPE_FLOW | TYPE_PHRASING | TYPE_INTERACTIVE | TYPE_PALPABLE
  },
  address: {
    invalid: ['h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'address', 'article', 'aside', 'section', 'div', 'header', 'footer'],
    self: false
  },
  audio: {
    children: ['track', 'source']
  },
  br: {
    type: TYPE_FLOW | TYPE_PHRASING,
    void: true
  },
  body: {
    content: TYPE_FLOW | TYPE_SECTION | TYPE_HEADING | TYPE_PHRASING | TYPE_EMBEDDED | TYPE_INTERACTIVE | TYPE_PALPABLE
  },
  button: {
    content: TYPE_PHRASING,
    type: TYPE_FLOW | TYPE_PHRASING | TYPE_INTERACTIVE | TYPE_PALPABLE
  },
  caption: {
    content: TYPE_FLOW,
    parent: ['table']
  },
  col: {
    parent: ['colgroup'],
    void: true
  },
  colgroup: {
    children: ['col'],
    parent: ['table']
  },
  details: {
    children: ['summary'],
    type: TYPE_FLOW | TYPE_INTERACTIVE | TYPE_PALPABLE
  },
  dd: {
    content: TYPE_FLOW,
    parent: ['dl']
  },
  dl: {
    children: ['dt', 'dd'],
    type: TYPE_FLOW
  },
  dt: {
    content: TYPE_FLOW,
    invalid: ['footer', 'header'],
    parent: ['dl']
  },
  figcaption: {
    content: TYPE_FLOW,
    parent: ['figure']
  },
  footer: {
    invalid: ['footer', 'header']
  },
  header: {
    invalid: ['footer', 'header']
  },
  hr: {
    type: TYPE_FLOW,
    void: true
  },
  img: {
    void: true
  },
  li: {
    content: TYPE_FLOW,
    parent: ['ul', 'ol', 'menu']
  },
  main: {
    self: false
  },
  ol: {
    children: ['li'],
    type: TYPE_FLOW
  },
  picture: {
    children: ['source', 'img'],
    type: TYPE_FLOW | TYPE_PHRASING | TYPE_EMBEDDED
  },
  rb: {
    parent: ['ruby', 'rtc']
  },
  rp: {
    parent: ['ruby', 'rtc']
  },
  rt: {
    content: TYPE_PHRASING,
    parent: ['ruby', 'rtc']
  },
  rtc: {
    content: TYPE_PHRASING,
    parent: ['ruby']
  },
  ruby: {
    children: ['rb', 'rp', 'rt', 'rtc']
  },
  source: {
    parent: ['audio', 'video', 'picture'],
    void: true
  },
  summary: {
    content: TYPE_PHRASING,
    parent: ['details']
  },
  table: {
    children: ['caption', 'colgroup', 'thead', 'tbody', 'tfoot', 'tr'],
    type: TYPE_FLOW
  },
  tbody: {
    parent: ['table'],
    children: ['tr']
  },
  td: {
    content: TYPE_FLOW,
    parent: ['tr']
  },
  tfoot: {
    parent: ['table'],
    children: ['tr']
  },
  th: {
    content: TYPE_FLOW,
    parent: ['tr']
  },
  thead: {
    parent: ['table'],
    children: ['tr']
  },
  tr: {
    parent: ['table', 'tbody', 'thead', 'tfoot'],
    children: ['th', 'td']
  },
  track: {
    parent: ['audio', 'video'],
    void: true
  },
  ul: {
    children: ['li'],
    type: TYPE_FLOW
  },
  video: {
    children: ['track', 'source']
  },
  wbr: {
    type: TYPE_FLOW | TYPE_PHRASING,
    void: true
  }
};

function createConfigBuilder(config) {
  return function (tagName) {
    tagConfigs[tagName] = _extends({}, config, tagConfigs[tagName]);
  };
}

['address', 'main', 'div', 'figure', 'p', 'pre'].forEach(createConfigBuilder({
  content: TYPE_FLOW,
  type: TYPE_FLOW | TYPE_PALPABLE
}));
['abbr', 'b', 'bdi', 'bdo', 'cite', 'code', 'data', 'dfn', 'em', 'i', 'kbd', 'mark', 'q', 'ruby', 'samp', 'strong', 'sub', 'sup', 'time', 'u', 'var'].forEach(createConfigBuilder({
  content: TYPE_PHRASING,
  type: TYPE_FLOW | TYPE_PHRASING | TYPE_PALPABLE
}));
['p', 'pre'].forEach(createConfigBuilder({
  content: TYPE_PHRASING,
  type: TYPE_FLOW | TYPE_PALPABLE
}));
['s', 'small', 'span', 'del', 'ins'].forEach(createConfigBuilder({
  content: TYPE_PHRASING,
  type: TYPE_FLOW | TYPE_PHRASING
}));
['article', 'aside', 'footer', 'header', 'nav', 'section', 'blockquote'].forEach(createConfigBuilder({
  content: TYPE_FLOW,
  type: TYPE_FLOW | TYPE_SECTION | TYPE_PALPABLE
}));
['h1', 'h2', 'h3', 'h4', 'h5', 'h6'].forEach(createConfigBuilder({
  content: TYPE_PHRASING,
  type: TYPE_FLOW | TYPE_HEADING | TYPE_PALPABLE
}));
['audio', 'canvas', 'iframe', 'img', 'video'].forEach(createConfigBuilder({
  type: TYPE_FLOW | TYPE_PHRASING | TYPE_EMBEDDED | TYPE_PALPABLE
}));
export var TAGS = Object.freeze(tagConfigs);
export var BANNED_TAG_LIST = ['applet', 'base', 'body', 'command', 'embed', 'frame', 'frameset', 'head', 'html', 'link', 'meta', 'noscript', 'object', 'script', 'style', 'title'];
export var ALLOWED_TAG_LIST = Object.keys(TAGS).filter(function (tag) {
  return tag !== 'canvas' && tag !== 'iframe';
});
export var FILTER_ALLOW = 1;
export var FILTER_DENY = 2;
export var FILTER_CAST_NUMBER = 3;
export var FILTER_CAST_BOOL = 4;
export var FILTER_NO_CAST = 5;
export var ATTRIBUTES = Object.freeze({
  alt: FILTER_ALLOW,
  cite: FILTER_ALLOW,
  class: FILTER_ALLOW,
  colspan: FILTER_CAST_NUMBER,
  controls: FILTER_CAST_BOOL,
  datetime: FILTER_ALLOW,
  default: FILTER_CAST_BOOL,
  disabled: FILTER_CAST_BOOL,
  dir: FILTER_ALLOW,
  height: FILTER_ALLOW,
  href: FILTER_ALLOW,
  id: FILTER_ALLOW,
  kind: FILTER_ALLOW,
  label: FILTER_ALLOW,
  lang: FILTER_ALLOW,
  loop: FILTER_CAST_BOOL,
  media: FILTER_ALLOW,
  muted: FILTER_CAST_BOOL,
  poster: FILTER_ALLOW,
  role: FILTER_ALLOW,
  rowspan: FILTER_CAST_NUMBER,
  scope: FILTER_ALLOW,
  sizes: FILTER_ALLOW,
  span: FILTER_CAST_NUMBER,
  style: FILTER_NO_CAST,
  src: FILTER_ALLOW,
  srclang: FILTER_ALLOW,
  srcset: FILTER_ALLOW,
  target: FILTER_ALLOW,
  title: FILTER_ALLOW,
  type: FILTER_ALLOW,
  width: FILTER_ALLOW
});
export var ATTRIBUTES_TO_PROPS = Object.freeze({
  class: 'className',
  colspan: 'colSpan',
  datetime: 'dateTime',
  rowspan: 'rowSpan',
  srclang: 'srcLang',
  srcset: 'srcSet'
});