import java.util.ArrayList;
import java.util.logging.Level;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;

import controlP5.*;


public class TriggerRange extends Controller<TriggerRange> {
  int HANDLESIZE = 12;
  int HANDLESIZE2 = 6;

  int mode = -1;
  int _myMin = 0;
  int _myMax = 32;
  int triggerMode = 0;

  String myName;
  Label[] _myValueLabels = new Label[2];
  int[] _myHandles = new int[2];
  boolean isDragging;


  public TriggerRange(ControlP5 _cp5, String _name) {
    super(_cp5, _cp5.getDefaultTab(), _name, 0, 0, 792, 20);
    myName = _name;
    _cp5.register(_cp5.papplet, _name, this);
    makeLabels();
    _myArrayValue = new float[2];
    float[] _d = {14, 18};
    setArrayValue(_d);
    update();
  }

  protected void makeLabels() {
    _myCaptionLabel = new controlP5.Label(cp5, myName)
      .setColor(240)
      .toUpperCase(false)
      .setFont(createFont("Comfortaa-Bold", 18))
      .align(CENTER, TOP_OUTSIDE);
    _myCaptionLabel.getStyle().setPadding(4, 4, 4, 4)
      .setMargin(-4, 0, 0, 0);

    String[] label_names = {"MinLabel", "MaxLabel"};
    int[] aligns = {LEFT, RIGHT};
    int[] paddings = {0, 0};
    for (int i = 0; i < 2; i++) {
      _myValueLabels[i] = new controlP5.Label(cp5, myName + label_names[i])
        .setColor(240)
        .toUpperCase(false)
        .set("")
        .align(aligns[i], BOTTOM_OUTSIDE)
        .setPadding(paddings[i], 5);
    }
  }

  int getMode() {
    final float posX = x(_myParent.getAbsolutePosition()) + x(position);
    final float posY = y(_myParent.getAbsolutePosition()) + y(position);
    final float mX = mouseX - posX;
    final float mY = mouseY - posY;

    if (mY < 0 || mY > getHeight()) {
      return -1;
    }

    for (int i = 0; i < 2; i++) {
      if (mX > _myHandles[i] - HANDLESIZE2 && mX < _myHandles[i] + HANDLESIZE2) {
        return i;
      }
    }
    return -1;
  }

  public TriggerRange setTriggerMode(int v) {
    triggerMode = v;
    return update();
  }

  public TriggerRange updateInternalEvents(PApplet theApplet) {
    if (isVisible) {
      int c = mouseX - pmouseX;
      if (c == 0) { return this; }
      if (isMousePressed && !cp5.isAltDown()) {
        if (mode >= 0 && mode < 2) {
          updateHandle(mode, _myHandles[mode] + c);
          updateLabels();
        }
      }
    }
    return this;
  }

  public float getValue(int i) {
    return _myArrayValue[i];
  }

  public TriggerRange setValue(int i, float theValue, boolean isUpdate) {
    if (i >= 0 && i < 2) {
      _myArrayValue[i] = theValue;
    }
    if (isUpdate) {
      updateHandleByValue(i);
      update();
    }
    return this;
  }

  public TriggerRange setValue(int i, float theValue) {
    return setValue(i, theValue, true);
  }

  public float[] getArrayValue() {
    return _myArrayValue;
  }

  public TriggerRange setArrayValue(int[] theArray) {
    for (int i = 0; i < 2; i++) {
      setValue(i, theArray[i], false);
    }
    update();
    return this;
  }

  // Call this to update handles based on value
  public void updateHandleByValue(int i) {
    _myHandles[i] = ((int)_myArrayValue[i] * (HANDLESIZE * 2)) + (HANDLESIZE * i) + HANDLESIZE2;
  }

  public void updateHandlesByValue() {
    for (int i = 0; i < 2; i++) { updateHandleByValue(i); }
  }

