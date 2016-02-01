import java.awt.event.KeyEvent;
import processing.serial.*;
import controlP5.*;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileNameExtensionFilter;

ControlP5 cp5;

int COLOR_BANK[][] = new int[48][3];
final static String[] PATTERNS = {"Strobe", "Vexer", "Edge", "Double", "Runner", "Stepper", "Random"};
/* final static String[] PPATTERNS = {"Strobe", "Vexer", "Edge", "Double", "Runner", "Stepper", "Random", "Flux"}; */
/* final static String[] ACCELMODES = {"Off", "Velocity", "Tilt", "Roll", "Flip"}; */
final static String[] MODETYPES = {"Vectr", "Primr"};

static final int SER_DUMP           = 10;
static final int SER_DUMP_LIGHT     = 11;
static final int SER_SAVE           = 20;
static final int SER_READ           = 30;
static final int SER_WRITE          = 40;
static final int SER_WRITE_LIGHT    = 41;
static final int SER_CHANGE_MODE    = 50;
static final int SER_VIEW_MODE      = 100;
static final int SER_VIEW_COLOR     = 110;
static final int SER_DUMP_START     = 200;
static final int SER_DUMP_END       = 210;
static final int SER_HANDSHAKE      = 250;
static final int SER_HANDSHACK      = 251;
static final int SER_DISCONNECT     = 254;

static final int SER_VERSION = 101;

int num_ports = 0;
Serial ports[] = new Serial[10];
Serial port = null;

// State variables
int gui_state = 0;
boolean initialized = false;
boolean reading = false;
boolean flashing = false;
boolean view_mode = true;

// Light variables
int cur_mode = 0;

// All the groups
Group gMain;
Group gTitle;
Group gControls;
Group gColorEdit;
Group gColorBank;
Group gMode;

// Main
Mode mode;
Mode[] modes = new Mode[7];

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

// ColorBank
Button[] bColorBank = new Button[48];

String file_path = "";


void setup() {
  surface.setTitle("VectrUI 02-02-2016");
  smooth(8);
  size(1280, 720);
  loadColorBank();

  cp5 = new ControlP5(this);
  cp5.setFont(createFont("Comfortaa-Bold", 14));

  setupMainGroup();

  for (int i = 0; i < 7; i++) {
    modes[i] = new Mode();
  }
}

void draw() {
  background(16);
  if (!initialized) {
    connectLight();
  }

  if (port == null) {
    for (int i = 0; i < num_ports; i++) {
      if (ports[i].available() >= 3) {
        port = ports[i];
        readCommand();
      }
    }
  } else {
    while (port.available() >= 3) {
      readCommand();
    }
  }

  if (!initialized || reading || flashing) {
    gMain.hide();
  } else {
    gMain.show();
  }
}


void connectLight() {
  for (String p: Serial.list()) {
    try {
      if (num_ports < 10) {
        ports[num_ports] = new Serial(this, p, 115200);
        num_ports++;
      }
    } catch (Exception e) {
    }
  }
}

void sendCommand(int cmd) {
  sendCommand(cmd, 0, 0, 0);
}

void sendCommand(int cmd, int in0) {
  sendCommand(cmd, in0, 0, 0);
}

void sendCommand(int cmd, int in0, int in1) {
  sendCommand(cmd, in0, in1, 0);
}

void sendCommand(int cmd, int in0, int in1, int in2) {
  /* println("send " + cmd + " " + in0 + " " + in1 + " " + in2); */
  if (initialized) {
    port.write(cmd);
    port.write(in0);
    port.write(in1);
    port.write(in2);
  }
}

void readCommand() {
  int cmd = port.read();
  int addr = port.read();
  int val = port.read();
  /* if (cmd > 7) println("get " + cmd + " " + addr + " " + val); */

  if (cmd == SER_HANDSHAKE) {
    if (addr == SER_VERSION && val == SER_VERSION) {
      initialized = true;
      reading = true;
      sendCommand(SER_HANDSHAKE, SER_VERSION, SER_VERSION);
    }
  } else if (cmd == SER_HANDSHACK) {
    cur_mode = addr;
    sendCommand(SER_DUMP, 0, 0);
  } else if (cmd == SER_DUMP_START) {
    mode.deselectColor();
    cur_mode = addr;
    reading = true;
  } else if (cmd == SER_DUMP_END) {
    cur_mode = addr;
    reading = false;
    tlTitle.setValue("Mode " + (cur_mode + 1));
  } else if (cmd == SER_DUMP_LIGHT) {
    flashing = false;
    actuallySaveLightFile();
  } else if (cmd < 7) {
    if (flashing) {
      modes[cmd].seta(addr, val);
    } else if (cmd == cur_mode) {
      mode.seta(addr, val);
    }
  }
}


