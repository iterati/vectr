palette = [
  ["#d00000", "#b61c00", "#9c3800", "#825400", "#687000", "#4e8c00", "#34a800", "#1ac400",
   "#00e000", "#00c41e", "#00a83c", "#008c5a", "#007078", "#005496", "#0038b4", "#001cd2",
   "#0000f0", "#1a00d2", "#3400b4", "#4e0096", "#680078", "#82005a", "#9c003c", "#b6001e"],
  ["#680000", "#5b0e00", "#4e1c00", "#412a00", "#343800", "#274600", "#1a5400", "#0d6200",
   "#007000", "#00620f", "#00541e", "#00462d", "#00383c", "#002a4b", "#001c5a", "#000e69",
   "#000078", "#0d0069", "#1a005a", "#27004b", "#34003c", "#41002d", "#4e001e", "#5b000f"],
  ["#340000", "#2d0700", "#270e00", "#201500", "#1a1c00", "#132300", "#0d2a00", "#063100",
   "#003800", "#003107", "#002a0f", "#002316", "#001c1e", "#001525", "#000e2d", "#000734",
   "#00003c", "#060034", "#0d002d", "#130025", "#1a001e", "#200016", "#27000f", "#2d0007"],
  ["#1a0000", "#160300", "#130700", "#100a00", "#0d0e00", "#091100", "#061500", "#031800",
   "#001c00", "#001803", "#001507", "#00110b", "#000e0f", "#000a12", "#000716", "#00031a",
   "#00001e", "#03001a", "#060016", "#090012", "#0d000f", "#10000b", "#130007", "#160003"],

  ["#000000", "#0d0e10", "#687078", "#82545a", "#4e8c5a", "#4e5496", "#9c383c", "#8f702d",
   "#828c1e", "#689a2d", "#34a83c", "#279a78", "#1a8c96", "#2770a5", "#3438b4", "#682aa5",
   "#821c96", "#8f2a78", "#1a0000", "#141500", "#001c00", "#001518", "#000020", "#140018"],
  ["#000000", "#060708", "#34383c", "#412a2d", "#27462d", "#272a4b", "#4e1c1e", "#473816",
   "#41460f", "#344d16", "#1a541e", "#134d3c", "#0d464b", "#133852", "#1a1c5a", "#341552",
   "#410e4b", "#47153c", "#0d0000", "#0a0a00", "#000e00", "#000a0c", "#000010", "#0a000c"],
  ["#000000", "#030304", "#1a1c1e", "#201516", "#132316", "#131525", "#270e0f", "#231c0b",
   "#202307", "#1a260b", "#0d2a0f", "#09261e", "#062325", "#091c29", "#0d0e2d", "#1a0a29",
   "#200725", "#230a1e", "#060000", "#050500", "#000700", "#000506", "#000008", "#050006"],
  ["#000000", "#010102", "#0d0e0f", "#100a0b", "#09110b", "#090a12", "#130707", "#110e05",
   "#101103", "#0d1305", "#061507", "#04130f", "#031112", "#040e14", "#060716", "#0d0514",
   "#100312", "#11050f", "#030000", "#020200", "#000300", "#000203", "#000004", "#020003"]
];

