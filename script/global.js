(function(outter) {

function $define(object, prototype) {
  var setterGetterPattern = /^(set|get)([A-Z])(.*)/;
  var setterGetters = {};
  for (var key in prototype) {
    var matches = setterGetterPattern.exec(key)
    if (matches) {
      var name = matches[2].toLowerCase() + matches[3];
      if (!setterGetters.hasOwnProperty(name))
        setterGetters[name] = {};
      setterGetters[name][matches[1]] = prototype[key];
    }
    Object.defineProperty(object, key, {
      value: prototype[key],
      writeable: false
    });
  }
  Object.defineProperties(object, setterGetters);
}

$define(outter, {
  
  $nop: function() { },

  $define: $define,
  
  $extend: function(klass, prototype) {
    $define(klass.prototype, prototype);
  },
  
  $inherit: function(klass, from, prototype) {
    klass.prototype = {
      constructor: klass,
      __proto__: from.__proto__
    };
    if (prototype)
      $extend(klass, prototype);
  },

  $bind: function(org, self) {
    var obj = {};
    for (var key in org)
      obj[key] = org[key].bind(self);
    return obj;
  },

  $default: function(val, def) {
    return val === undefined ? def : val;
  },

  $random: function(val) {
    return Math.floor(Math.random() * val);
  },

  $limit: function(val, min, max) {
    return Math.min(Math.max(val, min), max);
  },

  $wrap: function(data, klass) {
    data.__proto__ = klass.prototype;
    if (klass.__wrap)
      klass.__wrap(data);
    return data;
  }

});

$extend(String, {

  repeat: function(len) {
    var res = '';
    for (var i = 0; i < len; i++)
      res += this;
    return res;
  },
  
  paddingLeft: function(ch, len) {
    if (this.length < len)
      return ch.repeat(len - this.length) + this;
    else
      return this;
  },
  
  paddingRight: function(ch, len) {
    if (this.length < len)
      return this + ch.repeat(len - this.length);
    else
      return this;
  }
  
});

$extend(Number, {

  limit: function(lb, ub) {
    var rtn = Math.max(this, lb);
    if (ub != undefined) return Math.min(rtn, ub);
    return rtn;
  }
  
});

$extend(Array, {

  add: function(val) {
    if (this.indexOf(val) == -1)
      return this.push(val);
    return -1;
  },
  
  remove: function(val) {
    var idx = this.indexOf(val);
    if (idx == -1) return false;
    this.splice(idx, 1);
    return true;
  },
  
  equal: function(arr) {
    if (this.length != arr.length)
      return false;
    for (var i = 0; i < this.length; i++)
      if (this[i] != arr[i]) return false;
    return true;
  },
  
  clone: function() {
    return this.slice(0);
  }
  
});

})(global);

