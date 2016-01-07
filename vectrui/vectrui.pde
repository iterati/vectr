import java.awt.event.KeyEvent;
import processing.serial.*;
import controlP5.*;

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

int cur_mode = 0;
Mode[] modes = new Mode[7];

Editor editor;
int counter = 0;
boolean view_mode = true;


void printColor(int c) {
  int r = (c & 0xff0000) >> 16;
  int g = (c & 0x00ff00) >> 8;
  int b = (c & 0x0000ff) >> 0;
  println(r + ", " + g + ", " + b);
}

void setup() {
  surface.setTitle("VectrUI");
  size(1000, 800);
  cp5 = new ControlP5(this);
  /* cp5.setFont(createFont("Verdana", 10)); */
  cp5.setFont(createFont("Arial-Black", 11));

  editor = new Editor(0, 0);

  for (int i = 0; i < 7; i++) {
    modes[i] = new Mode();
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

  if (!initialized || reading) {
    editor.group.hide();
  } else {
    editor.group.show();
  }
}

void readCommand() {
  int target = port.read();
  int addr = port.read();
  int val = port.read();

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
    reading = true;
  } else if (target == 210) { // End of a dump
    // addr is the mode just dumped, val is the current mode
    cur_mode = val;
    editor.curModeChanged(cur_mode);
    reading = false;
  } else if (target < 7) {    // Data on a mode
    modes[target].seta(addr, val);
  }
}

void sendCommand(int cmd, int target, int addr, int val) {
  /* println("cmd: " + cmd + " " + target + " " + addr + " " + val); */
  if (cmd == SER_WRITE) {
    if (target == 100) {
      modes[cur_mode].seta(addr, val);
    } else if (target < 7) {
      modes[target].seta(addr, val);
    }
  }

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
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendPatternChange((int)eVal);
      editor.patternChanged((int)eVal);
      editor.curModeChanged(cur_mode);
    } else if (theEvent.getAction() == ControlP5.ACTION_LEAVE) {
      editor.base.close();
    }
  } else if (eController.equals(editor.prevMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      view_mode = true;
      editor.viewMode.setColorBackground(color(0, 90, 180));
      editor.viewColor.setColorBackground(color(0, 45, 90));
      sendCommand(SER_VIEW_MODE, 0, 0, 0);
      sendCommand(SER_MODE_SET, 99, 0, 0);
    }
  } else if (eController.equals(editor.nextMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      view_mode = true;
      editor.viewMode.setColorBackground(color(0, 90, 180));
      editor.viewColor.setColorBackground(color(0, 45, 90));
      sendCommand(SER_VIEW_MODE, 0, 0, 0);
      sendCommand(SER_MODE_SET, 101, 0, 0);
    }
  } else if (eController.equals(editor.saveMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      saveJSONObject(modes[cur_mode].asJSON(), editor.modeFilename.getText(), "compact");
    }
  } else if (eController.equals(editor.loadMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      modes[cur_mode].fromJSON(loadJSONObject(editor.modeFilename.getText()));
      sendMode(100);
      editor.curModeChanged(cur_mode);
    }
  } else if (eController.equals(editor.writeMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_SAVE, 0, 0, 0);
    }
  } else if (eController.equals(editor.resetMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_MODE_SET, cur_mode, 0, 0);
    }
  } else if (eController.equals(editor.saveLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      JSONArray jarr = new JSONArray();
      for (int i = 0; i < 7; i++) {
        jarr.setJSONObject(i, modes[i].asJSON());
      }
      saveJSONArray(jarr, editor.lightFilename.getText(), "compact");
    }
  } else if (eController.equals(editor.writeLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      JSONArray jarr = loadJSONArray(editor.lightFilename.getText());
      for (int i = 0; i < 7; i++) {
        modes[i].fromJSON(jarr.getJSONObject(i));
        sendMode(i);
      }
      editor.curModeChanged(cur_mode);
    }
  } else if (eController.equals(editor.disconnectLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_DISCONNECT, 0, 0, 0);
      initialized = false;
    }
  } else if (eController.equals(editor.viewMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      if (!view_mode) {
        sendCommand(SER_VIEW_MODE, 0, 0, 0);
        view_mode = true;
        editor.viewMode.setColorBackground(color(0, 90, 180));
        editor.viewColor.setColorBackground(color(0, 45, 90));
      }
    }
  } else if (eController.equals(editor.viewColor)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      if (view_mode) {
        sendCommand(SER_VIEW_COLOR, editor.color_set, editor.color_slot, 0);
        view_mode = false;
        editor.viewMode.setColorBackground(color(0, 45, 90));
        editor.viewColor.setColorBackground(color(0, 90, 180));
      }
    }
  } else if (eName.startsWith("editorArgs")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    }
  } else if (eName.startsWith("editorTimings")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
    }
  } else if (eName.startsWith("editorNumColors")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, eId, (int)eVal);
      editor.numColorsChanged(eId - 1, (int)eVal);
    } else if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      editor.numColorsChanged(eId - 1, (int)eVal);
    }
  } else if (eId == 1000) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      editor.selectColor(eVal);
      if (!view_mode) {
        sendCommand(SER_VIEW_COLOR, editor.color_set, editor.color_slot, 0);
      }
    }
  } else if (eName.startsWith("editorColorValues")) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      if (editor.color_set >= 0 && editor.color_slot >= 0) {
        sendCommand(SER_WRITE, 100, 39 + (editor.color_slot * 9) + (editor.color_set * 3) + (eId - 500), (int)eVal);
        editor.seta(39 + (editor.color_slot * 9) + (editor.color_set * 3) + (eId - 500), (int)eVal);
      }
    }
  } else if (eId >= 2000 && eId < 2100) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      if (editor.color_set >= 0 && editor.color_slot >= 0) {
        sendCommand(SER_WRITE, 100, 39 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][0]);
        sendCommand(SER_WRITE, 100, 40 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][1]);
        sendCommand(SER_WRITE, 100, 41 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][2]);
        editor.seta(39 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][0]);
        editor.seta(40 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][1]);
        editor.seta(41 + (editor.color_slot * 9) + (editor.color_set * 3), color_bank[eId - 2000][2]);
        editor.selectColor(editor.color_set, editor.color_slot);
      }
    }
  }
}

