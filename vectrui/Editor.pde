import java.util.ArrayList;
import java.util.Map;

final static String[] PATTERNS = {"Strobe", "Vexer", "Edge", "Double", "Runner"};
final static int color_bank[][] = {
  {0, 0, 0},
  {56, 64, 72},
  {24, 0, 0},
  {16, 16, 0},
  {0, 24, 0},
  {0, 16, 16},
  {0, 0, 24},
  {16, 0, 16},
  {255, 0, 0},
  {224, 32, 0},
  {192, 64, 0},
  {160, 96, 0},
  {128, 128, 0},
  {96, 160, 0},
  {64, 192, 0},
  {32, 224, 0},
  {0, 255, 0},
  {0, 224, 32},
  {0, 192, 64},
  {0, 160, 96},
  {0, 128, 128},
  {0, 96, 160},
  {0, 64, 192},
  {0, 32, 224},
  {0, 0, 255},
  {32, 0, 224},
  {64, 0, 192},
  {96, 0, 160},
  {128, 0, 128},
  {160, 0, 96},
  {192, 0, 64},
  {224, 0, 32},
  {64, 64, 64},
  {160, 16, 16},
  {16, 160, 16},
  {16, 16, 160},
  {128, 8, 48},
  {80, 48, 48},
  {128, 48, 8},
  {80, 80, 8},
  {48, 128, 8},
  {48, 80, 48},
  {8, 128, 48},
  {8, 80, 80},
  {8, 48, 128},
  {48, 48, 80},
  {48, 8, 128},
  {80, 8, 80},
};


class Editor {
  // GUI
  Group group;
  Textlabel title;
  Button nextMode;
  Button prevMode;
  DropdownList base;
  ThreshRange patternThresh;
  ThreshRange colorThresh;

  Textlabel[] argLabels = new Textlabel[3];
  Slider[][] args = new Slider[3][3];

  Textlabel[] timingLabels = new Textlabel[6];
  Slider[][] timings = new Slider[3][6];

  Textlabel[] colorLabels = new Textlabel[3];
  Slider[] numColors = new Slider[3];
  Button[][] colors = new Button[3][9];
  Button colorSelect;
  int color_set = -1;
  int color_slot = -1;

  Slider[] colorValues = new Slider[3];
  Button[] colorButtons = new Button[48];

  Button saveMode;
  Button loadMode;
  Textfield modeFilename;
  Button writeMode;
  Button resetMode;
  Button saveLight;
  Button writeLight;
  Textfield lightFilename;
  Button disconnectLight;
  Button viewMode;
  Button viewColor;

  Editor(int x, int y) {
    group = cp5.addGroup("editor")
      .setPosition(x, y)
      .setWidth(1000)
      .setHeight(800)
      .hideBar()
      .hideArrow();

    title = cp5.addTextlabel("editorTitle")
      .setGroup(group)
      .setValue("Mode 1")
      .setFont(createFont("Arial", 32))
      .setPosition(440, 5)
      .setColorValue(color(0));

    patternThresh = new ThreshRange(cp5, "editorPatternThresh")
      .setBroadcast(false)
      .setGroup(group)
      .setLabel("Pattern Thresholds")
      .setBroadcast(true);
    patternThresh.setPosition((group.getWidth() - patternThresh.getWidth()) / 2, 90);
    style("editorPatternThresh");

    colorThresh = new ThreshRange(cp5, "editorColorThresh")
      .setBroadcast(false)
      .setGroup(group)
      .setLabel("Color Set Thresholds")
      .setBroadcast(true);
    colorThresh.setPosition((group.getWidth() - colorThresh.getWidth()) / 2, 500);
    style("editorColorThresh");

    colorSelect = cp5.addButton("editorColorSelect")
      .setGroup(group)
      .setSize(40, 40)
      .setPosition(0, 0)
      .setCaptionLabel("")
      .setColorBackground(color(255))
      .hide();

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        args[i][j] = cp5.addSlider("editorArgs" + i + "." + j)
          .setBroadcast(false)
          .setCaptionLabel("")
          .setGroup(group)
          .setId(12 + (3 * j) + i)
          .setSize(250, 20)
          .setPosition(125 + (i * 275), 160 + (j * 30))
          .setColorBackground(color(32))
          .setColorActive(color(96))
          .setRange(0, 9)
          .setNumberOfTickMarks(10)
          .showTickMarks(false)
          .setDecimalPrecision(0)
          .setValue(0)
          .setBroadcast(true)
          .hide();

      }