var patterns = [
{
  "name": "Strobe",
  "args": [
    {"min": 0, "max": 9,   "name": "Group Size"},
    {"min": 0, "max": 9,   "name": "Skip After"},
    {"min": 1, "max": 100, "name": "Repeat Group"}
  ],
  "timings": [
    "Strobe",
    "Blank",
    "Gap",
  ]
},
{
  "name": "Tracer",
  "args": [
    {"min": 0, "max": 9,   "name": "Group Size"},
    {"min": 0, "max": 9,   "name": "Skip After"},
    {"min": 1, "max": 100, "name": "Repeat Tracer"}
  ],
  "timings": [
    "Color Strobe",
    "Color Blank",
    "Tracer Strobe",
    "Tracer Blank",
    "Gap"
  ]
},
{
  "name": "Morph",
  "args": [
    {"min": 1, "max": 100, "name": "Morph Steps"},
    {"min": 0, "max": 9,   "name": "Smooth or Fused"},
  ],
  "timings": [
    "Morph Strobe",
    "Morph Blank",
    "Solid Color",
    "Gap"
  ]
},
{
  "name": "Sword",
  "args": [
    {"min": 0, "max": 9,   "name": "Group Size"},
  ],
  "timings": [
    "Outer Strobe",
    "Outer Blank",
    "Inner Strobe",
    "Gap"
  ]
},
{
  "name": "Wave",
  "args": [
    {"min": 1, "max": 100, "name": "Steps"},
    {"min": 0, "max": 2,   "name": "Up, Down, or Both"},
    {"min": 0, "max": 1,   "name": "Color or Blank"},
    {"min": 0, "max": 1,   "name": "Change on Cycle or Step"}
  ],
  "timings": [
    "Base Strobe",
    "Base Blank",
    "Step Length"
  ]
},
{
  "name": "Stretch",
  "args": [
    {"min": 1, "max": 100, "name": "Steps"},
    {"min": 0, "max": 2,   "name": "Up, Down, or Both"},
    {"min": 0, "max": 1,   "name": "Color on Cycle or Step"}
  ],
  "timings": [
    "Base Strobe",
    "Base Blank",
    "Step Length"
  ]
},
{
  "name": "Shift",
  "args": [
    {"min": 1, "max": 100, "name": "Steps"},
    {"min": 0, "max": 2,   "name": "Up, Down, or Both"}
  ],
  "timings": [
    "Base Strobe",
    "Base Blank",
    "Step Length",
    "Gap"
  ]
},
{
  "name": "Triple",
  "args": [
    {"min": 1, "max": 100, "name": "Repeat A"},
    {"min": 1, "max": 100, "name": "Repeat B"},
    {"min": 0, "max": 100, "name": "Repeat C"},
    {"min": 0, "max": 8,   "name": "Skip Colors"}
  ],
  "timings": [
    "A Strobe",
    "A Blank",
    "B Strobe",
    "B Blank",
    "C Strobe",
    "C Blank",
    "Gap"
  ]
},
{
  "name": "Stepper",
  "args": [
    {"min": 1, "max": 7,   "name": "Steps"},
    {"min": 0, "max": 1,   "name": "Random Step?"},
    {"min": 0, "max": 1,   "name": "Random Color?"},
    {"min": 0, "max": 1,   "name": "Step Color or Blank"}
  ],
  "timings": [
    "Gap",
    "Step 1",
    "Step 2",
    "Step 3",
    "Step 4",
    "Step 5",
    "Step 6",
    "Step 7"
  ]
},
{
  "name": "Random",
  "args": [
    {"min": 0, "max": 1,   "name": "Random Color?"},
    {"min": 1, "max": 25,  "name": "Multiplier"}
  ],
  "timings": [
    "Strobe Low",
    "Strobe High",
    "Blank Low",
    "Blank High"
  ]
}
];

var colorbank = [
  [208, 0, 0],
  [182, 28, 0],
  [156, 56, 0],
  [130, 84, 0],
  [104, 112, 0],
  [78, 140, 0],
  [52, 168, 0],
  [26, 196, 0],
  [0, 224, 0],
  [0, 196, 30],
  [0, 168, 60],
  [0, 140, 90],
  [0, 112, 120],
  [0, 84, 150],
  [0, 56, 180],
  [0, 28, 210],
  [0, 0, 240],
  [26, 0, 210],
  [52, 0, 180],
  [78, 0, 150],
  [104, 0, 120],
  [130, 0, 90],
  [156, 0, 60],
  [182, 0, 30],
  [104, 112, 120],
  [130, 84, 90],
  [78, 140, 90],
  [78, 84, 150],
  [156, 56, 60],
  [143, 112, 45],
  [130, 140, 30],
  [104, 154, 45],
  [52, 168, 60],
  [39, 154, 120],
  [26, 140, 150],
  [39, 112, 165],
  [52, 56, 180],
  [104, 42, 165],
  [130, 28, 150],
  [143, 42, 120],
  [0, 0, 0],
  [13, 14, 16],
  [26, 0, 0],
  [20, 21, 0],
  [0, 28, 0],
  [0, 21, 24],
  [0, 0, 32],
  [20, 0, 24]
];

