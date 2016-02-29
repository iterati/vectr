class Mode {
  boolean use_gui = true;

  static final int _TYPE = 0;
  static final int _MODESIZE = 128;

  int   _type = 0;
  int[] data = new int[_MODESIZE];

  VectrMode vmode;
  PrimerMode pmode;

  Group gMain;
  Group gVectr;
  Group gPrimer;
  Group gTitle;
  Group gControls;
  Group gColorEdit;
  Group gColorBank;

  // Title
  Textlabel tlTitle;
  Button bNextMode;
  Button bPrevMode;

  // Controls
  Button bSaveMode;
  Button bLoadMode;
  Button bWriteMode;
  Button bResetMode;
  Button bSaveLight;
  Button bWriteLight;
  Button bDisconnectLight;
  Button bLoadColors;

  // ColorEdit
  Slider[] slColorValues = new Slider[3];
  Button bViewMode;
  Button bViewColor;

  // ColorBank
  Button[][] bColorBank = new Button[48][4];

  DropdownList dlType;


  Mode() {
    use_gui = false;
    vmode = new VectrMode();
    pmode = new PrimerMode();
  }

  Mode(Group g) {
    gMain = g;

    dlType = cp5.addDropdownList("type")
      .setGroup(gMain)
      .setId(ID_TYPE)
      /* .setPosition(50, 20) */
      .setPosition(580, 20)
      .setSize(90, 60)
      .setItems(MODETYPES);
    style(dlType);

    gTitle = cp5.addGroup("title")
      .setGroup(gMain)
      .setPosition(300, 10)
      .hideBar()
      .hideArrow();
    makeTitle();

    bDisconnectLight = cp5.addButton("disconnectLight")
      .setCaptionLabel("Disconnect")
      .setGroup(gMain)
      .setPosition(50, 20);
    style(bDisconnectLight, 90);

    bLoadColors = cp5.addButton("loadColorBank")
      .setCaptionLabel("Load Colors")
      .setGroup(gMain)
      .setPosition(170, 20);
    style(bLoadColors, 90);

    gControls = cp5.addGroup("controls")
      .setGroup(gMain)
      .setPosition(30, 680)
      .hideBar()
      .hideArrow();
    makeControls();

    gColorEdit = cp5.addGroup("colorEdit")
      .setGroup(gMain)
      .setPosition(554, 540)
      .hideBar()
      .hideArrow()
      .hide();
    makeColorEdit();

    gVectr = cp5.addGroup("vectr")
      .setGroup(gMain)
      .setPosition(20, 60)
      .hideBar()
      .hideArrow();
    vmode = new VectrMode(gVectr);
    gVectr.hide();

    gPrimer = cp5.addGroup("primer")
      .setGroup(gMain)
      .setPosition(20, 60)
      .hideBar()
      .hideArrow();
    pmode = new PrimerMode(gPrimer);
    gPrimer.hide();

    gColorBank = cp5.addGroup("colorBank")
      .setGroup(gMain)
      .setPosition(820, 0)
      .hideBar()
      .hideArrow();
    makeColorBank();

    dlType.bringToFront();
  }

  int geta(int addr) {
    if (addr < 0 || addr >= _MODESIZE) {
    } else if (addr == 0) {
      if (_type != 0 && _type != 1) {
        // force vectr mode
        _type = 0;
      }
      return _type;
    } else {
      if (_type != 1) {
        return vmode.geta(addr);
      } else {
        return pmode.geta(addr);
      }
    }
    return 0;
  }

  void seta(int addr, int val) {
    if (addr < 0 || addr >= _MODESIZE) {
    } else if (addr == 0) {
      setType(val);
      if (_type == 1) {
        pmode.setTriggerMode(0);
      }
    } else {
      if (_type != 1) {
        vmode.seta(addr, val);
      } else {
        pmode.seta(addr, val);
      }
    }
  }

  void resetTypeGui() {
    gVectr.hide();
    gPrimer.hide();

    if (_type != 1) {
      gVectr.show();
      vmode.deselectColor();
      vmode.setPattern(0);
      for (int i = 0; i < 4; i++) {
        vmode.setPatternThresh(i, 32);
        vmode.setColorThresh(i, 32);
      }
      for (int i = 0; i < 3; i++) {
        vmode.setNumColors(i, 3);
        vmode.setColor(i, 0, COLOR_BANK[0]);
        vmode.setColor(i, 1, COLOR_BANK[8]);
        vmode.setColor(i, 2, COLOR_BANK[16]);
        vmode.setColor(i, 3, COLOR_BANK[24]);
        vmode.setColor(i, 4, COLOR_BANK[24]);
        vmode.setColor(i, 5, COLOR_BANK[24]);
        vmode.setColor(i, 6, COLOR_BANK[24]);
        vmode.setColor(i, 7, COLOR_BANK[24]);
        vmode.setColor(i, 8, COLOR_BANK[24]);
      }
    } else {
      gPrimer.show();
      pmode.deselectColor();
      pmode.setPattern(0, 0);
      pmode.setPattern(1, 0);
      pmode.setTriggerMode(0);
      for (int i = 0; i < 2; i++) {
        pmode.setTriggerThresh(i, 32);
      }
      for (int i = 0; i < 2; i++) {
        pmode.setNumColors(i, 3);
        pmode.setColor(i, 0, COLOR_BANK[0]);
        pmode.setColor(i, 1, COLOR_BANK[8]);
        pmode.setColor(i, 2, COLOR_BANK[16]);
        pmode.setColor(i, 3, COLOR_BANK[24]);
        pmode.setColor(i, 4, COLOR_BANK[24]);
        pmode.setColor(i, 5, COLOR_BANK[24]);
        pmode.setColor(i, 6, COLOR_BANK[24]);
        pmode.setColor(i, 7, COLOR_BANK[24]);
        pmode.setColor(i, 8, COLOR_BANK[24]);
      }
    }

    deselectColor();
    resetPatternGui();
    resetArgsAndTimings();
  }

  void resetPatternGui() {
    if (_type != 1) {
      vmode.resetPatternGui();
    } else {
      pmode.resetPatternGui(0);
      pmode.resetPatternGui(1);
    }
  }

  void resetArgsAndTimings() {
    if (_type != 1) {
      vmode.resetArgsAndTimings();
    } else {
      pmode.resetArgsAndTimings(0);
      pmode.resetArgsAndTimings(1);
    }
  }

  void resetArgsAndTimings(int i) {
    if (_type == 1) {
      pmode.resetArgsAndTimings(i);
    }
  }

  void deselectColor() {
    gColorEdit.hide();
    if (_type != 1) {
      vmode.deselectColor();
    } else {
      pmode.deselectColor();
    }
  }

  void selectColor(int i) {
    boolean success = false;
    if (_type != 1) {
      success = vmode.selectColor(i);
    } else {
      success = pmode.selectColor(i);
    }
    if (success) {
      if (!view_mode) {
        sendCommand(SER_VIEW_COLOR, getColorSet(), getColorSlot());
      }
      gColorEdit.show();
    }
  }

  //********************************************************************************
  // Setters
  //********************************************************************************
  // Type
  void setType(int val) {
    if (oob(val, 0, 1)) { val = 0; }
    int old = _type;
    _type = val;
    if (use_gui) {
      dlType.setBroadcast(false).setValue(_type).setBroadcast(true);
      dlType.setCaptionLabel(dlType.getItem(val).get("text").toString());
      if (old != _type) {
        resetTypeGui();
        sendMode();
      }
    }
  }

  void sendType() {
    sendCommand(SER_WRITE, _TYPE, _type);
  }

  // Pattern
  void setPattern(int val) {
    setPattern(0, val);
  }

  void setPattern(int i, int val) {
    if (_type != 1) {
      vmode.setPattern(val);
    } else {
      pmode.setPattern(i, val);
    }
  }

  void sendPattern() {
    sendPattern(0);
  }

  void sendPattern(int i) {
    if (_type != 1) {
      vmode.sendPattern();
    } else {
      pmode.sendPattern(i);
    }
  }

  // Args
  void setArgs(int i, int val) {
    if (_type != 1) {
      vmode.setArgs(i, val);
    } else {
      pmode.setArgs(i, val);
    }
  }

  void sendArgs(int i) {
    if (_type != 1) {
      vmode.sendArgs(i);
    } else {
      pmode.sendArgs(i);
    }
  }

  // Timings
  void setTimings(int i, int val) {
    if (_type != 1) {
      vmode.setTimings(i, val);
    } else {
      pmode.setTimings(i, val);
    }
  }

  void setTimings(int x, int y, int val) {
    if (_type != 1) {
      vmode.setTimings(x, y, val);
    } else {
      pmode.setTimings(x, y, val);
    }
  }

  void sendTimings(int i) {
    if (_type != 1) {
      vmode.sendTimings(i);
    } else {
      pmode.sendTimings(i);
    }
  }

  void sendTimings(int x, int y) {
    if (_type != 1) {
      vmode.sendTimings(x, y);
    } else {
      pmode.sendTimings(x, y);
    }
  }

  // Num Colors
  void setNumColors(int i, int val) {
    if (_type != 1) {
      vmode.setNumColors(i, val);
    } else {
      pmode.setNumColors(i, val);
    }
  }

  void sendNumColors(int i) {
    if (_type != 1) {
      vmode.sendNumColors(i);
    } else {
      pmode.sendNumColors(i);
    }
  }

  // Colors
  void setColor(int _set, int _color, int _channel, int val) {
    if (_type != 1) {
      vmode.setColor(_set, _color, _channel, val);
    } else {
      pmode.setColor(_set, _color, _channel, val);
    }
  }

  void setColor(int _set, int _color, int[] val) {
    if (_type != 1) {
      vmode.setColor(_set, _color, val);
    } else {
      pmode.setColor(_set, _color, val);
    }
  }

  void setColor(int i, int val) {
    if (_type != 1) {
      vmode.setColor(i, val);
    } else {
      pmode.setColor(i, val);
    }
  }

  void sendColor(int _set, int _color, int _channel) {
    if (_type != 1) {
      vmode.sendColor(_set, _color, _channel);
    } else {
      pmode.sendColor(_set, _color, _channel);
    }
  }

  void sendColor(int _set, int _color) {
    if (_type != 1) {
      vmode.sendColor(_set, _color);
    } else {
      pmode.sendColor(_set, _color);
    }
  }

  // Pattern Thresh - Vectr only
  void setPatternThresh(float[] val) {
    if (_type != 1) {
      vmode.setPatternThresh(val);
    }
  }

  void setPatternThresh(int i, int val) {
    if (_type != 1) {
      vmode.setPatternThresh(i, val);
    }
  }

  void sendPatternThresh() {
    if (_type != 1) {
      vmode.sendPatternThresh();
    }
  }

  // Color Thresh - Vectr only
  void setColorThresh(float[] val) {
    if (_type != 1) {
      vmode.setColorThresh(val);
    }
  }

  void setColorThresh(int i, int val) {
    if (_type != 1) {
      vmode.setColorThresh(i, val);
    }
  }

  void sendColorThresh() {
    if (_type != 1) {
      vmode.sendColorThresh();
    }
  }

  // Trigger Mode - Primer Only
  void setTriggerMode(int val) {
    if (_type == 1) {
      pmode.setTriggerMode(val);
    }
  }

  void sendTriggerMode() {
    if (_type == 1) {
      pmode.sendTriggerMode();
    }
  }

  // Trigger Thresh - Primer Only
  void setTriggerThresh(float[] val) {
    if (_type == 1) {
      pmode.setTriggerThresh(val);
    }
  }

  void setTriggerThresh(int i, int val) {
    if (_type == 1) {
      pmode.setTriggerThresh(i, val);
    }
  }

  void sendTriggerThresh() {
    if (_type == 1) {
      pmode.sendTriggerThresh();
    }
  }


  //********************************************************************************
  // JSON
  //********************************************************************************
  void fromJSON(JSONObject jo) {
    try {
      setType(jo.getInt("type"));
    } catch (Exception ex) {
      setType(0);
    }
    if (_type != 1) {
      vmode.fromJSON(jo);
    } else {
      pmode.fromJSON(jo);
    }

    for (int i = 0; i < _MODESIZE; i++) {
      sendCommand(SER_WRITE, i, geta(i));
    }
  }

  JSONObject getJSON() {
    JSONObject jo = new JSONObject();
    if (_type != 1) {
      jo = vmode.getJSON();
    } else {
      jo = pmode.getJSON();
    }
    jo.setInt("type", _type);
    return jo;
  }

  void makeTitle() {
    tlTitle = cp5.addTextlabel("tlTitle")
      .setGroup(gTitle)
      .setValue("Mode 1")
      .setFont(createFont("Comfortaa-Regular", 32))
      .setPosition(60, 0)
      .setSize(120, 40)
      .setColorValue(color(240));

    bPrevMode = cp5.addButton("prevMode")
      .setCaptionLabel("<<")
      .setGroup(gTitle)
      .setPosition(0, 10);
    style(bPrevMode, 20);

    bNextMode = cp5.addButton("nextMode")
      .setCaptionLabel(">>")
      .setGroup(gTitle)
      .setPosition(220, 10);
    style(bNextMode, 20);
  }

  void makeControls() {
    bResetMode = cp5.addButton("resetChanges")
      .setCaptionLabel("Undo Edits")
      .setGroup(gControls)
      .setPosition(5, 0);
    style(bResetMode, 90);

    bWriteMode = cp5.addButton("writeChanges")
      .setCaptionLabel("Save Edits")
      .setGroup(gControls)
      .setPosition(105, 0);
    style(bWriteMode, 90);

    bSaveMode = cp5.addButton("saveMode")
      .setCaptionLabel("Save Mode")
      .setGroup(gControls)
      .setPosition(305, 0);
    style(bSaveMode, 90);

    bLoadMode = cp5.addButton("uploadMode")
      .setCaptionLabel("Upload Mode")
      .setGroup(gControls)
      .setPosition(405, 0);
    style(bLoadMode, 90);

    bSaveLight = cp5.addButton("saveLight")
      .setCaptionLabel("Save Light")
      .setGroup(gControls)
      .setPosition(605, 0);
    style(bSaveLight, 90);

    bWriteLight = cp5.addButton("uploadLight")
      .setCaptionLabel("Upload Light")
      .setGroup(gControls)
      .setPosition(705, 0);
    style(bWriteLight, 90);
  }

  void makeColorEdit() {
    slColorValues[0] = cp5.addSlider("ColorValuesRed")
      .setGroup(gColorEdit)
      .setId(ID_COLOREDIT + 0)
      .setLabel("")
      .setPosition(0, 0);
    style(slColorValues[0], 256, 0, 255);
    slColorValues[0].setColorBackground(color(64, 0, 0))
      .setColorForeground(color(128, 0, 0))
      .setColorActive(color(192, 0, 0));

    slColorValues[1] = cp5.addSlider("ColorValuesGreen")
      .setGroup(gColorEdit)
      .setId(ID_COLOREDIT + 1)
      .setLabel("")
      .setPosition(0, 30);
    style(slColorValues[1], 256, 0, 255);
    slColorValues[1].setColorBackground(color(0, 64, 0))
      .setColorForeground(color(0, 128, 0))
      .setColorActive(color(0, 192, 0));

    slColorValues[2] = cp5.addSlider("ColorValuesBlue")
      .setGroup(gColorEdit)
      .setId(ID_COLOREDIT + 2)
      .setLabel("")
      .setPosition(0, 60);
    style(slColorValues[2], 256, 0, 255);
    slColorValues[2].setColorBackground(color(0, 0, 64))
      .setColorForeground(color(0, 0, 128))
      .setColorActive(color(0, 0, 192));

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
  }

  void makeColorBank() {
    for (int g = 0; g < 6; g++) {
      for (int c = 0; c < 8; c++) {
        for (int s = 0; s < 4; s++) {
          bColorBank[(g * 8) + c][s] = cp5.addButton("ColorBank" + ((g * 8) + c) + "." + s)
            .setGroup(gColorBank)
            .setId(ID_COLORBANK + ((g * 8) + c) + (100 * s))
            .setLabel("")
            .setSize(16, 16)
            .setPosition(24 + 4 + (24 * c), 22 + 4 + (116 * g) + (24 * s))
            .setColorBackground(getColorBankColor((g * 8) + c, s))
            .setColorForeground(getColorBankColor((g * 8) + c, s))
            .setColorActive(getColorBankColor((g * 8) + c, s));
        }
      }
    }
  }

  void loadColorBank() {
    for (int c = 0; c < 48; c++) {
      for (int s = 0; s < 4; s++) {
        bColorBank[c][s]
          .setColorActive(getColorBankColor(c, s))
          .setColorBackground(getColorBankColor(c, s))
          .setColorForeground(getColorBankColor(c, s));
      }
    }
  }

  int getColorSet() {
    if (_type != 1) {
      return vmode.color_set;
    } else {
      return pmode.color_set;
    }
    return -1;
  }

  int getColorSlot() {
    if (_type != 1) {
      return vmode.color_slot;
    } else {
      return pmode.color_slot;
    }
    return -1;
  }

  void closeDropdowns() {
    vmode.dlPattern.close();
    pmode.dlPattern[0].close();
    pmode.dlPattern[1].close();
  }

  void sendMode() {
    if (!reading) {
      sendCommand(SER_WRITE_MODE);
      for (int i = 0; i < _MODESIZE; i++) {
        sendCommand(SER_WRITE, i, geta(i));
        delay(2);
      }
      sendCommand(SER_WRITE_MODE_END);
    }
  }
}
