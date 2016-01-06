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


void setup() {
  surface.setTitle("VectrUI");
  size(1000, 800);
  cp5 = new ControlP5(this);
  cp5.setFont(createFont("Verdana", 10));

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

  // TODO: Handle loading screen and refresh
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
  if (theEvent.getController().equals(editor.patternThresh)) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, 8, (int)editor.patternThresh.getMinA());
      sendCommand(SER_WRITE, 100, 9, (int)editor.patternThresh.getMaxA());
      sendCommand(SER_WRITE, 100, 10, (int)editor.patternThresh.getMinB());
      sendCommand(SER_WRITE, 100, 11, (int)editor.patternThresh.getMaxB());
    }
  } else if (theEvent.getController().equals(editor.colorThresh)) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, 4, (int)editor.colorThresh.getMinA());
      sendCommand(SER_WRITE, 100, 5, (int)editor.colorThresh.getMaxA());
      sendCommand(SER_WRITE, 100, 6, (int)editor.colorThresh.getMinB());
      sendCommand(SER_WRITE, 100, 7, (int)editor.colorThresh.getMaxB());
    }
  } else if (theEvent.getController().equals(editor.base)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_WRITE, 100, 0, (int)theEvent.getController().getValue());
      editor.patternChanged((int)theEvent.getController().getValue());
    } else if (theEvent.getAction() == ControlP5.ACTION_LEAVE) {
      editor.base.close();
    }
  } else if (theEvent.getController().equals(editor.prevMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_MODE_SET, 99, 0, 0);
    }
  } else if (theEvent.getController().equals(editor.nextMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_MODE_SET, 101, 0, 0);
    }
  } else if (theEvent.getController().equals(editor.saveMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      // TODO: Save in-memory mode to file
    }
  } else if (theEvent.getController().equals(editor.loadMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      // TODO: Load file to in-memory mode
    }
  } else if (theEvent.getController().equals(editor.writeMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_SAVE, 0, 0, 0);
    }
  } else if (theEvent.getController().equals(editor.resetMode)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      sendCommand(SER_MODE_SET, cur_mode, 0, 0);
    }
  } else if (theEvent.getController().equals(editor.saveLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      // TODO: Save all modes to file
    }
  } else if (theEvent.getController().equals(editor.writeLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      // TODO: Load file into EEPROM of light
    }
  } else if (theEvent.getController().equals(editor.disconnectLight)) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      // TODO: Send light disconnect, set state to uninitialized
      sendCommand(SER_DISCONNECT, 0, 0, 0);

    }
  } else if (theEvent.getController().getName().startsWith("editorArgs")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, theEvent.getController().getId(), (int)theEvent.getController().getValue());
    }
  } else if (theEvent.getController().getName().startsWith("editorTimings")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, theEvent.getController().getId(), (int)theEvent.getController().getValue());
    }
  } else if (theEvent.getController().getName().startsWith("editorNumColors")) {
    if (theEvent.getAction() == ControlP5.ACTION_RELEASED ||
        theEvent.getAction() == ControlP5.ACTION_RELEASEDOUTSIDE) {
      sendCommand(SER_WRITE, 100, theEvent.getController().getId(), (int)theEvent.getController().getValue());
      editor.numColorsChanged(theEvent.getController().getId() - 1, (int)theEvent.getController().getValue());
    } else if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      editor.numColorsChanged(theEvent.getController().getId() - 1, (int)theEvent.getController().getValue());
    }
  } else if (theEvent.getController().getId() >= 1000 && theEvent.getController().getId() < 1100) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      editor.selectColor(theEvent.getController().getValue());
    }
  } else if (theEvent.getController().getName().startsWith("editorColorValues")) {
    if (theEvent.getAction() == ControlP5.ACTION_BROADCAST) {
      if (editor.color_set >= 0 && editor.color_slot >= 0) {
        sendCommand(SER_WRITE, 100,
            39 + (editor.color_slot * 9) + (editor.color_set * 3) + (theEvent.getController().getId() - 500),
            (int)theEvent.getController().getValue());

        editor.seta(39 + (editor.color_slot * 9) + (editor.color_set * 3) + (theEvent.getController().getId() - 500),
            (int)theEvent.getController().getValue());
      }
    }
  }
}
