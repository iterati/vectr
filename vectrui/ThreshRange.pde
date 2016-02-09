import java.util.ArrayList;
import java.util.logging.Level;

import processing.core.PApplet;
import processing.core.PGraphics;
import processing.core.PVector;

import controlP5.*;


public class ThreshRange extends Controller<ThreshRange> {
  int HANDLESIZE = 6;
  int HANDLESIZE2 = 3;

  int mode = -1;
  int _myMin = 0;
  int _myMax = 32;

  String myName;
  Label[] _myValueLabels = new Label[5];
  int[] _myHandles = new int[4];
  boolean isDragging;


  public ThreshRange(ControlP5 _cp5, String _name) {
    super(_cp5, _cp5.getDefaultTab(), _name, 0, 0, 792, 20);
    myName = _name;
    _cp5.register(_cp5.papplet, _name, this);
    makeLabels();
    _myArrayValue = new float[4];
    float[] _d = {4, 14, 18, 28};
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

    String[] label_names = {"ZeroLabel", "MinALabel", "MaxALabel", "MinBLabel", "MaxBLabel"};
    int[] aligns = {LEFT, LEFT, CENTER, RIGHT, RIGHT};
    int[] paddings = {0, 160, 0, 160, 0};
    for (int i = 0; i < 5; i++) {
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

    for (int i = 0; i < 4; i++) {
      if (mX > _myHandles[i] - HANDLESIZE2 && mX < _myHandles[i] + HANDLESIZE2) {
        return i;
      }
    }
    return -1;
  }

  @Override public void mousePressed() {
    mode = getMode();
  }

  public ThreshRange updateInternalEvents(PApplet theApplet) {
    if (isVisible) {
      int c = mouseX - pmouseX;
      if (c == 0) { return this; }
      if (isMousePressed && !cp5.isAltDown()) {
        if (mode >= 0 && mode < 4) {
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

  public ThreshRange setValue(int i, float theValue, boolean isUpdate) {
    if (i >= 0 && i < 4) {
      _myArrayValue[i] = theValue;
    }
    if (isUpdate) {
      updateHandleByValue(i);
      update();
    }
    return this;
  }

  public ThreshRange setValue(int i, float theValue) {
    return setValue(i, theValue, true);
  }

  public float[] getArrayValue() {
    return _myArrayValue;
  }

  public ThreshRange setArrayValue(int[] theArray) {
    for (int i = 0; i < 4; i++) {
      setValue(i, theArray[i], false);
    }
    update();
    return this;
  }

  // Call this to update handles based on value
  public void updateHandleByValue(int i) {
    _myHandles[i] = ((int)_myArrayValue[i] * (HANDLESIZE * 4)) + (HANDLESIZE * i) + HANDLESIZE2;
  }

  public void updateHandlesByValue() {
    for (int i = 0; i < 4; i++) { updateHandleByValue(i); }
  }

  // Call this when the GUI updates the position - it'll update the value as well
  public void updateHandle(int i, int v) {
    int[] tabs = {0, _myHandles[0], _myHandles[1], _myHandles[2], _myHandles[3], getWidth()};
    _myHandles[i] = PApplet.max(tabs[i] + HANDLESIZE2, PApplet.min(tabs[i + 2] - HANDLESIZE2, v));
    setValue(i, _myHandles[i] / (HANDLESIZE * 4), false);
  }

  public void updateLabels() {
    String[] labels = {"A", "A->B", "B", "B->C", "C"};
    int[] vals = {0, (int)_myArrayValue[0], (int)_myArrayValue[1], (int)_myArrayValue[2], (int)_myArrayValue[3], 32};
    for (int i = 0; i < 5; i++) {
      _myValueLabels[i].set(labels[i] + ": " + vals[i] + " - " + vals[i + 1]);
    }
  }

  @Override public ThreshRange update() {
    updateHandlesByValue();
    updateLabels();
    return this;
  }


  //********************************************************************************
  // GUI Stuff
  //********************************************************************************
  class ThreshRangeView implements ControllerView<ThreshRange> {
    public void display(PGraphics theGraphics, ThreshRange theController) {
      int hl = getMode();
      theGraphics.pushMatrix();

      theGraphics.fill(0);
      theGraphics.stroke(0);
      theGraphics.rect(0, 0, getWidth(), getHeight());

      int[] colors = {color(255, 0, 0), color(255, 255, 0), color(0, 255, 0), color(0, 255, 255), color(0, 0, 255)};
      int[] tabs = {0, _myHandles[0], _myHandles[1], _myHandles[2], _myHandles[3], getWidth()};
      for (int i = 0; i < 5; i++) {
        theGraphics.fill(colors[i]);
        theGraphics.rect(tabs[i], 0, tabs[i + 1] - tabs[i], getHeight());
      }

      for (int i = 0; i < 4; i++) {
        theGraphics.fill((hl == i) ? 128 : 255);
        theGraphics.stroke((hl == i) ? 255 : 0);
        theGraphics.rect(_myHandles[i] - HANDLESIZE2, 0, HANDLESIZE, getHeight());
      }

      if (isLabelVisible) {
        _myCaptionLabel.draw(theGraphics, 0, 0, theController);
        for (int i = 0; i < 5; i++) {
          _myValueLabels[i].draw(theGraphics, 0, 0, theController);
        }
      }

      theGraphics.popMatrix();
      theGraphics.noStroke();
    }
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

  @Override public ThreshRange updateDisplayMode(int theMode) {
    _myDisplayMode = theMode;
    switch (theMode) {
      case (DEFAULT):
        _myControllerView = new ThreshRangeView();
        break;
      case (SPRITE):
        _myControllerView = new ThreshRangeSpriteView();
        break;
      case (IMAGE):
        _myControllerView = new ThreshRangeImageView();
        break;
      case (CUSTOM):
      default:
        break;
    }
    return this;
  }

  class ThreshRangeImageView implements ControllerView<ThreshRange> {
    public void display(PGraphics theGraphics, ThreshRange theController) {
      ControlP5.logger().log(Level.INFO, "ThreshRangeImageDisplay not implemented.");
    }
  }

  class ThreshRangeSpriteView implements ControllerView<ThreshRange> {
    public void display(PGraphics theGraphics, ThreshRange theController) {
      ControlP5.logger().log(Level.INFO, "ThreshRangeSpriteDisplay not available.");
    }
  }

  @Override public String toString() {
    return "type:\tThreshRange\n" + super.toString();
  }
}