void controlEvent(CallbackEvent theEvent) {
  Controller eController = theEvent.getController();
  String eName = eController.getName();
  int eVal = (int)eController.getValue();
  int eId = eController.getId();
  int eAction = theEvent.getAction();

  if (eController.equals(mode.dlType)) {
    /* if (eAction == ControlP5.ACTION_BROADCAST) { */
    /*   mode.setType(eVal); */
    /*   mode.sendType(); */
    /* } else if (eAction == ControlP5.ACTION_LEAVE) { */
    /*   mode.dlType.close(); */
    /* } */
  } else if (eController.equals(mode.dlPattern)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      if (eVal != mode.pattern) {
        mode.setPattern(eVal);
        mode.sendPattern();
        mode.resetArgsAndTimings();
      } else {
        mode.setPattern(eVal);
        mode.sendPattern();
      }
    } else if (eAction == ControlP5.ACTION_LEAVE) {
      mode.dlPattern.close();
    }
  } else if (eId >= 10200 && eId < 10300) { // slArgs
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setArgs(eId - 10200, eVal);
      mode.sendArgs(eId - 10200);
    }
  } else if (eController.equals(mode.trPatternThresh)) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      mode.setPatternThresh(mode.trPatternThresh.getArrayValue());
      mode.sendPatternThresh();
    }
  } else if (eId >= 10100 && eId < 10200) { // slTimings
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setTimings(eId - 10100, eVal);
      mode.sendTimings(eId - 10100);
    }
  } else if (eController.equals(mode.trColorThresh)) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      mode.setColorThresh(mode.trColorThresh.getArrayValue());
      mode.sendColorThresh();
    }
  } else if (eId >= 10300 && eId < 10400) { // slNumColors
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setNumColors(eId - 10300, eVal);
      mode.sendNumColors(eId - 10300);
    }
  } else if (eId >= 11000 && eId < 11100) { // bColors
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.selectColor(eId - 11000);
      if (!view_mode) {
        viewColor(0);
      }
    }
  } else if (eId >= 20000 && eId < 21000) { // bColorBank
    if (eAction == ControlP5.ACTION_BROADCAST) {
      int s = (eId - 20000) / 100;
      int i = (eId - 20000) % 100;
      int[] c = {COLOR_BANK[i][0] >> s, COLOR_BANK[i][1] >> s, COLOR_BANK[i][2] >> s};
      mode.setColor(mode.color_set, mode.color_slot, c);
      mode.sendColor(mode.color_set, mode.color_slot);
    }
  } else if (eId >= 21000 && eId < 22000) { // slColorValues
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setColors(mode.color_set, mode.color_slot, eId - 21000, eVal);
      mode.sendColors(mode.color_set, mode.color_slot, eId - 21000);
    }
  }
}


void style(ThreshRange thr) {
  thr.getCaptionLabel().toUpperCase(false)
    .getStyle().setPadding(4, 4, 4, 4)
    .setMargin(-4, 0, 0, 0);
}

void style(Button btn, int w) {
  btn.setSize(w, 20)
    .setColorBackground(color(48))
    .setColorForeground(color(96))
    .setColorActive(color(128));
  btn.getCaptionLabel().toUpperCase(false)
    .setColor(color(240))
    .getStyle().setPadding(-1, 0, 0, 0);
}

void style(DropdownList ddl) {
  ddl.getCaptionLabel().toUpperCase(false).getStyle().setPadding(4, 0, 0, 4);
  ddl.getValueLabel().toUpperCase(false).getStyle().setPadding(4, 0, 0, 4);
  ddl.setLabel("")
    .setColorBackground(color(48))
    .setColorForeground(color(96))
    .setColorActive(color(128))
    .setItemHeight(20)
    .setBarHeight(20)
    .close();
}

