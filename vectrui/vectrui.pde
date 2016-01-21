import java.awt.event.KeyEvent;
import processing.serial.*;
import controlP5.*;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileNameExtensionFilter;


static final int SER_DUMP       = 10;
static final int SER_SAVE       = 20;
static final int SER_READ       = 30;
static final int SER_WRITE      = 40;
static final int SER_MODE_SET   = 90;
static final int SER_VIEW_MODE  = 100;
static final int SER_VIEW_COLOR = 101;
static final int SER_HANDSHAKE  = 250;
static final int SER_DISCONNECT = 251;

Serial port;
ControlP5 cp5;

int gui_state = 0;
Boolean initialized = false;
Boolean reading = false;
Boolean flashing = false;

int cur_mode = 0;
Mode[] modes = new Mode[7];
Mode[] loaded_modes = new Mode[7];

Editor editor;
int counter = 0;
boolean view_mode = true;


void printColor(int c) {
  int r = (c & 0xff0000) >> 16;
  int g = (c & 0x00ff00) >> 8;
  int b = (c & 0x0000ff) >> 0;
  /* println(r + ", " + g + ", " + b); */
}

void setup() {
  surface.setTitle("VectrUI 01-21-16");
  size(1000, 700);
  cp5 = new ControlP5(this);
  /* cp5.setFont(createFont("Arial-Black", 11)); */
  cp5.setFont(createFont("Comfortaa-Bold", 14));

  editor = new Editor(0, 0);

  for (int i = 0; i < 7; i++) {
    modes[i] = new Mode();
    loaded_modes[i] = new Mode();
  }
}

void connectLight() {
  for (String p: Serial.list()) {
    try {
      port = new Serial(this, p, 115200);
    } catch (Exception e) {
    }
  }
}

void draw() {
  background(192);
  if (!initialized) {
    connectLight();
  }

  while (port.available() >= 3) {
    readCommand();
  }

  if (!initialized || reading || flashing) {
    editor.group.hide();
  } else {
    editor.group.show();
  }
}

void readCommand() {
  int target = port.read();
  int addr = port.read();
  int val = port.read();
  /* println("in << " + target + " " + addr + " " + val); */

  if (target == SER_HANDSHAKE) {        // Light wants to connect
    if (addr == 100 && val == 100) {
      initialized = true;
      reading = true;
      sendCommand(SER_HANDSHAKE, 100, 13, 13);
    }
  } else if (target == 251) { // Light acked handshake
    cur_mode = addr;
    sendCommand(SER_DUMP, 200, 0, 0);
  } else if (target == 200) { // Start of a dump
    // addr is the mode about to be dumped, val is the current mode
    cur_mode = val;
    reading = true;
    if (flashing) {
    }
  } else if (target == 210) { // End of a dump
    // addr is the mode just dumped, val is the current mode
    cur_mode = val;
    editor.curModeChanged(cur_mode);
    reading = false;
    if (flashing) {
      sendMode(cur_mode);
      cur_mode++;
      if (cur_mode == 7) {
        flashing = false;
        sendCommand(SER_MODE_SET, 0, 0, 0);
      } else {
        sendCommand(SER_MODE_SET, cur_mode, 0, 0);
      }
    }
  } else if (target < 7) {    // Data on a mode
    modes[target].seta(addr, val);
  } else if (target == 100) {
    modes[cur_mode].seta(addr, val);
    editor.seta(addr, val);
  }
}

void writeToLight(int target, int addr, int val) {
  sendCommand(SER_WRITE, target, addr, val);

  /* int rtarget = port.read(); */
  /* int raddr = port.read(); */
  /* int rval = port.read(); */

  /* if (addr != raddr || val != rval) { */
  /*   print("light didn't listen "); */
  /*   println(rtarget + " " + raddr + " " + rval); */
  /* } else { */
    if (target == 100) {
      modes[cur_mode].seta(addr, val);
    } else if (target < 7) {
      modes[target].seta(addr, val);
    }
  /* } */
}

void sendCommand(int cmd, int target, int addr, int val) {
  /* println("out>>" + cmd + " " + target + " " + addr + " " + val); */

  if (initialized) {
    port.write(cmd);
    port.write(target);
    port.write(addr);
    port.write(val);
  }
}