var data = [
  0,                                // type
  1, 0,                             // pattern
  1, 1, 5, 0,                       // args1
  0, 0, 0, 0,                       // args2
  5, 5, 5, 5, 5, 0, 0, 0,           // timings1
  6, 6, 6, 6, 6, 0, 0, 0,           // timings2
  7, 7, 7, 7, 7, 0, 0, 0,           // timings3
  1, 3, 1,
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

var read_listeners = {};
var send_listeners = {};
var vectrui = $('#vectr');
var version = "0.1";
var vectr_opts = {};
var vectr_obj = {};
var input_buffer = [];
var connection_id;


function readData(a, b) {
  data[a] = b;
  for (var i = 0; i < read_listeners[a].length; i++) {
    read_listeners[a][i](b);
  }
};

function sendData(a, b) {
  data[a] = b;
  for (var i = 0; i < send_listeners[a].length; i++) {
    send_listeners[a][i](b);
  }
  // console.log("send " + a + ": " + b);
  // TODO: send b to a
};

function sendCommand(cmd) {
  var buf = new ArrayBuffer(4);
  var view = new DataView(buf);
  view.setInt8(0, cmd[0]);
  view.setInt8(1, cmd[1]);
  view.setInt8(2, cmd[2]);
  view.setInt8(3, cmd[3]);
  chrome.serial.send(connection_id, buf, function() {});
};

function handleCommand(cmd) {
  if (cmd[0] == 100) { // Write
    readData(cmd[1], cmd[2]);
  } else if (cmd[0] == 200) { // Handshake
  } else if (cmd[0] == 201) { // Handshack
  }
};

function getColor(set, slot) {
  var r = ("0" + parseInt(data[(27 * set) + (3 * slot) + 38], 10).toString(16)).slice(-2);
  var g = ("0" + parseInt(data[(27 * set) + (3 * slot) + 39], 10).toString(16)).slice(-2);
  var b = ("0" + parseInt(data[(27 * set) + (3 * slot) + 40], 10).toString(16)).slice(-2);
  var hex = "#" + r + g + b;
  return hex;
};

function hex2rgb(hex) {
  return {
    r: parseInt(hex.substr(1, 2), 16),
    g: parseInt(hex.substr(3, 2), 16),
    b: parseInt(hex.substr(5, 2), 16)
  };
};


// Event Generators
function makeThreshSliderChange(fields, addr) {
  return function updateThreshVals(event, ui) {
    if (event.originalEvent) {
      var idx = $(ui.handle).data("ui-slider-handle-index");
      fields[idx].value = ui.values[idx];
      sendData(addr + idx, ui.values[idx]);
    }
  };
};

function makeThreshFieldChange(slider, addr) {
  return function updateThreshSlider(event) {
    var idx = Number(event.target.getAttribute("idx"));
    var val = Number(event.target.value);
    var values = slider.limitslider("values");
    var checks = [slider.limitslider("option", "min")].concat(values).concat([slider.limitslider("option", "max")]);

    if (val < checks[idx]) {
      val = checks[idx];
    } else if (val > checks[idx + 2]) {
      val = checks[idx + 2];
    }

    event.target.value = val;
    values[idx] = val;
    slider.limitslider("values", values);
    sendData(addr + idx, val);
  };
};

function makeThreshSliderListener(field, addr, idx) {
  return function (val) {
    var values = this.limitslider("values");
    values[idx] = val;
    field.value = val;
    this.limitslider("values", values);
    sendData(addr + idx, val);
  };
};

function makeSliderChange(field, addr) {
  // Send data, update field
  return function updateVal(event, ui) {
    field.val(ui.value);
    if (event.originalEvent) {
      sendData(addr, ui.value);
    }
  };
};

function makeFieldChange(slider, addr) {
  return function updateSlider(event) {
    if (event.target.value < slider.slider("option", "min")) {
      event.target.value = slider.slider("option", "min");
    } else if (event.target.value > slider.slider("option", "max")) {
      event.target.value = slider.slider("option", "max");
    }
    slider.slider("value", event.target.value);
    if (event.originalEvent) {
      sendData(addr, event.target.value);
    }
  };
};

function makeSliderListener(field, addr) {
  return function (val) {
    field.value = val;
    this.slider("value", val);
    sendData(addr, val);
  };
};

function makeUpdateVectrPattern(send_data) {
  return function (val) {
    var pattern = patterns[val];

    vectrui.children('.pattern').find('.dropdown').each(function(i, dropdown) {
      $(dropdown).val(pattern.name);
    });

    vectrui.children('.pattern').find('.arg').each(function(i, column) {
      if (i >= pattern.args.length) {
        $(column).hide();
      } else {
        $(column).show();
        $(column).children('.label').text(pattern.args[i].name);
        $(column).children('.container').children('.slider')
          .slider("option", "min", pattern.args[i].min)
          .slider("option", "max", pattern.args[i].max)
          .slider("value", pattern.args[i].min);
        $(column).children('.container').children('.value').val(pattern.args[i].min);
      }
    });

    vectrui.children('.timings').find('.row').each(function(i, row) {
      if (i >= pattern.timings.length) {
        $(row).hide();
      } else {
        $(row).show();
        $(row).find('.label').each(function(j, label) {
          $(label).text(pattern.timings[i]);
        });
      }
    });

    if (send_data) {
      for (var i = 0; i < 128; i++) {
        sendData(i, data[i]);
      }
    } else {
      sendData(1, val);
    }
  };
};

function arrayToMode(arr) {
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
    colors: [
      [
        [arr[38], arr[39], arr[40]],
        [arr[41], arr[42], arr[43]],
        [arr[44], arr[45], arr[46]],
        [arr[47], arr[48], arr[49]],
        [arr[50], arr[51], arr[52]],
        [arr[53], arr[54], arr[55]],
        [arr[56], arr[57], arr[58]],
        [arr[59], arr[60], arr[61]],
        [arr[62], arr[63], arr[64]]
      ],
      [
        [arr[65], arr[66], arr[67]],
        [arr[68], arr[69], arr[70]],
        [arr[71], arr[72], arr[73]],
        [arr[74], arr[75], arr[76]],
        [arr[77], arr[78], arr[79]],
        [arr[80], arr[81], arr[82]],
        [arr[83], arr[84], arr[85]],
        [arr[86], arr[87], arr[88]],
        [arr[89], arr[90], arr[91]]
      ],
      [
        [arr[92], arr[93], arr[94]],
        [arr[95], arr[96], arr[97]],
        [arr[98], arr[99], arr[100]],
        [arr[101], arr[102], arr[103]],
        [arr[104], arr[105], arr[106]],
        [arr[107], arr[108], arr[109]],
        [arr[110], arr[111], arr[112]],
        [arr[113], arr[114], arr[115]],
        [arr[116], arr[117], arr[118]]
      ]
    ],
    tr_meta: [arr[119], arr[120], arr[121], arr[122]],
    tr_flux: [arr[123], arr[124], arr[125], arr[126]],
    trigger: arr[127]
  };
}