void style(Slider sld, int _width, int _min, int _max) {
  sld.setBroadcast(false)
    .setTriggerEvent(ControlP5.RELEASE)
    .setSize(_width, 20)
    .setColorBackground(color(48))
    .setColorActive(color(128))
    .setRange(_min, _max)
    .setNumberOfTickMarks(_max - _min + 1)
    .showTickMarks(false)
    .setDecimalPrecision(0)
    .setValue(_min)
    .setBroadcast(true);
}

int translateColor(int r, int g, int b) {
  r = (r == 0) ? 0 : 32 + ((r * 7) / 8);
  g = (g == 0) ? 0 : 32 + ((g * 7) / 8);
  b = (b == 0) ? 0 : 32 + ((b * 7) / 8);
  return (255 << 24) + (r << 16) + (g << 8) + b;
}

int translateColor(int[] c) {
  return translateColor(c[0], c[1], c[2]);
}

int getColorBankColor(int i, int s) {
  return translateColor(COLOR_BANK[i][0] >> s, COLOR_BANK[i][1] >> s, COLOR_BANK[i][2] >> s);
}

void setupMainGroup() {
  gMain = cp5.addGroup("main")
    .setPosition(0, 0)
    .setWidth(1280)
    .setHeight(720)
    .hideBar()
    .hideArrow();

  gTitle = cp5.addGroup("title")
    .setGroup(gMain)
    .setPosition(240, 5)
    .hideBar()
    .hideArrow();
  gTitle = makeTitle(gTitle);

  gControls = cp5.addGroup("controls")
    .setGroup(gMain)
    .setPosition(30, 690)
    .hideBar()
    .hideArrow();
  gControls = makeControls(gControls);

  gMode = cp5.addGroup("mode")
    .setGroup(gMain)
    .setPosition(20, 60)
    .hideBar()
    .hideArrow();
  mode = new Mode(gMode);

  gColorBank = cp5.addGroup("colorBank")
    .setGroup(gMain)
    .setPosition(1010, 0)
    .hideBar()
    .hideArrow();

  for (int g = 0; g < 6; g++) {
    for (int c = 0; c < 8; c++) {
      for (int s = 0; s < 4; s++) {
        bColorBank[(g * 8) + c] = cp5.addButton("ColorBank" + ((g * 8) + c) + "." + s)
          .setGroup(gColorBank)
          .setId(20000 + ((g * 8) + c) + (100 * s))
          .setLabel("")
          .setSize(16, 16)
          .setPosition(24 + 4 + (24 * c), 12 + 4 + (120 * g) + (24 * s))
          .setColorBackground(getColorBankColor((g * 8) + c, s))
          .setColorForeground(getColorBankColor((g * 8) + c, s))
          .setColorActive(getColorBankColor((g * 8) + c, s));
      }
    }
  }
}

void loadColorBank() {
  JSONArray jarr = loadJSONArray("colorbank.json");
  for (int i = 0; i < 48; i++) {
    JSONArray jarr1 = jarr.getJSONArray(i);
    for (int j = 0; j < 3; j++) {
      COLOR_BANK[i][j] = jarr1.getInt(j);
    }
  }
}

Group makeTitle(Group g) {
  g.setSize(320, 40);
  tlTitle = cp5.addTextlabel("tlTitle")
    .setGroup(g)
    .setValue("Mode 1")
    .setFont(createFont("Comfortaa-Regular", 32))
    .setPosition(100, 0)
    .setSize(120, 40)
    .setColorValue(color(240));

  bPrevMode = cp5.addButton("prevMode")
    .setCaptionLabel("<< Prev")
    .setGroup(g)
    .setPosition(0, 10);
  style(bPrevMode, 60);

  bNextMode = cp5.addButton("nextMode")
    .setCaptionLabel("Next >>")
    .setGroup(g)
    .setPosition(260, 10);
  style(bNextMode, 60);

  return g;
}