void controlEvent(CallbackEvent theEvent) {
  Controller eController = theEvent.getController();
  String eName = eController.getName();
  float eVal = eController.getValue();
  int eId = eController.getId();
  int eAction = theEvent.getAction();

  if (eController.equals(editor.patternThresh)) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, 4, (int)editor.patternThresh.getMinA());
      sendCommand(SER_WRITE, 100, 5, (int)editor.patternThresh.getMaxA());
      sendCommand(SER_WRITE, 100, 6, (int)editor.patternThresh.getMinB());
      sendCommand(SER_WRITE, 100, 7, (int)editor.patternThresh.getMaxB());
    }
  } else if (eController.equals(editor.colorThresh)) {
    if (eAction == ControlP5.ACTION_RELEASED || eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, 8, (int)editor.colorThresh.getMinA());
      sendCommand(SER_WRITE, 100, 9, (int)editor.colorThresh.getMaxA());
      sendCommand(SER_WRITE, 100, 10, (int)editor.colorThresh.getMinB());
      sendCommand(SER_WRITE, 100, 11, (int)editor.colorThresh.getMaxB());
    }
  } else if (eController.equals(editor.base)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      sendPatternChange((int)eVal);
      editor.patternChanged((int)eVal);
      editor.curModeChanged(cur_mode);
    } else if (eAction == ControlP5.ACTION_LEAVE) {
      editor.base.close();
    }
  } else if (eController.equals(editor.prevMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      view_mode = true;
      editor.viewMode.setColorBackground(color(0, 90, 180));
      editor.viewColor.setColorBackground(color(0, 45, 90));
      sendCommand(SER_VIEW_MODE, 0, 0, 0);
      sendCommand(SER_MODE_SET, 99, 0, 0);
    }
  } else if (eController.equals(editor.nextMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      view_mode = true;
      editor.viewMode.setColorBackground(color(0, 90, 180));
      editor.viewColor.setColorBackground(color(0, 45, 90));
      sendCommand(SER_VIEW_MODE, 0, 0, 0);
      sendCommand(SER_MODE_SET, 101, 0, 0);
    }
  } else if (eController.equals(editor.saveMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      saveModeFile();
    }
  } else if (eController.equals(editor.loadMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      openModeFile();
    }
  } else if (eController.equals(editor.writeMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_SAVE, 0, 0, 0);
    }
  } else if (eController.equals(editor.resetMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_MODE_SET, cur_mode, 0, 0);
    }
  } else if (eController.equals(editor.saveLight)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      saveLightFile();
    }
  } else if (eController.equals(editor.writeLight)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      openLightFile();
    }
  } else if (eController.equals(editor.disconnectLight)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_DISCONNECT, 0, 0, 0);
      initialized = false;
    }
  } else if (eController.equals(editor.viewMode)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      if (!view_mode) {
        sendCommand(SER_VIEW_MODE, 0, 0, 0);
        view_mode = true;
        editor.viewMode.setColorBackground(color(0, 90, 180));
        editor.viewColor.setColorBackground(color(0, 45, 90));
      }
    }
  } else if (eController.equals(editor.viewColor)) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      if (view_mode) {
        sendCommand(SER_VIEW_COLOR, editor.color_set, editor.color_slot, 0);
        view_mode = false;
        editor.viewMode.setColorBackground(color(0, 45, 90));
        editor.viewColor.setColorBackground(color(0, 90, 180));
      }
    }
  } else if (eName.startsWith("editorArgs")) {
    if (eAction == ControlP5.ACTION_RELEASED ||
        eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    } else if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    }
  } else if (eName.startsWith("editorTimings")) {
    if (eAction == ControlP5.ACTION_RELEASED ||
        eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    } else if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    }
  } else if (eName.startsWith("editorNumColors")) {
    if (eAction == ControlP5.ACTION_RELEASED ||
        eAction == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
      editor.numColorsChanged(eId - 1, (int)eVal);
    } else if (eAction == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
      editor.numColorsChanged(eId - 1, (int)eVal);
    }
  } else if (eId == 1000) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      editor.selectColor(eVal);
      if (!view_mode) {
        sendCommand(SER_VIEW_COLOR, editor.color_set, editor.color_slot, 0);
      }
    }
  } else if (eName.startsWith("editorColorValues")) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      if (editor.color_set >= 0 && editor.color_slot >= 0) {
        sendCommand(SER_WRITE, 100, 33 + (editor.color_slot * 9) + (editor.color_set * 3) + (eId - 500), (int)eVal);
        editor.seta(33 + (editor.color_slot * 9) + (editor.color_set * 3) + (eId - 500), (int)eVal);
      }
    }
  } else if (eId >= 2000 && eId < 2100) {
    if (eAction == ControlP5.ACTION_BROADCAST) {
      if (editor.color_set >= 0 && editor.color_slot >= 0) {
        sendCommand(SER_WRITE, 100, 33 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][0]);
        sendCommand(SER_WRITE, 100, 34 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][1]);
        sendCommand(SER_WRITE, 100, 35 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][2]);
        editor.seta(33 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][0]);
        editor.seta(34 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][1]);
        editor.seta(35 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][2]);
        editor.selectColor(editor.color_set, editor.color_slot);
      }
    }
  }
}

