/*
 * Andy's Workshop Reflow Oven Controller.
 * Copyright (c) 2014 Andy Brown. All rights reserved.
 * Please see website (http://www.andybrown.me.uk) for full details.
 */

#include "Application.h"


namespace awreflow {

  /*
   * Buttons on this page.
   */

  static const UiButton GuiButtons[7]=
  {
    { 20,64,186,128, 0x26ccd9, 0x00c3d2,
        FlashInfo::SNPB::OFFSET,FlashInfo::SNPB::WIDTH,FlashInfo::SNPB::HEIGHT,FlashInfo::SNPB::LENGTH,
        FlashInfo::LEAD::OFFSET,FlashInfo::LEAD::WIDTH,FlashInfo::LEAD::HEIGHT,FlashInfo::LEAD::LENGTH },

    { 20,212,186,128, 0x26d4a8, 0x00cd99,
        FlashInfo::SNAGCU::OFFSET,FlashInfo::SNAGCU::WIDTH,FlashInfo::SNAGCU::HEIGHT,FlashInfo::SNAGCU::LENGTH,
        FlashInfo::LEADFREE::OFFSET,FlashInfo::LEADFREE::WIDTH,FlashInfo::LEADFREE::HEIGHT,FlashInfo::LEADFREE::LENGTH },

    { 227,64,186,128, 0xad63ac, 0x9f489e,
        FlashInfo::OVEN::OFFSET,FlashInfo::OVEN::WIDTH,FlashInfo::OVEN::HEIGHT,FlashInfo::OVEN::LENGTH,
        FlashInfo::REFLOW::OFFSET,FlashInfo::REFLOW::WIDTH,FlashInfo::REFLOW::HEIGHT,FlashInfo::REFLOW::LENGTH },

    { 227,212,186,128, 0x8e959d, 0x7a828c,
        FlashInfo::FLAME::OFFSET,FlashInfo::FLAME::WIDTH,FlashInfo::FLAME::HEIGHT,FlashInfo::FLAME::LENGTH,
        UiButton::NO_GRAPHIC, 0,0,0 },

    { 433,64,186,78, 0x4d77ba, 0x2e5fae,
        UiButton::NO_GRAPHIC, 0,0,0,
        FlashInfo::PROPORTIONAL::OFFSET,FlashInfo::PROPORTIONAL::WIDTH,FlashInfo::PROPORTIONAL::HEIGHT,FlashInfo::PROPORTIONAL::LENGTH },

    { 433,162,186,78, 0x4d77ba, 0x2e5fae,
        UiButton::NO_GRAPHIC, 0,0,0,
        FlashInfo::INTEGER::OFFSET,FlashInfo::INTEGER::WIDTH,FlashInfo::INTEGER::HEIGHT,FlashInfo::INTEGER::LENGTH },

    { 433,261,186,78, 0x4d77ba, 0x2e5fae,
        UiButton::NO_GRAPHIC, 0,0,0,
        FlashInfo::DERIVATIVE::OFFSET, FlashInfo::DERIVATIVE::WIDTH,FlashInfo::DERIVATIVE::HEIGHT,FlashInfo::DERIVATIVE::LENGTH }
  };


  /*
   * Run the options page and return what the user selected when they click
   * on the reflow button
   */

  ReflowParameters ControlPage::run() {

    uint32_t start;

    // draw the full GUI

    redrawAll();

    // go into a keypress/timeout event loop

    start=MillisecondTimer::millis();

    for(;;) {

      // each second, sample the temperature and display it

      if(MillisecondTimer::hasTimedOut(start,1000)) {
        drawTemperature();
        start=MillisecondTimer::millis();
      }

      if(_buttonPressed) {

        // interrupt handler has indicated that a button is down

        switch(_buttonId) {

          case ButtonIdentifier::LEFT:
            handleLeft();
            break;

          case ButtonIdentifier::RIGHT:
            handleRight();
            break;

          case ButtonIdentifier::OK:
            if(handleOk()) {

              // user has clicked reflow, return the parameters we gathered on this page

              ReflowParameters params;

              params.P=_p.getValue();
              params.I=_i.getValue();
              params.D=_d.getValue();
              params.Leaded=_leadedChecked;

              return params;
            }
            break;
        }

        // ready for the next press

        _buttonPressed=false;
      }
    }
  }