void sendMode(int m) {
  int mode = (m == 100) ? cur_mode : m;
  sendCommand(SER_WRITE, m, 0, modes[mode].pattern);
  for (int i = 0; i < 3; i++) {
    sendCommand(SER_WRITE, m, i + 1, modes[mode].numColors[i]);
  }
  for (int i = 0; i < 4; i++) {
    sendCommand(SER_WRITE, m, i + 4, modes[mode].patternThresh[i / 2][i % 2]);
  }
  for (int i = 0; i < 4; i++) {
    sendCommand(SER_WRITE, m, i + 8, modes[mode].colorThresh[i / 2][i % 2]);
  }
  for (int i = 0; i < 9; i++) {
    sendCommand(SER_WRITE, m, i + 12, modes[mode].args[i / 3][i % 3]);
  }
  for (int i = 0; i < 18; i++) {
    sendCommand(SER_WRITE, m, i + 21, modes[mode].timings[i / 6][i % 6]);
  }
  for (int i = 0; i < 81; i++) {
    sendCommand(SER_WRITE, m, i + 39, modes[mode].colors[i / 9][(i % 9) / 3][i % 3]);
  }
}

void sendPatternChange(int v) {
  sendCommand(SER_WRITE, 100, 0, v);
  // Set all args to 0
  for (int i = 0; i < 9; i++) { sendCommand(SER_WRITE, 100, 12 + i, 0); }

  if (v == 0) { // Strobe
    sendCommand(SER_WRITE, 100, 21, 9);
    sendCommand(SER_WRITE, 100, 22, 41);
    sendCommand(SER_WRITE, 100, 23, 0);
    sendCommand(SER_WRITE, 100, 24, 0);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 25);
    sendCommand(SER_WRITE, 100, 28, 25);
    sendCommand(SER_WRITE, 100, 29, 0);
    sendCommand(SER_WRITE, 100, 30, 0);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);

    sendCommand(SER_WRITE, 100, 33, 3);
    sendCommand(SER_WRITE, 100, 34, 22);
    sendCommand(SER_WRITE, 100, 35, 0);
    sendCommand(SER_WRITE, 100, 36, 0);
    sendCommand(SER_WRITE, 100, 37, 0);
    sendCommand(SER_WRITE, 100, 38, 0);
  } else if (v == 1) { // Vexer
    sendCommand(SER_WRITE, 100, 21, 9);
    sendCommand(SER_WRITE, 100, 22, 0);
    sendCommand(SER_WRITE, 100, 23, 41);
    sendCommand(SER_WRITE, 100, 24, 0);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 5);
    sendCommand(SER_WRITE, 100, 28, 0);
    sendCommand(SER_WRITE, 100, 29, 45);
    sendCommand(SER_WRITE, 100, 30, 0);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);

    sendCommand(SER_WRITE, 100, 33, 3);
    sendCommand(SER_WRITE, 100, 34, 0);
    sendCommand(SER_WRITE, 100, 35, 47);
    sendCommand(SER_WRITE, 100, 36, 0);
    sendCommand(SER_WRITE, 100, 37, 0);
    sendCommand(SER_WRITE, 100, 38, 0);
  } else if (v == 2) { // Edge
    sendCommand(SER_WRITE, 100, 21, 3);
    sendCommand(SER_WRITE, 100, 22, 0);
    sendCommand(SER_WRITE, 100, 23, 8);
    sendCommand(SER_WRITE, 100, 24, 50);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 2);
    sendCommand(SER_WRITE, 100, 28, 0);
    sendCommand(SER_WRITE, 100, 29, 8);
    sendCommand(SER_WRITE, 100, 30, 50);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);

    sendCommand(SER_WRITE, 100, 33, 1);
    sendCommand(SER_WRITE, 100, 34, 0);
    sendCommand(SER_WRITE, 100, 35, 8);
    sendCommand(SER_WRITE, 100, 36, 50);
    sendCommand(SER_WRITE, 100, 37, 0);
    sendCommand(SER_WRITE, 100, 38, 0);
  } else if (v == 3) { // Double
    sendCommand(SER_WRITE, 100, 21, 9);
    sendCommand(SER_WRITE, 100, 22, 41);
    sendCommand(SER_WRITE, 100, 23, 41);
    sendCommand(SER_WRITE, 100, 24, 9);
    sendCommand(SER_WRITE, 100, 25, 0);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 25);
    sendCommand(SER_WRITE, 100, 28, 25);
    sendCommand(SER_WRITE, 100, 29, 25);
    sendCommand(SER_WRITE, 100, 30, 25);
    sendCommand(SER_WRITE, 100, 31, 0);
    sendCommand(SER_WRITE, 100, 32, 0);

    sendCommand(SER_WRITE, 100, 33, 3);
    sendCommand(SER_WRITE, 100, 34, 22);
    sendCommand(SER_WRITE, 100, 35, 3);
    sendCommand(SER_WRITE, 100, 36, 22);
    sendCommand(SER_WRITE, 100, 37, 0);
    sendCommand(SER_WRITE, 100, 38, 0);
  } else if (v == 4) { // Runner
    sendCommand(SER_WRITE, 100, 21, 3);
    sendCommand(SER_WRITE, 100, 22, 22);
    sendCommand(SER_WRITE, 100, 23, 3);
    sendCommand(SER_WRITE, 100, 24, 22);
    sendCommand(SER_WRITE, 100, 25, 25);
    sendCommand(SER_WRITE, 100, 26, 0);

    sendCommand(SER_WRITE, 100, 27, 25);
    sendCommand(SER_WRITE, 100, 28, 25);
    sendCommand(SER_WRITE, 100, 29, 3);
    sendCommand(SER_WRITE, 100, 30, 22);
    sendCommand(SER_WRITE, 100, 31, 25);
    sendCommand(SER_WRITE, 100, 32, 0);

    sendCommand(SER_WRITE, 100, 33, 3);
    sendCommand(SER_WRITE, 100, 34, 22);
    sendCommand(SER_WRITE, 100, 35, 25);
    sendCommand(SER_WRITE, 100, 36, 25);
    sendCommand(SER_WRITE, 100, 37, 25);
    sendCommand(SER_WRITE, 100, 38, 0);
  }
}
