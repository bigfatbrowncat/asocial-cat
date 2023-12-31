import _pt from "../../es-react/dev/prop-types.js";

function _inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; subClass.__proto__ = superClass; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

import React from '../../es-react/dev/react.js';

var Element = function (_React$PureComponent) {
  _inheritsLoose(Element, _React$PureComponent);

  function Element() {
    return _React$PureComponent.apply(this, arguments) || this;
  }

  var _proto = Element.prototype;

  _proto.render = function render() {
    var _this$props = this.props,
        _this$props$attribute = _this$props.attributes,
        attributes = _this$props$attribute === void 0 ? {} : _this$props$attribute,
        _this$props$children = _this$props.children,
        children = _this$props$children === void 0 ? null : _this$props$children,
        _this$props$selfClose = _this$props.selfClose,
        selfClose = _this$props$selfClose === void 0 ? false : _this$props$selfClose,
        Tag = _this$props.tagName;
    return selfClose ? React.createElement(Tag, attributes) : React.createElement(Tag, attributes, children);
  };

  return Element;
}(React.PureComponent);

_defineProperty(Element, "propTypes", {
  attributes: _pt.any,
  children: _pt.node,
  selfClose: _pt.bool,
  tagName: _pt.string.isRequired
});

export { Element as default };