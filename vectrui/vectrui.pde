import java.awt.event.KeyEvent;
import processing.serial.*;
import controlP5.*;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileNameExtensionFilter;

ControlP5 cp5;

// Main
static final int ID_TYPE              = 100;
static final int ID_COLOREDIT         = 300;
static final int ID_COLORBANK         = 5000;

// Shared
static final int ID_PATTERN          = 1000;
static final int ID_ARG              = 1100;
static final int ID_TIMING           = 1200;
static final int ID_NUMCOLORS        = 1300;
static final int ID_COLORS           = 1400;

// Vectr Only
static final int ID_PATTERN_TRESH    = 1900;
static final int ID_COLOR_TRESH      = 1901;

// Primer Only
static final int ID_TRIGGER_MODE     = 2900;
static final int ID_TRIGGER_TRESH    = 2901;

final static String[] PATTERNS = {
  "Strobe",
  "Vexer",
  "Edge",
  "Multi",
  "Runner",
  "Stepper",
  "Random",
};
final static String[] TRIGGERMODES = {"Off", "Velocity", "Tilt", "Roll", "Flip"};
final static String[] MODETYPES = {"Vectr", "Primr"};

int COLOR_BANK[][] = {
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
  {0, 0, 0},
  {56, 64, 72},
  {24, 0, 0},
  {16, 16, 0},
  {0, 24, 0},
  {0, 16, 16},
  {0, 0, 24},
  {16, 0, 16},
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
  {80, 8, 80}
};

static final int SER_VERSION = 101;
static final int SER_DUMP           = 10;
static final int SER_DUMP_LIGHT     = 11;
static final int SER_SAVE           = 20;
static final int SER_READ           = 30;
static final int SER_WRITE          = 40;
static final int SER_WRITE_LIGHT    = 41;
static final int SER_WRITE_MODE     = 42;
static final int SER_WRITE_MODE_END = 43;
static final int SER_CHANGE_MODE    = 50;
static final int SER_RESET_MODE     = 51;
static final int SER_VIEW_MODE      = 100;
static final int SER_VIEW_COLOR     = 110;
static final int SER_DUMP_START     = 200;
static final int SER_DUMP_END       = 210;
static final int SER_HANDSHAKE      = 250;
static final int SER_HANDSHACK      = 251;
static final int SER_DISCONNECT     = 254;

int num_ports = 0;
Serial ports[] = new Serial[10];
Serial port = null;

// State variables
boolean connected_serial = false;
boolean initialized = false;
boolean reading = false;
boolean flashing = false;
boolean view_mode = true;

// Light variables
int cur_mode = 0;

// All the groups
Group gMain;
Group gSerial;

Textlabel tlSerial;
String[] sSerial = new String[4];
Button[] bSerial = new Button[4];


Mode mode;
Mode[] modes = new Mode[7];

String file_path = "";


void setup() {
  surface.setTitle("VectrUI 02-09-2016");
  smooth(8);
  frameRate(60);
  size(1040, 720);
  loadColorBank();

  cp5 = new ControlP5(this);
  cp5.setFont(createFont("Comfortaa-Bold", 14));

  gMain = cp5.addGroup("main")
    .setPosition(0, 0)
    .setWidth(1040)
    .setHeight(720)
    .hideBar()
    .hideArrow();
  mode = new Mode(gMain);

  gSerial = cp5.addGroup("serial")
    .setPosition(0, 0)
    .setWidth(1040)
    .setHeight(720)
    .hideBar()
    .hideArrow();

  cp5.addTextlabel("tlWelcome")
    .setGroup(gSerial)
    .setValue("Welcome to VectR!")
    .setFont(createFont("Comfortaa-Regular", 48))
    .setPosition(300, 100)
    .setSize(120, 40)
    .setColorValue(color(192, 192, 255));

  tlSerial = cp5.addTextlabel("tlSerial")
    .setGroup(gSerial)
    .setValue("Pick a serial port:")
    .setFont(createFont("Comfortaa-Regular", 32))
    .setPosition(380, 250)
    .setSize(120, 40)
    .setColorValue(color(240));

  for (int i = 0; i < 4; i++) {
    bSerial[i] = cp5.addButton("Serial" + i)
      .setCaptionLabel("")
      .setGroup(gSerial)
      .setId(10 + i)
      .setPosition(370, 300 + (30 * i))
      .hide();
    style(bSerial[i], 300);
  }

  for (int i = 0; i < 7; i++) {
    modes[i] = new Mode();
  }
}

