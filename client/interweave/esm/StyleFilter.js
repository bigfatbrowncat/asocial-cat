function _inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; subClass.__proto__ = superClass; }

import Filter from './Filter.js';
var INVALID_STYLES = /(url|image|image-set)\(/i;

var StyleFilter = function (_Filter) {
  _inheritsLoose(StyleFilter, _Filter);

  function StyleFilter() {
    return _Filter.apply(this, arguments) || this;
  }

  var _proto = StyleFilter.prototype;

  _proto.attribute = function attribute(name, value) {
    if (name === 'style') {
      Object.keys(value).forEach(function (key) {
        if (String(value[key]).match(INVALID_STYLES)) {
          delete value[key];
        }
      });
    }

    return value;
  };

  return StyleFilter;
}(Filter);

export { StyleFilter as default };