function modeToArray(m) {
  return [
    m.type,
    m.pattern[0], m.pattern[1],

    m.args[0][0], m.args[0][1], m.args[0][2], m.args[0][3],
    m.args[1][0], m.args[1][1], m.args[1][2], m.args[1][3],

    m.timings[0][0], m.timings[0][1], m.timings[0][2], m.timings[0][3], m.timings[0][4], m.timings[0][5], m.timings[0][6], m.timings[0][7],
    m.timings[1][0], m.timings[1][1], m.timings[1][2], m.timings[1][3], m.timings[1][4], m.timings[1][5], m.timings[1][6], m.timings[1][7],
    m.timings[2][0], m.timings[2][1], m.timings[2][2], m.timings[2][3], m.timings[2][4], m.timings[2][5], m.timings[2][6], m.timings[2][7],

    m.numc[0], m.numc[1], m.numc[2],

    m.colors[0][0][0], m.colors[0][0][1], m.colors[0][0][2],
    m.colors[0][1][0], m.colors[0][1][1], m.colors[0][1][2],
    m.colors[0][2][0], m.colors[0][2][1], m.colors[0][2][2],
    m.colors[0][3][0], m.colors[0][3][1], m.colors[0][3][2],
    m.colors[0][4][0], m.colors[0][4][1], m.colors[0][4][2],
    m.colors[0][5][0], m.colors[0][5][1], m.colors[0][5][2],
    m.colors[0][6][0], m.colors[0][6][1], m.colors[0][6][2],
    m.colors[0][7][0], m.colors[0][7][1], m.colors[0][7][2],
    m.colors[0][8][0], m.colors[0][8][1], m.colors[0][8][2],

    m.colors[1][0][0], m.colors[1][0][1], m.colors[1][0][2],
    m.colors[1][1][0], m.colors[1][1][1], m.colors[1][1][2],
    m.colors[1][2][0], m.colors[1][2][1], m.colors[1][2][2],
    m.colors[1][3][0], m.colors[1][3][1], m.colors[1][3][2],
    m.colors[1][4][0], m.colors[1][4][1], m.colors[1][4][2],
    m.colors[1][5][0], m.colors[1][5][1], m.colors[1][5][2],
    m.colors[1][6][0], m.colors[1][6][1], m.colors[1][6][2],
    m.colors[1][7][0], m.colors[1][7][1], m.colors[1][7][2],
    m.colors[1][8][0], m.colors[1][8][1], m.colors[1][8][2],

    m.colors[2][0][0], m.colors[2][0][1], m.colors[2][0][2],
    m.colors[2][1][0], m.colors[2][1][1], m.colors[2][1][2],
    m.colors[2][2][0], m.colors[2][2][1], m.colors[2][2][2],
    m.colors[2][3][0], m.colors[2][3][1], m.colors[2][3][2],
    m.colors[2][4][0], m.colors[2][4][1], m.colors[2][4][2],
    m.colors[2][5][0], m.colors[2][5][1], m.colors[2][5][2],
    m.colors[2][6][0], m.colors[2][6][1], m.colors[2][6][2],
    m.colors[2][7][0], m.colors[2][7][1], m.colors[2][7][2],
    m.colors[2][8][0], m.colors[2][8][1], m.colors[2][8][2],

    m.tr_meta[0], m.tr_meta[1], m.tr_meta[2], m.tr_meta[3],
    m.tr_flux[0], m.tr_flux[1], m.tr_flux[2], m.tr_flux[3],
    m.trigger
  ];
};

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


