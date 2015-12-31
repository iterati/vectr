import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import processing.core.PGraphics;

public class ColorSliders extends ControlGroup< ColorSliders > {
  protected Slider sliderRed;
  protected Slider sliderGreen;
  protected Slider sliderBlue;

  private Object _myPlug;
  private String _myPlugName;
  private boolean broadcast;

  public ColorSliders( ControlP5 theControlP5 , String theName ) {
    this( theControlP5 , theControlP5.getDefaultTab( ) , theName , 0 , 0 , 255 , 10 );
    theControlP5.register( theControlP5.papplet , theName , this );
  }

  protected ColorSliders( ControlP5 theControlP5 , ControllerGroup< ? > theParent , String theName , int theX , int theY , int theWidth , int theHeight ) {
    super( theControlP5 , theParent , theName , theX , theY , theWidth , theHeight );
    isBarVisible = false;
    isCollapse = false;
    _myArrayValue = new float[] { 255 , 255 , 255 };

    sliderRed = cp5.addSlider( theName + "-red" , 0 , 255 , 0 , 0 , theWidth , theHeight );
    cp5.removeProperty( sliderRed );
    sliderRed.setId( 0 );
    sliderRed.setBroadcast( false );
    sliderRed.addListener( this );
    sliderRed.moveTo( this );
    sliderRed.setMoveable( false );
    sliderRed.setColorBackground( 0xff660000 );
    sliderRed.setColorForeground( 0xffaa0000 );
    sliderRed.setColorActive( 0xffff0000 );
    sliderRed.getCaptionLabel( ).setVisible( false );
    sliderRed.setDecimalPrecision( 0 );
    sliderRed.setValue( 255 );

    sliderGreen = cp5.addSlider( theName + "-green" , 0 , 255 , 0 , theHeight + 1 , theWidth , theHeight );
    cp5.removeProperty( sliderGreen );
    sliderGreen.setId( 1 );
    sliderGreen.setBroadcast( false );
    sliderGreen.addListener( this );
    sliderGreen.moveTo( this );
    sliderGreen.setMoveable( false );
    sliderGreen.setColorBackground( 0xff006600 );
    sliderGreen.setColorForeground( 0xff00aa00 );
    sliderGreen.setColorActive( 0xff00ff00 );
    sliderGreen.getCaptionLabel( ).setVisible( false );
    sliderGreen.setDecimalPrecision( 0 );
    sliderGreen.setValue( 255 );

    sliderBlue = cp5.addSlider( theName + "-blue" , 0 , 255 , 0 , ( theHeight + 1 ) * 2 , theWidth , theHeight );
    cp5.removeProperty( sliderBlue );
    sliderBlue.setId( 2 );
    sliderBlue.setBroadcast( false );
    sliderBlue.addListener( this );
    sliderBlue.moveTo( this );
    sliderBlue.setMoveable( false );
    sliderBlue.setColorBackground( 0xff000066 );
    sliderBlue.setColorForeground( 0xff0000aa );
    sliderBlue.setColorActive( 0xff0000ff );
    sliderBlue.getCaptionLabel( ).setVisible( false );
    sliderBlue.setDecimalPrecision( 0 );
    sliderBlue.setValue( 255 );

    _myPlug = cp5.papplet;
    _myPlugName = getName( );
    if ( !ControllerPlug.checkPlug( _myPlug , _myPlugName , new Class[] { int.class } ) ) {
      _myPlug = null;
    }
    broadcast = true;
  }

  public ColorSliders plugTo( Object theObject ) {
    _myPlug = theObject;
    if ( !ControllerPlug.checkPlug( _myPlug , _myPlugName , new Class[] { int.class } ) ) {
      _myPlug = null;
    }
    return this;
  }

  public ColorSliders plugTo( Object theObject , String thePlugName ) {
    _myPlug = theObject;
    _myPlugName = thePlugName;
    if ( !ControllerPlug.checkPlug( _myPlug , _myPlugName , new Class[] { int.class } ) ) {
      _myPlug = null;
    }
    return this;
  }

  @Override
  public void controlEvent( ControlEvent theEvent ) {
    if ( broadcast ) {
      _myArrayValue[ theEvent.getId( ) ] = theEvent.getValue( );
      broadcast( );
    }
  }

  private ColorSliders broadcast( ) {
    ControlEvent ev = new ControlEvent( this );
    setValue( getColorValue( ) );
    cp5.getControlBroadcaster( ).broadcast( ev , ControlP5Constants.EVENT );
    if ( _myPlug != null ) {
      try {
        Method method = _myPlug.getClass( ).getMethod( _myPlugName , int.class );
        method.invoke( _myPlug , ( int ) _myValue );
      } catch ( SecurityException ex ) {
        ex.printStackTrace( );
      } catch ( NoSuchMethodException ex ) {
        ex.printStackTrace( );
      } catch ( IllegalArgumentException ex ) {
        ex.printStackTrace( );
      } catch ( IllegalAccessException ex ) {
        ex.printStackTrace( );
      } catch ( InvocationTargetException ex ) {
        ex.printStackTrace( );
      }
    }
    return this;
  }

  @Override
  public ColorSliders setArrayValue( float[] theArray ) {
    broadcast = false;
    sliderRed.setValue( theArray[ 0 ] );
    sliderGreen.setValue( theArray[ 1 ] );
    sliderBlue.setValue( theArray[ 2 ] );
    broadcast = true;
    _myArrayValue = theArray;
    return broadcast( );
  }

  @Override
  public ColorSliders setColorValue( int theColor ) {
    setArrayValue( new float[] { theColor >> 16 & 0xff , theColor >> 8 & 0xff , theColor >> 0 & 0xff } );
    return this;
  }

  public int getColorValue( ) {
    return 0xffffffff & (int)(_myArrayValue[0]) << 16 | (int)(_myArrayValue[1]) << 8 | (int)(_myArrayValue[2]) << 0;
  }

  @Override
  public String getInfo( ) {
    return "type:\tColorSliders\n" + super.toString( );
  }
}
