var Sc = require('../sc.js');
var Matrix = require('../matrix.js');

var kWidth = parseInt(process.argv[2]) || 640;
var kHeight = parseInt(process.argv[3]) || 480;
var kSamplingTimes = 5;
var kFirstWait = 2;
var kBetweenWait = 2;

function formatResult(mode, n, row, col, profile, summaries, details) {

  console.log('Mode: %s, frames: %d, row: %d, col: %d, profile: %s', mode, n, row, col, profile);
  console.log('+--------+---------+-----------+------------+');
  console.log('| Sample |   fps   | Time Skew | Frame Skew |');
  console.log('+--------+---------+-----------+------------+');
  for (var i = 0; i < summaries.length; i++) {
    var datum = summaries[i];
    console.log('| %s | %s | %s | %s |',
        (i + 1).toString().paddingLeft(' ', 6),
        datum[0].toFixed(1).paddingLeft(' ', 7),
        datum[1].toFixed(1).paddingLeft(' ', 9),
        datum[2].toFixed(1).paddingLeft(' ', 10));
  }
  console.log('+--------+---------+-----------+------------+');
  for (var i = 0; i < details.length; i++) {
    console.log("Sample %d", i+1)
    console.log('+--------+---------+---------+-----------+------------+');
    console.log('| Stream |  E.fps  |  A.fps  | Time Skew | Frame Skew |');
    console.log('+--------+---------+---------+-----------+------------+');
    var detail = details[i];
    for (var j = 0; j < detail.length; j++) {
      var datum = detail[j];
      console.log('| %s | %s | %s | %s | %s |',
          datum[0].paddingRight(' ', 6),
          parseFloat(datum[1]).toFixed(1).paddingLeft(' ', 7),
          parseFloat(datum[2]).toFixed(1).paddingLeft(' ', 7),
          parseFloat(datum[3]).toFixed(1).paddingLeft(' ', 9),
          parseFloat(datum[4]).toFixed(1).paddingLeft(' ', 10));
    }
    console.log('+--------+---------+---------+-----------+------------+');
  }

}

function runTest(mode, callback) {

  var sc = new Sc();
  sc.start(mode, kWidth, kHeight);
  
  function bestRowCol(n, aspect) {
    var bestArea = 0;
    var res;
    for (var r = 1; r <= n; r++) {
      var c = Math.ceil(n / r);
      var width = kWidth / c;
      var height = width / aspect;
      if (kHeight / r < height) {
        height = kHeight / r;
        width = height * aspect;
      }
      if (width * height > bestArea) {
        bestArea = width * height;
        res = {
          row: r,
          col: c,
          width: width,
          height: height
        };
      }
    }
    return res;
  }

  function staticCamera(n, aspect) {
    sc.cameraMode = Sc.kOrthographic;
    var source = sc.openCamera(0, n);
    var parms = bestRowCol(n, aspect);
    for (var r = 0, i = 0; r < parms.row; r++)
    for (var c = 0; c < parms.col && i < n; c++, i++) {
      source[i].matrix = 
          new Matrix.Scale(parms.width/2, parms.height/2, 1)
          .translate((-kWidth + parms.width)/2, (kHeight - parms.height)/2, 0)
          .translate(c * parms.width, -r * parms.height, 0);
    }
    sc.evaluate(kSamplingTimes, kFirstWait, kBetweenWait, formatResult.bind(
        null, mode, n, parms.row, parms.col, 'camera-static'));
    sc.reset();
  }
  
  function dynamicCamera(n, aspect, callback) {
    sc.cameraMode = Sc.kPerspective;
    sc.worldMatrix = new Matrix.Translate(0, 0, -1000);
    var source = sc.openCamera(0, n);
    var t0;
    var done = false;
    sc.sleep(kFirstWait * 1000, function() {
      t0 = Date.now();
    });
    var parms = bestRowCol(n, aspect);
    var summaries = [];
    var details = [];
    for (var j = 0; j < 60 * kSamplingTimes; j++) {
      for (var r = 0, i = 0; r < parms.row; r++)
      for (var c = 0; c < parms.col && i < n; c++, i++) {
        var ratio = 1+Math.sin(j/90*Math.PI)/2;
        source[i].matrix = 
            new Matrix.Scale(parms.width/2, parms.height/2, 1)
           .scale(ratio, ratio, 1)
           .rotateX(-5 * i - 2 *j).rotateY(-15 * i - 2 * j).rotateZ(j)
           .translate((-kWidth + parms.width)/2, (kHeight - parms.height)/2, 0)
           .translate(c * parms.width, -r * parms.height, 0);
      }
      if (j % 60 == 59)
        sc.perfInfo(function(summary, detail) {
          if (!done) {
            summaries.push(summary);
            details.push(detail);
          }
        })
      sc.sleep(30, function() {
        if (Date.now() - t0 > 1000*60*2.5) {
          sc.immediateRequest('close ' + source.id);
          done = true;
        }
      });
    }
    sc.reset(function() {
      formatResult(mode, n, parms.row, parms.col, 'camera-dynamic', summaries, 
          details);
      callback();
    });
  }

  function staticFile(n, uri, aspect) {
    sc.cameraMode = Sc.kOrthographic;
    var parms = bestRowCol(n, aspect);
    for (var r = 0, i = 0; r < parms.row; r++)
    for (var c = 0; c < parms.col && i < n; c++, i++) {
      var source = sc.openURI(uri);
      source.matrix = 
          new Matrix.Scale(parms.width/2, parms.height/2, 1)
          .translate((-kWidth + parms.width)/2, (kHeight - parms.height)/2, 0)
          .translate(c * parms.width, -r * parms.height, 0);
    }
    sc.evaluate(kSamplingTimes, kFirstWait, kBetweenWait, formatResult.bind(
        null, mode, n, parms.row, parms.col, 'file-static: ' + uri));
    sc.reset();
  }

  [1, 2, 4, 6, 8, 16].forEach(function(n) {
    staticCamera(n, 640/480);
  });

  [1, 2, 3, 4, 6, 8].forEach(function(n) {
    staticFile(n, '854x480.flv', 854/480);
  });

  [1, 2, 3, 4, 6].forEach(function(n) {
    staticFile(n, '1280x720.mp4', 1280/720);
  });

  [1, 2, 3, 4].forEach(function(n) {
    staticFile(n, '1920x1080.mp4', 1920/1080);
  });

  var ns = [1, 2, 4, 6, 8, 16];
  function nextDynamic() {
    if (ns.length) 
      dynamicCamera(ns.shift(), 4/3, nextDynamic);
    else
      sc.quit(callback);
  }
  nextDynamic();

}

var modes = ['pbo', 'proxy', 'cpu'];
function nextMode() {
  if (modes.length > 0)
    process.nextTick(runTest.bind(null, modes.shift(), nextMode));
}
nextMode();