function getSerialPorts() {
  chrome.serial.getDevices(function(devices) {
    for (var i = 0; i < devices.length; i++) {
      $('select#portList').append('<option value="' + devices[i].path + '">' + devices[i].path + '</option>');
    }
  });
};
getSerialPorts();

for (var i = 0; i < 128; i++) {
  readData(i, data[i]);
  read_listeners[i] = [];
  send_listeners[i] = [];
}

chrome.storage.local.get("vectr", function(data) {
  if (!data.vectr) {
    data.vectr = {};
  }
  if (data.vectr.version != version) {
    data.vectr.version = version;
  }
  if (!data.vectr.dir_id) {
    chrome.fileSystem.chooseEntry({type: "openDirectory"}, function(entry) {
      data.vectr.dir_id = chrome.fileSystem.retainEntry(entry);
      vectr_obj.dir_root = entry;
      entry.getDirectory('firmwares', {create: true}, function(entry) {
        vectr_obj.dir_firmwares = entry;
      });
      entry.getDirectory('modes', {create: true}, function(entry) {
        vectr_obj.dir_modes = entry;
      });
    });
  } else {
    chrome.fileSystem.restoreEntry(data.vectr.dir_id, function(entry) {
      vectr_dir_entry = entry;
    });
  }
  vectr_ops = data;
  chrome.storage.local.set(data);
});