void draw() {
  background(16);
  if (!connected_serial) {
    for (String p: Serial.list()) {
      if (num_ports < 4 && !p.contains("Bluetooth")) {
        try {
          bSerial[num_ports].hide();
          ports[num_ports] = new Serial(this, p, 115200);
          sSerial[num_ports] = p;
          bSerial[num_ports].setCaptionLabel(p).show();
          println("Found serial port " + p + " #" + num_ports + ": " + ports[num_ports]);
          num_ports++;
        } catch (Exception e) {
        }
      }
    }
    gSerial.show();
    gMain.hide();
  } else if (!initialized) {
    if (port.available() >= 3) {
      readCommand();
    }
    gSerial.hide();
    gMain.hide();
  } else {
    while (port.available() >= 3) {
      readCommand();
    }
    if (reading || flashing) {
      gMain.hide();
    } else {
      gMain.show();
    }
    gSerial.hide();
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
  println("send " + cmd + " " + in0 + " " + in1 + " " + in2);
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
  println("get " + cmd + " " + addr + " " + val);

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
    mode.tlTitle.setValue("Mode " + (cur_mode + 1));
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

void loadColorBank() {
  try {
    JSONArray jarr = loadJSONArray("colorbank.json");
    for (int i = 0; i < 48; i++) {
      JSONArray jarr1 = jarr.getJSONArray(i);
      for (int j = 0; j < 3; j++) {
        COLOR_BANK[i][j] = jarr1.getInt(j);
      }
    }
  } catch (Exception ex) {
    println("SHIT! Loading colorbank.json failed! Falling back on default. " + ex);
  }
}


//********************************************************************************
// Button actions
//********************************************************************************
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
      println("SHIT Write light failed! " + ex);
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
    println("SHIT! Save light failed! " + ex);
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
      println("SHIT! Load mode failed! " + ex);
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
      println("SHIT! Save mode failed! " + ex);
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

  connected_serial = false;
  initialized = false;
  cur_mode = 0;

  for (int i = 0; i < 4; i++) {
    sSerial[i] = "";
    bSerial[i].setCaptionLabel("").hide();
    if (i < num_ports) {
      ports[i] = null;
    }
  }
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
  sendCommand(SER_VIEW_COLOR, mode.getColorSet(), mode.getColorSlot());
  mode.bViewMode.setColorBackground(color(48));
  mode.bViewColor.setColorBackground(color(128));
}


void controlEvent(CallbackEvent theEvent) {
  Controller eController = theEvent.getController();
  String eName = eController.getName();
  int eVal = (int)eController.getValue();
  int eId = eController.getId();
  int eAction = theEvent.getAction();

  if (eId == ID_TYPE) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setType(eVal);
    } else if (eAction == ControlP5.ACTION_LEAVE) {
      mode.dlType.close();
    }
  } else if (eId >= ID_PATTERN && eId < ID_PATTERN + 2) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setPattern(eId - ID_PATTERN, eVal);
    } else if (eAction == ControlP5.ACTION_LEAVE) {
      mode.closeDropdowns();
    }
  } else if (eId == ID_TRIGGER_MODE) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setTriggerMode(eVal);
      mode.sendTriggerMode();
    } else if (eAction == ControlP5.ACTION_LEAVE) {
      mode.pmode.dlTriggerMode.close();
    }
  } else if (eId == ID_PATTERN_TRESH) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      mode.setPatternThresh(eController.getArrayValue());
      mode.sendPatternThresh();
    }
  } else if (eId == ID_COLOR_TRESH) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      mode.setColorThresh(eController.getArrayValue());
      mode.sendColorThresh();
    }
  } else if (eId == ID_TRIGGER_TRESH) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      mode.setTriggerThresh(eController.getArrayValue());
      mode.sendTriggerThresh();
    }
  } else if (eId >= ID_ARG && eId < ID_ARG + 15) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setArgs(eId - ID_ARG, eVal);
      mode.sendArgs(eId - ID_ARG);
    }
  } else if (eId >= ID_TIMING && eId < ID_TIMING + 24) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setTimings(eId - ID_TIMING, eVal);
      mode.sendTimings(eId - ID_TIMING);
    }
  } else if (eId >= ID_NUMCOLORS && eId < ID_NUMCOLORS + 3) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setNumColors(eId - ID_NUMCOLORS, eVal);
      mode.sendNumColors(eId - ID_NUMCOLORS);
    }
  } else if (eId >= ID_COLORS && eId < ID_COLORS + 27) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.selectColor(eId - ID_COLORS);
      if (!view_mode) {
        viewColor(0);
      }
    }
  } else if (eId >= ID_COLOREDIT && eId < ID_COLOREDIT + 3) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      mode.setColor(mode.getColorSet(), mode.getColorSlot(), eId - ID_COLOREDIT, eVal);
      mode.sendColor(mode.getColorSet(), mode.getColorSlot(), eId - ID_COLOREDIT);
    }
  } else if (eId >= ID_COLORBANK) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      int s = (eId - ID_COLORBANK) / 100;
      int i = (eId - ID_COLORBANK) % 100;
      int[] c = {COLOR_BANK[i][0] >> s, COLOR_BANK[i][1] >> s, COLOR_BANK[i][2] >> s};
      mode.setColor(mode.getColorSet(), mode.getColorSlot(), c);
      mode.sendColor(mode.getColorSet(), mode.getColorSlot());
    }
  } else if (eName.startsWith("Serial")) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      println("Connect to port " + (10 - eId));
      port = ports[eId - 10];
      connected_serial = true;
    }
  }
}


boolean oob(float x, float _min, float _max) {
  return x < _min || x > _max;
}

boolean oob(int x, int _min, int _max) {
  return x < _min || x > _max;
}
