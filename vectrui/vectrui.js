var VectrUI = function() {
  'use strict';

  jQuery.colorpicker.swatches.custom_array = [
    {name: 'red',         r: 208, g: 0, b: 0},
    {name: 'sunrise',     r: 182, g: 28, b: 0},
    {name: 'orange',      r: 156, g: 56, b: 0},
    {name: 'gold',        r: 130, g: 84, b: 0},
    {name: 'yellow',      r: 104, g: 112, b: 0},
    {name: 'lemon',       r: 78, g: 140, b: 0},
    {name: 'lime',        r: 52, g: 168, b: 0},
    {name: 'virus',       r: 26, g: 196, b: 0},
    {name: 'green',       r: 0, g: 224, b: 0},
    {name: 'sea',         r: 0, g: 196, b: 30},
    {name: 'aqua',        r: 0, g: 168, b: 60},
    {name: 'turqoise',    r: 0, g: 140, b: 90},
    {name: 'cyan',        r: 0, g: 112, b: 120},
    {name: 'baby blue',   r: 0, g: 84, b: 150},
    {name: 'sky',         r: 0, g: 56, b: 180},
    {name: 'royal blue',  r: 0, g: 28, b: 210},
    {name: 'blue',        r: 0, g: 0, b: 240},
    {name: 'indigo',      r: 26, g: 0, b: 210},
    {name: 'purple',      r: 52, g: 0, b: 180},
    {name: 'violet',      r: 78, g: 0, b: 150},
    {name: 'magenta',     r: 104, g: 0, b: 120},
    {name: 'blush',       r: 130, g: 0, b: 90},
    {name: 'pink',        r: 156, g: 0, b: 60},
    {name: 'sunset',      r: 182, g: 0, b: 30},
    {name: 'black',       r: 0, g: 0, b: 0}
  ];

  var main = document.querySelector("#main");
  var editor = document.querySelector("#editor");
  var bundles = document.querySelector("#bundles");
  var modes = document.querySelector("#modes");

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

  var readListeners = [];
  var updateListeners = [];
  for (var i = 0; i < 128; i++) {
    readListeners[i] = [];
    updateListeners[i] = [];
  }

  function sendCommand(cmd) {
    // TODO
  };

  function sendData(addr, val) {
    // Updates in-memory array and sends value to light
    data[addr] = val;
    console.log(addr + ": " + val);
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

  function getColorHex(set, slot) {
    var r = ("0" + parseInt(data[(27 * set) + (3 * slot) + 38], 10).toString(16)).slice(-2);
    var g = ("0" + parseInt(data[(27 * set) + (3 * slot) + 39], 10).toString(16)).slice(-2);
    var b = ("0" + parseInt(data[(27 * set) + (3 * slot) + 40], 10).toString(16)).slice(-2);
    var hex = "#" + r + g + b;
    return hex;
  };

  function hex2rgb(hex) {
    return {
      r: parseInt(hex.substr(0, 2), 16),
      g: parseInt(hex.substr(2, 2), 16),
      b: parseInt(hex.substr(4, 2), 16)
    };
  }


  function SliderField(parent, opts) {
    var opts = opts || {};
    if (opts.min === void 0) { opts.min = 0; }
    if (opts.max === void 0) { opts.max = 100; }
    if (opts.value === void 0) { opts.value = opts.min; }
    if (opts.step === void 0) { opts.step = 1; }
    if (opts.addr === void 0) { opts.addr = 0; }
    if (opts.multiplier === void 0) { opts.multiplier = 1; }
    if (opts.width === void 0) { opts.size = 100; }

    var sliderChange = function () {
      return function(event, ui) {
        field.value = ui.value;
        if (event.originalEvent) {
          updateData(opts.addr, ui.value * opts.multiplier);
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
    slider.style.float = "right";
    slider.style.width = (opts.width - 60) + "px";
    slider.style.marginLeft = "15px";
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
          updateData(1 + pattern_idx, this.value);
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
    spacer.style.minHeight = "20px";
    elem.appendChild(spacer);

    var colors = document.createElement("div");
    elem.appendChild(colors);

    new ColorSetRow(colors, 0);
    new ColorSetRow(colors, 1);
    new ColorSetRow(colors, 2);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "30px";
    elem.appendChild(spacer);

    var timingThresh = ThreshRow(elem, 0, 4);

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
            // TODO
            // send defaults
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

    new PatternRow(elem, 0);
    new ColorSetRow(elem, 0);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "30px";
    elem.appendChild(spacer);

    // TODO: Trigger
    new ThreshRow(elem, 0, 2);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "10px";
    elem.appendChild(spacer);

    new PatternRow(elem, 1);
    new ColorSetRow(elem, 1);

    var spacer = document.createElement("div");
    spacer.style.minHeight = "40px";
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
            // TODO
            // send defaults
          }
        } else {
          elem.style.display = 'none';
        }
      };
    };

    readListeners[0].push(typeListener(false));
    updateListeners[0].push(typeListener(true));
  };


  function ColorSetRow(parent, set_idx, prefix) {
    // Numc sliderfield + colorpickers
    function ColorSlot(parent, slot_idx) {
      // Color picker
      var addr = 38 + (27 * set_idx) + (3 * slot_idx);
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
          if (color.colorPicker.generated) {
            var rgb = hex2rgb(color.formatted);
            sendData(addr + 0, rgb.r);
            sendData(addr + 1, rgb.g);
            sendData(addr + 2, rgb.b);
            color_target.style.background = "#" + color.formatted;
          }
        };
      }();

      var viewColor = function() {
        return function(event, ui) {
          sendCommand([230, set_idx, slot_idx, 0]);
        };
      }();

      var viewMode = function() {
        return function(event, ui) {
          sendCommand([220, 0, 0, 0]);
        };
      }();

      $(color_input).colorpicker({
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
          $(color_input).colorpicker("setColor", getColorHex(set_idx, slot_idx));
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
    elem.style.width = "535px";
    parent.appendChild(elem);

    var slider = new SliderField(elem, {
      min: 1,
      max: 9,
      width: 200,
      addr: 35 + set_idx
    });

    var slots = [];
    var color_container = document.createElement("div");
    color_container.style.marginLeft = "20px";
    color_container.style.display = "inline-block";
    elem.appendChild(color_container);

    for (var i = 0; i < 9; i++) {
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
    var addr = 119 + (4 * thresh_idx);
    var elem = document.createElement("div");
    elem.style.margin = "10px";
    parent.appendChild(elem);

    var value_elems = [];

    var ranges;
    var def_values;
    if (thresh_vals == 2) {
      ranges = [
        {styleClass: 'trigger-0'},
        {styleClass: 'trigger-01'},
        {styleClass: 'trigger-1'}
      ];
      def_values = [4, 28];
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

    var slider = document.createElement("div");
    elem.appendChild(slider);

    var values_container = document.createElement("div");
    values_container.style.display = "flex";
    values_container.style.justifyContent = "space-between";
    values_container.style.listStyleType = "none";
    elem.appendChild(values_container);

    for (var i = 0; i < thresh_vals; i++) {
      var valueChange = function(idx) {
        return function(event) {
          var value_addr = 119 + (4 * thresh_idx) + idx;
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
          sendData(value_addr, val);
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

      var valueListener = function() {
        return function(val) {
          var values = $(slider).limitslider("values");
          $(slider).limitslider("values", values);
        }
      }();

      readListeners[addr].push(valueListener);
    }

    var threshChange = function() {
      return function(event, ui) {
        if (event.originalEvent) {
          _e = ui;
          var idx = $(ui.handle).data("ui-slider-handle-index");
          value_elems[idx].value = ui.values[idx];
          sendData(addr, ui.values[idx]);
        }
      };
    }();

    $(slider).limitslider({
      min: 0, max: 32, gap: 0,
      values: def_values,
      ranges: ranges,
      slide: threshChange,
      change: threshChange
    });
  };

  // TODO Make serial control
  // TODO settings handling
  // TODO mode saving/loading
  // TODO modes library
  // TODO bundles
  // TODO generate source UI

  return {
    VectrEditor: VectrEditor,
    PrimerEditor: PrimerEditor,

    PatternRow: PatternRow,
    TimingColumn: TimingColumn,
    ColorSetRow: ColorSetRow,
    ThreshRow: ThreshRow,

    updateData: updateData,
    readData: readData,
    sendData: sendData,

    readListeners: readListeners,
    updateListeners: updateListeners
  };
}();