$('button#refresh').click(function() {
  getSerialPorts();
});

$('button#connect').click(function() {
  var clicks = $(this).data('clicks');

  if (!clicks) {
    var port = $('select#portList').val();
    chrome.serial.connect(port, {bitrate: 115200}, function(info) {
      connection_id = info.connectionId;
      $("button#open").html("Disconnect");
      chrome.serial.onReceive.addListener(onReceiveCallback);
    });
  } else {
    chrome.serial.disconnect(connection_id, function(result) {
      $("button#open").html("Connect");
    });
  }

  $(this).data("clicks", !clicks);
});

vectrui.children('.pattern').each(function(index, item) {
  var elem = $(item);

  var column = $('<div class="container inline pattern"></div>')
    .css("width", "90px")
    .css("padding", "5px")
    .appendTo(elem);

  var label = $('<div class="span">Pattern</div>').appendTo(column);
  var updateVectrPattern = makeUpdateVectrPattern(true);
  var dropdown = $('<select class="span dropdown"></select>')
    .change(function() {
      updateVectrPattern($('option:selected', $(this)).index());
    }).appendTo(column);

  for (var p in patterns) {
    $('<option>' + patterns[p].name + '</option>').appendTo(dropdown);
  }

  read_listeners[1].push(makeUpdateVectrPattern(false));

  for (var i = 0; i < 4; i++) {
    column = $('<div class="container inline arg"></div>')
      .css("width", "165px")
      .css("padding", "5px")
      .appendTo(elem);

    label = $('<div class="span label">Arg ' + i + '</div>').appendTo(column);

    var slider_container = $('<div class="container"></div>').appendTo(column);
    var field = $('<input class="inline value" type="text" idx="' + i + '">')
      .appendTo(slider_container);

    var slider = $('<div class="inline slider"></div>').slider({
      min: 0, max: 100, value: 0,
      slide: makeSliderChange(field, 3 + i),
      change: makeSliderChange(field, 3 + i)
    }).css("width", "107px")
    .appendTo(slider_container);

    field.change(makeFieldChange(slider, 3 + i));
    read_listeners[3 + i].push(makeSliderListener(field, 3 + i).bind(slider));
  }
});

vectrui.children('.timings').each(function(index, item) {
  var elem = $(item);
  for (var i = 0; i < 8; i++) {
    var container = $('<div class="container span"></div>')
      .css("min-height", "25px")
      .appendTo(elem);
    var row = $('<div class="container span row" idx="' + i + '"></div>').appendTo(container);
    var label = $('<div class="inline label">Timing ' + i + '</div>')
      .css("width", "100px")
      .css("padding", "2px 5px")
      .appendTo(row);

    for (var j = 0; j < 3; j++) {
      var addr = (8 * j) + i + 11;
      var container = $('<div class="container inline"></div>')
        .css("width", "220px")
        .css("padding", "2px 5px")
        .appendTo(row);

      var field = $('<input class="inline value" type="text">')
        .appendTo(container);

      var slider = $('<div class="inline slider"></div>').slider({
        min: 0, max: 125, step: 0.5, value: 0,
        slide: makeSliderChange(field, addr),
        change: makeSliderChange(field, addr)
      }).css("width", "162px")
      .appendTo(container);

      field.change(makeFieldChange(slider, addr));
      read_listeners[addr].push(makeSliderListener(field, addr).bind(slider));
    }
  }
});