  /*
   * Left button pressed: navigate backwards
   */

  void ControlPage::handleLeft() {

    // if captive then we adjust the number

    if(_captive) {
      switch(_selectedButton) {
        case PROPORTIONAL:
          --_p;
          _p.redraw(_panel);
          break;

        case INTEGER:
          --_i;
          _i.redraw(_panel);
          break;

        case DERIVATIVE:
          --_d;
          _d.redraw(_panel);
          break;

        default:
          break;
      }
    }
    else {
      // erase old selection

      drawSelection(false);

      // set the new selection

      switch(_selectedButton) {

        case LEADED:
          _selectedButton=DERIVATIVE;
          break;

        case LEAD_FREE:
          _selectedButton=LEADED;
          break;

        case REFLOW:
          _selectedButton=LEAD_FREE;
          break;

        case PROPORTIONAL:
          _selectedButton=REFLOW;
          break;

        case INTEGER:
          _selectedButton=PROPORTIONAL;
          break;

        case DERIVATIVE:
          _selectedButton=INTEGER;
          break;
      }

      // draw the new selection

      drawSelection(true);
    }
  }


  /*
   * Right button pressed: navigate forwards through the items
   */

  void ControlPage::handleRight() {

    // if captive then we adjust the number

    if(_captive) {
      switch(_selectedButton) {
        case PROPORTIONAL:
          ++_p;
          _p.redraw(_panel);
          break;

        case INTEGER:
          ++_i;
          _i.redraw(_panel);
          break;

        case DERIVATIVE:
          ++_d;
          _d.redraw(_panel);
          break;

        default:
          break;
      }
    }
    else {

      // erase old selection

      drawSelection(false);

      // set the new selection

      switch(_selectedButton) {

        case LEADED:
          _selectedButton=LEAD_FREE;
          break;

        case LEAD_FREE:
          _selectedButton=REFLOW;
          break;

        case REFLOW:
          _selectedButton=PROPORTIONAL;
          break;

        case PROPORTIONAL:
          _selectedButton=INTEGER;
          break;

        case INTEGER:
          _selectedButton=DERIVATIVE;
          break;

        case DERIVATIVE:
          _selectedButton=LEADED;
          break;
      }

      // draw the new selection

      drawSelection(true);
    }
  }


  /*
   * Handle the OK button
   */

  bool ControlPage::handleOk() {

    if(_captive) {
      _captive=false;
      drawSelection(true);
    }
    else {

      switch(_selectedButton) {

        case PROPORTIONAL:
        case INTEGER:
        case DERIVATIVE:
          _captive^=true;       // move into/out of the captive state where left/right are directed to these buttons
          drawSelection(true);
          break;

        case LEADED:
        case LEAD_FREE:
          {
            // change the selected profile to leaded/unleaded

            FlashGraphics flash(_panel);

            _leadedChecked=_selectedButton==LEADED;
            drawCheck(flash);
          }
          break;

        case REFLOW:
          if(_broken) {
            handleBroken();
            break;
          }
          else
            return true;      // trigger this class to exit

        default:
          break;
      }
    }

    // no exit from the page

    return false;
  }


  /*
   * The link to the temperature sensor is broken but the user has clicked on reflow.
   * Flash the broken icon to draw the attention to it
   */

