import _pt from "../../es-react/dev/prop-types.js";

function _objectWithoutPropertiesLoose(source, excluded) { if (source == null) return {}; var target = {}; var sourceKeys = Object.keys(source); var key, i; for (i = 0; i < sourceKeys.length; i++) { key = sourceKeys[i]; if (excluded.indexOf(key) >= 0) continue; target[key] = source[key]; } return target; }

function _inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; subClass.__proto__ = superClass; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

import React from '../../es-react/dev/react.js';
import Element from './Element.js';
import Parser from './Parser.js';

var Markup = function (_React$PureComponent) {
  _inheritsLoose(Markup, _React$PureComponent);

  function Markup() {
    return _React$PureComponent.apply(this, arguments) || this;
  }

  var _proto = Markup.prototype;

  _proto.getContent = function getContent() {
    var _this$props = this.props,
        content = _this$props.content,
        emptyContent = _this$props.emptyContent,
        parsedContent = _this$props.parsedContent,
        tagName = _this$props.tagName,
        props = _objectWithoutPropertiesLoose(_this$props, ["content", "emptyContent", "parsedContent", "tagName"]);

    if (parsedContent) {
      return parsedContent;
    }

    var markup = new Parser(content || '', props).parse();
    return markup.length > 0 ? markup : null;
  };

  _proto.render = function render() {
    var content = this.getContent() || this.props.emptyContent;
    var tag = this.props.tagName || 'div';
    return tag === 'fragment' ? React.createElement(React.Fragment, null, content) : React.createElement(Element, {
      tagName: tag
    }, content);
  };

  return Markup;
}(React.PureComponent);

_defineProperty(Markup, "propTypes", {
  content: _pt.oneOfType([_pt.string, _pt.oneOf([null])]),
  emptyContent: _pt.node,
  parsedContent: _pt.node,
  tagName: _pt.oneOfType([_pt.oneOf(['fragment']), _pt.string])
});

_defineProperty(Markup, "defaultProps", {
  content: '',
  emptyContent: null,
  parsedContent: null,
  tagName: 'div'
});

export { Markup as default };