vectrui.children('.thresh').each(function(index, item) {
  var elem = $(item).addClass("span");
  var target = item.getAttribute("target");
  var addr = (target == "pattern") ? 119 : 123;

  var label = $('<div class="span"></div>')
   .text(target.charAt(0).toUpperCase() + target.slice(1) + ' Thresholds')
   .css("padding", "5px")
   .appendTo(elem);

  var slider = $('<div class="span"></div>')
    .css("width", "798px")
    .appendTo(elem);

  var values = $('<div class="container span"></div>')
    .css("padding", "5px")
    .appendTo(elem);

  for (var i = 0; i < 4; i++) {
    var value = $('<input class="inline" type="text" idx="' + i + '">')
      .css("width", "25px")
      .css("margin", "5px 82px")
      .change(makeThreshFieldChange(slider, addr))
      .appendTo(values);
  }

  slider.limitslider({
    min: 0, max: 32, gap: 0,
    values: [0, 0, 0, 0],
    ranges: [
      {styleClass: 'range-0'},
      {styleClass: 'range-01'},
      {styleClass: 'range-1'},
      {styleClass: 'range-12'},
      {styleClass: 'range-2'}
    ],
    slide:  makeThreshSliderChange(values.children(), addr),
    change: makeThreshSliderChange(values.children(), addr)
  });

  read_listeners[addr + 0].push(makeThreshSliderListener(values.children()[0], addr, 0).bind(slider));
  read_listeners[addr + 1].push(makeThreshSliderListener(values.children()[1], addr, 1).bind(slider));
  read_listeners[addr + 2].push(makeThreshSliderListener(values.children()[2], addr, 2).bind(slider));
  read_listeners[addr + 3].push(makeThreshSliderListener(values.children()[3], addr, 3).bind(slider));
});

vectrui.children('.colors').each(function(idx, div) {
  makeGetColor = function(color, set, slot, channel) {
    return function(val) {
      var rgb = hex2rgb(getColor(set, slot));
      color.spectrum("set", getColor(set, slot));
      if (channel == 0) {
        sendData((27 * set) + (3 * slot) + 38, rgb.r);
      } else if (channel == 1) {
        sendData((27 * set) + (3 * slot) + 39, rgb.g);
      } else if (channel == 2) {
        sendData((27 * set) + (3 * slot) + 40, rgb.b);
      }
    };
  };

  makeSendColor = function(addr) {
    return function(color)  {
      var rgb = hex2rgb(color.toHexString());
      sendData(addr + 0, rgb.r);
      sendData(addr + 1, rgb.g);
      sendData(addr + 2, rgb.b);
    };
  };

  makeShowHideColor = function(idx) {
    return function(val) {
      if (idx >= val) {
        $(".color." +  this.attr("id")).hide();
        // this.hide();
      } else {
        $(".color." +  this.attr("id")).show();
        // this.show();
      }
    };
  };

  var elem = $(div);
  for (var i = 0; i < 3; i++) {
    var row = $('<div class="container span"></div>').appendTo(elem);

    for (var j = 0; j < 9; j++) {
      var color_addr = (27 * i) + (3 * j) + 38;
      var color = $('<input type="text" id="color' + color_addr + '">').appendTo(row).hide();
      $('#color' + color_addr)
        .spectrum({
          addr: color_addr,
          showPalette: true,
          palette: palette,
          showButtons: false,
          showInput: true,
          localStorageKey: "spectrum.colors",
          maxSelectionSize: 24,
          preferredFormat: "rgb",
          change: makeSendColor(color_addr),
          move: makeSendColor(color_addr)
        });
      // var color = $('.color.color' + color_addr);

      read_listeners[color_addr + 0].push(makeGetColor(color, i, j, 0));
      read_listeners[color_addr + 1].push(makeGetColor(color, i, j, 1));
      read_listeners[color_addr + 2].push(makeGetColor(color, i, j, 2));

      read_listeners[35 + i].push(makeShowHideColor(j).bind(color));
      send_listeners[35 + i].push(makeShowHideColor(j).bind(color));
    }

    var slider_container = $('<div class="container inline"></div>').prependTo(row);
    var field = $('<input class="inline value" type="text">')
      .appendTo(slider_container);

    var slider = $('<div class="inline slider"></div>').slider({
      min: 1, max: 9, value: 1,
      slide: makeSliderChange(field, 35 + i),
      change: makeSliderChange(field, 35 + i)
    }).css("width", "100px")
    .appendTo(slider_container);

    field.change(makeFieldChange(slider, 35 + i));
    read_listeners[35 + i].push(makeSliderListener(field, 35 + i).bind(slider));
  }
});
