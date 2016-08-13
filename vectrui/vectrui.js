var r;
var VectrUI = function() {
  'use strict';

  var SER_VERSION    = 121;
  var SER_WRITE      = 100;
  var SER_HANDSHAKE  = 200;
  var SER_DISCONNECT = 210;
  var SER_VIEW_MODE  = 220;
  var SER_VIEW_COLOR = 230;
  var SER_INIT       = 240;

  var MAX_MODES      = 16;

  var version = "0.3.1";
  var dir_root;
  var dir_firmwares;
  var dir_modes;
  var connection_id;
  var connected = false;
  var input_buffer = [];
  var readListeners = [];
  var updateListeners = [];
  var modelib = {};

  var main = document.getElementById("main");
  var editor = document.getElementById("editor");
  var bundles = document.getElementById("bundles");
  var bundle0 = document.getElementById("bundle0");
  var bundle1 = document.getElementById("bundle1");
  var modes = document.getElementById("mode-list");
  var dialog = document.querySelector("dialog");
  var close = document.getElementById("close-dialog");

  var mode_bundles = [
    [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []],
    [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
  ];
  var mode_bundle_ids = [
    [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []],
    [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
  ];

  var modetypes = ["Vectr", "Primer"];
  var triggers = ["Off", "Velocity", "Pitch", "Roll", "Flip"];

  jQuery.colorpicker.swatches.custom_array = [
    {name: 'red',         r: 208 / 255, g:   0 / 255, b:   0 / 255},
    {name: 'sunrise',     r: 182 / 255, g:  28 / 255, b:   0 / 255},
    {name: 'orange',      r: 156 / 255, g:  56 / 255, b:   0 / 255},
    {name: 'gold',        r: 130 / 255, g:  84 / 255, b:   0 / 255},
    {name: 'yellow',      r: 104 / 255, g: 112 / 255, b:   0 / 255},
    {name: 'lemon',       r:  78 / 255, g: 140 / 255, b:   0 / 255},
    {name: 'lime',        r:  52 / 255, g: 168 / 255, b:   0 / 255},
    {name: 'virus',       r:  26 / 255, g: 196 / 255, b:   0 / 255},

    {name: 'green',       r:   0 / 255, g: 224 / 255, b:   0 / 255},
    {name: 'sea',         r:   0 / 255, g: 196 / 255, b:  30 / 255},
    {name: 'aqua',        r:   0 / 255, g: 168 / 255, b:  60 / 255},
    {name: 'turqoise',    r:   0 / 255, g: 140 / 255, b:  90 / 255},
    {name: 'cyan',        r:   0 / 255, g: 112 / 255, b: 120 / 255},
    {name: 'baby blue',   r:   0 / 255, g:  84 / 255, b: 150 / 255},
    {name: 'sky',         r:   0 / 255, g:  56 / 255, b: 180 / 255},
    {name: 'royal blue',  r:   0 / 255, g:  28 / 255, b: 210 / 255},

    {name: 'blue',        r:   0 / 255, g:   0 / 255, b: 240 / 255},
    {name: 'indigo',      r:  26 / 255, g:   0 / 255, b: 210 / 255},
    {name: 'purple',      r:  52 / 255, g:   0 / 255, b: 180 / 255},
    {name: 'violet',      r:  78 / 255, g:   0 / 255, b: 150 / 255},
    {name: 'magenta',     r: 104 / 255, g:   0 / 255, b: 120 / 255},
    {name: 'blush',       r: 130 / 255, g:   0 / 255, b:  90 / 255},
    {name: 'pink',        r: 156 / 255, g:   0 / 255, b:  60 / 255},
    {name: 'sunset',      r: 182 / 255, g:   0 / 255, b:  30 / 255},

    {name: 'black',       r:   0 / 255, g:   0 / 255, b:   0 / 255},
    {name: 'white',       r:  65 / 255, g:  70 / 255, b:  75 / 255},
    {name: 'dim red',     r:   4 / 255, g:   0 / 255, b:   0 / 255},
    {name: 'dim yellow',  r:   2 / 255, g:   2 / 255, b:   0 / 255},
    {name: 'dim green',   r:   0 / 255, g:   4 / 255, b:   0 / 255},
    {name: 'dim cyan',    r:   0 / 255, g:   2 / 255, b:   2 / 255},
    {name: 'dim blue',    r:   0 / 255, g:   0 / 255, b:   4 / 255},
    {name: 'dim purple',  r:   2 / 255, g:   0 / 255, b:   2 / 255},

    {name: '0',           r: 195 / 255, g:   0 / 255, b:   0 / 255},
    {name: '20',          r: 162 / 255, g:  35 / 255, b:   0 / 255},
    {name: '40',          r: 130 / 255, g:  70 / 255, b:   0 / 255},
    {name: '60',          r:  97 / 255, g: 105 / 255, b:   0 / 255},
    {name: '80',          r:  65 / 255, g: 140 / 255, b:   0 / 255},
    {name: '100',         r:  32 / 255, g: 175 / 255, b:   0 / 255},
    {name: '120',         r:   0 / 255, g: 210 / 255, b:   0 / 255},
    {name: '140',         r:   0 / 255, g: 175 / 255, b:  37 / 255},
    {name: '160',         r:   0 / 255, g: 140 / 255, b:  75 / 255},
    {name: '180',         r:   0 / 255, g: 105 / 255, b: 112 / 255},
    {name: '200',         r:   0 / 255, g:  70 / 255, b: 150 / 255},
    {name: '220',         r:   0 / 255, g:  35 / 255, b: 188 / 255},
    {name: '240',         r:   0 / 255, g:   0 / 255, b: 225 / 255},
    {name: '260',         r:  32 / 255, g:   0 / 255, b: 188 / 255},
    {name: '280',         r:  65 / 255, g:   0 / 255, b: 150 / 255},
    {name: '300',         r:  97 / 255, g:   0 / 255, b: 112 / 255},
    {name: '320',         r: 130 / 255, g:   0 / 255, b:  75 / 255},
    {name: '340',         r: 162 / 255, g:   0 / 255, b:  37 / 255},
  ];

  dragula([modes, bundle0, bundle1], {
    copy: true,
    removeOnSpill: true,
    accepts: function(el, target, source, sibling) {
      if (target === modes) {
        return false;
      } else if (target.childElementCount > MAX_MODES) {
        return false;
      }
      return true;
    }
  }).on("drop", function(el, target, source, sibling) {
    el.data = {};
    el.data.id = el.id;
    el.setAttribute("id", "mode-item-" + Math.floor(Math.random() * Math.pow(2, 16)));
  }).on("cancel", function(el, container, source) {
    if (source === bundle0 || source === bundle1) {
      $(source).find("#" + el.getAttribute('id')).remove()
    }
  });

  var data = [
    0,                                // type
    1, 0,                             // pattern
    1, 1, 5, 0,                       // args1
    0, 0, 0, 0,                       // args2
    5, 5, 5, 5, 5, 0, 0, 0,           // timings1
    6, 6, 6, 6, 6, 0, 0, 0,           // timings2
    7, 7, 7, 7, 7, 0, 0, 0,           // timings3
    1, 3, 1,                          // numc
    255, 0, 0, 0, 0, 0, 0, 0, 0,      // colors1
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    255, 0, 0, 0, 255, 0, 0, 0, 255,  // colors2
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 255, 0, 0, 0, 0, 0, 0,      // colors3
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 14, 18, 28,                    // pattern thresh
    8, 8, 20, 20,                     // color thresh
    0
  ];

  for (var i = 0; i < 191; i++) {
    readListeners[i] = [];
    updateListeners[i] = [];
  }

  function sendCommand(cmd, force) {
    console.log("sent: " + cmd[0] + " " + cmd[1] + " " + cmd[2] + " " + cmd[3]);
    if (connected || force) {
      var buf = new ArrayBuffer(4);
      var view = new DataView(buf);
      view.setInt8(0, cmd[0]);
      view.setInt8(1, cmd[1]);
      view.setInt8(2, cmd[2]);
      view.setInt8(3, cmd[3]);
      chrome.serial.send(connection_id, buf, function() {});
    }
  };

  function handleCommand(cmd) {
    console.log("got: " + cmd[0] + " " + cmd[1] + " " + cmd[2] + " " + cmd[3]);
    var delay_send = false;
    if (cmd[0] == SER_HANDSHAKE && cmd[1] == SER_VERSION && cmd[2] == cmd[3] && !connected) {
      connected = true;
      sendCommand([SER_HANDSHAKE, SER_VERSION, 42, 42], true);
      var now = new Date().getTime();
      while (new Date().getTime() < now + 500) {}
      delay_send = true;
    }
    // If on windows, or rebooting, you'll get the handshake first
    if (cmd[0] == SER_HANDSHAKE && cmd[1] == SER_VERSION && cmd[2] == cmd[3] && delay_send) {
      var now = new Date().getTime();
      while (new Date().getTime() < now + 500) {}
      for (var i = 0; i < 191; i++) {
        sendData(i, data[i]);
      }
      sendCommand([SER_INIT, 0, 0, 0]);
      delay_send = false;
    }
  };

  function sendData(addr, val) {
    // Updates in-memory array and sends value to light
    data[addr] = val;
    sendCommand([SER_WRITE, addr, val, 0]);
  };

  function updateData(addr, val) {
    // UX changed value
    sendData(addr, val);
    for (var i = 0; i < updateListeners[addr].length; i++) {
      updateListeners[addr][i](val);
    }
  };

  function readData(addr, val) {
    // Read in new value
    sendData(addr, val);
    for (var i = 0; i < readListeners[addr].length; i++) {
      readListeners[addr][i](val);
    }
  };

  function arrayToMode(arr) {
    // array to json mode
    return {
      type: arr[0],
      pattern: [arr[1], arr[2]],
      args: [
        [arr[3], arr[4], arr[5], arr[6]],
        [arr[7], arr[8], arr[9], arr[10]]
      ],
      timings: [
        [arr[11], arr[12], arr[13], arr[14], arr[15], arr[16], arr[17], arr[18]],
        [arr[19], arr[20], arr[21], arr[22], arr[23], arr[24], arr[25], arr[26]],
        [arr[27], arr[28], arr[29], arr[30], arr[31], arr[32], arr[33], arr[34]]
      ],
      numc: [arr[35], arr[36], arr[37]],
      thresh0: [arr[38], arr[39], arr[40], arr[41]],
      thresh1: [arr[42], arr[43], arr[44], arr[45]],
      trigger: arr[46],
      colors: [
        [
          [arr[47], arr[48], arr[49]],
          [arr[50], arr[51], arr[52]],
          [arr[53], arr[54], arr[55]],
          [arr[56], arr[57], arr[58]],
          [arr[59], arr[60], arr[61]],
          [arr[62], arr[63], arr[64]],
          [arr[65], arr[66], arr[67]],
          [arr[68], arr[69], arr[70]],
          [arr[71], arr[72], arr[73]],
          [arr[74], arr[75], arr[76]],
          [arr[77], arr[78], arr[79]],
          [arr[80], arr[81], arr[82]],
          [arr[83], arr[84], arr[85]],
          [arr[86], arr[87], arr[88]],
          [arr[89], arr[90], arr[91]],
          [arr[92], arr[93], arr[94]]
        ],
        [
          [arr[95], arr[96], arr[97]],
          [arr[98], arr[99], arr[100]],
          [arr[101], arr[102], arr[103]],
          [arr[104], arr[105], arr[106]],
          [arr[107], arr[108], arr[109]],
          [arr[110], arr[111], arr[112]],
          [arr[113], arr[114], arr[115]],
          [arr[116], arr[117], arr[118]],
          [arr[119], arr[120], arr[121]],
          [arr[122], arr[123], arr[124]],
          [arr[125], arr[126], arr[127]],
          [arr[128], arr[129], arr[130]],
          [arr[131], arr[132], arr[133]],
          [arr[134], arr[135], arr[136]],
          [arr[137], arr[138], arr[139]],
          [arr[140], arr[141], arr[142]]
        ],
        [
          [arr[143], arr[144], arr[145]],
          [arr[146], arr[147], arr[148]],
          [arr[149], arr[150], arr[151]],
          [arr[152], arr[153], arr[154]],
          [arr[155], arr[156], arr[157]],
          [arr[158], arr[159], arr[160]],
          [arr[161], arr[162], arr[163]],
          [arr[164], arr[165], arr[166]],
          [arr[167], arr[168], arr[169]],
          [arr[170], arr[171], arr[172]],
          [arr[173], arr[174], arr[175]],
          [arr[176], arr[177], arr[178]],
          [arr[179], arr[180], arr[181]],
          [arr[182], arr[183], arr[184]],
          [arr[185], arr[186], arr[187]],
          [arr[188], arr[189], arr[190]]
        ]
      ]
    };
  };

  function modeToArray(m) {
    // json mode to array
    return [
      m.type,
      m.pattern[0], m.pattern[1],

      m.args[0][0], m.args[0][1], m.args[0][2], m.args[0][3],
      m.args[1][0], m.args[1][1], m.args[1][2], m.args[1][3],

      m.timings[0][0], m.timings[0][1], m.timings[0][2], m.timings[0][3], m.timings[0][4], m.timings[0][5], m.timings[0][6], m.timings[0][7],
      m.timings[1][0], m.timings[1][1], m.timings[1][2], m.timings[1][3], m.timings[1][4], m.timings[1][5], m.timings[1][6], m.timings[1][7],
      m.timings[2][0], m.timings[2][1], m.timings[2][2], m.timings[2][3], m.timings[2][4], m.timings[2][5], m.timings[2][6], m.timings[2][7],

      m.numc[0], m.numc[1], m.numc[2],

      m.thresh0[0], m.thresh0[1], m.thresh0[2], m.thresh0[3],
      m.thresh1[0], m.thresh1[1], m.thresh1[2], m.thresh1[3],
      m.trigger,

      m.colors[0][0][0],  m.colors[0][0][1],  m.colors[0][0][2],
      m.colors[0][1][0],  m.colors[0][1][1],  m.colors[0][1][2],
      m.colors[0][2][0],  m.colors[0][2][1],  m.colors[0][2][2],
      m.colors[0][3][0],  m.colors[0][3][1],  m.colors[0][3][2],
      m.colors[0][4][0],  m.colors[0][4][1],  m.colors[0][4][2],
      m.colors[0][5][0],  m.colors[0][5][1],  m.colors[0][5][2],
      m.colors[0][6][0],  m.colors[0][6][1],  m.colors[0][6][2],
      m.colors[0][7][0],  m.colors[0][7][1],  m.colors[0][7][2],
      m.colors[0][8][0],  m.colors[0][8][1],  m.colors[0][8][2],
      m.colors[0][9][0],  m.colors[0][9][1],  m.colors[0][8][2],
      m.colors[0][10][0], m.colors[0][10][1], m.colors[0][8][2],
      m.colors[0][11][0], m.colors[0][11][1], m.colors[0][8][2],
      m.colors[0][12][0], m.colors[0][12][1], m.colors[0][8][2],
      m.colors[0][13][0], m.colors[0][13][1], m.colors[0][8][2],
      m.colors[0][14][0], m.colors[0][14][1], m.colors[0][8][2],
      m.colors[0][15][0], m.colors[0][15][1], m.colors[0][8][2],

      m.colors[1][0][0],  m.colors[1][0][1],  m.colors[1][0][2],
      m.colors[1][1][0],  m.colors[1][1][1],  m.colors[1][1][2],
      m.colors[1][2][0],  m.colors[1][2][1],  m.colors[1][2][2],
      m.colors[1][3][0],  m.colors[1][3][1],  m.colors[1][3][2],
      m.colors[1][4][0],  m.colors[1][4][1],  m.colors[1][4][2],
      m.colors[1][5][0],  m.colors[1][5][1],  m.colors[1][5][2],
      m.colors[1][6][0],  m.colors[1][6][1],  m.colors[1][6][2],
      m.colors[1][7][0],  m.colors[1][7][1],  m.colors[1][7][2],
      m.colors[1][8][0],  m.colors[1][8][1],  m.colors[1][8][2],
      m.colors[1][8][0],  m.colors[1][9][1],  m.colors[1][9][2],
      m.colors[1][8][0], m.colors[1][10][1], m.colors[1][10][2],
      m.colors[1][9][0], m.colors[1][11][1], m.colors[1][11][2],
      m.colors[1][10][0], m.colors[1][12][1], m.colors[1][12][2],
      m.colors[1][11][0], m.colors[1][13][1], m.colors[1][13][2],
      m.colors[1][12][0], m.colors[1][14][1], m.colors[1][14][2],
      m.colors[1][13][0], m.colors[1][15][1], m.colors[1][15][2],

      m.colors[2][0][0],  m.colors[2][0][1],  m.colors[2][0][2],
      m.colors[2][1][0],  m.colors[2][1][1],  m.colors[2][1][2],
      m.colors[2][2][0],  m.colors[2][2][1],  m.colors[2][2][2],
      m.colors[2][3][0],  m.colors[2][3][1],  m.colors[2][3][2],
      m.colors[2][4][0],  m.colors[2][4][1],  m.colors[2][4][2],
      m.colors[2][5][0],  m.colors[2][5][1],  m.colors[2][5][2],
      m.colors[2][6][0],  m.colors[2][6][1],  m.colors[2][6][2],
      m.colors[2][7][0],  m.colors[2][7][1],  m.colors[2][7][2],
      m.colors[2][8][0],  m.colors[2][8][1],  m.colors[2][8][2],
      m.colors[2][9][0],  m.colors[2][9][1],  m.colors[2][9][2],
      m.colors[2][10][0], m.colors[2][10][1], m.colors[2][10][2],
      m.colors[2][11][0], m.colors[2][11][1], m.colors[2][11][2],
      m.colors[2][12][0], m.colors[2][12][1], m.colors[2][12][2],
      m.colors[2][13][0], m.colors[2][13][1], m.colors[2][13][2],
      m.colors[2][14][0], m.colors[2][14][1], m.colors[2][14][2],
      m.colors[2][15][0], m.colors[2][15][1], m.colors[2][15][2]
    ];
  };

  function rgb2hex(rgb) {
    var r = ("0" + rgb.r.toString(16)).slice(-2);
    var g = ("0" + rgb.g.toString(16)).slice(-2);
    var b = ("0" + rgb.b.toString(16)).slice(-2);
    return "#" + r + g + b;
  };

  function hex2rgb(hex) {
    return {
      r: parseInt(hex.substr(0, 2), 16),
      g: parseInt(hex.substr(2, 2), 16),
      b: parseInt(hex.substr(4, 2), 16)
    };
  };

  function getColorHex(set, slot) {
    var red = data[(48 * set) + (3 * slot) + 47];
    if (red === null || red === undefined) {
      red = 0;
    }

    var green = data[(48 * set) + (3 * slot) + 48];
    if (green === null || green === undefined) {
      green = 0;
    }

    var blue = data[(48 * set) + (3 * slot) + 49];
    if (blue === null || blue === undefined) {
      blue = 0;
    }

    var rgb = {
      r: red,
      g: green,
      b: blue
    };

    return rgb2hex(rgb);
  };

  function normColor(hex) {
    var rgb = hex2rgb(hex);
    if (rgb.r > 0) {
      rgb.r = Math.round(64 + ((rgb.r * 3) / 4));
    }
    if (rgb.g > 0) {
      rgb.g = Math.round(64 + ((rgb.g * 3) / 4));
    }
    if (rgb.b > 0) {
      rgb.b = Math.round(64 + ((rgb.b * 3) / 4));
    }
    return rgb2hex(rgb);
  };

  function SliderField(parent, opts) {
    var opts = opts || {};
    if (opts.min === void 0) { opts.min = 0; }
    if (opts.max === void 0) { opts.max = 100; }
    if (opts.value === void 0) { opts.value = opts.min; }
    if (opts.step === void 0) { opts.step = 1; }
    if (opts.addr === void 0) { opts.addr = 0; }
    if (opts.multiplier === void 0) { opts.multiplier = 1; }
    if (opts.width === void 0) { opts.size = 200; }

    var sliderChange = function () {
      return function(event, ui) {
        field.value = ui.value;
        if (event.originalEvent) {
          updateData(opts.addr, ui.value * opts.multiplier);
          sendCommand([SER_INIT, 0, 0, 0]);
        }
      }
    }();

    var fieldChange = function() {
      return function(event) {
        if (event.target.value < $(slider).slider("option", "min")) {
          event.target.value = slider.slider("option", "min");
        } else if (event.target.value > $(slider).slider("option", "max")) {
          event.target.value = slider.slider("option", "max");
        }
        $(slider).slider("value", event.target.value);
        updateData(opts.addr, event.target.value * opts.multiplier);
        sendCommand([SER_INIT, 0, 0, 0]);
      };
    }();

    var listener = function() {
      return function(val) {
        val = Number(val);
        field.value = val / opts.multiplier;
        $(slider).slider("value", field.value);
      }
    }();

    var elem = document.createElement("div");
    elem.style.display = "inline-block";
    elem.style.width = opts.width + "px";
    parent.appendChild(elem);

    var field = document.createElement("input");
    field.type = "text";
    field.value = opts.value;
    field.onchange = fieldChange;
    field.style.width = "30px";
    field.style.float = "left";
    elem.appendChild(field);

    var slider = document.createElement("div");
    slider.style.width = (opts.width - 60) + "px";
    slider.style.float = "right";
    $(slider).slider({
      min: opts.min,
      max: opts.max,
      step: opts.step,
      value: opts.value,
      slide: sliderChange,
      change: sliderChange
    });
    elem.appendChild(slider);

    readListeners[opts.addr].push(listener);

    this.setMin = function(val) {
      $(slider).slider("option", "min", val);
      if (field.value < val) {
        field.value = val;
      }
    };

    this.setMax = function(val) {
      $(slider).slider("option", "max", val);
      if (field.value > val) {
        field.value = val;
      }
    };

    this.setValue = function(val) {
      $(slider).slider("value", val);
    };
  };


  function PatternRow(parent, pattern_idx) {
    // Pattern dropdown + arg sliderfields

    function ArgElement(parent, arg_idx) {
      // Arg label + sliderfield
      var addr = 3 + (4 * pattern_idx) + arg_idx;
      var elem = document.createElement("div");
      elem.title = "Defines the pattern.";
      elem.style.width = "175px";
      elem.style.display = "inline-block";
      elem.style.marginLeft = "20px";
      parent.appendChild(elem);

      var label = document.createElement("span");
      label.textContent = "Arg " + pattern_idx + " " +  arg_idx;
      label.style.width = "100%";
      elem.appendChild(label);

      var slider = new SliderField(elem, {
        addr: addr,
        width: 175
      });

      var listener = function(send_data) {
        return function(val) {
          var pattern = Patterns.getPattern(val);
          if (arg_idx >= pattern.args.length) {
            elem.style.display = 'none';
          } else {
            elem.style.display = 'inline-block';
            elem.title = pattern.args[arg_idx].tooltip;
            label.textContent = pattern.args[arg_idx].name;
            slider.setMin(pattern.args[arg_idx].min);
            slider.setMax(pattern.args[arg_idx].max);
            if (send_data) {
              slider.setValue(pattern.args[arg_idx].default);
              sendData(addr, pattern.args[arg_idx].default);
            }
          }
        };
      };

      readListeners[1 + pattern_idx].push(listener(false));
      updateListeners[1 + pattern_idx].push(listener(true));
    };

    function PatternElement(parent) {
      // Pattern label + dropdown
      var elem = document.createElement("div");
      elem.title = "Base pattern.";
      elem.style.width = "115px";
      elem.style.display = "inline-block";
      elem.style.verticalAlign = "top";
      parent.appendChild(elem);

      var label = document.createElement("span");
      label.textContent = "Base Pattern";
      label.style.width = "100%";
      elem.appendChild(label);

      var dropdown = document.createElement("select");
      dropdown.style.width = "100%";
      dropdown.style.display = "inline-block";
      dropdown.onchange = function() {
        return function(event) {
          updateData(1 + pattern_idx, Number(this.value));
          sendCommand([SER_INIT, 0, 0, 0]);
        };
      }();
      elem.appendChild(dropdown);

      var patterns = Patterns.getPatterns();
      for (var i = 0; i < patterns.length; i++) {
        var pattern = document.createElement("option");
        pattern.value = i;
        pattern.textContent = patterns[i].name;
        dropdown.appendChild(pattern);
      }

      var listener = function() {
        return function(val) {
          dropdown.value = val;
        };
      }();

      readListeners[1 + pattern_idx].push(listener);
    };

    var elem = document.createElement("div");
    elem.style.margin = "10px";
    parent.appendChild(elem);

    var pattern = new PatternElement(elem);
    for (var i = 0; i < 4; i++) {
      var arg = new ArgElement(elem, i);
    }
  };

  function TimingColumn(parent, pattern_idx, timing_group, show_label) {
    // Column of 8 timing sliderfields with optional label
    function TimingElement(parent, timing_idx) {
      // Timing sliderfield with optional label
      var addr = 11 + (8 * pattern_idx) + (8 * timing_group) + timing_idx;
      var width = (show_label) ? 460 : 230;
      var elem = document.createElement("div");
      elem.title = "Timing for the pattern.";
      elem.style.width = width + "px";
      elem.setAttribute("timing_group", timing_group);
      elem.setAttribute("timing_idx", timing_idx);
      parent.appendChild(elem);

      if (show_label) {
        var label = document.createElement("span");
        label.textContent = "Timing " + pattern_idx + " " + timing_idx;
        label.style.width = "210px";
        label.style.margin = "0px 10px 0px 10px";
        label.style.float = "left";
        label.style.verticalAlign = "top";
        elem.appendChild(label);
      }

      var slider = new SliderField(elem, {
        min: 0,
        max: 125,
        step: 0.5,
        multiplier: 2,
        width: 210,
        addr: addr
      });

      var listener = function(send_data) {
        return function(val) {
          var pattern = Patterns.getPattern(val);
          if (timing_idx >= pattern.timings.length) {
            // elem.style.display = 'none';
            elem.style.visibility = 'hidden';
          } else {
            // elem.style.display = null;
            elem.style.visibility = 'visible';
            elem.title = pattern.timings[timing_idx].tooltip;
            if (show_label) {
              label.textContent = pattern.timings[timing_idx].name;
            }
            if (send_data) {
              slider.setValue(pattern.timings[timing_idx].default);
              sendData(addr, pattern.timings[timing_idx].default);
            }
          }
        };
      };

      readListeners[1 + pattern_idx].push(listener(false));
      updateListeners[1 + pattern_idx].push(listener(true));
    };

    var elem = document.createElement("div");
    elem.style.verticalAlign = "text-top";
    parent.appendChild(elem);

    for (var i = 0; i < 8; i++) {
      var timing = new TimingElement(elem, i);
    };
  };

  function VectrEditor(parent) {
    var elem = document.createElement("div");
    parent.appendChild(elem);

    new PatternRow(elem, 0);
    new ThreshRow(elem, 1, 4);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "15px";
    elem.appendChild(spacer);

    var colors = document.createElement("div");
    elem.appendChild(colors);

    new ColorSetRow(colors, 0, "vectr");
    new ColorSetRow(colors, 1, "vectr");
    new ColorSetRow(colors, 2, "vectr");

    var spacer = document.createElement("div");
    spacer.style.minHeight = "15px";
    elem.appendChild(spacer);

    var timingThresh = ThreshRow(elem, 0, 4);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "15px";
    elem.appendChild(spacer);

    var timings = document.createElement("div");
    timings.style.display = "inline-block";
    elem.appendChild(timings);

    var timing0 = document.createElement("div");
    timing0.style.display = "inline-block";
    timings.appendChild(timing0);

    var timing1 = document.createElement("div");
    timing1.style.display = "inline-block";
    timings.appendChild(timing1);

    var timing2 = document.createElement("div");
    timing2.style.display = "inline-block";
    timings.appendChild(timing2);

    new TimingColumn(timing0, 0, 0, true);
    new TimingColumn(timing1, 0, 1, false);
    new TimingColumn(timing2, 0, 2, false);

    var typeListener = function(send_data) {
      return function(val) {
        if (val == 0) {
          elem.style.display = null;
          if (send_data) {
            readData(1, 0);
          }
        } else {
          elem.style.display = 'none';
        }
      };
    };

    readListeners[0].push(typeListener(false));
    updateListeners[0].push(typeListener(true));
  };

  function PrimerEditor(parent) {
    var elem = document.createElement("div");
    parent.appendChild(elem);

    new ThreshRow(elem, 0, 2);

    var container0 = document.createElement("div");
    elem.appendChild(container0);
    var pattern0 = new PatternRow(container0, 0);
    var colors0 = new ColorSetRow(container0, 0, "primer");

    var spacer = document.createElement("div");
    spacer.style.minHeight = "10px";
    elem.appendChild(spacer);

    var container1 = document.createElement("div");
    elem.appendChild(container1);
    var pattern1 = new PatternRow(container1, 1);
    var colors1 = new ColorSetRow(container1, 1, "primer");

    var spacer = document.createElement("div");
    spacer.style.minHeight = "20px";
    elem.appendChild(spacer);

    var timings = document.createElement("div");
    timings.style.display = "inline-block";
    elem.appendChild(timings);

    var timing0 = document.createElement("div");
    timing0.style.display = "inline-block";
    timings.appendChild(timing0);

    var timing1 = document.createElement("div");
    timing1.style.display = "inline-block";
    timings.appendChild(timing1);

    new TimingColumn(timing0, 0, 0, true);
    new TimingColumn(timing1, 1, 0, true);

    var typeListener = function(send_data) {
      return function(val) {
        if (val == 1) {
          elem.style.display = null;
          if (send_data) {
            updateData(1, 0);
            updateData(2, 0);
            updateData(46, 0);
            sendCommand([SER_INIT, 0, 0, 0]);
          }
        } else {
          elem.style.display = 'none';
        }
      };
    };

    var triggerListener = function(send_data) {
      return function(val) {
        if (val == 0) {
          container1.style.visibility = 'hidden';
          timing1.style.display = 'none';
        } else {
          container1.style.visibility = 'visible';
          timing1.style.display = 'inline-block';
        }
      };
    };

    readListeners[46].push(triggerListener(false));
    updateListeners[46].push(triggerListener(false));

    readListeners[0].push(typeListener(false));
    updateListeners[0].push(typeListener(true));
  };


  function ColorSetRow(parent, set_idx, prefix) {
    // Numc sliderfield + colorpickers
    function ColorSlot(parent, slot_idx) {
      // Color picker
      var addr = 47 + (48 * set_idx) + (3 * slot_idx);
      var id = prefix + "-color-" + set_idx + "-" + slot_idx;

      var color_elem = document.createElement("div");
      color_elem.style.display = "inline-block";
      parent.appendChild(color_elem);

      var color_input = document.createElement("input");
      color_input.id = id;
      color_input.type = "text";
      color_input.setAttribute("set", set_idx);
      color_input.setAttribute("slot", slot_idx);
      color_input.style.display = 'none';
      color_elem.appendChild(color_input);

      var color_target = document.createElement("div");
      color_target.id = id + "-target";
      color_target.className = "color";
      color_elem.appendChild(color_target);

      var sendColor = function() {
        return function(event, color) {
          if (color.formatted && color.colorPicker.generated) {
            var rgb = hex2rgb(color.formatted);
            sendData(addr + 0, rgb.r);
            sendData(addr + 1, rgb.g);
            sendData(addr + 2, rgb.b);
            color_target.style.background = normColor(color.formatted);
          }
        };
      }();

      var viewColor = function() {
        return function(event, ui) {
          sendCommand([SER_VIEW_COLOR, set_idx, slot_idx, 0]);
        };
      }();

      var viewMode = function() {
        return function(event, ui) {
          sendCommand([SER_VIEW_MODE, 0, 0, 0]);
        };
      }();

      var picker = $(color_input).colorpicker({
        alpha: false,
        closeOnOutside: false,
        swatches: 'custom_array',
        swatchesWidth: 96,
        colorFormat: 'HEX',
        altField: color_target.id,
        altProperties: "background-color",
        parts: ['header', 'preview', 'map', 'bar', 'rgb', 'hsv', 'swatches', 'footer'],
        layout: {
          map: [0, 0, 1, 3],
          bar: [1, 0, 1, 3],
          preview: [2, 0, 1, 1],
          rgb: [2, 1, 1, 1],
          hsv: [2, 2, 1, 1],
          swatches: [3, 0, 1, 3]
        },
        select: sendColor,
        open: viewColor,
        close: viewMode
      });

      var onclick = function() {
        return function() {
          $(color_input).colorpicker("open");
        };
      }();
      color_target.onclick = onclick;

      var updateColor = function(channel) {
        return function(val) {
          var hex = getColorHex(set_idx, slot_idx);
          color_target.style.background = hex;
          $(color_input).val(hex);
        };
      };

      var showOrHide = function() {
        return function(val) {
          if (slot_idx >= val) {
            color_target.style.display = 'none';
          } else {
            color_target.style.display = null;
          }
        }
      }();

      // listeners for RGB changes
      readListeners[addr + 0].push(updateColor(0));
      readListeners[addr + 1].push(updateColor(1));
      readListeners[addr + 2].push(updateColor(2));

      // listener on numc change
      readListeners[35 + set_idx].push(showOrHide);
      updateListeners[35 + set_idx].push(showOrHide);
    };

    var elem = document.createElement("div");
    elem.style.margin = "0 auto";
    elem.style.width = "785px";
    parent.appendChild(elem);

    var slider = new SliderField(elem, {
      min: 1,
      max: 16,
      width: 200,
      addr: 35 + set_idx
    });

    var slots = [];
    var color_container = document.createElement("div");
    color_container.style.marginLeft = "20px";
    color_container.style.display = "inline-block";
    elem.appendChild(color_container);

    for (var i = 0; i < 16; i++) {
      var color_slot = new ColorSlot(color_container, i);
      slots.push(color_slot);
    }

    var listener = function() {
      return function(val) {
        slider.setValue(val);
      }
    }();

    readListeners[35 + set_idx].push(listener);
  };

  function ThreshRow(parent, thresh_idx, thresh_vals) {
    var elem = document.createElement("div");
    elem.style.margin = "10px";
    parent.appendChild(elem);

    var value_elems = [];

    var slider = document.createElement("div");
    var ranges;
    var def_values;
    if (thresh_vals == 2) {
      ranges = [
        {styleClass: 'trigger-0'},
        {styleClass: 'trigger-01'},
        {styleClass: 'trigger-1'}
      ];
      def_values = [4, 28];

      var dropdown_container = document.createElement("div");
      elem.appendChild(dropdown_container);

      var label = document.createElement("span");
      label.textContent = "Trigger Type";
      label.style.display = "inline-block";
      label.style.margin = "5px";
      dropdown_container.appendChild(label);

      var dropdown = document.createElement("select");
      dropdown.style.display = "inline-block";
      dropdown.onchange = function() {
        return function(event) {
          if (this.value == 0) {
            $(slider).limitslider("disable");
          } else {
            $(slider).limitslider("enable");
          }
          updateData(46, this.value);
          sendCommand([SER_INIT, 0, 0, 0]);
        };
      }();
      dropdown_container.appendChild(dropdown);

      for (var i = 0; i < triggers.length; i++) {
        var trigger = document.createElement("option");
        trigger.value = i;
        trigger.textContent = triggers[i];
        dropdown.appendChild(trigger);
      }

      var listener = function() {
        return function(val) {
          dropdown.value = val;
          if (val == 0) {
            $(slider).limitslider("disable");
          } else {
            $(slider).limitslider("enable");
          }
        };
      }();

      readListeners[46].push(listener);
    } else {
      ranges = [
        {styleClass: 'range-0'},
        {styleClass: 'range-01'},
        {styleClass: 'range-1'},
        {styleClass: 'range-12'},
        {styleClass: 'range-2'}
      ];
      def_values = [4, 12, 20, 28];
    }

    elem.appendChild(slider);

    var addr = 38 + (4 * thresh_idx);
    var values_container = document.createElement("div");
    values_container.style.display = "flex";
    values_container.style.justifyContent = "space-between";
    values_container.style.listStyleType = "none";
    elem.appendChild(values_container);

    for (var i = 0; i < thresh_vals; i++) {
      var value_addr = 38;
      if (thresh_vals == 2) {
        value_addr += 1 - i;
      } else {
        value_addr += (4 * thresh_idx) + i;
      }
      var valueChange = function(idx) {
        return function(event) {
          var val = Number(event.target.value);
          var values = $(slider).limitslider("values");
          var checks = [$(slider).limitslider("option", "min")]
                        .concat(values)
                        .concat([$(slider).limitslider("option", "max")]);

          if (val < checks[idx]) {
            val = checks[idx];
          } else if (val > checks[idx + 2]) {
            val = checks[idx + 2];
          }

          event.target.value = val;
          values[idx] = val;
          $(slider).limitslider("values", values);
          if (thresh_vals == 2) {
            sendData(39 - idx, val);
          } else {
            sendData(38 + (4 * thresh_idx) + idx, val);
          }
        }
      }(i);

      var value = document.createElement("input");
      value.type = "text";
      value.value = def_values[i];
      value.style.width = "30px";
      value.style.margin = "5px 50px";
      value.onchange = valueChange;
      value_elems.push(value);
      values_container.appendChild(value);

      var valueListener = function(idx) {
        return function(val) {
          var values = $(slider).limitslider("values");
          values[idx] = val;
          value_elems[idx].value = val;
          $(slider).limitslider("values", values);
        }
      }(i);

      readListeners[value_addr].push(valueListener);
    }

    var threshChange = function() {
      return function(event, ui) {
        if (event.originalEvent) {
          var idx = $(ui.handle).data("ui-slider-handle-index");
          value_elems[idx].value = ui.values[idx];
          sendData(addr + idx, ui.values[idx]);
        }
      };
    }();

    $(slider).limitslider({
      min: 0, max: 64, gap: 0,
      values: def_values,
      ranges: ranges,
      slide: threshChange,
      change: threshChange
    });
  };

  function writeFile(dir, filename, content) {
    dir.getFile(filename, {create: true}, function(entry) {
      entry.createWriter(function(writer) {
        writer.onwriteend = function(e) {
          if (writer.length === 0) {
            writer.write(blob);
            // console.log("Write success!");
          }
        };

        writer.onerror = function(e) {
          console.log("Write failed: " + e.toString());
        };

        writer.truncate(0);
        var blob = new Blob([content], {type: 'text/plain'});
        writer.write(blob);
      });
    });
  };

  function writeMode(mode) {
    var modecopy = JSON.parse(JSON.stringify(mode));
    delete modecopy.id;
    modecopy.version = version;
    var content = JSON.stringify(modecopy, null, 2);
    writeFile(dir_modes, modecopy.name + ".mode", content);
  };

  function writeSource(name, num_modes, bundle_a, bundle_b) {
    var content = getSource(num_modes, bundle_a, bundle_b);
    dir_firmwares.getDirectory(name, {create: true}, function(entry) {
      writeFile(entry, name + ".ino", content);
    });
  };

  function makeModeItem(modeobj) {
    var modeitem = document.createElement("div");
    modeitem.className = "mode";
    modeitem.id = modeobj.id;
    modeitem.textContent = modeobj.name;
    modeitem.data = modeobj;
    modeitem.data.filename = modeobj.name;
    modeitem.onclick = function(e) {
      var field = document.querySelector("#mode-save");
      field.value = this.data.filename;
      var arr = modeToArray(this.data);
      for (var i = 0; i < 191; i++) {
        readData(i, arr[i]);
      }
      sendCommand([SER_INIT, 0, 0, 0]);
    };

    modes.appendChild(modeitem);
  };

  function translateMode(modeobj) {
    console.log("version mismatch: " + modeobj.version);

    // < 0.2.5, tracer only had one gap
    if (modeobj.version === null || modeobj.version === undefined) {
      if (modeobj.type == 0) {
        if (modeobj.pattern[0] == 1) {
          modeobj.timings[0][5] = modeobj.timings[0][4];
        }
        if (modeobj.pattern[1] == 1) {
          modeobj.timings[1][5] = modeobj.timings[1][4];
        }
      } else {
        if (modeobj.pattern[0] == 1) {
          modeobj.timings[0][5] = modeobj.timings[0][4];
          modeobj.timings[1][5] = modeobj.timings[1][4];
          modeobj.timings[2][5] = modeobj.timings[2][4];
        }
      }
    }

    return modeobj;
  };

  function readModeFile(i, file) {
    file.file(function(file) {
      var reader = new FileReader();
      reader.onload = function(e) {
        var contents = e.target.result;
        var modeobj = JSON.parse(contents);
        if (modeobj.version != version) {
          modeobj = translateMode(modeobj);
          writeMode(modeobj);
        }
        modeobj.name = file.name.replace(".mode", "");
        modeobj.id = modeobj.name.replace(/\s/g, "-").toLowerCase();
        modelib[modeobj.id] = modeobj;
        makeModeItem(modeobj);
        if (i == 0) document.getElementById(modeobj.id).click();
      };
      reader.readAsText(file);
    });
  };

  function initUI() {
    var serialElement = new SerialElement(editor);
    var typeDropdown = new TypeDropdown(editor);
    var vectrUi = new VectrEditor(editor);
    var primerUi = new PrimerEditor(editor);

    var modeControls = new ModeControls(document.querySelector("#mode-controls"));
    var bundleControls = new BundleControls(document.querySelector("#bundle-controls"));
    serialElement.updatePorts();
  };

  function initSettings() {
    chrome.storage.local.get("vectr", function(data) {
      if (!data.vectr) {
        data.vectr = {};
      }
      if (data.vectr.version != version) {
        data.vectr = {};
        data.vectr.version = version;
        chrome.storage.local.set(data);
      }
      if (!data.vectr.dir_id) {
        chrome.fileSystem.chooseEntry({type: "openDirectory"}, function(entry) {
          data.vectr.dir_id = chrome.fileSystem.retainEntry(entry);
          dir_root = entry;
          chrome.storage.local.set(data);

          var default_modes = DefaultModes.getModes();
          dir_root.getDirectory("modes",     {create: true}, function(entry) {
            dir_modes = entry;
            for (var i = 0; i < default_modes.length; i++) {
              var default_mode = default_modes[i];

              for (var b = 0; b < default_mode.bundles.length; b++) {
                mode_bundles[default_mode.bundles[b]][default_mode.slot] = modeToArray(default_mode);
                mode_bundle_ids[default_mode.bundles[b]][default_mode.slot] = default_mode.id;
              }

              modelib[default_mode.id] = default_mode;
              makeModeItem(default_mode);
              writeMode(default_mode);
              if (i == 0) {
                document.getElementById(default_mode.id).click();
              }
            }
          });
          dir_root.getDirectory("firmwares", {create: true}, function(entry) {
            dir_firmwares = entry;
            var num_modes = [8, 8];
            for (var i = 0; i < 8; i++) {
              var child0 = document.getElementById(mode_bundle_ids[0][i]);
              var el0 = child0.cloneNode();
              el0.data = {id: child0.id};
              el0.textContent = child0.textContent;
              el0.setAttribute("id", "mode-item-" + Math.floor(Math.random() * Math.pow(2, 16)));
              bundle0.appendChild(el0);

              var child1 = document.getElementById(mode_bundle_ids[1][i]);
              var el1 = child1.cloneNode();
              el1.data = {id: child1.id};
              el1.textContent = child1.textContent;
              el1.setAttribute("id", "mode-item-" + Math.floor(Math.random() * Math.pow(2, 16)));
              bundle1.appendChild(el1);
            }
            document.getElementById("firmware-save").value = "default";
            writeSource("default", num_modes, mode_bundles[0], mode_bundles[1]);
          });
        });
      } else {
        chrome.fileSystem.restoreEntry(data.vectr.dir_id, function(entry) {
          dir_root = entry;
          dir_root.getDirectory("firmwares", {create: false}, function(entry) { dir_firmwares = entry; });
          dir_root.getDirectory("modes",     {create: false}, function(entry) {
            dir_modes = entry;
            var reader = entry.createReader();
            reader.readEntries(function(entries) {
              var c = 0;
              for (var i = 0; i < entries.length; i++) {
                if (entries[i].name.endsWith(".mode")) {
                  readModeFile(c, entries[i]);
                  c++;
                }
              }
            },
            function(e) {
              console.log(e);
            });
          });
        });
      }
    });
  };

  function TypeDropdown(parent) {
    var elem = document.createElement("div");
    elem.style.margin = "5px";
    parent.appendChild(elem);

    var dropdown = document.createElement("select");
    dropdown.onchange = function() {
      return function(event) {
        updateData(0, Number(this.value));
        sendCommand([SER_INIT, 0, 0, 0]);
      };
    }();
    elem.appendChild(dropdown);

    for (var i = 0; i < modetypes.length; i++) {
      var modetype = document.createElement("option");
      modetype.value = i;
      modetype.textContent = modetypes[i];
      dropdown.appendChild(modetype);
    }

    var listener = function() {
      return function(val) {
        dropdown.value = val;
      };
    }();

    readListeners[0].push(listener);
  };

  function SerialElement(parent) {
    var elem = document.createElement("div");
    elem.style.margin = "5px";
    parent.appendChild(elem);

    var dropdown = document.createElement("select");
    elem.appendChild(dropdown);

    function onReceiveCallback(info) {
      if (info.connectionId == connection_id && info.data) {
        var view = new Uint8Array(info.data);
        for (var i = 0; i < view.length; i++) {
          input_buffer.push(view[i]);
        }

        while (input_buffer.length >= 4) {
          handleCommand(input_buffer.splice(0, 4));
        }
      }
    };

    var connect_button = document.createElement("input");
    connect_button.type = "button";
    connect_button.value = "Connect";
    connect_button.style.width = "120px";
    connect_button.onclick = function() {
      return function(event) {
        if (this.value === "Connect") {
          var port = dropdown.childNodes[dropdown.value].textContent;
          chrome.serial.connect(port, {bitrate: 115200}, function(info) {
            connection_id = info.connectionId;
            chrome.serial.onReceive.addListener(onReceiveCallback);
            sendCommand([SER_HANDSHAKE, SER_VERSION, 42, 42], true);
          });
          this.value = "Disconnect";
        } else {
          sendCommand([SER_DISCONNECT, 0, 0, 0]);
          connected = false;
          chrome.serial.disconnect(connection_id, function(result) {});
          connection_id = null;
          this.value = "Connect";
        }
      };
    }();
    elem.appendChild(connect_button);

    var refresh_button = document.createElement("input");
    refresh_button.type = "button";
    refresh_button.value = "Refresh Ports";
    refresh_button.style.width = "120px";
    refresh_button.onclick = function() {
      return function(event) {
        this.updatePorts();
      };
    }().bind(this);
    elem.appendChild(refresh_button);

    this.updatePorts = function() {
      return function() {
        dropdown.innerHTML = "";
        chrome.serial.getDevices(function(devices) {
          for (var i = 0; i < devices.length; i++) {
            var port = document.createElement("option");
            port.value = i;
            port.textContent = devices[i].path;
            dropdown.appendChild(port);
          }
        });
      };
    }();
  };

  function ModeControls(parent) {
    var elem = document.createElement("div");
    elem.style.margin = "5px";
    elem.style.textAlign = "center";
    parent.appendChild(elem);

    var field = document.createElement("input");
    field.id = "mode-save";
    field.type = "text";
    field.style.display = "block";
    field.style.width =  "170px";
    elem.appendChild(field);

    var button = document.createElement("button");
    button.type = "button";
    button.textContent = "Save Mode";
    button.style.width =  "100px";
    button.style.display = "block";
    button.style.margin = "0 auto";
    elem.appendChild(button);

    button.onclick = function(e) {
      // Save mode
      var name = field.value.replace(/\s/g, "-").toLowerCase();
      var modeobj = arrayToMode(data);
      modeobj.name = field.value;
      modeobj.id = name;

      // Update modelib with new mode data
      modelib[modeobj.id] = modeobj;

      // If no existing modeitem, create a new one!
      var elem = document.querySelector("#" + name);
      if (elem === null) {
        makeModeItem(modeobj);
      }

      var modeitem = document.getElementById(modeobj.id);
      modeitem.data = modeobj;

      // Write the file
      writeMode(modeobj);
    };
  };

  function BundleControls(parent) {
    var elem = document.createElement("div");
    elem.style.margin = "5px";
    elem.style.textAlign = "center";
    parent.appendChild(elem);

    var field = document.createElement("input");
    field.id = "firmware-save";
    field.type = "text";
    field.style.display = "block";
    field.style.width =  "150px";
    elem.appendChild(field);

    var button = document.createElement("button");
    button.type = "button";
    button.textContent = "Save Firmware";
    button.style.width =  "100px";
    button.style.display = "block";
    button.style.margin = "0 auto";
    elem.appendChild(button);

    button.onclick = function(e) {
      var num_modes = [bundle0.children.length, bundle1.children.length];

      var bundle_a = [];
      for (var i = 0; i < bundle0.children.length; i++) {
        var modeitem = bundle0.children[i];
        var modeobj = modelib[modeitem.data.id];
        bundle_a.push(modeToArray(modeobj));
      }

      var bundle_b = [];
      for (var i = 0; i < bundle1.children.length; i++) {
        var modeitem = bundle1.children[i];
        var modeobj = modelib[modeitem.data.id];
        bundle_b.push(modeToArray(modeobj));
      }

      if (field.value == "") {
        dialog.children[0].textContent = "Firmware must have a name.";
      } else if (field.value.includes(" ")) {
        dialog.children[0].textContent = "Firmware name must not have spaces.";
      } else {
        writeSource(field.value, num_modes, bundle_a, bundle_b);
        dialog.children[0].textContent = "Firmware saved.";
      }
      dialog.show();
    };
  };

  function initDragDrop() {
    var dnd = new DnDFileController('body', function(data) {
      var fileEntry = data.items[0].webkitGetAsEntry();

      fileEntry.file(function(file) {
        var reader = new FileReader();
        reader.onload = function(e) {
          var contents = e.target.result;
          var modeobj = JSON.parse(contents);
          modeobj.name = file.name.replace(".mode", "");
          modeobj.id = modeobj.name.replace(/\s/g, "-").toLowerCase();

          if (modelib[modeobj.id]) {
            modelib[modeobj.id] = modeobj;
            writeMode(modeobj);
          } else {
            modelib[modeobj.id] = modeobj;
            makeModeItem(modeobj);
            writeMode(modeobj);
          }
        }
        reader.readAsText(file);
      });
    });
  };

  initSettings();
  initUI();
  initDragDrop();

  close.onclick = function () { dialog.close(); }

  return {
    writeMode: writeMode,
    writeSource: writeSource,
    modelib: modelib,
    data: data
  };
}();
