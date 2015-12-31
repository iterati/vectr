String[] COLOR_FUNCS = {"Static", "Speed", "Tilt"};
String[] PATTERNS = {"Strobe", "Tracer", "Vexer", "Edge"};

class Mode {
  int idx;
  int color_func = 1;
  int pattern = 0;
  int num_colors = 7;
  int[][] args = {
    {5, 20, 0, 0},
    {25, 0, 0, 0},
  };
  int[][][] colors = {
    {
      {64, 0, 0},
      {64, 64, 0},
      {0, 64, 0},
      {0, 64, 64},
      {0, 0, 64},
      {64, 0, 64},
      {64, 64, 64},
    },
    {
      {128, 0, 0},
      {128, 128, 0},
      {0, 128, 0},
      {0, 128, 128},
      {0, 0, 128},
      {128, 0, 128},
      {128, 128, 128},
    },
    {
      {255, 0, 0},
      {255, 255, 0},
      {0, 255, 0},
      {0, 255, 255},
      {0, 0, 255},
      {255, 0, 255},
      {255, 255, 255},
    },
  };

  Mode(int i) {
    idx = i;
  }
}

class ModeEditor {
  Group grp;
  Textlabel tlTitle;
  Button btnPrev, btnNext;
  DropdownList ddlColorFunc, ddlPattern;
  Button btnLess, btnMore;
  Button[][] btnColors = new Button[3][7];
  Slider[][] sdrArgs = new Slider[2][4];
  Button btnSelected;
  Mode[] modes = new Mode[7];
  int selected_p, selected_s;

  ColorSliders csl;

  ModeEditor(int x, int y) {
    grp = cp5.addGroup("mode")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();

    /* tlTitle = cp5.addTextlabel("modeTitle") */
    /*   .setText("Mode 1") */
    /*   .setPosition(340, 20) */
    /*   .setColorValue(0xddddddd) */
    /*   .setFont(createFont("Arial", 32)) */
    /*   .setGroup(grp); */

    /* for (int p = 0; p < 3; p++) { */
    /*   for (int s = 0; s < 7; s++) { */
    /*     btnColors[p][s] = makeColorButton(p, s, grp) */
    /*       .setPosition(s * 50, p * 70); */
    /*   } */
    /* } */

    /* for (int p = 0; p < 2; p++) { */
    /*   for (int a = 0; a < 4; a++) { */
    /*     sdrArgs[p][a] = makeArgSlider(p, a, grp) */
    /*       .setPosition(20 + (p * 260), 400 + (a * 20)); */
    /*   } */
    /* } */

    csl = new ColorSliders(cp5, "test");
  }
}

Button makeColorButton(int p, int s, Group grp) {
  return cp5.addButton("modeColors_" + p + "_" + s)
    .setSize(32, 32)
    .setColorBackground(color(0))
    .setCaptionLabel("")
    .setGroup(grp);
}


Textfield makeArgSliderTF(int p, int a, Group grp) {
  return cp5.addTextfield("modeTest")
    .setValue("")
    .setSize(140, 20)
    .setColorBackground(color(0))
    .setText("")
    .setCaptionLabel("")
    .setGroup(grp);
}

Slider makeArgSlider(int p, int a, Group grp) {
  return cp5.addSlider("modeArgs_" + p + "_" + a)
    .setSize(250, 8)
    .setRange(0, 125)
    .setNumberOfTickMarks(251)
    .snapToTickMarks(true)
    .showTickMarks(false)
    .setValue(0)
    .setColorForeground(color(96))
    .setColorBackground(color(64))
    .setColorActive(color(128))
    .setCaptionLabel("")
    .setTriggerEvent(Slider.RELEASE)
    .setGroup(grp);
}

DropdownList makeColorFuncDropdownList(Group grp) {
  DropdownList dd = cp5.addDropdownList("modeColorFunc")
    .setBackgroundColor(color(200))
    .setColorActive(color(255, 128))
    .setItemHeight(20)
    .setBarHeight(15);

  for (int i = 0; i < COLOR_FUNCS.length; i++) {
    dd.addItem(COLOR_FUNCS[i], i);
  }
  dd.close();
  return dd;
}