      for (int j = 0; j < 6; j++) {
        timings[i][j] = cp5.addSlider("editorTimings" + i + "." + j)
          .setBroadcast(false)
          .setCaptionLabel("")
          .setGroup(group)
          .setId(21 + (6 * i) + j)
          .setSize(250, 20)
          .setPosition(125 + (i * 275), 280 + (j * 30))
          .setColorBackground(color(32))
          .setColorActive(color(96))
          .setRange(0, 250)
          .setNumberOfTickMarks(251)
          .showTickMarks(false)
          .setDecimalPrecision(0)
          .setValue(0)
          .setBroadcast(true)
          .hide();
      }

      for (int j = 0; j < 9; j++) {
        colors[i][j] = cp5.addButton("editorColor" + i + "." + j, (i * 9) + j)
          .setId(1000)
          .setGroup(group)
          .setSize(32, 32)
          .setPosition(284 + (40 * j), 554 + (50 * i))
          .setCaptionLabel("")
          .setColorBackground(color(0));
      }

      colorLabels[i] = cp5.addTextlabel("editorColorLabels" + i)
        .setGroup(group)
        .setValue("Color Set " + (i + 1))
        .setPosition(30, 562 + (i * 50))
        .setColorValue(color(0));

      numColors[i] = cp5.addSlider("editorNumColors" + i)
        .setBroadcast(false)
        .setCaptionLabel("")
        .setGroup(group)
        .setId(1 + i)
        .setSize(150, 20)
        .setPosition(110, 560 + (i * 50))
        .setColorBackground(color(32))
        .setColorActive(color(96))
        .setRange(1, 9)
        .setNumberOfTickMarks(9)
        .showTickMarks(false)
        .setDecimalPrecision(0)
        .setBroadcast(true);
      style("editorNumColors" + i);
    }

    for (int i = 0; i < 3; i++) {
      argLabels[i] = cp5.addTextlabel("editorArgLabels" + i)
        .setGroup(group)
        .setValue("Arg")
        .setPosition(30, 162 + (i * 30))
        .setColorValue(color(0))
        .hide();
    }

    for (int i = 0; i < 6; i++) {
      timingLabels[i] = cp5.addTextlabel("editorTimingLabels" + i)
        .setGroup(group)
        .setValue("Timing")
        .setPosition(30, 282 + (i * 30))
        .setColorValue(color(0))
        .hide();
    }

    colorValues[0] = cp5.addSlider("editorColorValuesR")
      .setBroadcast(false)
      .setCaptionLabel("")
      .setGroup(group)
      .setId(500)
      .setSize(256, 20)
      .setPosition(660, 560)
      .setColorBackground(color(96, 0, 0))
      .setColorForeground(color(192, 0, 0))
      .setColorActive(color(255, 0, 0))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);

    colorValues[1] = cp5.addSlider("editorColorValuesG")
      .setBroadcast(false)
      .setCaptionLabel("")
      .setGroup(group)
      .setId(501)
      .setSize(256, 20)
      .setPosition(660, 590)
      .setColorBackground(color(0, 96, 0))
      .setColorForeground(color(0, 192, 0))
      .setColorActive(color(0, 255, 0))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);

    colorValues[2] = cp5.addSlider("editorColorValuesB")
      .setBroadcast(false)
      .setCaptionLabel("")
      .setGroup(group)
      .setId(502)
      .setSize(256, 20)
      .setPosition(660, 620)
      .setColorBackground(color(0, 0, 96))
      .setColorForeground(color(0, 0, 192))
      .setColorActive(color(0, 0, 255))
      .setRange(0, 255)
      .setNumberOfTickMarks(256)
      .showTickMarks(false)
      .setDecimalPrecision(0)
      .setValue(0)
      .setBroadcast(true);

    for (int i = 0; i < 48; i++) {
        colorButtons[i] = cp5.addButton("editorColorBank" + i)
          .setId(2000 + i)
          .setGroup(group)
          .setSize(16, 16)
          .setPosition(22 + (20 * i), 702)
          .setCaptionLabel("")
          .setColorActive(translateColor(i))
          .setColorForeground(translateColor(i))
          .setColorBackground(translateColor(i));
    }

    base = cp5.addDropdownList("editorBase")
      .setGroup(group)
      .setLabel("Base Pattern")
      .setPosition(450, 50)
      .setSize(100, 120)
      .setItems(PATTERNS)
      .setItemHeight(20)
      .setBarHeight(20);
    base.getCaptionLabel().toUpperCase(false);
    base.getValueLabel().toUpperCase(false);
    base.getCaptionLabel().getStyle().setPadding(4,4,4,4);
    base.getValueLabel().getStyle().setPadding(4,4,4,4);
    base.close();

    prevMode = cp5.addButton("editorPrevMode")
      .setCaptionLabel("<< Prev")
      .setGroup(group)
      .setSize(60, 20)
      .setPosition(340, 15);
    prevMode.getCaptionLabel().toUpperCase(false);

    nextMode = cp5.addButton("editorNextMode")
      .setCaptionLabel("Next >>")
      .setGroup(group)
      .setSize(60, 20)
      .setPosition(600, 15);
    nextMode.getCaptionLabel().toUpperCase(false);

    resetMode = cp5.addButton("editorResetMode")
      .setCaptionLabel("Reset Mode")
      .setGroup(group)
      .setSize(100, 20)
      .setPosition(30, 730);
    resetMode.getCaptionLabel().toUpperCase(false);

    saveMode = cp5.addButton("editorSaveMode")
      .setCaptionLabel("Save Mode")
      .setGroup(group)
      .setSize(100, 20)
      .setPosition(150, 730);
    saveMode.getCaptionLabel().toUpperCase(false);

    loadMode = cp5.addButton("editorLoadMode")
      .setCaptionLabel("Load Mode")
      .setGroup(group)
      .setSize(100, 20)
      .setPosition(270, 730);
    loadMode.getCaptionLabel().toUpperCase(false);

    writeMode = cp5.addButton("editorWriteMode")
      .setCaptionLabel("Write Mode")
      .setGroup(group)
      .setSize(100, 20)
      .setPosition(390, 730);
    writeMode.getCaptionLabel().toUpperCase(false);

    /* resetLight = cp5.addButton("editorResetLight") */
    /*   .setCaptionLabel("Reset Light") */
    /*   .setGroup(group) */
    /*   .setSize(100, 20) */
    /*   .setPosition(510, 730); */
    /* resetLight.getCaptionLabel().toUpperCase(false); */

    /* saveLight = cp5.addButton("editorSaveLight") */
    /*   .setCaptionLabel("Save Light") */
    /*   .setGroup(group) */
    /*   .setSize(100, 20) */
    /*   .setPosition(630, 730); */
    /* saveLight.getCaptionLabel().toUpperCase(false); */

    /* writeLight = cp5.addButton("editorWriteLight") */
    /*   .setCaptionLabel("Write Light") */
    /*   .setGroup(group) */
    /*   .setSize(100, 20) */
    /*   .setPosition(750, 730); */
    /* writeLight.getCaptionLabel().toUpperCase(false); */

    disconnectLight = cp5.addButton("editorDisconnectLight")
      .setCaptionLabel("Disconnect")
      .setGroup(group)
      .setSize(100, 20)
      .setPosition(870, 730);
    disconnectLight.getCaptionLabel().toUpperCase(false);

    modeFilename = cp5.addTextfield("editorModeFilename")
      .setGroup(group)
      .setCaptionLabel("")
      .setColorBackground(color(0))
      .setPosition(150, 760)
      .setSize(220, 20)
      .setText("vectr.mode")
      .setAutoClear(false);
    modeFilename.getValueLabel().getStyle().setPadding(0, 0, 0, 4);

    /* lightFilename = cp5.addTextfield("editorLightFilename") */
    /*   .setGroup(group) */
    /*   .setCaptionLabel("") */
    /*   .setColorBackground(color(0)) */
    /*   .setPosition(630, 760) */
    /*   .setSize(220, 20) */
    /*   .setText("vectr.light") */
    /*   .setAutoClear(false); */
    /* lightFilename.getValueLabel().getStyle().setPadding(0, 0, 0, 4); */

    viewMode = cp5.addButton("editorViewMode")
      .setCaptionLabel("View Mode")
      .setGroup(group)
      .setColorBackground(color(0, 90, 180))
      .setSize(100, 20)
      .setPosition(678, 660);
    viewMode.getCaptionLabel().toUpperCase(false);

    viewColor = cp5.addButton("editorViewColor")
      .setCaptionLabel("View Color")
      .setGroup(group)
      .setColorBackground(color(0, 45, 90))
      .setSize(100, 20)
      .setPosition(798, 660)
      .hide();
    viewColor.getCaptionLabel().toUpperCase(false);
  }

  void curModeChanged(int m) {
    title.setText("Mode " + (m + 1));
    seta(0, modes[m].pattern);
    for (int i = 0; i < 3; i++) { seta(i + 1, modes[cur_mode].numColors[i]); }
    for (int i = 0; i < 4; i++) { seta(i + 4, modes[cur_mode].patternThresh[i / 2][i % 2]); }
    for (int i = 0; i < 4; i++) { seta(i + 8, modes[cur_mode].colorThresh[i / 2][i % 2]); }
    for (int i = 0; i < 9; i++) { seta(i + 12, modes[cur_mode].args[i / 3][i % 3]); }
    for (int i = 0; i < 18; i++) { seta(i + 21, modes[cur_mode].timings[i / 6][i % 6]); }
    for (int i = 0; i < 81; i++) { seta(i + 39, modes[cur_mode].colors[i / 9][(i % 9) / 3][i % 3]); }

    color_set = -1;
    color_slot = -1;
    colorSelect.hide();
    viewMode.hide();
    viewColor.hide();
  }

  void numColorsChanged(int i, int v) {
    for (int j = 0; j < 9; j++) {
      if (j < v) {
        colors[i][j].show();
      } else {
        colors[i][j].hide();
      }
    }
  }

  void patternThreshChanged(int i, int j) {
    patternThresh.setBroadcast(false).setArrayValue(i, j).setBroadcast(true);
  }

  void colorThreshChanged(int i, int j) {
    colorThresh.setBroadcast(false).setArrayValue(i, j).setBroadcast(true);
  }

  void patternChanged(int p) {
    // TODO
    switch (p) {
      case 0: // Strobe
        argLabels[0].setValue("Group Size").show();
        for (int i = 0; i < 3; i++) {
          args[i][0].setBroadcast(false)
            .setRange(0, 9)
            .setNumberOfTickMarks(10)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[1].setValue("Skip After").show();
        for (int i = 0; i < 3; i++) {
          args[i][1].setBroadcast(false)
            .setRange(0, 9)
            .setNumberOfTickMarks(10)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[2].setValue("Repeat Group").show();
        for (int i = 0; i < 3; i++) {
          args[i][2].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        timingLabels[0].setValue("Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][0].show(); }

        timingLabels[1].setValue("Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][1].show(); }

        timingLabels[2].setValue("Tail Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][2].show(); }

        timingLabels[3].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][3].hide(); }

        timingLabels[4].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][4].hide(); }

        timingLabels[5].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][5].hide(); }
        break;

      case 1: // Vexer
        argLabels[0].setValue("Repeat Strobe").show();
        for (int i = 0; i < 3; i++) {
          args[i][0].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[1].setValue("Repeat Tracer").show();
        for (int i = 0; i < 3; i++) {
          args[i][1].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[2].setValue("").hide();
        for (int i = 0; i < 3; i++) { args[i][2].hide(); }

        timingLabels[0].setValue("Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][0].show(); }

        timingLabels[1].setValue("Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][1].show(); }

        timingLabels[2].setValue("Tracer Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][2].show(); }

        timingLabels[3].setValue("Tracer Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][3].show(); }

        timingLabels[4].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][4].hide(); }

        timingLabels[5].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][5].hide(); }
        break;

      case 2: // Edge
        argLabels[0].setValue("Group Size").show();
        for (int i = 0; i < 3; i++) {
          args[i][0].setBroadcast(false)
            .setRange(0, 9)
            .setNumberOfTickMarks(10)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[1].setValue("").hide();
        for (int i = 0; i < 3; i++) { args[i][1].hide(); }

        argLabels[2].setValue("").hide();
        for (int i = 0; i < 3; i++) { args[i][2].hide(); }

        timingLabels[0].setValue("Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][0].show(); }

        timingLabels[1].setValue("Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][1].show(); }

        timingLabels[2].setValue("Center Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][2].show(); }

        timingLabels[3].setValue("Trailing Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][3].show(); }

        timingLabels[4].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][4].hide(); }

        timingLabels[5].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][5].hide(); }
        break;

      case 3: // Double
        argLabels[0].setValue("Repeat First").show();
        for (int i = 0; i < 3; i++) {
          args[i][0].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[1].setValue("Repeat Second").show();
        for (int i = 0; i < 3; i++) {
          args[i][1].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[2].setValue("").hide();
        for (int i = 0; i < 3; i++) { args[i][2].hide(); }

        timingLabels[0].setValue("First Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][0].show(); }

        timingLabels[1].setValue("First Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][1].show(); }

        timingLabels[2].setValue("Second Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][2].show(); }

        timingLabels[3].setValue("Second Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][3].show(); }

        timingLabels[4].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][4].hide(); }

        timingLabels[5].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][5].hide(); }
        break;

      case 4: // Runner
        argLabels[0].setValue("Group Size").show();
        for (int i = 0; i < 3; i++) {
          args[i][0].setBroadcast(false)
            .setRange(0, 9)
            .setNumberOfTickMarks(10)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[1].setValue("Skip Between").show();
        for (int i = 0; i < 3; i++) {
          args[i][1].setBroadcast(false)
            .setRange(0, 9)
            .setNumberOfTickMarks(10)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        argLabels[2].setValue("Repeat Runner").show();
        for (int i = 0; i < 3; i++) {
          args[i][2].setBroadcast(false)
            .setRange(1, 100)
            .setNumberOfTickMarks(100)
            .showTickMarks(false)
            .setValue(0)
            .setBroadcast(true)
            .show();
        }

        timingLabels[0].setValue("Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][0].show(); }

        timingLabels[1].setValue("Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][1].show(); }

        timingLabels[2].setValue("Runner Strobe").show();
        for (int i = 0; i < 3; i++) { timings[i][2].show(); }

        timingLabels[3].setValue("Runner Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][3].show(); }

        timingLabels[4].setValue("Split Blank").show();
        for (int i = 0; i < 3; i++) { timings[i][4].show(); }

        timingLabels[5].setValue("").hide();
        for (int i = 0; i < 3; i++) { timings[i][5].hide(); }
        break;
    }
  }

  void seta(int addr, int val) {
    if (addr == 0) {
      base.setBroadcast(false).setValue(val).setBroadcast(true);
      base.setCaptionLabel(base.getItem(val).get("text").toString());
      patternChanged(val);
    } else if (addr < 4) {
      numColors[addr - 1].setBroadcast(false).setValue(val).setBroadcast(true);
      numColorsChanged(addr - 1, val);
    } else if (addr < 8) {
      patternThresh.setBroadcast(false).setArrayValue(addr - 4, val).setBroadcast(true);
    } else if (addr < 12) {
      colorThresh.setBroadcast(false).setArrayValue(addr - 8, val).setBroadcast(true);
    } else if (addr < 21) {
      args[(addr - 12) / 3][(addr - 12) % 3].setBroadcast(false).setValue(val).setBroadcast(true);
    } else if (addr < 39) {
      timings[(addr - 21) / 6][(addr - 21) % 6].setBroadcast(false).setValue(val).setBroadcast(true);
    } else if (addr < 120) {
      int chan = (addr - 39) % 3;
      int slot = (addr - 39) / 9;
      int set = ((addr - 39) % 9) / 3;
      int x, y;

      if (chan == 0) {
        x = modes[cur_mode].colors[slot][set][1];
        y = modes[cur_mode].colors[slot][set][2];
        x = (x == 0) ? 0 : (x / 2) + 128;
        y = (y == 0) ? 0 : (y / 2) + 128;
        val = (val == 0) ? 0 : (val / 2) + 128;
        colors[set][slot].setColorBackground(color(val, x, y));
        colors[set][slot].setColorForeground(color(val, x, y));
        colors[set][slot].setColorActive(color(val, x, y));
      } else if (chan == 1) {
        x = modes[cur_mode].colors[slot][set][0];
        y = modes[cur_mode].colors[slot][set][2];
        x = (x == 0) ? 0 : (x / 2) + 128;
        y = (y == 0) ? 0 : (y / 2) + 128;
        val = (val == 0) ? 0 : (val / 2) + 128;
        colors[set][slot].setColorBackground(color(x, val, y));
        colors[set][slot].setColorForeground(color(x, val, y));
        colors[set][slot].setColorActive(color(x, val, y));
      } else if (chan == 2) {
        x = modes[cur_mode].colors[slot][set][0];
        y = modes[cur_mode].colors[slot][set][1];
        x = (x == 0) ? 0 : (x / 2) + 128;
        y = (y == 0) ? 0 : (y / 2) + 128;
        val = (val == 0) ? 0 : (val / 2) + 128;
        colors[set][slot].setColorBackground(color(x, y, val));
        colors[set][slot].setColorForeground(color(x, y, val));
        colors[set][slot].setColorActive(color(x, y, val));
      }
    }
  }

  void selectColor(float v) {
    selectColor((int)v / 9, (int)v % 9);
  }

  void selectColor(int set, int slot) {
    if (slot >= modes[cur_mode].numColors[set]) { return; }

    color_set = set;
    color_slot = slot;

    float p[] = colors[set][slot].getPosition();
    colorSelect.setPosition(p[0] - 4, p[1] - 4).show();
    viewMode.show();
    viewColor.show();

    for (int i = 0; i < 3; i++) {
      colorValues[i].setBroadcast(false)
        .setValue(modes[cur_mode].colors[slot][set][i])
        .setBroadcast(true);
    }
  }
}


void style(String theControllerName) {
  Controller c = cp5.getController(theControllerName);
  c.getCaptionLabel().toUpperCase(false);
  c.getCaptionLabel().getStyle().setPadding(4,4,4,4);
  c.getCaptionLabel().getStyle().setMargin(-4,0,0,0);
}

int translateColor(int i) {
  int r, g, b;
  r = (color_bank[i][0] == 0) ? 0 : 128 + (color_bank[i][0] / 2);
  g = (color_bank[i][1] == 0) ? 0 : 128 + (color_bank[i][1] / 2);
  b = (color_bank[i][2] == 0) ? 0 : 128 + (color_bank[i][2] / 2);
  return (255 << 24) + (r << 16) + (g << 8) + b;
}
