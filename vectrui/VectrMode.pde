class VectrMode {
  boolean use_gui = true;

  static final int _PATTERN = 1;
  static final int _ARGS = 2;
  static final int _PATTERNTHRESH = 7;
  static final int _TIMINGS = 11;
  static final int _COLORTHRESH = 35;
  static final int _NUMCOLORS = 39;
  static final int _COLORS = 42;
  static final int _PADDING = 123;

  int       pattern;
  int[]     args = new int[5];
  int[]     patternThresh = new int[4];
  int[][]   timings = new int[3][8];
  int[]     colorThresh = new int[4];
  int[]     numColors = new int[3];
  int[][][] colors = new int[3][9][3];

  //********************************************************************************
  // GUI Elements
  //********************************************************************************
  Group gVectr;
  Group gPatternArgs;
  Group gTimings;
  Group gColors;

  Textlabel tlPatternLabel;
  ThreshRange trPatternThresh;
  ThreshRange trColorThresh;
  DropdownList dlPattern;
  Textlabel[] tlArgLabels = new Textlabel[5];
  Slider[] slArgs = new Slider[5];
  Textlabel[] tlTimingLabels = new Textlabel[8];
  Slider[][] slTimings = new Slider[3][8];
  Slider[] slNumColors = new Slider[3];
  Button[][] bColors = new Button[3][9];
  /* Button[][] bColorBgs = new Button[3][9]; */

  Button bSelectedColor;
  int color_set = -1;
  int color_slot = -1;

  VectrMode() {
    use_gui = false;
  }

  VectrMode(Group g) {
    gVectr = g;

    gPatternArgs = cp5.addGroup("VectrPatternArgs")
      .setGroup(gVectr)
      .setPosition(0, 0)
      .setSize(800, 50)
      .hideBar()
      .hideArrow();

    trPatternThresh = new ThreshRange(cp5, "VectrPatternThresh")
      .setGroup(gVectr)
      .setId(ID_PATTERN_TRESH)
      .setPosition(4, 80)
      .setLabel("Pattern Thresholds");

    gTimings = cp5.addGroup("VectrTimings")
      .setGroup(gVectr)
      .setPosition(0, 145)
      .hideBar()
      .hideArrow();

    trColorThresh = new ThreshRange(cp5, "VectrColorThresh")
      .setGroup(gVectr)
      .setId(ID_COLOR_TRESH)
      .setPosition(4, 415)
      .setLabel("Color Thresholds");

    gColors = cp5.addGroup("VectrColorGroup")
      .setGroup(gVectr)
      .setPosition(10, 480)
      .hideBar()
      .hideArrow();

    tlPatternLabel = cp5.addTextlabel("VectrPatternLabel")
        .setGroup(gPatternArgs)
        .setValue("Pattern")
        .setPosition(0, 0)
        .setSize(80, 20)
        .setColorValue(color(240));

    dlPattern = cp5.addDropdownList("VectrPattern")
      .setGroup(gPatternArgs)
      .setId(ID_PATTERN)
      .setPosition(0, 20)
      .setSize(80, 160)
      .setItems(PATTERNS);
    style(dlPattern);

    for (int i = 0; i < 5; i++) {
      tlArgLabels[i] = cp5.addTextlabel("VectrArgLabels" + i)
        .setGroup(gPatternArgs)
        .setValue("Arg " + (i + 1))
        .setPosition(150 + (137 * i), 0)
        .setSize(100, 20)
        .setColorValue(color(240));

      slArgs[i] = cp5.addSlider("VectrArgs" + i)
        .setGroup(gPatternArgs)
        .setId(ID_ARG + i)
        .setLabel("")
        .setPosition(150 + (137 * i), 20);
      style(slArgs[i], 101, 0, 10);
    }

    for (int j = 0; j < 8; j++) {
      tlTimingLabels[j] = cp5.addTextlabel("VectrTimingLabels" + j)
        .setGroup(gTimings)
        .setValue("Timing" + (j + 1))
        .setPosition(0, 30 * j)
        .setSize(200, 20)
        .setColorValue(color(240));

      for (int i = 0; i < 3; i++) {
        slTimings[i][j] = cp5.addSlider("VectrTimings" + i + "." + j)
          .setGroup(gTimings)
          .setId(ID_TIMING + (8 * i) + j)
          .setLabel("")
          .setPosition(150 + (225 * i), 30 * j);
        style(slTimings[i][j], 201, 0, 200);
      }
    }

    for (int i = 0; i < 3; i++) {
      slNumColors[i] = cp5.addSlider("VectrNumColors" + i)
        .setGroup(gColors)
        .setId(ID_NUMCOLORS + i)
        .setLabel("")
        .setPosition(0, 10 + (40 * i));
      style(slNumColors[i], 90, 1, 9);

      for (int j = 0; j < 9; j++) {
        /* bColorBgs[i][j] = cp5.addButton("VectrColorBgs" + i + "." + j) */
        /*   .setGroup(gColors) */
        /*   .setLabel("") */
        /*   .setSize(34, 34) */
        /*   .setPosition(113 + (40 * j), 3 + (40 * i)) */
        /*   .setColorBackground(color(64)) */
        /*   .setColorForeground(color(192)) */
        /*   .setColorActive(color(240)); */

        bColors[i][j] = cp5.addButton("VectrColors" + i + "." + j)
          .setGroup(gColors)
          .setId(ID_COLORS + (i * 9) + j)
          .setLabel("")
          .setSize(32, 32)
          .setPosition(114 + (40 * j), 4 + (40 * i));
      }
    }

    bSelectedColor = cp5.addButton("VectrSelectedColor")
      .setGroup(gColors)
      .setSize(40, 40)
      .setColorBackground(color(255))
      .setColorForeground(color(255))
      .setColorActive(color(255))
      .setLabel("")
      .hide();

    gPatternArgs.bringToFront();
  }

  int geta(int addr) {
    if (addr < _ARGS) {                 return pattern;
    } else if (addr < _PATTERNTHRESH) { return args[addr - _ARGS];
    } else if (addr < _TIMINGS) {       return patternThresh[addr - _PATTERNTHRESH];
    } else if (addr < _COLORTHRESH) {   return timings[(addr - _TIMINGS) / 8][(addr - _TIMINGS) % 8];
    } else if (addr < _NUMCOLORS) {     return colorThresh[addr - _COLORTHRESH];
    } else if (addr < _COLORS) {        return numColors[addr - _NUMCOLORS];
    } else if (addr < _PADDING) {       return colors[(addr - _COLORS) / 27][((addr - _COLORS) % 27) / 3][(addr - _COLORS) % 3];
    }
    return 0;
  }

  void seta(int addr, int val) {
    if (addr < _ARGS) {                 setPattern(val);
    } else if (addr < _PATTERNTHRESH) { setArgs(addr - _ARGS, val);
    } else if (addr < _TIMINGS) {       setPatternThresh(addr - _PATTERNTHRESH, val);
    } else if (addr < _COLORTHRESH) {   setTimings(addr - _TIMINGS, val);
    } else if (addr < _NUMCOLORS) {     setColorThresh(addr - _COLORTHRESH, val);
    } else if (addr < _COLORS) {        setNumColors(addr - _NUMCOLORS, val);;
    } else if (addr < _PADDING) {       setColor(addr - _COLORS, val);
    }
  }

  void deselectColor() {
    bSelectedColor.hide();
    color_set = color_slot = -1;
    viewMode(0);
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

  void updateArg(int i, String label, int _min, int _max) {
    tlArgLabels[i].setValue(label).show();
    slArgs[i].setBroadcast(false)
      .setRange(_min, _max)
      .setNumberOfTickMarks(_max - _min + 1)
      .showTickMarks(false)
      .setBroadcast(true)
      .show();
  }

  void updateArg(int i) {
    tlArgLabels[i].hide();
    slArgs[i].hide();
  }

  void updateTiming(int i, String label) {
    tlTimingLabels[i].setValue(label).show();
    slTimings[0][i].show();
    slTimings[1][i].show();
    slTimings[2][i].show();
  }

  void updateTiming(int i) {
    tlTimingLabels[i].hide();
    slTimings[0][i].hide();
    slTimings[1][i].hide();
    slTimings[2][i].hide();
  }

  void resetPatternGui() {
    switch (pattern) {
      case 0:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1, "Skip After", 0, 9);
        updateArg(2, "Repeat Group", 1, 100);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Tail Blank");
        updateTiming(3);
        updateTiming(4);
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
      case 1:
        updateArg(0, "Repeat Strobe", 1, 100);
        updateArg(1, "Repeat Tracer", 1, 100);
        updateArg(2);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Tracer Strobe");
        updateTiming(3, "Tracer Blank");
        updateTiming(4, "Split Blank");
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
      case 2:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1);
        updateArg(2);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Center Strobe");
        updateTiming(3, "Tail Blank");
        updateTiming(4);
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
      case 3:
        updateArg(0, "Repeat First", 1, 100);
        updateArg(1, "Repeat Second", 1, 100);
        updateArg(2, "Repeat Third", 1, 100);
        updateArg(3, "Skip Colors", 0, 8);
        updateArg(4, "Use Third", 0, 1);
        updateTiming(0, "First Strobe");
        updateTiming(1, "First Blank");
        updateTiming(2, "Second Strobe");
        updateTiming(3, "Second Blank");
        updateTiming(4, "Third Strobe");
        updateTiming(5, "Third Blank");
        updateTiming(6, "Separating Blank");
        updateTiming(7);
        break;
      case 4:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1, "Skip Between", 0, 9);
        updateArg(2, "Repeat Runner", 1, 100);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Runner Strobe");
        updateTiming(3, "Runner Blank");
        updateTiming(4, "Separating Blank");
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
      case 5:
        updateArg(0, "Use Steps", 1, 7);
        updateArg(1, "Random Steps", 0, 1);
        updateArg(2, "Random Colors", 0, 1);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Blank");
        updateTiming(1, "Step 1");
        updateTiming(2, "Step 2");
        updateTiming(3, "Step 3");
        updateTiming(4, "Step 4");
        updateTiming(5, "Step 5");
        updateTiming(6, "Step 6");
        updateTiming(7, "Step 7");
        break;
      case 6:
        updateArg(0, "Random Colors", 0, 1);
        updateArg(1, "Time Multiplier", 1, 10);
        updateArg(2);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe Low");
        updateTiming(1, "Strobe High");
        updateTiming(2, "Blank Low");
        updateTiming(3, "Blank High");
        updateTiming(4);
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
      case 7:
        updateArg(0, "Steps", 0, 100);
        updateArg(1, "Strobe/Blank", 0, 1);
        updateArg(2, "Up/Down/Both", 0, 2);
        updateArg(3);
        updateArg(4);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Flux");
        updateTiming(3);
        updateTiming(4);
        updateTiming(5);
        updateTiming(6);
        updateTiming(7);
        break;
    }
  }

  void resetArgsAndTimings() {
    switch (pattern) {
      case 0:
        setArgs(0, 0);
        setArgs(1, 0);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5);
          setTimings(i, 1, 8);
          setTimings(i, 2, 0);
          setTimings(i, 3, 0);
          setTimings(i, 4, 0);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 1:
        setArgs(0, 1);
        setArgs(1, 1);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5);
          setTimings(i, 1, 1);
          setTimings(i, 2, 20);
          setTimings(i, 3, 0);
          setTimings(i, 4, 0);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 2:
        setArgs(0, 0);
        setArgs(1, 0);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 2);
          setTimings(i, 1, 0);
          setTimings(i, 2, 5);
          setTimings(i, 3, 50);
          setTimings(i, 4, 0);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 3:
        setArgs(0, 2);
        setArgs(1, 2);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5);
          setTimings(i, 1, 8);
          setTimings(i, 2, 1);
          setTimings(i, 3, 12);
          setTimings(i, 4, 5);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 4:
        setArgs(0, 0);
        setArgs(1, 0);
        setArgs(2, 5);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5);
          setTimings(i, 1, 0);
          setTimings(i, 2, 1);
          setTimings(i, 3, 12);
          setTimings(i, 4, 12);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 5:
        setArgs(0, 5);
        setArgs(1, 0);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 10);
          setTimings(i, 1, 2);
          setTimings(i, 2, 4);
          setTimings(i, 3, 6);
          setTimings(i, 4, 8);
          setTimings(i, 5, 10);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 6:
        setArgs(0, 1);
        setArgs(1, 4);
        setArgs(2, 0);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 1);
          setTimings(i, 1, 5);
          setTimings(i, 2, 5);
          setTimings(i, 3, 5);
          setTimings(i, 4, 0);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
      case 7:
        setArgs(0, 10);
        setArgs(1, 1);
        setArgs(2, 2);
        setArgs(3, 0);
        setArgs(4, 0);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 1);
          setTimings(i, 1, 0);
          setTimings(i, 2, 1);
          setTimings(i, 3, 0);
          setTimings(i, 4, 0);
          setTimings(i, 5, 0);
          setTimings(i, 6, 0);
          setTimings(i, 7, 0);
        }
        break;
    }
  }

  // Pattern
  void setPattern(int val) {
    if (oob(val, 0, 6)) { return; }
    int old = pattern;
    pattern = val;
    if (use_gui) {
      dlPattern.setBroadcast(false).setValue(pattern).setBroadcast(true);
      dlPattern.setCaptionLabel(dlPattern.getItem(val).get("text").toString());
      resetPatternGui();
      if (old != pattern) {
        resetArgsAndTimings();
        mode.sendMode();
      }
    }
  }

  void sendPattern() {
    sendCommand(SER_WRITE, _PATTERN, pattern);
  }

  // Args
  void setArgs(int i, int val) {
    if (oob(i, 0, 4)) { return; }

    args[i] = val;
    if (use_gui) {
      slArgs[i].setBroadcast(false).setValue(args[i]).setBroadcast(true);
    }
  }

  void sendArgs(int i) {
    sendCommand(SER_WRITE, _ARGS + i, args[i]);
  }

  // Pattern Thresh
  void setPatternThresh(float[] val) {
    if (oob(val[0], 0, val[1]) || oob(val[1], val[0], val[2]) || oob(val[2], val[1], val[3]) || oob(val[3], val[2], 32)) { return; }

    patternThresh[0] = (int)val[0];
    patternThresh[1] = (int)val[1];
    patternThresh[2] = (int)val[2];
    patternThresh[3] = (int)val[3];

    if (use_gui) {
      trPatternThresh.setBroadcast(false).setArrayValue(patternThresh).setBroadcast(true);
    }
  }

  void setPatternThresh(int i, int val) {
    if (oob(i, 0, 3) || oob(val, 0, 32)) { return; }

    patternThresh[i] = val;
    if (use_gui) {
      trPatternThresh.setBroadcast(false).setArrayValue(patternThresh).setBroadcast(true);
    }
  }

  void sendPatternThresh() {
    sendCommand(SER_WRITE, _PATTERNTHRESH + 0, patternThresh[0]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 1, patternThresh[1]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 2, patternThresh[2]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 3, patternThresh[3]);
  }

  // Timings
  void setTimings(int i, int val) {
    setTimings(i / 8, i % 8, val);
  }

  void setTimings(int x, int y, int val) {
    if (oob(x, 0, 2) || oob(y, 0, 7)) { return; }

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

  // Color Thresh
  void setColorThresh(float[] val) {
    if (oob(val[0], 0, val[1]) || oob(val[1], val[0], val[2]) || oob(val[2], val[1], val[3]) || oob(val[3], val[2], 32)) { return; }

    colorThresh[0] = (int)val[0];
    colorThresh[1] = (int)val[1];
    colorThresh[2] = (int)val[2];
    colorThresh[3] = (int)val[3];
    if (use_gui) {
      trColorThresh.setBroadcast(false).setArrayValue(val).setBroadcast(true);
    }
  }

  void setColorThresh(int i, int val) {
    if (oob(i, 0, 3) || oob(val, 0, 32)) { return; }

    colorThresh[i] = val;
    if (use_gui) {
      trColorThresh.setBroadcast(false).setArrayValue(colorThresh).setBroadcast(true);
    }
  }

  void sendColorThresh() {
    sendCommand(SER_WRITE, _COLORTHRESH + 0, colorThresh[0]);
    sendCommand(SER_WRITE, _COLORTHRESH + 1, colorThresh[1]);
    sendCommand(SER_WRITE, _COLORTHRESH + 2, colorThresh[2]);
    sendCommand(SER_WRITE, _COLORTHRESH + 3, colorThresh[3]);
  }

  // Num colors
  void setNumColors(int i, int val) {
    if (oob(i, 0, 2) || oob(val, 1, 9)) { return; }

    numColors[i] = val;
    if (color_set == i && color_slot >= numColors[i]) {
      deselectColor();
    }
    if (use_gui) {
      for (int j = 0; j < 9; j++) {
        if (j < numColors[i]) { bColors[i][j].show(); } //bColorBgs[i][j].show(); }
        else                  { bColors[i][j].hide(); } //bColorBgs[i][j].hide(); }
      }
      slNumColors[i].setBroadcast(false).setValue(numColors[i]).setBroadcast(true);
    }
  }

  void sendNumColors(int i) {
    sendCommand(SER_WRITE, _NUMCOLORS + i, numColors[i]);
  }

  // Colors
  void setColor(int _set, int _color, int _channel, int val) {
    if (oob(_set, 0, 2) || oob(_color, 0, 8) || oob(_channel, 0, 2)) { return; }

    colors[_set][_color][_channel] = val;
    updateColor(_set, _color);
  }

  void setColor(int _set, int _color, int[] val) {
    if (oob(_set, 0, 2) || oob(_color, 0, 8)) { return; }

    colors[_set][_color][0] = val[0];
    colors[_set][_color][1] = val[1];
    colors[_set][_color][2] = val[2];
    updateColor(_set, _color);
  }

  void setColor(int i, int val) {
    setColor(i / 27, (i % 27) / 3, i % 3, val);
  }

  void sendColor(int _set, int _color, int _channel) {
    if (oob(_set, 0, 2) || oob(_color, 0, 8) || oob(_channel, 0, 2)) { return; }
    sendCommand(SER_WRITE, _COLORS + (_set * 27) + (_color * 3) + _channel, colors[_set][_color][_channel]);
  }

  void sendColor(int _set, int _color) {
    if (oob(_set, 0, 2) || oob(_color, 0, 8)) { return; }
    sendColor(_set, _color, 0);
    sendColor(_set, _color, 1);
    sendColor(_set, _color, 2);
  }

  //********************************************************************************
  //** JSON
  //********************************************************************************
  void fromJSON(JSONObject j) {
    setPattern(j.getInt("pattern"));
    setArgs(j.getJSONArray("args"));
    setPatternThresh(j.getJSONArray("pattern_thresh"));
    setTimings(j.getJSONArray("timings"));
    setColorThresh(j.getJSONArray("color_thresh"));
    setNumColors(j.getJSONArray("num_colors"));
    setColors(j.getJSONArray("colors"));
  }

  JSONObject getJSON() {
    JSONObject jo = new JSONObject();
    jo.setInt("pattern", pattern);
    jo.setJSONArray("args", getArgs());
    jo.setJSONArray("pattern_thresh", getPatternThresh());
    jo.setJSONArray("timings", getTimings());
    jo.setJSONArray("color_thresh", getColorThresh());
    jo.setJSONArray("num_colors", getNumColors());
    jo.setJSONArray("colors", getColors());
    return jo;
  }

  JSONArray getArgs() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 5; i++) {
      ja.setInt(i, args[i]);
    }
    return ja;
  }

  JSONArray getPatternThresh() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 4; i++) {
      ja.setInt(i, patternThresh[i]);
    }
    return ja;
  }

  JSONArray getTimings() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 3; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 8; j++) {
        ja1.setInt(j, timings[i][j]);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  JSONArray getColorThresh() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 4; i++) {
      ja.setInt(i, colorThresh[i]);
    }
    return ja;
  }

  JSONArray getNumColors() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 3; i++) {
      ja.setInt(i, numColors[i]);
    }
    return ja;
  }

  JSONArray getColors() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 3; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 9; j++) {
        JSONArray ja2 = new JSONArray();
        for (int k = 0; k < 3; k++) {
          if (j < numColors[i]) {
            ja2.setInt(k, colors[i][j][k]);
          } else {
            ja2.setInt(k, 0);
          }
        }
        ja1.setJSONArray(j, ja2);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }


  void setArgs(JSONArray ja) {
    for (int i = 0; i < 5; i++) {
      try {
        setArgs(i, ja.getInt(i));
      } catch (Exception ex) {
        setArgs(i, 0);
      }
    }
  }

  void setPatternThresh(JSONArray ja) {
    try {
      for (int i = 0; i < 4; i++) {
        setPatternThresh(i, ja.getInt(i));
      }
    } catch (Exception ex) {
      for (int i = 0; i < 2; i++) {
        JSONArray ja1 = ja.getJSONArray(i);
        for (int j = 0; j < 2; j++) {
          setPatternThresh((i * 2) + j, ja1.getInt(j));
        }
      }
    }
  }

  void setTimings(JSONArray ja) {
    for (int i = 0; i < 3; i++) {
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

  void setColorThresh(JSONArray ja) {
    try {
      for (int i = 0; i < 4; i++) {
        setColorThresh(i, ja.getInt(i));
      }
    } catch (Exception ex) {
      for (int i = 0; i < 2; i++) {
        JSONArray ja1 = ja.getJSONArray(i);
        for (int j = 0; j < 2; j++) {
          setPatternThresh((i * 2) + j, ja1.getInt(j));
        }
      }
    }
  }

  void setNumColors(JSONArray ja) {
    for (int i = 0; i < 3; i++) {
      setNumColors(i, ja.getInt(i));
    }
  }

  void setColors(JSONArray ja) {
    for (int _set = 0; _set < 3; _set++) {
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
