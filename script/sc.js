require('./global.js');

var spawn = require('child_process').spawn;
var path = require('path');

function $nop() {}

function Feed(sc, id, channel) {
  this.sc = sc;
  this.id = id;
  this.channel = channel;
}
$extend(Feed, {
  setTuneColor: function(hsb) {
    this.sc.request(['tunecolor', this.id + ':' + this.channel, 
        hsb.join(' ')].join(' '));
  },
  setMatrix: function(matrix) {
    this.sc.request(['matrix', this.id + ':' + this.channel, 
        matrix.join(' ')].join(' '));
  }
});

function Source(sc, id, channels) {
  var source = new Array(channels);
  source.sc = sc;
  source.id = id;
  for (var i = 0; i < channels; i++)
    source[i] = new Feed(sc, id, i+1);
  return $wrap(source, Source);
}
$extend(Source, {
  close: function() {
    this.sc.request(['close', this.id].join(' '));
  },
  setDelay: function(msec) {
    this.sc.request(['delay', this.id, msec].join(' '));
  },
  setTuneColor: function(hsb) {
    this.sc.request(['tunecolor', this.id, hsb.join(' ')].join(' '));
  },
  setMatrix: function(matrix) {
    this.sc.request(['matrix', this.id, matrix.join(' ')].join(' '));
  }
});

function sendRequest(sc) {

  this.processedRequestId = this.stdinQueue[0].id;
  console.error(this.stdinQueue[0].command);
  this.stdin.write(this.stdinQueue[0].command + '\n');

}

function trySendRequest() {

  if (!this.ready || this.stdinQueue.length == 0 || 
      this.stdinQueue[0].id <= this.processedRequestId)
    return;
  if (this.stdinQueue[0].wait) {
    if (!this.stdinQueue[0].timer)
      this.stdinQueue[0].timer = setTimeout(sendRequest.bind(this), 
        this.stdinQueue[0].wait * 1000);
  } else {
    sendRequest.call(this);
  }

}

function onData(queue, data) {

  if (this.processedRequestId < 0) {
    if (!this.timer) {
      var self = this;
      this.timer = setTimeout(function() {
        self.ready = true;
        trySendRequest.call(self);
      }, 2000);
    }
    return;
  }

  var parts = data.split(String.fromCharCode(31));
  if (queue.buffer) {
    parts[0] = queue.buffer + parts[0];
    queue.buffer = null;
  }
  for (var i = 0; i < parts.length; i++) {
    if (i + 1 < parts.length)
      queue.push(parts[i]);
    else if (parts[i])
      queue.buffer = parts[i];
  }
  for (var i = 0, count = Math.min(this.stdinQueue.length, 
      this.stdoutQueue.length, this.stderrQueue.length); i < count; i++) {
    var request = this.stdinQueue.shift();
    var out = this.stdoutQueue.shift();
    var err = this.stderrQueue.shift();
    if (request.callback)
      request.callback(out == "Failed.\n", out, err, request.command);
  }
  trySendRequest.call(this);

}

function Sc() {

  this.stdinQueue = [];
  this.stdoutQueue = [];
  this.stderrQueue = [];

  this.requestId = 0;
  this.processedRequestId = -1;
  this.sourceId = 0;
  this.ready = false;
  this.timer;

  this.rootPath = path.resolve(path.dirname(module.filename), '..');

}
$extend(Sc, {
  
  start: function(mode, width, height) {
    if (this.proc)
      return;
    this.proc = spawn(path.resolve(this.rootPath, 'sc.' + mode), 
        ['--spawn', '--size=' + width + 'x' + height], {
          cwd: this.rootPath
        });
    this.stdin = this.proc.stdin;
    this.stdout = this.proc.stdout;
    this.stderr = this.proc.stderr;
    this.stdout.setEncoding('utf8');
    this.stdout.on('data', onData.bind(this, this.stdoutQueue));
    this.stderr.setEncoding('utf8');
    this.stderr.on('data', onData.bind(this, this.stderrQueue));
    this.proc.on('exit', this.end.bind(this))
  },
  
  end: function() {
    if (!this.proc)
      return;
    this.ready = false;
    if (this.stdin.writable)
      this.stdin.end();
    for (var i = 0; i < this.stdinQueue.length; i++) {
      var request = this.stdinQueue.shift();
      if (request.callback)
        request.callback(true, null, null, request.command);
    }
    this.requestId = 0;
    this.processedRequestId = -1;
    this.stdoutQueue.length = 0;
    this.stdoutQueue.buffer = null;
    this.stderrQueue.length = 0;
    this.stderrQueue.buffer = null;
    this.proc = null;
  },
  
  request: function(command, callback) {
    if (command instanceof Array) {
      for (var i = 0; i < command.length; i++)
        this.request(command[i], callback);
      return;
    }
    this.stdinQueue.push({
      id: this.requestId++,
      command: command,
      callback: callback
    });
    trySendRequest.call(this);
  },
  
  immediateRequest: function(command, callback) {
    this.stdinQueue.unshift({
      id: this.processedRequestId--,
      command: command,
      callback: callback
    });
    trySendRequest.call(this);
  },
  
  openURI: function(uri, callback) {
    var id = 'u' + this.sourceId++;
    this.request(['open', id, uri].join(' '), callback);
    return new Source(this, id, 1);
  },
  
  openCamera: function(cameraId, channels, callback) {
    var id = 'u' + this.sourceId++;
    this.request(
        ['open', id, ['camera', cameraId, channels].join(':')].join(' '),
        callback);
    return new Source(this, id, channels);
  },
  
  perfInfo: function(callback) {
    function handler(err, res, other, cmd) {
      res = res.split('\n');
      var summary = res[0].split(' ').map(function(v) {
        return parseFloat(v); 
      });
      var detail = res.slice(1, -1).map(function(line) {
        return line.split(' ');
      });
      callback(summary, detail);
    }
    this.request('perfinfo', handler);
  },
  
  evaluate: function(times, firstWait, betweenWait, callback) {
    var summaries = [];
    var details = [];
    function handler(render, sources) {
      summaries.push(render);
      details.push(sources);
      if (--times == 0)
        callback(summaries, details);
    }
    this.sleep(firstWait * 1000);
    this.perfInfo(handler);
    for (var i = 1; i < times; i++) {
      this.sleep(betweenWait * 1000);
      this.perfInfo(handler);
    }
  },

  quit: function(callback) {
    this.request('quit', callback);
  },

  reset: function(callback) {
    this.request('reset', callback);
  },

  setWorldMatrix: function(matrix) {
    this.request(['worldmatrix', matrix.join(' ')].join(' '));
  },

  setPostMatrix: function(matrix) {
    this.request(['postmatrix', matrix.join(' ')].join(' '));
  },

  setBgColor: function(color) {
    if (color instanceof Array)
      this.request(['bgcolor', color.join(' ')].join(' '));
    else if (typeof color == "number")
      this.request(['bgcolor', '#' + color.toString(16)].join(' '));
    else
      this.request(['bgcolor', color].join(' '));
  },

  setCameraMode: function(mode) {
    this.request(['cameramode', mode].join(' '));
  },

  snapshot: function(path, callback) {
    this.request(['snapshot', path].join(' '), callback);
  },
  
  sleep: function(msec, callback) {
    this.request(['sleep', msec].join(' '), callback);
  }

});
$define(Sc, {
  kOrthographic: 'orthographic',
  kPerspective: 'perspective'
});

module.exports = Sc;
