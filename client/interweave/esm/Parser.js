function _extends() { _extends = Object.assign || function (target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i]; for (var key in source) { if (Object.prototype.hasOwnProperty.call(source, key)) { target[key] = source[key]; } } } return target; }; return _extends.apply(this, arguments); }

function _objectWithoutPropertiesLoose(source, excluded) { if (source == null) return {}; var target = {}; var sourceKeys = Object.keys(source); var key, i; for (i = 0; i < sourceKeys.length; i++) { key = sourceKeys[i]; if (excluded.indexOf(key) >= 0) continue; target[key] = source[key]; } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

import React from '../../es-react/dev/react.js';
import escapeHtml from '../../escape-html';
import Element from './Element.js';
import StyleFilter from './StyleFilter.js';
import { FILTER_DENY, FILTER_CAST_NUMBER, FILTER_CAST_BOOL, FILTER_NO_CAST, TAGS, BANNED_TAG_LIST, ALLOWED_TAG_LIST, ATTRIBUTES, ATTRIBUTES_TO_PROPS } from './constants.js';
var ELEMENT_NODE = 1;
var TEXT_NODE = 3;
var INVALID_ROOTS = /^<(!doctype|(html|head|body)(\s|>))/i;
var ALLOWED_ATTRS = /^(aria\x2D|data\x2D|[0-9A-Z_a-z\u017F\u212A]+:)/i;

var Parser = function () {
  function Parser(markup, props, matchers, filters) {
    if (props === void 0) {
      props = {};
    }

    if (matchers === void 0) {
      matchers = [];
    }

    if (filters === void 0) {
      filters = [];
    }

    _defineProperty(this, "allowed", void 0);

    _defineProperty(this, "banned", void 0);

    _defineProperty(this, "blocked", void 0);

    _defineProperty(this, "doc", void 0);

    _defineProperty(this, "content", []);

    _defineProperty(this, "props", void 0);

    _defineProperty(this, "matchers", void 0);

    _defineProperty(this, "filters", void 0);

    _defineProperty(this, "keyIndex", void 0);

    if ("production" !== window.process.env.NODE_ENV) {
      if (markup && typeof markup !== 'string') {
        throw new TypeError('Interweave parser requires a valid string.');
      }
    }

    this.props = props;
    this.matchers = matchers;
    this.filters = [].concat(filters, [new StyleFilter()]);
    this.keyIndex = -1;
    this.doc = this.createDocument(markup || '');
    this.allowed = new Set(props.allowList || ALLOWED_TAG_LIST);
    this.banned = new Set(BANNED_TAG_LIST);
    this.blocked = new Set(props.blockList);
  }

  var _proto = Parser.prototype;

  _proto.applyAttributeFilters = function applyAttributeFilters(name, value) {
    return this.filters.reduce(function (nextValue, filter) {
      return nextValue !== null && typeof filter.attribute === 'function' ? filter.attribute(name, nextValue) : nextValue;
    }, value);
  };

  _proto.applyNodeFilters = function applyNodeFilters(name, node) {
    return this.filters.reduce(function (nextNode, filter) {
      return nextNode !== null && typeof filter.node === 'function' ? filter.node(name, nextNode) : nextNode;
    }, node);
  };

  _proto.applyMatchers = function applyMatchers(string, parentConfig) {
    var _this = this;

    var elements = [];
    var props = this.props;
    var matchedString = string;
    var parts = null;
    this.matchers.forEach(function (matcher) {
      var tagName = matcher.asTag().toLowerCase();

      var config = _this.getTagConfig(tagName);

      if (props[matcher.inverseName] || !_this.isTagAllowed(tagName)) {
        return;
      }

      if (!_this.canRenderChild(parentConfig, config)) {
        return;
      }

      while (parts = matcher.match(matchedString)) {
        var _ref = parts,
            match = _ref.match,
            partProps = _objectWithoutPropertiesLoose(_ref, ["match"]);

        matchedString = matchedString.replace(match, "#{{" + elements.length + "}}#");
        _this.keyIndex += 1;
        var element = matcher.createElement(match, _extends({}, props, partProps, {
          key: _this.keyIndex
        }));

        if (element) {
          elements.push(element);
        }
      }
    });

    if (elements.length === 0) {
      return matchedString;
    }

    var matchedArray = [];
    var lastIndex = 0;

    while (parts = matchedString.match(/#\{\{(\d+)\}\}#/)) {
      var _ref2 = parts,
          no = _ref2[1];
      var _ref3 = parts,
          _ref3$index = _ref3.index,
          index = _ref3$index === void 0 ? 0 : _ref3$index;

      if (lastIndex !== index) {
        matchedArray.push(matchedString.slice(lastIndex, index));
      }

      matchedArray.push(elements[parseInt(no, 10)]);
      lastIndex = index + parts[0].length;
      matchedString = matchedString.replace("#{{" + no + "}}#", "%{{" + no + "}}%");
    }

    if (lastIndex < matchedString.length) {
      matchedArray.push(matchedString.slice(lastIndex));
    }

    return matchedArray;
  };

  _proto.canRenderChild = function canRenderChild(parentConfig, childConfig) {
    if (!parentConfig.tagName || !childConfig.tagName) {
      return false;
    }

    if (parentConfig.void) {
      return false;
    }

    if (parentConfig.children.length > 0) {
      return parentConfig.children.includes(childConfig.tagName);
    }

    if (parentConfig.invalid.length > 0 && parentConfig.invalid.includes(childConfig.tagName)) {
      return false;
    }

    if (childConfig.parent.length > 0) {
      return childConfig.parent.includes(parentConfig.tagName);
    }

    if (!parentConfig.self && parentConfig.tagName === childConfig.tagName) {
      return false;
    }

    return Boolean(parentConfig && parentConfig.content & childConfig.type);
  };

  _proto.convertLineBreaks = function convertLineBreaks(markup) {
    var _this$props = this.props,
        noHtml = _this$props.noHtml,
        disableLineBreaks = _this$props.disableLineBreaks;

    if (noHtml || disableLineBreaks || markup.match(/<((?:\/[a-z ]+)|(?:[a-z ]+\/))>/gi)) {
      return markup;
    }

    var nextMarkup = markup.replace(/\r\n/g, '\n');
    nextMarkup = nextMarkup.replace(/\n{3,}/g, '\n\n\n');
    nextMarkup = nextMarkup.replace(/\n/g, '<br/>');
    return nextMarkup;
  };

  _proto.createDocument = function createDocument(markup) {
    var doc = document.implementation.createHTMLDocument('Interweave');

    if (markup.match(INVALID_ROOTS)) {
      if ("production" !== process.env.NODE_ENV) {
        throw new Error('HTML documents as Interweave content are not supported.');
      }
    } else {
      doc.body.innerHTML = this.convertLineBreaks(this.props.escapeHtml ? escapeHtml(markup) : markup);
    }

    return doc;
  };

  _proto.extractAttributes = function extractAttributes(node) {
    var _this2 = this;

    var allowAttributes = this.props.allowAttributes;
    var attributes = {};
    var count = 0;

    if (node.nodeType !== ELEMENT_NODE || !node.attributes) {
      return null;
    }

    Array.from(node.attributes).forEach(function (attr) {
      var name = attr.name,
          value = attr.value;
      var newName = name.toLowerCase();
      var filter = ATTRIBUTES[newName] || ATTRIBUTES[name];

      if (!_this2.isSafe(node)) {
        return;
      }

      if (!newName.match(ALLOWED_ATTRS)) {
        if (!allowAttributes && (!filter || filter === FILTER_DENY) || newName.match(/^on/) || value.replace(/(\s|\0|&#x0(9|A|D);)/, '').match(/(javascript|vbscript|livescript|xss):/i)) {
          return;
        }
      }

      var newValue = newName === 'style' ? _this2.extractStyleAttribute(node) : value;

      if (filter === FILTER_CAST_BOOL) {
        newValue = true;
      } else if (filter === FILTER_CAST_NUMBER) {
        newValue = parseFloat(newValue);
      } else if (filter !== FILTER_NO_CAST) {
        newValue = String(newValue);
      }

      attributes[ATTRIBUTES_TO_PROPS[newName] || newName] = _this2.applyAttributeFilters(newName, newValue);
      count += 1;
    });

    if (count === 0) {
      return null;
    }

    return attributes;
  };

  _proto.extractStyleAttribute = function extractStyleAttribute(node) {
    var styles = {};

    var camelCase = function camelCase(match, letter) {
      return letter.toUpperCase();
    };

    Array.from(node.style).forEach(function (key) {
      var value = node.style[key];
      styles[key.replace(/-([a-z])/g, camelCase)] = value;
    });
    return styles;
  };

  _proto.getTagConfig = function getTagConfig(tagName) {
    var common = {
      children: [],
      content: 0,
      invalid: [],
      parent: [],
      self: true,
      tagName: '',
      type: 0,
      void: false
    };

    if (TAGS[tagName]) {
      return _extends({}, common, TAGS[tagName], {
        tagName: tagName
      });
    }

    return common;
  };

  _proto.isSafe = function isSafe(node) {
    if (typeof HTMLAnchorElement !== 'undefined' && node instanceof HTMLAnchorElement) {
      var href = node.getAttribute('href');

      if (href && href.charAt(0) === '#') {
        return true;
      }

      var protocol = node.protocol.toLowerCase();
      return protocol === ':' || protocol === 'http:' || protocol === 'https:' || protocol === 'mailto:';
    }

    return true;
  };

  _proto.isTagAllowed = function isTagAllowed(tagName) {
    if (this.banned.has(tagName) || this.blocked.has(tagName)) {
      return false;
    }

    return this.props.allowElements || this.allowed.has(tagName);
  };

  _proto.parse = function parse() {
    return this.parseNode(this.doc.body, this.getTagConfig('body'));
  };

  _proto.parseNode = function parseNode(parentNode, parentConfig) {
    var _this3 = this;

    var _this$props2 = this.props,
        noHtml = _this$props2.noHtml,
        noHtmlExceptMatchers = _this$props2.noHtmlExceptMatchers,
        allowElements = _this$props2.allowElements,
        transform = _this$props2.transform;
    var content = [];
    var mergedText = '';
    Array.from(parentNode.childNodes).forEach(function (node) {
      if (node.nodeType === ELEMENT_NODE) {
        var tagName = node.nodeName.toLowerCase();

        var config = _this3.getTagConfig(tagName);

        if (mergedText) {
          content.push(mergedText);
          mergedText = '';
        }

        var nextNode = _this3.applyNodeFilters(tagName, node);

        if (!nextNode) {
          return;
        }

        var children;

        if (transform) {
          _this3.keyIndex += 1;
          var _key = _this3.keyIndex;
          children = _this3.parseNode(nextNode, config);
          var transformed = transform(nextNode, children, config);

          if (transformed === null) {
            return;
          } else if (typeof transformed !== 'undefined') {
            content.push(React.cloneElement(transformed, {
              key: _key
            }));
            return;
          }

          _this3.keyIndex = _key - 1;
        }

        if (_this3.banned.has(tagName)) {
          return;
        }

        if (!(noHtml || noHtmlExceptMatchers && tagName !== 'br') && _this3.isTagAllowed(tagName) && (allowElements || _this3.canRenderChild(parentConfig, config))) {
          _this3.keyIndex += 1;

          var attributes = _this3.extractAttributes(nextNode);

          var elementProps = {
            tagName: tagName
          };

          if (attributes) {
            elementProps.attributes = attributes;
          }

          if (config.void) {
            elementProps.selfClose = config.void;
          }

          content.push(React.createElement(Element, _extends({}, elementProps, {
            key: _this3.keyIndex
          }), children || _this3.parseNode(nextNode, config)));
        } else {
          content = content.concat(_this3.parseNode(nextNode, config.tagName ? config : parentConfig));
        }
      } else if (node.nodeType === TEXT_NODE) {
        var text = noHtml && !noHtmlExceptMatchers ? node.textContent : _this3.applyMatchers(node.textContent || '', parentConfig);

        if (Array.isArray(text)) {
          content = content.concat(text);
        } else {
          mergedText += text;
        }
      }
    });

    if (mergedText) {
      content.push(mergedText);
    }

    return content;
  };

  return Parser;
}();

export { Parser as default };