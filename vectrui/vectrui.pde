import java.awt.event.KeyEvent;
import processing.serial.*;
import controlP5.*;

Serial port;
ControlP5 cp5;

int gui_state = 0;
Boolean gui_initialized = false;
Boolean initialized = false;
Boolean reading = false;
int cur_mode = 0;

ModeEditor mode_editor;

void setup() {
  surface.setTitle("VectrUI b00");
  size(1000, 800);
  cp5 = new ControlP5(this);
  mode_editor = new ModeEditor(20, 20);
  gui_initialized = true;
}

void connectLight() {
  for (String p: Serial.list()) {
    try {
      port = new Serial(this, p, 57600);
    } catch (Exception e) {
    }
  }
}

void draw() {
  background(130, 135, 135);
}

void controlEvent(ControlEvent theEvent) {
  String evt = theEvent.getName();

  if (gui_initialized) {
    if (evt.startsWith("test")) {
      println(int(theEvent.getValue()));
    }
  }
}