void sendMode(int m) {
  Mode mode = (m == 100) ? modes[cur_mode] : loaded_modes[m];
  writeToLight(m, 0, mode.pattern);
  for (int i = 0; i < 3; i++) {
    writeToLight(m, i + 1, mode.numColors[i]);
  }
  for (int i = 0; i < 4; i++) {
    writeToLight(m, i + 4, mode.patternThresh[i / 2][i % 2]);
  }
  for (int i = 0; i < 4; i++) {
    writeToLight(m, i + 8, mode.colorThresh[i / 2][i % 2]);
  }
  for (int i = 0; i < 3; i++) {
    writeToLight(m, i + 12, mode.args[i]);
  }
  for (int i = 0; i < 18; i++) {
    writeToLight(m, i + 15, mode.timings[i / 6][i % 6]);
  }
  for (int i = 0; i < 81; i++) {
    writeToLight(m, i + 33, mode.colors[i / 9][(i % 9) / 3][i % 3]);
  }
}

void sendPatternChange(int v) {
  sendCommand(SER_WRITE, 100, 0, v);
  // Set all args to 0
  for (int i = 0; i < 3; i++) { sendCommand(SER_WRITE, 100, 12 + i, 0); }

  if (v == 0) { // Strobe
    sendCommand(SER_WRITE, 100, 15, 9);
    sendCommand(SER_WRITE, 100, 16, 41);
    sendCommand(SER_WRITE, 100, 17, 0);
    sendCommand(SER_WRITE, 100, 18, 0);
    sendCommand(SER_WRITE, 100, 19, 0);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 25);
    sendCommand(SER_WRITE, 100, 22, 25);
    sendCommand(SER_WRITE, 100, 23, 0);
    sendCommand(SER_WRITE, 100, 24, 0);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 3);
    sendCommand(SER_WRITE, 100, 28, 22);
    sendCommand(SER_WRITE, 100, 29, 0);
    sendCommand(SER_WRITE, 100, 30, 0);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);
  } else if (v == 1) { // Vexer
    sendCommand(SER_WRITE, 100, 15, 9);
    sendCommand(SER_WRITE, 100, 16, 0);
    sendCommand(SER_WRITE, 100, 17, 41);
    sendCommand(SER_WRITE, 100, 18, 0);
    sendCommand(SER_WRITE, 100, 19, 0);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 5);
    sendCommand(SER_WRITE, 100, 22, 0);
    sendCommand(SER_WRITE, 100, 23, 45);
    sendCommand(SER_WRITE, 100, 24, 0);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 3);
    sendCommand(SER_WRITE, 100, 28, 0);
    sendCommand(SER_WRITE, 100, 29, 47);
    sendCommand(SER_WRITE, 100, 30, 0);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);
  } else if (v == 2) { // Edge
    sendCommand(SER_WRITE, 100, 15, 3);
    sendCommand(SER_WRITE, 100, 16, 0);
    sendCommand(SER_WRITE, 100, 17, 8);
    sendCommand(SER_WRITE, 100, 18, 50);
    sendCommand(SER_WRITE, 100, 19, 0);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 2);
    sendCommand(SER_WRITE, 100, 22, 0);
    sendCommand(SER_WRITE, 100, 23, 8);
    sendCommand(SER_WRITE, 100, 24, 50);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 1);
    sendCommand(SER_WRITE, 100, 28, 0);
    sendCommand(SER_WRITE, 100, 29, 8);
    sendCommand(SER_WRITE, 100, 30, 50);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);
  } else if (v == 3) { // Double
    sendCommand(SER_WRITE, 100, 15, 25);
    sendCommand(SER_WRITE, 100, 16, 0);
    sendCommand(SER_WRITE, 100, 17, 25);
    sendCommand(SER_WRITE, 100, 18, 0);
    sendCommand(SER_WRITE, 100, 19, 25);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 25);
    sendCommand(SER_WRITE, 100, 22, 0);
    sendCommand(SER_WRITE, 100, 23, 5);
    sendCommand(SER_WRITE, 100, 24, 0);
    sendCommand(SER_WRITE, 100, 25, 25);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 5);
    sendCommand(SER_WRITE, 100, 28, 0);
    sendCommand(SER_WRITE, 100, 29, 5);
    sendCommand(SER_WRITE, 100, 30, 0);
    sendCommand(SER_WRITE, 100, 31, 25);
    sendCommand(SER_WRITE, 100, 32, 0);
  } else if (v == 4) { // Runner
    sendCommand(SER_WRITE, 100, 15, 3);
    sendCommand(SER_WRITE, 100, 16, 22);
    sendCommand(SER_WRITE, 100, 17, 3);
    sendCommand(SER_WRITE, 100, 18, 22);
    sendCommand(SER_WRITE, 100, 19, 25);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 25);
    sendCommand(SER_WRITE, 100, 22, 25);
    sendCommand(SER_WRITE, 100, 23, 3);
    sendCommand(SER_WRITE, 100, 24, 22);
    sendCommand(SER_WRITE, 100, 25, 25);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 3);
    sendCommand(SER_WRITE, 100, 28, 22);
    sendCommand(SER_WRITE, 100, 29, 25);
    sendCommand(SER_WRITE, 100, 30, 25);
    sendCommand(SER_WRITE, 100, 31, 25);
    sendCommand(SER_WRITE, 100, 32, 0);
  } else if (v == 5) { // Stepper
    sendCommand(SER_WRITE, 100, 15, 25);
    sendCommand(SER_WRITE, 100, 16, 5);
    sendCommand(SER_WRITE, 100, 17, 10);
    sendCommand(SER_WRITE, 100, 18, 15);
    sendCommand(SER_WRITE, 100, 19, 20);
    sendCommand(SER_WRITE, 100, 20, 25);

    sendCommand(SER_WRITE, 100, 21, 25);
    sendCommand(SER_WRITE, 100, 22, 5);
    sendCommand(SER_WRITE, 100, 23, 10);
    sendCommand(SER_WRITE, 100, 24, 15);
    sendCommand(SER_WRITE, 100, 25, 20);
    sendCommand(SER_WRITE, 100, 26, 25);

    sendCommand(SER_WRITE, 100, 27, 25);
    sendCommand(SER_WRITE, 100, 28, 5);
    sendCommand(SER_WRITE, 100, 29, 10);
    sendCommand(SER_WRITE, 100, 30, 15);
    sendCommand(SER_WRITE, 100, 31, 20);
    sendCommand(SER_WRITE, 100, 32, 25);
  } else if (v == 6) { // Random
    sendCommand(SER_WRITE, 100, 15, 1);
    sendCommand(SER_WRITE, 100, 16, 5);
    sendCommand(SER_WRITE, 100, 17, 10);
    sendCommand(SER_WRITE, 100, 18, 20);
    sendCommand(SER_WRITE, 100, 19, 0);
    sendCommand(SER_WRITE, 100, 20, 0);

    sendCommand(SER_WRITE, 100, 21, 1);
    sendCommand(SER_WRITE, 100, 22, 5);
    sendCommand(SER_WRITE, 100, 23, 10);
    sendCommand(SER_WRITE, 100, 24, 20);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 1);
    sendCommand(SER_WRITE, 100, 28, 5);
    sendCommand(SER_WRITE, 100, 29, 10);
    sendCommand(SER_WRITE, 100, 30, 20);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);
  }
}