  void ControlPage::handleBroken() {

    FlashGraphics flash(_panel);
    uint8_t i;
    Panel::LcdPanel& gl(_panel.getGraphicsLibrary());
    Rectangle rcBroken(374,305,26,26);

    gl.setForeground(0x7a828c);

    for(i=0;i<5;i++) {

      gl.fillRectangle(rcBroken);
      MillisecondTimer::delay(500);

      flash.drawBitmap(rcBroken,FlashInfo::BROKEN_GREY::OFFSET,FlashInfo::BROKEN_GREY::LENGTH);
      MillisecondTimer::delay(500);
    }
  }


  /*
   * Clear down and redraw everything
   */

  void ControlPage::redrawAll() {

    FlashGraphics flash(_panel);

    // fade out (it's asynchronous and takes about a second)

    fadeAndClear();

    // draw the lot

    flash.drawBitmap(Rectangle(20,20,91,24),FlashInfo::CONTROL::OFFSET,FlashInfo::CONTROL::LENGTH);

    drawButtons(flash,GuiButtons,sizeof(GuiButtons)/sizeof(GuiButtons[0]));
    drawSelectionAndCheck(flash,true);

    _p.redraw(_panel);
    _i.redraw(_panel);
    _d.redraw(_panel);

    // lights back on

    _panel.setBacklight(FULL_BRIGHTNESS);
  }


  /*
   * Redraw the temperature
   */

  void ControlPage::drawTemperature() {

    DefaultTemperatureReader::Result result;

    // the temperature reader cannot exist in the same scope as the flash because they
    // share the SPI bus. Hence we use it in a sub-scope here.

    {
      DefaultTemperatureReader reader;
      result=reader.readTemperature();
    }

    // record whether the connection is broken

    _broken=result.Status!=DefaultTemperatureReader::Result::NO_ERROR;

    // the bus has been released so it's safe to construct the flash object here now

    FlashGraphics flash(_panel);
    _temperatureWriter.redraw(flash,Point(352,317),result);
  }


  /*
   * Draw the selection and check boxes
   */

  void ControlPage::drawSelectionAndCheck(FlashGraphics& flash,bool draw) {

    drawSelection(draw);
    drawCheck(flash);
  }


  /*
   * Draw the selection box
   */

  void ControlPage::drawSelection(bool draw) const {

    // selection box

    const UiButton& button=GuiButtons[static_cast<uint8_t>(_selectedButton)];

    _gl.setForeground(draw ? (_captive ? ColourNames::GREEN : ColourNames::ORANGE) : ColourNames::BLACK);
    _gl.drawRectangle(
        Rectangle(button.X-5,button.Y-5,button.Width+10,button.Height+10)
      );
  }


  /*
   * Draw the check box
   */

  void ControlPage::drawCheck(FlashGraphics& flash) const {

    // check box

    if(_leadedChecked)
      drawCheck(flash,LEADED,FlashInfo::LEADCHECKED::OFFSET,FlashInfo::LEADCHECKED::LENGTH,0x00cd99,LEAD_FREE);
    else
      drawCheck(flash,LEAD_FREE,FlashInfo::LEADFREECHECKED::OFFSET,FlashInfo::LEADFREECHECKED::LENGTH,0x00c3d2,LEADED);
  }


  /*
   * Draw the checkbox and erase the area of the other check box
   */

  void ControlPage::drawCheck(
      FlashGraphics& flash,
      uint8_t selbtn,
      uint32_t offset,
      uint32_t length,
      Panel::tCOLOUR colour,
      uint8_t deselbtn) const {

    // draw the checkbox

    flash.drawBitmap(
        Rectangle(GuiButtons[selbtn].X+GuiButtons[selbtn].Width-26-7,
                  GuiButtons[selbtn].Y+7,
                  26,
                  26),
                  offset,
                  length);

    // erase other checkbox space

    _gl.setForeground(colour);
    _gl.fillRectangle(
        Rectangle(GuiButtons[deselbtn].X+GuiButtons[deselbtn].Width-26-7,
                  GuiButtons[deselbtn].Y+7,
                  26,
                  26));
  }
}
