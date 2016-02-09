class PrimerMode {
  boolean use_gui = true;

  static final int _TRIGGER_MODE = 1;
  static final int _TRIGGER_THRESH = 2;
  static final int _PATTERN = 4;
  static final int _ARGS = 6;
  static final int _TIMINGS = 16;
  static final int _NUMCOLORS = 32;
  static final int _COLORS = 34;
  static final int _PADDING = 88;

  int       triggerMode = 0;
  int[]     triggerThresh = new int[2];
  int[]     pattern = new int[2];
  int[][]   args = new int[2][5];
  int[][]   timings = new int[2][8];
  int[]     numColors = new int[2];
  int[][][] colors = new int[2][9][3];

  //********************************************************************************
  // GUI Elements
  //********************************************************************************
  Group gPrimer;
  Group gPatternArgs;
  Group gTimings;
  Group gColors;

  Textlabel tlTriggerMode;
  DropdownList dlTriggerMode;
  TriggerRange trTriggerThresh;
  Textlabel[] tlPatternLabel = new Textlabel[2];
  DropdownList[] dlPattern = new DropdownList[2];
  Textlabel[][] tlArgLabels = new Textlabel[2][5];
  Slider[][] slArgs = new Slider[2][5];
  Textlabel[][] tlTimingLabels = new Textlabel[2][8];
  Slider[][] slTimings = new Slider[2][8];
  Slider[] slNumColors = new Slider[2];
  Button[][] bColors = new Button[2][9];

  Button bSelectedColor;
  int color_set = -1;
  int color_slot = -1;


  PrimerMode() {
    use_gui = false;
  }

  PrimerMode(Group g) {
    gPrimer = g;

    tlTriggerMode = cp5.addTextlabel("PrimerTriggerModeLabel")
        .setGroup(gPatternArgs)
        .setValue("Trigger Mode")
        .setPosition(640, -40)
        .setSize(80, 20)
        .setColorValue(color(240));

    dlTriggerMode = cp5.addDropdownList("PrimerTriggerMode")
      .setGroup(gPrimer)
      .setId(ID_TRIGGER_MODE)
      .setPosition(150, -45)
      .setSize(80, 120)
      .setItems(TRIGGERMODES);
    style(dlTriggerMode);

    trTriggerThresh = new TriggerRange(cp5, "PrimerTriggerThresh")
      .setGroup(gPrimer)
      .setId(ID_TRIGGER_TRESH)
      .setPosition(4, 15)
      .setLabel("Trigger Thresholds");

    gPatternArgs = cp5.addGroup("PrimerPatternArgs")
      .setGroup(gPrimer)
      .setPosition(0, 75)
      .setSize(800, 100)
      .hideBar()
      .hideArrow();

    gTimings = cp5.addGroup("PrimerTimings")
      .setGroup(gPrimer)
      .setPosition(0, 210)
      .hideBar()
      .hideArrow();

    gColors = cp5.addGroup("PrimerColorGroup")
      .setGroup(gPrimer)
      .setPosition(0, 480)
      .hideBar()
      .hideArrow();

    gPatternArgs.bringToFront();

    // Pattern
    for (int i = 1; i >= 0; i--) {
      tlPatternLabel[i] = cp5.addTextlabel("PrimerPatternLabel" + i)
          .setGroup(gPatternArgs)
          .setValue("Pattern " + (i + 1))
          .setPosition(0, 50 * i)
          .setSize(80, 20)
          .setColorValue(color(240));

      dlPattern[i] = cp5.addDropdownList("PrimerPattern" + i)
        .setGroup(gPatternArgs)
        .setId(ID_PATTERN + i)
        .setPosition(0, 20 + (50 * i))
        .setSize(80, 160)
        .setItems(PATTERNS);
      style(dlPattern[i]);

      for (int j = 0; j < 5; j++) {
        tlArgLabels[i][j] = cp5.addTextlabel("PrimerArgLabels" + i + "." + j)
          .setGroup(gPatternArgs)
          .setValue("Arg " + ((5 * i) + j + 1))
          .setPosition(150 + (137 * j), 50 * i)
          .setSize(100, 20)
          .setColorValue(color(240));

        slArgs[i][j] = cp5.addSlider("PrimerArgs" + i + "." + j)
          .setGroup(gPatternArgs)
          .setId(ID_ARG + (5 * i) + j)
          .setLabel("")
          .setPosition(150 + (137 * j), 20 + (50 * i));
        style(slArgs[i][j], 101, 0, 10);
      }

      for (int j = 0; j < 8; j++) {
        tlTimingLabels[i][j] = cp5.addTextlabel("PrimerTimingLabels" + i + "." + j)
          .setGroup(gTimings)
          .setValue("Timing" + (j + 1))
          .setPosition(450 * i, 30 * j)
          .setSize(200, 20)
          .setColorValue(color(240));

        slTimings[i][j] = cp5.addSlider("PrimerTimings" + i + "." + j)
          .setGroup(gTimings)
          .setId(ID_TIMING + (8 * i) + j)
          .setLabel("")
          .setPosition(150 + (450 * i), 30 * j);
        style(slTimings[i][j], 201, 0, 200);
      }

      slNumColors[i] = cp5.addSlider("PrimerNumColors" + i)
        .setGroup(gColors)
        .setId(ID_NUMCOLORS + i)
        .setLabel("")
        .setPosition(0, 30 + (40 * i));
      style(slNumColors[i], 90, 1, 9);

      for (int j = 0; j < 9; j++) {
        bColors[i][j] = cp5.addButton("PrimerColors" + i + "." + j)
          .setGroup(gColors)
          .setId(ID_COLORS + (i * 9) + j)
          .setLabel("")
          .setSize(32, 32)
          .setPosition(114 + (40 * j), 24 + (40 * i));
      }
    }

    bSelectedColor = cp5.addButton("PrimerSelectedColor")
      .setGroup(gColors)
      .setSize(40, 40)
      .setColorBackground(color(255))
      .setColorForeground(color(255))
      .setColorActive(color(255))
      .setLabel("")
      .hide();

    gPatternArgs.bringToFront();
    dlTriggerMode.bringToFront();
  }

  int geta(int addr) {
    if (addr < _TRIGGER_THRESH) {   return triggerMode;
    } else if (addr < _PATTERN) {   return triggerThresh[(addr - _TRIGGER_THRESH + 1) % 2];
    } else if (addr < _ARGS) {      return pattern[addr - _PATTERN];
    } else if (addr < _TIMINGS) {   return args[(addr - _ARGS) / 5][(addr - _ARGS) % 5];
    } else if (addr < _NUMCOLORS) { return timings[(addr - _TIMINGS) / 8][(addr - _TIMINGS) % 8];
    } else if (addr < _COLORS) {    return numColors[addr - _NUMCOLORS];
    } else if (addr < _PADDING) {   return colors[(addr - _COLORS) / 27][((addr - _COLORS) % 27) / 3][(addr - _COLORS) % 3];
    }
    return 0;
  }

  void seta(int addr, int val) {
    if (addr < _TRIGGER_THRESH) {   setTriggerMode(val);
    } else if (addr < _PATTERN) {   setTriggerThresh((addr - _TRIGGER_THRESH + 1) % 2, val);
    } else if (addr < _ARGS) {      setPattern(addr - _PATTERN, val);
    } else if (addr < _TIMINGS) {   setArgs(addr - _ARGS, val);
    } else if (addr < _NUMCOLORS) { setTimings(addr - _TIMINGS, val);
    } else if (addr < _COLORS) {    setNumColors(addr - _NUMCOLORS, val);
    } else if (addr < _PADDING) {   setColor(addr - _COLORS, val);
    }
  }

  void deselectColor() {
    bSelectedColor.hide();
    color_set = color_slot = -1;
  }

  boolean selectColor(int i) {
    return selectColor(i / 9, i % 9);
  }

  boolean selectColor(int _set, int _color) {
    if (_set < 0 || _set >= 3 || _color < 0 || _color >= numColors[_set]) {
    } else {
      float[] pos = bColors[_set][_color].getPosition();
      color_set = _set;
      color_slot = _color;
      bSelectedColor.setPosition(pos[0] - 4, pos[1] - 4).show();
      bColors[_set][_color].bringToFront();
      mode.gColorEdit.show();
      mode.slColorValues[0].setBroadcast(false).setValue(colors[_set][_color][0]).setBroadcast(true);
      mode.slColorValues[1].setBroadcast(false).setValue(colors[_set][_color][1]).setBroadcast(true);
      mode.slColorValues[2].setBroadcast(false).setValue(colors[_set][_color][2]).setBroadcast(true);
      return true;
    }
    return false;
  }

  void updateColor(int _set, int _color) {
    int c = translateColor(colors[_set][_color]);
    if (use_gui) {
      bColors[_set][_color].setColorBackground(c);
      bColors[_set][_color].setColorForeground(c);
      bColors[_set][_color].setColorActive(c);

      if (_set == color_set && _color == color_slot) {
        mode.slColorValues[0].setBroadcast(false).setValue(colors[_set][_color][0]).setBroadcast(true);
        mode.slColorValues[1].setBroadcast(false).setValue(colors[_set][_color][1]).setBroadcast(true);
        mode.slColorValues[2].setBroadcast(false).setValue(colors[_set][_color][2]).setBroadcast(true);
      }
    }
  }

  void updateArg(int _set, int i, String label, int _min, int _max) {
    tlArgLabels[_set][i].setValue(label).show();
    slArgs[_set][i].setBroadcast(false)
      .setRange(_min, _max)
      .setNumberOfTickMarks(_max - _min + 1)
      .showTickMarks(false)
      .setBroadcast(true)
      .show();
  }

  void updateArg(int _set, int i) {
    tlArgLabels[_set][i].hide();
    slArgs[_set][i].hide();
  }

  void updateTiming(int _set, int i, String label) {
    tlTimingLabels[_set][i].setValue(label).show();
    slTimings[_set][i].show();
  }

  void updateTiming(int _set, int i) {
    tlTimingLabels[_set][i].hide();
    slTimings[_set][i].hide();
  }

  void resetPatternGui(int i) {
    switch (pattern[i]) {
      case 0:
        updateArg(i, 0, "Group Size", 0, 9);
        updateArg(i, 1, "Skip After", 0, 9);
        updateArg(i, 2, "Repeat Group", 1, 100);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Strobe");
        updateTiming(i, 1, "Blank");
        updateTiming(i, 2, "Tail Blank");
        updateTiming(i, 3);
        updateTiming(i, 4);
        updateTiming(i, 5);
        updateTiming(i, 6);
        updateTiming(i, 7);
        break;
      case 1:
        updateArg(i, 0, "Repeat Strobe", 1, 100);
        updateArg(i, 1, "Repeat Tracer", 1, 100);
        updateArg(i, 2);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Strobe");
        updateTiming(i, 1, "Blank");
        updateTiming(i, 2, "Tracer Strobe");
        updateTiming(i, 3, "Tracer Blank");
        updateTiming(i, 4, "Split Blank");
        updateTiming(i, 5);
        updateTiming(i, 6);
        updateTiming(i, 7);
        break;
      case 2:
        updateArg(i, 0, "Group Size", 0, 9);
        updateArg(i, 1);
        updateArg(i, 2);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Strobe");
        updateTiming(i, 1, "Blank");
        updateTiming(i, 2, "Center Strobe");
        updateTiming(i, 3, "Tail Blank");
        updateTiming(i, 4);
        updateTiming(i, 5);
        updateTiming(i, 6);
        updateTiming(i, 7);
        break;
      case 3:
        updateArg(i, 0, "Repeat First", 1, 100);
        updateArg(i, 1, "Repeat Second", 1, 100);
        updateArg(i, 2, "Repeat Third", 1, 100);
        updateArg(i, 3, "Skip Colors", 0, 8);
        updateArg(i, 4, "Use Third", 0, 1);
        updateTiming(i, 0, "First Strobe");
        updateTiming(i, 1, "First Blank");
        updateTiming(i, 2, "Second Strobe");
        updateTiming(i, 3, "Second Blank");
        updateTiming(i, 4, "Third Strobe");
        updateTiming(i, 5, "Third Blank");
        updateTiming(i, 6, "Separating Blank");
        updateTiming(i, 7);
        break;
      case 4:
        updateArg(i, 0, "Group Size", 0, 9);
        updateArg(i, 1, "Skip Between", 0, 9);
        updateArg(i, 2, "Repeat Runner", 1, 100);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Strobe");
        updateTiming(i, 1, "Blank");
        updateTiming(i, 2, "Runner Strobe");
        updateTiming(i, 3, "Runner Blank");
        updateTiming(i, 4, "Separating Blank");
        updateTiming(i, 5);
        updateTiming(i, 6);
        updateTiming(i, 7);
        break;
      case 5:
        updateArg(i, 0, "Use Steps", 1, 7);
        updateArg(i, 1, "Random Steps", 0, 1);
        updateArg(i, 2, "Random Colors", 0, 1);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Blank");
        updateTiming(i, 1, "Step 1");
        updateTiming(i, 2, "Step 2");
        updateTiming(i, 3, "Step 3");
        updateTiming(i, 4, "Step 4");
        updateTiming(i, 5, "Step 5");
        updateTiming(i, 6, "Step 6");
        updateTiming(i, 7, "Step 7");
        break;
      case 6:
        updateArg(i, 0, "Random Colors", 0, 1);
        updateArg(i, 1, "Time Multiplier", 1, 10);
        updateArg(i, 2);
        updateArg(i, 3);
        updateArg(i, 4);
        updateTiming(i, 0, "Strobe Low");
        updateTiming(i, 1, "Strobe High");
        updateTiming(i, 2, "Blank Low");
        updateTiming(i, 3, "Blank High");
        updateTiming(i, 4);
        updateTiming(i, 5);
        updateTiming(i, 6);
        updateTiming(i, 7);
        break;
    }
  }

  void resetArgsAndTimings(int i) {
    switch (pattern[i]) {
      case 0:
        setArgs((5 * i) + 0, 0);
        setArgs((5 * i) + 1, 0);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 5);
        setTimings(i, 1, 8);
        setTimings(i, 2, 0);
        setTimings(i, 3, 0);
        setTimings(i, 4, 0);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 1:
        setArgs((5 * i) + 0, 1);
        setArgs((5 * i) + 1, 1);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 5);
        setTimings(i, 1, 1);
        setTimings(i, 2, 20);
        setTimings(i, 3, 0);
        setTimings(i, 4, 0);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 2:
        setArgs((5 * i) + 0, 0);
        setArgs((5 * i) + 1, 0);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 2);
        setTimings(i, 1, 0);
        setTimings(i, 2, 5);
        setTimings(i, 3, 50);
        setTimings(i, 4, 0);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 3:
        setArgs((5 * i) + 0, 2);
        setArgs((5 * i) + 1, 2);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 5);
        setTimings(i, 1, 8);
        setTimings(i, 2, 1);
        setTimings(i, 3, 12);
        setTimings(i, 4, 5);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 4:
        setArgs((5 * i) + 0, 0);
        setArgs((5 * i) + 1, 0);
        setArgs((5 * i) + 2, 5);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 5);
        setTimings(i, 1, 0);
        setTimings(i, 2, 1);
        setTimings(i, 3, 12);
        setTimings(i, 4, 12);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 5:
        setArgs((5 * i) + 0, 5);
        setArgs((5 * i) + 1, 0);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 10);
        setTimings(i, 1, 2);
        setTimings(i, 2, 4);
        setTimings(i, 3, 6);
        setTimings(i, 4, 8);
        setTimings(i, 5, 10);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
      case 6:
        setArgs((5 * i) + 0, 1);
        setArgs((5 * i) + 1, 4);
        setArgs((5 * i) + 2, 0);
        setArgs((5 * i) + 3, 0);
        setArgs((5 * i) + 4, 0);
        setTimings(i, 0, 1);
        setTimings(i, 1, 5);
        setTimings(i, 2, 5);
        setTimings(i, 3, 5);
        setTimings(i, 4, 0);
        setTimings(i, 5, 0);
        setTimings(i, 6, 0);
        setTimings(i, 7, 0);
        break;
    }
  }

  void setTriggerMode(int val) {
    if (oob(val, 0, 5)) { return; };

    triggerMode = val;
    if (use_gui) {
      dlTriggerMode.setBroadcast(false).setValue(triggerMode).setBroadcast(true);
      dlTriggerMode.setCaptionLabel(dlTriggerMode.getItem(triggerMode).get("text").toString());
      trTriggerThresh.setTriggerMode(triggerMode);
    }
  }

  void sendTriggerMode() {
    sendCommand(SER_WRITE, _TRIGGER_MODE, triggerMode);
  }

  void setTriggerThresh(float[] val) {
    if (oob(val[0], 0, val[1]) || oob(val[1], val[0], 32)) { return; }

    triggerThresh[0] = (int)val[0];
    triggerThresh[1] = (int)val[1];

    if (use_gui) {
      trTriggerThresh.setBroadcast(false).setArrayValue(triggerThresh).setBroadcast(true);
    }
  }

  void setTriggerThresh(int i, int val) {
    if (oob(i, 0, 1) || oob(val, 0, 32)) { return; }
    triggerThresh[i] = val;

    if (use_gui) {
      trTriggerThresh.setBroadcast(false).setArrayValue(triggerThresh).setBroadcast(true);
    }
  }

  void sendTriggerThresh() {
    sendCommand(SER_WRITE, _TRIGGER_THRESH + 0, triggerThresh[1]);
    sendCommand(SER_WRITE, _TRIGGER_THRESH + 1, triggerThresh[0]);
  }


  // Pattern
  void setPattern(int i, int val) {
    if (oob(i, 0, 1) || oob(val, 0, 6)) { return; }
    int old = pattern[i];
    pattern[i] = val;
    if (use_gui) {
      dlPattern[i].setBroadcast(false).setValue(pattern[i]).setBroadcast(true);
      dlPattern[i].setCaptionLabel(dlPattern[i].getItem(val).get("text").toString());
      resetPatternGui(i);
      if (old != pattern[i]) {
        resetArgsAndTimings(i);
        mode.sendMode();
      }
    }
  }

  void sendPattern(int i) {
    sendCommand(SER_WRITE, _PATTERN + i, pattern[i]);
  }

  // Args
  void setArgs(int i, int val) {
    if (oob(i, 0, 9)) { return; }

    args[i / 5][i % 5] = val;
    if (use_gui) {
      slArgs[i / 5][i % 5].setBroadcast(false).setValue(args[i / 5][i % 5]).setBroadcast(true);
    }
  }

  void sendArgs(int i) {
    sendCommand(SER_WRITE, _ARGS + i, args[i / 5][i % 5]);
  }

  // Timings
  void setTimings(int i, int val) {
    setTimings(i / 8, i % 8, val);
  }

  void setTimings(int x, int y, int val) {
    if (oob(x, 0, 1) || oob(y, 0, 7)) { return; }

    timings[x][y] = val;
    if (use_gui) {
      slTimings[x][y].setBroadcast(false).setValue(timings[x][y]).setBroadcast(true);
    }
  }

  void sendTimings(int i) {
    sendTimings(i / 8, i % 8);
  }

  void sendTimings(int x, int y) {
    sendCommand(SER_WRITE, _TIMINGS + (8 * x) + y, timings[x][y]);
  }

  // Num colors
  void setNumColors(int i, int val) {
    if (oob(i, 0, 1) || oob(val, 1, 9)) { return; }

    numColors[i] = val;
    if (color_set == i && color_slot >= numColors[i]) {
      deselectColor();
    }
    if (use_gui) {
      for (int j = 0; j < 9; j++) {
        if (j < numColors[i]) bColors[i][j].show();
        else                  bColors[i][j].hide();
      }
      slNumColors[i].setBroadcast(false).setValue(numColors[i]).setBroadcast(true);
    }
  }

  void sendNumColors(int i) {
    sendCommand(SER_WRITE, _NUMCOLORS + i, numColors[i]);
  }

  // Colors
  void setColor(int _set, int _color, int _channel, int val) {
    if (oob(_set, 0, 1) || oob(_color, 0, 8) || oob(_channel, 0, 2)) { return; }

    colors[_set][_color][_channel] = val;
    updateColor(_set, _color);
  }

  void setColor(int _set, int _color, int[] val) {
    if (oob(_set, 0, 1) || oob(_color, 0, 8)) { return; }

    colors[_set][_color][0] = val[0];
    colors[_set][_color][1] = val[1];
    colors[_set][_color][2] = val[2];
    updateColor(_set, _color);
  }

  void setColor(int i, int val) {
    setColor(i / 27, (i % 27) / 3, i % 3, val);
  }

  void sendColor(int _set, int _color, int _channel) {
    if (oob(_set, 0, 1) || oob(_color, 0, 8) || oob(_channel, 0, 2)) { return; }
    sendCommand(SER_WRITE, _COLORS + (_set * 27) + (_color * 3) + _channel, colors[_set][_color][_channel]);
  }

  void sendColor(int _set, int _color) {
    if (oob(_set, 0, 1) || oob(_color, 0, 8)) { return; }
    sendColor(_set, _color, 0);
    sendColor(_set, _color, 1);
    sendColor(_set, _color, 2);
  }

  //********************************************************************************
  //** JSON
  //********************************************************************************
  void fromJSON(JSONObject j) {
    triggerMode = j.getInt("trigger_mode");
    setTriggerThresh(j.getJSONArray("trigger_thresh"));
    setPattern(j.getJSONArray("pattern"));
    setArgs(j.getJSONArray("args"));
    setTimings(j.getJSONArray("timings"));
    setNumColors(j.getJSONArray("num_colors"));
    setColors(j.getJSONArray("colors"));
  }

  JSONObject getJSON() {
    JSONObject jo = new JSONObject();
    jo.setInt("trigger_mode", triggerMode);
    jo.setJSONArray("trigger_thresh", getTriggerThresh());
    jo.setJSONArray("pattern", getPattern());
    jo.setJSONArray("args", getArgs());
    jo.setJSONArray("timings", getTimings());
    jo.setJSONArray("num_colors", getNumColors());
    jo.setJSONArray("colors", getColors());
    return jo;
  }

  JSONArray getTriggerThresh() {
    JSONArray ja = new JSONArray();
    ja.setInt(0, triggerThresh[0]);
    ja.setInt(1, triggerThresh[1]);
    return ja;
  }

  JSONArray getPattern() {
    JSONArray ja = new JSONArray();
    ja.setInt(0, pattern[0]);
    ja.setInt(1, pattern[1]);
    return ja;
  }

  JSONArray getArgs() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 5; j++) {
        ja1.setInt(j, args[i][j]);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  JSONArray getTimings() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 8; j++) {
        ja1.setInt(j, timings[i][j]);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  JSONArray getNumColors() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      ja.setInt(i, numColors[i]);
    }
    return ja;
  }

  JSONArray getColors() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 9; j++) {
        JSONArray ja2 = new JSONArray();
        for (int k = 0; k < 3; k++) {
          ja2.setInt(k, colors[i][j][k]);
        }
        ja1.setJSONArray(j, ja2);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  void setTriggerThresh(JSONArray ja) {
    setTriggerThresh(0, ja.getInt(0));
    setTriggerThresh(1, ja.getInt(1));
  }

  void setPattern(JSONArray ja) {
    setPattern(0, ja.getInt(0));
    setPattern(1, ja.getInt(1));
  }

  void setArgs(JSONArray ja) {
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = ja.getJSONArray(i);
      for (int j = 0; j < 5; j++) {
        try {
          setArgs(i, ja1.getInt(i));
        } catch (Exception ex) {
          setArgs((i * 5) + j, 0);
        }
      }
    }
  }

  void setTimings(JSONArray ja) {
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = ja.getJSONArray(i);
      for (int j = 0; j < 8; j++) {
        try {
          setTimings(i, j, ja1.getInt(j));
        } catch (Exception ex) {
          setTimings(i, j, 0);
        }
      }
    }
  }

  void setNumColors(JSONArray ja) {
    for (int i = 0; i < 2; i++) {
      setNumColors(i, ja.getInt(i));
    }
  }

  void setColors(JSONArray ja) {
    for (int _set = 0; _set < 2; _set++) {
      JSONArray ja1 = ja.getJSONArray(_set);
      for (int _color = 0; _color < 9; _color++) {
        JSONArray ja2 = ja1.getJSONArray(_color);
        for (int _channel = 0; _channel < 3; _channel++) {
          setColor(_set, _color, _channel, ja2.getInt(_channel));
        }
      }
    }
  }
}