void openLightFile() {
  selectInput("Select light file to flash to light", "_openLightFile");
}

void _openLightFile(File file) {
  if (file == null) {
  } else {
    try {
      String path = file.getAbsolutePath();
      JSONArray jarr = loadJSONArray(path);
      for (int i = 0; i < 7; i++) {
        loaded_modes[i].fromJSON(jarr.getJSONObject(i));
      }
      flashing = true;
      sendCommand(SER_MODE_SET, 0, 0, 0);
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}


void saveLightFile() {
  selectOutput("Select light file to save to", "_saveLightFile");
}

void _saveLightFile(File file) {
  if (file == null) {
  } else {
    try {
      String path = file.getAbsolutePath();
      if (!path.endsWith(".light")) {
        path += ".light";
      }
      JSONArray jarr = new JSONArray();
      for (int i = 0; i < 7; i++) {
        jarr.setJSONObject(i, modes[i].asJSON());
      }
      saveJSONArray(jarr, path, "compact");
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}

void openModeFile() {
  selectInput("Select mode file to load", "_openModeFile");
}

void _openModeFile(File file) {
  if (file == null) {
  } else {
    try {
      String path = file.getAbsolutePath();
      modes[cur_mode].fromJSON(loadJSONObject(path));
      sendMode(100);
      editor.curModeChanged(cur_mode);
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}

void saveModeFile() {
  selectOutput("Select mode file to save", "_saveModeFile");
}

void _saveModeFile(File file) {
  if (file == null) {
  } else {
    try {
      String path = file.getAbsolutePath();
      if (!path.endsWith(".mode")) {
        path += ".mode";
      }
      saveJSONObject(modes[cur_mode].asJSON(), path, "compact");
    } catch (Exception ex) {
      // TODO popup error message
    }
  }
}