Group makeControls(Group g) {
  g.setSize(940, 20);
  bResetMode = cp5.addButton("resetMode")
    .setCaptionLabel("Reset Mode")
    .setGroup(g)
    .setPosition(0, 0);
  style(bResetMode, 100);

  bWriteMode = cp5.addButton("writeMode")
    .setCaptionLabel("Write Mode")
    .setGroup(g)
    .setPosition(120, 0);
  style(bWriteMode, 100);

  bSaveMode = cp5.addButton("saveMode")
    .setCaptionLabel("Save Mode")
    .setGroup(g)
    .setPosition(240, 0);
  style(bSaveMode, 100);

  bLoadMode = cp5.addButton("loadMode")
    .setCaptionLabel("Load Mode")
    .setGroup(g)
    .setPosition(360, 0);
  style(bLoadMode, 100);

  bSaveLight = cp5.addButton("saveLight")
    .setCaptionLabel("Save Light")
    .setGroup(g)
    .setPosition(600, 0);
  style(bSaveLight, 100);

  bWriteLight = cp5.addButton("writeLight")
    .setCaptionLabel("Write Light")
    .setGroup(g)
    .setPosition(720, 0);
  style(bWriteLight, 100);

  bDisconnectLight = cp5.addButton("disconnectLight")
    .setCaptionLabel("Disconnect")
    .setGroup(g)
    .setPosition(840, 0);
  style(bDisconnectLight, 100);

  return g;
}

void prevMode(int v) {
  if (!view_mode) viewMode(0);
  sendCommand(SER_CHANGE_MODE, 99);
}

void nextMode(int v) {
  if (!view_mode) viewMode(0);
  sendCommand(SER_CHANGE_MODE, 101);
}

void writeLight() {
  selectInput("Select light file to write to light", "_writeLightFile");
}

void _writeLightFile(File file) {
  if (file == null) {
  } else {
    try {
      flashing = true;
      String path = file.getAbsolutePath();
      JSONArray ja = loadJSONArray(path);
      for (int i = 0; i < 7; i++) {
        modes[i].fromJSON(ja.getJSONObject(i));
        for (int b = 0; b < mode._MODESIZE; b++) {
          sendCommand(SER_WRITE_LIGHT, i, b, modes[i].geta(b));
          delay(2);
        }
      }
      sendCommand(SER_CHANGE_MODE, cur_mode);
      flashing = false;
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}

void saveLight() {
  selectOutput("Select light file to save to", "_saveLightFile");
}

void actuallySaveLightFile() {
  JSONArray ja = new JSONArray();

  for (int i = 0; i < 7; i++) {
    ja.setJSONObject(i, modes[i].getJSON());
  }

  try {
    saveJSONArray(ja, file_path, "compact");
  } catch (Exception ex) {
    // TODO popup error message
  }
}

void _saveLightFile(File file) {
  if (file == null) {
  } else {
    file_path = file.getAbsolutePath();
    if (!file_path.endsWith(".light")) file_path = file_path + ".light";
    flashing = true;
    sendCommand(SER_DUMP_LIGHT);
  }
}

void loadMode() {
  selectInput("Select mode file to load", "_loadModeFile");
}

void _loadModeFile(File file) {
  if (file == null) {
  } else {
    try {
      String path = file.getAbsolutePath();
      mode.fromJSON(loadJSONObject(path));
      for (int b = 0; b < mode._MODESIZE; b++) {
        sendCommand(SER_WRITE, b, mode.geta(b));
      }
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}

void saveMode() {
  selectOutput("Select mode file to save", "_saveModeFile");
}

void _saveModeFile(File file) {
  if (file == null) {
  } else {
    String path = file.getAbsolutePath();
    if (!path.endsWith(".mode")) path = path + ".mode";

    try {
      saveJSONObject(mode.getJSON(), path, "compact");
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}

void writeMode(int v) {
  sendCommand(SER_SAVE);
}

void resetMode(int v) {
  sendCommand(SER_CHANGE_MODE, cur_mode);
}

void disconnectLight(int v) {
  sendCommand(SER_DISCONNECT);
  initialized = false;
  port = null;
}

void viewMode(int v) {
  view_mode = true;
  sendCommand(SER_VIEW_MODE);
  mode.bViewMode.setColorBackground(color(128));
  mode.bViewColor.setColorBackground(color(48));
}

void viewColor(int v) {
  view_mode = false;
  sendCommand(SER_VIEW_COLOR, mode.color_set, mode.color_slot);
  mode.bViewMode.setColorBackground(color(48));
  mode.bViewColor.setColorBackground(color(128));
}
