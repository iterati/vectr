class Mode {
  boolean use_gui = true;

  static final int _TYPE = 0;
  static final int _MODESIZE = 128;

  static final int _PATTERN = 1;
  static final int _ARGS = 2;
  static final int _PATTERNTHRESH = 5;
  static final int _TIMINGS = 9;
  static final int _COLORTHRESH = 27;
  static final int _NUMCOLORS = 31;
  static final int _COLORS = 34;
  static final int _PADDING = 115;

  /* static final int _PACCELMODE = 1; */
  /* static final int _PACCELTRIG = 2; */
  /* static final int _PACCELDROP = 4; */
  /* static final int _PPATTERNS = 6; */
  /* static final int _PARGS = 9; */
  /* static final int _PTIMINGS = 18; */
  /* static final int _PNUMCOLORS = 36; */
  /* static final int _PCOLORS = 39; */

  int       _type;
  int       pattern;
  int[]     args = new int[3];
  int[][]   patternThresh = new int[2][2];
  int[][]   timings = new int[3][6];
  int[][]   colorThresh = new int[2][2];
  int[]     numColors = new int[3];
  int[][][] colors = new int[3][9][3];

  /* int       pAccelMode; */
  /* int       pAccelTrig; */
  /* int       pAccelDrop; */
  /* int[]     pPatterns = new int[2]; */
  /* int[][]   pArgs = new int[2][3]; */
  /* int[][]   pTimings = new int[2][6]; */
  /* int[]     pNumColors = new int[2]; */
  /* int[][][] pColors = new int[2][16][3]; */

  //********************************************************************************
  // GUI Elements
  //********************************************************************************
  Group gMode;
  Group gType0;
  /* Group gType1; */

  // Both
  DropdownList dlType;

  // Type0 - Vectr Mode
  Textlabel tlPatternLabel;
  ThreshRange trPatternThresh;
  ThreshRange trColorThresh;
  DropdownList dlPattern;
  Textlabel[] tlArgLabels = new Textlabel[3];
  Slider[] slArgs = new Slider[3];
  Textlabel[] tlTimingLabels = new Textlabel[6];
  Slider[][] slTimings = new Slider[3][6];
  Slider[] slNumColors = new Slider[3];
  Button[][] bColors = new Button[3][9];

  // ColorEdit
  Slider[] slColorValues = new Slider[3];
  Button bViewMode;
  Button bViewColor;

  // Type1 - Primer Mode
  /* DropdownList dlAccelMode; */
  /* PrimerRange prPrimerThresh; */
  /* DropdownList[] dlPPatterns = new DropdownList[2]; */
  /* Textlabel[][] tlPArgLabels = new Textlabel[2][3]; */
  /* Slider[][] slPArgs = new Slider[2][3]; */
  /* Textlabel[][] tlPTimingLabels = new Textlabel[2][6]; */
  /* Slider[][] slPTimings = new Slider[2][6]; */
  /* Slider[] slPNumColors = new Slider[2]; */
  /* Button[][] bPColors = new Button[2][15]; */

  Button bSelectedColor;
  int color_set = -1;
  int color_slot = -1;


  Mode() {
    _type = 0;
    use_gui = false;
  }

  Mode(Group g) {
    gMode = g;
    _type = 0;

    // Type0
    gType0 = cp5.addGroup("type0")
      .setGroup(gMode)
      .hideBar()
      .hideArrow();

    bSelectedColor = cp5.addButton("selectedColor")
      .setGroup(gType0)
      .setSize(40, 40)
      .setColorBackground(color(255))
      .setColorForeground(color(255))
      .setColorActive(color(255))
      .setLabel("")
      .hide();

    for (int i = 0; i < 3; i++) {
      slArgs[i] = cp5.addSlider("Args" + i)
        .setGroup(gType0)
        .setId(10200 + i)
        .setLabel("")
        .setPosition(130 + (275 * i), 20);
      style(slArgs[i], 250, 0, 10);

      tlArgLabels[i] = cp5.addTextlabel("ArgLabels" + i)
        .setGroup(gType0)
        .setValue("Arg " + (i + 1))
        .setPosition(130 + (i * 275), 0)
        .setSize(250, 20)
        .setColorValue(color(240));
    }

    trPatternThresh = new ThreshRange(cp5, "PatternThresh", 0)
      .setGroup(gType0)
      .setPosition(49, 80)
      .setBroadcast(false)
      .setLabel("Pattern Thresholds")
      .setBroadcast(true);
    style(trPatternThresh);

    for (int j = 0; j < 6; j++) {
      for (int i = 0; i < 3; i++) {
        slTimings[i][j] = cp5.addSlider("Timings" + i + "." + j)
          .setGroup(gType0)
          .setId(10100 + (6 * i) + j)
          .setLabel("")
          .setPosition(130 + (275 * i), 150 + (30 * j));
        style(slTimings[i][j], 250, 0, 200);
      }

      tlTimingLabels[j] = cp5.addTextlabel("TimingLabels" + j)
        .setGroup(gType0)
        .setValue("Timing" + (j + 1))
        .setPosition(0, 150 + (30 * j))
        .setSize(250, 20)
        .setColorValue(color(240));
    }

    trColorThresh = new ThreshRange(cp5, "ColorThresh", 0)
      .setGroup(gType0)
      .setPosition(49, 380)
      .setBroadcast(false)
      .setLabel("Color Thresholds")
      .setBroadcast(true);
    style(trColorThresh);

    for (int i = 0; i < 3; i++) {
      slNumColors[i] = cp5.addSlider("NumColors" + i)
        .setGroup(gType0)
        .setId(10300 + i)
        .setLabel("")
        .setPosition(20, 470 + (40 * i));
      style(slNumColors[i], 160, 1, 9);

      for (int j = 0; j < 9; j++) {
        bColors[i][j] = cp5.addButton("Colors" + i + "." + j)
          .setGroup(gType0)
          .setId(11000 + (i * 9) + j)
          .setLabel("")
          .setSize(32, 32)
          .setPosition(214 + (40 * j), 464 + (40 * i));
      }
    }

    tlPatternLabel = cp5.addTextlabel("PatternLabel")
        .setGroup(gType0)
        .setValue("Base Pattern")
        .setPosition(0, 0)
        .setSize(80, 20)
        .setColorValue(color(240));

    dlPattern = cp5.addDropdownList("Pattern")
      .setGroup(gType0)
      .setPosition(0, 20)
      .setSize(80, 160)
      .setItems(PATTERNS);
    style(dlPattern);

    /*
    // Type1
    gType1 = cp5.addGroup("type1")
      .setGroup(gMode)
      .hideBar()
      .hideArrow();

    for (int i = 2; i >= 0; i--) {
      for (int j = 0; j < 3; j++) {
        slPArgs[i][j] = cp5.addSlider("PArgs" + i + "." + j)
          .setGroup(gType1)
          .setId(10500 + (i * 3) + j)
          .setLabel("")
          .setPosition(130 + (275 * j), 20 + (40 * i));
        style(slPArgs[i][j], 250, 0, 10);

        tlPArgLabels[i][j] = cp5.addTextlabel("PArgLabels" + i + "." + j)
          .setGroup(gType1)
          .setValue("Args " + (i + 1) + " " + (j + 1))
          .setPosition(130 + (j * 275), (40 * i))
          .setSize(250, 20)
          .setColorValue(color(240));
      }

      dlPPatterns[i] = cp5.addDropdownList("PPattern" + i)
        .setGroup(gType1)
        .setId(10400 + i)
        .setPosition(0, 10 + (40 * i))
        .setSize(80, 160)
        .setItems(PATTERNS);
      style(dlPPatterns[i]);
    }

    dlAccelMode = cp5.addDropdownList("PAccelMode")
      .setGroup(gType1)
      .setPosition(460, -35)
      .setSize(80, 120)
      .setItems(ACCELMODES);
    style(dlAccelMode);

    dlType = cp5.addDropdownList("Type")
      .setGroup(gMode)
      .setPosition(360, -35)
      .setSize(80, 120)
      .setItems(MODETYPES);
    style(dlType);

    gType1.hide();
    */

    gColorEdit = cp5.addGroup("colorEdit")
      .setGroup(gMode)
      .setPosition(640, 460)
      .hideBar()
      .hideArrow()
      .hide();

    bViewMode = cp5.addButton("viewMode")
      .setCaptionLabel("View Mode")
      .setGroup(gColorEdit)
      .setPosition(15, 100);
    style(bViewMode, 100);
    bViewMode.setColorBackground(color(128))
      .setColorForeground(color(96));

    bViewColor = cp5.addButton("viewColor")
      .setCaptionLabel("View Color")
      .setGroup(gColorEdit)
      .setPosition(141, 100);
    style(bViewColor, 100);
    bViewColor.setColorBackground(color(48))
      .setColorForeground(color(96));

    slColorValues[0] = cp5.addSlider("colorValuesRed")
      .setGroup(gColorEdit)
      .setBroadcast(false)
      .setId(21000)
      .setLabel("")
      .setPosition(0, 0)
      .setTriggerEvent(ControlP5.RELEASE)
      .setSize(256, 20)
      .setColorBackground(color(64, 0, 0))
      .setColorForeground(color(128, 0, 0))
      .setColorActive(color(192, 0, 0))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);

    slColorValues[1] = cp5.addSlider("colorValuesGreen")
      .setGroup(gColorEdit)
      .setBroadcast(false)
      .setId(21001)
      .setLabel("")
      .setPosition(0, 30)
      .setTriggerEvent(ControlP5.RELEASE)
      .setSize(256, 20)
      .setColorBackground(color(0, 64, 0))
      .setColorForeground(color(0, 128, 0))
      .setColorActive(color(0, 192, 0))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);

    slColorValues[2] = cp5.addSlider("colorValuesBlue")
      .setGroup(gColorEdit)
      .setBroadcast(false)
      .setId(21002)
      .setLabel("")
      .setPosition(0, 60)
      .setTriggerEvent(ControlP5.RELEASE)
      .setSize(256, 20)
      .setColorBackground(color(0, 0, 64))
      .setColorForeground(color(0, 0, 128))
      .setColorActive(color(0, 0, 192))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);
  }

  int geta(int addr) {
    if (addr < 0) {
    } else if (addr == 0) {
    } else {
      if (_type == 0) {
        if (addr < _ARGS) {                 return pattern;
        } else if (addr < _PATTERNTHRESH) { return args[addr - _ARGS];
        } else if (addr < _TIMINGS) {       return patternThresh[(addr - _PATTERNTHRESH) / 2][(addr - _PATTERNTHRESH) % 2];
        } else if (addr < _COLORTHRESH) {   return timings[(addr - _TIMINGS) / 6][(addr - _TIMINGS) % 6];
        } else if (addr < _NUMCOLORS) {     return colorThresh[(addr - _COLORTHRESH) / 2][(addr - _COLORTHRESH) % 2];
        } else if (addr < _COLORS) {        return numColors[addr - _NUMCOLORS];
        } else if (addr < _PADDING) {       return colors[(addr - _COLORS) / 27][((addr - _COLORS) % 27) / 3][(addr - _COLORS) % 3];
        }
      /* } else { */
      /*   if (addr < _PACCELTRIG) {           return pAccelMode; */
      /*   } else if (addr < _PACCELDROP) {    return pAccelTrig[addr - _PACCELTRIG]; */
      /*   } else if (addr < _PPATTERNS) {     return pAccelDrop[addr - _PACCELDROP]; */
      /*   } else if (addr < _PARGS) {         return pPatterns[addr - _PPATTERNS]; */
      /*   } else if (addr < _PTIMINGS) {      return pArgs[(addr - _PARGS) / 3][(addr - _PARGS) % 3]; */
      /*   } else if (addr < _PNUMCOLORS) {    return timings[(addr - _PTIMINGS) / 6][(addr - _PTIMINGS) % 6]; */
      /*   } else if (addr < _PCOLORS) {       return numColors[addr - _PNUMCOLORS]; */
      /*   } else if (addr < _MODESIZE) {      return colors[(addr - _PCOLORS) / 9][((addr - _PCOLORS) % 9) / 3][(addr - _PCOLORS) % 3]; */
      /*   } */
      }
    }
    return 0;
  }

  void seta(int addr, int val) {
    // From light -> update GUI
    if (addr < 0) {
    } else if (addr == 0) {                 setType(val);
    } else {
      if (_type == 0) {
        if (addr < _ARGS) {                 setPattern(val);
        } else if (addr < _PATTERNTHRESH) { setArgs(addr - _ARGS, val);
        } else if (addr < _TIMINGS) {       setPatternThresh(addr - _PATTERNTHRESH, val);
        } else if (addr < _COLORTHRESH) {   setTimings(addr - _TIMINGS, val);
        } else if (addr < _NUMCOLORS) {     setColorThresh(addr - _COLORTHRESH, val);
        } else if (addr < _COLORS) {        setNumColors(addr - _NUMCOLORS, val);;
        } else if (addr < _PADDING) {       setColors(addr - _COLORS, val);
        }
      /* } else { */
      /*   if (addr < _PACCELTRIG) {           setPAccelMode(val); */
      /*   } else if (addr < _PACCELDROP) {    setPAccelTrig(addr - _PACCELTRIG, val); */
      /*   } else if (addr < _PPATTERNS) {     setPAccelDrop(addr - _PACCELDROP, val); */
      /*   } else if (addr < _PARGS) {         setPPatterns(addr - _PPATTERNS, val); */
      /*   } else if (addr < _PTIMINGS) {      setPArgs(addr - _PARGS, val); */
      /*   } else if (addr < _PNUMCOLORS) {    setPTimings(addr - _PTIMINGS, val); */
      /*   } else if (addr < _PCOLORS) {       setPNumColors(addr - _PNUMCOLORS, val); */
      /*   } else if (addr < _MODESIZE) {      setPColors(addr - _PCOLORS, val); */
      /*   } */
      }
    }
  }

  void deselectColor() {
    bSelectedColor.hide();
    gColorEdit.hide();
    color_set = color_slot = -1;
  }

  void selectColor(int idx) {
    selectColor(idx / 9, idx % 9);
  }

  void selectColor(int _set, int _color) {
    if (_set < 0 || _set >= 3 || _color < 0 || _color >= numColors[_set]) {
    } else {
      float[] pos = bColors[_set][_color].getPosition();
      color_set = _set;
      color_slot = _color;
      bSelectedColor.setPosition(pos[0] - 4, pos[1] - 4).show();
      bColors[_set][_color].bringToFront();
      gColorEdit.show();
      slColorValues[0].setBroadcast(false).setValue(colors[_set][_color][0]).setBroadcast(true);
      slColorValues[1].setBroadcast(false).setValue(colors[_set][_color][1]).setBroadcast(true);
      slColorValues[2].setBroadcast(false).setValue(colors[_set][_color][2]).setBroadcast(true);
    }
  }

  //********************************************************************************
  //** Getters and Setters
  //********************************************************************************
  int getType() {
    return _type;
  }

  int setType(int val) {
    if (val != 0) { return _type; }
    /* if (val != 0 || val != 1) { return _type; } */
    if (_type != val) {
      // TODO: Type changed - reset and update GUI
    }
    _type = val;
    if (use_gui) {
      /* dlType.setBroadcast(false).setValue(_type).setBroadcast(true); */
    }
    return val;
  }

  void sendType() {
    sendCommand(SER_WRITE, _TYPE, _type);
  }


  void updateArg(int idx, String label, int _min, int _max) {
    tlArgLabels[idx].setValue(label).show();
    slArgs[idx].setBroadcast(false)
      .setRange(_min, _max)
      .setNumberOfTickMarks(_max - _min + 1)
      .showTickMarks(false)
      .setBroadcast(true)
      .show();
  }

  void updateArg(int idx) {
    tlArgLabels[idx].hide();
    slArgs[idx].hide();
  }

  void updateTiming(int idx, String label) {
    tlTimingLabels[idx].setValue(label).show();
    slTimings[0][idx].show();
    slTimings[1][idx].show();
    slTimings[2][idx].show();
  }

  void updateTiming(int idx) {
    tlTimingLabels[idx].hide();
    slTimings[0][idx].hide();
    slTimings[1][idx].hide();
    slTimings[2][idx].hide();
  }

  void resetPatternGui() {
    switch (pattern) {
      case 0:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1, "Skip After", 0, 9);
        updateArg(2, "Repeat Group", 1, 100);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Tail Blank");
        updateTiming(3);
        updateTiming(4);
        updateTiming(5);
        break;
      case 1:
        updateArg(0, "Repeat Strobe", 1, 100);
        updateArg(1, "Repeat Tracer", 1, 100);
        updateArg(2);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Tracer Strobe");
        updateTiming(3, "Tracer Blank");
        updateTiming(4);
        updateTiming(5);
        break;
      case 2:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1);
        updateArg(2);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Center Strobe");
        updateTiming(3, "Tail Blank");
        updateTiming(4);
        updateTiming(5);
        break;
      case 3:
        updateArg(0, "Repeat First", 1, 100);
        updateArg(1, "Repeat Second", 1, 100);
        updateArg(2, "Skip Colors", 0, 8);
        updateTiming(0, "First Strobe");
        updateTiming(1, "First Blank");
        updateTiming(2, "Second Strobe");
        updateTiming(3, "Second Blank");
        updateTiming(4, "Separating Blank");
        updateTiming(5);
        break;
      case 4:
        updateArg(0, "Group Size", 0, 9);
        updateArg(1, "Skip Between", 0, 9);
        updateArg(2, "Repeat Runner", 1, 100);
        updateTiming(0, "Strobe");
        updateTiming(1, "Blank");
        updateTiming(2, "Runner Strobe");
        updateTiming(3, "Runner Blank");
        updateTiming(4, "Separating Blank");
        updateTiming(5);
        break;
      case 5:
        updateArg(0, "Use Steps", 1, 5);
        updateArg(1, "Randomize Steps", 0, 1);
        updateArg(2, "Randomize Colors", 0, 1);
        updateTiming(0, "Blank");
        updateTiming(1, "Step 1");
        updateTiming(2, "Step 2");
        updateTiming(3, "Step 3");
        updateTiming(4, "Step 4");
        updateTiming(5, "Step 5");
        break;
      case 6:
        updateArg(0, "Randomize Colors", 0, 1);
        updateArg(1, "Time Multiplier", 1, 10);
        updateArg(2);
        updateTiming(0, "Strobe Low");
        updateTiming(1, "Strobe High");
        updateTiming(2, "Blank Low");
        updateTiming(3, "Blank High");
        updateTiming(4);
        updateTiming(5);
        break;
    }
  }

  void resetArgsAndTimings() {
    switch (pattern) {
      case 0:
        setArgs(0, 0); sendArgs(0);
        setArgs(1, 0); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5); sendTimings(i, 0);
          setTimings(i, 1, 8); sendTimings(i, 1);
          setTimings(i, 2, 0); sendTimings(i, 2);
          setTimings(i, 3, 0); sendTimings(i, 3);
          setTimings(i, 4, 0); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
      case 1:
        setArgs(0, 1); sendArgs(0);
        setArgs(1, 1); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5); sendTimings(i, 0);
          setTimings(i, 1, 1); sendTimings(i, 1);
          setTimings(i, 2, 20); sendTimings(i, 2);
          setTimings(i, 3, 0); sendTimings(i, 3);
          setTimings(i, 4, 0); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
      case 2:
        setArgs(0, 0); sendArgs(0);
        setArgs(1, 0); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 2); sendTimings(i, 0);
          setTimings(i, 1, 0); sendTimings(i, 1);
          setTimings(i, 2, 5); sendTimings(i, 2);
          setTimings(i, 3, 50); sendTimings(i, 3);
          setTimings(i, 4, 0); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
      case 3:
        setArgs(0, 2); sendArgs(0);
        setArgs(1, 2); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5); sendTimings(i, 0);
          setTimings(i, 1, 8); sendTimings(i, 1);
          setTimings(i, 2, 1); sendTimings(i, 2);
          setTimings(i, 3, 12); sendTimings(i, 3);
          setTimings(i, 4, 5); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
      case 4:
        setArgs(0, 0); sendArgs(0);
        setArgs(1, 0); sendArgs(1);
        setArgs(2, 5); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 5); sendTimings(i, 0);
          setTimings(i, 1, 0); sendTimings(i, 1);
          setTimings(i, 2, 1); sendTimings(i, 2);
          setTimings(i, 3, 12); sendTimings(i, 3);
          setTimings(i, 4, 12); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
      case 5:
        setArgs(0, 5); sendArgs(0);
        setArgs(1, 0); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 10); sendTimings(i, 0);
          setTimings(i, 1, 2); sendTimings(i, 1);
          setTimings(i, 2, 4); sendTimings(i, 2);
          setTimings(i, 3, 6); sendTimings(i, 3);
          setTimings(i, 4, 8); sendTimings(i, 4);
          setTimings(i, 5, 10); sendTimings(i, 5);
        }
        break;
      case 6:
        setArgs(0, 1); sendArgs(0);
        setArgs(1, 4); sendArgs(1);
        setArgs(2, 0); sendArgs(2);
        for (int i = 0; i < 3; i++) {
          setTimings(i, 0, 1); sendTimings(i, 0);
          setTimings(i, 1, 5); sendTimings(i, 1);
          setTimings(i, 2, 5); sendTimings(i, 2);
          setTimings(i, 3, 5); sendTimings(i, 3);
          setTimings(i, 4, 0); sendTimings(i, 4);
          setTimings(i, 5, 0); sendTimings(i, 5);
        }
        break;
    }
  }


  int getPattern() {
    return pattern;
  }

  int setPattern(int val) {
    if (_type != 0 || val < 0 || val >= 7) { return pattern; }
    pattern = val;

    if (use_gui) {
      dlPattern.setBroadcast(false).setValue(pattern).setBroadcast(true);
      dlPattern.setCaptionLabel(dlPattern.getItem(val).get("text").toString());
      resetPatternGui();
    }
    return val;
  }

  void sendPattern() {
    sendCommand(SER_WRITE, _PATTERN, pattern);
  }


  int getArgs(int idx) {
    return args[idx];
  }

  int setArgs(int idx, int val) {
    if (_type != 0 || idx < 0 || idx >= 3) { return 0; }
    if (val < 0 || val > 255) { return args[idx]; }
    args[idx] = val;

    if (use_gui) {
      slArgs[idx].setBroadcast(false).setValue(args[idx]).setBroadcast(true);
    }
    return val;
  }

  void sendArgs(int idx) {
    sendCommand(SER_WRITE, _ARGS + idx, args[idx]);
  }


  int getPatternThresh(int x, int y) {
    return patternThresh[x][y];
  }

  int getPatternThresh(int idx) {
    return getPatternThresh(idx / 2, idx % 2);
  }

  void setPatternThresh(float[] val) {
    patternThresh[0][0] = (int)val[0];
    patternThresh[0][1] = (int)val[1];
    patternThresh[1][0] = (int)val[2];
    patternThresh[1][1] = (int)val[3];

    if (use_gui) {
      trPatternThresh.setBroadcast(false).setArrayValue(val).setBroadcast(true);
    }
  }

  int setPatternThresh(int idx, int val) {
    return setPatternThresh(idx / 2, idx % 2, val);
  }

  int setPatternThresh(int x, int y, int val) {
    if (_type != 0 || x < 0 || y < 0 || x >= 2 || y >= 2) { return 0; }
    if (val < 0 || val > 32) { return patternThresh[x][y]; }
    patternThresh[x][y] = val;

    if (use_gui) {
      trPatternThresh.setBroadcast(false).setArrayValue((x * 2) + y, val).setBroadcast(true);
    }
    return val;
  }

  void sendPatternThresh() {
    sendCommand(SER_WRITE, _PATTERNTHRESH + 0, patternThresh[0][0]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 1, patternThresh[0][1]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 2, patternThresh[1][0]);
    sendCommand(SER_WRITE, _PATTERNTHRESH + 3, patternThresh[1][1]);
  }

  void sendPatternThresh(int x, int y) {
    sendCommand(SER_WRITE, _PATTERNTHRESH + (2 * x) + y, patternThresh[x][y]);
  }


  int getTimings(int x, int y) {
    return timings[x][y];
  }

  int getTimings(int idx) {
    return getTimings(idx / 6, idx % 6);
  }

  int setTimings(int idx, int val) {
    return setTimings(idx / 6, idx % 6, val);
  }

  int setTimings(int x, int y, int val) {
    if (x < 0 || y < 0 || x >= 3 || y >= 6) { return 0; }
    if (val < 0 || val > 255) { return timings[x][y]; }
    timings[x][y] = val;
    if (use_gui) {
      slTimings[x][y].setBroadcast(false).setValue(timings[x][y]).setBroadcast(true);
    }
    return val;
  }

  void sendTimings(int idx) {
    sendTimings(idx / 6, idx % 6);
  }

  void sendTimings(int x, int y) {
    sendCommand(SER_WRITE, _TIMINGS + (6 * x) + y, timings[x][y]);
  }


  int getColorThresh(int x, int y) {
    return colorThresh[x][y];
  }

  int getColorThresh(int idx) {
    return getColorThresh(idx / 2, idx % 2);
  }

  void setColorThresh(float[] val) {
    colorThresh[0][0] = (int)val[0];
    colorThresh[0][1] = (int)val[1];
    colorThresh[1][0] = (int)val[2];
    colorThresh[1][1] = (int)val[3];
    if (use_gui) {
      trColorThresh.setBroadcast(false).setArrayValue(val).setBroadcast(true);
    }
  }

  int setColorThresh(int idx, int val) {
    return setColorThresh(idx / 2, idx % 2, val);
  }

  int setColorThresh(int x, int y, int val) {
    if (_type != 0 || x < 0 || y < 0 || x >= 2 || y >= 2) { return 0; }
    if (val < 0 || val > 32) { return colorThresh[x][y]; }
    colorThresh[x][y] = val;
    if (use_gui) {
      trColorThresh.setBroadcast(false).setArrayValue((2 * x) + y, colorThresh[x][y]).setBroadcast(true);
    }
    return val;
  }

  void sendColorThresh(int x, int y) {
    sendCommand(SER_WRITE, _COLORTHRESH + (2 * x) + y, colorThresh[x][y]);
  }

  void sendColorThresh() {
    sendCommand(SER_WRITE, _COLORTHRESH + 0, colorThresh[0][0]);
    sendCommand(SER_WRITE, _COLORTHRESH + 1, colorThresh[0][1]);
    sendCommand(SER_WRITE, _COLORTHRESH + 2, colorThresh[1][0]);
    sendCommand(SER_WRITE, _COLORTHRESH + 3, colorThresh[1][1]);
  }

  int getNumColors(int idx) {
    return numColors[idx];
  }

  int setNumColors(int idx, int val) {
    if (idx < 0 || idx >= 3) { return 0; }
    if (val < 1 || val > 9) { return numColors[idx]; }
    numColors[idx] = val;
    if (color_set == idx && color_slot >= numColors[idx]) {
      deselectColor();
    }
    if (use_gui) {
      for (int i = 0; i < 9; i++) {
        if (i < numColors[idx]) bColors[idx][i].show();
        else                    bColors[idx][i].hide();
      }
      slNumColors[idx].setBroadcast(false).setValue(numColors[idx]).setBroadcast(true);
    }
    return val;
  }

  void sendNumColors(int idx) {
    sendCommand(SER_WRITE, _NUMCOLORS + idx, numColors[idx]);
  }


  int getColors(int _color, int _set, int _channel) {
    return colors[_color][_set][_channel];
  }

  int getColors(int idx) {
    return getColors(idx / 27, (idx % 27) / 3, idx % 3);
  }

  void setColor(int _set, int _color, int[] val) {
    if (_color < 0 || _set < 0 || _color >= 9 || _set >= 3) { return; }
    colors[_set][_color][0] = val[0];
    colors[_set][_color][1] = val[1];
    colors[_set][_color][2] = val[2];
    updateColor(_set, _color);
  }

  int setColors(int idx, int val) {
    return setColors(idx / 27, (idx % 27) / 3, idx % 3, val);
  }

  void updateColor(int _set, int _color) {
    int c = translateColor(colors[_set][_color]);
    if (use_gui) {
      bColors[_set][_color].setColorBackground(c);
      bColors[_set][_color].setColorForeground(c);
      bColors[_set][_color].setColorActive(c);

      if (_set == color_set && _color == color_slot) {
        slColorValues[0].setBroadcast(false).setValue(colors[_set][_color][0]).setBroadcast(true);
        slColorValues[1].setBroadcast(false).setValue(colors[_set][_color][1]).setBroadcast(true);
        slColorValues[2].setBroadcast(false).setValue(colors[_set][_color][2]).setBroadcast(true);
      }
    }
  }

  int setColors(int _set, int _color, int _channel, int val) {
    if (_color < 0 || _set < 0 || _channel < 0 || _color >= 9 || _set >= 3 || _channel >= 3) { return 0; }
    if (val < 0 || val > 255) { return colors[_set][_color][_channel]; }
    colors[_set][_color][_channel] = val;
    updateColor(_set, _color);
    return val;
  }

  void sendColors(int _set, int _color, int _channel) {
    sendCommand(SER_WRITE, _COLORS + (_set * 27) + (_color * 3) + _channel, colors[_set][_color][_channel]);
  }

  void sendColor(int _set, int _color) {
    sendColors(_set, _color, 0);
    sendColors(_set, _color, 1);
    sendColors(_set, _color, 2);
  }


  //********************************************************************************
  //** JSON
  //********************************************************************************
  Mode fromJSON(JSONObject j) {
    setType(j.getInt("type"));
    if (_type == 0) {
      setPattern(j.getInt("pattern"));
      sendPattern();

      setArgs(j.getJSONArray("args"));
      setPatternThresh(j.getJSONArray("pattern_thresh"));
      setTimings(j.getJSONArray("timings"));
      setColorThresh(j.getJSONArray("color_thresh"));
      setNumColors(j.getJSONArray("num_colors"));
      setColors(j.getJSONArray("colors"));
    /* } else { */
    /*   setPAccelMode(j.getInt("accel_mode")); */
    /*   sendPAccelMode(); */

    /*   setPAccelTrig(j.getJSONArray("accel_trig")); */
    /*   setPAccelDrop(j.getJSONArray("accel_drop")); */
    /*   setPPatterns(j.getJSONArray("patterns")); */
    /*   setPArgs(j.getJSONArray("args")); */
    /*   setPTimings(j.getJSONArray("timings")); */
    /*   setPNumColors(j.getJSONArray("num_colors")); */
    /*   setPColors(j.getJSONArray("colors")); */
    }
    return this;
  }

  JSONObject getJSON() {
    JSONObject jo = new JSONObject();
    jo.setInt("type", _type);

    if (_type == 0) {
      jo.setInt("pattern", pattern);
      jo.setJSONArray("args", getArgs());
      jo.setJSONArray("pattern_thresh", getPatternThresh());
      jo.setJSONArray("timings", getTimings());
      jo.setJSONArray("color_thresh", getColorThresh());
      jo.setJSONArray("num_colors", getNumColors());
      jo.setJSONArray("colors", getColors());
    /* } else if (_type == 1) { */
    /*   jo.setInt("accel_mode", pAccelMode); */
    /*   jo.setJSONArray("accel_trig", getPAccelTrig()); */
    /*   jo.setJSONArray("accel_drop", getPAccelDrop()); */
    /*   jo.setJSONArray("patterns", getPPatterns()); */
    /*   jo.setJSONArray("args", getPArgs()); */
    /*   jo.setJSONArray("timings", getPTimings()); */
    /*   jo.setJSONArray("num_colors", getPNumColors()); */
    /*   jo.setJSONArray("colors", getPColors()); */
    }

    return jo;
  }

  JSONArray getArgs() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 3; i++) {
      ja.setInt(i, args[i]);
    }
    return ja;
  }

  JSONArray getPatternThresh() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 2; j++) {
        ja1.setInt(j, patternThresh[i][j]);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  JSONArray getTimings() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 3; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 6; j++) {
        ja1.setInt(j, timings[i][j]);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }

  JSONArray getColorThresh() {
    JSONArray ja = new JSONArray();
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = new JSONArray();
      for (int j = 0; j < 2; j++) {
        ja1.setInt(j, colorThresh[i][j]);
      }
      ja.setJSONArray(i, ja1);
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
          ja2.setInt(k, colors[i][j][k]);
        }
        ja1.setJSONArray(j, ja2);
      }
      ja.setJSONArray(i, ja1);
    }
    return ja;
  }


  void setArgs(JSONArray ja) {
    for (int i = 0; i < 3; i++) {
      setArgs(i, ja.getInt(i));
      sendArgs(i);
    }
  }

  void setPatternThresh(JSONArray ja) {
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = ja.getJSONArray(i);
      for (int j = 0; j < 2; j++) {
        setPatternThresh(i, j, ja1.getInt(j));
        sendPatternThresh(i, j);
      }
    }
  }

  void setTimings(JSONArray ja) {
    for (int i = 0; i < 3; i++) {
      JSONArray ja1 = ja.getJSONArray(i);
      for (int j = 0; j < 6; j++) {
        setTimings(i, j, ja1.getInt(j));
        sendTimings(i, j);
      }
    }
  }

  void setColorThresh(JSONArray ja) {
    for (int i = 0; i < 2; i++) {
      JSONArray ja1 = ja.getJSONArray(i);
      for (int j = 0; j < 2; j++) {
        setColorThresh(i, j, ja1.getInt(j));
        sendColorThresh(i, j);
      }
    }
  }

  void setNumColors(JSONArray ja) {
    for (int i = 0; i < 3; i++) {
      setNumColors(i, ja.getInt(i));
      sendNumColors(i);
    }
  }

  void setColors(JSONArray ja) {
    for (int _set = 0; _set < 3; _set++) {
      JSONArray ja1 = ja.getJSONArray(_set);
      for (int _color = 0; _color < 9; _color++) {
        JSONArray ja2 = ja1.getJSONArray(_color);
        for (int _channel = 0; _channel < 3; _channel++) {
          setColors(_set, _color, _channel, ja2.getInt(_channel));
          sendColors(_set, _color, _channel);
        }
      }
    }
  }
}