  // Call this when the GUI updates the position - it'll update the value as well
  public void updateHandle(int i, int v) {
    int[] tabs = {0, _myHandles[0], _myHandles[1], getWidth()};
    _myHandles[i] = PApplet.max(tabs[i] + HANDLESIZE2, PApplet.min(tabs[i + 2] - HANDLESIZE2, v));
    setValue(i, _myHandles[i] / (HANDLESIZE * 2), false);
  }

  public void updateLabels() {
    if (triggerMode < 1 || triggerMode > 4) {
    } else {
      String[] labels = {"Trigger A", "Trigger B"};
      int v;
      for (int i = 0; i < 2; i++) {
        if (triggerMode == 1) {
          v = (int)_myArrayValue[i];
        } else {
          v = ((int)_myArrayValue[i] * 5) - 80;
        }
        _myValueLabels[i].set(labels[i] + ": " + v);
      }
    }

  }

  @Override public TriggerRange update() {
    updateHandlesByValue();
    updateLabels();
    return this;
  }


  //********************************************************************************
  // GUI Stuff
  //********************************************************************************
  class TriggerRangeView implements ControllerView<TriggerRange> {
    public void display(PGraphics theGraphics, TriggerRange theController) {
      if (triggerMode < 1 || triggerMode > 4) {
        return;
      }

      int hl = getMode();
      theGraphics.pushMatrix();

      theGraphics.fill(0);
      theGraphics.stroke(0);
      theGraphics.rect(0, 0, getWidth(), getHeight());

      int[] colors = {color(255, 0, 0), color(255, 0, 255), color(0, 0, 255)};
      int[] tabs = {0, _myHandles[0], _myHandles[1], getWidth()};
      for (int i = 0; i < 3; i++) {
        theGraphics.fill(colors[i]);
        theGraphics.rect(tabs[i], 0, tabs[i + 1] - tabs[i], getHeight());
      }

      for (int i = 0; i < 2; i++) {
        theGraphics.fill((hl == i) ? 128 : 255);
        theGraphics.stroke((hl == i) ? 255 : 0);
        theGraphics.rect(_myHandles[i] - HANDLESIZE2, 0, HANDLESIZE, getHeight());
      }

      if (isLabelVisible) {
        _myCaptionLabel.draw(theGraphics, 0, 0, theController);
        for (int i = 0; i < 2; i++) {
          _myValueLabels[i].draw(theGraphics, 0, 0, theController);
        }
      }

      theGraphics.popMatrix();
      theGraphics.noStroke();
    }
  }

  @Override public void mousePressed() {
    mode = getMode();
  }

  @Override public void mouseReleased() {
    isDragging = false;
    update();
    mode = -1;
  }

  @Override public void mouseReleasedOutside() {
    mouseReleased();
  }

  @Override public void onLeave() {
    isDragging = false;
  }

  @Override public TriggerRange updateDisplayMode(int theMode) {
    _myDisplayMode = theMode;
    switch (theMode) {
      case (DEFAULT):
        _myControllerView = new TriggerRangeView();
        break;
      case (SPRITE):
        _myControllerView = new TriggerRangeSpriteView();
        break;
      case (IMAGE):
        _myControllerView = new TriggerRangeImageView();
        break;
      case (CUSTOM):
      default:
        break;
    }
    return this;
  }

  class TriggerRangeImageView implements ControllerView<TriggerRange> {
    public void display(PGraphics theGraphics, TriggerRange theController) {
      ControlP5.logger().log(Level.INFO, "TriggerRangeImageDisplay not implemented.");
    }
  }

  class TriggerRangeSpriteView implements ControllerView<TriggerRange> {
    public void display(PGraphics theGraphics, TriggerRange theController) {
      ControlP5.logger().log(Level.INFO, "TriggerRangeSpriteDisplay not available.");
    }
  }

  @Override public String toString() {
    return "type:\tTriggerRange\n" + super.toString();
  }
}
