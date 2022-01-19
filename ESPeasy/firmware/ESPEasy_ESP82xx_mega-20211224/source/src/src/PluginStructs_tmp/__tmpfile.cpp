#include "../PluginStructs/P124_data_struct.h"

#ifdef USES_P124

// **************************************************************************/
// Constructor
// **************************************************************************/
P124_data_struct::P124_data_struct(int8_t  i2c_address,
                                   uint8_t relayCount,
                                   bool    changeAddress)
  : _i2c_address(i2c_address), _relayCount(relayCount) {
  relay = new (std::nothrow) Multi_Channel_Relay();

  if (isInitialized()) {
    relay->begin(_i2c_address);

    if (changeAddress) {
      // This increment shpould match with the range of addresses in _P124_MultiRelay.ino PLUGIN_I2C_HAS_ADDRESS
      uint8_t _new_address = _i2c_address == 0x18 ? 0x11 : _i2c_address + 1; // Set to next address
      relay->changeI2CAddress(_new_address, _i2c_address);
      # ifndef BUILD_NO_DEBUG
      String log = F("MultiRelay: Change I2C address 0x");
      log += String(_i2c_address, HEX);
      log += F(" to 0x");
      log += String(_new_address, HEX);
      addLog(LOG_LEVEL_INFO, log);
      # endif // ifndef BUILD_NO_DEBUG
    }
  }
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P124_data_struct::~P124_data_struct() {
  if (isInitialized()) {
    delete relay;
    relay = nullptr;
  }
}

uint8_t P124_data_struct::getChannelState() {
  if (isInitialized()) {
    return relay->getChannelState();
  }
  return 0u;
}

uint8_t P124_data_struct::getFirmwareVersion() {
  if (isInitialized()) {
    return relay->getFirmwareVersion();
  }
  return 0u;
}

bool P124_data_struct::channelCtrl(uint8_t state) {
  if (isInitialized()) {
    relay->channelCtrl(state);
    return true;
  }
  return false;
}

bool P124_data_struct::turn_on_channel(uint8_t channel) {
  if (isInitialized() && (validChannel(channel))) {
    relay->turn_on_channel(channel);
    return true;
  }
  return false;
}

bool P124_data_struct::turn_off_channel(uint8_t channel) {
  if (isInitialized() && (validChannel(channel))) {
    relay->turn_off_channel(channel);
    return true;
  }
  return false;
}

uint8_t P124_data_struct::getNextLoop() {
  if (_loopEnabled) {
    _getLoop++;

    if (_getLoop > _relayCount) {
      _getLoop = 1;
    }
    return _getLoop;
  }
  return 0;
}

#endif // ifdef USES_P124

#include "../PluginStructs/P023_data_struct.h"
#ifdef USES_P023

#include "../Helpers/Misc.h"
#include "../Helpers/StringParser.h"


const char Plugin_023_myFont_Size[] PROGMEM = {
  0x05, // SPACE
  0x05, // !
  0x07, // "
  0x08, // #
  0x08, // $
  0x08, // %
  0x08, // &
  0x06, // '
  0x06, // (
  0x06, // )
  0x08, // *
  0x08, // +
  0x05, // ,
  0x08, // -
  0x05, // .
  0x08, // /
  0x08, // 0
  0x07, // 1
  0x08, // 2
  0x08, // 3
  0x08, // 4
  0x08, // 5
  0x08, // 6
  0x08, // 7
  0x08, // 8
  0x08, // 9
  0x06, // :
  0x06, // ;
  0x07, // <
  0x08, // =
  0x07, // >
  0x08, // ?
  0x08, // @
  0x08, // A
  0x08, // B
  0x08, // C
  0x08, // D
  0x08, // E
  0x08, // F
  0x08, // G
  0x08, // H
  0x06, // I
  0x08, // J
  0x08, // K
  0x08, // L
  0x08, // M
  0x08, // N
  0x08, // O
  0x08, // P
  0x08, // Q
  0x08, // R
  0x08, // S
  0x08, // T
  0x08, // U
  0x08, // V
  0x08, // W
  0x08, // X
  0x08, // Y
  0x08, // Z
  0x06, // [
  0x08, // BACKSLASH
  0x06, // ]
  0x08, // ^
  0x08, // _
  0x06, // `
  0x08, // a
  0x08, // b
  0x07, // c
  0x08, // d
  0x08, // e
  0x07, // f
  0x08, // g
  0x08, // h
  0x05, // i
  0x06, // j
  0x07, // k
  0x06, // l
  0x08, // m
  0x07, // n
  0x07, // o
  0x07, // p
  0x07, // q
  0x07, // r
  0x07, // s
  0x06, // t
  0x07, // u
  0x08, // v
  0x08, // w
  0x08, // x
  0x07, // y
  0x08, // z
  0x06, // {
  0x05, // |
  0x06, // }
  0x08, // ~
  0x08  // DEL
};


const char Plugin_023_myFont[][8] PROGMEM = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // SPACE
  { 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // !
  { 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00 }, // "
  { 0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, 0x00 }, // #
  { 0x00, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, 0x00 }, // $
  { 0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00 }, // %
  { 0x00, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00 }, // &
  { 0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00 }, // '
  { 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x00, 0x00 }, // (
  { 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00 }, // )
  { 0x00, 0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00, 0x00 }, // *
  { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00 }, // +
  { 0x00, 0xA0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 }, // ,
  { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00 }, // -
  { 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 }, // .
  { 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00 }, // /
  { 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00 }, // 0
  { 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00 }, // 1
  { 0x00, 0x62, 0x51, 0x49, 0x49, 0x46, 0x00, 0x00 }, // 2
  { 0x00, 0x22, 0x41, 0x49, 0x49, 0x36, 0x00, 0x00 }, // 3
  { 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00 }, // 4
  { 0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00 }, // 5
  { 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00 }, // 6
  { 0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00 }, // 7
  { 0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00 }, // 8
  { 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00 }, // 9
  { 0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00 }, // :
  { 0x00, 0x00, 0xAC, 0x6C, 0x00, 0x00, 0x00, 0x00 }, // ;
  { 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00 }, // <
  { 0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00 }, // =
  { 0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00 }, // >
  { 0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00 }, // ?
  { 0x00, 0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, 0x00 }, // @
  { 0x00, 0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00, 0x00 }, // A
  { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00 }, // B
  { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00 }, // C
  { 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00 }, // D
  { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00 }, // E
  { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00 }, // F
  { 0x00, 0x3E, 0x41, 0x41, 0x51, 0x72, 0x00, 0x00 }, // G
  { 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00 }, // H
  { 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00, 0x00 }, // I
  { 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 0x00 }, // J
  { 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00 }, // K
  { 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00 }, // L
  { 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00, 0x00 }, // M
  { 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x00 }, // N
  { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00 }, // O
  { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00 }, // P
  { 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 0x00 }, // Q
  { 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00 }, // R
  { 0x00, 0x26, 0x49, 0x49, 0x49, 0x32, 0x00, 0x00 }, // S
  { 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x00 }, // T
  { 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 0x00 }, // U
  { 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 0x00 }, // V
  { 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 0x00 }, // W
  { 0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00 }, // X
  { 0x00, 0x03, 0x04, 0x78, 0x04, 0x03, 0x00, 0x00 }, // Y
  { 0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00 }, // Z
  { 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00, 0x00 }, // [
  { 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00 }, // BACKSLASH
  { 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 0x00, 0x00 }, // ]
  { 0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00 }, // ^
  { 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00 }, // _
  { 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00 }, // `
  { 0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00 }, // a
  { 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00 }, // b
  { 0x00, 0x38, 0x44, 0x44, 0x28, 0x00, 0x00, 0x00 }, // c
  { 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, 0x00 }, // d
  { 0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00 }, // e
  { 0x00, 0x08, 0x7E, 0x09, 0x02, 0x00, 0x00, 0x00 }, // f
  { 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C, 0x00, 0x00 }, // g
  { 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00 }, // h
  { 0x00, 0x00, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00 }, // i
  { 0x00, 0x80, 0x84, 0x7D, 0x00, 0x00, 0x00, 0x00 }, // j
  { 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00 }, // k
  { 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00 }, // l
  { 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00 }, // m
  { 0x00, 0x7C, 0x08, 0x04, 0x7C, 0x00, 0x00, 0x00 }, // n
  { 0x00, 0x38, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00 }, // o
  { 0x00, 0xFC, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00 }, // p
  { 0x00, 0x18, 0x24, 0x24, 0xFC, 0x00, 0x00, 0x00 }, // q
  { 0x00, 0x00, 0x7C, 0x08, 0x04, 0x00, 0x00, 0x00 }, // r
  { 0x00, 0x48, 0x54, 0x54, 0x24, 0x00, 0x00, 0x00 }, // s
  { 0x00, 0x04, 0x7F, 0x44, 0x00, 0x00, 0x00, 0x00 }, // t
  { 0x00, 0x3C, 0x40, 0x40, 0x7C, 0x00, 0x00, 0x00 }, // u
  { 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 0x00 }, // v
  { 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, 0x00 }, // w
  { 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00 }, // x
  { 0x00, 0x1C, 0xA0, 0xA0, 0x7C, 0x00, 0x00, 0x00 }, // y
  { 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x00 }, // z
  { 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x00 }, // {
  { 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // |
  { 0x00, 0x41, 0x36, 0x08, 0x00, 0x00, 0x00, 0x00 }, // }
  { 0x00, 0x02, 0x01, 0x01, 0x02, 0x01, 0x00, 0x00 }, // ~
  { 0x00, 0x02, 0x05, 0x05, 0x02, 0x00, 0x00, 0x00 }  // DEL
};


P023_data_struct::P023_data_struct(uint8_t _address,   uint8_t _type, P023_data_struct::Spacing _font_spacing, uint8_t _displayTimer,uint8_t _use_sh1106)
  :  address(_address), type(_type),  font_spacing(_font_spacing),  displayTimer(_displayTimer), use_sh1106(_use_sh1106)
{}

void P023_data_struct::setDisplayTimer(uint8_t _displayTimer) {
  displayOn();
  displayTimer = _displayTimer;
}

void P023_data_struct::checkDisplayTimer() {
  if (displayTimer > 0)
  {
    displayTimer--;

    if (displayTimer == 0) {
      displayOff();
    }
  }
}

// Perform some specific changes for OLED display
String P023_data_struct::parseTemplate(String& tmpString, uint8_t lineSize) {
  String result             = parseTemplate_padded(tmpString, lineSize);
  const char degree[3]      = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_oled[2] = { 0x7F, 0 };       // P023_OLED degree symbol

  result.replace(degree, degree_oled);
  return result;
}

void P023_data_struct::resetDisplay()
{
  displayOff();
  clearDisplay();
  displayOn();
}

void P023_data_struct::StartUp_OLED()
{
  init_OLED();
  resetDisplay();
  displayOff();
  setXY(0, 0);
  clearDisplay();
  displayOn();
}

void P023_data_struct::displayOn()
{
  sendCommand(0xaf); // display on
}

void P023_data_struct::displayOff()
{
  sendCommand(0xae); // display off
}

void P023_data_struct::clearDisplay()
{
  unsigned char i, k;

  for (k = 0; k < 8; k++)
  {
    setXY(k, 0);
    {
      for (i = 0; i < 128; i++) // clear all COL
      {
        sendChar(0);            // clear all COL
      }
    }
  }
}

// Actually this sends a byte, not a char to draw in the display.
void P023_data_struct::sendChar(unsigned char data)
{
  Wire.beginTransmission(address); // begin transmitting
  Wire.write(0x40);                // data mode
  Wire.write(data);
  Wire.endTransmission();          // stop transmitting
}

// Prints a display char (not just a byte) in coordinates X Y,
// currently unused:
// void P023_data_struct::Plugin_023_sendCharXY(unsigned char data, int X, int Y)
// {
//   //if (interrupt && !doing_menu) return; // Stop printing only if interrupt is call but not in button functions
//   setXY(X, Y);
//   Wire.beginTransmission(Plugin_023_OLED_address); // begin transmitting
//   Wire.write(0x40);//data mode
//
//   for (int i = 0; i < 8; i++)
//     Wire.write(pgm_read_byte(Plugin_023_myFont[data - 0x20] + i));
//
//   Wire.endTransmission();    // stop transmitting
// }


void P023_data_struct::sendCommand(unsigned char com)
{
  Wire.beginTransmission(address); // begin transmitting
  Wire.write(0x80);                // command mode
  Wire.write(com);
  Wire.endTransmission();          // stop transmitting
}

// Set the cursor position in a 16 COL * 8 ROW map (128x64 pixels)
// or 8 COL * 5 ROW map (64x48 pixels)
void P023_data_struct::setXY(unsigned char row, unsigned char col)
{
  unsigned char col_offset = 0;

  if (use_sh1106) {
      col_offset = 0x02;    // offset of 2 when using SSH1106 controller
  }

  switch (type)
  {
    case OLED_64x48:
      col += 4;
      break;
    case OLED_64x48 | OLED_rotated:
      col += 4;
      row += 2;
  }

  sendCommand(0xb0 + row);                              // set page address
  sendCommand(0x00 + ((8 * col + col_offset) & 0x0f));  // set low col address
  sendCommand(0x10 + (((8 * col) >> 4) & 0x0f));          // set high col address
}

// Prints a string regardless the cursor position.
// unused:
// void P023_data_struct::Plugin_023_sendStr(unsigned char *string)
// {
//   unsigned char i = 0;
//   while (*string)
//   {
//     for (i = 0; i < 8; i++)
//     {
//       sendChar(pgm_read_byte(Plugin_023_myFont[*string - 0x20] + i));
//     }
//     string++;
//   }
// }


// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
void P023_data_struct::sendStrXY(const char *string, int X, int Y)
{
  setXY(X, Y);
  unsigned char i             = 0;
  unsigned char char_width    = 0;
  unsigned char currentPixels = Y * 8; // setXY always uses char_width = 8, Y = 0-based
  unsigned char maxPixels     = 128;   // Assumed default display width

  switch (type) {                      // Cater for that 1 smaller size display
    case OLED_64x48:
    case OLED_64x48 | OLED_rotated:
      maxPixels = 64;
      break;
  }

  while (*string && currentPixels < maxPixels) // Prevent display overflow on the character level
  {
    switch (font_spacing)
    {
      case Spacing::optimized:
        char_width = pgm_read_byte(&(Plugin_023_myFont_Size[*string - 0x20]));
        break;
      default:
        char_width = 8;
    }

    for (i = 0; i < char_width && currentPixels + i < maxPixels; i++) // Prevent display overflow on the pixel-level
    {
      sendChar(pgm_read_byte(Plugin_023_myFont[*string - 0x20] + i));
    }
    currentPixels += char_width;
    string++;
  }
}

void P023_data_struct::init_OLED()
{
  unsigned char multiplex;
  unsigned char compins;

  switch (type)
  {
    case OLED_128x32:
      multiplex = 0x1F;
      compins   = 0x02;
      break;
    default:
      multiplex = 0x3F;
      compins   = 0x12;
  }

  sendCommand(0xAE);       // display off
  sendCommand(0xD5);       // SETDISPLAYCLOCKDIV
  sendCommand(0x80);       // the suggested ratio 0x80
  sendCommand(0xA8);       // SSD1306_SETMULTIPLEX
  sendCommand(multiplex);  // 0x1F if 128x32, 0x3F if others (e.g. 128x64)
  sendCommand(0xD3);       // SETDISPLAYOFFSET
  sendCommand(0x00);       // no offset
  sendCommand(0x40 | 0x0); // SETSTARTLINE
  if (use_sh1106) {
    sendCommand(0xAD);       // CHARGEPUMP mode SH1106
    sendCommand(0x8B);       // CHARGEPUMP On SH1106
    sendCommand(0x32);       // CHARGEPUMP voltage 8V SH1106
    sendCommand(0x81);       // SETCONTRAS
    sendCommand(0x80);       // SH1106
  } else {
    sendCommand(0x8D);       // CHARGEPUMP
    sendCommand(0x14);
    sendCommand(0x81);       // SETCONTRAS
    sendCommand(0xCF);
  }
  sendCommand(0x20);       // MEMORYMODE
  sendCommand(0x00);       // 0x0 act like ks0108
  sendCommand(0xA0);       // 128x32 ???
  sendCommand(0xC0);       // 128x32 ???
  sendCommand(0xDA);       // COMPINS
  sendCommand(compins);    // 0x02 if 128x32, 0x12 if others (e.g. 128x64)
  sendCommand(0xD9);       // SETPRECHARGE
  sendCommand(0xF1);
  sendCommand(0xDB);       // SETVCOMDETECT
  sendCommand(0x40);
  sendCommand(0xA4);       // DISPLAYALLON_RESUME
  sendCommand(0xA6);       // NORMALDISPLAY

  clearDisplay();
  sendCommand(0x2E);       // stop scroll
  sendCommand(0x20);       // Set Memory Addressing Mode
  sendCommand(0x00);       // Set Memory Addressing Mode ab Horizontal addressing mode
}

#endif // ifdef USES_P023

#include "../PluginStructs/P032_data_struct.h"

#ifdef USES_P032

#include "../Globals/I2Cdev.h"


enum
{
  MS5xxx_CMD_RESET    = 0x1E, // perform reset
  MS5xxx_CMD_ADC_READ = 0x00, // initiate read sequence
  MS5xxx_CMD_ADC_CONV = 0x40, // start conversion
  MS5xxx_CMD_ADC_D1   = 0x00, // read ADC 1
  MS5xxx_CMD_ADC_D2   = 0x10, // read ADC 2
  MS5xxx_CMD_ADC_256  = 0x00, // set ADC oversampling ratio to 256
  MS5xxx_CMD_ADC_512  = 0x02, // set ADC oversampling ratio to 512
  MS5xxx_CMD_ADC_1024 = 0x04, // set ADC oversampling ratio to 1024
  MS5xxx_CMD_ADC_2048 = 0x06, // set ADC oversampling ratio to 2048
  MS5xxx_CMD_ADC_4096 = 0x08, // set ADC oversampling ratio to 4096
  MS5xxx_CMD_PROM_RD  = 0xA0  // initiate readout of PROM registers
};


P032_data_struct::P032_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}


// **************************************************************************/
// Initialize MS5611
// **************************************************************************/
bool P032_data_struct::begin() {
  Wire.beginTransmission(i2cAddress);
  uint8_t ret = Wire.endTransmission(true);

  return ret == 0;
}

// **************************************************************************/
// Reads the PROM of MS5611
// There are in total 8 addresses resulting in a total memory of 128 bit.
// Address 0 contains factory data and the setup, addresses 1-6 calibration
// coefficients and address 7 contains the serial code and CRC.
// The command sequence is 8 bits long with a 16 bit result which is
// clocked with the MSB first.
// **************************************************************************/
void P032_data_struct::read_prom() {
  I2C_write8(i2cAddress, MS5xxx_CMD_RESET);
  delay(3);

  for (uint8_t i = 0; i < 8; i++)
  {
    ms5611_prom[i] = I2C_read16_reg(i2cAddress, MS5xxx_CMD_PROM_RD + 2 * i);
  }
}

// **************************************************************************/
// Read analog/digital converter
// **************************************************************************/
unsigned long P032_data_struct::read_adc(unsigned char aCMD)
{
  I2C_write8(i2cAddress, MS5xxx_CMD_ADC_CONV + aCMD); // start DAQ and conversion of ADC data

  switch (aCMD & 0x0f)
  {
    case MS5xxx_CMD_ADC_256: delayMicroseconds(900);
      break;
    case MS5xxx_CMD_ADC_512: delay(3);
      break;
    case MS5xxx_CMD_ADC_1024: delay(4);
      break;
    case MS5xxx_CMD_ADC_2048: delay(6);
      break;
    case MS5xxx_CMD_ADC_4096: delay(10);
      break;
  }

  // read out values
  return I2C_read24_reg(i2cAddress, MS5xxx_CMD_ADC_READ);
}

// **************************************************************************/
// Readout
// **************************************************************************/
void P032_data_struct::readout() {
  unsigned long D1 = 0, D2 = 0;

  double dT;
  double Offset;
  double SENS;

  D2 = read_adc(MS5xxx_CMD_ADC_D2 + MS5xxx_CMD_ADC_4096);
  D1 = read_adc(MS5xxx_CMD_ADC_D1 + MS5xxx_CMD_ADC_4096);

  // calculate 1st order pressure and temperature (MS5611 1st order algorithm)
  dT                 = D2 - ms5611_prom[5] * static_cast<double>(1 << 8);
  Offset             = ms5611_prom[2] * static_cast<double>(1 << 16) + dT * ms5611_prom[4] / static_cast<double>(1 << 7);
  SENS               = ms5611_prom[1] * static_cast<double>(1 << 15) + dT * ms5611_prom[3] / static_cast<double>(1 << 8);
  ms5611_temperature = (2000 + (dT * ms5611_prom[6]) / static_cast<double>(1 << 23));

  // perform higher order corrections
  double T2 = 0., OFF2 = 0., SENS2 = 0.;

  if (ms5611_temperature < 2000) {
    T2    = dT * dT / static_cast<double>(1 << 31);
    const double temp_20deg = ms5611_temperature - 2000;
    OFF2  = 5.0 * temp_20deg * temp_20deg / static_cast<double>(1 << 1);
    SENS2 = 5.0 * temp_20deg * temp_20deg / static_cast<double>(1 << 2);

    if (ms5611_temperature < -1500) {
      const double temp_min15deg = ms5611_temperature + 1500;
      OFF2  += 7.0 * temp_min15deg * temp_min15deg;
      SENS2 += 11.0 * temp_min15deg * temp_min15deg / static_cast<double>(1 << 1);
    }
  }

  ms5611_temperature -= T2;
  Offset             -= OFF2;
  SENS               -= SENS2;
  ms5611_pressure     = (((D1 * SENS) / static_cast<double>(1 << 21) - Offset) / static_cast<double>(1 << 15));
}


#endif // ifdef USES_P032

#include "../PluginStructs/P012_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <LiquidCrystal_I2C.h>

#ifdef USES_P012

P012_data_struct::P012_data_struct(uint8_t addr,
                                   uint8_t lcd_size,
                                   uint8_t mode,
                                   uint8_t    timer) :
  lcd(addr, 20, 4),
  Plugin_012_mode(mode),
  displayTimer(timer) {
  switch (lcd_size)
  {
    case 1:
      Plugin_012_rows = 2;
      Plugin_012_cols = 16;
      break;
    case 2:
      Plugin_012_rows = 4;
      Plugin_012_cols = 20;
      break;

    default:
      Plugin_012_rows = 2;
      Plugin_012_cols = 16;
      break;
  }


  // Setup LCD display
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print(F("ESP Easy"));
  createCustomChars();
}

void P012_data_struct::setBacklightTimer(uint8_t timer) {
  displayTimer = timer;
  lcd.backlight();
}

void P012_data_struct::checkTimer() {
  if (displayTimer > 0)
  {
    displayTimer--;

    if (displayTimer == 0) {
      lcd.noBacklight();
    }
  }
}

void P012_data_struct::lcdWrite(const String& text, uint8_t col, uint8_t row) {
  // clear line before writing new string
  if (Plugin_012_mode == 2) {
    lcd.setCursor(col, row);

    for (uint8_t i = col; i < Plugin_012_cols; i++) {
      lcd.print(' ');
    }
  }

  lcd.setCursor(col, row);

  if ((Plugin_012_mode == 1) || (Plugin_012_mode == 2)) {
    lcd.setCursor(col, row);

    for (uint8_t i = 0; i < Plugin_012_cols - col; i++) {
      if (text[i]) {
        lcd.print(text[i]);
      }
    }
  }

  // message exceeding cols will continue to next line
  else {
    // Fix Weird (native) lcd display behaviour that split long string into row 1,3,2,4, instead of 1,2,3,4
    bool stillProcessing = 1;
    uint8_t charCount       = 1;

    while (stillProcessing) {
      if (++col > Plugin_012_cols) { // have we printed 20 characters yet (+1 for the logic)
        row += 1;
        lcd.setCursor(0, row);       // move cursor down
        col = 1;
      }

      // dont print if "lower" than the lcd
      if (row < Plugin_012_rows) {
        lcd.print(text[charCount - 1]);
      }

      if (!text[charCount]) { // no more chars to process?
        stillProcessing = 0;
      }
      charCount += 1;
    }

    // lcd.print(text.c_str());
    // end fix
  }
}

// Perform some specific changes for LCD display
// https://www.letscontrolit.com/forum/viewtopic.php?t=2368
String P012_data_struct::P012_parseTemplate(String& tmpString, uint8_t lineSize) {
  String result            = parseTemplate_padded(tmpString, lineSize);
  const char degree[3]     = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_lcd[2] = { 0xdf, 0 };       // P012_LCD degree symbol

  result.replace(degree, degree_lcd);

  char unicodePrefix = 0xc4;

# ifdef USES_P012_POLISH_CHARS

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_a_uni[3] = { 0xc4, 0x85, 0 }; // Unicode znak a
    const char znak_a_lcd[2] = { 0x05, 0 };       // P012_LCD znak a
    result.replace(znak_a_uni, znak_a_lcd);

    const char znak_A_uni[3] = { 0xc4, 0x84, 0 }; // Unicode znak A
    result.replace(znak_A_uni, znak_a_lcd);

    const char znak_c_uni[3] = { 0xc4, 0x87, 0 }; // Unicode znak c
    const char znak_c_lcd[2] = { 0x03, 0 };       // P012_LCD znak c
    result.replace(znak_c_uni, znak_c_lcd);

    const char znak_C_uni[3] = { 0xc4, 0x86, 0 }; // Unicode znak C
    result.replace(znak_C_uni, znak_c_lcd);

    const char znak_e_uni[3] = { 0xc4, 0x99, 0 }; // Unicode znak e
    const char znak_e_lcd[2] = { 0x02, 0 };       // P012_LCD znak e
    result.replace(znak_e_uni, znak_e_lcd);

    const char znak_E_uni[3] = { 0xc4, 0x98, 0 }; // Unicode znak E
    result.replace(znak_E_uni, znak_e_lcd);
  }

  unicodePrefix = 0xc5;

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_l_uni[3] = { 0xc5, 0x82, 0 };  // Unicode znak l
    const char znak_l_lcd[2] = { 0x01, 0 };        // P012_LCD znak l
    result.replace(znak_l_uni, znak_l_lcd);

    const char znak_L_uni[3] = { 0xc5, 0x81, 0 };  // Unicode znak L
    result.replace(znak_L_uni, znak_l_lcd);

    const char znak_n_uni[3] = { 0xc5, 0x84, 0 };  // Unicode znak n
    const char znak_n_lcd[2] = { 0x04, 0 };        // P012_LCD znak n
    result.replace(znak_n_uni, znak_n_lcd);

    const char znak_N_uni[3] = { 0xc5, 0x83, 0 };  // Unicode znak N
    result.replace(znak_N_uni, znak_n_lcd);

    const char znak_s_uni[3] = { 0xc5, 0x9b, 0 };  // Unicode znak s
    const char znak_s_lcd[2] = { 0x06, 0 };        // P012_LCD znak s
    result.replace(znak_s_uni, znak_s_lcd);

    const char znak_S_uni[3] = { 0xc5, 0x9a, 0 };  // Unicode znak S
    result.replace(znak_S_uni, znak_s_lcd);

    const char znak_z1_uni[3] = { 0xc5, 0xba, 0 }; // Unicode znak z z kreska
    const char znak_z1_lcd[2] = { 0x07, 0 };       // P012_LCD znak z z kropka
    result.replace(znak_z1_uni, znak_z1_lcd);

    const char znak_Z1_uni[3] = { 0xc5, 0xb9, 0 }; // Unicode znak Z z kreska
    result.replace(znak_Z1_uni, znak_z1_lcd);

    const char znak_z2_uni[3] = { 0xc5, 0xbc, 0 }; // Unicode znak z z kropka
    const char znak_z2_lcd[2] = { 0x07, 0 };       // P012_LCD znak z z kropka
    result.replace(znak_z2_uni, znak_z2_lcd);

    const char znak_Z2_uni[3] = { 0xc5, 0xbb, 0 }; // Unicode znak Z z kropka
    result.replace(znak_Z2_uni, znak_z2_lcd);
  }

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    const char znak_o_uni[3] = { 0xc3, 0xB3, 0 }; // Unicode znak o
    const char znak_o_lcd[2] = { 0x08, 0 };       // P012_LCD znak o
    result.replace(znak_o_uni, znak_o_lcd);

    const char znak_O_uni[3] = { 0xc3, 0x93, 0 }; // Unicode znak O
    result.replace(znak_O_uni, znak_o_lcd);
  }
# endif // USES_P012_POLISH_CHARS

  unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3] = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_lcd[2] = { 0xe1, 0 };        // P012_LCD Umlaute
    result.replace(umlautAE_uni,  umlautAE_lcd);

    const char umlaut_ae_uni[3] = { 0xc3, 0xa4, 0 }; // Unicode Umlaute ae
    result.replace(umlaut_ae_uni, umlautAE_lcd);

    const char umlautOE_uni[3] = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_lcd[2] = { 0xef, 0 };        // P012_LCD Umlaute
    result.replace(umlautOE_uni,  umlautOE_lcd);

    const char umlaut_oe_uni[3] = { 0xc3, 0xb6, 0 }; // Unicode Umlaute oe
    result.replace(umlaut_oe_uni, umlautOE_lcd);

    const char umlautUE_uni[3] = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_lcd[2] = { 0xf5, 0 };        // P012_LCD Umlaute
    result.replace(umlautUE_uni,  umlautUE_lcd);

    const char umlaut_ue_uni[3] = { 0xc3, 0xbc, 0 }; // Unicode Umlaute ue
    result.replace(umlaut_ue_uni, umlautUE_lcd);

    const char umlaut_sz_uni[3] = { 0xc3, 0x9f, 0 }; // Unicode Umlaute sz
    const char umlaut_sz_lcd[2] = { 0xe2, 0 };       // P012_LCD Umlaute
    result.replace(umlaut_sz_uni, umlaut_sz_lcd);
  }
  return result;
}

void P012_data_struct::createCustomChars() {
# ifdef USES_P012_POLISH_CHARS
/*
  static const char LETTER_null[8] PROGMEM = { // spacja
    0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000
  };
*/
  static const char LETTER_a[8] PROGMEM = {    // a
    0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111, 0b00010
  };
  static const char LETTER_c[8] PROGMEM = {    // c
    0b00010, 0b00100, 0b01110, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000
  };
  static const char LETTER_e[8] PROGMEM = {    // e
    0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110, 0b00010
  };
  static const char LETTER_l[8] PROGMEM = {    // l
    0b01100, 0b00100, 0b00101, 0b00110, 0b01100, 0b00100, 0b01110, 0b00000
  };
  static const char LETTER_n[8] PROGMEM = {    // n
    0b00010, 0b00100, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000
  };
  static const char LETTER_o[8] PROGMEM = {    // o
    0b00010, 0b00100, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000
  };
  static const char LETTER_s[8] PROGMEM = {    // s
    0b00010, 0b00100, 0b01110, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000
  };
  /*
  static const char LETTER_z1[8] PROGMEM = {   // z z kreska
    0b00010, 0b00100, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000
  };
  */
  static const char LETTER_z2[8] PROGMEM = {   // z z kropka
    0b00100, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000
  };
  lcd.createChar(0, LETTER_o);  // probably defected memory cell
  lcd.createChar(1, LETTER_l);
  lcd.createChar(2, LETTER_e);
  lcd.createChar(3, LETTER_c);
  lcd.createChar(4, LETTER_n);
  lcd.createChar(5, LETTER_a);
  lcd.createChar(6, LETTER_s);
  lcd.createChar(7, LETTER_z2);
  lcd.createChar(8, LETTER_o);
# endif // ifdef USES_P012_POLISH_CHARS
}

#endif  // ifdef USES_P012

#include "../PluginStructs/P095_data_struct.h"

#ifdef USES_P095


P095_data_struct::P095_data_struct(int8_t _CS, int8_t _DC, int8_t _RST)
  : tft(_CS,  _DC, _RST) 
  {
    tft.begin();
  }

//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default ILI9341_WHITE)
//param [in] bkcolor : The background color (default ILI9341_BLACK)
void P095_data_struct::printText(const String& string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  tft.setCursor(X, Y);
  tft.setTextColor(color, bkcolor);
  tft.setTextSize(textSize);
  tft.println(string);
}


//Parse color string to ILI9341 color
//param [in] s : The color string (white, red, ...)
//return : color (default ILI9341_WHITE)
unsigned short P095_data_struct::ParseColor(String & s)
{
  if (s.equalsIgnoreCase(F("BLACK")))
    return ILI9341_BLACK;
  if (s.equalsIgnoreCase(F("NAVY")))
    return ILI9341_NAVY;
  if (s.equalsIgnoreCase(F("DARKGREEN")))
    return ILI9341_DARKGREEN;
  if (s.equalsIgnoreCase(F("DARKCYAN")))
    return ILI9341_DARKCYAN;
  if (s.equalsIgnoreCase(F("MAROON")))
    return ILI9341_MAROON;
  if (s.equalsIgnoreCase(F("PURPLE")))
    return ILI9341_PURPLE;
  if (s.equalsIgnoreCase(F("OLIVE")))
    return ILI9341_OLIVE;
  if (s.equalsIgnoreCase(F("LIGHTGREY")))
    return ILI9341_LIGHTGREY;
  if (s.equalsIgnoreCase(F("DARKGREY")))
    return ILI9341_DARKGREY;
  if (s.equalsIgnoreCase(F("BLUE")))
    return ILI9341_BLUE;
  if (s.equalsIgnoreCase(F("GREEN")))
    return ILI9341_GREEN;
  if (s.equalsIgnoreCase(F("CYAN")))
    return ILI9341_CYAN;
  if (s.equalsIgnoreCase(F("RED")))
    return ILI9341_RED;
  if (s.equalsIgnoreCase(F("MAGENTA")))
    return ILI9341_MAGENTA;
  if (s.equalsIgnoreCase(F("YELLOW")))
    return ILI9341_YELLOW;
  if (s.equalsIgnoreCase(F("WHITE")))
    return ILI9341_WHITE;
  if (s.equalsIgnoreCase(F("ORANGE")))
    return ILI9341_ORANGE;
  if (s.equalsIgnoreCase(F("GREENYELLOW")))
    return ILI9341_GREENYELLOW;
  if (s.equalsIgnoreCase(F("PINK")))
    return ILI9341_PINK;

  if(s.length() == 7 && s[0] == '#')
  {
    // convrt to long value in base16, then split up into r, g, b values
    long long number = strtoll( &s[1], NULL, 16);
    //long long r = number >> 16;
    //long long g = number >> 8 & 0xFF;
    //long long b = number & 0xFF;
    //convert to color565 (used by adafruit lib)
    return tft.color565(number >> 16, number >> 8 & 0xFF, number & 0xFF);
  }
  return ILI9341_WHITE; //fallback value
}

//Split a string by delimiter
//param [in] s : The input string
//param [in] c : The delimiter
//param [out] op : The resulting string array
//param [in] limit : The maximum strings to find
//return : The string count
int P095_data_struct::StringSplit(String &s, char c, String op[], int limit)
{
  int count = 0;
  char * pch;
  String d = String(c);
  pch = strtok ((char*)(s.c_str()),d.c_str());
  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok (NULL, ",");
  }
  return count;
}

#endif // ifdef USES_P095

#include "../PluginStructs/P083_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <Adafruit_SGP30.h>


#ifdef USES_P083

P083_data_struct::P083_data_struct() {
  initialized = sgp.begin();
  init_time   = millis();
}

#endif // ifdef USES_P083

#include "../PluginStructs/P060_data_struct.h"


#ifdef USES_P060

P060_data_struct::P060_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}

void P060_data_struct::overSampleRead()
{
  OversamplingValue += readMCP3221();
  OversamplingCount++;
}

float P060_data_struct::getValue()
{
  float value;

  if (OversamplingCount > 0)
  {
    value             = static_cast<float>(OversamplingValue) / OversamplingCount;
    OversamplingValue = 0;
    OversamplingCount = 0;
  } else {
    value = readMCP3221();
  }
  return value;
}

uint16_t P060_data_struct::readMCP3221()
{
  uint16_t value;

  Wire.requestFrom(i2cAddress, (uint8_t)2);

  if (Wire.available() == 2)
  {
    value = (Wire.read() << 8) | Wire.read();
  }
  else {
    value = 9999;
  }

  return value;
}

#endif // ifdef USES_P060

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter

#include "../PluginStructs/P050_data_struct.h"


#ifdef USES_P050

# include "../Helpers/ESPEasy_Storage.h"

# include <Adafruit_TCS34725.h>

P050_data_struct::P050_data_struct(uint16_t integrationSetting, uint16_t gainSetting) {

  // Map integration time setting (uint16_t to enum)
  _integrationTime = static_cast<tcs34725IntegrationTime_t>(integrationSetting);

  // Map gain setting (uint16_t -> enum)
  _gain = static_cast<tcs34725Gain_t>(gainSetting);

  /* Initialise with specific int time and gain values */
  tcs = Adafruit_TCS34725(_integrationTime, _gain);

  resetTransformation();

  // String log = F("P050_data sizeof(TransformationSettings): ");
  // log += sizeof(TransformationSettings);
  // addLog(LOG_LEVEL_INFO, log);
}

/**
 * resetTransformation
 * Effectgively sets matrix[0][0], matrix[1][1] and matrix[2][2] to 1.0f, all other fields to 0.0f 
 */
void P050_data_struct::resetTransformation() {
  // Initialize Transformationn defaults
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      TransformationSettings.matrix[i][j] = i == j ? 1.0f : 0.0f;
    }
  }
}

/**
 * applyTransformation : calibrate r/g/b inputs (uint16_t) to rc/gc/bc outputs (float, by reference)
 */
void P050_data_struct::applyTransformation(uint16_t r, uint16_t g, uint16_t b, float *rc, float *gc, float *bc) {
  *rc = TransformationSettings.matrix[0][0] * static_cast<float>(r) + TransformationSettings.matrix[0][1] * static_cast<float>(g) + TransformationSettings.matrix[0][2] * static_cast<float>(b);
  *gc = TransformationSettings.matrix[1][0] * static_cast<float>(r) + TransformationSettings.matrix[1][1] * static_cast<float>(g) + TransformationSettings.matrix[1][2] * static_cast<float>(b);
  *bc = TransformationSettings.matrix[2][0] * static_cast<float>(r) + TransformationSettings.matrix[2][1] * static_cast<float>(g) + TransformationSettings.matrix[2][2] * static_cast<float>(b);
}

/**
 * applyTransformation : calibrate normalized r/g/b inputs (float) to rc/gc/bc outputs (float, by reference)
 */
void P050_data_struct::applyTransformation(float nr, float ng, float nb, float *rc, float *gc, float *bc) {
  *rc = TransformationSettings.matrix[0][0] * static_cast<float>(nr) + TransformationSettings.matrix[0][1] * static_cast<float>(ng) + TransformationSettings.matrix[0][2] * static_cast<float>(nb);
  *gc = TransformationSettings.matrix[1][0] * static_cast<float>(nr) + TransformationSettings.matrix[1][1] * static_cast<float>(ng) + TransformationSettings.matrix[1][2] * static_cast<float>(nb);
  *bc = TransformationSettings.matrix[2][0] * static_cast<float>(nr) + TransformationSettings.matrix[2][1] * static_cast<float>(ng) + TransformationSettings.matrix[2][2] * static_cast<float>(nb);
}

bool P050_data_struct::loadSettings(taskIndex_t taskIndex) {
  LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&TransformationSettings), sizeof(TransformationSettings));
  return  true;
}

bool P050_data_struct::saveSettings(taskIndex_t taskIndex) {
  SaveCustomTaskSettings(taskIndex, reinterpret_cast<const uint8_t *>(&TransformationSettings), sizeof(TransformationSettings));
  return true;
}

#endif // ifdef USES_P050


#include "../PluginStructs/P022_data_struct.h"

#ifdef USES_P022

bool P022_data_struct::p022_is_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    return initializeState_lo & (1 << address_offset);
  } else {
    return initializeState_hi & (1 << (address_offset - 32));
  }
}

bool P022_data_struct::p022_set_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    initializeState_lo |= (1 << address_offset);
  } else {
    initializeState_hi |= (1 << (address_offset - 32));
  }
  return true;
}

bool P022_data_struct::p022_clear_init(uint8_t address) {
  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) { return false; }
  uint32_t address_offset = address - PCA9685_ADDRESS;

  if (address_offset < 32) {
    initializeState_lo &= ~(1 << address_offset);
  } else {
    initializeState_hi &= ~(1 << (address_offset - 32));
  }
  return true;
}

// ********************************************************************************
// PCA9685 config
// ********************************************************************************
void P022_data_struct::Plugin_022_writeRegister(int i2cAddress, int regAddress, uint8_t data) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t P022_data_struct::Plugin_022_readRegister(int i2cAddress, int regAddress) {
  uint8_t res = 0;

  Wire.requestFrom(i2cAddress, 1, 1);

  while (Wire.available()) {
    res = Wire.read();
  }
  return res;
}

// ********************************************************************************
// PCA9685 write
// ********************************************************************************
void P022_data_struct::Plugin_022_Off(int address, int pin)
{
  Plugin_022_Write(address, pin, 0);
}

void P022_data_struct::Plugin_022_On(int address, int pin)
{
  Plugin_022_Write(address, pin, PCA9685_MAX_PWM);
}

void P022_data_struct::Plugin_022_Write(int address, int Par1, int Par2)
{
  int i2cAddress = address;

  // boolean success = false;
  int regAddress = Par1 == -1
                   ? PCA9685_ALLLED_REG
                   : PCA9685_LED0 + 4 * Par1;
  uint16_t LED_ON  = 0;
  uint16_t LED_OFF = Par2;

  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.write(lowByte(LED_ON));
  Wire.write(highByte(LED_ON));
  Wire.write(lowByte(LED_OFF));
  Wire.write(highByte(LED_OFF));
  Wire.endTransmission();
}

void P022_data_struct::Plugin_022_Frequency(int address, uint16_t freq)
{
  int i2cAddress = address;

  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)0x0);
  freq *= 0.9f;

  //  prescale = 25000000 / 4096;
  uint16_t prescale = 6103;

  prescale /=  freq;
  prescale -= 1;
  uint8_t oldmode = Plugin_022_readRegister(i2cAddress, 0);
  uint8_t newmode = (oldmode & 0x7f) | 0x10;

  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)newmode);
  Plugin_022_writeRegister(i2cAddress, 0xfe,                     (uint8_t)prescale); // prescale register
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)oldmode);
  delayMicroseconds(5000);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)oldmode | 0xa1);
}

void P022_data_struct::Plugin_022_initialize(int address)
{
  int i2cAddress = address;

  // default mode is open drain output, drive leds connected to VCC
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)0x01);      // reset the device
  delay(1);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (uint8_t)B10100000); // set up for auto increment
  // Plugin_022_writeRegister(i2cAddress, PCA9685_MODE2, (uint8_t)0x10); // set to output
  p022_set_init(address);
}

#endif // ifdef USES_P022

#include "../PluginStructs/P028_data_struct.h"

#ifdef USES_P028

# include "../Helpers/Convert.h"


P028_data_struct::P028_data_struct(uint8_t addr) :
  last_hum_val(0.0f),
  last_press_val(0.0f),
  last_temp_val(0.0f),
  last_dew_temp_val(0.0f),
  last_measurement(0),
  sensorID(Unknown_DEVICE),
  i2cAddress(addr),
  state(BMx_Uninitialized) {}


uint8_t P028_data_struct::get_config_settings() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE:  return 0x28; // Tstandby 62.5ms, filter 4, 3-wire SPI Disable
    default: return 0;
  }
}

uint8_t P028_data_struct::get_control_settings() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE:  return 0x93; // Oversampling: 8x P, 8x T, normal mode
    default: return 0;
  }
}

String P028_data_struct::getFullDeviceName() const {
  String devicename = getDeviceName();

  if ((sensorID == BMP280_DEVICE_SAMPLE1) ||
      (sensorID == BMP280_DEVICE_SAMPLE2))
  {
    devicename += F(" sample");
  }
  return devicename;
}

String P028_data_struct::getDeviceName() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:  return F("BMP280");
    case BME280_DEVICE:  return F("BME280");
    default: return F("Unknown");
  }
}

boolean P028_data_struct::hasHumidity() const {
  switch (sensorID) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:  return false;
    case BME280_DEVICE:  return true;
    default: return false;
  }
}

bool P028_data_struct::initialized() const {
  return state != BMx_Uninitialized;
}

void P028_data_struct::setUninitialized() {
  state = BMx_Uninitialized;
}

// Only perform the measurements with big interval to prevent the sensor from warming up.
bool P028_data_struct::updateMeasurements(float tempOffset, unsigned long task_index) {
  const unsigned long current_time = millis();

  check(); // Check id device is present

  if (!initialized()) {
    if (!begin()) {
      return false;
    }
    state            = BMx_Initialized;
    last_measurement = 0;
  }

  if (state != BMx_Wait_for_samples) {
    if ((last_measurement != 0) &&
        !timeOutReached(last_measurement + (Settings.TaskDeviceTimer[task_index] * 1000))) {
      // Timeout has not yet been reached.
      return false;
    }

    last_measurement = current_time;

    // Set the Sensor in sleep to be make sure that the following configs will be stored
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

    if (hasHumidity()) {
      I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROLHUMID, BME280_CONTROL_SETTING_HUMIDITY);
    }
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONFIG,  get_config_settings());
    I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, get_control_settings());
    state = BMx_Wait_for_samples;
    return false;
  }

  // It takes at least 1.587 sec for valit measurements to complete.
  // The datasheet names this the "T63" moment.
  // 1 second = 63% of the time needed to perform a measurement.
  if (!timeOutReached(last_measurement + 1587)) {
    return false;
  }

  if (!readUncompensatedData()) {
    return false;
  }

  // Set to sleep mode again to prevent the sensor from heating up.
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_CONTROL, 0x00);

  last_measurement = current_time;
  state            = BMx_New_values;
  last_temp_val    = readTemperature();
  last_press_val   = readPressure() / 100.0f;
  last_hum_val     = readHumidity();


  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log.reserve(120); // Prevent re-allocation
    log  = getDeviceName();
    log += F(":");
  }
  boolean logAdded = false;

  if (hasHumidity()) {
    // Apply half of the temp offset, to correct the dew point offset.
    // The sensor is warmer than the surrounding air, which has effect on the perceived humidity.
    last_dew_temp_val = compute_dew_point_temp(last_temp_val + (tempOffset / 2.0f), last_hum_val);
  } else {
    // No humidity measurement, thus set dew point equal to air temperature.
    last_dew_temp_val = last_temp_val;
  }

  if ((tempOffset > 0.1f) || (tempOffset < -0.1f)) {
    // There is some offset to apply.
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" Apply temp offset ");
      log += tempOffset;
      log += F("C");
    }

    if (hasHumidity()) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" humidity ");
        log += last_hum_val;
      }
      last_hum_val = compute_humidity_from_dewpoint(last_temp_val + tempOffset, last_dew_temp_val);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("% => ");
        log += last_hum_val;
        log += F("%");
      }
    } else {
      last_hum_val = 0.0f;
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" temperature ");
      log += last_temp_val;
    }
    last_temp_val = last_temp_val + tempOffset;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F("C => ");
      log     += last_temp_val;
      log     += F("C");
      logAdded = true;
    }
  }

  if (hasHumidity()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F(" dew point ");
      log     += last_dew_temp_val;
      log     += F("C");
      logAdded = true;
    }
  }

  if (logAdded && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, log);
  }
  return true;
}

// **************************************************************************/
// Check BME280 presence
// **************************************************************************/
bool P028_data_struct::check() {
  bool wire_status      = false;
  const uint8_t chip_id = I2C_read8_reg(i2cAddress, BMx280_REGISTER_CHIPID, &wire_status);
  if (!wire_status) { setUninitialized(); }

  switch (chip_id) {
    case BMP280_DEVICE_SAMPLE1:
    case BMP280_DEVICE_SAMPLE2:
    case BMP280_DEVICE:
    case BME280_DEVICE: {
      if (wire_status) {
        // Store detected chip ID when chip found.
        if (sensorID != chip_id) {
          sensorID = static_cast<BMx_ChipId>(chip_id);
          setUninitialized();
          String log = F("BMx280 : Detected ");
          log += getFullDeviceName();
          addLog(LOG_LEVEL_INFO, log);
        }
      } else {
        sensorID = Unknown_DEVICE;
      }
      break;
    }
    default:
      sensorID = Unknown_DEVICE;
      break;
  }

  if (sensorID == Unknown_DEVICE) {
    String log = F("BMx280 : Unable to detect chip ID (");
    log += chip_id;
    if (!wire_status) {
      log += F(", failed");
    }
    log += ')';
    addLog(LOG_LEVEL_INFO, log);
    return false;
  }
  return wire_status;
}

bool P028_data_struct::begin() {
  if (!check()) {
    return false;
  }

  // Perform soft reset
  I2C_write8_reg(i2cAddress, BMx280_REGISTER_SOFTRESET, 0xB6);
  delay(2); // Startup time is 2 ms (datasheet)
  readCoefficients();

  //  delay(65); //May be needed here as well to fix first wrong measurement?
  return true;
}

void P028_data_struct::readCoefficients()
{
  calib.dig_T1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T1);
  calib.dig_T2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T2);
  calib.dig_T3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_T3);

  calib.dig_P1 = I2C_read16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P1);
  calib.dig_P2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P2);
  calib.dig_P3 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P3);
  calib.dig_P4 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P4);
  calib.dig_P5 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P5);
  calib.dig_P6 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P6);
  calib.dig_P7 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P7);
  calib.dig_P8 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P8);
  calib.dig_P9 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_P9);

  if (hasHumidity()) {
    calib.dig_H1 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H1);
    calib.dig_H2 = I2C_readS16_LE_reg(i2cAddress, BMx280_REGISTER_DIG_H2);
    calib.dig_H3 = I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H3);
    calib.dig_H4 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H4 + 1) & 0xF);
    calib.dig_H5 = (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5 + 1) << 4) | (I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H5) >> 4);
    calib.dig_H6 = (int8_t)I2C_read8_reg(i2cAddress, BMx280_REGISTER_DIG_H6);
  }
}

bool P028_data_struct::readUncompensatedData() {
  // wait until measurement has been completed, otherwise we would read
  // the values from the last measurement
  if (I2C_read8_reg(i2cAddress, BMx280_REGISTER_STATUS) & 0x08) {
    return false;
  }

  I2Cdata_bytes BME280_data(BME280_P_T_H_DATA_LEN, BME280_DATA_ADDR);
  bool allDataRead = I2C_read_bytes(i2cAddress, BME280_data);

  if (!allDataRead) {
    return false;
  }

  /* Variables to store the sensor data */
  uint32_t data_xlsb;
  uint32_t data_lsb;
  uint32_t data_msb;

  /* Store the parsed register values for pressure data */
  data_msb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 0] << 12;
  data_lsb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 1] << 4;
  data_xlsb              = (uint32_t)BME280_data[BME280_DATA_ADDR + 2] >> 4;
  uncompensated.pressure = data_msb | data_lsb | data_xlsb;

  /* Store the parsed register values for temperature data */
  data_msb                  = (uint32_t)BME280_data[BME280_DATA_ADDR + 3] << 12;
  data_lsb                  = (uint32_t)BME280_data[BME280_DATA_ADDR + 4] << 4;
  data_xlsb                 = (uint32_t)BME280_data[BME280_DATA_ADDR + 5] >> 4;
  uncompensated.temperature = data_msb | data_lsb | data_xlsb;

  /* Store the parsed register values for temperature data */
  data_lsb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 6] << 8;
  data_msb               = (uint32_t)BME280_data[BME280_DATA_ADDR + 7];
  uncompensated.humidity = data_msb | data_lsb;
  return true;
}

float P028_data_struct::readTemperature()
{
  int32_t var1, var2;
  int32_t adc_T = uncompensated.temperature;

  var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
          ((int32_t)calib.dig_T2)) >> 11;

  var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
          ((int32_t)calib.dig_T3)) >> 14;

  calib.t_fine = var1 + var2;

  float T = (calib.t_fine * 5 + 128) >> 8;

  return T / 100;
}

float P028_data_struct::readPressure()
{
  int64_t var1, var2, p;
  int32_t adc_P = uncompensated.pressure;

  var1 = ((int64_t)calib.t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) +
         ((var1 * (int64_t)calib.dig_P2) << 12);
  var1 = ((((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1)) >> 33;

  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p    = 1048576 - adc_P;
  p    = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
  return static_cast<float>(p) / 256;
}

float P028_data_struct::readHumidity()
{
  if (!hasHumidity()) {
    // No support for humidity
    return 0.0f;
  }
  int32_t adc_H = uncompensated.humidity;

  int32_t v_x1_u32r;

  v_x1_u32r = (calib.t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib.dig_H4) << 20) -
                  (((int32_t)calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
               (((((((v_x1_u32r * ((int32_t)calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r >> 12);

  return h / 1024.0f;
}

#endif // ifdef USES_P028

#include "../PluginStructs/P107_data_struct.h"

#ifdef USES_P107

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <Adafruit_SI1145.h>

bool P107_data_struct::begin()
{
  return uv.begin();
}

#endif // ifdef USES_P107

#include "../PluginStructs/P082_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <TinyGPS++.h>
# include <ESPeasySerial.h>

#ifdef USES_P082

const __FlashStringHelper * Plugin_082_valuename(P082_query value_nr, bool displayString) {
  switch (value_nr) {
    case P082_query::P082_QUERY_LONG:        return displayString ? F("Longitude")          : F("long");
    case P082_query::P082_QUERY_LAT:         return displayString ? F("Latitude")           : F("lat");
    case P082_query::P082_QUERY_ALT:         return displayString ? F("Altitude")           : F("alt");
    case P082_query::P082_QUERY_SPD:         return displayString ? F("Speed (m/s)")        : F("spd");
    case P082_query::P082_QUERY_SATVIS:      return displayString ? F("Satellites Visible") : F("sat_vis");
    case P082_query::P082_QUERY_SATUSE:      return displayString ? F("Satellites Tracked") : F("sat_tr");
    case P082_query::P082_QUERY_HDOP:        return displayString ? F("HDOP")               : F("hdop");
    case P082_query::P082_QUERY_FIXQ:        return displayString ? F("Fix Quality")        : F("fix_qual");
    case P082_query::P082_QUERY_DB_MAX:      return displayString ? F("Max SNR in dBHz")    : F("snr_max");
    case P082_query::P082_QUERY_CHKSUM_FAIL: return displayString ? F("Checksum Fail")      : F("chksum_fail");
    case P082_query::P082_QUERY_DISTANCE:    return displayString ? F("Distance (ODO)")     : F("dist");
    case P082_query::P082_QUERY_DIST_REF:    return displayString ? F("Distance from Reference Point") : F("dist_ref");
    case P082_query::P082_NR_OUTPUT_OPTIONS: break;
  }
  return F("");
}

const __FlashStringHelper* toString(P082_PowerMode mode) {
  switch (mode) {
    case P082_PowerMode::Max_Performance: return F("Max Performance");
    case P082_PowerMode::Power_Save:      return F("Power Save");
    case P082_PowerMode::Eco:             return F("ECO");
  }
  return F("");
}

const __FlashStringHelper* toString(P082_DynamicModel model) {
  switch (model) {
    case P082_DynamicModel::Portable:    return F("Portable");
    case P082_DynamicModel::Stationary:  return F("Stationary");
    case P082_DynamicModel::Pedestrian:  return F("Pedestrian");
    case P082_DynamicModel::Automotive:  return F("Automotive");
    case P082_DynamicModel::Sea:         return F("Sea");
    case P082_DynamicModel::Airborne_1g: return F("Airborne_1g");
    case P082_DynamicModel::Airborne_2g: return F("Airborne_2g");
    case P082_DynamicModel::Airborne_4g: return F("Airborne_4g");
    case P082_DynamicModel::Wrist:       return F("Wrist");
    case P082_DynamicModel::Bike:        return F("Bike");
  }
  return F("");
}

P082_data_struct::P082_data_struct() : gps(nullptr), easySerial(nullptr) {}

P082_data_struct::~P082_data_struct() {
  powerDown();
  reset();
}

void P082_data_struct::reset() {
  if (gps != nullptr) {
    delete gps;
    gps = nullptr;
  }

  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P082_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx) {
  if (serial_rx < 0) {
    return false;
  }
  reset();
  gps        = new (std::nothrow) TinyGPSPlus();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (easySerial != nullptr) {
    easySerial->begin(9600);
    wakeUp();
  }
  return isInitialized();
}

bool P082_data_struct::isInitialized() const {
  return gps != nullptr && easySerial != nullptr;
}

bool P082_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool completeSentence = false;

  if (easySerial != nullptr) {
    int available           = easySerial->available();
    unsigned long startLoop = millis();

    while (available > 0 && timePassedSince(startLoop) < 10) {
      --available;
      int c = easySerial->read();
      if (c >= 0) {
# ifdef P082_SEND_GPS_TO_LOG
        if (_currentSentence.length() <= 80) {
          // No need to capture more than 80 bytes as a NMEA message is never that long.
          if (c != 0) {
            _currentSentence += static_cast<char>(c);
          }
        }
# endif // ifdef P082_SEND_GPS_TO_LOG

        if (c == 0x85) {
          // Found possible start of u-blox message
          unsigned long timeout = millis() + 200;
          unsigned int bytesRead = 0;
          bool done = false;
          bool ack_nak_read = false;
          while (!timeOutReached(timeout) && !done)
          {
            if (available == 0) {
              available = easySerial->available();
            } else {
              const int c = easySerial->read();
              if (c >= 0) {
                switch (bytesRead) {
                  case 0:
                    if (c != 0x62) {
                      done = true;
                    }
                    ++bytesRead;
                    break;
                  case 1:
                    if (c != 0x05) {
                      done = true;
                    }
                    ++bytesRead;
                    break;
                  case 2:
                    if (c == 0x01) {
                      ack_nak_read = true;
                      addLog(LOG_LEVEL_INFO, F("GPS  : ACK-ACK"));
                    } else if (c == 0x00) {
                      ack_nak_read = true;
                      addLog(LOG_LEVEL_ERROR, F("GPS  : ACK-NAK"));
                    }
                    done = true;
                    break;
                  default:
                    done = true;                    
                    break;
                }
              }
            }
          }
          if (!done) {
            addLog(LOG_LEVEL_ERROR, F("GPS  : Ack/Nack timeout"));
          } else if (!ack_nak_read) {
            addLog(LOG_LEVEL_ERROR, F("GPS  : Unexpected reply"));
          }
        }

        if (gps->encode(c)) {
          // Full sentence received
# ifdef P082_SEND_GPS_TO_LOG
          _lastSentence    = _currentSentence;
          _currentSentence.clear();
# endif // ifdef P082_SEND_GPS_TO_LOG
          completeSentence = true;
        } else {
          if (available == 0) {
            available = easySerial->available();
          }
        }
      }
    }
  }
  return completeSentence;
}

bool P082_data_struct::hasFix(unsigned int maxAge_msec) {
  if (!isInitialized()) {
    return false;
  }
  return gps->location.isValid() && gps->location.age() < maxAge_msec;
}

bool P082_data_struct::storeCurPos(unsigned int maxAge_msec) {
  if (!hasFix(maxAge_msec)) {
    return false;
  }

  _distance += distanceSinceLast(maxAge_msec);
  _last_lat = gps->location.lat();
  _last_lng = gps->location.lng();
  return true;
}

// Return the distance in meters compared to last stored position.
// @retval  -1 when no fix.
double P082_data_struct::distanceSinceLast(unsigned int maxAge_msec) {
  if (!hasFix(maxAge_msec)) {
    return -1.0;
  }

  if (((_last_lat < 0.0001) && (_last_lat > -0.0001)) || ((_last_lng < 0.0001) && (_last_lng > -0.0001))) {
    return -1.0;
  }
  return gps->distanceBetween(_last_lat, _last_lng, gps->location.lat(), gps->location.lng());
}

// Return the GPS time stamp, which is in UTC.
// @param age is the time in msec since the last update of the time +
// additional centiseconds given by the GPS.
bool P082_data_struct::getDateTime(struct tm& dateTime, uint32_t& age, bool& pps_sync) {
  if (!isInitialized()) {
    return false;
  }

  if (_pps_time != 0) {
    age      = timePassedSince(_pps_time);
    _pps_time = 0;
    pps_sync = true;

    if ((age > 1000) || (gps->time.age() > age)) {
      return false;
    }
  } else {
    age      = gps->time.age();
    pps_sync = false;
  }

  if (age > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (gps->date.age() > P082_TIMESTAMP_AGE) {
    return false;
  }

  if (!gps->date.isValid() || !gps->time.isValid()) {
    return false;
  }
  dateTime.tm_year = gps->date.year() - 1900;
  dateTime.tm_mon  = gps->date.month() - 1; // GPS month starts at 1, tm_mon at 0
  dateTime.tm_mday = gps->date.day();

  dateTime.tm_hour = gps->time.hour();
  dateTime.tm_min  = gps->time.minute();
  dateTime.tm_sec  = gps->time.second();

  // FIXME TD-er: Must the offset in centisecond be added when pps_sync active?
  if (!pps_sync) {
    age += (gps->time.centisecond() * 10);
  }
  return true;
}

bool P082_data_struct::powerDown() {
  const uint8_t UBLOX_GPSStandby[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B}; 
  return writeToGPS(UBLOX_GPSStandby, sizeof(UBLOX_GPSStandby));
}

bool P082_data_struct::wakeUp() {
  if (isInitialized()) {
    if (easySerial->isTxEnabled()) {
      easySerial->println();   // Send some character to wake it up.
    }
  }
  return false;
}

#ifdef P082_USE_U_BLOX_SPECIFIC
bool P082_data_struct::setPowerMode(P082_PowerMode mode) {
  switch (mode) {
    case P082_PowerMode::Max_Performance: 
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
    case P082_PowerMode::Power_Save:      
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
    case P082_PowerMode::Eco:             
    {
      const uint8_t UBLOX_command[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x04, 0x25, 0x95}; 
      return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
    }
  }
  return false;
}

bool P082_data_struct::setDynamicModel(P082_DynamicModel model) {

  const uint8_t dynModel = static_cast<uint8_t>(model);
  if (dynModel == 1 || dynModel > 10) {
    return false;
  }

  uint8_t UBLOX_command[] = {
    0xB5, 0x62, // header
    0x06, // class
    0x24, // ID, UBX-CFG-NAV5
    0x24, 0x00, // length
    0x01, 0x00, // mask
    dynModel, // dynModel
    0x03, // fixMode auto 2D/3D
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  setUbloxChecksum(UBLOX_command, sizeof(UBLOX_command));
  return writeToGPS(UBLOX_command, sizeof(UBLOX_command));
}
#endif

#ifdef P082_USE_U_BLOX_SPECIFIC
void P082_data_struct::computeUbloxChecksum(const uint8_t* data, size_t size, uint8_t & CK_A, uint8_t & CK_B) {
  CK_A = 0;
  CK_B = 0;
  for (size_t i = 0; i < size; ++i) {
    CK_A = CK_A + data[i];
    CK_B = CK_B + CK_A;
  }
}

void P082_data_struct::setUbloxChecksum(uint8_t* data, size_t size) {
  uint8_t CK_A;
  uint8_t CK_B;
  computeUbloxChecksum(data + 2, size - 4, CK_A, CK_B);
  data[size - 2] = CK_A;
  data[size - 1] = CK_B;
}
#endif

bool P082_data_struct::writeToGPS(const uint8_t* data, size_t size) {
  if (isInitialized()) {
    if (easySerial->isTxEnabled()) {
      if (size != easySerial->write(data, size)) {
        addLog(LOG_LEVEL_ERROR, F("GPS  : Written less bytes than expected"));
        return false;
      } 
      return true;
    }
  }
  addLog(LOG_LEVEL_ERROR, F("GPS  : Cannot send to GPS"));
  return false;
}

#endif // ifdef USES_P082

#include "../PluginStructs/P058_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <HT16K33.h>


#ifdef USES_P058

P058_data_struct::P058_data_struct(uint8_t i2c_addr) {
  keyPad.Init(i2c_addr);
}

bool P058_data_struct::readKey(uint8_t& key) {
  key = keyPad.ReadKeys();

  if (keyLast != key)
  {
    keyLast = key;
    return true;
  }
  return false;
}

#endif // ifdef USES_P058

#include "../PluginStructs/P110_data_struct.h"

#ifdef USES_P110

P110_data_struct::P110_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {}

//**************************************************************************/
// Initialize VL53L0X
//**************************************************************************/
bool P110_data_struct::begin() {

  initState = true;

  sensor.setAddress(i2cAddress); // Initialize for configured address

  if (!sensor.init()) {
    String log = F("VL53L0X: Sensor not found, init failed for 0x");
    log += String(i2cAddress, HEX);
    addLog(LOG_LEVEL_INFO, log);
    addLog(LOG_LEVEL_INFO, sensor.getInitResult());
    initState = false;
    return initState;
  }

  sensor.setTimeout(500);

  if (range) {
    // lower the return signal rate limit (default is 0.25 MCPS)
    sensor.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  }

  sensor.setMeasurementTimingBudget(timing * 1000);

  delay(timing + 50);

  return initState;
}


long P110_data_struct::readDistance() {

  long dist = -1; // Invalid

#if defined(P110_DEBUG) || defined (P110_DEBUG_DEBUG)
  String log;
#endif
#ifdef P110_DEBUG_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    log  = F("VL53L0X  : idx: 0x");
    log += String(i2cAddress, HEX);
    log += F(" init: ");
    log += String(initState, BIN);
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif // P110_DEBUG_DEBUG

  if (initState) {
    success = true;
    dist = sensor.readRangeSingleMillimeters();
    if (sensor.timeoutOccurred()) {
      #ifdef P110_DEBUG_DEBUG
      addLog(LOG_LEVEL_DEBUG, "VL53L0X: TIMEOUT");
      #endif // P110_DEBUG_DEBUG
      success = false;
    } else if ( dist >= 8190 ) {
      #ifdef P110_DEBUG_DEBUG
      addLog(LOG_LEVEL_DEBUG, "VL53L0X: NO MEASUREMENT");
      #endif // P110_DEBUG_DEBUG
      success = false;
    }

#ifdef P110_DEBUG
    log = F("VL53L0X: Address: 0x");
    log += String(i2cAddress, HEX);
    log += F(" / Timing: ");
    log += String(timing, DEC);
    log += F(" / Long Range: ");
    log += String(range, BIN);
    log += F(" / Distance: ");
    log += dist;
    addLog(LOG_LEVEL_INFO, log);
#endif // P110_DEBUG
  }
  return dist;
};

bool P110_data_struct::isReadSuccessful() {
  return success;
};

#endif // ifdef USES_P110

#include "../PluginStructs/P044_data_struct.h"

#ifdef USES_P044

#include "../ESPEasyCore/Serial.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/EventQueue.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"

#define P044_RX_WAIT              PCONFIG(0)


P044_Task::P044_Task() {
  clearBuffer();
}

P044_Task::~P044_Task() {
  stopServer();
  serialEnd();
}

bool P044_Task::serverActive(WiFiServer *server) {
#if defined(ESP8266)
  return nullptr != server && server->status() != CLOSED;
#elif defined(ESP32)
  return nullptr != server && *server;
#endif // if defined(ESP8266)
}

void P044_Task::startServer(uint16_t portnumber) {
  if ((gatewayPort == portnumber) && serverActive(P1GatewayServer)) {
    // server is already listening on this port
    return;
  }
  stopServer();
  gatewayPort     = portnumber;
  P1GatewayServer = new (std::nothrow) WiFiServer(portnumber);

  if ((nullptr != P1GatewayServer) && NetworkConnected()) {
    P1GatewayServer->begin();

    if (serverActive(P1GatewayServer)) {
      addLog(LOG_LEVEL_INFO, String(F("P1   : WiFi server started at port ")) + portnumber);
    } else {
      addLog(LOG_LEVEL_ERROR, String(F("P1   : WiFi server start failed at port ")) +
             portnumber + String(F(", retrying...")));
    }
  }
}

void P044_Task::checkServer() {
  if ((nullptr != P1GatewayServer) && !serverActive(P1GatewayServer) && NetworkConnected()) {
    P1GatewayServer->close();
    P1GatewayServer->begin();

    if (serverActive(P1GatewayServer)) {
      addLog(LOG_LEVEL_INFO, F("P1   : WiFi server started"));
    }
  }
}

void P044_Task::stopServer() {
  if (nullptr != P1GatewayServer) {
    if (P1GatewayClient) { P1GatewayClient.stop(); }
    clientConnected = false;
    P1GatewayServer->close();
    addLog(LOG_LEVEL_INFO, F("P1   : WiFi server closed"));
    delete P1GatewayServer;
    P1GatewayServer = nullptr;
  }
}

bool P044_Task::hasClientConnected() {
  if ((nullptr != P1GatewayServer) && P1GatewayServer->hasClient())
  {
    if (P1GatewayClient) { P1GatewayClient.stop(); }
    P1GatewayClient = P1GatewayServer->available();
    P1GatewayClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
    addLog(LOG_LEVEL_INFO, F("P1   : Client connected!"));
  }

  if (P1GatewayClient.connected())
  {
    clientConnected = true;
  }
  else
  {
    if (clientConnected) // there was a client connected before...
    {
      clientConnected = false;
      addLog(LOG_LEVEL_INFO, F("P1   : Client disconnected!"));
    }
  }
  return clientConnected;
}

void P044_Task::discardClientIn() {
  // flush all data received from the WiFi gateway
  // as a P1 meter does not receive data
  while (P1GatewayClient.available()) {
    P1GatewayClient.read();
  }
}

void P044_Task::blinkLED() {
  blinkLEDStartTime = millis();
  digitalWrite(P044_STATUS_LED, 1);
}

void P044_Task::checkBlinkLED() {
  if ((blinkLEDStartTime > 0) && (timePassedSince(blinkLEDStartTime) >= 500)) {
    digitalWrite(P044_STATUS_LED, 0);
    blinkLEDStartTime = 0;
  }
}

void P044_Task::clearBuffer() {
  if (serial_buffer.length() > maxMessageSize) {
    maxMessageSize = _min(serial_buffer.length(), P044_DATAGRAM_MAX_SIZE);
  }

  serial_buffer = "";
  serial_buffer.reserve(maxMessageSize);
}

void P044_Task::addChar(char ch) {
  serial_buffer += ch;
}

/*  checkDatagram
    checks whether the P044_CHECKSUM of the data received from P1 matches the P044_CHECKSUM
    attached to the telegram
 */
bool P044_Task::checkDatagram() const {
  int endChar = serial_buffer.length() - 1;

  if (CRCcheck) {
    endChar -= P044_CHECKSUM_LENGTH;
  }

  if ((endChar < 0) || (serial_buffer[0] != P044_DATAGRAM_START_CHAR) ||
      (serial_buffer[endChar] != P044_DATAGRAM_END_CHAR)) { return false; }

  if (!CRCcheck) { return true; }

  const int checksumStartIndex = endChar + 1;

  #ifdef PLUGIN_044_DEBUG
    for (unsigned int cnt = 0; cnt < serial_buffer.length(); ++cnt) {
      serialPrint(serial_buffer.substring(cnt, 1));
    }
  #endif

  // calculate the CRC and check if it equals the hexadecimal one attached to the datagram
  unsigned int crc = CRC16(serial_buffer, checksumStartIndex);
  return strtoul(serial_buffer.substring(checksumStartIndex).c_str(), NULL, 16) == crc;
}

/*
   CRC16
      based on code written by Jan ten Hove
     https://github.com/jantenhove/P1-Meter-ESP8266
 */
unsigned int P044_Task::CRC16(const String& buf, int len)
{
  unsigned int crc = 0;

  for (int pos = 0; pos < len; pos++)
  {
    crc ^= static_cast<const unsigned int>(buf[pos]); // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {                    // Loop over each bit
      if ((crc & 0x0001) != 0) {                      // If the LSB is set
        crc >>= 1;                                    // Shift right and XOR 0xA001
        crc  ^= 0xA001;
      }
      else {                                          // Else LSB is not set
        crc >>= 1;                                    // Just shift right
      }
    }
  }

  return crc;
}

/*
   validP1char
       Checks if the character is valid as part of the P1 datagram contents and/or checksum.
       Returns false on a datagram start ('/'), end ('!') or invalid character
 */
bool P044_Task::validP1char(char ch) {
  if (isAlphaNumeric(ch))
  {
    return true;
  }

  switch (ch) {
    case '.':
    case ' ':
    case '\\': // Single backslash, but escaped in C++
    case '\r':
    case '\n':
    case '(':
    case ')':
    case '-':
    case '*':
    case ':':
    case '_':
      return true;
  }
  return false;
}

void P044_Task::serialBegin(const ESPEasySerialPort port, int16_t rxPin, int16_t txPin,
                            unsigned long baud, uint8_t config) {
  serialEnd();

  if (rxPin >= 0) {
    P1EasySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin);

    if (nullptr != P1EasySerial) {
#if defined(ESP8266)
      P1EasySerial->begin(baud, (SerialConfig)config);
#elif defined(ESP32)
      P1EasySerial->begin(baud, config);
#endif // if defined(ESP8266)
      addLog(LOG_LEVEL_DEBUG, F("P1   : Serial opened"));
    }
  }
  state = ParserState::WAITING;
}

void P044_Task::serialEnd() {
  if (nullptr != P1EasySerial) {
    delete P1EasySerial;
    P1EasySerial = nullptr;
    addLog(LOG_LEVEL_DEBUG, F("P1   : Serial closed"));
  }
}

void P044_Task::handleSerialIn(struct EventStruct *event) {
  if (nullptr == P1EasySerial) { return; }
  int  RXWait  = P044_RX_WAIT;
  bool done    = false;
  int  timeOut = RXWait;

  do {
    if (P1EasySerial->available()) {
      digitalWrite(P044_STATUS_LED, 1);
      done = handleChar(P1EasySerial->read());
      digitalWrite(P044_STATUS_LED, 0);

      if (done) { break; }
      timeOut = RXWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) { break; }
      delay(1);
      --timeOut;
    }
  } while (true);

  if (done) {
    P1GatewayClient.print(serial_buffer);
    P1GatewayClient.flush();

    addLog(LOG_LEVEL_DEBUG, F("P1   : data send!"));
    blinkLED();

    if (Settings.UseRules)
    {
      LoadTaskSettings(event->TaskIndex);
      String eventString = getTaskDeviceName(event->TaskIndex);
      eventString += F("#Data");
      eventQueue.addMove(std::move(eventString));
    }
  } // done
}

bool P044_Task::handleChar(char ch) {
  if (serial_buffer.length() >= P044_DATAGRAM_MAX_SIZE - 2) { // room for cr/lf
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Buffer overflow, discarded input."));
    state = ParserState::WAITING;                             // reset
  }

  bool done    = false;
  bool invalid = false;

  switch (state) {
    case ParserState::WAITING:

      if (ch == P044_DATAGRAM_START_CHAR)  {
        clearBuffer();
        addChar(ch);
        state = ParserState::READING;
      } // else ignore data
      break;
    case ParserState::READING:

      if (validP1char(ch)) {
        addChar(ch);
      } else if (ch == P044_DATAGRAM_END_CHAR) {
        addChar(ch);

        if (CRCcheck) {
          checkI = 0;
          state  = ParserState::CHECKSUM;
        } else {
          done = true;
        }
      } else if (ch == P044_DATAGRAM_START_CHAR) {
        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Start detected, discarded input."));
        state = ParserState::WAITING; // reset
        return handleChar(ch);
      } else {
        invalid = true;
      }
      break;
    case ParserState::CHECKSUM:

      if (validP1char(ch)) {
        addChar(ch);
        ++checkI;

        if (checkI == P044_CHECKSUM_LENGTH) {
          done = true;
        }
      } else {
        invalid = true;
      }
      break;
  } // switch

  if (invalid) {
    // input is not a datagram char
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: DATA corrupt, discarded input."));

    #ifdef PLUGIN_044_DEBUG
      serialPrint(F("faulty char>"));
      serialPrint(String(ch));
      serialPrintln("<");
    #endif
    state = ParserState::WAITING; // reset
  }

  if (done) {
    done = checkDatagram();

    if (done) {
      // add the cr/lf pair to the datagram ahead of reading both
      // from serial as the datagram has already been validated
      addChar('\r');
      addChar('\n');
    } else if (CRCcheck) {
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid CRC, dropped data"));
    } else {
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid datagram, dropped data"));
    }
    state = ParserState::WAITING; // prepare for next one
  }

  return done;
}

void P044_Task::discardSerialIn() {
  if (nullptr != P1EasySerial) {
    while (P1EasySerial->available()) {
      P1EasySerial->read();
    }
  }
  state = ParserState::WAITING;
}

bool P044_Task::isInit() const {
  return nullptr != P1GatewayServer && nullptr != P1EasySerial;
}

#endif

#include "../PluginStructs/P079_data_struct.h"

#ifdef USES_P079



WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq)
  : _address(address)
{
  _use_STBY_IO = false;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  setfreq(freq);
}

WemosMotor::WemosMotor(uint8_t address, uint8_t motor, uint32_t freq, uint8_t STBY_IO)
  : _address(address)
{
  _use_STBY_IO = true;
  _STBY_IO     = STBY_IO;

  if (motor == P079_MOTOR_A) {
    _motor = P079_MOTOR_A;
  }
  else {
    _motor = P079_MOTOR_B;
  }

  setfreq(freq);

  pinMode(_STBY_IO, OUTPUT);
  digitalWrite(_STBY_IO, LOW);
}

/* setfreq() -- set PWM's frequency
   freq: PWM's frequency

   total 4bytes
 |0.5byte CMD     | 3.5byte Parm|
 |CMD             | parm        |
 |0x0X  set freq  | uint32  freq|
 */
void WemosMotor::setfreq(uint32_t freq)
{
  Wire.beginTransmission(_address);
  Wire.write(((uint8_t)(freq >> 24)) & (uint8_t)0x0f);
  Wire.write((uint8_t)(freq >> 16));
  Wire.write((uint8_t)(freq >> 8));
  Wire.write((uint8_t)freq);
  Wire.endTransmission(); // stop transmitting
  delay(0);
}

/* setmotor() -- set motor
   motor:
        P079_MOTOR_A    0   Motor A
        P079_MOTOR_B    1   Motor B

   dir:
        P079_SHORT_BRAKE  0
        P079_CCW          1
        P079_CW               2
        P079_STOP         3
        P079_STANDBY      4

   pwm_val:
        0.00 - 100.00  (%)

   total 4bytes
 |0.5byte CMD      | 3.5byte Parm         |
 |CMD              | parm                 |
 |0x10  set motorA | uint8 dir  uint16 pwm|
 |0x11  set motorB | uint8 dir  uint16 pwm|
 */
void WemosMotor::setmotor(uint8_t dir, float pwm_val)
{
  uint16_t _pwm_val;

  if (_use_STBY_IO == true) {
    if (dir == P079_STANDBY) {
      digitalWrite(_STBY_IO, LOW);
      return;
    } else {
      digitalWrite(_STBY_IO, HIGH);
    }
  }

  Wire.beginTransmission(_address);
  Wire.write(_motor | (uint8_t)0x10); // CMD either 0x10 or 0x11
  Wire.write(dir);

  // PWM in %
  _pwm_val = uint16_t(pwm_val * 100);

  if (_pwm_val > 10000) { // _pwm_val > 100.00
    _pwm_val = 10000;
  }

  Wire.write((uint8_t)(_pwm_val >> 8));
  Wire.write((uint8_t)_pwm_val);
  Wire.endTransmission(); // stop transmitting

  delay(0);
}

void WemosMotor::setmotor(uint8_t dir)
{
  setmotor(dir, 100);
}

LOLIN_I2C_MOTOR::LOLIN_I2C_MOTOR(uint8_t address) : _address(address) {}

/*
    Change Motor Status.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    sta: Motor Status
        MOTOR_STATUS_STOP
        MOTOR_STATUS_CCW
        MOTOR_STATUS_CW
        MOTOR_STATUS_SHORT_BRAKE
        MOTOR_STATUS_STANDBY
 */
unsigned char LOLIN_I2C_MOTOR::changeStatus(unsigned char ch, unsigned char sta)
{
  send_data[0] = CHANGE_STATUS;
  send_data[1] = ch;
  send_data[2] = sta;
  unsigned char result = sendData(send_data, 3);

  return result;
}

/*
    Change Motor Frequency
        ch: Motor Channel
            MOTOR_CH_A
            MOTOR_CH_B
            MOTOR_CH_BOTH

        freq: PWM frequency (Hz)
            1 - 80KHz, typically 1000Hz
 */
unsigned char LOLIN_I2C_MOTOR::changeFreq(unsigned char ch, uint32_t freq)
{
  send_data[0] = CHANGE_FREQ;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(freq & 0xff);
  send_data[3] = (uint8_t)((freq >> 8) & 0xff);
  send_data[4] = (uint8_t)((freq >> 16) & 0xff);
  unsigned char result = sendData(send_data, 5);

  return result;
}

/*
    Change Motor Duty.
    ch: Motor Channel
        MOTOR_CH_A
        MOTOR_CH_B
        MOTOR_CH_BOTH

    duty: PWM Duty (%)
        0.01 - 100.00 (%)
 */
unsigned char LOLIN_I2C_MOTOR::changeDuty(unsigned char ch, float duty)
{
  uint16_t _duty;

  _duty = (uint16_t)(duty * 100);

  send_data[0] = CHANGE_DUTY;
  send_data[1] = ch;

  send_data[2] = (uint8_t)(_duty & 0xff);
  send_data[3] = (uint8_t)((_duty >> 8) & 0xff);
  unsigned char result = sendData(send_data, 4);

  return result;
}

/*
    Reset Device.
 */
unsigned char LOLIN_I2C_MOTOR::reset()
{
  send_data[0] = RESET_SLAVE;
  unsigned char result = sendData(send_data, 1);

  return result;
}

/*
    Change Device I2C address
    address: when address=0, address>=127, will change address to default I2C address 0x31
 */
unsigned char LOLIN_I2C_MOTOR::changeAddress(unsigned char address)
{
  send_data[0] = CHANGE_I2C_ADDRESS;
  send_data[1] = address;
  unsigned char result = sendData(send_data, 2);

  return result;
}

/*
    Get PRODUCT_ID and Firmwave VERSION
 */
unsigned char LOLIN_I2C_MOTOR::getInfo(void)
{
  send_data[0] = GET_SLAVE_STATUS;
  unsigned char result = sendData(send_data, 1);

  if (result == 0)
  {
    PRODUCT_ID = get_data[0];
    VERSION_ID = get_data[1];
  }
  else
  {
    PRODUCT_ID = 0;
    VERSION_ID = 0;
  }

  return result;
}

/*
    Send and Get I2C Data
 */
unsigned char LOLIN_I2C_MOTOR::sendData(unsigned char *data, unsigned char len)
{
  unsigned char i;

  if ((_address == 0) || (_address >= 127))
  {
    return 1;
  }
  else
  {
    Wire.beginTransmission(_address);

    for (i = 0; i < len; i++) {
      Wire.write(data[i]);
    }
    Wire.endTransmission();
    delay(50);

    if (data[0] == GET_SLAVE_STATUS) {
      Wire.requestFrom(static_cast<int>(_address), 2);
    }
    else {
      Wire.requestFrom(static_cast<int>(_address), 1);
    }

    i = 0;

    while (Wire.available())
    {
      get_data[i] = Wire.read();
      i++;
    }

    return 0;
  }
}

#endif // ifdef USES_P079

#include "../PluginStructs/P106_data_struct.h"

#ifdef USES_P106


// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>


bool P106_data_struct::begin(uint8_t addr, bool initSettings)
{
  if (!initialized) {
    initialized = bme.begin(addr, initSettings);

    if (initialized) {
      // Set up oversampling and filter initialization
      bme.setTemperatureOversampling(BME680_OS_8X);
      bme.setHumidityOversampling(BME680_OS_2X);
      bme.setPressureOversampling(BME680_OS_4X);
      bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
      bme.setGasHeater(320, 150); // 320*C for 150 ms
    }
  }

  return initialized;
}

#endif // ifdef USES_P106

#include "../PluginStructs/P114_data_struct.h"

#ifdef USES_P114

// **************************************************************************/
// Constructor
// **************************************************************************/
P114_data_struct::P114_data_struct(uint8_t i2c_addr, uint8_t integration_time, bool highDensity)
  : i2cAddress(i2c_addr), IT(integration_time), HD(highDensity) {}


// **************************************************************************/
// Initialize sensor and read data from VEML6075
// **************************************************************************/
bool P114_data_struct::read_sensor(float& _UVA, float& _UVB, float& _UVIndex) {
  if (!initialised) {
    initialised = init_sensor(); // Check id device is present
  }

  String log;


  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    log.reserve(40);
    log  = F("VEML6075: i2caddress: 0x");
    log += String(i2cAddress, HEX);
    addLog(LOG_LEVEL_DEBUG, log);
    log  = F("VEML6075: initialized: ");
    log += String(initialised ? F("true") : F("false"));
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (initialised) {
    for (int j = 0; j < 5; j++) {
      UVData[j] = I2C_read16_LE_reg(i2cAddress, VEML6075_UVA_DATA + j);
    }

    // Calculate the UV Index, valid in open air not behind glass!
    UVAComp  = (UVData[0] - UVData[1]) - ACoef * (UVData[3] - UVData[1]) - BCoef * (UVData[4] - UVData[1]);
    UVBComp  = (UVData[2] - UVData[1]) - CCoef * (UVData[3] - UVData[1]) - DCoef * (UVData[4] - UVData[1]);
    _UVIndex = ((UVBComp * UVBresponsivity) +  (UVAComp * UVAresponsivity)) / 2.0f;

    _UVA = static_cast<float>(UVData[0]) / static_cast<float>(1 << (IT - 1)); // UVA light sensitivity increases linear with integration time
    _UVB = static_cast<float>(UVData[2]) / static_cast<float>(1 << (IT - 1)); // UVB light sensitivity increases linear with integration time

    // float UVASensitivity = 0.93/(static_cast<float>(IT + 1)); // UVA light sensitivity increases with integration time
    // float UVBSensitivity = 2.10/(static_cast<float>(IT + 1)); // UVB light sensitivity increases with integration time
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      log  = F("VEML6075: IT raw: 0x");
      log += String(IT + 1, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
    return true;
  }
  return false;
}

// **************************************************************************/
// Check VEML6075 presence and initialize
// **************************************************************************/
bool P114_data_struct::init_sensor() {
  uint16_t deviceID = I2C_readS16_LE_reg(i2cAddress, VEML6075_UV_ID);

  String log;


  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    log.reserve(60);
    log  = F("VEML6075: ID: 0x");
    log += String(deviceID, HEX);
    log += F(" / checked Address: 0x");
    log += String(i2cAddress, HEX);
    log += F(" / 0x");
    log += String(VEML6075_UV_ID, HEX);
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (deviceID != 0x26) {
    log.reserve(60);
    log  = F("VEML6075: wrong deviceID: ");
    log += String(deviceID, HEX);
    addLog(LOG_LEVEL_ERROR, log);
    return false;
  } else {
    log.reserve(60);

    // log  = F("VEML6075: found deviceID: 0x");
    // log += String(deviceID, HEX);

    if (!I2C_write16_LE_reg(i2cAddress, VEML6075_UV_CONF, (IT << 4) | (HD << 3))) { // Bit 3 must be 0, bit 0 is 0 for run and 1 for
      // shutdown, LS Byte
      log  = F("VEML6075: setup failed!!");
      log += F(" / CONF: ");
      log += String(static_cast<uint16_t>(IT << 4) | (HD << 3), BIN);
      addLog(LOG_LEVEL_ERROR, log);
      return false;
    } else if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("VEML6075: sensor initialised / CONF: ");
      log += String((uint16_t)(IT << 4) | (HD << 3), BIN);
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  return true;
}

#endif // ifdef USES_P114

#include "../PluginStructs/P025_data_struct.h"

#ifdef USES_P025

P025_data_struct::P025_data_struct(uint8_t i2c_addr, uint8_t _pga, uint8_t _mux) : pga(_pga), mux(_mux), i2cAddress(i2c_addr) {}

int16_t P025_data_struct::read() {
  uint16_t config = (0x0003)    | // Disable the comparator (default val)
                    (0x0000)    | // Non-latching (default val)
                    (0x0000)    | // Alert/Rdy active low   (default val)
                    (0x0000)    | // Traditional comparator (default val)
                    (0x0080)    | // 128 samples per second (default)
                    (0x0100);     // Single-shot mode (default)

  config |= static_cast<uint16_t>(pga) << 9;
  config |= static_cast<uint16_t>(mux) << 12;
  config |= (0x8000); // Start a single conversion

  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)(0x01));
  Wire.write((uint8_t)(config >> 8));
  Wire.write((uint8_t)(config & 0xFF));
  Wire.endTransmission();

  delay(9); // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
  return readRegister025(0x00);
}

uint16_t P025_data_struct::readRegister025(uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((0x00));
  Wire.endTransmission();

  if (Wire.requestFrom(i2cAddress, (uint8_t)2) != 2) {
    return 0x8000;
  }
  return (Wire.read() << 8) | Wire.read();
}

#endif // ifdef USES_P025

#include "../PluginStructs/P099_data_struct.h"

#ifdef USES_P099
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/SystemVariables.h"



#include <XPT2046_Touchscreen.h>

P099_data_struct::P099_data_struct() : touchscreen(nullptr) {}

P099_data_struct::~P099_data_struct() {
  reset();
}

/**
 * Proper reset and cleanup.
 */
void P099_data_struct::reset() {
  if (touchscreen != nullptr) {
    delete touchscreen;
    touchscreen = nullptr;
  }
#ifdef PLUGIN_099_DEBUG
  addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen reset."));
#endif // PLUGIN_099_DEBUG
}

/**
 * Initialize data and set up the touchscreen.
 */
bool P099_data_struct::init(taskIndex_t taskIndex,
                            uint8_t     cs,
                            uint8_t     rotation,
                            bool        flipped,
                            uint8_t     z_treshold,
                            bool        send_xy,
                            bool        send_z,
                            bool        useCalibration,
                            uint16_t    ts_x_res,
                            uint16_t    ts_y_res) {
  reset();

  _address_ts_cs  = cs;
  _z_treshold     = z_treshold;
  _rotation       = rotation;
  _flipped        = flipped;
  _send_xy        = send_xy;
  _send_z         = send_z;
  _useCalibration = useCalibration;
  _ts_x_res       = ts_x_res;
  _ts_y_res       = ts_y_res;

  touchscreen = new (std::nothrow) XPT2046_Touchscreen(_address_ts_cs);
  if (touchscreen != nullptr) {
    touchscreen->setRotation(_rotation);
    touchscreen->setRotationFlipped(_flipped);
    touchscreen->begin();
    loadTouchObjects(taskIndex);
#ifdef PLUGIN_099_DEBUG
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG Plugin & touchscreen initialized."));
   } else {
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG Touchscreen initialisation FAILED."));
#endif // PLUGIN_099_DEBUG
  }
  return isInitialized();
}

/**
 * Properly initialized? then true
 */
bool P099_data_struct::isInitialized() const {
  return touchscreen != nullptr;
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P099_data_struct::loadTouchObjects(taskIndex_t taskIndex) {
#ifdef PLUGIN_099_DEBUG
  String log = F("P099 DEBUG loadTouchObjects size: ");
  log += sizeof(StoredSettings);
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&StoredSettings), sizeof(StoredSettings));

  for (int i = 0; i < P099_MaxObjectCount; i++) {
    StoredSettings.TouchObjects[i].objectname[P099_MaxObjectNameLength - 1] = 0; // Terminate strings in case of uninitialized data
    SurfaceAreas[i] = 0; // Reset
    TouchTimers[i]  = 0;
    TouchStates[i]  = false;
  }
}

/**
 * Check if the screen is touched.
 */
bool P099_data_struct::touched() {
  if (isInitialized()) {
    return touchscreen->touched();
  } 
  return false;
}

/**
 * Read the raw data if the touchscreen is initialized.
 */
void P099_data_struct::readData(uint16_t *x, uint16_t *y, uint8_t *z) {
  if (isInitialized()) {
    touchscreen->readData(x, y, z);
#ifdef PLUGIN_099_DEBUG
    addLog(LOG_LEVEL_INFO, F("P099 DEBUG readData"));
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotation if the touchscreen is initialized.
 */
void P099_data_struct::setRotation(uint8_t n) {
  if (isInitialized()) {
    touchscreen->setRotation(n);
#ifdef PLUGIN_099_DEBUG
    String log = F("P099 DEBUG Rotation set: ");
    log += n;
    addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Only set rotationFlipped if the touchscreen is initialized.
 */
void P099_data_struct::setRotationFlipped(bool flipped) {
  if (isInitialized()) {
    touchscreen->setRotationFlipped(flipped);
#ifdef PLUGIN_099_DEBUG
    String log = F("P099 DEBUG RotationFlipped set: ");
    log += flipped;
    addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
  }
}

/**
 * Determine if calibration is enabled and usable.
 */
bool P099_data_struct::isCalibrationActive() {
  return    _useCalibration
         && StoredSettings.Calibration.top_left.x > 0
         && StoredSettings.Calibration.top_left.y > 0
         && StoredSettings.Calibration.bottom_right.x > 0
         && StoredSettings.Calibration.bottom_right.y > 0; // Enabled and all values != 0 => Active
}

/**
 * Check within the list of defined objects if we touched one of them.
 * The smallest matching surface is selected if multiple objects overlap.
 * Returns state, and sets selectedObjectName to the best matching object
 */
bool P099_data_struct::isValidAndTouchedTouchObject(uint16_t x, uint16_t y, String &selectedObjectName, int &selectedObjectIndex, uint8_t checkObjectCount) {
  uint32_t lastObjectArea = 0;
  bool     selected = false;
  for (uint8_t objectNr = 0; objectNr < checkObjectCount; objectNr++) {
    String objectName = String(StoredSettings.TouchObjects[objectNr].objectname);
    if ( objectName.length() > 0
      && objectName.substring(0,1 ) != F("_")         // Ignore if name starts with an underscore
      && StoredSettings.TouchObjects[objectNr].bottom_right.x > 0
      && StoredSettings.TouchObjects[objectNr].bottom_right.y > 0) { // Not initial could be valid

      if (SurfaceAreas[objectNr] == 0) { // Need to calculate the surface area
        SurfaceAreas[objectNr] = (StoredSettings.TouchObjects[objectNr].bottom_right.x - StoredSettings.TouchObjects[objectNr].top_left.x) * (StoredSettings.TouchObjects[objectNr].bottom_right.y - StoredSettings.TouchObjects[objectNr].top_left.y);
      }

      if ( StoredSettings.TouchObjects[objectNr].top_left.x <= x
        && StoredSettings.TouchObjects[objectNr].top_left.y <= y
        && StoredSettings.TouchObjects[objectNr].bottom_right.x >= x
        && StoredSettings.TouchObjects[objectNr].bottom_right.y >= y
        && (lastObjectArea == 0 
          || SurfaceAreas[objectNr] < lastObjectArea)) { // Select smallest area that fits the coordinates
        selectedObjectName  = objectName;
        selectedObjectIndex = objectNr;
        lastObjectArea      = SurfaceAreas[objectNr];
        selected            = true;
      }
#ifdef PLUGIN_099_DEBUG
      String log = F("P099 DEBUG Touched: obj: ");
      log += objectName;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].top_left.x;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].top_left.y;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].bottom_right.x;
      log += ',';
      log += StoredSettings.TouchObjects[objectNr].bottom_right.y;
      log += F(" surface:");
      log += SurfaceAreas[objectNr];
      log += F(" x,y:");
      log += x;
      log += ',';
      log += y;
      log += F(" sel:");
      log += selectedObjectName;
      log += '/';
      log += selectedObjectIndex;
      addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
    }
  }
  return selected;
}

/**
 * Set the enabled/disabled state by inserting or deleting an underscore '_' as the first character of the object name.
 * Checks if the name doesn't exceed the max. length.
 */
bool P099_data_struct::setTouchObjectState(String touchObject, bool state, uint8_t checkObjectCount) {
  if (touchObject.isEmpty() || touchObject.substring(0, 1) == F("_")) return false;
  String findObject = (state ? F("_") : F("")); // When enabling, try to find a disabled object
  findObject += touchObject;
  String thisObject;
  bool   success = false;
  thisObject.reserve(P099_MaxObjectNameLength);
  for (uint8_t objectNr = 0; objectNr < checkObjectCount; objectNr++) {
    thisObject = String(StoredSettings.TouchObjects[objectNr].objectname);
    if (thisObject.length() > 0 && findObject.equalsIgnoreCase(thisObject)) {
      if (state) {
        success = safe_strncpy(StoredSettings.TouchObjects[objectNr].objectname, thisObject.substring(1), P099_MaxObjectNameLength); // Keep original character casing
      } else {
        if (thisObject.length() < P099_MaxObjectNameLength - 2) { // Leave room for the underscore and the terminating 0.
          String disabledObject = F("_");
          disabledObject += thisObject;
          success = safe_strncpy(StoredSettings.TouchObjects[objectNr].objectname, disabledObject, P099_MaxObjectNameLength);
        }
      }
      StoredSettings.TouchObjects[objectNr].objectname[P099_MaxObjectNameLength - 1] = 0; // Just to be safe
#ifdef PLUGIN_099_DEBUG
      String log = F("P099 setTouchObjectState: obj: ");
      log += thisObject;
      if (success) {
      log += F(", new state: ");
      log += (state ? F("en") : F("dis"));
      log += F("abled.");
      } else {
        log += F("failed!");
      }
      addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_099_DEBUG
      // break; // Only first one found is processed
    }
  }
  return success;
}

/**
 * Scale the provided raw coordinates to screen-resolution coordinates if calibration is enabled/configured
 */
void P099_data_struct::scaleRawToCalibrated(uint16_t &x, uint16_t &y) {
  if (isCalibrationActive()) {
    uint16_t lx = x - StoredSettings.Calibration.top_left.x;
    if (lx <= 0) {
      x = 0;
    } else {
      if (lx > StoredSettings.Calibration.bottom_right.x) {
        lx = StoredSettings.Calibration.bottom_right.x;
      }
      float x_fact = static_cast<float>(StoredSettings.Calibration.bottom_right.x - StoredSettings.Calibration.top_left.x) / static_cast<float>(_ts_x_res);
      x = static_cast<uint16_t>(round(lx / x_fact));
    }
    uint16_t ly = y - StoredSettings.Calibration.top_left.y;
    if (ly <= 0) {
      y = 0;
    } else {
      if (ly > StoredSettings.Calibration.bottom_right.y) {
        ly = StoredSettings.Calibration.bottom_right.y;
      }
      float y_fact = (StoredSettings.Calibration.bottom_right.y - StoredSettings.Calibration.top_left.y) / _ts_y_res;
      y = static_cast<uint16_t>(round(ly / y_fact));
    }
  }
}

#endif  // ifdef USES_P099
#include "../PluginStructs/P104_data_struct.h"

#ifdef USES_P104

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Numerical.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/WebServer.h"
# include "../WebServer/Markup.h"
# include "../WebServer/HTML_wrappers.h"
# include "../ESPEasyCore/ESPEasyRules.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/RTC.h"

# include <vector>
# include <MD_Parola.h>
# include <MD_MAX72xx.h>

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string); // Forward definition
# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void reverseStr(String& str);       // Forward definition

/****************************************************************
 * Constructor
 ***************************************************************/
P104_data_struct::P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                                   taskIndex_t              _taskIndex,
                                   int8_t                   _cs_pin,
                                   uint8_t                  _modules,
                                   uint8_t                  _zonesCount)
  : mod(_mod), taskIndex(_taskIndex), cs_pin(_cs_pin), modules(_modules), expectedZones(_zonesCount) {
  if (Settings.isSPI_valid()) {
    P = new (std::nothrow) MD_Parola(mod, cs_pin, modules);
  } else {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Required SPI not enabled. Initialization aborted!"));
  }
}

/*******************************
 * Destructor
 ******************************/
P104_data_struct::~P104_data_struct() {
  # ifdef P104_USE_BAR_GRAPH

  if (nullptr != pM) {
    pM = nullptr; // Not created here, only reset
  }
  # endif // ifdef P104_USE_BAR_GRAPH

  if (nullptr != P) {
    // P->~MD_Parola(); // Call destructor directly, as delete of the object fails miserably
    // do not: delete P; // Warning: the MD_Parola object doesn't have a virtual destructor, and when changed,
    // a reboot uccurs when the object is deleted here!
    P = nullptr; // Reset only
  }
}

/*******************************
 * Initializer/starter
 ******************************/
bool P104_data_struct::begin() {
  if (!initialized) {
    loadSettings();
    initialized = true;
  }

  if ((P != nullptr) && (cs_pin > -1)) {
    # ifdef P104_DEBUG
    addLog(LOG_LEVEL_INFO, F("dotmatrix: begin() called"));
    # endif // ifdef P104_DEBUG
    P->begin(expectedZones);
    # ifdef P104_USE_BAR_GRAPH
    pM = P->getGraphicObject();
    # endif // ifdef P104_USE_BAR_GRAPH
    return true;
  }
  return false;
}

# define P104_ZONE_SEP   '\x02'
# define P104_FIELD_SEP  '\x01'
# define P104_ZONE_DISP  ';'
# define P104_FIELD_DISP ','

# define P104_CONFIG_VERSION_V2  0xF000 // Marker in first uint16_t to to indicate second version config settings, anything else if first
                                        // version.
                                        // Any third version or later could use 0xE000, etc. The 'version' is stored in the first uint16_t
                                        // stored in the custom settings

/*
   Settings layout:
   Version 1:
   - uint16_t : size of the next blob holding all settings
   - char[x]  : Blob with settings, with csv-like strings, using P104_FIELD_SEP and P104_ZONE_SEP separators
   Version 2:
   - uint16_t : marker with content P104_CONFIG_VERSION_V2
   - uint16_t : size of next blob holding 1 zone settings string
   - char[y]  : Blob holding 1 zone settings string, with csv like string, using P104_FIELD_SEP separators
   - uint16_t : next size, if 0 then no more blobs
   - char[x]  : Blob
   - ...
   - Max. allowed total custom settings size = 1024
 */
/**************************************
 * loadSettings
 *************************************/
void P104_data_struct::loadSettings() {
  uint16_t bufferSize;
  char    *settingsBuffer;

  if (taskIndex < TASKS_MAX) {
    int loadOffset = 0;

    // Read size of the used buffer, could be the settings-version marker
    LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)&bufferSize, sizeof(bufferSize), loadOffset);
    bool settingsVersionV2  = (bufferSize == P104_CONFIG_VERSION_V2) || (bufferSize == 0u);
    uint16_t structDataSize = 0;
    uint16_t reservedBuffer = 0;

    if (!settingsVersionV2) {
      reservedBuffer = bufferSize + 1;              // just add 1 for storing a string-terminator
      addLog(LOG_LEVEL_INFO, F("dotmatrix: Reading Settings V1, will be stored as Settings V2."));
    } else {
      reservedBuffer = P104_SETTINGS_BUFFER_V2 + 1; // just add 1 for storing a string-terminator
    }
    reservedBuffer++;                               // Add 1 for 0..size use
    settingsBuffer = new char[reservedBuffer]();    // Allocate buffer and reset to all zeroes
    loadOffset    += sizeof(bufferSize);

    if (settingsVersionV2) {
      LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)&bufferSize, sizeof(bufferSize), loadOffset);
      loadOffset += sizeof(bufferSize); // Skip the size
    }
    structDataSize = bufferSize;
    # ifdef P104_DEBUG_DEV
    String log;

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(54)) {
      log  = F("P104: loadSettings stored Size: ");
      log += structDataSize;
      log += F(" taskindex: ");
      log += taskIndex;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV

    // Read actual data
    if (structDataSize > 0) { // Reading 0 bytes logs an error, so lets avoid that
      LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)settingsBuffer, structDataSize, loadOffset);
    }
    settingsBuffer[bufferSize + 1] = '\0'; // Terminate string

    uint8_t zoneIndex = 0;

    {
      String buffer;
      buffer.reserve(bufferSize + 1);
      buffer = String(settingsBuffer);
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: loadSettings bufferSize: ");
        log += bufferSize;
        log += F(" untrimmed: ");
        log += buffer.length();
      }
      # endif // ifdef P104_DEBUG_DEV
      buffer.trim();
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" trimmed: ");
        log += buffer.length();
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG_DEV

      if (zones.size() > 0) {
        zones.clear();
      }
      zones.reserve(P104_MAX_ZONES);
      numDevices = 0;

      String   tmp;
      String   fld;
      int      tmp_int;
      uint16_t prev2   = 0;
      int16_t  offset2 = buffer.indexOf(P104_ZONE_SEP);

      if ((offset2 == -1) && (buffer.length() > 0)) {
        offset2 = buffer.length();
      }

      while (offset2 > -1) {
        tmp.reserve(offset2 - prev2);
        tmp = buffer.substring(prev2, offset2);
        # ifdef P104_DEBUG_DEV

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          log  = F("P104: reading string: ");
          log += tmp;
          log.replace(P104_FIELD_SEP, P104_FIELD_DISP);
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P104_DEBUG_DEV

        zones.push_back(P104_zone_struct(zoneIndex + 1));

        tmp_int = 0;

        // WARNING: Order of parsing these values should match the numeric order of P104_OFFSET_* values

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SIZE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].size = tmp_int;
        }

        zones[zoneIndex].text = parseStringKeepCase(tmp, 1 + P104_OFFSET_TEXT, P104_FIELD_SEP);

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ALIGNMENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].alignment = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_IN, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationIn = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEED, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].speed = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_OUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationOut = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_PAUSE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].pause = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_FONT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].font = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_CONTENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].content = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_LAYOUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].layout = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEC_EFFECT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].specialEffect = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_OFFSET, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].offset = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_BRIGHTNESS, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].brightness = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_REPEATDELAY, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].repeatDelay = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_INVERTED, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].inverted = tmp_int;
        }

        delay(0);

        numDevices += zones[zoneIndex].size + zones[zoneIndex].offset;

        if (!settingsVersionV2) {
          prev2   = offset2 + 1;
          offset2 = buffer.indexOf(P104_ZONE_SEP, prev2);
        } else {
          loadOffset    += bufferSize;
          structDataSize = sizeof(bufferSize);
          LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)&bufferSize, structDataSize, loadOffset);
          offset2 = bufferSize;  // Length

          if (bufferSize == 0) { // End of zones reached
            offset2 = -1;        // fall out of while loop
          } else {
            structDataSize = bufferSize;
            loadOffset    += sizeof(bufferSize);
            LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)settingsBuffer, structDataSize, loadOffset);
            settingsBuffer[bufferSize + 1] = '\0'; // Terminate string
            buffer.reserve(bufferSize + 1);
            buffer = String(settingsBuffer);
          }
        }
        zoneIndex++;

        # ifdef P104_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log  = F("dotmatrix: parsed zone: ");
          log += zoneIndex;
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P104_DEBUG
      }

      buffer.clear();        // Free some memory
    }

    delete[] settingsBuffer; // Release allocated buffer
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: read zones from config: ");
      log += zoneIndex;

      // log += F(" struct size: ");
      // log += sizeof(P104_zone_struct);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV

    if (expectedZones == -1) { expectedZones = zoneIndex; }

    if (expectedZones == 0) { expectedZones++; } // Guarantee at least 1 zone to be displayed

    while (zoneIndex < expectedZones) {
      zones.push_back(P104_zone_struct(zoneIndex + 1));

      if (zones[zoneIndex].text == F("\"\"")) { // Special case
        zones[zoneIndex].text = EMPTY_STRING;
      }

      zoneIndex++;
      delay(0);
    }
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: total zones initialized: ");
      log += zoneIndex;
      log += F(" expected: ");
      log += expectedZones;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV
  }
}

/****************************************************
 * configureZones: initialize Zones setup
 ***************************************************/
void P104_data_struct::configureZones() {
  if (!initialized) {
    loadSettings();
    initialized = true;
  }

  uint8_t currentZone = 0;
  uint8_t zoneOffset  = 0;

  # ifdef P104_DEBUG_DEV
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(45)) {
    log  = F("P104: configureZones to do: ");
    log += zones.size();
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG_DEV

  if (nullptr == P) { return; }

  P->displayClear();

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    if (it->zone <= expectedZones) {
      zoneOffset += it->offset;
      P->setZone(currentZone, zoneOffset, zoneOffset + it->size - 1);
      # ifdef P104_USE_BAR_GRAPH
      it->_startModule = zoneOffset;
      P->getDisplayExtent(currentZone, it->_lower, it->_upper);
      # endif // ifdef P104_USE_BAR_GRAPH
      zoneOffset += it->size;

      switch (it->font) {
        # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        case P104_DOUBLE_HEIGHT_FONT_ID: {
          P->setFont(currentZone, numeric7SegDouble);
          P->setCharSpacing(currentZone, P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        case P104_FULL_DOUBLEHEIGHT_FONT_ID: {
          P->setFont(currentZone, BigFont);
          P->setCharSpacing(currentZone, P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_VERTICAL_FONT
        case P104_VERTICAL_FONT_ID: {
          P->setFont(currentZone, _fontVertical);
          break;
        }
        # endif // ifdef P104_USE_VERTICAL_FONT
        # ifdef P104_USE_EXT_ASCII_FONT
        case P104_EXT_ASCII_FONT_ID: {
          P->setFont(currentZone, ExtASCII);
          break;
        }
        # endif // ifdef P104_USE_EXT_ASCII_FONT
        # ifdef P104_USE_ARABIC_FONT
        case P104_ARABIC_FONT_ID: {
          P->setFont(currentZone, fontArabic);
          break;
        }
        # endif // ifdef P104_USE_ARABIC_FONT
        # ifdef P104_USE_GREEK_FONT
        case P104_GREEK_FONT_ID: {
          P->setFont(currentZone, fontGreek);
          break;
        }
        # endif // ifdef P104_USE_GREEK_FONT
        # ifdef P104_USE_KATAKANA_FONT
        case P104_KATAKANA_FONT_ID: {
          P->setFont(currentZone, fontKatakana);
          break;
        }
        # endif // ifdef P104_USE_KATAKANA_FONT

        // Extend above this comment with more fonts if/when available,
        // case P104_DEFAULT_FONT_ID: and default: clauses should be the last options.
        // This should also make sure the default font is set if a no longer available font was selected
        case P104_DEFAULT_FONT_ID:
        default: {
          P->setFont(currentZone, nullptr); // default font
          break;
        }
      }

      // Inverted
      P->setInvert(currentZone, it->inverted);

      // Special Effects
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_UP_DOWN) == P104_SPECIAL_EFFECT_UP_DOWN,       PA_FLIP_UD);
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_LEFT_RIGHT) == P104_SPECIAL_EFFECT_LEFT_RIGHT, PA_FLIP_LR);

      // Brightness
      P->setIntensity(currentZone, it->brightness);

      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: configureZones #");
        log += (currentZone + 1);
        log += '/';
        log += expectedZones;
        log += F(" offset: ");
        log += zoneOffset;
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG_DEV

      delay(0);

      // Content == text && text != ""
      if (((it->content == P104_CONTENT_TEXT) ||
           (it->content == P104_CONTENT_TEXT_REV))
          && (!it->text.isEmpty())) {
        displayOneZoneText(currentZone, *it, it->text);
      }

      # ifdef P104_USE_BAR_GRAPH

      // Content == Bar-graph && text != ""
      if ((it->content == P104_CONTENT_BAR_GRAPH)
          && (!it->text.isEmpty())) {
        displayBarGraph(currentZone, *it, it->text);
      }
      # endif // ifdef P104_USE_BAR_GRAPH

      if (it->repeatDelay > -1) {
        it->_repeatTimer = millis();
      }
      currentZone++;
      delay(0);
    }
  }

  // Synchronize the start
  P->synchZoneStart();
}

/**********************************************************
 * Display the text with attributes for a specific zone
 *********************************************************/
void P104_data_struct::displayOneZoneText(uint8_t                 zone,
                                          const P104_zone_struct& zstruct,
                                          const String          & text) {
  if ((nullptr == P) || (zone >= P104_MAX_ZONES)) { return; } // double check
  sZoneInitial[zone].reserve(text.length());
  sZoneInitial[zone] = text; // Keep the original string for future use
  sZoneBuffers[zone].reserve(text.length());
  sZoneBuffers[zone] = text; // We explicitly want a copy here so it can be modified by parseTemplate()

  sZoneBuffers[zone] = parseTemplate(sZoneBuffers[zone]);

  # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  if (zstruct.layout == P104_LAYOUT_DOUBLE_UPPER) {
    createHString(sZoneBuffers[zone]);
  }
  # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  if (zstruct.content == P104_CONTENT_TEXT_REV) {
    reverseStr(sZoneBuffers[zone]);
  }

  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      logAllText &&
      log.reserve(28 + text.length() + sZoneBuffers[zone].length())) {
    log  = F("dotmatrix: ZoneText: ");
    log += zone + 1; // UI-number
    log += F(", '");
    log += text;
    log += F("' -> '");
    log += sZoneBuffers[zone];
    log += '\'';
    addLog(LOG_LEVEL_INFO, log);
  }

  P->displayZoneText(zone,
                     sZoneBuffers[zone].c_str(),
                     static_cast<textPosition_t>(zstruct.alignment),
                     zstruct.speed,
                     zstruct.pause,
                     static_cast<textEffect_t>(zstruct.animationIn),
                     static_cast<textEffect_t>(zstruct.animationOut));
}

/*********************************************
 * Update all or the specified zone
 ********************************************/
void P104_data_struct::updateZone(uint8_t                 zone,
                                  const P104_zone_struct& zstruct) {
  if (nullptr == P) { return; }

  if (zone == 0) {
    for (auto it = zones.begin(); it != zones.end(); ++it) {
      if ((it->zone > 0) &&
          ((it->content == P104_CONTENT_TEXT) ||
           (it->content == P104_CONTENT_TEXT_REV))) {
        displayOneZoneText(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed text
        P->displayReset(it->zone - 1);
      }
      # ifdef P104_USE_BAR_GRAPH

      if ((it->zone > 0) &&
          (it->content == P104_CONTENT_BAR_GRAPH)) {
        displayBarGraph(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed bar graph
      }
      # endif // ifdef P104_USE_BAR_GRAPH

      if ((zstruct.content == P104_CONTENT_TEXT)
          || zstruct.content == P104_CONTENT_TEXT_REV
          # ifdef P104_USE_BAR_GRAPH
          || zstruct.content == P104_CONTENT_BAR_GRAPH
          # endif // ifdef P104_USE_BAR_GRAPH
          ) {
        if (it->repeatDelay > -1) { // Restart repeat timer
          it->_repeatTimer = millis();
        }
      }
    }
  } else {
    if ((zstruct.zone > 0) &&
        ((zstruct.content == P104_CONTENT_TEXT) ||
         (zstruct.content == P104_CONTENT_TEXT_REV))) {
      displayOneZoneText(zstruct.zone - 1, zstruct, sZoneInitial[zstruct.zone - 1]); // Re-send last displayed text
      P->displayReset(zstruct.zone - 1);
    }
    # ifdef P104_USE_BAR_GRAPH

    if ((zstruct.zone > 0) &&
        (zstruct.content == P104_CONTENT_BAR_GRAPH)) {
      displayBarGraph(zstruct.zone - 1, zstruct, sZoneInitial[zstruct.zone - 1]); // Re-send last displayed bar graph
    }
    # endif // ifdef P104_USE_BAR_GRAPH

    // Repeat timer is/should be started elsewhere
  }
}

# ifdef P104_USE_BAR_GRAPH

/***********************************************
 * Enable/Disable updating a range of modules
 **********************************************/
void P104_data_struct::modulesOnOff(uint8_t start, uint8_t end, MD_MAX72XX::controlValue_t on_off) {
  for (uint8_t m = start; m <= end; m++) {
    pM->control(m, MD_MAX72XX::UPDATE, on_off);
  }
}

/********************************************************
 * draw a single bar-graph, arguments already adjusted for direction
 *******************************************************/
void P104_data_struct::drawOneBarGraph(uint16_t lower,
                                       uint16_t upper,
                                       int16_t  pixBottom,
                                       int16_t  pixTop,
                                       uint16_t zeroPoint,
                                       uint8_t  barWidth,
                                       uint8_t  barType,
                                       uint8_t  row) {
  bool on_off;

  for (uint8_t r = 0; r < barWidth; r++) {
    for (uint8_t col = lower; col <= upper; col++) {
      on_off = (col >= pixBottom && col <= pixTop); // valid area

      if ((zeroPoint != 0) &&
          (barType == P104_BARTYPE_STANDARD) &&
          (barWidth > 2) &&
          ((r == 0) || (r == barWidth - 1)) &&
          (col == lower + zeroPoint)) {
        on_off = false; // when bar wider than 2, turn off zeropoint top and bottom led
      }

      if ((barType == P104_BARTYPE_SINGLE) && (r > 0)) {
        on_off = false; // barType 1 = only a single line is drawn, independent of the width
      }

      if ((barType == P104_BARTYPE_ALT_DOT) && (barWidth > 1) && on_off) {
        on_off = ((r % 2) == (col % 2)); // barType 2 = dotted line when bar is wider than 1 pixel
      }
      pM->setPoint(row + r, col, on_off);

      if (col % 16 == 0) { delay(0); }
    }
    delay(0); // Leave some breathingroom
  }
}

/********************************************************************
 * Process a graph-string to display in a zone, format:
 * value,max-value,min-value,direction,bartype|...
 *******************************************************************/
void P104_data_struct::displayBarGraph(uint8_t                 zone,
                                       const P104_zone_struct& zstruct,
                                       const String          & graph) {
  if ((nullptr == P) || (nullptr == pM) || graph.isEmpty()) { return; }
  sZoneInitial[zone] = graph; // Keep the original string for future use

  #  define NOT_A_COMMA 0x02  // Something else than a comma, or the parseString function will get confused
  String parsedGraph = graph; // Extra copy created so we don't mess up the incoming String
  parsedGraph = parseTemplate(parsedGraph);
  parsedGraph.replace(',', NOT_A_COMMA);

  std::vector<P104_bargraph_struct> barGraphs;
  uint8_t currentBar = 0;
  bool    loop       = true;

  // Parse the graph-string
  while (loop && currentBar < 8) { // Maximum 8 valuesets possible
    String graphpart = parseString(parsedGraph, currentBar + 1, '|');
    graphpart.trim();
    graphpart.replace(NOT_A_COMMA, ',');

    if (graphpart.isEmpty()) {
      loop = false;
    } else {
      barGraphs.push_back(P104_bargraph_struct(currentBar));
    }

    if (loop && validDoubleFromString(parseString(graphpart, 1), barGraphs[currentBar].value)) { // value
      String datapart = parseString(graphpart, 2);                                               // max (default: 100.0)

      if (datapart.isEmpty()) {
        barGraphs[currentBar].max = 100.0;
      } else {
        validDoubleFromString(datapart, barGraphs[currentBar].max);
      }
      datapart = parseString(graphpart, 3); // min (default: 0.0)

      if (datapart.isEmpty()) {
        barGraphs[currentBar].min = 0.0;
      } else {
        validDoubleFromString(datapart, barGraphs[currentBar].min);
      }
      datapart = parseString(graphpart, 4); // direction

      if (datapart.isEmpty()) {
        barGraphs[currentBar].direction = 0;
      } else {
        int value = 0;
        validIntFromString(datapart, value);
        barGraphs[currentBar].direction = value;
      }
      datapart = parseString(graphpart, 5); // barType

      if (datapart.isEmpty()) {
        barGraphs[currentBar].barType = 0;
      } else {
        int value = 0;
        validIntFromString(datapart, value);
        barGraphs[currentBar].barType = value;
      }

      if (definitelyGreaterThan(barGraphs[currentBar].min, barGraphs[currentBar].max)) {
        std::swap(barGraphs[currentBar].min, barGraphs[currentBar].max);
      }
    }
    #  ifdef P104_DEBUG

    if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(70);
      log = F("dotmatrix: Bar-graph: ");

      if (loop) {
        log += currentBar;
        log += F(" in: ");
        log += graphpart;
        log += F(" value: ");
        log += barGraphs[currentBar].value;
        log += F(" max: ");
        log += barGraphs[currentBar].max;
        log += F(" min: ");
        log += barGraphs[currentBar].min;
        log += F(" dir: ");
        log += barGraphs[currentBar].direction;
        log += F(" typ: ");
        log += barGraphs[currentBar].barType;
      } else {
        log += F(" bsize: ");
        log += barGraphs.size();
      }
      addLog(LOG_LEVEL_INFO, log);
    }
    #  endif // ifdef P104_DEBUG
    currentBar++; // next
    delay(0); // Leave some breathingroom
  }
  #  undef NOT_A_COMMA

  if (barGraphs.size() > 0) {
    uint8_t  barWidth = 8 / barGraphs.size(); // Divide the 8 pixel width per number of bars to show
    int16_t  pixTop, pixBottom;
    uint16_t zeroPoint;
    #  ifdef P104_DEBUG
    String log;

    if (logAllText &&
        loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(64)) {
      log  = F("dotmatrix: bar Width: ");
      log += barWidth;
      log += F(" low: ");
      log += zstruct._lower;
      log += F(" high: ");
      log += zstruct._upper;
    }
    #  endif // ifdef P104_DEBUG
    modulesOnOff(zstruct._startModule, zstruct._startModule + zstruct.size - 1, MD_MAX72XX::MD_OFF); // Stop updates on modules
    P->setIntensity(zstruct.zone - 1, zstruct.brightness); // don't forget to set the brightness
    uint8_t row = 0;

    if ((barGraphs.size() == 3) || (barGraphs.size() == 5) || (barGraphs.size() == 6)) { // Center within the rows a bit
      for (; row < (barGraphs.size() == 5 ? 2 : 1); row++) {
        for (uint8_t col = zstruct._lower; col <= zstruct._upper; col++) {
          pM->setPoint(row, col, false);                                                 // all off

          if (col % 16 == 0) { delay(0); }
        }
        delay(0); // Leave some breathingroom
      }
    }

    for (auto it = barGraphs.begin(); it != barGraphs.end(); ++it) {
      if (essentiallyEqual(it->min, 0.0)) {
        pixTop    = zstruct._lower - 1 + (((zstruct._upper + 1) - zstruct._lower) / it->max) * it->value;
        pixBottom = zstruct._lower - 1;
        zeroPoint = 0;
      } else {
        if (definitelyLessThan(it->min, 0.0) &&
            definitelyGreaterThan(it->max,           0.0) &&
            definitelyGreaterThan(it->max - it->min, 0.01)) { // Zero-point is used
          zeroPoint = (it->min * -1.0) / ((it->max - it->min) / (1.0 * ((zstruct._upper + 1) - zstruct._lower)));
        } else {
          zeroPoint = 0;
        }
        pixTop    = zstruct._lower + zeroPoint + (((zstruct._upper + 1) - zstruct._lower) / (it->max - it->min)) * it->value;
        pixBottom = zstruct._lower + zeroPoint;

        if (definitelyLessThan(it->value, 0.0)) {
          std::swap(pixTop, pixBottom);
        }
      }

      if (it->direction == 1) { // Left to right display: Flip values within the lower/upper range
        pixBottom = zstruct._upper - (pixBottom - zstruct._lower);
        pixTop    = zstruct._lower + (zstruct._upper - pixTop);
        std::swap(pixBottom, pixTop);
        zeroPoint = zstruct._upper - zstruct._lower - zeroPoint + (zeroPoint == 0 ? 1 : 0);
      }
      #  ifdef P104_DEBUG_DEV

      if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" B: ");
        log += pixBottom;
        log += F(" T: ");
        log += pixTop;
        log += F(" Z: ");
        log += zeroPoint;
      }
      #  endif // ifdef P104_DEBUG_DEV
      drawOneBarGraph(zstruct._lower, zstruct._upper, pixBottom, pixTop, zeroPoint, barWidth, it->barType, row);
      row += barWidth; // Next set of rows
      delay(0); // Leave some breathingroom
    }

    for (; row < 8; row++) {           // Clear unused rows
      for (uint8_t col = zstruct._lower; col <= zstruct._upper; col++) {
        pM->setPoint(row, col, false); // all off

        if (col % 16 == 0) { delay(0); }
      }
      delay(0); // Leave some breathingroom
    }
    #  ifdef P104_DEBUG

    if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, log);
    }
    #  endif // ifdef P104_DEBUG
    modulesOnOff(zstruct._startModule, zstruct._startModule + zstruct.size - 1, MD_MAX72XX::MD_ON);  // Continue updates on modules
  }
}

# endif // ifdef P104_USE_BAR_GRAPH

/**************************************************
 * Check if an animation is available in the current build
 *************************************************/
bool isAnimationAvailable(uint8_t animation, bool noneIsAllowed = false) {
  textEffect_t selection = static_cast<textEffect_t>(animation);

  switch (selection) {
    case PA_NO_EFFECT:
    {
      return noneIsAllowed;
    }
    case PA_PRINT:
    case PA_SCROLL_UP:
    case PA_SCROLL_DOWN:
    case PA_SCROLL_LEFT:
    case PA_SCROLL_RIGHT:
    {
      return true;
    }
    # if ENA_SPRITE
    case PA_SPRITE:
    {
      return true;
    }
    # endif // ENA_SPRITE
    # if ENA_MISC
    case PA_SLICE:
    case PA_MESH:
    case PA_FADE:
    case PA_DISSOLVE:
    case PA_BLINDS:
    case PA_RANDOM:
    {
      return true;
    }
    # endif // ENA_MISC
    # if ENA_WIPE
    case PA_WIPE:
    case PA_WIPE_CURSOR:
    {
      return true;
    }
    # endif // ENA_WIPE
    # if ENA_SCAN
    case PA_SCAN_HORIZ:
    case PA_SCAN_HORIZX:
    case PA_SCAN_VERT:
    case PA_SCAN_VERTX:
    {
      return true;
    }
    # endif // ENA_SCAN
    # if ENA_OPNCLS
    case PA_OPENING:
    case PA_OPENING_CURSOR:
    case PA_CLOSING:
    case PA_CLOSING_CURSOR:
    {
      return true;
    }
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
    case PA_SCROLL_UP_LEFT:
    case PA_SCROLL_UP_RIGHT:
    case PA_SCROLL_DOWN_LEFT:
    case PA_SCROLL_DOWN_RIGHT:
    {
      return true;
    }
    # endif // ENA_SCR_DIA
    # if ENA_GROW
    case PA_GROW_UP:
    case PA_GROW_DOWN:
    {
      return true;
    }
    # endif // ENA_GROW
    default:
      return false;
  }
}

/*******************************************************
 * handlePluginWrite : process commands
 ******************************************************/
bool P104_data_struct::handlePluginWrite(taskIndex_t   taskIndex,
                                         const String& string) {
  # ifdef P104_USE_COMMANDS
  bool reconfigure = false;
  # endif // ifdef P104_USE_COMMANDS
  bool   success = false;
  String command = parseString(string, 1);

  if ((nullptr != P) && command.equals(F("dotmatrix"))) { // main command: dotmatrix
    String sub = parseString(string, 2);

    int zoneIndex;
    String string4 = parseStringKeepCase(string, 4);
    # ifdef P104_USE_COMMANDS
    int value4;
    validIntFromString(string4, value4);
    # endif // ifdef P104_USE_COMMANDS

    // Global subcommands
    if (sub.equals(F("clear")) && // subcommand: clear[,all]
        (string4.isEmpty() ||
         string4.equalsIgnoreCase(F("all")))) {
      P->displayClear();
      success = true;
    }

    if (sub.equals(F("update")) && // subcommand: update[,all]
        (string4.isEmpty() ||
         string4.equalsIgnoreCase(F("all")))) {
      updateZone(0, P104_zone_struct(0));
      success = true;
    }

    // Zone-specific subcommands
    if (validIntFromString(parseString(string, 3), zoneIndex) &&
        (zoneIndex > 0) &&
        (static_cast<unsigned int>(zoneIndex) <= zones.size())) {
      // subcommands are processed in the same order as they are presented in the UI
      for (auto it = zones.begin(); it != zones.end() && !success; ++it) {
        if ((it->zone == zoneIndex)) {  // This zone
          if (sub.equals(F("clear"))) { // subcommand: clear,<zone>
            P->displayClear(zoneIndex - 1);
            success = true;
            break;
          }

          if (sub.equals(F("update"))) { // subcommand: update,<zone>
            updateZone(zoneIndex, *it);
            success = true;
            break;
          }

          # ifdef P104_USE_COMMANDS

          if (sub.equals(F("size")) && // subcommand: size,<zone>,<size> (1..)
              (value4 > 0) &&
              (value4 <= P104_MAX_MODULES_PER_ZONE)) {
            reconfigure = (it->size != value4);
            it->size    = value4;
            success     = true;
            break;
          }
          # endif // ifdef P104_USE_COMMANDS

          if ((sub.equals(F("txt")) ||                                                          // subcommand: [set]txt,<zone>,<text> (only
               sub.equals(F("settxt"))) &&                                                      // allowed for zones with Text content)
              ((it->content == P104_CONTENT_TEXT) || (it->content == P104_CONTENT_TEXT_REV))) { // no length check, so longer than the UI
                                                                                                // allows is made possible
            if (sub.equals(F("settxt")) &&                                                      // subcommand: settxt,<zone>,<text> (stores
                (string4.length() <= P104_MAX_TEXT_LENGTH_PER_ZONE)) {                          // the text in the settings, is not saved)
              it->text = string4;                                                               // Only if not too long, could 'blow up' the
            }                                                                                   // settings when saved
            displayOneZoneText(zoneIndex - 1, *it, string4);
            success = true;
            break;
          }

          # ifdef P104_USE_COMMANDS

          if (sub.equals(F("content")) && // subcommand: content,<zone>,<contenttype> (0..<P104_CONTENT_count>-1)
              (value4 >= 0) &&
              (value4 < P104_CONTENT_count)) {
            reconfigure = (it->content != value4);
            it->content = value4;
            success     = true;
            break;
          }

          if (sub.equals(F("alignment")) &&                             // subcommand: alignment,<zone>,<alignment> (0..3)
              (value4 >= 0) &&
              (value4 <= static_cast<int>(textPosition_t::PA_RIGHT))) { // last item in the enum
            it->alignment = value4;
            success       = true;
            break;
          }

          if (sub.equals(F("anim.in")) && // subcommand: anim.in,<zone>,<animation> (1..)
              isAnimationAvailable(value4)) {
            it->animationIn = value4;
            success         = true;
            break;
          }

          if (sub.equals(F("speed")) && // subcommand: speed,<zone>,<speed_ms> (0..P104_MAX_SPEED_PAUSE_VALUE)
              (value4 >= 0) &&
              (value4 <= P104_MAX_SPEED_PAUSE_VALUE)) {
            it->speed = value4;
            success   = true;
            break;
          }

          if (sub.equals(F("anim.out")) && // subcommand: anim.out,<zone>,<animation> (0..)
              isAnimationAvailable(value4, true)) {
            it->animationOut = value4;
            success          = true;
            break;
          }

          if (sub.equals(F("pause")) && // subcommand: pause,<zone>,<pause_ms> (0..P104_MAX_SPEED_PAUSE_VALUE)
              (value4 >= 0) &&
              (value4 <= P104_MAX_SPEED_PAUSE_VALUE)) {
            it->pause = value4;
            success   = true;
            break;
          }

          if (sub.equals(F("font")) && // subcommand: font,<zone>,<font id> (only for incuded font id's)
              (
                (value4 == 0)
                #  ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                || (value4 == P104_DOUBLE_HEIGHT_FONT_ID)
                #  endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                #  ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                || (value4 == P104_FULL_DOUBLEHEIGHT_FONT_ID)
                #  endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                #  ifdef P104_USE_VERTICAL_FONT
                || (value4 == P104_VERTICAL_FONT_ID)
                #  endif // ifdef P104_USE_VERTICAL_FONT
                #  ifdef P104_USE_EXT_ASCII_FONT
                || (value4 == P104_EXT_ASCII_FONT_ID)
                #  endif // ifdef P104_USE_EXT_ASCII_FONT
                #  ifdef P104_USE_ARABIC_FONT
                || (value4 == P104_ARABIC_FONT_ID)
                #  endif // ifdef P104_USE_ARABIC_FONT
                #  ifdef P104_USE_GREEK_FONT
                || (value4 == P104_GREEK_FONT_ID)
                #  endif // ifdef P104_USE_GREEK_FONT
                #  ifdef P104_USE_KATAKANA_FONT
                || (value4 == P104_KATAKANA_FONT_ID)
                #  endif // ifdef P104_USE_KATAKANA_FONT
              )
              ) {
            reconfigure = (it->font != value4);
            it->font    = value4;
            success     = true;
            break;
          }

          if (sub.equals(F("inverted")) && // subcommand: inverted,<zone>,<invertedstate> (disable/enable)
              (value4 >= 0) &&
              (value4 <= 1)) {
            reconfigure  = (it->inverted != value4);
            it->inverted = value4;
            success      = true;
            break;
          }

          #  if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("layout")) && // subcommand: layout,<zone>,<layout> (0..2), only when double-height font is available
              (value4 >= 0) &&
              (value4 <= P104_LAYOUT_DOUBLE_LOWER)) {
            reconfigure = (it->layout != value4);
            it->layout  = value4;
            success     = true;
            break;
          }
          #  endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("specialeffect")) && // subcommand: specialeffect,<zone>,<effect> (0..3)
              (value4 >= 0) &&
              (value4 <= P104_SPECIAL_EFFECT_BOTH)) {
            reconfigure       = (it->specialEffect != value4);
            it->specialEffect = value4;
            success           = true;
            break;
          }

          if (sub.equals(F("offset")) && // subcommand: offset,<zone>,<size> (0..<size>-1)
              (value4 >= 0) &&
              (value4 < P104_MAX_MODULES_PER_ZONE) &&
              (value4 < it->size)) {
            reconfigure = (it->offset != value4);
            it->offset  = value4;
            success     = true;
            break;
          }

          if (sub.equals(F("brightness")) && // subcommand: brightness,<zone>,<brightness> (0..15)
              (value4 >= 0) &&
              (value4 <= P104_BRIGHTNESS_MAX)) {
            it->brightness = value4;
            P->setIntensity(zoneIndex - 1, it->brightness); // Change brightness directly
            success = true;
            break;
          }

          if (sub.equals(F("repeat")) && // subcommand: repeaat,<zone>,<repeat_sec> (-1..86400 = 24h)
              (value4 >= -1) &&
              (value4 <= P104_MAX_REPEATDELAY_VALUE)) {
            it->repeatDelay = value4;
            success         = true;

            if (it->repeatDelay > -1) {
              it->_repeatTimer = millis();
            }
            break;
          }
          # else // ifdef P104_USE_COMMANDS
          {
            String validCommands = F(
              "|size|content|alignment|anim.in|speed|anim.out|pause|font|inverted|layout|specialeffect|offset|brightness|repeat|");
            String testSub = '|';
            testSub += sub;
            testSub += '|';

            if (validCommands.indexOf(testSub) > -1) {
              addLog(LOG_LEVEL_ERROR, F("dotmatrix: subcommand not included in build."));
            }
          }
          # endif // ifdef P104_USE_COMMANDS

          # ifdef P104_USE_BAR_GRAPH

          if ((sub.equals(F("bar")) ||                                 // subcommand: [set]bar,<zone>,<graph-string> (only allowed for zones
               sub.equals(F("setbar"))) &&                             // with Bargraph content) no length check, so longer than the UI
              (it->content == P104_CONTENT_BAR_GRAPH)) {               // allows is made possible
            if (sub.equals(F("setbar")) &&                             // subcommand: setbar,<zone>,<graph-string> (stores the graph-string
                (string4.length() <= P104_MAX_TEXT_LENGTH_PER_ZONE)) { // in the settings, is not saved)
              it->text = string4;                                      // Only if not too long, could 'blow up' the settings when saved
            }
            displayBarGraph(zoneIndex - 1, *it, string4);
            success = true;
            break;
          }
          # endif // ifdef P104_USE_BAR_GRAPH

          // FIXME TD-er: success is always false here. Maybe this must be done outside the for-loop?
          if (success) { // Reset the repeat timer
            if (it->repeatDelay > -1) {
              it->_repeatTimer = millis();
            }
          }
        }
      }
    }
  }

  # ifdef P104_USE_COMMANDS

  if (reconfigure) {
    configureZones(); // Re-initialize
    success = true;   // Successful
  }
  # endif // ifdef P104_USE_COMMANDS

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(34 + string.length())) {
      log = F("dotmatrix: command ");

      if (!success) { log += F("NOT "); }
      log += F("succesful: ");
      log += string;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  return success; // Default: unknown command
}

int8_t getTime(char *psz,
               bool  seconds  = false,
               bool  colon    = true,
               bool  time12h  = false,
               bool  timeAmpm = false) {
  uint16_t h, M, s;
  String   ampm;

  # ifdef P104_USE_DATETIME_OPTIONS

  if (time12h) {
    if (timeAmpm) {
      ampm = (node_time.hour() >= 12 ? F("p") : F("a"));
    }
    h = node_time.hour() % 12;

    if (h == 0) { h = 12; }
  } else
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  {
    h = node_time.hour();
  }
  M = node_time.minute();

  if (!seconds) {
    sprintf_P(psz, PSTR("%02d%c%02d%s"), h, (colon ? ':' : ' '), M, ampm.c_str());
  } else {
    s = node_time.second();
    sprintf_P(psz, PSTR("%02d%c%02d %02d%s"), h, (colon ? ':' : ' '), M, s, ampm.c_str());
  }
  return M;
}

void getDate(char           *psz,
             bool            showYear = true,
             bool            fourDgt  = false
             # ifdef         P104_USE_DATETIME_OPTIONS
             , const uint8_t dateFmt = 0
             , const uint8_t dateSep = 0
             # endif // ifdef P104_USE_DATETIME_OPTIONS
             ) {
  uint16_t d, m, y;
  const uint16_t year = node_time.year() - (fourDgt ? 0 : 2000);

  # ifdef P104_USE_DATETIME_OPTIONS
  const String separators = F(" /-.");
  const char   sep        = separators[dateSep];
  # else // ifdef P104_USE_DATETIME_OPTIONS
  const char sep = ' ';
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  d = node_time.day();
  m = node_time.month();
  y = year;
  # ifdef P104_USE_DATETIME_OPTIONS

  if (showYear) {
    switch (dateFmt) {
      case P104_DATE_FORMAT_US:
        d = node_time.month();
        m = node_time.day();
        y = year;
        break;
      case P104_DATE_FORMAT_JP:
        d = year;
        m = node_time.month();
        y = node_time.day();
        break;
    }
  } else {
    if ((dateFmt == P104_DATE_FORMAT_US) ||
        (dateFmt == P104_DATE_FORMAT_JP)) {
      std::swap(d, m);
    }
  }
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  if (showYear) {
    sprintf_P(psz, PSTR("%02d%c%02d%c%02d"), d, sep, m, sep, y);     // %02d will expand to 04 when needed
  } else {
    sprintf_P(psz, PSTR("%02d%c%02d"), d, sep, m);
  }
}

uint8_t getDateTime(char           *psz,
                    bool            colon    = true,
                    bool            time12h  = false,
                    bool            timeAmpm = false,
                    bool            fourDgt  = false
                    # ifdef         P104_USE_DATETIME_OPTIONS
                    , const uint8_t dateFmt = 0
                    , const uint8_t dateSep = 0
                    # endif // ifdef P104_USE_DATETIME_OPTIONS
                    ) {
  String   ampm;
  uint16_t d, M, y;
  uint8_t  h, m;
  const uint16_t year = node_time.year() - (fourDgt ? 0 : 2000);

  # ifdef P104_USE_DATETIME_OPTIONS
  const String separators = F(" /-.");
  const char   sep        = separators[dateSep];
  # else // ifdef P104_USE_DATETIME_OPTIONS
  const char sep = ' ';
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  # ifdef P104_USE_DATETIME_OPTIONS

  if (time12h) {
    if (timeAmpm) {
      ampm = (node_time.hour() >= 12 ? F("p") : F("a"));
    }
    h = node_time.hour() % 12;

    if (h == 0) { h = 12; }
  } else
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  {
    h = node_time.hour();
  }
  M = node_time.minute();

  # ifdef P104_USE_DATETIME_OPTIONS

  switch (dateFmt) {
    case P104_DATE_FORMAT_US:
      d = node_time.month();
      m = node_time.day();
      y = year;
      break;
    case P104_DATE_FORMAT_JP:
      d = year;
      m = node_time.month();
      y = node_time.day();
      break;
    default:
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  d = node_time.day();
  m = node_time.month();
  y = year;
  # ifdef P104_USE_DATETIME_OPTIONS
}

  # endif // ifdef P104_USE_DATETIME_OPTIONS
  sprintf_P(psz, PSTR("%02d%c%02d%c%02d %02d%c%02d%s"), d, sep, m, sep, y, h, (colon ? ':' : ' '), M, ampm.c_str()); // %02d will expand to
                                                                                                                     // 04 when needed
  return M;
}

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string) {
  const uint16_t stringLen = string.length();

  for (uint16_t i = 0; i < stringLen; i++) {
    string[i] |= 0x80; // use 'high' part of the font, by adding 0x80
  }
}

# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

void reverseStr(String& str) {
  const uint16_t n = str.length();

  // Swap characters starting from two corners
  for (uint16_t i = 0; i < n / 2; i++) {
    std::swap(str[i], str[n - i - 1]);
  }
}

/************************************************************************
 * execute all PLUGIN_ONE_PER_SECOND tasks
 ***********************************************************************/
bool P104_data_struct::handlePluginOncePerSecond(struct EventStruct *event) {
  if (nullptr == P) { return false; }
  bool redisplay = false;
  bool success   = false;

  # ifdef P104_USE_DATETIME_OPTIONS
  bool useFlasher = !bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FLASH);
  bool time12h    = bitRead(P104_CONFIG_DATETIME,  P104_CONFIG_DATETIME_12H);
  bool timeAmpm   = bitRead(P104_CONFIG_DATETIME,  P104_CONFIG_DATETIME_AMPM);
  bool year4dgt   = bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_YEAR4DGT);
  # else // ifdef P104_USE_DATETIME_OPTIONS
  bool useFlasher = true;
  bool time12h    = false;
  bool timeAmpm   = false;
  bool year4dgt   = false;
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  bool newFlasher = !flasher && useFlasher;

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    redisplay = false;

    if (P->getZoneStatus(it->zone - 1)) { // Animations done?
      switch (it->content) {
        case P104_CONTENT_TIME:           // time
        case P104_CONTENT_TIME_SEC:       // time sec
        {
          bool   useSeconds = (it->content == P104_CONTENT_TIME_SEC);
          int8_t m          = getTime(szTimeL, useSeconds, flasher || !useFlasher, time12h, timeAmpm);
          flasher          = newFlasher;
          redisplay        = useFlasher || useSeconds || (it->_lastChecked != m);
          it->_lastChecked = m;
          break;
        }
        case P104_CONTENT_DATE4: // date/4
        case P104_CONTENT_DATE6: // date/6
        {
          if (it->_lastChecked != node_time.day()) {
            getDate(szTimeL,
                    it->content != P104_CONTENT_DATE4,
                    year4dgt
                    # ifdef P104_USE_DATETIME_OPTIONS
                    , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT)
                    , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR)
                    # endif // ifdef P104_USE_DATETIME_OPTIONS
                    );
            redisplay        = true;
            it->_lastChecked = node_time.day();
          }
          break;
        }
        case P104_CONTENT_DATE_TIME: // date-time/9
        {
          int8_t m = getDateTime(szTimeL,
                                 flasher || !useFlasher,
                                 time12h,
                                 timeAmpm,
                                 year4dgt
                                 # ifdef P104_USE_DATETIME_OPTIONS
                                 , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT)
                                 , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR)
                                 # endif // ifdef P104_USE_DATETIME_OPTIONS
                                 );
          flasher          = newFlasher;
          redisplay        = useFlasher || (it->_lastChecked != m);
          it->_lastChecked = m;
          break;
        }
        default:
          break;
      }

      if (redisplay) {
        displayOneZoneText(it->zone - 1, *it, String(szTimeL));
        P->displayReset(it->zone - 1);

        if (it->repeatDelay > -1) {
          it->_repeatTimer = millis();
        }
      }
    }
    delay(0); // Leave some breathingroom
  }

  if (redisplay) {
    // synchronise the start
    P->synchZoneStart();
  }
  return redisplay || success;
}

/***************************************************
 * restart a zone if the repeat delay (if any) has passed
 **************************************************/
void P104_data_struct::checkRepeatTimer(uint8_t z) {
  if (nullptr == P) { return; }
  bool handled = false;

  for (auto it = zones.begin(); it != zones.end() && !handled; ++it) {
    if (it->zone == z + 1) {
      handled = true;

      if ((it->repeatDelay > -1) && (timePassedSince(it->_repeatTimer) >= (it->repeatDelay - 1) * 1000)) { // Compensated for the '1' in
                                                                                                           // PLUGIN_ONE_PER_SECOND
        # ifdef P104_DEBUG

        if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(51);
          log  = F("dotmatrix: Repeat zone: ");
          log += it->zone;
          log += F(" delay: ");
          log += it->repeatDelay;
          log += F(" (");
          log += (timePassedSince(it->_repeatTimer) / 1000.0f); // Decimals can be useful here
          log += ')';
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P104_DEBUG

        if ((it->content == P104_CONTENT_TEXT) ||
            (it->content == P104_CONTENT_TEXT_REV)) {
          displayOneZoneText(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed text
          P->displayReset(it->zone - 1);
        }

        if ((it->content == P104_CONTENT_TIME) ||
            (it->content == P104_CONTENT_TIME_SEC) ||
            (it->content == P104_CONTENT_DATE4) ||
            (it->content == P104_CONTENT_DATE6) ||
            (it->content == P104_CONTENT_DATE_TIME)) {
          it->_lastChecked = -1; // Invalidate so next run will re-display the date/time
        }
        # ifdef P104_USE_BAR_GRAPH

        if (it->content == P104_CONTENT_BAR_GRAPH) {
          displayBarGraph(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed bar graph
        }
        # endif // ifdef P104_USE_BAR_GRAPH
        it->_repeatTimer = millis();
      }
    }
    delay(0); // Leave some breathingroom
  }
}

/******************************************
 * enquoteString wrap in ", ' or ` unless all 3 quote types are used
 *****************************************/
String P104_data_struct::enquoteString(const String& input) {
  char quoteChar = '"';

  if (input.indexOf(quoteChar) > -1) {
    quoteChar = '\'';

    if (input.indexOf(quoteChar) > -1) {
      quoteChar = '`';

      if (input.indexOf(quoteChar) > -1) {
        return input; // All types of supported quotes used, return original string
      }
    }
  }
  String result;

  result.reserve(input.length() + 2);
  result  = quoteChar;
  result += input;
  result += quoteChar;

  return result;
}

/***************************************
 * saveSettings gather the zones data from the UI and store in customsettings
 **************************************/
bool P104_data_struct::saveSettings() {
  error = EMPTY_STRING; // Clear
  String zbuffer;

  # ifdef P104_DEBUG_DEV
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(64)) {
    log  = F("P104: saving zones, count: ");
    log += expectedZones;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG_DEV

  uint8_t index      = 0;
  uint8_t action     = P104_ACTION_NONE;
  uint8_t zoneIndex  = 0;
  int8_t  zoneOffset = 0;

  zones.clear(); // Start afresh

  for (uint8_t zCounter = 0; zCounter < expectedZones; zCounter++) {
    # ifdef P104_USE_ZONE_ACTIONS
    action = getFormItemInt(getPluginCustomArgName(index + P104_OFFSET_ACTION));

    if (((action == P104_ACTION_ADD_ABOVE) && (zoneOrder == 0)) ||
        ((action == P104_ACTION_ADD_BELOW) && (zoneOrder == 1))) {
      zones.push_back(P104_zone_struct(0));
      zoneOffset++;
      #  ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: insert before zone: ");
        log += (zoneIndex + 1);
        addLog(LOG_LEVEL_INFO, log);
      }
      #  endif // ifdef P104_DEBUG_DEV
    }
    # endif    // ifdef P104_USE_ZONE_ACTIONS
    zoneIndex = zCounter + zoneOffset;

    if (action == P104_ACTION_DELETE) {
      zoneOffset--;
    } else {
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: read zone: ");
        log += (zoneIndex + 1);
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG_DEV
      zones.push_back(P104_zone_struct(zoneIndex + 1));

      zones[zoneIndex].size          = getFormItemIntCustomArgName(index + P104_OFFSET_SIZE);
      zones[zoneIndex].text          = enquoteString(webArg(getPluginCustomArgName(index + P104_OFFSET_TEXT)));
      zones[zoneIndex].content       = getFormItemIntCustomArgName(index + P104_OFFSET_CONTENT);
      zones[zoneIndex].alignment     = getFormItemIntCustomArgName(index + P104_OFFSET_ALIGNMENT);
      zones[zoneIndex].animationIn   = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_IN);
      zones[zoneIndex].speed         = getFormItemIntCustomArgName(index + P104_OFFSET_SPEED);
      zones[zoneIndex].animationOut  = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_OUT);
      zones[zoneIndex].pause         = getFormItemIntCustomArgName(index + P104_OFFSET_PAUSE);
      zones[zoneIndex].font          = getFormItemIntCustomArgName(index + P104_OFFSET_FONT);
      zones[zoneIndex].layout        = getFormItemIntCustomArgName(index + P104_OFFSET_LAYOUT);
      zones[zoneIndex].specialEffect = getFormItemIntCustomArgName(index + P104_OFFSET_SPEC_EFFECT);
      zones[zoneIndex].offset        = getFormItemIntCustomArgName(index + P104_OFFSET_OFFSET);
      zones[zoneIndex].inverted      = getFormItemIntCustomArgName(index + P104_OFFSET_INVERTED);

      if (zones[zoneIndex].size != 0) { // for newly added zone, use defaults
        zones[zoneIndex].brightness  = getFormItemIntCustomArgName(index + P104_OFFSET_BRIGHTNESS);
        zones[zoneIndex].repeatDelay = getFormItemIntCustomArgName(index + P104_OFFSET_REPEATDELAY);
      }
    }
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: add zone: ");
      log += (zoneIndex + 1);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV

    # ifdef P104_USE_ZONE_ACTIONS

    if (((action == P104_ACTION_ADD_BELOW) && (zoneOrder == 0)) ||
        ((action == P104_ACTION_ADD_ABOVE) && (zoneOrder == 1))) {
      zones.push_back(P104_zone_struct(0));
      zoneOffset++;
      #  ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: insert after zone: ");
        log += (zoneIndex + 2);
        addLog(LOG_LEVEL_INFO, log);
      }
      #  endif // ifdef P104_DEBUG_DEV
    }
    # endif    // ifdef P104_USE_ZONE_ACTIONS

    index += P104_OFFSET_COUNT;
    delay(0);
  }

  uint16_t bufferSize;
  int saveOffset = 0;

  numDevices = 0;                      // Count the number of connected display units

  bufferSize = P104_CONFIG_VERSION_V2; // Save special marker that we're using V2 settings
  // This write is counting
  error      += SaveToFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)&bufferSize, sizeof(bufferSize), saveOffset);
  saveOffset += sizeof(bufferSize);

  if (zbuffer.reserve(P104_SETTINGS_BUFFER_V2 + 2)) {
    for (auto it = zones.begin(); it != zones.end() && error.length() == 0; ++it) {
      zbuffer = EMPTY_STRING;

      // WARNING: Order of values should match the numeric order of P104_OFFSET_* values
      zbuffer += it->size;          // 2
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->text;          // 2 + ~15
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->content;       // 1
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->alignment;     // 1
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->animationIn;   // 2
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->speed;         // 5
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->animationOut;  // 2
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->pause;         // 5
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->font;          // 1
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->layout;        // 1
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->specialEffect; // 1
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->offset;        // 2
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->brightness;    // 2
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->repeatDelay;   // 4
      zbuffer += P104_FIELD_SEP;    // 1
      zbuffer += it->inverted;      // 1
      zbuffer += P104_FIELD_SEP;    // 1

      // 47 total + (max) 100 characters for it->text requires a buffer of ~150 (P104_SETTINGS_BUFFER_V2), but only the required length is
      // stored with the length prefixed

      numDevices += (it->size != 0 ? it->size : 1) + it->offset;                                // Count corrected for newly added zones

      if (saveOffset + zbuffer.length() + (sizeof(bufferSize) * 2) > (DAT_TASKS_CUSTOM_SIZE)) { // Detect ourselves if we've reached the
        error.reserve(55);                                                                      // high-water mark
        error += F("Total combination of Zones & text too long to store.\n");
        addLog(LOG_LEVEL_ERROR, error);
      } else {
        // Store length of buffer
        bufferSize = zbuffer.length();

        // As we write in parts, only count as single write.
        if (RTC.flashDayCounter > 0) {
          RTC.flashDayCounter--;
        }
        error += SaveToFile(SettingsType::Enum::CustomTaskSettings_Type,
                            taskIndex,
                            (uint8_t *)&bufferSize,
                            sizeof(bufferSize),
                            saveOffset);
        saveOffset += sizeof(bufferSize);

        // As we write in parts, only count as single write.
        if (RTC.flashDayCounter > 0) {
          RTC.flashDayCounter--;
        }
        error += SaveToFile(SettingsType::Enum::CustomTaskSettings_Type,
                            taskIndex,
                            (uint8_t *)zbuffer.c_str(),
                            bufferSize,
                            saveOffset);
        saveOffset += bufferSize;

        # ifdef P104_DEBUG_DEV

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(64);
          log  = F("P104: saveSettings zone: ");
          log += it->zone;
          log += F(" bufferSize: ");
          log += bufferSize;
          log += F(" offset: ");
          log += saveOffset;
          addLog(LOG_LEVEL_INFO, log);
          zbuffer.replace(P104_FIELD_SEP, P104_FIELD_DISP);
          addLog(LOG_LEVEL_INFO, zbuffer);
        }
        # endif // ifdef P104_DEBUG_DEV
      }

      delay(0);
    }

    // Store an End-of-settings marker == 0
    bufferSize = 0u;

    // This write is counting
    SaveToFile(SettingsType::Enum::CustomTaskSettings_Type, taskIndex, (uint8_t *)&bufferSize, sizeof(bufferSize), saveOffset);

    if (numDevices > 255) {
      error += F("More than 255 modules configured (");
      error += numDevices;
      error += ')';
      error += '\n';
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Can't allocate string for saving settings, insufficient memory!"));
    return false; // Don't continue
  }

  return error.isEmpty();
}

/**************************************************************
* webform_load
**************************************************************/
bool P104_data_struct::webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Device settings"));

  {                                       // Hardware types
    # define P104_hardwareTypeCount 8
    const __FlashStringHelper *hardwareTypes[P104_hardwareTypeCount] = {
      F("Generic (DR:0, CR:1, RR:0)"),    // 010
      F("Parola (DR:1, CR:1, RR:0)"),     // 110
      F("FC16 (DR:1, CR:0, RR:0)"),       // 100
      F("IC Station (DR:1, CR:1, RR:1)"), // 111
      F("Other 1 (DR:0, CR:0, RR:0)"),    // 000
      F("Other 2 (DR:0, CR:0, RR:1)"),    // 001
      F("Other 3 (DR:0, CR:1, RR:1)"),    // 011
      F("Other 4 (DR:1, CR:0, RR:1)")     // 101
    };
    const int hardwareOptions[P104_hardwareTypeCount] = {
      static_cast<int>(MD_MAX72XX::moduleType_t::GENERIC_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::PAROLA_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::FC16_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::ICSTATION_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR0_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR1RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR1CR0RR1_HW)
    };
    addFormSelector(F("Hardware type"),
                    F("plugin_104_hardware"),
                    P104_hardwareTypeCount,
                    hardwareTypes,
                    hardwareOptions,
                    P104_CONFIG_HARDWARETYPE);
    # ifdef P104_ADD_SETTINGS_NOTES
    addFormNote(F("DR = Digits as Rows, CR = Column Reversed, RR = Row Reversed; 0 = no, 1 = yes."));
    # endif // ifdef P104_ADD_SETTINGS_NOTES
  }

  {
    addFormCheckBox(F("Clear display on disable"), F("plugin_104_cleardisable"),
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE));

    addFormCheckBox(F("Log all displayed text (info)"),
                    F("plugin_104_logalltext"),
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT));
  }

  # ifdef P104_USE_DATETIME_OPTIONS
  {
    addFormSubHeader(F("Content options"));

    addFormCheckBox(F("Clock with flashing colon"), F("plugin_104_clockflash"), !bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FLASH));
    addFormCheckBox(F("Clock 12h display"),         F("plugin_104_clock12h"),   bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_12H));
    addFormCheckBox(F("Clock 12h AM/PM indicator"), F("plugin_104_clockampm"),  bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_AMPM));
  }
  { // Date format
    const __FlashStringHelper *dateFormats[] = {
      F("Day Month [Year]"),
      F("Month Day [Year] (US-style)"),
      F("[Year] Month Day (Japanese-style)")
    };
    const int dateFormatOptions[] = {
      P104_DATE_FORMAT_EU,
      P104_DATE_FORMAT_US,
      P104_DATE_FORMAT_JP
    };
    addFormSelector(F("Date format"), F("plugin_104_dateformat"),
                    3,
                    dateFormats, dateFormatOptions,
                    get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT));
  }
  { // Date separator
    const __FlashStringHelper *dateSeparators[] = {
      F("Space"),
      F("Slash /"),
      F("Dash -"),
      F("Dot <b>.</b>")
    };
    const int dateSeparatorOptions[] = {
      P104_DATE_SEPARATOR_SPACE,
      P104_DATE_SEPARATOR_SLASH,
      P104_DATE_SEPARATOR_DASH,
      P104_DATE_SEPARATOR_DOT
    };
    addFormSelector(F("Date separator"), F("plugin_104_dateseparator"),
                    4,
                    dateSeparators, dateSeparatorOptions,
                    get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR));

    addFormCheckBox(F("Year uses 4 digits"), F("plugin_104_year4dgt"), bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_YEAR4DGT));
  }
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  addFormSubHeader(F("Zones"));

  { // Zones
    String zonesList[P104_MAX_ZONES];
    int    zonesOptions[P104_MAX_ZONES];

    for (uint8_t i = 0; i < P104_MAX_ZONES; i++) {
      zonesList[i]    = i + 1;
      zonesOptions[i] = i + 1; // No 0 needed or wanted
    }
    # if defined(P104_USE_TOOLTIPS) || defined(P104_ADD_SETTINGS_NOTES)
    String zonetip;

    if (zonetip.reserve(90)) {
      zonetip  = F("Select between 1 and ");
      zonetip += P104_MAX_ZONES;
      zonetip += F(" zones, changing");
      #  ifdef P104_USE_ZONE_ORDERING
      zonetip += F(" Zones or Zone order");
      #  endif // ifdef P104_USE_ZONE_ORDERING
      zonetip += F(" will save and reload the page.");
    }
    # endif    // if defined(P104_USE_TOOLTIPS) || defined(P104_ADD_SETTINGS_NOTES)
    addFormSelector(F("Zones"), F("plugin_104_zonecount"), P104_MAX_ZONES, zonesList, zonesOptions, NULL, P104_CONFIG_ZONE_COUNT, true
                    # ifdef P104_USE_TOOLTIPS
                    , zonetip
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

    # ifdef P104_USE_ZONE_ORDERING
    const String orderTypes[] = {
      F("Numeric order (1..n)"),
      F("Display order (n..1)")
    };
    const int    orderOptions[] = { 0, 1 };
    addFormSelector(F("Zone order"), F("plugin_104_zoneorder"), 2, orderTypes, orderOptions, NULL,
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER) ? 1 : 0, true
                    #  ifdef P104_USE_TOOLTIPS
                    , zonetip
                    #  endif // ifdef P104_USE_TOOLTIPS
                    );
    # endif                  // ifdef P104_USE_ZONE_ORDERING
    # ifdef P104_ADD_SETTINGS_NOTES
    addFormNote(zonetip);
    # endif // ifdef P104_ADD_SETTINGS_NOTES
  }
  expectedZones = P104_CONFIG_ZONE_COUNT;

  if (expectedZones == 0) { expectedZones++; } // Minimum of 1 zone

  { // Optionlists and zones table
    const __FlashStringHelper *alignmentTypes[3] = {
      F("Left"),
      F("Center"),
      F("Right")
    };
    const int alignmentOptions[3] = {
      static_cast<int>(textPosition_t::PA_LEFT),
      static_cast<int>(textPosition_t::PA_CENTER),
      static_cast<int>(textPosition_t::PA_RIGHT)
    };
    int animationCount = 6;
    # if ENA_SPRITE
    animationCount += 1;
    # endif // ENA_SPRITE
    # if ENA_MISC
    animationCount += 6;
    # endif // ENA_MISC
    # if ENA_WIPE
    animationCount += 2;
    # endif // ENA_WIPE
    # if ENA_SCAN
    animationCount += 4;
    # endif // ENA_SCAN
    # if ENA_OPNCLS
    animationCount += 4;
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
    animationCount += 4;
    # endif // ENA_SCR_DIA
    # if ENA_GROW
    animationCount += 2;
    # endif // ENA_GROW
    String animationTypes[] {
      F("None")
      , F("Print")
      , F("Scroll up")
      , F("Scroll down")
      , F("Scroll left *")
      , F("Scroll right *")
    # if ENA_SPRITE
      , F("Sprite")
    # endif // ENA_SPRITE
    # if ENA_MISC
      , F("Slice *")
      , F("Mesh")
      , F("Fade")
      , F("Dissolve")
      , F("Blinds")
      , F("Random")
    # endif // ENA_MISC
    # if ENA_WIPE
      , F("Wipe")
      , F("Wipe w. cursor")
    # endif // ENA_WIPE
    # if ENA_SCAN
      , F("Scan horiz.")
      , F("Scan horiz. cursor")
      , F("Scan vert.")
      , F("Scan vert. cursor")
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , F("Opening")
      , F("Opening w. cursor")
      , F("Closing")
      , F("Closing w. cursor")
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , F("Scroll up left *")
      , F("Scroll up right *")
      , F("Scroll down left *")
      , F("Scroll down right *")
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , F("Grow up")
      , F("Grow down")
    # endif // ENA_GROW
    };
    const int animationOptions[] = {
      static_cast<int>(textEffect_t::PA_NO_EFFECT)
      , static_cast<int>(textEffect_t::PA_PRINT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN)
      , static_cast<int>(textEffect_t::PA_SCROLL_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_RIGHT)
    # if ENA_SPRITE
      , static_cast<int>(textEffect_t::PA_SPRITE)
    # endif // ENA_SPRITE
    # if ENA_MISC
      , static_cast<int>(textEffect_t::PA_SLICE)
      , static_cast<int>(textEffect_t::PA_MESH)
      , static_cast<int>(textEffect_t::PA_FADE)
      , static_cast<int>(textEffect_t::PA_DISSOLVE)
      , static_cast<int>(textEffect_t::PA_BLINDS)
      , static_cast<int>(textEffect_t::PA_RANDOM)
    # endif // ENA_MISC
    # if ENA_WIPE
      , static_cast<int>(textEffect_t::PA_WIPE)
      , static_cast<int>(textEffect_t::PA_WIPE_CURSOR)
    # endif // ENA_WIPE
    # if ENA_SCAN
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZ)
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZX)
      , static_cast<int>(textEffect_t::PA_SCAN_VERT)
      , static_cast<int>(textEffect_t::PA_SCAN_VERTX)
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , static_cast<int>(textEffect_t::PA_OPENING)
      , static_cast<int>(textEffect_t::PA_OPENING_CURSOR)
      , static_cast<int>(textEffect_t::PA_CLOSING)
      , static_cast<int>(textEffect_t::PA_CLOSING_CURSOR)
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_RIGHT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_RIGHT)
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , static_cast<int>(textEffect_t::PA_GROW_UP)
      , static_cast<int>(textEffect_t::PA_GROW_DOWN)
    # endif // ENA_GROW
    };

    // Append the numeric value as a reference for the 'anim.in' and 'anim.out' subcommands
    for (uint8_t a = 0; a < animationCount; a++) {
      animationTypes[a] += F(" (");
      animationTypes[a] += animationOptions[a];
      animationTypes[a] += ')';
    }
    delay(0);

    int fontCount = 1;
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
    fontCount++;
    # endif // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
    fontCount++;
    # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
    fontCount++;
    # endif // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
    fontCount++;
    # endif // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
    fontCount++;
    # endif // ifdef P104_USE_KATAKANA_FONT
    const __FlashStringHelper *fontTypes[] = {
      F("Default (0)")
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , F("Numeric, double height (1)")
    # endif   // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , F("Full, double height (2)")
    # endif   // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , F("Vertical (3)")
    # endif   // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , F("Extended ASCII (4)")
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , F("Arabic (5)")
    # endif   // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , F("Greek (6)")
    # endif   // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , F("Katakana (7)")
    # endif   // ifdef P104_USE_KATAKANA_FONT
    };
    const int fontOptions[] = {
      P104_DEFAULT_FONT_ID
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , P104_DOUBLE_HEIGHT_FONT_ID
    # endif   // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , P104_FULL_DOUBLEHEIGHT_FONT_ID
    # endif   // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , P104_VERTICAL_FONT_ID
    # endif   // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , P104_EXT_ASCII_FONT_ID
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , P104_ARABIC_FONT_ID
    # endif   // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , P104_GREEK_FONT_ID
    # endif   // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , P104_KATAKANA_FONT_ID
    # endif   // ifdef P104_USE_KATAKANA_FONT
    };

    int layoutCount = 1;
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    layoutCount += 2;
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    const __FlashStringHelper *layoutTypes[] = {
      F("Standard")
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , F("Double, upper")
      , F("Double, lower")
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };
    const int layoutOptions[] = {
      P104_LAYOUT_STANDARD
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , P104_LAYOUT_DOUBLE_UPPER
      , P104_LAYOUT_DOUBLE_LOWER
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };

    const int specialEffectCount                    = 4;
    const __FlashStringHelper *specialEffectTypes[] = {
      F("None"),
      F("Flip up/down"),
      F("Flip left/right *"),
      F("Flip u/d &amp; l/r *")
    };
    const int specialEffectOptions[] = {
      P104_SPECIAL_EFFECT_NONE,
      P104_SPECIAL_EFFECT_UP_DOWN,
      P104_SPECIAL_EFFECT_LEFT_RIGHT,
      P104_SPECIAL_EFFECT_BOTH
    };

    const __FlashStringHelper *contentTypes[] = {
      F("Text"),
      F("Text reverse"),
      F("Clock (4 mod)"),
      F("Clock sec (6 mod)"),
      F("Date (4 mod)"),
      F("Date yr (6/7 mod)"),
      F("Date/time (9/13 mod)")
      # ifdef P104_USE_BAR_GRAPH
      , F("Bar graph")
      # endif // ifdef P104_USE_BAR_GRAPH
    };
    const int contentOptions[] {
      P104_CONTENT_TEXT,
      P104_CONTENT_TEXT_REV,
      P104_CONTENT_TIME,
      P104_CONTENT_TIME_SEC,
      P104_CONTENT_DATE4,
      P104_CONTENT_DATE6,
      P104_CONTENT_DATE_TIME
      # ifdef P104_USE_BAR_GRAPH
      , P104_CONTENT_BAR_GRAPH
      # endif // ifdef P104_USE_BAR_GRAPH
    };
    const __FlashStringHelper *invertedTypes[3] = {
      F("Normal"),
      F("Inverted")
    };
    const int invertedOptions[] = {
      0,
      1
    };
    # ifdef P104_USE_ZONE_ACTIONS
    uint8_t actionCount = 0;
    const __FlashStringHelper *actionTypes[4];
    int actionOptions[4];
    actionTypes[actionCount]   = F("None");
    actionOptions[actionCount] = P104_ACTION_NONE;
    actionCount++;

    if (zones.size() < P104_MAX_ZONES) {
      actionTypes[actionCount]   = F("New above");
      actionOptions[actionCount] = P104_ACTION_ADD_ABOVE;
      actionCount++;
      actionTypes[actionCount]   = F("New below");
      actionOptions[actionCount] = P104_ACTION_ADD_BELOW;
      actionCount++;
    }
    actionTypes[actionCount]   = F("Delete");
    actionOptions[actionCount] = P104_ACTION_DELETE;
    actionCount++;
    # endif // ifdef P104_USE_ZONE_ACTIONS

    delay(0);

    addFormSubHeader(F("Zone configuration"));

    {
      html_table(EMPTY_STRING); // Sub-table
      html_table_header(F("Zone #&nbsp;"));
      html_table_header(F("Modules"));
      html_table_header(F("Text"), 180);
      html_table_header(F("Content"));
      html_table_header(F("Alignment"));
      html_table_header(F("Animation In/Out"));               // 1st and 2nd row title
      html_table_header(F("Speed/Pause"));                    // 1st and 2nd row title
      html_table_header(F("Font/Layout"));                    // 1st and 2nd row title
      html_table_header(F("Inverted/ Special&nbsp;Effects")); // 1st and 2nd row title
      html_table_header(F("Offset"));
      html_table_header(F("Brightness"));
      html_table_header(F("Repeat (sec)"));
      # ifdef P104_USE_ZONE_ACTIONS
      html_table_header(F(""),       15); // Spacer
      html_table_header(F("Action"), 45);
      # endif // ifdef P104_USE_ZONE_ACTIONS
    }

    uint16_t index;
    int16_t  startZone, endZone;
    int8_t   incrZone = 1;
    # ifdef P104_USE_ZONE_ACTIONS
    uint8_t currentRow = 0;
    # endif // ifdef P104_USE_ZONE_ACTIONS

    # ifdef P104_USE_ZONE_ORDERING

    if (bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER)) {
      startZone = zones.size() - 1;
      endZone   = -1;
      incrZone  = -1;
    } else
    # endif // ifdef P104_USE_ZONE_ORDERING
    {
      startZone = 0;
      endZone   = zones.size();
    }

    for (int8_t zone = startZone; zone != endZone; zone += incrZone) {
      if (zones[zone].zone <= expectedZones) {
        index = (zones[zone].zone - 1) * P104_OFFSET_COUNT;

        html_TR_TD(); // All columns use max. width available
        addHtml(F("&nbsp;"));
        addHtmlInt(zones[zone].zone);

        html_TD(); // Modules
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SIZE), zones[zone].size, 1, P104_MAX_MODULES_PER_ZONE);

        html_TD(); // Text
        addTextBox(getPluginCustomArgName(index + P104_OFFSET_TEXT),
                   zones[zone].text,
                   P104_MAX_TEXT_LENGTH_PER_ZONE,
                   false,
                   false,
                   EMPTY_STRING,
                   EMPTY_STRING);

        html_TD(); // Content
        addSelector(getPluginCustomArgName(index + P104_OFFSET_CONTENT),
                    P104_CONTENT_count,
                    contentTypes,
                    contentOptions,
                    NULL,
                    zones[zone].content,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Alignment
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ALIGNMENT),
                    3,
                    alignmentTypes,
                    alignmentOptions,
                    NULL,
                    zones[zone].alignment,
                    false,
                    true,
                    EMPTY_STRING);

        {
          html_TD(); // Animation In (without None by passing the second element index)
          addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_IN),
                      animationCount - 1,
                      &animationTypes[1],
                      &animationOptions[1],
                      NULL,
                      zones[zone].animationIn,
                      false,
                      true,
                      F("")
                      # ifdef P104_USE_TOOLTIPS
                      , F("Animation In")
                      # endif // ifdef P104_USE_TOOLTIPS
                      );
        }

        html_TD();                   // Speed In
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SPEED), zones[zone].speed, 0, P104_MAX_SPEED_PAUSE_VALUE
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Speed")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Font
        addSelector(getPluginCustomArgName(index + P104_OFFSET_FONT),
                    fontCount,
                    fontTypes,
                    fontOptions,
                    NULL,
                    zones[zone].font,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Font") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Inverted
        addSelector(getPluginCustomArgName(index + P104_OFFSET_INVERTED),
                    2,
                    invertedTypes,
                    invertedOptions,
                    NULL,
                    zones[zone].inverted,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Inverted") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(3); // Fill columns
        # ifdef P104_USE_ZONE_ACTIONS

        html_TD();  // Spacer
        addHtml(F("|"));

        if (currentRow < 2) {
          addHtml(F("<TD style=\"text-align:center;font-size:90%\">")); // Action column, text centered and font-size 90%
        } else {
          html_TD();
        }

        if (currentRow == 0) {
          addHtml(F("(applied immediately!)"));
        } else if (currentRow == 1) {
          addHtml(F("(Delete can't be undone!)"));
        }
        currentRow++;
        # endif // ifdef P104_USE_ZONE_ACTIONS

        // Split here
        html_TR_TD(); // Start new row
        html_TD(4);  // Start with some blank columns

        {
          html_TD(); // Animation Out
          addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_OUT),
                      animationCount,
                      animationTypes,
                      animationOptions,
                      NULL,
                      zones[zone].animationOut,
                      false,
                      true,
                      EMPTY_STRING
                      # ifdef P104_USE_TOOLTIPS
                      , F("Animation Out")
                      # endif // ifdef P104_USE_TOOLTIPS
                      );
        }

        html_TD();                   // Pause after Animation In
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_PAUSE), zones[zone].pause, 0, P104_MAX_SPEED_PAUSE_VALUE
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Pause")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Layout
        addSelector(getPluginCustomArgName(index + P104_OFFSET_LAYOUT),
                    layoutCount,
                    layoutTypes,
                    layoutOptions,
                    NULL,
                    zones[zone].layout,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Layout") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Special effects
        addSelector(getPluginCustomArgName(index + P104_OFFSET_SPEC_EFFECT),
                    specialEffectCount,
                    specialEffectTypes,
                    specialEffectOptions,
                    NULL,
                    zones[zone].specialEffect,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Special Effects") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Offset
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_OFFSET), zones[zone].offset, 0, 254);

        html_TD(); // Brightness

        if (zones[zone].brightness == -1) { zones[zone].brightness = P104_BRIGHTNESS_DEFAULT; }
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_BRIGHTNESS), zones[zone].brightness, 0, P104_BRIGHTNESS_MAX);

        html_TD(); // Repeat (sec)
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_REPEATDELAY),
                      zones[zone].repeatDelay,
                      -1,
                      P104_MAX_REPEATDELAY_VALUE                     // max delay 86400 sec. = 24 hours
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING                                 // classname
                      , F("Repeat after this delay (sec), -1 = off") // tooltip
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        # ifdef P104_USE_ZONE_ACTIONS
        html_TD(); // Spacer
        addHtml(F("|"));

        html_TD(); // Action
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ACTION),
                    actionCount,
                    actionTypes,
                    actionOptions,
                    NULL,
                    P104_ACTION_NONE, // Always start with None
                    true,
                    true,
                    EMPTY_STRING);
        # endif // ifdef P104_USE_ZONE_ACTIONS

        delay(0);
      }
    }
    html_end_table();
  }

  # ifdef P104_ADD_SETTINGS_NOTES
  String devicesMsg;

  if (devicesMsg.reserve(80)) {
    devicesMsg  = F("- Maximum nr. of modules possible (Zones * Size + Offset) = 255. Last saved: ");
    devicesMsg += numDevices;
    addFormNote(devicesMsg);
  }
  addFormNote(F("- 'Animation In' or 'Animation Out' and 'Special Effects' marked with <b>*</b> should <b>not</b> be combined in a Zone."));
  #  if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  addFormNote(F("- 'Layout' 'Double upper' and 'Double lower' are only supported for numeric 'Content' types like 'Clock' and 'Date'."));
  #  endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  # endif    // ifdef P104_ADD_SETTINGS_NOTES

  return true;
}

/**************************************************************
* webform_save
**************************************************************/
bool P104_data_struct::webform_save(struct EventStruct *event) {
  P104_CONFIG_ZONE_COUNT   = getFormItemInt(F("plugin_104_zonecount"));
  P104_CONFIG_HARDWARETYPE = getFormItemInt(F("plugin_104_hardware"));

  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE, isFormItemChecked(F("plugin_104_cleardisable")));
  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT,  isFormItemChecked(F("plugin_104_logalltext")));

  # ifdef P104_USE_ZONE_ORDERING
  zoneOrder = getFormItemInt(F("plugin_104_zoneorder")); // Is used in saveSettings()
  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER, zoneOrder == 1);
  # endif // ifdef P104_USE_ZONE_ORDERING

  # ifdef P104_USE_DATETIME_OPTIONS
  uint32_t ulDateTime = 0;
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_FLASH,    !isFormItemChecked(F("plugin_104_clockflash"))); // Inverted flag
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_12H,      isFormItemChecked(F("plugin_104_clock12h")));
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_AMPM,     isFormItemChecked(F("plugin_104_clockampm")));
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_YEAR4DGT, isFormItemChecked(F("plugin_104_year4dgt")));
  set4BitToUL(ulDateTime, P104_CONFIG_DATETIME_FORMAT,   getFormItemInt(F("plugin_104_dateformat")));
  set4BitToUL(ulDateTime, P104_CONFIG_DATETIME_SEP_CHAR, getFormItemInt(F("plugin_104_dateseparator")));
  P104_CONFIG_DATETIME = ulDateTime;
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  previousZones = expectedZones;
  expectedZones = P104_CONFIG_ZONE_COUNT;

  bool result = saveSettings();         // Determines numDevices and re-fills zones list

  P104_CONFIG_ZONE_COUNT  = zones.size();
  P104_CONFIG_TOTAL_UNITS = numDevices; // Store counted number of devices

  zones.clear();                        // Free some memory (temporarily)

  return result;
}

#endif // ifdef USES_P104

#include "../PluginStructs/P039_data_struct.h"

#ifdef USES_P039

P039_data_struct::P039_data_struct(
      uint16_t       l_conversionResult,
      uint8_t        l_devicefaults,
      unsigned long  l_timer,
      bool           l_sensorFault, 
      bool           l_convReady)
  :  conversionResult(l_conversionResult), deviceFaults(l_devicefaults), timer(l_timer), sensorFault(l_sensorFault), convReady(l_convReady) {}

bool P039_data_struct::begin()
{
  return false;
}

bool P039_data_struct::read()
{
 return false;
}

bool P039_data_struct::write()
{
 return false;
}

#endif // ifdef USES_P039
#include "../PluginStructs/P105_data_struct.h"

#ifdef USES_P105

# include "../Helpers/Convert.h"

struct AHTx_Status {
  inline AHTx_Status(uint8_t stat) : status(stat) {}

  inline bool valid() const {
    return status != 0xFF;
  }

  inline bool calibrated() const {
    return (status & (1 << 3)) != 0;
  }

  inline bool busy() const {
    return (status & (1 << 7)) != 0;
  }

  const uint8_t status;
};

AHTx_Device::AHTx_Device(uint8_t addr, AHTx_device_type type) :
  i2cAddress(addr),
  device_type(type),
  last_hum_val(0.0f),
  last_temp_val(0.0f) {}

const __FlashStringHelper * AHTx_Device::getDeviceName() const {
  switch (device_type) {
    case AHTx_device_type::AHT10_DEVICE: return F("AHT10");
    case AHTx_device_type::AHT20_DEVICE: return F("AHT20");
    case AHTx_device_type::AHT21_DEVICE: return F("AHT21");
  }
  return F("AHTx");
}

bool AHTx_Device::initialize() {
  const uint8_t cmd_init = (device_type == AHTx_device_type::AHT10_DEVICE) ? 0xE1 : 0xBE;

  return I2C_write16_reg(i2cAddress, cmd_init, 0x0800);
}

bool AHTx_Device::triggerMeasurement() {
  return I2C_write16_reg(i2cAddress, 0xAC, 0x3300); // measurement time takes over 80 msec
}

bool AHTx_Device::softReset() {
  return I2C_write8(i2cAddress, 0xBA); // soft reset takes less than 20 msec
}

uint8_t AHTx_Device::readStatus() {
  return I2C_read8(i2cAddress, NULL);
}

bool AHTx_Device::readData() {
  const uint8_t data_len = 6;

  // I2C_read8 len
  if (Wire.requestFrom(i2cAddress, data_len) == 0) {
    return false;
  }

  uint8_t data[data_len];

  for (uint8_t i = 0; i < data_len; ++i) {
    data[i] = Wire.read();
  }

  // check status
  AHTx_Status status = data[0];

  if (!(status.valid() && status.calibrated())) {
    return false;
  }

  // 20 bits humidity value
  uint32_t value = data[1];

  value        = (value << 8) | data[2];
  value        = (value << 4) | (data[3] >> 4);
  last_hum_val = (static_cast<float>(value) / (1 << 20)) * 100.0f;

  // 20 bits temperature value
  value         = data[3] & 0x0F;
  value         = (value << 8) | data[4];
  value         = (value << 8) | data[5];
  last_temp_val = ((static_cast<float>(value) / (1 << 20)) * 200.0f) - 50.0f;

  return true;
}

P105_data_struct::P105_data_struct(uint8_t addr, AHTx_device_type dev) :
  device(addr, dev),
  state(AHTx_state::AHTx_Uninitialized),
  last_measurement(0),
  trigger_time(0) {}

bool P105_data_struct::initialized() const {
  return state != AHTx_state::AHTx_Uninitialized;
}

void P105_data_struct::setUninitialized() {
  state = AHTx_state::AHTx_Uninitialized;
}

// Perform the measurements with interval
bool P105_data_struct::updateMeasurements(unsigned long task_index) {
  const unsigned long current_time = millis();

  if (!initialized()) {
    String log;
    log.reserve(30);

    if (!device.initialize()) {
      log  = getDeviceName();
      log += F(" : unable to initialize");
      addLog(LOG_LEVEL_ERROR, log);
      return false;
    }
    log  = getDeviceName();
    log += F(" : initialized");
    addLog(LOG_LEVEL_INFO, log);

    trigger_time = current_time;
    state        = AHTx_state::AHTx_Trigger_measurement;
    return false;
  }

  if ((state != AHTx_state::AHTx_Wait_for_samples) && (state != AHTx_state::AHTx_Trigger_measurement)) {
    if (!timeOutReached(last_measurement + (Settings.TaskDeviceTimer[task_index] * 1000))) {
      // Timeout has not yet been reached.
      return false;
    }
    trigger_time = current_time;
    state        = AHTx_state::AHTx_Trigger_measurement;
  }

  // state: AHTx_Wait_for_samples or AHTx_Trigger_measurement
  AHTx_Status status = device.readStatus();

  if (status.valid() && status.calibrated() && !status.busy()) {
    if (state == AHTx_state::AHTx_Trigger_measurement) {
      device.triggerMeasurement();

      trigger_time = current_time;
      state        = AHTx_state::AHTx_Wait_for_samples;
      return false;
    }

    // state: AHTx_Wait_for_samples
    if (!device.readData()) {
      return false;
    }

    last_measurement = current_time;
    state            = AHTx_state::AHTx_New_values;

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Log raw measuerd values only on level DEBUG
      String log;
      log.reserve(50);                        // Prevent re-allocation
      log  = getDeviceName();
      log += F(" : humidity ");
      log += device.getHumidity();
      log += F("% temperature ");
      log += device.getTemperature();
      log += 'C';
      addLog(LOG_LEVEL_DEBUG, log);
    }

    return true;
  }

  if (timePassedSince(trigger_time) > 1000) {
    // should not happen
    String log;
    log.reserve(15); // Prevent re-allocation
    log  = getDeviceName();
    log += F(" : reset");
    addLog(LOG_LEVEL_ERROR, log);
    device.softReset();

    state = AHTx_state::AHTx_Uninitialized;
  }

  return false;
}

#endif // ifdef USES_P105

#include "../PluginStructs/P036_data_struct.h"

#ifdef USES_P036

# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"
# include "../Helpers/Scheduler.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringParser.h"
# include "../Helpers/SystemVariables.h"

# include <Dialog_Plain_12_font.h>
# include <OLED_SSD1306_SH1106_images.h>

P036_data_struct::P036_data_struct() : display(nullptr) {}

P036_data_struct::~P036_data_struct() {
  reset();
}

void P036_data_struct::reset() {
  if (display != nullptr) {
    display->displayOff();
    display->end();
    delete display;
    display = nullptr;
  }
}

const tFontSizes FontSizes[P36_MaxFontCount] = {
  { ArialMT_Plain_24, 24, 28},
  { ArialMT_Plain_16, 16, 19},
  { Dialog_plain_12,  13, 15},
  { ArialMT_Plain_10, 10, 13}
};

const tSizeSettings SizeSettings[P36_MaxSizesCount] = {
  { P36_MaxDisplayWidth, P36_MaxDisplayHeight, 0, // 128x64
       4,               // max. line count
       113, 15          // WiFi indicator
  },
  { P36_MaxDisplayWidth, 32,                   0, // 128x32
       2,               // max. line count
       113, 15          // WiFi indicator
  },
  { 64,                  48,                   32, // 64x48
       3,               // max. line count
       32,  10          // WiFi indicator
  }
};


const tSizeSettings& P036_data_struct::getDisplaySizeSettings(p036_resolution disp_resolution) {
  int index = static_cast<int>(disp_resolution);

  if ((index < 0) || (index >= P36_MaxSizesCount)) { index = 0; }

  return SizeSettings[index];
}

bool P036_data_struct::init(taskIndex_t     taskIndex,
                            uint8_t         LoadVersion,
                            uint8_t         Type,
                            uint8_t         Address,
                            uint8_t         Sda,
                            uint8_t         Scl,
                            p036_resolution Disp_resolution,
                            bool            Rotated,
                            uint8_t         Contrast,
                            uint8_t         DisplayTimer,
                            uint8_t         NrLines) {
  reset();

  lastWiFiState       = P36_WIFI_STATE_UNSET;
  disp_resolution     = p036_resolution::pix128x64;
  bAlternativHeader   = false; // start with first header content
  HeaderCount         = 0;
  bPageScrollDisabled = true;  // first page after INIT without scrolling
  TopLineOffset       = 0;     // Offset for top line, used for rotated image while using displays < P36_MaxDisplayHeight lines

  HeaderContent            = eHeaderContent::eSysName;
  HeaderContentAlternative = eHeaderContent::eSysName;
  MaxFramesToDisplay       = 0xFF;
  currentFrameToDisplay    = 0;
  nextFrameToDisplay       = 0xFF; // next frame because content changed in PLUGIN_WRITE

  ButtonState     = false;         // button not touched
  ButtonLastState = 0xFF;          // Last state checked (debouncing in progress)
  DebounceCounter = 0;             // debounce counter
  RepeatCounter   = 0;             // Repeat delay counter when holding button pressed

  displayTimer          = DisplayTimer;
  frameCounter          = 0;       // need to keep track of framecounter from call to call
  disableFrameChangeCnt = 0;       // counter to disable frame change after JumpToPage in case PLUGIN_READ already scheduled

  switch (Type) {
    case 1:
      display = new (std::nothrow) SSD1306Wire(Address, Sda, Scl);
      break;
    case 2:
      display = new (std::nothrow) SH1106Wire(Address, Sda, Scl);
      break;
    default:
      return false;
  }

  if (display != nullptr) {
    display->init(); // call to local override of init function

    disp_resolution = Disp_resolution;
    bHideFooter = !(getDisplaySizeSettings(disp_resolution).Height == P36_MaxDisplayHeight);

    if (disp_resolution == p036_resolution::pix128x32) {
      display->displayOff();
      display->SetComPins(0x02); // according to the adafruit lib, sometimes this may need to be 0x02
      bHideFooter = true;
    }

    display->displayOn();
    loadDisplayLines(taskIndex, LoadVersion);

    // Flip screen if required
    setOrientationRotated(Rotated);

    setContrast(Contrast);

    //      Display the device name, logo, time and wifi
    display_logo();
    update_display();

    //    Initialize frame counter
    frameCounter                 = 0;
    currentFrameToDisplay        = 0;
    nextFrameToDisplay           = 0;
    bPageScrollDisabled          = true;  // first page after INIT without scrolling
    ScrollingPages.linesPerFrame = NrLines;
    bLineScrollEnabled           = false; // start without line scrolling

    //    Clear scrolling line data
    for (uint8_t i = 0; i < P36_MAX_LinesPerPage; i++) {
      ScrollingLines.Line[i].Width     = 0;
      ScrollingLines.Line[i].LastWidth = 0;
    }

    //    prepare font and positions for page and line scrolling
    prepare_pagescrolling();
  }

  return isInitialized();
}

bool P036_data_struct::isInitialized() const {
  return display != nullptr;
}

void P036_data_struct::loadDisplayLines(taskIndex_t taskIndex, uint8_t LoadVersion) {
  if (LoadVersion == 0) {
    // read data of version 0 (up to 22.11.2019)
    String DisplayLinesV0[P36_Nlines];                                           // used to load the CustomTaskSettings for V0
    LoadCustomTaskSettings(taskIndex, DisplayLinesV0, P36_Nlines, P36_NcharsV0); // max. length 1024 Byte  (DAT_TASKS_CUSTOM_SIZE)

    for (int i = 0; i < P36_Nlines; ++i) {
      safe_strncpy(DisplayLinesV1[i].Content, DisplayLinesV0[i], P36_NcharsV1);
      DisplayLinesV1[i].Content[P36_NcharsV1 - 1] = 0; // Terminate in case of uninitalized data
      DisplayLinesV1[i].FontType                  = 0xff;
      DisplayLinesV1[i].FontHeight                = 0xff;
      DisplayLinesV1[i].FontSpace                 = 0xff;
      DisplayLinesV1[i].reserved                  = 0xff;
    }
  }
  else {
    // read data of version 1 (beginning from 22.11.2019)
    LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&DisplayLinesV1), sizeof(DisplayLinesV1));

    for (int i = 0; i < P36_Nlines; ++i) {
      DisplayLinesV1[i].Content[P36_NcharsV1 - 1] = 0; // Terminate in case of uninitalized data
    }
  }
}

void P036_data_struct::setContrast(uint8_t OLED_contrast) {
  if (!isInitialized()) {
    return;
  }
  char contrast  = 100;
  char precharge = 241;
  char comdetect = 64;

  switch (OLED_contrast) {
    case P36_CONTRAST_OFF:
      display->displayOff();
      return;
    case P36_CONTRAST_LOW:
      contrast = 10; precharge = 5; comdetect = 0;
      break;
    case P36_CONTRAST_MED:
      contrast = P36_CONTRAST_MED; precharge = 0x1F; comdetect = 64;
      break;
    case P36_CONTRAST_HIGH:
    default:
      contrast = P36_CONTRAST_HIGH; precharge = 241; comdetect = 64;
      break;
  }
  display->displayOn();
  display->setContrast(contrast, precharge, comdetect);
}

void P036_data_struct::setOrientationRotated(bool rotated) {
  if (rotated) {
    display->flipScreenVertically();
    TopLineOffset = P36_MaxDisplayHeight - getDisplaySizeSettings(disp_resolution).Height;
  } else {
    TopLineOffset = 0;
  }
}

void P036_data_struct::display_header() {
  if (!isInitialized()) {
    return;
  }
  if (bHideHeader) {  //  hide header
    return;
  }

  eHeaderContent iHeaderContent;
  String newString, strHeader;

  if ((HeaderContentAlternative == HeaderContent) || !bAlternativHeader) {
    iHeaderContent = HeaderContent;
  }
  else
  {
    iHeaderContent = HeaderContentAlternative;
  }

  switch (iHeaderContent) {
    case eHeaderContent::eSSID:

      if (NetworkConnected()) {
        strHeader = WiFi.SSID();
      }
      else {
        newString = F("%sysname%");
      }
      break;
    case eHeaderContent::eSysName:
      newString = F("%sysname%");
      break;
    case eHeaderContent::eTime:
      newString = F("%systime%");
      break;
    case eHeaderContent::eDate:
      newString = F("%sysday_0%.%sysmonth_0%.%sysyear%");
      break;
    case eHeaderContent::eIP:
      newString = F("%ip%");
      break;
    case eHeaderContent::eMAC:
      newString = F("%mac%");
      break;
    case eHeaderContent::eRSSI:
      newString = F("%rssi%dBm");
      break;
    case eHeaderContent::eBSSID:
      newString = F("%bssid%");
      break;
    case eHeaderContent::eWiFiCh:
      newString = F("Channel: %wi_ch%");
      break;
    case eHeaderContent::eUnit:
      newString = F("Unit: %unit%");
      break;
    case eHeaderContent::eSysLoad:
      newString = F("Load: %sysload%%");
      break;
    case eHeaderContent::eSysHeap:
      newString = F("Mem: %sysheap%");
      break;
    case eHeaderContent::eSysStack:
      newString = F("Stack: %sysstack%");
      break;
    case eHeaderContent::ePageNo:
      strHeader  = F("page ");
      strHeader += (currentFrameToDisplay + 1);

      if (MaxFramesToDisplay != 0xFF) {
        strHeader += F("/");
        strHeader += (MaxFramesToDisplay + 1);
      }
      break;
    default:
      return;
  }

  if (newString.length() > 0) {
    // Right now only systemvariables have been used, so we don't have to call the parseTemplate.
    parseSystemVariables(newString, false);
    strHeader = newString;
  }

  strHeader.trim();
  display_title(strHeader);

  // Display time and wifibars both clear area below, so paint them after the title.
  if (getDisplaySizeSettings(disp_resolution).Width == P36_MaxDisplayWidth) {
    display_time(); // only for 128pix wide displays
  }
  display_wifibars();
}

void P036_data_struct::display_time() {
  if (!isInitialized()) {
    return;
  }

  String dtime = F("%systime%");

  parseSystemVariables(dtime, false);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, 28, GetHeaderHeight() - 2);
  display->setColor(WHITE);
  display->drawString(0, TopLineOffset, dtime.substring(0, 5));
}

void P036_data_struct::display_title(const String& title) {
  if (!isInitialized()) {
    return;
  }
  display->setFont(ArialMT_Plain_10);
  display->setColor(BLACK);
  display->fillRect(0, TopLineOffset, P36_MaxDisplayWidth, GetHeaderHeight()); // don't clear line under title.
  display->setColor(WHITE);

  if (getDisplaySizeSettings(disp_resolution).Width == P36_MaxDisplayWidth) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(P36_DisplayCentre, TopLineOffset, title);
  }
  else {
    display->setTextAlignment(TEXT_ALIGN_LEFT); // Display right of WiFi bars
    display->drawString(getDisplaySizeSettings(disp_resolution).PixLeft + getDisplaySizeSettings(disp_resolution).WiFiIndicatorWidth + 3,
                        TopLineOffset,
                        title);
  }
}

void P036_data_struct::display_logo() {
  if (!isInitialized()) {
    return;
  }
  # ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("P036_DisplayLogo"));
  # endif // PLUGIN_036_DEBUG

  int left = 24;
  int top;
  tFontSettings iFontsettings = CalculateFontSettings(2); // get font with max. height for displaying "ESP Easy"

  bDisplayingLogo = true; // next time the display must be cleared completely
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(iFontsettings.fontData);
  display->clear(); // resets all pixels to black
  display->setColor(WHITE);
  display->drawString(65, iFontsettings.Top + TopLineOffset, F("ESP"));
  display->drawString(65, iFontsettings.Top + iFontsettings.Height + iFontsettings.Space + TopLineOffset, F("Easy"));

  if (getDisplaySizeSettings(disp_resolution).PixLeft < left) { left = getDisplaySizeSettings(disp_resolution).PixLeft; }
  top = (getDisplaySizeSettings(disp_resolution).Height-espeasy_logo_height)/2;
  if (top < 0) { top = 0; }
  display->drawXbm(left,
                   top+TopLineOffset,
                   espeasy_logo_width,
                   espeasy_logo_height,
                   espeasy_logo_bits); // espeasy_logo_width=espeasy_logo_height=36
}

// Draw the frame position

void P036_data_struct::display_indicator() {
  if (!isInitialized()) {
    return;
  }
  if (bHideFooter) {  //  hide footer
    return;
  }

  int frameCount = MaxFramesToDisplay + 1;

  //  Erase Indicator Area
  display->setColor(BLACK);
  display->fillRect(0, P036_IndicatorTop + TopLineOffset, P36_MaxDisplayWidth, P036_IndicatorHeight);

  // Only display when there is something to display.
  if ((frameCount <= 1) || (frameCount > P36_Nlines)) { return; }

  display->setColor(WHITE);

  // Display chars as required
  for (uint8_t i = 0; i < frameCount; i++) {
    const char *image;

    if (currentFrameToDisplay == i) {
      image = activeSymbole;
    } else {
      image = inactiveSymbole;
    }

    int x, y;

    y = P036_IndicatorTop + 2 + TopLineOffset;

    // I would like a margin of 20 pixels on each side of the indicator.
    // Therefore the width of the indicator should be 128-40=88 and so space between indicator dots is 88/(framecount-1)
    // The width of the display is 128 and so the first dot must be at x=20 if it is to be centred at 64
    const int number_spaces = frameCount - 1;

    if (number_spaces <= 0) {
      return;
    }
    int margin  = 20;
    int spacing = (P36_MaxDisplayWidth - 2 * margin) / number_spaces;

    // Now apply some max of 30 pixels between the indicators and center the row of indicators.
    if (spacing > 30) {
      spacing = 30;
      margin  = (P36_MaxDisplayWidth - number_spaces * spacing) / 2;
    }

    x = margin + (spacing * i);
    display->drawXbm(x, y, 8, 8, image);
  }
}

int16_t P036_data_struct::GetHeaderHeight()
{
  if (bHideHeader) {
    // no header
    return 0;
  }
  return P36_HeaderHeight;

}
int16_t P036_data_struct::GetIndicatorTop()
{
  if (bHideFooter) {
    // no footer (indicator) -> returm max. display height
    return getDisplaySizeSettings(disp_resolution).Height;
  }
  return P036_IndicatorTop;
}

tFontSettings P036_data_struct::CalculateFontSettings(uint8_t lDefaultLines)
{
  tFontSettings result;
  int iHeight;
  int8_t iFontIndex = -1;
  uint8_t iMaxHeightForFont;
  uint8_t iLinesPerFrame;

  if (lDefaultLines == 0) 
  {
    // number of lines can be reduced if no font fits the setting
    iLinesPerFrame = ScrollingPages.linesPerFrame;
    iHeight = GetIndicatorTop() - GetHeaderHeight();
  }
  else
  {
    // number of lines is fixed (e.g. for splash screen)
    iLinesPerFrame = lDefaultLines;
    iHeight = getDisplaySizeSettings(disp_resolution).Height;
  }
  
  while (iFontIndex < 0) {
    iMaxHeightForFont = (iHeight - (iLinesPerFrame - 1)) / iLinesPerFrame;  // at least 1 pixel space between lines

    for (uint8_t i = P36_MaxFontCount; i > 0; i--) {
      // check available fonts for the line setting  
      if (FontSizes[i-1].Height > iMaxHeightForFont) {
        // height of font is to big
        break;
      }
      iFontIndex = i-1; // save the current index
      if (FontSizes[iFontIndex].Height == iMaxHeightForFont) {
        // height of font just fits the line setting
        break;
      }
    }
    if (iFontIndex < 0) {
      // no font fits -> reduce number of lines per page
      iLinesPerFrame--;
      if (iLinesPerFrame == 0) {
        // lines per frame is at minimum
        break;
      }
    }
  }
  if (iFontIndex >= 0) {
    // font found -> calculate top position and space between lines
    iMaxHeightForFont = FontSizes[iFontIndex].Height * iLinesPerFrame;
    if (iLinesPerFrame > 1) {
      // more than one lines per frame -> calculate space inbetween
      result.Space = (iHeight-iMaxHeightForFont) / iLinesPerFrame;
    }
    else {
      // just one lines per frame -> no space inbetween
      result.Space = 0;
    }
    result.Top = (iHeight - (iMaxHeightForFont + (result.Space * (iLinesPerFrame-1)))) / 2;
  }
  else {
    // no font found -> return font with shortest height
    result.Top = 0;
    result.Space = 1;
    iLinesPerFrame = 1;
    iFontIndex = P36_MaxFontCount-1;
 }
  result.fontData = FontSizes[iFontIndex].fontData;
  result.Height = FontSizes[iFontIndex].Height;

# ifdef PLUGIN_036_DEBUG
  String log;
  log.reserve(128); // estimated
  log = F("CalculateFontSettings: FontIndex:");
  log += iFontIndex;
  log += F(" Top:");
  log += result.Top;
  log += F(" FontHeight:");
  log += result.Height;
  log += F(" Space:");
  log += result.Space;
  log += F(" Height:");
  log += iHeight;
  log += F(" LinesPerFrame:");
  log += iLinesPerFrame;
  log += F(" DefaultLines:");
  log += lDefaultLines;
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
  
  if (lDefaultLines == 0) 
    ScrollingPages.linesPerFrame = iLinesPerFrame;
  return result;
}

void P036_data_struct::prepare_pagescrolling()
{
  if (!isInitialized()) {
    return;
  }

  tFontSettings iFontsettings = CalculateFontSettings(0);

  ScrollingPages.Font    = iFontsettings.fontData;
  ScrollingPages.ypos[0] = iFontsettings.Top + GetHeaderHeight() + TopLineOffset;
  ScrollingPages.ypos[1] = ScrollingPages.ypos[0] + iFontsettings.Height + iFontsettings.Space;
  ScrollingPages.ypos[2] = ScrollingPages.ypos[1] + iFontsettings.Height + iFontsettings.Space;
  ScrollingPages.ypos[3] = ScrollingPages.ypos[2] + iFontsettings.Height + iFontsettings.Space;

  ScrollingLines.Font  = ScrollingPages.Font;
  ScrollingLines.Space = iFontsettings.Height + iFontsettings.Space + 1;

  for (uint8_t i = 0; i < P36_MAX_LinesPerPage; i++) {
    ScrollingLines.Line[i].ypos = ScrollingPages.ypos[i];
  }
}

uint8_t P036_data_struct::display_scroll(ePageScrollSpeed lscrollspeed, int lTaskTimer)
{
  if (!isInitialized()) {
    return 0;
  }

  // LineOut[] contain the outgoing strings in this frame
  // LineIn[] contain the incoming strings in this frame

  int iPageScrollTime;
  int iCharToRemove;

# ifdef PLUGIN_036_DEBUG
  String log;
  log.reserve(128); // estimated
  log = F("Start Scrolling: Speed: ");
  log += static_cast<int>(lscrollspeed);
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG

  display->setFont(ScrollingPages.Font);

  ScrollingLines.wait = 0;

  // calculate total page scrolling time
  if (lscrollspeed == ePageScrollSpeed::ePSS_Instant) {
    // no scrolling, just the handling time to build the new page
    iPageScrollTime = P36_PageScrollTick - P36_PageScrollTimer;
  } else {
    iPageScrollTime = (P36_MaxDisplayWidth / (P36_PageScrollPix * static_cast<int>(lscrollspeed))) * P36_PageScrollTick;
  }
  int iScrollTime = static_cast<float>(lTaskTimer * 1000 - iPageScrollTime - 2 * P36_WaitScrollLines * 100) / 100; // scrollTime in ms

# ifdef PLUGIN_036_DEBUG
  log  = F("PageScrollTime: ");
  log += iPageScrollTime;
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG

  uint16_t MaxPixWidthForPageScrolling = P36_MaxDisplayWidth;

  if (bLineScrollEnabled) {
    // Reduced scrolling width because line is displayed left or right aligned
    MaxPixWidthForPageScrolling -= getDisplaySizeSettings(disp_resolution).PixLeft;
  }

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    // default no line scrolling and strings are centered
    ScrollingLines.Line[j].LastWidth = 0;
    ScrollingLines.Line[j].Width     = 0;

    // get last and new line width
    uint16_t PixLengthLineOut = display->getStringWidth(ScrollingPages.LineOut[j]);
    uint16_t PixLengthLineIn  = display->getStringWidth(ScrollingPages.LineIn[j]);

    if (PixLengthLineIn > 255) {
      // shorten string because OLED controller can not handle such long strings
      int   strlen         = ScrollingPages.LineIn[j].length();
      float fAvgPixPerChar = static_cast<float>(PixLengthLineIn) / strlen;
      iCharToRemove            = ceil((static_cast<float>(PixLengthLineIn - 255)) / fAvgPixPerChar);
      ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
      PixLengthLineIn          = display->getStringWidth(ScrollingPages.LineIn[j]);
    }

    if (PixLengthLineOut > 255) {
      // shorten string because OLED controller can not handle such long strings
      int   strlen         = ScrollingPages.LineOut[j].length();
      float fAvgPixPerChar = static_cast<float>(PixLengthLineOut) / strlen;
      iCharToRemove             = ceil((static_cast<float>(PixLengthLineOut - 255)) / fAvgPixPerChar);
      ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(0, strlen - iCharToRemove);
      PixLengthLineOut          = display->getStringWidth(ScrollingPages.LineOut[j]);
    }

    if (bLineScrollEnabled) {
      // settings for following line scrolling
      if (PixLengthLineOut > getDisplaySizeSettings(disp_resolution).Width) {
        ScrollingLines.Line[j].LastWidth = PixLengthLineOut; // while page scrolling this line is right aligned
      }

      if ((PixLengthLineIn > getDisplaySizeSettings(disp_resolution).Width) && (iScrollTime > 0))
      {
        // width of the line > display width -> scroll line
        ScrollingLines.Line[j].LineContent = ScrollingPages.LineIn[j];
        ScrollingLines.Line[j].Width       = PixLengthLineIn; // while page scrolling this line is left aligned
        ScrollingLines.Line[j].CurrentLeft = getDisplaySizeSettings(disp_resolution).PixLeft;
        ScrollingLines.Line[j].fPixSum     = getDisplaySizeSettings(disp_resolution).PixLeft;

        // pix change per scrolling line tick
        ScrollingLines.Line[j].dPix = (static_cast<float>(PixLengthLineIn - getDisplaySizeSettings(disp_resolution).Width)) / iScrollTime;

# ifdef PLUGIN_036_DEBUG
        log  = String(F("Line: ")) + String(j + 1);
        log += F(" width: ");
        log += ScrollingLines.Line[j].Width;
        log += F(" dPix: ");
        log += ScrollingLines.Line[j].dPix;
        addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
      }
    }

    // reduce line content for page scrolling to max width
    if (PixLengthLineIn > MaxPixWidthForPageScrolling) {
      int strlen = ScrollingPages.LineIn[j].length();
# ifdef PLUGIN_036_DEBUG
      String LineInStr = ScrollingPages.LineIn[j];
# endif // PLUGIN_036_DEBUG
      float fAvgPixPerChar = static_cast<float>(PixLengthLineIn) / strlen;

      if (bLineScrollEnabled) {
        // shorten string on right side because line is displayed left aligned while scrolling
        // using floor() because otherwise empty space on right side
        iCharToRemove            = floor((static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
      }
      else {
        // shorten string on both sides because line is displayed centered
        // using floor() because otherwise empty space on both sides
        iCharToRemove            = floor((static_cast<float>(PixLengthLineIn - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(0, strlen - iCharToRemove);
        ScrollingPages.LineIn[j] = ScrollingPages.LineIn[j].substring(iCharToRemove);
      }
# ifdef PLUGIN_036_DEBUG
      log  = String(F("Line: ")) + String(j + 1);
      log += String(F(" LineIn: ")) + String(LineInStr);
      log += String(F(" Length: ")) + String(strlen);
      log += String(F(" PixLength: ")) + String(PixLengthLineIn);
      log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
      log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
      addLog(LOG_LEVEL_INFO, log);
      log  = String(F(" -> Changed to: ")) + String(ScrollingPages.LineIn[j]);
      log += String(F(" Length: ")) + String(ScrollingPages.LineIn[j].length());
      log += String(F(" PixLength: ")) + String(display->getStringWidth(ScrollingPages.LineIn[j]));
      addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
    }

    // reduce line content for page scrolling to max width
    if (PixLengthLineOut > MaxPixWidthForPageScrolling) {
      int strlen = ScrollingPages.LineOut[j].length();
# ifdef PLUGIN_036_DEBUG
      String LineOutStr = ScrollingPages.LineOut[j];
# endif // PLUGIN_036_DEBUG
      float fAvgPixPerChar = static_cast<float>(PixLengthLineOut) / strlen;

      if (bLineScrollEnabled) {
        // shorten string on left side because line is displayed right aligned while scrolling
        // using ceil() because otherwise overlapping the new text
        iCharToRemove             = ceil((static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / fAvgPixPerChar);
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(iCharToRemove);

        if (display->getStringWidth(ScrollingPages.LineOut[j]) > MaxPixWidthForPageScrolling) {
          // remove one more character because still overlapping the new text
          ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(1, iCharToRemove - 1);
        }
      }
      else {
        // shorten string on both sides because line is displayed centered
        // using ceil() because otherwise overlapping the new text
        iCharToRemove             = ceil((static_cast<float>(PixLengthLineOut - MaxPixWidthForPageScrolling)) / (2 * fAvgPixPerChar));
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(0, strlen - iCharToRemove);
        ScrollingPages.LineOut[j] = ScrollingPages.LineOut[j].substring(iCharToRemove);
      }
# ifdef PLUGIN_036_DEBUG
      log  = String(F("Line: ")) + String(j + 1);
      log += String(F(" LineOut: ")) + String(LineOutStr);
      log += String(F(" Length: ")) + String(strlen);
      log += String(F(" PixLength: ")) + String(PixLengthLineOut);
      log += String(F(" AvgPixPerChar: ")) + String(fAvgPixPerChar);
      log += String(F(" CharsRemoved: ")) + String(iCharToRemove);
      addLog(LOG_LEVEL_INFO, log);
      log  = String(F(" -> Changed to: ")) + String(ScrollingPages.LineOut[j]);
      log += String(F(" Length: ")) + String(ScrollingPages.LineOut[j].length());
      log += String(F(" PixLength: ")) + String(display->getStringWidth(ScrollingPages.LineOut[j]));
      addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
    }
  }

  ScrollingPages.dPix    = P36_PageScrollPix * static_cast<int>(lscrollspeed); // pix change per scrolling page tick
  ScrollingPages.dPixSum = ScrollingPages.dPix;

  display->setColor(BLACK);
  // We allow 12 pixels at the top because otherwise the wifi indicator gets too squashed!!
  // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  display->fillRect(0, GetHeaderHeight() + TopLineOffset, P36_MaxDisplayWidth, GetIndicatorTop() - GetHeaderHeight());
  display->setColor(WHITE);

  if (!bHideHeader) {
    display->drawLine(0,
                      GetHeaderHeight() + TopLineOffset,
                      P36_MaxDisplayWidth,
                      GetHeaderHeight() + TopLineOffset); // line below title
  }

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    if (lscrollspeed < ePageScrollSpeed::ePSS_Instant) { // scrolling
      if (ScrollingLines.Line[j].LastWidth > 0) {
        // width of LineOut[j] > display width -> line at beginning of scrolling page is right aligned
        display->setTextAlignment(TEXT_ALIGN_RIGHT);
        display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                            ScrollingPages.ypos[j],
                            ScrollingPages.LineOut[j]);
      }
      else {
        // line at beginning of scrolling page is centered
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->drawString(P36_DisplayCentre + ScrollingPages.dPixSum,
                            ScrollingPages.ypos[j],
                            ScrollingPages.LineOut[j]);
      }
    }

    if (ScrollingLines.Line[j].Width > 0) {
      // width of LineIn[j] > display width -> line at end of scrolling page should be left aligned
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(-P36_MaxDisplayWidth + getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
    else {
      // line at end of scrolling page should be centered
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(-P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
  }

  update_display();

  if (lscrollspeed < ePageScrollSpeed::ePSS_Instant) {
    // page scrolling (using PLUGIN_TIMER_IN)
    ScrollingPages.dPixSum += ScrollingPages.dPix;
  }
  else {
    // no page scrolling
    ScrollingPages.Scrolling = 0; // allow following line scrolling
  }
# ifdef PLUGIN_036_DEBUG
  log = F("Scrolling finished");
  addLog(LOG_LEVEL_INFO, log);
# endif // PLUGIN_036_DEBUG
  return ScrollingPages.Scrolling;
}

uint8_t P036_data_struct::display_scroll_timer() {
  if (!isInitialized()) {
    return 0;
  }

  // page scrolling (using PLUGIN_TIMER_IN)
  display->setColor(BLACK);

  // We allow 13 pixels (including underline) at the top because otherwise the wifi indicator gets too squashed!!
  // scrolling window is 42 pixels high - ie 64 less margin of 12 at top and 10 at bottom
  display->fillRect(0, GetHeaderHeight() + 1 + TopLineOffset, P36_MaxDisplayWidth, GetIndicatorTop() - GetHeaderHeight());
  display->setColor(WHITE);
  display->setFont(ScrollingPages.Font);

  for (uint8_t j = 0; j < ScrollingPages.linesPerFrame; j++)
  {
    if (ScrollingLines.Line[j].LastWidth > 0) {
      // width of LineOut[j] > display width -> line is right aligned while scrolling page
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineOut[j]);
    }
    else {
      // line is centered while scrolling page
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineOut[j]);
    }

    if (ScrollingLines.Line[j].Width > 0) {
      // width of LineIn[j] > display width -> line is left aligned while scrolling page
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(-P36_MaxDisplayWidth + getDisplaySizeSettings(disp_resolution).PixLeft + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
    else {
      // line is centered while scrolling page
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->drawString(-P36_DisplayCentre + ScrollingPages.dPixSum,
                          ScrollingPages.ypos[j],
                          ScrollingPages.LineIn[j]);
    }
  }

  update_display();

  if (ScrollingPages.dPixSum < P36_MaxDisplayWidth) { // scrolling
    // page still scrolling
    ScrollingPages.dPixSum += ScrollingPages.dPix;
  }
  else {
    // page scrolling finished
    ScrollingPages.Scrolling = 0; // allow following line scrolling
    // String log = F("Scrolling finished");
    // addLog(LOG_LEVEL_INFO, log);
  }
  return ScrollingPages.Scrolling;
}

// Draw scrolling line (1pix/s)
void P036_data_struct::display_scrolling_lines() {
  if (!isInitialized()) {
    return;
  }

  // line scrolling (using PLUGIN_TEN_PER_SECOND)

  int  i;
  bool bscroll       = false;
  bool updateDisplay = false;
  int  iCurrentLeft;

  for (i = 0; i < ScrollingPages.linesPerFrame; i++) {
    if (ScrollingLines.Line[i].Width != 0) {
      display->setFont(ScrollingLines.Font);
      bscroll = true;
      break;
    }
  }

  if (bscroll) {
    ScrollingLines.wait++;

    if (ScrollingLines.wait < P36_WaitScrollLines) {
      return; // wait before scrolling line not finished
    }

    for (i = 0; i < ScrollingPages.linesPerFrame; i++) {
      if (ScrollingLines.Line[i].Width != 0) {
        // scroll this line
        ScrollingLines.Line[i].fPixSum -= ScrollingLines.Line[i].dPix;
        iCurrentLeft                    = round(ScrollingLines.Line[i].fPixSum);

        if (iCurrentLeft != ScrollingLines.Line[i].CurrentLeft) {
          // still scrolling
          ScrollingLines.Line[i].CurrentLeft = iCurrentLeft;
          updateDisplay                      = true;
          display->setColor(BLACK);
          display->fillRect(0, ScrollingLines.Line[i].ypos + 1, P36_MaxDisplayWidth,
                            ScrollingLines.Space + 1); // clearing window was too high
          display->setColor(WHITE);

          if (((ScrollingLines.Line[i].CurrentLeft - getDisplaySizeSettings(disp_resolution).PixLeft) +
               ScrollingLines.Line[i].Width) >= getDisplaySizeSettings(disp_resolution).Width) {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->drawString(ScrollingLines.Line[i].CurrentLeft,
                                ScrollingLines.Line[i].ypos,
                                ScrollingLines.Line[i].LineContent);
          }
          else {
            // line scrolling finished -> line is shown as aligned right
            display->setTextAlignment(TEXT_ALIGN_RIGHT);
            display->drawString(P36_MaxDisplayWidth - getDisplaySizeSettings(disp_resolution).PixLeft,
                                ScrollingPages.ypos[i],
                                ScrollingLines.Line[i].LineContent);
            ScrollingLines.Line[i].Width = 0; // Stop scrolling
          }
        }
      }
    }

    if (updateDisplay && (ScrollingPages.Scrolling == 0)) { update_display(); }
  }
}

// Draw Signal Strength Bars, return true when there was an update.
bool P036_data_struct::display_wifibars() {
  if (!isInitialized()) {
    return false;
  }
  if (bHideHeader) {  //  hide header
    return false;
  }

  const bool connected    = NetworkConnected();
  const int  nbars_filled = (WiFi.RSSI() + 100) / 12; // all bars filled if RSSI better than -46dB
  const int  newState     = connected ? nbars_filled : P36_WIFI_STATE_NOT_CONNECTED;

  if (newState == lastWiFiState) {
    return false; // nothing to do.
  }
  int x         = getDisplaySizeSettings(disp_resolution).WiFiIndicatorLeft;
  int y         = TopLineOffset;
  int size_x    = getDisplaySizeSettings(disp_resolution).WiFiIndicatorWidth;
  int size_y    = GetHeaderHeight() - 2;
  int nbars     = 5;
  int16_t width = (size_x / nbars);

  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  display->setColor(BLACK);
  display->fillRect(x, y, size_x, size_y);
  display->setColor(WHITE);

  if (NetworkConnected()) {
    for (uint8_t ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos   = x + ibar * width;
      int16_t ypos   = y + size_y - height;

      if (ibar <= nbars_filled) {
        // Fill complete bar
        display->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        display->fillRect(xpos, ypos,           width - 1, 1);
        display->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign (empty rectangle)
    display->drawRect(x, y, size_x, size_y - 1);
  }
  return true;
}

void P036_data_struct::update_display()
{
  if (isInitialized()) {
    display->display();
  }
}

void P036_data_struct::P036_JumpToPage(struct EventStruct *event, uint8_t nextFrame)
{
  if (!isInitialized()) {
    return;
  }
  Scheduler.schedule_task_device_timer(event->TaskIndex,
                                       millis() + (Settings.TaskDeviceTimer[event->TaskIndex] * 1000)); // reschedule page change
  nextFrameToDisplay    = nextFrame;
  bPageScrollDisabled   = true;                                                                         //  show next page without scrolling
  disableFrameChangeCnt = 2;                                                                            //  disable next page change in
                                                                                                        // PLUGIN_READ if
  // PLUGIN_READ was already scheduled
  P036_DisplayPage(event);                                                                              //  Display the selected page,
                                                                                                        // function needs
                                                                                                        // 65ms!
  displayTimer = PCONFIG(4);                                                                            //  Restart timer
}

void P036_data_struct::P036_DisplayPage(struct EventStruct *event)
{
  # ifdef PLUGIN_036_DEBUG
  addLog(LOG_LEVEL_INFO, F("P036_DisplayPage"));
  # endif // PLUGIN_036_DEBUG

  if (!isInitialized()) {
    return;
  }

  int NFrames; // the number of frames

  if (essentiallyEqual(UserVar[event->BaseVarIndex], 1.0f)) {
    // Display is on.
    ScrollingPages.Scrolling = 1;                                                              // page scrolling running -> no
    // line scrolling allowed
    NFrames                  = P36_Nlines / ScrollingPages.linesPerFrame;
    HeaderContent            = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 8)); // Bit15-8 HeaderContent
    HeaderContentAlternative = static_cast<eHeaderContent>(get8BitFromUL(PCONFIG_LONG(0), 0)); // Bit 7-0
    // HeaderContentAlternative

    //      Construct the outgoing string
    for (uint8_t i = 0; i < ScrollingPages.linesPerFrame; i++)
    {
      String tmpString(DisplayLinesV1[(ScrollingPages.linesPerFrame * frameCounter) + i].Content);
      ScrollingPages.LineOut[i] = P36_parseTemplate(tmpString, 20);
    }

    // now loop round looking for the next frame with some content
    //   skip this frame if all lines in frame are blank
    // - we exit the while loop if any line is not empty
    bool foundText = false;
    int  ntries    = 0;

    while (!foundText) {
      //        Stop after framecount loops if no data found
      ntries += 1;

      if (ntries > NFrames) { break; }

      if (nextFrameToDisplay == 0xff) {
        // Increment the frame counter
        frameCounter++;

        if (frameCounter > NFrames - 1) {
          frameCounter          = 0;
          currentFrameToDisplay = 0;
        }
      }
      else {
        // next frame because content changed in PLUGIN_WRITE
        frameCounter = nextFrameToDisplay;
      }

      //        Contruct incoming strings
      for (uint8_t i = 0; i < ScrollingPages.linesPerFrame; i++)
      {
        String tmpString(DisplayLinesV1[(ScrollingPages.linesPerFrame * frameCounter) + i].Content);
        ScrollingPages.LineIn[i] = P36_parseTemplate(tmpString, 20);

        if (ScrollingPages.LineIn[i].length() > 0) { foundText = true; }
      }

      if (foundText) {
        if (nextFrameToDisplay == 0xff) {
          if (frameCounter != 0) {
            ++currentFrameToDisplay;
          }
        }
        else { currentFrameToDisplay = nextFrameToDisplay; }
      }
    }
    nextFrameToDisplay = 0xFF;

    // Update max page count
    if (MaxFramesToDisplay == 0xFF) {
      // not updated yet
      for (uint8_t i = 0; i < NFrames; i++) {
        for (uint8_t k = 0; k < ScrollingPages.linesPerFrame; k++)
        {
          String tmpString(DisplayLinesV1[(ScrollingPages.linesPerFrame * i) + k].Content);
          tmpString = P36_parseTemplate(tmpString, 20);

          if (tmpString.length() > 0) {
            // page not empty
            if (MaxFramesToDisplay == 0xFF) {
              MaxFramesToDisplay = 0;
            } else {
              MaxFramesToDisplay++;
            }
            break;
          }
        }
      }

      if (MaxFramesToDisplay == 0xFF) {
        // nothing to display
        MaxFramesToDisplay = 0;
      }
    }

    //      Update display
    if (bDisplayingLogo) {
      bDisplayingLogo = false;
      display->clear(); // resets all pixels to black
    }

    bAlternativHeader = false; // start with first header content
    HeaderCount       = 0;     // reset header count
    display_header();

    display_indicator();

    update_display();

    bool bScrollWithoutWifi = bitRead(PCONFIG_LONG(0), 24);                            // Bit 24
    bool bScrollLines       = bitRead(PCONFIG_LONG(0), 17);                            // Bit 17
    bLineScrollEnabled = (bScrollLines && (NetworkConnected() || bScrollWithoutWifi)); // scroll lines only if WifiIsConnected,
    // otherwise too slow

    ePageScrollSpeed lscrollspeed = static_cast<ePageScrollSpeed>(PCONFIG(3));

    if (bPageScrollDisabled) { lscrollspeed = ePageScrollSpeed::ePSS_Instant; // first page after INIT without scrolling
    }
    int lTaskTimer = Settings.TaskDeviceTimer[event->TaskIndex];

    if (display_scroll(lscrollspeed, lTaskTimer)) {
      Scheduler.setPluginTaskTimer(P36_PageScrollTimer, event->TaskIndex, event->Par1); // calls next page scrollng tick
    }

    if (NetworkConnected() || bScrollWithoutWifi) {
      // scroll lines only if WifiIsConnected, otherwise too slow
      bPageScrollDisabled = false; // next PLUGIN_READ will do page scrolling
    }
  } else {
    # ifdef PLUGIN_036_DEBUG
    addLog(LOG_LEVEL_INFO, F("P036_DisplayPage Display off"));
    # endif // PLUGIN_036_DEBUG
  }
}

// Perform some specific changes for OLED display
String P036_data_struct::P36_parseTemplate(String& tmpString, uint8_t lineSize) {
  String result = parseTemplate_padded(tmpString, lineSize);

  // OLED lib uses this routine to convert UTF8 to extended ASCII
  // http://playground.arduino.cc/Main/Utf8ascii
  // Attempt to display euro sign (FIXME)

  /*
     const char euro[4] = {0xe2, 0x82, 0xac, 0}; // Unicode euro symbol
     const char euro_oled[3] = {0xc2, 0x80, 0}; // Euro symbol OLED display font
     result.replace(euro, euro_oled);
   */
  result.trim();
  return result;
}

void P036_data_struct::registerButtonState(uint8_t newButtonState, bool bPin3Invers) {
  if ((ButtonLastState == 0xFF) || (bPin3Invers != (!!newButtonState))) {
    ButtonLastState = newButtonState;
    DebounceCounter++;

    if (RepeatCounter > 0) {
      RepeatCounter--;      // decrease the repeat count
    }
  } else {
    ButtonLastState = 0xFF; // Reset
    DebounceCounter = 0;
    RepeatCounter   = 0;
    ButtonState     = false;
  }

  if ((ButtonLastState == newButtonState) &&
      (DebounceCounter >= P36_DebounceTreshold) &&
      (RepeatCounter == 0)) {
    ButtonState = true;
  }
}

void P036_data_struct::markButtonStateProcessed() {
  ButtonState     = false;
  DebounceCounter = 0;
  RepeatCounter   = P36_RepeatDelay; //  Wait a bit before repeating the button action
}

#endif // ifdef USES_P036

#include "../PluginStructs/P027_data_struct.h"

#ifdef USES_P027


# define INA219_READ                            (0x01)
# define INA219_REG_CONFIG                      (0x00)
# define INA219_CONFIG_RESET                    (0x8000) // Reset Bit

# define INA219_CONFIG_BVOLTAGERANGE_MASK       (0x2000) // Bus Voltage Range Mask
# define INA219_CONFIG_BVOLTAGERANGE_16V        (0x0000) // 0-16V Range
# define INA219_CONFIG_BVOLTAGERANGE_32V        (0x2000) // 0-32V Range

# define INA219_CONFIG_GAIN_MASK                (0x1800) // Gain Mask
# define INA219_CONFIG_GAIN_1_40MV              (0x0000) // Gain 1, 40mV Range
# define INA219_CONFIG_GAIN_2_80MV              (0x0800) // Gain 2, 80mV Range
# define INA219_CONFIG_GAIN_4_160MV             (0x1000) // Gain 4, 160mV Range
# define INA219_CONFIG_GAIN_8_320MV             (0x1800) // Gain 8, 320mV Range

# define INA219_CONFIG_BADCRES_MASK             (0x0780) // Bus ADC Resolution Mask
# define INA219_CONFIG_BADCRES_9BIT             (0x0080) // 9-bit bus res = 0..511
# define INA219_CONFIG_BADCRES_10BIT            (0x0100) // 10-bit bus res = 0..1023
# define INA219_CONFIG_BADCRES_11BIT            (0x0200) // 11-bit bus res = 0..2047
# define INA219_CONFIG_BADCRES_12BIT            (0x0400) // 12-bit bus res = 0..4097

# define INA219_CONFIG_SADCRES_MASK             (0x0078) // Shunt ADC Resolution and Averaging Mask
# define INA219_CONFIG_SADCRES_9BIT_1S_84US     (0x0000) // 1 x 9-bit shunt sample
# define INA219_CONFIG_SADCRES_10BIT_1S_148US   (0x0008) // 1 x 10-bit shunt sample
# define INA219_CONFIG_SADCRES_11BIT_1S_276US   (0x0010) // 1 x 11-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018) // 1 x 12-bit shunt sample
# define INA219_CONFIG_SADCRES_12BIT_2S_1060US  (0x0048) // 2 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_4S_2130US  (0x0050) // 4 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_8S_4260US  (0x0058) // 8 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_16S_8510US (0x0060) // 16 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_32S_17MS   (0x0068) // 32 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_64S_34MS   (0x0070) // 64 x 12-bit shunt samples averaged together
# define INA219_CONFIG_SADCRES_12BIT_128S_69MS  (0x0078) // 128 x 12-bit shunt samples averaged together

# define INA219_CONFIG_MODE_MASK                (0x0007) // Operating Mode Mask
# define INA219_CONFIG_MODE_POWERDOWN           (0x0000)
# define INA219_CONFIG_MODE_SVOLT_TRIGGERED     (0x0001)
# define INA219_CONFIG_MODE_BVOLT_TRIGGERED     (0x0002)
# define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED (0x0003)
# define INA219_CONFIG_MODE_ADCOFF              (0x0004)
# define INA219_CONFIG_MODE_SVOLT_CONTINUOUS    (0x0005)
# define INA219_CONFIG_MODE_BVOLT_CONTINUOUS    (0x0006)
# define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

# define INA219_REG_SHUNTVOLTAGE                (0x01)
# define INA219_REG_BUSVOLTAGE                  (0x02)
# define INA219_REG_POWER                       (0x03)
# define INA219_REG_CURRENT                     (0x04)
# define INA219_REG_CALIBRATION                 (0x05)


P027_data_struct::P027_data_struct(uint8_t i2c_addr) : i2caddr(i2c_addr) {}

void P027_data_struct::setCalibration_32V_2A() {
  calValue = 4027;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

void P027_data_struct::setCalibration_32V_1A() {
  calValue = 10240;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 25; // Current LSB = 40uA per bit (1000/40 = 25)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

void P027_data_struct::setCalibration_16V_400mA() {
  calValue = 8192;

  // Set multipliers to convert raw current/power values
  currentDivider_mA = 20; // Current LSB = 50uA per bit (1000/50 = 20)

  // Set Calibration register to 'Cal' calculated above
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
                    INA219_CONFIG_GAIN_1_40MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  wireWriteRegister(INA219_REG_CONFIG, config);
}

int16_t P027_data_struct::getBusVoltage_raw() {
  uint16_t value;

  wireReadRegister(INA219_REG_BUSVOLTAGE, &value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

int16_t P027_data_struct::getShuntVoltage_raw() {
  uint16_t value;

  wireReadRegister(INA219_REG_SHUNTVOLTAGE, &value);
  return (int16_t)value;
}

int16_t P027_data_struct::getCurrent_raw() {
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  wireWriteRegister(INA219_REG_CALIBRATION, calValue);

  // Now we can safely read the CURRENT register!
  wireReadRegister(INA219_REG_CURRENT, &value);

  return (int16_t)value;
}

float P027_data_struct::getShuntVoltage_mV() {
  int16_t value;

  value = P027_data_struct::getShuntVoltage_raw();
  return value * 0.01f;
}

float P027_data_struct::getBusVoltage_V() {
  int16_t value = getBusVoltage_raw();

  return value * 0.001f;
}

float P027_data_struct::getCurrent_mA() {
  float valueDec = getCurrent_raw();

  valueDec /= currentDivider_mA;
  return valueDec;
}

void P027_data_struct::wireWriteRegister(uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(reg);                 // Register
  Wire.write((value >> 8) & 0xFF); // Upper 8-bits
  Wire.write(value & 0xFF);        // Lower 8-bits
  Wire.endTransmission();
}

void P027_data_struct::wireReadRegister(uint8_t reg, uint16_t *value)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(reg); // Register
  Wire.endTransmission();

  delay(1);        // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(i2caddr, (uint8_t)2);

  // Shift values to create properly formed integer
  *value = ((Wire.read() << 8) | Wire.read());
}

#endif // ifdef USES_P027

#include "../PluginStructs/P090_data_struct.h"

#ifdef USES_P090

#include "../Globals/I2Cdev.h"

// Register addresses
# define CSS811_STATUS          0x00
# define CSS811_MEAS_MODE       0x01
# define CSS811_ALG_RESULT_DATA 0x02
# define CSS811_RAW_DATA        0x03
# define CSS811_ENV_DATA        0x05
# define CSS811_NTC             0x06
# define CSS811_THRESHOLDS      0x10
# define CSS811_BASELINE        0x11
# define CSS811_HW_ID           0x20
# define CSS811_HW_VERSION      0x21
# define CSS811_FW_BOOT_VERSION 0x23
# define CSS811_FW_APP_VERSION  0x24
# define CSS811_ERROR_ID        0xE0
# define CSS811_APP_START       0xF4
# define CSS811_SW_RESET        0xFF

// ****************************************************************************//
//
//  LIS3DHCore functions
//
//  For I2C, construct LIS3DHCore myIMU(<address>);
//
//  Default <address> is 0x5B.
//
// ****************************************************************************//
CCS811Core::CCS811Core(uint8_t inputArg) : I2CAddress(inputArg)
{}

void CCS811Core::setAddress(uint8_t address)
{
  I2CAddress = address;
}

CCS811Core::status CCS811Core::beginCore(void)
{
  CCS811Core::status returnError = SENSOR_SUCCESS;

  // Wire.begin(); // not necessary

    # ifdef __AVR__
    # else
    # endif

    # ifdef __MK20DX256__
    # else
    # endif

    # ifdef ARDUINO_ARCH_ESP8266
    # else
    # endif

  // Spin for a few ms
  volatile uint8_t temp = 0;

  for (uint16_t i = 0; i < 10000; i++)
  {
    temp++;
  }

  while (Wire.available()) // Clear wire as a precaution
  {
    Wire.read();
  }

  // Check the ID register to determine if the operation was a success.
  uint8_t readCheck;
  readCheck   = 0;
  returnError = readRegister(CSS811_HW_ID, &readCheck);

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  if (readCheck != 0x81)
  {
    returnError = SENSOR_ID_ERROR;
  }

  return returnError;
} // CCS811Core::beginCore

// ****************************************************************************//
//
//  ReadRegister
//
//  Parameters:
//    offset -- register to read
//    *outputPointer -- Pass &variable (address of) to save read data to
//
// ****************************************************************************//
CCS811Core::status CCS811Core::readRegister(uint8_t offset, uint8_t *outputPointer)
{
  bool wire_status = false;

  *outputPointer = I2C_read8_reg(I2CAddress, offset, &wire_status);

  if (wire_status) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// ****************************************************************************//
//
//  writeRegister
//
//  Parameters:
//    offset -- register to write
//    dataToWrite -- 8 bit data to write to register
//
// ****************************************************************************//
CCS811Core::status CCS811Core::writeRegister(uint8_t offset, uint8_t dataToWrite)
{
  if (I2C_write8_reg(I2CAddress, offset, dataToWrite)) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// ****************************************************************************//
//
//  multiReadRegister
//
//  Parameters:
//    offset -- register to read
//    *inputPointer -- Pass &variable (base address of) to save read data to
//    length -- number of bytes to read
//
//  Note:  Does not know if the target memory space is an array or not, or
//    if there is the array is big enough.  if the variable passed is only
//    two bytes long and 3 bytes are requested, this will over-write some
//    other memory!
//
// ****************************************************************************//
CCS811Core::status CCS811Core::multiWriteRegister(uint8_t offset, uint8_t *inputPointer, uint8_t length)
{
  CCS811Core::status returnError = SENSOR_SUCCESS;

  // define pointer that will point to the external space
  uint8_t i = 0;

  // Set the address
  Wire.beginTransmission(I2CAddress);
  Wire.write(offset);

  while (i < length)           // send data bytes
  {
    Wire.write(*inputPointer); // receive a uint8_t as character
    inputPointer++;
    i++;
  }

  if (Wire.endTransmission() != 0)
  {
    returnError = SENSOR_I2C_ERROR;
  }

  return returnError;
}

// ****************************************************************************//
//
//  Main user class -- wrapper for the core class + maths
//
//  Construct with same rules as the core ( uint8_t busType, uint8_t inputArg )
//
// ****************************************************************************//
CCS811::CCS811(uint8_t inputArg) : CCS811Core(inputArg)
{
  refResistance = 10000;
  resistance    = 0;
  _temperature  = 0;
  tVOC          = 0;
  CO2           = 0;
}

// ****************************************************************************//
//
//  Begin
//
//  This starts the lower level begin, then applies settings
//
// ****************************************************************************//
CCS811Core::status CCS811::begin(void)
{
  uint8_t data[4] = { 0x11, 0xE5, 0x72, 0x8A };    // Reset key

  CCS811Core::status returnError = SENSOR_SUCCESS; // Default error state

  // restart the core
  returnError = beginCore();

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  // Reset the device
  multiWriteRegister(CSS811_SW_RESET, data, 4);
  delay(1);

  if (checkForStatusError() == true)
  {
    return SENSOR_INTERNAL_ERROR;
  }

  if (appValid() == false)
  {
    return SENSOR_INTERNAL_ERROR;
  }

  // Write 0 bytes to this register to start app
  Wire.beginTransmission(I2CAddress);
  Wire.write(CSS811_APP_START);

  if (Wire.endTransmission() != 0)
  {
    return SENSOR_I2C_ERROR;
  }

  delay(200);

  // returnError = setDriveMode(1); //Read every second
  //    Serial.println();

  return returnError;
} // CCS811::begin

// ****************************************************************************//
//
//  Sensor functions
//
// ****************************************************************************//
// Updates the total voltatile organic compounds (TVOC) in parts per billion (PPB)
// and the CO2 value
// Returns nothing
CCS811Core::status CCS811::readAlgorithmResults(void)
{
  I2Cdata_bytes data(4, CSS811_ALG_RESULT_DATA);
  bool allDataRead = I2C_read_bytes(I2CAddress, data);

  if (!allDataRead) {
    return SENSOR_I2C_ERROR;
  }

  // Data ordered:
  // co2MSB, co2LSB, tvocMSB, tvocLSB

  CO2  = ((uint16_t)data[CSS811_ALG_RESULT_DATA + 0] << 8) | data[CSS811_ALG_RESULT_DATA + 1];
  tVOC = ((uint16_t)data[CSS811_ALG_RESULT_DATA + 2] << 8) | data[CSS811_ALG_RESULT_DATA + 3];
  return SENSOR_SUCCESS;
}

// Checks to see if error bit is set
bool CCS811::checkForStatusError(void)
{
  uint8_t value;

  // return the status bit
  readRegister(CSS811_STATUS, &value);
  return value & (1 << 0);
}

// Checks to see if DATA_READ flag is set in the status register
bool CCS811::dataAvailable(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_STATUS, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return false;
  }
  else
  {
    return value & (1 << 3);
  }
}

// Checks to see if APP_VALID flag is set in the status register
bool CCS811::appValid(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_STATUS, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return false;
  }
  else
  {
    return value & (1 << 4);
  }
}

uint8_t CCS811::getErrorRegister(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_ERROR_ID, &value);

  if (returnError != SENSOR_SUCCESS)
  {
    return 0xFF;
  }
  else
  {
    return value; // Send all errors in the event of communication error
  }
}

// Returns the baseline value
// Used for telling sensor what 'clean' air is
// You must put the sensor in clean air and record this value
uint16_t CCS811::getBaseline(void)
{
  return I2C_read16_reg(I2CAddress, CSS811_BASELINE);
}

CCS811Core::status CCS811::setBaseline(uint16_t input)
{
  if (I2C_write16_reg(I2CAddress, CSS811_BASELINE, input)) {
    return SENSOR_SUCCESS;
  }
  return SENSOR_I2C_ERROR;
}

// Enable the nINT signal
CCS811Core::status CCS811::enableInterrupts(void)
{
  uint8_t value;
  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  //    Serial.println(value, HEX);
  value |= (1 << 3); // Set INTERRUPT bit
  writeRegister(CSS811_MEAS_MODE, value);

  //    Serial.println(value, HEX);
  return returnError;
}

// Disable the nINT signal
CCS811Core::status CCS811::disableInterrupts(void)
{
  uint8_t value;

  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  value      &= ~(1 << 3); // Clear INTERRUPT bit
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Mode 0 = Idle
// Mode 1 = read every 1s
// Mode 2 = every 10s
// Mode 3 = every 60s
// Mode 4 = RAW mode
CCS811Core::status CCS811::setDriveMode(uint8_t mode)
{
  if (mode > 4)
  {
    mode = 4; // sanitize input
  }

  uint8_t value;
  CCS811Core::status returnError = readRegister(CSS811_MEAS_MODE, &value); // Read what's currently there

  if (returnError != SENSOR_SUCCESS)
  {
    return returnError;
  }

  value      &= ~(0b00000111 << 4); // Clear DRIVE_MODE bits
  value      |= (mode << 4);        // Mask in mode
  returnError = writeRegister(CSS811_MEAS_MODE, value);
  return returnError;
}

// Given a temp and humidity, write this data to the CSS811 for better compensation
// This function expects the humidity and temp to come in as floats
CCS811Core::status CCS811::setEnvironmentalData(float relativeHumidity, float temperature)
{
  // Check for invalid temperatures
  if ((temperature < -25) || (temperature > 50))
  {
    return SENSOR_GENERIC_ERROR;
  }

  // Check for invalid humidity
  if ((relativeHumidity < 0) || (relativeHumidity > 100))
  {
    return SENSOR_GENERIC_ERROR;
  }

  uint32_t rH   = relativeHumidity * 1000; // 42.348 becomes 42348
  uint32_t temp = temperature * 1000;      // 23.2 becomes 23200

  uint8_t envData[4];

  //Split value into 7-bit integer and 9-bit fractional

  //Incorrect way from datasheet.
  //envData[0] = ((rH % 1000) / 100) > 7 ? (rH / 1000 + 1) << 1 : (rH / 1000) << 1;
  //envData[1] = 0; //CCS811 only supports increments of 0.5 so bits 7-0 will always be zero
  //if (((rH % 1000) / 100) > 2 && (((rH % 1000) / 100) < 8))
  //{
  //	envData[0] |= 1; //Set 9th bit of fractional to indicate 0.5%
  //}

  //Correct rounding. See issue 8: https://github.com/sparkfun/Qwiic_BME280_CCS811_Combo/issues/8
  envData[0] = (rH + 250) / 500;
  envData[1] = 0; //CCS811 only supports increments of 0.5 so bits 7-0 will always be zero

  temp += 25000; //Add the 25C offset
  //Split value into 7-bit integer and 9-bit fractional
  //envData[2] = ((temp % 1000) / 100) > 7 ? (temp / 1000 + 1) << 1 : (temp / 1000) << 1;
  //envData[3] = 0;
  //if (((temp % 1000) / 100) > 2 && (((temp % 1000) / 100) < 8))
  //{
  //	envData[2] |= 1;  //Set 9th bit of fractional to indicate 0.5C
  //}

  //Correct rounding
  envData[2] = (temp + 250) / 500;
  envData[3] = 0;

  CCS811Core::status returnError = multiWriteRegister(CSS811_ENV_DATA, envData, 4);

  return returnError;
} // CCS811::setEnvironmentalData

void CCS811::setRefResistance(float input)
{
  refResistance = input;
}

CCS811Core::status CCS811::readNTC(void)
{
  I2Cdata_bytes data(4, CSS811_NTC);
  bool allDataRead = I2C_read_bytes(I2CAddress, data);

  if (!allDataRead) {
    return SENSOR_I2C_ERROR;
  }

  vrefCounts = (static_cast<uint16_t>(data[CSS811_NTC + 0]) << 8) | data[CSS811_NTC + 1];

  // Serial.print("vrefCounts: ");
  // Serial.println(vrefCounts);
  ntcCounts = (static_cast<uint16_t>(data[CSS811_NTC + 2]) << 8) | data[CSS811_NTC + 3];

  // Serial.print("ntcCounts: ");
  // Serial.println(ntcCounts);
  // Serial.print("sum: ");
  // Serial.println(ntcCounts + vrefCounts);
  resistance = (static_cast<float>(ntcCounts) * refResistance / static_cast<float>(vrefCounts));

  // Code from Milan Malesevic and Zoran Stupic, 2011,
  // Modified by Max Mayfield,
  _temperature = log(static_cast<long>(resistance));
  _temperature = 1  / (0.001129148f + (0.000234125f * _temperature) + (0.0000000876741f * _temperature * _temperature * _temperature));
  _temperature = _temperature - 273.15f; // Convert Kelvin to Celsius

  return SENSOR_SUCCESS;
}

uint16_t CCS811::getTVOC(void)
{
  return tVOC;
}

uint16_t CCS811::getCO2(void)
{
  return CO2;
}

float CCS811::getResistance(void)
{
  return resistance;
}

float CCS811::getTemperature(void)
{
  return _temperature;
}

// getDriverError decodes the CCS811Core::status type and prints the
// type of error to the serial terminal.
//
// Save the return value of any function of type CCS811Core::status, then pass
// to this function to see what the output was.
const __FlashStringHelper * CCS811::getDriverError(CCS811Core::status errorCode)
{
  switch (errorCode)
  {
    case CCS811Core::SENSOR_SUCCESS:
      return F("SUCCESS");

    case CCS811Core::SENSOR_ID_ERROR:
      return F("ID_ERROR");

    case CCS811Core::SENSOR_I2C_ERROR:
      return F("I2C_ERROR");

    case CCS811Core::SENSOR_INTERNAL_ERROR:
      return F("INTERNAL_ERROR");

    case CCS811Core::SENSOR_GENERIC_ERROR:
      return F("GENERIC_ERROR");

    default:
      return F("Unknown");
  }
}

// getSensorError gets, clears, then prints the errors
// saved within the error register.
const __FlashStringHelper * CCS811::getSensorError()
{
  uint8_t error = getErrorRegister();

  if (error == 0xFF)
  {
    return F("Failed to get ERROR_ID register.");
  }
  else
  {
    if (error & 1 << 5)
    {
      return F("HeaterSupply");
    }

    if (error & 1 << 4)
    {
      return F("HeaterFault");
    }

    if (error & 1 << 3)
    {
      return F("MaxResistance");
    }

    if (error & 1 << 2)
    {
      return F("MeasModeInvalid");
    }

    if (error & 1 << 1)
    {
      return F("ReadRegInvalid");
    }

    if (error & 1 << 0)
    {
      return F("MsgInvalid");
    }
  }
  return F("");
}

P090_data_struct::P090_data_struct(uint8_t i2cAddr) :
  myCCS811(0x5B) // start with default, but will update later on with user settings
{
  myCCS811.setAddress(i2cAddr);
}

#endif // ifdef USES_P090

#include "../PluginStructs/P087_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <ESPeasySerial.h>
#include <Regexp.h>


#ifdef USES_P087


P087_data_struct::P087_data_struct() :  easySerial(nullptr) {}

P087_data_struct::~P087_data_struct() {
  reset();
}

void P087_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P087_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (isInitialized()) {
    easySerial->begin(baudrate);
    return true;
  }
  return false;
}

void P087_data_struct::post_init() {
  for (uint8_t i = 0; i < P87_MAX_CAPTURE_INDEX; ++i) {
    capture_index_used[i] = false;
  }
  regex_empty = _lines[P087_REGEX_POS].isEmpty();
  String log = F("P087_post_init:");

  for (uint8_t i = 0; i < P087_NR_FILTERS; ++i) {
    // Create some quick lookup table to see if we have a filter for the specific index
    capture_index_must_not_match[i] = _lines[i * 3 + P087_FIRST_FILTER_POS + 1].toInt() == P087_Filter_Comp::NotEqual;
    int index = _lines[i * 3 + P087_FIRST_FILTER_POS].toInt();

    // Index is negative when not used.
    if ((index >= 0) && (index < P87_MAX_CAPTURE_INDEX) && (_lines[i * 3 + P087_FIRST_FILTER_POS + 2].length() > 0)) {
      log                      += ' ';
      log                      += String(i);
      log                      += ':';
      log                      += String(index);
      capture_index[i]          = index;
      capture_index_used[index] = true;
    }
  }
  addLog(LOG_LEVEL_DEBUG, log);
}

bool P087_data_struct::isInitialized() const {
  return easySerial != nullptr;
}

void P087_data_struct::sendString(const String& data) {
  if (isInitialized()) {
    if (data.length() > 0) {
      setDisableFilterWindowTimer();
      easySerial->write(data.c_str());

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Proxy: Sending: ");
        log += data;
        addLog(LOG_LEVEL_INFO, log);
      }
    }
  }
}

bool P087_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool fullSentenceReceived = false;

  if (easySerial != nullptr) {
    int available = easySerial->available();

    while (available > 0 && !fullSentenceReceived) {
      // Look for end marker
      char c = easySerial->read();
      --available;

      if (available == 0) {
        available = easySerial->available();
        delay(0);
      }

      switch (c) {
        case 13:
        {
          const size_t length = sentence_part.length();
          bool valid          = length > 0;

          for (size_t i = 0; i < length && valid; ++i) {
            if ((sentence_part[i] > 127) || (sentence_part[i] < 32)) {
              sentence_part = "";
              ++sentences_received_error;
              valid = false;
            }
          }

          if (valid) {
            fullSentenceReceived = true;
            last_sentence = sentence_part;
            sentence_part = "";
          }
          break;
        }
        case 10:

          // Ignore LF
          break;
        default:
          sentence_part += c;
          break;
      }

      if (max_length_reached()) { fullSentenceReceived = true; }
    }
  }

  if (fullSentenceReceived) {
    ++sentences_received;
    length_last_received = last_sentence.length();
  }
  return fullSentenceReceived;
}

bool P087_data_struct::getSentence(String& string) {
  string        = last_sentence;
  if (string.isEmpty()) {
    return false;
  }
  last_sentence = "";
  return true;
}

void P087_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
  succes      = sentences_received;
  error       = sentences_received_error;
  length_last = length_last_received;
}

void P087_data_struct::setMaxLength(uint16_t maxlenght) {
  max_length = maxlenght;
}

void P087_data_struct::setLine(uint8_t varNr, const String& line) {
  if (varNr < P87_Nlines) {
    _lines[varNr] = line;
  }
}

String P087_data_struct::getRegEx() const {
  return _lines[P087_REGEX_POS];
}

uint16_t P087_data_struct::getRegExpMatchLength() const {
  return _lines[P087_NR_CHAR_USE_POS].toInt();
}

uint32_t P087_data_struct::getFilterOffWindowTime() const {
  return _lines[P087_FILTER_OFF_WINDOW_POS].toInt();
}

P087_Match_Type P087_data_struct::getMatchType() const {
  return static_cast<P087_Match_Type>(_lines[P087_MATCH_TYPE_POS].toInt());
}

bool P087_data_struct::invertMatch() const {
  switch (getMatchType()) {
    case Regular_Match:          // fallthrough
    case Global_Match:
      break;
    case Regular_Match_inverted: // fallthrough
    case Global_Match_inverted:
      return true;
    case Filter_Disabled:
      break;
  }
  return false;
}

bool P087_data_struct::globalMatch() const {
  switch (getMatchType()) {
    case Regular_Match: // fallthrough
    case Regular_Match_inverted:
      break;
    case Global_Match:  // fallthrough
    case Global_Match_inverted:
      return true;
    case Filter_Disabled:
      break;
  }
  return false;
}

String P087_data_struct::getFilter(uint8_t lineNr, uint8_t& capture, P087_Filter_Comp& comparator) const
{
  uint8_t varNr = lineNr * 3 + P087_FIRST_FILTER_POS;

  if ((varNr + 3) > P87_Nlines) { return ""; }

  capture    = _lines[varNr++].toInt();
  comparator = _lines[varNr++] == "1" ? P087_Filter_Comp::NotEqual : P087_Filter_Comp::Equal;
  return _lines[varNr];
}

void P087_data_struct::setDisableFilterWindowTimer() {
  if (getFilterOffWindowTime() == 0) {
    disable_filter_window = 0;
  }
  else {
    disable_filter_window = millis() + getFilterOffWindowTime();
  }
}

bool P087_data_struct::disableFilterWindowActive() const {
  if (disable_filter_window != 0) {
    if (!timeOutReached(disable_filter_window)) {
      // We're still in the window where filtering is disabled
      return true;
    }
  }
  return false;
}

typedef std::pair<uint8_t, String> capture_tuple;
static std::vector<capture_tuple> capture_vector;


// called for each match
void P087_data_struct::match_callback(const char *match, const unsigned int length, const MatchState& ms)
{
  for (uint8_t i = 0; i < ms.level; i++)
  {
    capture_tuple tuple;
    tuple.first  = i;
    tuple.second = ms.GetCapture(i);
    capture_vector.push_back(tuple);
  } // end of for each capture
}

bool P087_data_struct::matchRegexp(String& received) const {
  size_t strlength = received.length();

  if (strlength == 0) {
    return false;
  }
  if (regex_empty || getMatchType() == Filter_Disabled) {
    return true;
  }


  uint32_t regexp_match_length = getRegExpMatchLength();

  if ((regexp_match_length > 0) && (strlength > regexp_match_length)) {
    strlength = regexp_match_length;
  }

  // We need to do a const_cast here, but this only is valid as long as we
  // don't call a replace function from regexp.
  MatchState ms(const_cast<char *>(received.c_str()), strlength);

  bool match_result = false;
  if (globalMatch()) {
    capture_vector.clear();
    ms.GlobalMatch(_lines[P087_REGEX_POS].c_str(), match_callback);
    const uint8_t vectorlength = capture_vector.size();

    for (uint8_t i = 0; i < vectorlength; ++i) {
      if ((capture_vector[i].first < P87_MAX_CAPTURE_INDEX) && capture_index_used[capture_vector[i].first]) {
        for (uint8_t n = 0; n < P087_NR_FILTERS; ++n) {
          unsigned int lines_index = n * 3 + P087_FIRST_FILTER_POS + 2;

          if ((capture_index[n] == capture_vector[i].first) && !(_lines[lines_index].isEmpty())) {
            String log;
            log.reserve(32);
            log  = F("P087: Index: ");
            log += capture_vector[i].first;
            log += F(" Found ");
            log += capture_vector[i].second;

            // Found a Capture Filter with this capture index.
            if (capture_vector[i].second == _lines[lines_index]) {
              log += F(" Matches");

              // Found a match. Now check if it is supposed to be one or not.
              if (capture_index_must_not_match[n]) {
                log += F(" (!=)");
                addLog(LOG_LEVEL_INFO, log);
                return false;
              } else {
                match_result = true;
                log         += F(" (==)");
              }
            } else {
              log += F(" No Match");

              if (capture_index_must_not_match[n]) {
                log += F(" (!=) ");
              } else {
                log += F(" (==) ");
              }
              log += _lines[lines_index];
            }
            addLog(LOG_LEVEL_INFO, log);
          }
        }
      }
    }
    capture_vector.clear();
  } else {
    char result = ms.Match(_lines[P087_REGEX_POS].c_str());

    if (result == REGEXP_MATCHED) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("Match at: ");
        log += ms.MatchStart;
        log += F(" Match Length: ");
        log += ms.MatchLength;
        addLog(LOG_LEVEL_DEBUG, log);
      }
      match_result = true;
    }
  }
  return match_result;
}

const __FlashStringHelper * P087_data_struct::MatchType_toString(P087_Match_Type matchType) {
  switch (matchType)
  {
    case P087_Match_Type::Regular_Match:          return F("Regular Match");
    case P087_Match_Type::Regular_Match_inverted: return F("Regular Match inverted");
    case P087_Match_Type::Global_Match:           return F("Global Match");
    case P087_Match_Type::Global_Match_inverted:  return F("Global Match inverted");
    case P087_Match_Type::Filter_Disabled:        return F("Filter Disabled");
  }
  return F("");
}

bool P087_data_struct::max_length_reached() const {
  if (max_length == 0) { return false; }
  return sentence_part.length() >= max_length;
}

#endif // USES_P087

#include "../PluginStructs/P015_data_struct.h"
#ifdef USES_P015

#include "../Helpers/Misc.h"



# define TSL2561_CMD           0x80
# define TSL2561_REG_CONTROL   0x00
# define TSL2561_REG_TIMING    0x01
# define TSL2561_REG_DATA_0    0x0C
# define TSL2561_REG_DATA_1    0x0E


P015_data_struct::P015_data_struct(uint8_t i2caddr, unsigned int gain, uint8_t integration) :
  _gain(gain),
  _i2cAddr(i2caddr),
  _integration(integration)
{
  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)

  _gain16xActive = gain == 1;

  if (!useAutoGain()) {
    _gain16xActive = gain == 1;
  }
}

bool P015_data_struct::performRead(float& luxVal,
                                   float& infraredVal,
                                   float& broadbandVal,
                                   float& ir_broadband_ratio)
{
  bool success = false;
  int  attempt = useAutoGain() ? 2 : 1;

  while (!success && attempt > 0) {
    --attempt;

    float ms; // Integration ("shutter") time in milliseconds

    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    unsigned char time = _integration;

    plugin_015_setTiming(_gain16xActive, time, ms);
    setPowerUp();
    delayBackground(ms); // FIXME TD-er: Do not use delayBackground but collect data later.
    unsigned int data0, data1;

    if (getData(data0, data1))
    {
      float lux;       // Resulting lux value
      float infrared;  // Resulting infrared value
      float broadband; // Resulting broadband value


      // Perform lux calculation:
      success = !ADC_saturated(time,  data0) && !ADC_saturated(time, data1);
      getLux(_gain16xActive, ms, data0, data1, lux, infrared, broadband);

      if (useAutoGain()) {
        if (_gain16xActive) {
          // Last reading was using 16x gain
          // Check using some margin to see if gain is still needed
          if (ADC_saturated(time,  data0 * 16)) {
            _gain16xActive = false;
          }
        } else {
          // Check using some margin to see if gain will improve reading resolution
          if (lux < 40) {
            _gain16xActive = true;
          }
        }
      }

      if (success) {
        if (broadband > 0.0f) {
          // Store the ratio in an unused user var. (should we make it available?)
          // Only store/update it when not close to the limits of both ADC ranges.
          // When using this value to compute extended ranges, it must not be using a ratio taken from a
          // heated sensor, since then the IR part may be off quite a bit resulting in very unrealistic values.
          if (!ADC_saturated(time,  data0 * 2) && !ADC_saturated(time, data1 * 2)) {
            ir_broadband_ratio = infrared / broadband;
          }
        }
      } else {
        // Use last known ratio to reconstruct the broadband value
        // If IR is saturated, output the max value based on the last known ratio.
        if ((ir_broadband_ratio > 0.0f) && (_gain == P015_EXT_AUTO_GAIN)) {
          data0 = static_cast<float>(data1) / ir_broadband_ratio;
          getLux(_gain16xActive, ms, data0, data1, lux, infrared, broadband);
          success = true;
        }
      }
      luxVal       = lux;
      infraredVal  = infrared;
      broadbandVal = broadband;
    }
    else
    {
      // getData() returned false because of an I2C error, inform the user.
      addLog(LOG_LEVEL_ERROR, F("TSL2561: i2c error"));
      success = false;
      attempt = 0;
    }
  }
  return success;
}

bool P015_data_struct::useAutoGain() const
{
  const bool autoGain = _gain == P015_AUTO_GAIN || _gain == P015_EXT_AUTO_GAIN;

  return autoGain;
}

bool P015_data_struct::begin()
{
  // Wire.begin();   called in ESPEasy framework
  return true;
}

bool P015_data_struct::readByte(unsigned char address, unsigned char& value)

// Reads a byte from a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value will be set to stored byte
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Set up command byte for read
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(_i2cAddr, (uint8_t)1);

    if (Wire.available() == 1)
    {
      value = Wire.read();
      return true;
    }
  }
  return false;
}

bool P015_data_struct::writeByte(unsigned char address, unsigned char value)

// Write a byte to a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value: byte to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for write
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);

  // Write byte
  Wire.write(value);
  _error = Wire.endTransmission();

  if (_error == 0) {
    return true;
  }

  return false;
}

bool P015_data_struct::readUInt(unsigned char address, unsigned int& value)

// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value will be set to stored unsigned integer
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for read
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read two bytes (low and high)
  if (_error == 0)
  {
    Wire.requestFrom(_i2cAddr, (uint8_t)2);

    if (Wire.available() == 2)
    {
      char high, low;
      low  = Wire.read();
      high = Wire.read();

      // Combine bytes into unsigned int
      value = word(high, low);
      return true;
    }
  }
  return false;
}

bool P015_data_struct::writeUInt(unsigned char address, unsigned int value)

// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value: unsigned int to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Split int into lower and upper bytes, write each byte
  if (writeByte(address, lowByte(value))
      && writeByte(address + 1, highByte(value))) {
    return true;
  }

  return false;
}

bool P015_data_struct::plugin_015_setTiming(bool gain, unsigned char time)

// If gain = false (0), device is set to low gain (1X)
// If gain = high (1), device is set to high gain (16X)
// If time = 0, integration will be 13.7ms
// If time = 1, integration will be 101ms
// If time = 2, integration will be 402ms
// If time = 3, use manual start / stop
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  unsigned char timing;

  // Get timing byte
  if (readByte(TSL2561_REG_TIMING, timing))
  {
    // Set gain (0 or 1)
    if (gain) {
      timing |= 0x10;
    }
    else {
      timing &= ~0x10;
    }

    // Set integration time (0 to 3)
    timing &= ~0x03;
    timing |= (time & 0x03);

    // Write modified timing byte back to device
    if (writeByte(TSL2561_REG_TIMING, timing)) {
      return true;
    }
  }
  return false;
}

bool P015_data_struct::plugin_015_setTiming(bool gain, unsigned char time, float& ms)

// If gain = false (0), device is set to low gain (1X)
// If gain = high (1), device is set to high gain (16X)
// If time = 0, integration will be 13.7ms
// If time = 1, integration will be 101ms
// If time = 2, integration will be 402ms
// If time = 3, use manual start / stop (ms = 0)
// ms will be set to integration time
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Calculate ms for user
  switch (time)
  {
    case 0: ms  = 13.7f; break;
    case 1: ms  = 101; break;
    case 2: ms  = 402; break;
    default: ms = 402; // used in a division, so do not use 0
  }

  // Set integration using base function
  return plugin_015_setTiming(gain, time);
}

// Determine if either sensor saturated (max depends on clock freq. and integration time)
// If so, abandon ship (calculation will not be accurate)
bool P015_data_struct::ADC_saturated(unsigned char time, unsigned int value) {
  unsigned int max_ADC_count = 65535;

  switch (time)
  {
    case 0: max_ADC_count = 5047; break;
    case 1: max_ADC_count = 37177; break;
    case 2:
    default: break;
  }
  return value >= max_ADC_count;
}

bool P015_data_struct::setPowerUp(void)

// Turn on TSL2561, begin integrations
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Write 0x03 to command byte (power on)
  return writeByte(TSL2561_REG_CONTROL, 0x03);
}

bool P015_data_struct::setPowerDown(void)

// Turn off TSL2561
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Clear command byte (power off)
  return writeByte(TSL2561_REG_CONTROL, 0x00);
}

bool P015_data_struct::getData(unsigned int& data0, unsigned int& data1)

// Retrieve raw integration results
// data0 and data1 will be set to integration results
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Get data0 and data1 out of result registers
  if (readUInt(TSL2561_REG_DATA_0, data0) && readUInt(TSL2561_REG_DATA_1, data1)) {
    return true;
  }

  return false;
}

void P015_data_struct::getLux(unsigned char gain,
                              float         ms,
                              unsigned int  CH0,
                              unsigned int  CH1,
                              float       & lux,
                              float       & infrared,
                              float       & broadband)

// Convert raw data to lux
// gain: 0 (1X) or 1 (16X), see setTiming()
// ms: integration time in ms, from setTiming() or from manual integration
// CH0, CH1: results from getData()
// lux will be set to resulting lux calculation
// returns true (1) if calculation was successful
// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
  float ratio, d0, d1;

  // Convert from unsigned integer to floating point
  d0 = CH0; d1 = CH1;

  // We will need the ratio for subsequent calculations
  ratio = d1 / d0;

  // save original values
  infrared  = d1;
  broadband = d0;

  // Normalize for integration time
  d0 *= (402.0f / ms);
  d1 *= (402.0f / ms);

  // Normalize for gain
  if (!gain)
  {
    d0 *= 16;
    d1 *= 16;
  }

  // Determine lux per datasheet equations:
  if (ratio < 0.5f)
  {
    lux = 0.0304f * d0 - 0.062f * d0 * powf(ratio, 1.4f);
  } else if (ratio < 0.61f)
  {
    lux = 0.0224f * d0 - 0.031f * d1;
  } else if (ratio < 0.80f)
  {
    lux = 0.0128f * d0 - 0.0153f * d1;
  } else if (ratio < 1.30f)
  {
    lux = 0.00146f * d0 - 0.00112f * d1;
  } else {
    // ratio >= 1.30
    lux = 0.0f;
  }
}

#endif // ifdef USES_P015

#include "../PluginStructs/P045_data_struct.h"

#ifdef USES_P045

# define MPU6050_RA_GYRO_CONFIG              0x1B
# define MPU6050_RA_ACCEL_CONFIG             0x1C
# define MPU6050_RA_ACCEL_XOUT_H             0x3B
# define MPU6050_RA_PWR_MGMT_1               0x6B
# define MPU6050_ACONFIG_AFS_SEL_BIT         4
# define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
# define MPU6050_GCONFIG_FS_SEL_BIT          4
# define MPU6050_GCONFIG_FS_SEL_LENGTH       2
# define MPU6050_CLOCK_PLL_XGYRO             0x01
# define MPU6050_GYRO_FS_250                 0x00
# define MPU6050_ACCEL_FS_2                  0x00
# define MPU6050_PWR1_SLEEP_BIT              6
# define MPU6050_PWR1_CLKSEL_BIT             2
# define MPU6050_PWR1_CLKSEL_LENGTH          3

P045_data_struct::P045_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr)
{
  // Initialize the MPU6050, for details look at the MPU6050 library: MPU6050::Initialize
  writeBits(MPU6050_RA_PWR_MGMT_1,   MPU6050_PWR1_CLKSEL_BIT,     MPU6050_PWR1_CLKSEL_LENGTH,     MPU6050_CLOCK_PLL_XGYRO);
  writeBits(MPU6050_RA_GYRO_CONFIG,  MPU6050_GCONFIG_FS_SEL_BIT,  MPU6050_GCONFIG_FS_SEL_LENGTH,  MPU6050_GYRO_FS_250);
  writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
  writeBits(MPU6050_RA_PWR_MGMT_1,   MPU6050_PWR1_SLEEP_BIT,      1,                              0);

  // Read the MPU6050 once to clear out zeros (1st time reading MPU6050 returns all 0s)
  int16_t ax, ay, az, gx, gy, gz;

  getRaw6AxisMotion(&ax, &ay, &az, &gx, &gy, &gz);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 5; ++j) {
      _axis[i][j] = 0;
    }
  }
}

void P045_data_struct::loop()
{
  // Read the sensorvalues, we run this bit every 1/10th of a second
  getRaw6AxisMotion(&_axis[0][3],
                    &_axis[1][3],
                    &_axis[2][3],
                    &_axis[0][4],
                    &_axis[1][4],
                    &_axis[2][4]);

  // Set the minimum and maximum value for each axis a-value, overwrite previous values if smaller/larger
  trackMinMax(_axis[0][3], &_axis[0][0], &_axis[0][1]);
  trackMinMax(_axis[1][3], &_axis[1][0], &_axis[1][1]);
  trackMinMax(_axis[2][3], &_axis[2][0], &_axis[2][1]);

  //          ^ current value @ 3   ^ min val @ 0         ^ max val @ 1

  /*
     // Uncomment this block if you want to debug your MPU6050, but be prepared for a log overload
     String log = F("MPU6050 : axis values: ");

     log += _axis[0][3];
     log += F(", ");
     log += _axis[1][3];
     log += F(", ");
     log += _axis[2][3];
     log += F(", g values: ");
     log += _axis[0][4];
     log += F(", ");
     log += _axis[1][4];
     log += F(", ");
     log += _axis[2][4];
     addLog(LOG_LEVEL_INFO, log);
   */

  // Run this bit every 5 seconds per deviceaddress (not per instance)
  if (timeOutReached(_timer + 5000))
  {
    _timer = millis();

    // Determine the maximum measured range of each axis
    for (uint8_t i = 0; i < 3; i++) {
      _axis[i][2] = abs(_axis[i][1] - _axis[i][0]);
      _axis[i][0] = _axis[i][3];
      _axis[i][1] = _axis[i][3];
    }
  }
}

void P045_data_struct::trackMinMax(int16_t current, int16_t *min, int16_t *max)
{
  // From nodemcu-laundry.ino by Nolan Gilley
  if (current > *max)
  {
    *max = current;
  }
  else if (current < *min)
  {
    *min = current;
  }
}

void P045_data_struct::getRaw6AxisMotion(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {
  // From I2Cdev::readBytes and MPU6050::getMotion6, both by Jeff Rowberg
  uint8_t buffer[14];
  uint8_t count = 0;

  I2C_write8(i2cAddress, MPU6050_RA_ACCEL_XOUT_H);
  Wire.requestFrom(i2cAddress, (uint8_t)14);

  for (; Wire.available(); count++) {
    buffer[count] = Wire.read();
  }
  *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
  *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
  *az = (((int16_t)buffer[4]) << 8) | buffer[5];
  *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
  *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
  *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

void P045_data_struct::writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
  // From I2Cdev::writeBits by Jeff Rowberg
  //      010 value to write
  // 76543210 bit numbers
  //    xxx   args: bitStart=4, length=3
  // 00011100 mask byte
  // 10101111 original value (sample)
  // 10100011 original & ~mask
  // 10101011 masked | value
  bool is_ok = true;
  uint8_t b  = I2C_read8_reg(i2cAddress, regAddr, &is_ok);

  if (is_ok) {
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data  &= mask;                    // zero all non-important bits in data
    b     &= ~(mask);                 // zero all important bits in existing byte
    b     |= data;                    // combine data with existing byte
    I2C_write8_reg(i2cAddress, regAddr, b);
  }
}

#endif // ifdef USES_P045

#include "../PluginStructs/P096_data_struct.h"

#ifdef USES_P096


P096_data_struct::P096_data_struct(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY)
 : eInkScreen(width, height, DC, RST, CS, BUSY),
   plugin_096_sequence_in_progress(false)
{
  eInkScreen.begin();
  eInkScreen.clearBuffer();
}


//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default ILI9341_WHITE)
//param [in] bkcolor : The background color (default ILI9341_BLACK)
void P096_data_struct::printText(const char *string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  eInkScreen.clearBuffer();
  eInkScreen.clearDisplay();
  eInkScreen.setCursor(X, Y);
  eInkScreen.setTextColor(color, bkcolor);
  eInkScreen.setTextSize(textSize);
  String fixString = string;
  FixText(fixString);
  eInkScreen.println(fixString);
  eInkScreen.display();
}

//Parse color string to color
//param [in] colorString : The color string (white, red, ...)
//return : color (default EPD_WHITE)
unsigned short P096_data_struct::ParseColor(const String & colorString)
{
  //copy to local var and ensure lowercase
  //this optimise the next equlaity checks
  String s = colorString;
  s.toLowerCase();

  if (s.equals(F("black")))
    return EPD_BLACK;
  if (s.equals(F("white")))
    return EPD_WHITE;
  if (s.equals(F("inverse")))
    return EPD_INVERSE;
  if (s.equals(F("red")))
    return EPD_RED;
  if (s.equals(F("dark")))
    return EPD_DARK;
  if (s.equals(F("light")))
    return EPD_LIGHT;
  return EPD_WHITE;
}

//Fix text with handling special characters (degrees and main monetary symbols)
//This is specific case for current AdafruitGfx standard fontused for eink screen
//param [in/out] s : The string to fix
void P096_data_struct::FixText(String & s)
{
  const char degree[3] = {0xc2, 0xb0, 0};  // Unicode degree symbol
  const char degree_eink[2] = {0xf7, 0};  // eink degree symbol
  s.replace(degree, degree_eink);
  s.replace(F("{D}"), degree_eink);
  s.replace(F("&deg;"), degree_eink);
  
  const char euro[4]  = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_eink[2] = {0xED, 0};  // eink degree symbol
  s.replace(euro, euro_eink);
  s.replace(F("{E}"), euro_eink);
  s.replace(F("&euro;"), euro_eink);

  const char pound[3] = { 0xc2, 0xa3, 0 };       // Unicode pound symbol
  const char pound_eink[2] = {0x9C, 0};  // eink pound symbol
  s.replace(pound, pound_eink);
  s.replace(F("{P}"), pound_eink);
  s.replace(F("&pound;"), pound_eink);

  const char yen[3]   = { 0xc2, 0xa5, 0 };       // Unicode yen symbol
  const char yen_eink[2] = {0x9D, 0};  // eink yen symbol
  s.replace(yen, yen_eink);
  s.replace(F("{Y}"), yen_eink);
  s.replace(F("&yen;"), yen_eink);

  const char cent[3]   = { 0xc2, 0xa2, 0 };       // Unicode yen symbol
  const char cent_eink[2] = {0x9B, 0};  // eink cent symbol
  s.replace(cent, cent_eink);
  s.replace(F("{c}"), cent_eink);
  s.replace(F("&cent;"), cent_eink);

}

//Split a string by delimiter
//param [in] s : The input string
//param [in] c : The delimiter
//param [out] op : The resulting string array
//param [in] limit : The maximum strings to find
//return : The string count
int P096_data_struct::StringSplit(const String &s, char c, String op[], int limit)
{
  int count = 0;
  char * pch;
  String d = String(c);
  pch = strtok ((char*)(s.c_str()),d.c_str());
  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok (NULL, ",");
  }  
  return count;
}


#endif // ifdef USES_P096

#include "../PluginStructs/P004_data_struct.h"

#ifdef USES_P004


P004_data_struct::P004_data_struct(int8_t pin_rx, int8_t pin_tx, const uint8_t addr[], uint8_t res) : _gpio_rx(pin_rx), _gpio_tx(pin_tx), _res(res)
{
  if ((_res < 9) || (_res > 12)) { _res = 12; }

  add_addr(addr, 0);
  set_measurement_inactive();
}

void P004_data_struct::add_addr(const uint8_t addr[], uint8_t index) {
  if (index < 4) {
    _sensors[index].addr = Dallas_addr_to_uint64(addr);

    // If the address already exists, set it to 0 to avoid duplicates
    for (uint8_t i = 0; i < 4; ++i) {
      if (index != i) {
        if (_sensors[index].addr == _sensors[i].addr) {
          _sensors[index].addr = 0;
        }
      }
    }
    _sensors[index].check_sensor(_gpio_rx, _gpio_tx, _res);
  }
}

bool P004_data_struct::initiate_read() {
  _measurementStart = millis();

  for (uint8_t i = 0; i < 4; ++i) {
    if (_sensors[i].initiate_read(_gpio_rx, _gpio_tx, _res)) {
      if (!measurement_active()) {
        // Set the timer right after initiating the first sensor


        /*********************************************************************************************\
        *  Dallas Start Temperature Conversion, expected max duration:
        *    9 bits resolution ->  93.75 ms
        *   10 bits resolution -> 187.5 ms
        *   11 bits resolution -> 375 ms
        *   12 bits resolution -> 750 ms
        \*********************************************************************************************/
        uint8_t res = _res;

        if ((res < 9) || (res > 12)) { res = 12; }
        _timer = millis() + (800 / (1 << (12 - res)));
      }
      _sensors[i].measurementActive = true;
    }
  }

  return measurement_active();
}

bool P004_data_struct::collect_values() {
  bool success = false;

  for (uint8_t i = 0; i < 4; ++i) {
    if (_sensors[i].collect_value(_gpio_rx, _gpio_tx)) {
      success = true;
    }
  }
  return success;
}

bool P004_data_struct::read_temp(float& value, uint8_t index) const {
    if (index >= 4) return false;
  if ((_sensors[index].addr == 0) || !_sensors[index].valueRead) { return false; }

  value = _sensors[index].value;
  return true;
}

String P004_data_struct::get_formatted_address(uint8_t index) const {
    if (index < 4) return _sensors[index].get_formatted_address();
    return "";
}

bool P004_data_struct::measurement_active() const {
  for (uint8_t i = 0; i < 4; ++i) {
    if (_sensors[i].measurementActive) { return true; }
  }

  return false;
}

bool P004_data_struct::measurement_active(uint8_t index) const {
  if (index < 4) {
    return _sensors[index].measurementActive;
  }
  return false;
}

void P004_data_struct::set_measurement_inactive() {
  for (uint8_t i = 0; i < 4; ++i) {
    _sensors[i].set_measurement_inactive();
  }
}

Dallas_SensorData P004_data_struct::get_sensor_data(uint8_t index) const {
    if (index < 4) return _sensors[index];
    return Dallas_SensorData();
}


#endif // ifdef USES_P004

#include "../PluginStructs/P113_data_struct.h"

#ifdef USES_P113

P113_data_struct::P113_data_struct(uint8_t i2c_addr, int timing, bool range) : i2cAddress(i2c_addr), timing(timing), range(range) {}

// **************************************************************************/
// Initialize VL53L1X
// **************************************************************************/
bool P113_data_struct::begin() {
  initState = true;

  sensor.setI2CAddress(i2cAddress); // Initialize for configured address

  if (sensor.begin() != 0) {
    String log = F("VL53L1X: Sensor not found, init failed for 0x");
    log += String(i2cAddress, HEX);
    addLog(LOG_LEVEL_INFO, log);
    initState = false;
    return initState;
  }

  sensor.setTimingBudgetInMs(timing);

  if (range) {
    sensor.setDistanceModeLong();
  } else {
    sensor.setDistanceModeShort();
  }

  return initState;
}

bool P113_data_struct::startRead() {
  if (initState && !readActive) {
    sensor.startRanging();
    readActive = true;
    distance   = -1;
  }
  return readActive;
}

bool P113_data_struct::readAvailable() {
  bool ready = sensor.checkForDataReady();

  if (ready) {
    distance = sensor.getDistance();
    sensor.clearInterrupt();
    sensor.stopRanging();

    // readActive = false;
  }
  return ready;
}

uint16_t P113_data_struct::readDistance() {
  success = false;

  # if defined(P113_DEBUG) || defined(P113_DEBUG_DEBUG)
  String log;
  # endif // if defined(P113_DEBUG) || defined(P113_DEBUG_DEBUG)
  # ifdef P113_DEBUG_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    log  = F("VL53L1X  : idx: 0x");
    log += String(i2cAddress, HEX);
    log += F(" init: ");
    log += String(initState, BIN);
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // P113_DEBUG_DEBUG

  success    = true;
  readActive = false;

  if (distance >= 8190) {
    # ifdef P113_DEBUG_DEBUG
    addLog(LOG_LEVEL_DEBUG, "VL53L1X: NO MEASUREMENT");
    # endif // P113_DEBUG_DEBUG
    success = false;
  }

  # ifdef P113_DEBUG
  log  = F("VL53L1X: Address: 0x");
  log += String(i2cAddress, HEX);
  log += F(" / Timing: ");
  log += String(timing, DEC);
  log += F(" / Long Range: ");
  log += String(range, BIN);
  log += F(" / Distance: ");
  log += distance;
  addLog(LOG_LEVEL_INFO, log);
  # endif // P113_DEBUG

  return distance;
}

uint16_t P113_data_struct::readAmbient() {
  return sensor.getAmbientRate();
}

bool P113_data_struct::isReadSuccessful() {
  return success;
}

#endif // ifdef USES_P113

#include "../PluginStructs/P057_data_struct.h"


// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <HT16K33.h>

#ifdef USES_P057

P057_data_struct::P057_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {
  ledMatrix.Init(i2cAddress);
}

#endif // ifdef USES_P057

#include "../PluginStructs/P112_data_struct.h"

#ifdef USES_P112

// #######################################################################################################
// #################### Plugin 112 I2C AS7265X Triad Spectroscopy Sensor and White, IR and UV LED ########
// #######################################################################################################
//
// Triad Spectroscopy Sensor and White, IR and UV LED
// like this one: https://www.sparkfun.com/products/15050
// based on this library: https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library
// this code is based on 29 Mar 2019-03-29 version of the above library
//
// 2021-03-29 heinemannj: Initial commit
//

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter
#include <SparkFun_AS7265X.h>

bool P112_data_struct::begin()
{
  if (!initialized) {
    initialized = sensor.begin();

    if (initialized) {
//      sensor.takeMeasurementsWithBulb();
    }
  }
  return initialized;
}

#endif // ifdef USES_P112

#include "../PluginStructs/P053_data_struct.h"

#ifdef USES_P053

const __FlashStringHelper* toString(PMSx003_type sensorType) {
  switch (sensorType) {
    case PMSx003_type::PMS1003_5003_7003: return F("PMS1003 / PMS5003 / PMS7003");
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_type::PMS2003_3003: return F("PMS2003 / PMS3003");
    case PMSx003_type::PMS5003_S:    return F("PMS5003S");
    case PMSx003_type::PMS5003_T:    return F("PMS5003T");
    case PMSx003_type::PMS5003_ST:   return F("PMS5003ST");
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_output_selection selection) {
  switch (selection) {
    case PMSx003_output_selection::Particles_ug_m3: return F("Particles &micro;g/m3: pm1.0, pm2.5, pm10");
    case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:  return F("Particles &micro;g/m3: pm2.5; Other: Temp, Humi, HCHO (PMS5003ST)");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
      return F("Particles count/0.1L: cnt1.0, cnt2.5, cnt5, cnt10 (PMS1003/5003(ST)/7003)");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
      return F("Particles count/0.1L: cnt0.3, cnt0.5, cnt1.0, cnt2.5 (PMS1003/5003(ST)/7003)");
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_event_datatype selection) {
  switch (selection) {
    case PMSx003_event_datatype::Event_None:       return F("None");
    case PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde:  return F("Particles &micro;g/m3 and Temp/Humi/HCHO");
    case PMSx003_event_datatype::Event_All:  return F("Particles &micro;g/m3, Temp/Humi/HCHO and Particles count/0.1L");
    case PMSx003_event_datatype::Event_All_count_bins: return F("Particles count/0.1L");
  }
  return F("Unknown");
}

P053_data_struct::P053_data_struct(
  taskIndex_t             TaskIndex,
  int8_t                  rxPin,
  int8_t                  txPin,
  const ESPEasySerialPort port,
  int8_t                  resetPin,
  int8_t                  pwrPin,
  PMSx003_type            sensortype,
  uint32_t                delay_read_after_wakeup_ms
  # ifdef                 PLUGIN_053_ENABLE_EXTRA_SENSORS
  , bool                  oversample
  , bool                  splitCntBins
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  )
  : _taskIndex(TaskIndex),
  _sensortype(sensortype),
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _oversample(oversample),
  _splitCntBins(splitCntBins),
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _delay_read_after_wakeup_ms(delay_read_after_wakeup_ms),
  _resetPin(resetPin), _pwrPin(pwrPin)
{
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(25);
    log  = F("PMSx003 : config ");
    log += rxPin;
    log += ' ';
    log += txPin;
    log += ' ';
    log += _resetPin;
    log += ' ';
    log += _pwrPin;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (port == ESPEasySerialPort::software) {
    addLog(LOG_LEVEL_INFO, F("PMSx003 : using software serial"));
  } else {
    addLog(LOG_LEVEL_INFO, F("PMSx003 : using hardware serial"));
  }
  _easySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.

  if (_easySerial != nullptr) {
    _easySerial->begin(9600);
    _easySerial->flush();

    wakeSensor();

    resetSensor();

    // Make sure to set the mode to active reading mode.
    // This is the default, but not sure if passive mode can be set persistant.
    setActiveReadingMode();
  }
  clearReceivedData();
}

P053_data_struct::~P053_data_struct() {
  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }
}

bool P053_data_struct::initialized() const
{
  return _easySerial != nullptr;
}

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void P053_data_struct::SerialRead16(uint16_t& value, uint16_t *checksum)
{
  if (!initialized()) { return; }
  const uint8_t data_high = _easySerial->read();
  const uint8_t data_low  = _easySerial->read();

  value  = data_low;
  value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

  # ifdef P053_LOW_LEVEL_DEBUG

  // Low-level logging to see data from sensor
  String log = F("PMSx003 : uint8_t high=0x");
  log += String(data_high, HEX);
  log += F(" uint8_t low=0x");
  log += String(data_low, HEX);
  log += F(" result=0x");
  log += String(value, HEX);
  addLog(LOG_LEVEL_INFO, log);
  # endif // ifdef P053_LOW_LEVEL_DEBUG
}

void P053_data_struct::SerialFlush() {
  if (_easySerial != nullptr) {
    _easySerial->flush();
  }
}

uint8_t P053_data_struct::packetSize() const {
  switch (_sensortype) {
    case PMSx003_type::PMS1003_5003_7003:    return PMS1003_5003_7003_SIZE;
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_type::PMS5003_S:    return PMS5003_S_SIZE;
    case PMSx003_type::PMS5003_T:    return PMS5003_T_SIZE;
    case PMSx003_type::PMS5003_ST:   return PMS5003_ST_SIZE;
    case PMSx003_type::PMS2003_3003: return PMS2003_3003_SIZE;
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  }
  return 0u;
}

bool P053_data_struct::packetAvailable()
{
  if (_easySerial != nullptr)
  {
    // When there is enough data in the buffer, search through the buffer to
    // find header (buffer may be out of sync)
    if (!_easySerial->available()) { return false; }

    while ((_easySerial->peek() != PMSx003_SIG1) && _easySerial->available()) {
      _easySerial->read(); // Read until the buffer starts with the
      // first uint8_t of a message, or buffer
      // empty.
    }

    if (_easySerial->available() < packetSize()) {
      return false; // Not enough yet for a complete packet
    }
  }
  return true;
}

# ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
void P053_data_struct::sendEvent(const String& baseEvent,
                                 uint8_t       index) {
  float value = 0.0f;

  if (!getValue(index, value)) { return; }


  String valueEvent;

  valueEvent.reserve(32);
  const unsigned char nrDecimals = getNrDecimals(index, _oversample);

  // Temperature
  valueEvent  = baseEvent;
  valueEvent += getEventString(index);
  valueEvent += '=';
  valueEvent += toString(value, nrDecimals);
  eventQueue.addMove(std::move(valueEvent));
}

bool P053_data_struct::hasFormaldehyde() const {
  #  ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_S ||
         _sensortype == PMSx003_type::PMS5003_ST;
  #  else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  #  endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

bool P053_data_struct::hasTempHum() const {
  #  ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_T ||
         _sensortype == PMSx003_type::PMS5003_ST;
  #  else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  #  endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

# endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

bool P053_data_struct::processData(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength   = 0;
  uint16_t packet_header = 0;

  SerialRead16(packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2

  if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
    // Not the start of the packet, stop reading.
    return false;
  }

  SerialRead16(framelength, &checksum);

  if ((framelength + 4) != packetSize()) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(34);
      log  = F("PMSx003 : invalid framelength - ");
      log += framelength;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  uint8_t frameData = packetSize();

  if (frameData > 0u) {
    frameData /= 2;                               // Each value is 16 bits
    frameData -= 3;                               // start markers, length, checksum
  }
  uint16_t data[PMS_RECEIVE_BUFFER_SIZE] = { 0 }; // uint8_t data_low, data_high;

  for (uint8_t i = 0; i < frameData; i++) {
    SerialRead16(data[i], &checksum);
  }

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_T) {
    data[PMS_Temp_C]  = data[PMS_T_Temp_C]; // Move data to the 'usual' location for Temp/Hum
    data[PMS_Hum_pct] = data[PMS_T_Hum_pct];
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  # ifndef BUILD_NO_DEBUG
  #  ifdef P053_LOW_LEVEL_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Available on all supported sensor models
    String log;
    log.reserve(87);
    log  = F("PMSx003 : pm1.0=");
    log += data[PMS_PM1_0_ug_m3_factory];
    log += F(", pm2.5=");
    log += data[PMS_PM2_5_ug_m3_factory];
    log += F(", pm10=");
    log += data[PMS_PM10_0_ug_m3_factory];
    log += F(", pm1.0a=");
    log += data[PMS_PM1_0_ug_m3_normal];
    log += F(", pm2.5a=");
    log += data[PMS_PM2_5_ug_m3_normal];
    log += F(", pm10a=");
    log += data[PMS_PM10_0_ug_m3_normal];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  #   ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) { // 'Count' values not available on
    // PMS2003/PMS3003 models
    // (handled as 1 model in code)
    String log;
    log.reserve(96);
    log  = F("PMSx003 : count/0.1L : 0.3um=");
    log += data[PMS_cnt0_3_100ml];
    log += F(", 0.5um=");
    log += data[PMS_cnt0_5_100ml];
    log += F(", 1.0um=");
    log += data[PMS_cnt1_0_100ml];
    log += F(", 2.5um=");
    log += data[PMS_cnt2_5_100ml];
    log += F(", 5.0um=");
    log += data[PMS_cnt5_0_100ml];
    log += F(", 10um=");
    log += data[PMS_cnt10_0_100ml];
    addLog(LOG_LEVEL_DEBUG, log);
  }


  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && ((GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_ST)
          || (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_T))) { // Values only available on PMS5003ST & PMS5003T
    String log;
    log.reserve(45);
    log  = F("PMSx003 : temp=");
    log += static_cast<float>(data[PMS_Temp_C]) / 10.0f;
    log += F(", humi=");
    log += static_cast<float>(data[PMS_Hum_pct]) / 10.0f;

    if (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_ST) {
      log += F(", hcho=");
      log += static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f;
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #   endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  #  endif     // ifdef P053_LOW_LEVEL_DEBUG
  # endif      // ifndef BUILD_NO_DEBUG

  // Compare checksums
  SerialRead16(checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.

  if (checksum != checksum2) {
    addLog(LOG_LEVEL_ERROR, F("PMSx003 : Checksum error"));
    return false;
  }

  if (_last_wakeup_moment.isSet() && !_last_wakeup_moment.timeReached()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("PMSx003 : Less than ");
      log += _delay_read_after_wakeup_ms / 1000ul;
      log += F(" sec since sensor wakeup => Ignoring sample");
      addLog(LOG_LEVEL_INFO, log);
    }
    return false;
  }

  if (checksum == _last_checksum) {
    // Duplicate message
      # ifndef BUILD_NO_DEBUG

    addLog(LOG_LEVEL_DEBUG, F("PMSx003 : Duplicate message"));
      # endif // ifndef BUILD_NO_DEBUG
    return false;
  }
  # ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Data is checked and good, fill in output
  UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_ug_m3_normal];
  UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_ug_m3_normal];
  UserVar[event->BaseVarIndex + 2] = data[PMS_PM10_0_ug_m3_normal];
  _values_received                 = 1;
  # else // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Store in the averaging buffer to process later
  if (!_oversample) {
    clearReceivedData();
  }

  for (uint8_t i = 0; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
    _data[i] += data[i];
  }
  ++_values_received;
  # endif // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS


  // Store new checksum, to help detect duplicates.
  _last_checksum = checksum;

  return true;
}

bool P053_data_struct::checkAndClearValuesReceived(struct EventStruct *event) {
  if (_values_received == 0) { return false; }

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Data is checked and good, fill in output

  _value_mask = 0;

  switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
    case PMSx003_output_selection::Particles_ug_m3:
    {
      UserVar[event->BaseVarIndex]     = getValue(PMS_PM1_0_ug_m3_normal);
      UserVar[event->BaseVarIndex + 1] = getValue(PMS_PM2_5_ug_m3_normal);
      UserVar[event->BaseVarIndex + 2] = getValue(PMS_PM10_0_ug_m3_normal);
      UserVar[event->BaseVarIndex + 3] = 0.0f;
      break;
    }
    case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
    {
      UserVar[event->BaseVarIndex]     = getValue(PMS_PM2_5_ug_m3_normal);
      UserVar[event->BaseVarIndex + 1] = getValue(PMS_Temp_C);
      UserVar[event->BaseVarIndex + 2] = getValue(PMS_Hum_pct);
      UserVar[event->BaseVarIndex + 3] = getValue(PMS_Formaldehyde_mg_m3);
      break;
    }
    case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
    {
      UserVar[event->BaseVarIndex]     = getValue(PMS_cnt0_3_100ml);
      UserVar[event->BaseVarIndex + 1] = getValue(PMS_cnt0_5_100ml);
      UserVar[event->BaseVarIndex + 2] = getValue(PMS_cnt1_0_100ml);
      UserVar[event->BaseVarIndex + 3] = getValue(PMS_cnt2_5_100ml);
      break;
    }
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
    {
      UserVar[event->BaseVarIndex]     = getValue(PMS_cnt1_0_100ml);
      UserVar[event->BaseVarIndex + 1] = getValue(PMS_cnt2_5_100ml);
      UserVar[event->BaseVarIndex + 2] = getValue(PMS_cnt5_0_100ml);
      UserVar[event->BaseVarIndex + 3] = getValue(PMS_cnt10_0_100ml);
      break;
    }
  }

  if (Settings.UseRules
      && (GET_PLUGIN_053_EVENT_OUT_SELECTOR != PMSx003_event_datatype::Event_None)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) {
    // Events not applicable to PMS2003 & PMS3003 models
    String baseEvent;
    baseEvent.reserve(21);
    baseEvent  = getTaskDeviceName(event->TaskIndex);
    baseEvent += '#';


    // Send out events for those values not present in the task output
    switch (GET_PLUGIN_053_EVENT_OUT_SELECTOR) {
      case PMSx003_event_datatype::Event_None: break;
      case PMSx003_event_datatype::Event_All:
      {
        // Send all remaining
        for (uint8_t i = PMS_PM1_0_ug_m3_normal; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
          sendEvent(baseEvent, i);
        }
        break;
      }
      case PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde:
      {
        const uint8_t indices[] =
        {
          PMS_PM1_0_ug_m3_normal,
          PMS_PM2_5_ug_m3_normal,
          PMS_PM10_0_ug_m3_normal,
          PMS_Temp_C,
          PMS_Hum_pct,
          PMS_Formaldehyde_mg_m3
        };

        for (uint8_t i = 0; i < 6; ++i) {
          sendEvent(baseEvent, indices[i]);
        }
        break;
      }
      case PMSx003_event_datatype::Event_All_count_bins:
      {
        // Thexe values are sequential, so just use a simple for loop.
        for (uint8_t i = PMS_cnt0_3_100ml; i <= PMS_cnt10_0_100ml; ++i) {
          sendEvent(baseEvent, i);
        }
        break;
      }
    }
  }

  if (_oversample) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("PMSx003: Oversampling using ");
      log += _values_received;
      log += F(" samples");
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  clearReceivedData();
  return true;
}

bool P053_data_struct::resetSensor() {
  if (_resetPin >= 0) { // Reset if pin is configured
    // Toggle 'reset' to assure we start reading header
    addLog(LOG_LEVEL_INFO, F("PMSx003: resetting module"));
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
    delay(250);
    digitalWrite(_resetPin, HIGH);
    pinMode(_resetPin, INPUT_PULLUP);
    return true;
  }
  return false;
}

bool P053_data_struct::wakeSensor() {
  if (!initialized()) {
    return false;
  }
  addLog(LOG_LEVEL_INFO, F("PMSx003: Wake sensor"));

  if (_pwrPin >= 0) {
    // Make sure the sensor is "on"
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, HIGH);
    pinMode(_pwrPin, INPUT_PULLUP);
  } else {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74
    };
    _easySerial->write(command, 7);
  }

  // No idea how old this data is, so clear it.
  // Otherwise oversampling may give weird results
  clearReceivedData();

  if (_delay_read_after_wakeup_ms != 0) {
    _last_wakeup_moment.setMillisFromNow(_delay_read_after_wakeup_ms);
  } else {
    _last_wakeup_moment.clear();
  }
  return true;
}

bool P053_data_struct::sleepSensor() {
  if (!initialized()) {
    return false;
  }

  // Put the sensor to sleep
  addLog(LOG_LEVEL_INFO, F("PMSx003: Sleep sensor"));

  if (_pwrPin >= 0) {
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, LOW);
  } else {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
    _easySerial->write(command, 7);
  }

  if (_values_received > 0) {
    // Going to sleep, so flush whatever is read.
    Scheduler.schedule_task_device_timer(_taskIndex, millis() + 10);
  }
  return true;
}

void P053_data_struct::setActiveReadingMode() {
  if (initialized()) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71 };
    _easySerial->write(command, 7);
    _activeReadingModeEnabled = true;
  }
}

void P053_data_struct::setPassiveReadingMode() {
  if (initialized()) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70 };
    _easySerial->write(command, 7);
    _activeReadingModeEnabled = false;
  }
}

void P053_data_struct::requestData() {
  if (initialized() && !_activeReadingModeEnabled) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71 };
    _easySerial->write(command, 7);
  }
}

# ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
float P053_data_struct::getValue(uint8_t index) {
  float value = 0.0f;

  getValue(index, value);
  return value;
}

bool P053_data_struct::getValue(uint8_t index, float& value) {
  if (bitRead(_value_mask, index) || (index >= PMS_RECEIVE_BUFFER_SIZE)) {
    // Already read
    return false;
  }

  switch (index) {
    case PMS_PM1_0_ug_m3_factory:
    case PMS_PM2_5_ug_m3_factory:
    case PMS_PM10_0_ug_m3_factory:
    case PMS_FW_rev_error:
    case PMS_Reserved:
      return false;

    case PMS_Temp_C:
    case PMS_Hum_pct:

      if (!hasTempHum()) { return false; }
      value = _data[index] / 10.0f;
      break;
    case PMS_Formaldehyde_mg_m3:

      if (!hasFormaldehyde()) { return false; }
      value = _data[index] / 1000.0f;
      break;
    case PMS_cnt5_0_100ml:
    case PMS_cnt10_0_100ml: // this option was missing :-|

      if (_sensortype == PMSx003_type::PMS5003_T) { return false; } // else: fall through
    case PMS_cnt0_3_100ml:
    case PMS_cnt0_5_100ml:
    case PMS_cnt1_0_100ml:
    case PMS_cnt2_5_100ml:
      value = _data[index];

      if (_splitCntBins) {
        value -= _data[index + 1];

        if (value < 0.0f) { value = 0.0f; }
      }
      break;

    default:
      value = _data[index];
      break;
  }

  if (_values_received > 1) {
    value /= static_cast<float>(_values_received);
  }

  bitSet(_value_mask, index);
  return true;
}

# endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

void P053_data_struct::clearReceivedData() {
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  for (uint8_t i = 0; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
    _data[i] = 0.0f;
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _values_received = 0;
}

const __FlashStringHelper * P053_data_struct::getEventString(uint8_t index) {
  switch (index) {
    // case PMS_PM1_0_ug_m3_factory   : return F("");
    // case PMS_PM2_5_ug_m3_factory   : return F("");
    // case PMS_PM10_0_ug_m3_factory  : return F("");
    case PMS_PM1_0_ug_m3_normal: return F("pm1.0");
    case PMS_PM2_5_ug_m3_normal: return F("pm2.5");
    case PMS_PM10_0_ug_m3_normal: return F("pm10");
    case PMS_cnt0_3_100ml: return F("cnt0.3");
    case PMS_cnt0_5_100ml: return F("cnt0.5");
    case PMS_cnt1_0_100ml: return F("cnt1.0");
    case PMS_cnt2_5_100ml: return F("cnt2.5");
    case PMS_cnt5_0_100ml: return F("cnt5.0");
    case PMS_cnt10_0_100ml: return F("cnt10");
    case PMS_Formaldehyde_mg_m3: return F("HCHO");
    case PMS_Temp_C: return F("Temp");
    case PMS_Hum_pct: return F("Humi");

      // case PMS_FW_rev_error          : return F("");
  }
  return F("Unknown");
}

void P053_data_struct::setTaskValueNames(ExtraTaskSettingsStruct& settings, const uint8_t indices[], uint8_t nrElements,
                                         bool oversample) {
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    settings.TaskDeviceValueDecimals[i] = 0;

    if (i < nrElements) {
      safe_strncpy(
        settings.TaskDeviceValueNames[i],
        P053_data_struct::getEventString(indices[i]),
        sizeof(settings.TaskDeviceValueNames[i]));

      settings.TaskDeviceValueDecimals[i] = getNrDecimals(indices[i], oversample);
    } else {
      ZERO_FILL(settings.TaskDeviceValueNames[i]);
    }
  }
}

unsigned char P053_data_struct::getNrDecimals(uint8_t index, bool oversample) {
  switch (index) {
    case PMS_Temp_C:
    case PMS_Hum_pct:
      return 1;
    case PMS_Formaldehyde_mg_m3:
      return 3;
  }

  if (oversample) { return 1; }
  return 0;
}

#endif // ifdef USES_P053

#include "../PluginStructs/P024_data_struct.h"

#ifdef USES_P024

P024_data_struct::P024_data_struct(uint8_t i2c_addr) : i2cAddress(i2c_addr) {}

float P024_data_struct::readTemperature(uint8_t reg)
{
  float temp = readRegister024(reg);

  temp *= .02f;
  temp -= 273.15f;
  return temp;
}

uint16_t P024_data_struct::readRegister024(uint8_t reg) {
  uint16_t ret;

  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2cAddress, (uint8_t)3);
  ret  = Wire.read();      // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  Wire.read();
  return ret;
}

#endif // ifdef USES_P024

#include "../PluginStructs/P111_data_struct.h"

#ifdef USES_P111

#include "../PluginStructs/P111_data_struct.h"
// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter

#include <MFRC522.h>

P111_data_struct::P111_data_struct(uint8_t csPin, uint8_t rstPin) : mfrc522(nullptr), _csPin(csPin), _rstPin(rstPin)
{}

P111_data_struct::~P111_data_struct() {
  if (mfrc522 != nullptr) {
    delete mfrc522;
    mfrc522 = nullptr;
  }
}

void P111_data_struct::init() {
  if (mfrc522 != nullptr) {
    delete mfrc522;
    mfrc522 = nullptr;
  }
  mfrc522 = new (std::nothrow) MFRC522(_csPin, _rstPin);   // Instantiate a MFRC522
  if (mfrc522 != nullptr) {
    mfrc522->PCD_Init();  // Initialize MFRC522 reader
  }
}

/**
 * read status and tag
 */
uint8_t P111_data_struct::readCardStatus(unsigned long *key, bool *removedTag) {

  uint8_t error = 0;

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  error = readPassiveTargetID(uid, &uidLength);

  switch(error) {
    case 1: // Read error
    {
      errorCount++;
      removedState = false;
      String log = F("MFRC522: Read error: ");
      log += errorCount;
      addLog(LOG_LEVEL_ERROR, log);
      break;
    } 
    case 2: // No tag found
      if (!removedState) {
        removedState = true;
        *removedTag  = true;
        error = 0; // pass through that removal just once
      }
      errorCount = 0;
      break;
    default:  // Read a tag correctly
      errorCount = 0;
      removedState = false; // No longer removed
      break;
  }

  if (errorCount > 2) { // if three consecutive read errors, reset MFRC522
    reset(_csPin,_rstPin);
  }
  unsigned long tmpKey = uid[0];
  for (uint8_t i = 1; i < 4; i++) {
    tmpKey <<= 8;
    tmpKey += uid[i];
  }
  *key = tmpKey;

  return error;
}

/**
 * Returns last read card (type) name
 */
String P111_data_struct::getCardName() {
  return mfrc522->PICC_GetTypeName(mfrc522->PICC_GetType(mfrc522->uid.sak));
}

/*********************************************************************************************\
 * MFRC522 init
\*********************************************************************************************/
bool P111_data_struct::reset(int8_t csPin, int8_t resetPin) {
  if (resetPin != -1) {
    String log = F("MFRC522: Reset on pin: ");
    log += resetPin;
    addLog(LOG_LEVEL_INFO, log);
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(100);
    digitalWrite(resetPin, HIGH);
    pinMode(resetPin, INPUT_PULLUP);
    delay(10);
  }

  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, LOW);
    
  mfrc522->PCD_Init(csPin, resetPin);   // Init MFRC522 module

  //If you set Antenna Gain to Max it will increase reading distance
  mfrc522->PCD_SetAntennaGain(mfrc522->RxGain_max);
  
  bool result = mfrc522->PCD_PerformSelfTest(); // perform the test
  
  if (result) {
    //String log = F("RC522: Found");
    // Get the MFRC522 software version
    uint8_t v = mfrc522->PCD_ReadRegister(mfrc522->VersionReg);
    
    // When 0x00 or 0xFF is returned, communication probably failed
    if ((v == 0x00) || (v == 0xFF)) {
      String log=F("MFRC522: Communication failure, is the MFRC522 properly connected?");
      addLog(LOG_LEVEL_ERROR,log);
      return false;
    } else {
      String log=F("MFRC522: Software Version: ");
      if (v == 0x91)
        log+=F(" = v1.0");
      else if (v == 0x92)
        log+=F(" = v2.0");
      else
        log+=F(" (unknown),probably a chinese clone?");
            
      addLog(LOG_LEVEL_INFO, log);
    }
    return true;
  }
  return false;
}

/*********************************************************************************************\
 * RC522 read tag ID
\*********************************************************************************************/
uint8_t P111_data_struct::readPassiveTargetID(uint8_t *uid, uint8_t *uidLength) { //needed ? see above (not PN532)
  // Getting ready for Reading PICCs
  if ( ! mfrc522->PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 2;
  }
  addLog(LOG_LEVEL_INFO, F("MFRC522: New Card Detected"));
  if ( ! mfrc522->PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 1;
  }
  
  // There are Mifare PICCs which have 4 uint8_t or 7 uint8_t UID care if you use 7 uint8_t PICC
  // I think we should assume every PICC as they have 4 uint8_t UID
  // Until we support 7 uint8_t PICCs
  addLog(LOG_LEVEL_INFO, F("MFRC522: Scanned PICC's UID"));
  for (uint8_t i = 0; i < 4; i++) {  //
    uid[i] = mfrc522->uid.uidByte[i];
  }
  *uidLength = 4;
  mfrc522->PICC_HaltA(); // Stop reading
  return 0;
}


#endif // ifdef USES_P111

#include "../PluginStructs/P003_data_struct.h"


#ifdef USES_P003
P003_data_struct::P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config)
  :
  pulseHelper(config)
{}

P003_data_struct::~P003_data_struct()
{}
#endif
#include "../PluginStructs/P064_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <SparkFun_APDS9960.h> // Lib is modified to work with ESP

#ifdef USES_P064

P064_data_struct::P064_data_struct() {}


#endif // ifdef USES_P064

#include "../PluginStructs/P016_data_struct.h"

#ifdef USES_P016

# include "../Commands/InternalCommands.h"
# include "../Helpers/ESPEasy_Storage.h"
# include <IRutils.h>

# ifdef P16_SETTINGS_V1
tCommandLinesV2::tCommandLinesV2() {} // Default constructor

// Conversion constructor
tCommandLinesV2::tCommandLinesV2(const String& command,
                                 uint32_t      oldCode,
                                 uint32_t      oldAlternativeCode,
                                 uint8_t       i) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    #  ifndef PLUGIN_016_DEBUG
    log.reserve(20); // less space needed
    #  else // ifndef PLUGIN_016_DEBUG
    log.reserve(80);
    #  endif // ifndef PLUGIN_016_DEBUG
  }

  log  = F("P016: converting "); // Still a little logging
  log += i;
  #  ifdef PLUGIN_016_DEBUG
  log += ':';
  #  endif // ifdef PLUGIN_016_DEBUG

  if (command.length() > 0) {
    safe_strncpy(Command, command, P16_Nchars);
  }

  if (oldCode > 0) {
    CodeDecodeType = static_cast<decode_type_t>((oldCode >> 24));                // decode_type
    bitWrite(CodeFlags, P16_FLAGS_REPEAT, oldCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    Code = oldCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" type ");
      log += typeToString(CodeDecodeType, (oldCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" code 0x");
      log += uint64ToString(oldCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }

  if (oldAlternativeCode > 0) {
    AlternativeCodeDecodeType = static_cast<decode_type_t>((oldAlternativeCode >> 24));                // decode_type
    bitWrite(AlternativeCodeFlags, P16_FLAGS_REPEAT, oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    AlternativeCode = oldAlternativeCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" alt.type ");
      log += typeToString(AlternativeCodeDecodeType, (oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" alt.code 0x");
      log += uint64ToString(oldAlternativeCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }
  addLog(LOG_LEVEL_INFO, log);
}

# endif // ifdef P16_SETTINGS_V1

P016_data_struct::P016_data_struct() {}

void P016_data_struct::init(struct EventStruct *event, uint16_t CmdInhibitTime) {
  loadCommandLines(event);
  iCmdInhibitTime = CmdInhibitTime;
  iLastCmd        = 0;
  iLastCmdTime    = 0;
}

# ifdef P16_SETTINGS_V1
void P016_data_struct::convertCommandLines(struct EventStruct *event) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    #  ifndef PLUGIN_016_DEBUG
    log.reserve(20); // less space needed
    #  else // ifndef PLUGIN_016_DEBUG
    log.reserve(80);

    log  = F("P016: struct size: ");
    log += sizeof(tCommandLinesV2);
    log += '/';
    log += sizeof(tCommandLines);
    log += F(" enum: ");
    log += sizeof(decode_type_t);
    log += F(" p016: ");
    log += sizeof(P016_data_struct);
    addLog(LOG_LEVEL_INFO, log);
    #  endif // ifdef PLUGIN_016_DEBUG
  }

  // read V1 data && convert
  CommandLinesV1.clear(); // Start fresh
  int loadOffset = 0;

  for (uint8_t i = 0; i < P16_Nlines; i++) {
    CommandLinesV1.push_back(tCommandLines());
    LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
                 event->TaskIndex,
                 reinterpret_cast<uint8_t *>(&(CommandLinesV1[i])),
                 sizeof(tCommandLines),
                 loadOffset);
    loadOffset += sizeof(tCommandLines);
  }

  CommandLines.clear(); // Start fresh

  for (int i = 0; i < P16_Nlines; ++i) {
    CommandLines.push_back(tCommandLinesV2(String(CommandLinesV1[i].Command), CommandLinesV1[i].Code, CommandLinesV1[i].AlternativeCode, i));

    delay(0);
  }
  CommandLinesV1.clear(); // clean up after conversion
}

# endif // ifdef P16_SETTINGS_V1

void P016_data_struct::loadCommandLines(struct EventStruct *event) {
  # ifdef P16_SETTINGS_V1

  // Convert the settings if both versions are defined and PCONFIG(7) != latest version
  if (PCONFIG(7) != P16_SETTINGS_LATEST) {
    addLog(LOG_LEVEL_ERROR, F("P016 IR: Settings conversion, save task settings to store in new format."));

    convertCommandLines(event);
  }
  else
  # endif // ifdef P16_SETTINGS_V1
  {
    // read V2 settings data

    CommandLines.clear(); // Start fresh
    int loadOffset = 0;

    for (uint8_t i = 0; i < P16_Nlines; i++) {
      CommandLines.push_back(tCommandLinesV2());
      LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
                   event->TaskIndex,
                   reinterpret_cast<uint8_t *>(&(CommandLines[i])),
                   sizeof(tCommandLinesV2),
                   loadOffset);
      loadOffset += sizeof(tCommandLinesV2);
    }
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    CommandLines[i].Command[P16_Nchars - 1] = 0; // Terminate in case of uninitalized data
  }
}

void P016_data_struct::saveCommandLines(struct EventStruct *event) {
  int saveOffset = 0;

  for (uint8_t i = 0; i < P16_Nlines; i++) {
    SaveToFile(SettingsType::Enum::CustomTaskSettings_Type,
               event->TaskIndex,
               reinterpret_cast<const uint8_t *>(&(CommandLines[i])),
               sizeof(tCommandLinesV2),
               saveOffset);
    saveOffset += sizeof(tCommandLinesV2);
  }
}

void P016_data_struct::AddCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  // add received code
  int _index = P16_Nlines;

  if (Code == 0) {
    return;
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      return;
    }

    if (CommandLines[i].Code == 0) {
      // get first index to add the code
      _index = std::min(i, _index);
    }
  }

  if (_index == P16_Nlines) {
    // no free index
    return;
  }
  CommandLines[_index].Code           = Code;
  CommandLines[_index].CodeDecodeType = DecodeType;
  CommandLines[_index].CodeFlags      = CodeFlags;
  bCodeChanged                        = true;
  # ifdef PLUGIN_016_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80); // estimated
    log  = F("[P36] AddCode: ");
    log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
    log += F(" code: 0x");
    log += uint64ToString(Code, 16);
    log += F(" to index ");
    log += _index;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // PLUGIN_016_DEBUG
}

void P016_data_struct::ExecuteCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  if (Code == 0) {
    return;
  }

  if ((iLastCmd == Code) && (iLastDecodeType == DecodeType)) {
    // same code as before
    if (iCmdInhibitTime > timePassedSince(iLastCmdTime)) {
      // inhibit time not ellapsed
      return;
    }
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      iLastCmd        = Code;
      iLastDecodeType = DecodeType;
      iLastCodeFlags  = CodeFlags;
      iLastCmdTime    = millis();

      if (CommandLines[i].Command[0] != 0) {
        # ifdef PLUGIN_016_DEBUG
        bool _success =
        # endif // ifdef PLUGIN_016_DEBUG
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SYSTEM, CommandLines[i].Command);
        # ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(128); // estimated
          log  = F("[P36] Execute: ");
          log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
          log += F(" Code: 0x");
          log += uint64ToString(Code, 16);
          log += F(" with command ");
          log += (i + 1);
          log += F(": {");
          log += String(CommandLines[i].Command);
          log += '}';

          if (!_success) {
            log += F(" FAILED!");
          }
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // PLUGIN_016_DEBUG
      }
      return;
    }
  }
}

bool P016_data_struct::validateCode(int i, uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  return ((CommandLines[i].Code == Code)
          && (CommandLines[i].CodeDecodeType == DecodeType)
          && (CommandLines[i].CodeFlags == CodeFlags))
         || ((CommandLines[i].AlternativeCode == Code)
             && (CommandLines[i].AlternativeCodeDecodeType == DecodeType)
             && (CommandLines[i].AlternativeCodeFlags == CodeFlags));
}

#endif // ifdef USES_P016

#include "../PluginStructs/P020_data_struct.h"

#ifdef USES_P020

# include "../ESPEasyCore/Serial.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"

# include "../Globals/EventQueue.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"

# define P020_RX_WAIT              PCONFIG(4)
# define P020_RX_BUFFER            PCONFIG(7)


P020_Task::P020_Task(taskIndex_t taskIndex) : _taskIndex(taskIndex) {
  clearBuffer();
}

P020_Task::~P020_Task() {
  stopServer();
  serialEnd();
}

bool P020_Task::serverActive(WiFiServer *server) {
# if defined(ESP8266)
  return nullptr != server && server->status() != CLOSED;
# elif defined(ESP32)
  return nullptr != server && *server;
# endif // if defined(ESP8266)
}

void P020_Task::startServer(uint16_t portnumber) {
  if ((gatewayPort == portnumber) && serverActive(ser2netServer)) {
    // server is already listening on this port
    return;
  }
  stopServer();
  gatewayPort   = portnumber;
  ser2netServer = new (std::nothrow) WiFiServer(portnumber);

  if ((nullptr != ser2netServer) && NetworkConnected()) {
    ser2netServer->begin();

    if (serverActive(ser2netServer)) {
      addLog(LOG_LEVEL_INFO, String(F("Ser2Net  : WiFi server started at port ")) + portnumber);
    } else {
      addLog(LOG_LEVEL_ERROR, String(F("Ser2Net   : WiFi server start failed at port ")) +
             portnumber + String(F(", retrying...")));
    }
  }
}

void P020_Task::checkServer() {
  if ((nullptr != ser2netServer) && !serverActive(ser2netServer) && NetworkConnected()) {
    ser2netServer->close();
    ser2netServer->begin();

    if (serverActive(ser2netServer)) {
      addLog(LOG_LEVEL_INFO, F("Ser2net   : WiFi server started"));
    }
  }
}

void P020_Task::stopServer() {
  if (nullptr != ser2netServer) {
    if (ser2netClient) { ser2netClient.stop(); }
    clientConnected = false;
    ser2netServer->close();
    addLog(LOG_LEVEL_INFO, F("Ser2net   : WiFi server closed"));
    delete ser2netServer;
    ser2netServer = nullptr;
  }
}

bool P020_Task::hasClientConnected() {
  if ((nullptr != ser2netServer) && ser2netServer->hasClient())
  {
    if (ser2netClient) { ser2netClient.stop(); }
    ser2netClient = ser2netServer->available();
    ser2netClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
    sendConnectedEvent(true);
    addLog(LOG_LEVEL_INFO, F("Ser2Net   : Client connected!"));
  }

  if (ser2netClient.connected())
  {
    clientConnected = true;
  }
  else
  {
    if (clientConnected) // there was a client connected before...
    {
      clientConnected = false;
      sendConnectedEvent(false);
      addLog(LOG_LEVEL_INFO, F("Ser2net   : Client disconnected!"));
    }
  }
  return clientConnected;
}

void P020_Task::discardClientIn() {
  // flush all data received from the WiFi gateway
  // as a P1 meter does not receive data
  while (ser2netClient.available()) {
    ser2netClient.read();
  }
}

void P020_Task::clearBuffer() {
  serial_buffer = "";
  serial_buffer.reserve(P020_DATAGRAM_MAX_SIZE);
}

void P020_Task::serialBegin(const ESPEasySerialPort port, int16_t rxPin, int16_t txPin, unsigned long baud, uint8_t config) {
  serialEnd();

  if (rxPin >= 0) {
    ser2netSerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin);

    if (nullptr != ser2netSerial) {
        # if defined(ESP8266)
      ser2netSerial->begin(baud, (SerialConfig)config);
        # elif defined(ESP32)
      ser2netSerial->begin(baud, config);
        # endif // if defined(ESP8266)
      addLog(LOG_LEVEL_DEBUG, F("Ser2net   : Serial opened"));
    }
  }
}

void P020_Task::serialEnd() {
  if (nullptr != ser2netSerial) {
    delete ser2netSerial;
    clearBuffer();
    ser2netSerial = nullptr;
    addLog(LOG_LEVEL_DEBUG, F("Ser2net   : Serial closed"));
  }
}

void P020_Task::handleClientIn(struct EventStruct *event) {
  int count      = ser2netClient.available();
  int bytes_read = 0;
  uint8_t net_buf[P020_DATAGRAM_MAX_SIZE];

  if (count > 0) {
    if (count > P020_DATAGRAM_MAX_SIZE) { count = P020_DATAGRAM_MAX_SIZE; }
    bytes_read = ser2netClient.read(net_buf, count);
    ser2netSerial->write(net_buf, bytes_read);
    ser2netSerial->flush();             // Waits for the transmission of outgoing serial data to

    while (ser2netClient.available()) { // flush overflow data if available
      ser2netClient.read();
    }
  }
}

void P020_Task::handleSerialIn(struct EventStruct *event) {
  if (nullptr == ser2netSerial) { return; }
  int RXWait  = P020_RX_WAIT;
  int timeOut = RXWait;

  do {
    if (ser2netSerial->available()) {
      if (serial_buffer.length() > static_cast<size_t>(P020_RX_BUFFER)) {
        addLog(LOG_LEVEL_DEBUG, F("Ser2Net   : Error: Buffer overflow, discarded input."));
        ser2netSerial->read();
      }
      else { serial_buffer += (char)ser2netSerial->read(); }
      timeOut = RXWait; // if serial received, reset timeout counter
    } else {
      if (timeOut <= 0) { break; }
      delay(1);
      --timeOut;
    }
  } while (true);

  if (serial_buffer.length() > 0) {
    ser2netClient.print(serial_buffer);
    rulesEngine(serial_buffer);
    ser2netClient.flush();
    clearBuffer();
    addLog(LOG_LEVEL_DEBUG, F("Ser2Net   : data send!"));
  } // done
}

void P020_Task::discardSerialIn() {
  if (nullptr != ser2netSerial) {
    while (ser2netSerial->available()) {
      ser2netSerial->read();
    }
  }
}

// We can also use the rules engine for local control!
void P020_Task::rulesEngine(String message) {
  if (!(Settings.UseRules)) { return; }
  int NewLinePos = message.indexOf(F("\r\n"));

  if (NewLinePos > 0) { message = message.substring(0, NewLinePos); }
  String eventString;

  switch (serial_processing) {
    case 0: { break; }
    case 1: { // Generic
      eventString  = F("!Serial#");
      eventString += message;
      break;
    }
    case 2: {                               // RFLink
      message = message.substring(6);       // RFLink, strip 20;xx; from incoming message

      if (message.startsWith("ESPEASY"))    // Special treatment for gpio values, strip unneeded parts...
      {
        message     = message.substring(8); // Strip "ESPEASY;"
        eventString = F("RFLink#");
      }
      else {
        eventString = F("!RFLink#"); // default event as it comes in, literal match needed in rules, using '!'
      }
      eventString += message;
      break;
    }
  } // switch

  if (eventString.length() > 0) { eventQueue.addMove(std::move(eventString)); }
}

bool P020_Task::isInit() const {
  return nullptr != ser2netServer && nullptr != ser2netSerial;
}

void P020_Task::sendConnectedEvent(bool connected)
{
  if (Settings.UseRules)
  {
    String RuleEvent;
    RuleEvent += getTaskDeviceName(_taskIndex);
    RuleEvent += '#';
    RuleEvent += F("Client");
    RuleEvent += '=';
    RuleEvent += (connected ? 1 : 0);
    eventQueue.addMove(std::move(RuleEvent));
  }
}

#endif // ifdef USES_P020

#include "../PluginStructs/P117_data_struct.h"

#ifdef USES_P117

// **************************************************************************/
// Constructor
// **************************************************************************/
P117_data_struct::P117_data_struct(uint16_t altitude, float temperatureOffset)
  : _altitude(altitude), _temperatureOffset(temperatureOffset) {}


// **************************************************************************/
// Initialize sensor and read data from SCD30
// **************************************************************************/
uint32_t P117_data_struct::read_sensor(uint16_t *scd30_CO2, uint16_t *scd30_CO2EAvg, float *scd30_Temp, float *scd30_Humid) {
  if (!initialised) {
    initialised = init_sensor(); // Check id device is present
  }

  if (initialised) {
    return scd30.readMeasurement(scd30_CO2, scd30_CO2EAvg, scd30_Temp, scd30_Humid);
  }
  return ERROR_SCD30_NO_DATA;
}

// **************************************************************************/
// Check SCD30 presence and initialize
// **************************************************************************/
bool P117_data_struct::softReset() {
  if (initialised) {
    scd30.softReset();
  }
  return initialised;
}

// **************************************************************************/
// Check SCD30 presence and initialize
// **************************************************************************/
bool P117_data_struct::init_sensor() {
  if (!initialised) {
    scd30.begin();
    uint16_t calibration = 0;
    scd30.getCalibrationType(&calibration);

    if (calibration) {
      scd30.setManualCalibration();
    }
    scd30.beginMeasuring();
    scd30.setAltitudeCompensation(_altitude);
    scd30.setTemperatureOffset(_temperatureOffset);
    return true;
  }

  return initialised;
}

#endif // ifdef USES_P117

#include "../PluginStructs/P115_data_struct.h"

#ifdef USES_P115

P115_data_struct::P115_data_struct(
  uint8_t                i2c_addr,
  sfe_max1704x_devices_e device,
  int                    threshold)
  : _device(device), lipo(device), _threshold(threshold), initialized(false)
{
  // Call begin() immediately, or else _i2cPort in lipo object is still a nullptr
  lipo.begin();
  _threshold = constrain(_threshold, 1, 32);
}

bool P115_data_struct::begin()
{
  // quickStart() - Restarts the MAX1704x to allow it to re-"guess" the
  // parameters that go into its SoC algorithms. Calling this in your setup()
  // usually results in more accurate SoC readings.
  // Output: 0 on success, positive integer on fail.

  // FIXME TD-er: Looks like begin() and/or quickStart() may not return expected values.
  // const bool success = lipo.begin() && (lipo.quickStart() == 0);
  lipo.begin();
  lipo.quickStart();
  const bool success = true;

  if (success) {
    switch (_device) {
      case MAX1704X_MAX17043:
      case MAX1704X_MAX17044:
        break;
      case MAX1704X_MAX17048:
      case MAX1704X_MAX17049:
        lipo.enableSOCAlert();
        break;
    }
    lipo.setThreshold(_threshold);
    initialized = true;
    return true;
  }
  initialized = false;
  return false;
}

bool P115_data_struct::read(bool clearAlert)
{
  voltage    = lipo.getVoltage();
  soc        = lipo.getSOC();
  changeRate = lipo.getChangeRate();
  return lipo.getAlert(clearAlert);
}

void P115_data_struct::clearAlert() {
  alert = false;
  lipo.clearAlert();
}

#endif // ifdef USES_P115

#include "../PluginStructs/P069_data_struct.h"

#ifdef USES_P069

#include "../Globals/I2Cdev.h"


# define LM75A_BASE_ADDRESS        0x48
# define LM75A_DEGREES_RESOLUTION  0.125f
# define LM75A_REG_ADDR_TEMP     0


P069_data_struct::P069_data_struct(bool A0_value, bool A1_value, bool A2_value)
{
  _i2c_device_address = LM75A_BASE_ADDRESS;

  if (A0_value) {
    _i2c_device_address += 1;
  }

  if (A1_value) {
    _i2c_device_address += 2;
  }

  if (A2_value) {
    _i2c_device_address += 4;
  }
}

P069_data_struct::P069_data_struct(uint8_t addr)
{
  _i2c_device_address = addr;
}

void P069_data_struct::setAddress(uint8_t addr)
{
  _i2c_device_address = addr;
}

float P069_data_struct::getTemperatureInDegrees() const
{
  float   real_result = NAN;
  int16_t value       = 0;

  // Go to temperature data register
  Wire.beginTransmission(_i2c_device_address);
  Wire.write(LM75A_REG_ADDR_TEMP);

  if (Wire.endTransmission())
  {
    // Transmission error
    return real_result;
  }

  // Get content
  Wire.requestFrom(_i2c_device_address, (uint8_t)2);

  if (Wire.available() == 2)
  {
    value = (Wire.read() << 8) | Wire.read();
  }
  else
  {
    // Can't read temperature
    return real_result;
  }

  // Shift data (left-aligned)
  value >>= 5;

  // Relocate negative bit (11th bit to 16th bit)
  if (value & 0x0400) // negative?
  {
    value |= 0xFC00;  // expand to 16 bit
  }

  // Real value can be calculated with sensor resolution
  real_result = static_cast<float>(value) * LM75A_DEGREES_RESOLUTION;

  return real_result;
}

#endif // ifdef USES_P069

#include "../PluginStructs/P092_data_struct.h"

#ifdef USES_P092

//
// DLBus reads and decodes the DL-Bus.
// The DL-Bus is used in heating control units e.g. sold by Technische Alternative (www.ta.co.at).
// Author uwekaditz

// #define DLbus_DEBUG

// Flags for pulse width (bit 0 is the content!)
#define DLbus_FlagSingleWidth                 0x02
#define DLbus_FlagDoubleWidth                 0x04
#define DLbus_FlagShorterThanSingleWidth      0x10
#define DLbus_FlagBetweenDoubleSingleWidth    0x20
#define DLbus_FlagLongerThanDoubleWidth       0x40
#define DLbus_FlagLongerThanTwiceDoubleWidth  0x80
#define DLbus_FlagsWrongTiming                (DLbus_FlagLongerThanTwiceDoubleWidth | DLbus_FlagLongerThanDoubleWidth | \
                                               DLbus_FlagBetweenDoubleSingleWidth | DLbus_FlagShorterThanSingleWidth)

// Helper for ISR call
DLBus *DLBus::__instance                         = nullptr;
volatile  static uint8_t *ISR_PtrChangeBitStream = nullptr; // pointer to received bit change stream

DLBus::DLBus()
{
  if (__instance == nullptr)
  {
    __instance             = this;
    ISR_PtrChangeBitStream = DLbus_ChangeBitStream;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus created"));
  }
}

DLBus::~DLBus()
{
  if (__instance == this)
  {
    __instance             = nullptr;
    ISR_PtrChangeBitStream = nullptr;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus destroyed"));
  }
}

void DLBus::AddToInfoLog(const String& string)
{
  if ((IsLogLevelInfo) && (LogLevelInfo != 0xff)) {
    addToLog(LogLevelInfo, string);
  }
}

void DLBus::AddToErrorLog(const String& string)
{
  if (LogLevelError != 0xff) {
    addToLog(LogLevelError, string);
  }
}

void DLBus::attachDLBusInterrupt(void)
{
  ISR_Receiving = false;
  IsISRset = true;
  IsNoData = false;
  attachInterrupt(digitalPinToInterrupt(ISR_DLB_Pin), ISR, CHANGE);
}

void DLBus::StartReceiving(void)
{
  noInterrupts(); // make sure we don't get interrupted before we are ready
  ISR_PulseCount      = 0;
  ISR_Receiving       = (ISR_PtrChangeBitStream != nullptr);
  ISR_AllBitsReceived = false;
  interrupts(); // interrupts allowed now, next instruction WILL be executed
}

void ICACHE_RAM_ATTR DLBus::ISR(void)
{
  if (__instance)
  {
    __instance->ISR_PinChanged();
  }
}

void ICACHE_RAM_ATTR DLBus::ISR_PinChanged(void)
{
//  long TimeDiff = usecPassedSince(ISR_TimeLastBitChange); // time difference to previous pulse in Âµs
  uint32_t _now = micros();
  int32_t TimeDiff = (int32_t)(_now - ISR_TimeLastBitChange);

//  ISR_TimeLastBitChange = micros();                           // save last pin change time
  ISR_TimeLastBitChange = _now;                           // save last pin change time

  if (ISR_Receiving) {
    uint8_t val = digitalRead(ISR_DLB_Pin);               // read state

    // check pulse width
    if (TimeDiff >= 2 * ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagLongerThanTwiceDoubleWidth; // longer then 2x double pulse width
    }
    else if (TimeDiff > ISR_MaxDoublePulseWidth) {
      val |= DLbus_FlagLongerThanDoubleWidth;      // longer then double pulse width
    }
    else if (TimeDiff >= ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagDoubleWidth;                // double pulse width
    }
    else if (TimeDiff > ISR_MaxPulseWidth) {
      val |= DLbus_FlagBetweenDoubleSingleWidth;   // between double and single pulse width
    }
    else if (TimeDiff < ISR_MinPulseWidth) {
      val |= DLbus_FlagShorterThanSingleWidth;     // shorter then single pulse width
    }
    else {
      val |= DLbus_FlagSingleWidth;                // single pulse width
    }

    if (ISR_PulseCount < 2) {
      // check if sync is received
      if (val & DLbus_FlagLongerThanTwiceDoubleWidth) {
        // sync received
        *ISR_PtrChangeBitStream       = !(val & 0x01);
        *(ISR_PtrChangeBitStream + 1) = val;
        ISR_PulseCount                = 2;
      }
      else {
        ISR_PulseCount = 1; // flag that interrupt is receiving
      }
    }
    else {
      *(ISR_PtrChangeBitStream + ISR_PulseCount) = val;         // set bit
      ISR_PulseCount++;
      ISR_Receiving       = (ISR_PulseCount < ISR_PulseNumber); // stop P092_receiving when data frame is complete
      ISR_AllBitsReceived = !ISR_Receiving;
    }
  }
}

boolean DLBus::CheckTimings(void) {
  uint8_t rawval, val;
  uint8_t WrongTimeCnt = 0;
  int     i;

#ifdef DLbus_DEBUG
  uint16_t WrongTimingArray[5][6];
#endif // DLbus_DEBUG

  //  AddToInfoLog(F("Receive stopped."));

  ISR_PulseCount = 0;

  for (i = 0; i <= ISR_PulseNumber; i++) {
    // store DLbus_ChangeBitStream into ByteStream
    rawval = *(ISR_PtrChangeBitStream + i);

    if (rawval & DLbus_FlagsWrongTiming) {
      // wrong DLbus_time_diff
      if (ISR_PulseCount > 0) {
#ifdef DLbus_DEBUG
        WrongTimingArray[WrongTimeCnt][0] = i;
        WrongTimingArray[WrongTimeCnt][1] = ISR_PulseCount;
        WrongTimingArray[WrongTimeCnt][2] = BitNumber;
        WrongTimingArray[WrongTimeCnt][3] = rawval;
#endif // DLbus_DEBUG

        if ((rawval == DLbus_FlagLongerThanTwiceDoubleWidth) && (*(ISR_PtrChangeBitStream + i - 1) == (DLbus_FlagDoubleWidth | 0x01))) {
          // Add two additional short pulses (low and high), previous bit is High and contains DLbus_FlagDoubleWidth
          ProcessBit(0);
          ProcessBit(1);
#ifdef DLbus_DEBUG
          WrongTimingArray[WrongTimeCnt][4] = DLbus_FlagSingleWidth;
          WrongTimingArray[WrongTimeCnt][5] = DLbus_FlagSingleWidth + 1;
#endif // DLbus_DEBUG
        }
#ifdef DLbus_DEBUG
        else {
          WrongTimingArray[WrongTimeCnt][4] = 0xff;
          WrongTimingArray[WrongTimeCnt][5] = 0xff;
        }
#endif // DLbus_DEBUG
        WrongTimeCnt++;

        if (WrongTimeCnt >= 5) {
          return false;
        }
      }
    }
    else {
      val = rawval & 0x01;

      if ((rawval & DLbus_FlagDoubleWidth) == DLbus_FlagDoubleWidth) {
        // double pulse width
        ProcessBit(!val);
        ProcessBit(val);
      }
      else {
        // single pulse width
        ProcessBit(val);
      }
    }
  }

  //  AddToInfoLog(F("DLbus_ChangeBitStream copied."));

#ifdef DLbus_DEBUG

  if (WrongTimeCnt > 0) {
    if (IsLogLevelInfo) {
      String log = F("Wrong Timings: ");
      AddToInfoLog(log);

      for (i = 0; i < WrongTimeCnt; i++) {
        log  = i + 1;
        log += F(": PulseCount:");
        log += WrongTimingArray[i][1];
        log += F(": BitCount:");
        log += WrongTimingArray[i][2];
        log += F(" Value:0x");
        log += String(WrongTimingArray[i][3], HEX);
        log += F(" ValueBefore:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] - 1), HEX);
        log += F(" ValueAfter:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] + 1), HEX);

        if (WrongTimingArray[i][4] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][4], HEX);
        }

        if (WrongTimingArray[i][5] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][5], HEX);
        }
        AddToInfoLog(log);
      }
    }
  }
#endif // DLbus_DEBUG
  return true;
}

void DLBus::ProcessBit(uint8_t b) {
  // ignore first pulse
  ISR_PulseCount++;

  if (ISR_PulseCount % 2) {
    return;
  }
  BitNumber = (ISR_PulseCount / 2);

  if (b) {
    ByteStream[BitNumber / 8] |= (1 << (BitNumber % 8));  // set bit
  }
  else {
    ByteStream[BitNumber / 8] &= ~(1 << (BitNumber % 8)); // clear bit
  }
}

boolean DLBus::Processing(void) {
  boolean inverted = false;
  int16_t StartBit; // first bit of data frame (-1 not recognized)
  String  log;

  AddToInfoLog(F("Processing..."));
  StartBit = Analyze(); // find the data frame's beginning

  // inverted signal?
  while (StartBit == -1) {
    if (inverted) {
      AddToErrorLog(F("Error: Already inverted!"));
      return false;
    }
    Invert(); // invert again
    inverted = true;
    StartBit = Analyze();

    if (StartBit == -1) {
      AddToErrorLog(F("Error: No data frame available!"));
      return false;
    }
    uint16_t RequiredBitStreamLength = (ISR_PulseNumber - DLBus_ReserveBytes) / DLBus_BitChangeFactor;

    if ((BitNumber - StartBit) < RequiredBitStreamLength) {
      // no complete data frame available (difference between start_bit and received bits is < RequiredBitStreamLength)
      AddToErrorLog(F("Start bit too close to end of stream!"));

      if (IsLogLevelInfo) {
        log  = F("# Required bits: ");
        log += RequiredBitStreamLength;
        log += F(" StartBit: ");
        log += StartBit;
        log += F(" / EndBit: ");
        log += BitNumber;
        AddToInfoLog(log);
      }
      return false;
    }
  }

  if (IsLogLevelInfo) {
    log  = F("StartBit: ");
    log += StartBit;
    log += F(" / EndBit: ");
    log += BitNumber;
    AddToInfoLog(log);
  }
  Trim(StartBit);      // remove start and stop bits

  if (CheckDevice()) { // check connected device
    return true;
  }
  else {
    AddToErrorLog(F("Error: Device not found!"));
    return false;
  }
}

int DLBus::Analyze(void) {
  uint8_t sync = 0;

  // find SYNC (16 * sequential 1)
  for (int i = 0; i < BitNumber; i++) {
    if (ReadBit(i)) {
      sync++;
    }
    else {
      sync = 0;
    }

    if (sync == DLBus_SyncBits) {
      // finde erste 0 // find first 0
      while (ReadBit(i) == 1) {
        i++;
      }
      return i; // beginning of data frame
    }
  }

  // no data frame available. check signal?
  return -1;
}

void DLBus::Invert(void) {
  AddToInfoLog(F("Invert bit stream..."));

  for (int i = 0; i < BitNumber; i++) {
    WriteBit(i, ReadBit(i) ? 0 : 1); // invert every bit
  }
}

uint8_t DLBus::ReadBit(int pos) {
  int row = pos / 8;                          // detect position in bitmap
  int col = pos % 8;

  return ((ByteStream[row]) >> (col)) & 0x01; // return bit
}

void DLBus::WriteBit(int pos, uint8_t set) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;

  if (set) {
    ByteStream[row] |= 1 << col;    // set bit
  }
  else {
    ByteStream[row] &= ~(1 << col); // clear bit
  }
}

void DLBus::Trim(int start_bit) {
  for (int i = start_bit, bit = 0; i < BitNumber; i++) {
    int offset = i - start_bit;

    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      WriteBit(bit, ReadBit(i));
      bit++;
    }
  }
}

boolean DLBus::CheckDevice(void) {
  // Data frame of a device?
  if (ByteStream[0] == DeviceBytes[0]) {
    if ((DeviceBytes[1] == 0) || (ByteStream[1] == DeviceBytes[1])) {
      return true;
    }
  }

  if (IsLogLevelInfo) {
    String log = F("# Received DeviceByte(s): 0x");
    log += String(ByteStream[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(ByteStream[1], HEX);
    }
    log += F(" Requested: 0x");
    log += String(DeviceBytes[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(DeviceBytes[1], HEX);
    }
    AddToInfoLog(log);
  }
  return false;
}

boolean DLBus::CheckCRC(uint8_t IdxCRC) {
  // CRC check sum
  if (IdxCRC == 0) {
    return true;
  }
  AddToInfoLog(F("Check CRC..."));
  uint16_t dataSum = 0;

  for (int i = 0; i < IdxCRC; i++) {
    dataSum = dataSum + ByteStream[i];
  }
  dataSum = dataSum & 0xff;

  if (dataSum == ByteStream[IdxCRC]) {
    return true;
  }
  AddToErrorLog(F("Check CRC failed!"));

  if (IsLogLevelInfo) {
    String log = F("# Calculated CRC: 0x");
    log += String(dataSum, HEX);
    log += F(" Received: 0x");
    log += String(ByteStream[IdxCRC], HEX);
    AddToInfoLog(log);
  }
  return false;
}

// sensor types
# define DLbus_UNUSED              0b000
# define DLbus_Sensor_DIGITAL      0b001
# define DLbus_Sensor_TEMP         0b010
# define DLbus_Sensor_VOLUME_FLOW  0b011
# define DLbus_Sensor_RAYS         0b110
# define DLbus_Sensor_ROOM         0b111

// room sensor modes
# define DLbus_RSM_AUTO            0b00
# define DLbus_RSM_NORMAL          0b01
# define DLbus_RSM_LOWER           0b10
# define DLbus_RSM_STANDBY         0b11


P092_data_struct::P092_data_struct() {}

P092_data_struct::~P092_data_struct() {
  if (DLbus_Data != nullptr) {
    if (DLbus_Data->ISR_DLB_Pin != 0xFF) {
      detachInterrupt(digitalPinToInterrupt(DLbus_Data->ISR_DLB_Pin));
    }
    delete DLbus_Data;
    DLbus_Data = nullptr;
  }

}

bool P092_data_struct::init(int8_t pin1, int P092DeviceIndex, eP092pinmode P092pinmode) {
  DLbus_Data = new (std::nothrow) DLBus;

  if (DLbus_Data == nullptr) {
    return false;
  }
  DLbus_Data->LogLevelInfo   = LOG_LEVEL_INFO;
  DLbus_Data->LogLevelError  = LOG_LEVEL_ERROR;
  DLbus_Data->IsLogLevelInfo = loglevelActiveFor(LOG_LEVEL_INFO);
  DLbus_Data->ISR_DLB_Pin    = pin1;

  //interrupt is detached in PLUGIN_WEBFORM_SAVE and attached in PLUGIN_ONCE_A_SECOND
  //to ensure that new interrupt is attached after new pin is configured, setting
  //IsISRset to false is done here.
  DLbus_Data->IsISRset       = false;

  switch (P092pinmode) {
    case eP092pinmode::ePPM_InputPullUp:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin with pullup"));
      pinMode(pin1, INPUT_PULLUP);
    break;
#ifdef INPUT_PULLDOWN
    case eP092pinmode::ePPM_InputPullDown:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin with pulldown"));
      pinMode(pin1, INPUT_PULLDOWN);
    break;
#endif
    default:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin"));
      pinMode(pin1, INPUT);
  }

// on a CHANGE on the data pin P092_Pin_changed is called
//DLbus_Data->attachDLBusInterrupt();
  return true;
}

void P092_data_struct::Plugin_092_SetIndices(int P092DeviceIndex) {
  // Set the indices for the DL bus packet
  int iDeviceBytes, iDontCareBytes, iTimeStampBytes;

  // default settings for ESR21
  P092_DataSettings.DataBytes                 = 31;
  P092_DataSettings.DLbus_MinPulseWidth       = P092_min_width_488;
  P092_DataSettings.DLbus_MaxPulseWidth       = P092_max_width_488;
  P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
  P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

  P092_DataSettings.DeviceByte0    = 0x70;
  P092_DataSettings.DeviceByte1    = 0x8F;
  iDeviceBytes                     = 2;
  iDontCareBytes                   = 0;
  iTimeStampBytes                  = 0;
  P092_DataSettings.MaxSensors     = 3;
  P092_DataSettings.MaxExtSensors  = 6;
  P092_DataSettings.OutputBytes    = 1;
  P092_DataSettings.SpeedBytes     = 1;
  P092_DataSettings.MaxAnalogOuts  = 1;
  P092_DataSettings.AnalogBytes    = 1;
  P092_DataSettings.VolumeBytes    = 0;
  P092_DataSettings.MaxHeatMeters  = 1;
  P092_DataSettings.CurrentHmBytes = 2;
  P092_DataSettings.MWhBytes       = 2;
  P092_DataSettings.IdxCRC         = 30;

  switch (P092DeviceIndex) {
    case 31: // UVR31
      P092_DataSettings.DataBytes                 = 8;
      P092_DataSettings.DLbus_MinPulseWidth       = P092_min_width_50;
      P092_DataSettings.DLbus_MaxPulseWidth       = P092_max_width_50;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_50;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_50;

      P092_DataSettings.DeviceByte0    = 0x30;
      P092_DataSettings.DeviceByte1    = 0;
      iDeviceBytes                     = 1;
      P092_DataSettings.MaxExtSensors  = 0;
      P092_DataSettings.SpeedBytes     = 0;
      P092_DataSettings.AnalogBytes    = 0;
      P092_DataSettings.MaxAnalogOuts  = 0;
      P092_DataSettings.MaxHeatMeters  = 0;
      P092_DataSettings.CurrentHmBytes = 0;
      P092_DataSettings.MWhBytes       = 0;
      P092_DataSettings.IdxCRC         = 0;
      break;
    case 1611: // UVR1611
      P092_DataSettings.DataBytes = 64;

      P092_DataSettings.DeviceByte0    = 0x80;
      P092_DataSettings.DeviceByte1    = 0x7F;
      iDontCareBytes                   = 1;
      iTimeStampBytes                  = 5;
      P092_DataSettings.MaxSensors     = 16;
      P092_DataSettings.MaxExtSensors  = 0;
      P092_DataSettings.OutputBytes    = 2;
      P092_DataSettings.SpeedBytes     = 4;
      P092_DataSettings.AnalogBytes    = 0;
      P092_DataSettings.MaxAnalogOuts  = 0;
      P092_DataSettings.MaxHeatMeters  = 2;
      P092_DataSettings.CurrentHmBytes = 4;
      P092_DataSettings.IdxCRC         = P092_DataSettings.DataBytes - 1;

      break;
    case 6132: // UVR 61-3 (up to V8.2)
      P092_DataSettings.DataBytes = 35;

      P092_DataSettings.DeviceByte0   = 0x90;
      P092_DataSettings.DeviceByte1   = 0x6F;
      iDontCareBytes                  = 1;
      iTimeStampBytes                 = 5;
      P092_DataSettings.MaxSensors    = 6;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.MaxAnalogOuts = 1;
      P092_DataSettings.VolumeBytes   = 2;
      P092_DataSettings.MWhBytes      = 4;
      P092_DataSettings.IdxCRC        = P092_DataSettings.DataBytes - 1;

      break;
    case 6133: // UVR 61-3 (from V8.3)
      P092_DataSettings.DataBytes = 62;

      P092_DataSettings.DeviceByte0   = 0x90;
      P092_DataSettings.DeviceByte1   = 0x9F;
      iDontCareBytes                  = 1;
      iTimeStampBytes                 = 5;
      P092_DataSettings.MaxSensors    = 6;
      P092_DataSettings.MaxExtSensors = 9;
      P092_DataSettings.MaxAnalogOuts = 2;
      P092_DataSettings.MaxHeatMeters = 3;
      P092_DataSettings.IdxCRC        = P092_DataSettings.DataBytes - 1;

      break;
  }
  P092_DataSettings.IdxSensor     = iDeviceBytes + iDontCareBytes + iTimeStampBytes;
  P092_DataSettings.IdxExtSensor  = P092_DataSettings.IdxSensor + 2 * P092_DataSettings.MaxSensors;
  P092_DataSettings.IdxOutput     = P092_DataSettings.IdxExtSensor + 2 * P092_DataSettings.MaxExtSensors;
  P092_DataSettings.IdxDrehzahl   = P092_DataSettings.IdxOutput + P092_DataSettings.OutputBytes;
  P092_DataSettings.IdxAnalog     = P092_DataSettings.IdxDrehzahl + P092_DataSettings.SpeedBytes;
  P092_DataSettings.IdxHmRegister = P092_DataSettings.IdxAnalog + (P092_DataSettings.AnalogBytes * P092_DataSettings.MaxAnalogOuts);
  P092_DataSettings.IdxVolume     = P092_DataSettings.IdxHmRegister + 1;
  P092_DataSettings.IdxHeatMeter1 = P092_DataSettings.IdxVolume + P092_DataSettings.VolumeBytes;
  P092_DataSettings.IdxkWh1       = P092_DataSettings.IdxHeatMeter1 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh1       = P092_DataSettings.IdxkWh1 + 2;
  P092_DataSettings.IdxHeatMeter2 = P092_DataSettings.IdxMWh1 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh2       = P092_DataSettings.IdxHeatMeter2 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh2       = P092_DataSettings.IdxkWh2 + 2;
  P092_DataSettings.IdxHeatMeter3 = P092_DataSettings.IdxMWh2 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh3       = P092_DataSettings.IdxHeatMeter3 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh3       = P092_DataSettings.IdxkWh3 + 2;
}

/****************\
   DLBus P092_receiving
\****************/
void P092_data_struct::Plugin_092_StartReceiving(taskIndex_t taskindex) {
  DLbus_Data->ISR_Receiving   = false;
  DLbus_Data->DeviceBytes[0]  = P092_DataSettings.DeviceByte0;
  DLbus_Data->DeviceBytes[1]  = P092_DataSettings.DeviceByte1;
  DLbus_Data->ISR_PulseNumber =
    (((P092_DataSettings.DataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 +  DLbus_StopBits) + DLBus_SyncBits) *
     DLBus_BitChangeFactor) + DLBus_ReserveBytes;
  DLbus_Data->ISR_MinPulseWidth       = P092_DataSettings.DLbus_MinPulseWidth;
  DLbus_Data->ISR_MaxPulseWidth       = P092_DataSettings.DLbus_MaxPulseWidth;
  DLbus_Data->ISR_MinDoublePulseWidth = P092_DataSettings.DLbus_MinDoublePulseWidth;
  DLbus_Data->ISR_MaxDoublePulseWidth = P092_DataSettings.DLbus_MaxDoublePulseWidth;
  DLbus_Data->StartReceiving();
  uint32_t start = millis();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("P092_receiving ... TaskIndex:");
    log += taskindex;
    addLog(LOG_LEVEL_INFO, log);
  }

  while ((timePassedSince(start) < 100) && (DLbus_Data->ISR_PulseCount == 0)) {
    // wait for first pulse received (timeout 100ms)
    delay(0);
  }

  if (DLbus_Data->ISR_PulseCount == 0) {
    // nothing received
    DLbus_Data->ISR_Receiving = false;
    DLbus_Data->IsNoData = true;  // stop receiving until next PLUGIN_092_READ
    addLog(LOG_LEVEL_ERROR, F("## StartReceiving: Error: Nothing received! No DL bus connected!"));
  }
}

/****************\
   DLBus get data
\****************/
boolean P092_data_struct::P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData *ReadData) {
  String  log;
  boolean result = false;

  switch (OptionIdx) {
    case 1: // F("Sensor")
      log  = F("Get Sensor");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxSensors) {
        result = false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxSensor;
      result        = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 2: // F("Sensor")
      log  = F("Get ExtSensor");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxExtSensors) {
        result = false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxExtSensor;
      result        = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 3: // F("Digital output")
      log  = F("Get DigitalOutput");
      log += CurIdx;

      if (CurIdx > (8 * P092_DataSettings.OutputBytes)) {
        result = false;
        break;
      }
      result = P092_fetch_output(CurIdx, ReadData);
      break;
    case 4: // F("Speed step")
      log  = F("Get SpeedStep");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.SpeedBytes) {
        result = false;
        break;
      }
      result = P092_fetch_speed(CurIdx, ReadData);
      break;
    case 5: // F("Analog output")
      log  = F("Get AnalogOutput");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.AnalogBytes) {
        result = false;
        break;
      }
      result = P092_fetch_analog(CurIdx, ReadData);
      break;
    case 6: // F("Heat power (kW)")
      log  = F("Get HeatPower");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result = false;
        break;
      }
      result = P092_fetch_heatpower(CurIdx, ReadData);
      break;
    case 7: // F("Heat meter (MWh)"
      log  = F("Get HeatMeter");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result = false;
        break;
      }
      result = P092_fetch_heatmeter(CurIdx, ReadData);
      break;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log += F(": ");

    if (result) {
      log += String(ReadData->value, 1);
    }
    else {
      log += F("nan");
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  return result;
}

boolean P092_data_struct::P092_fetch_sensor(int number, sP092_ReadData *ReadData) {
  float value = 0.0f;

  ReadData->mode = -1;
  number         = ReadData->Idx + (number - 1) * 2;
  int32_t sensorvalue = (DLbus_Data->ByteStream[number + 1] << 8) | DLbus_Data->ByteStream[number];

  if (sensorvalue == 0) {
    return false;
  }
  uint8_t sensortype = (sensorvalue & 0x7000) >> 12;

  if (!(sensorvalue & 0x8000)) { // sign positive
    sensorvalue &= 0xfff;

    // calculations for different sensor types
    switch (sensortype) {
      case DLbus_Sensor_DIGITAL:
        value = false;
        break;
      case DLbus_Sensor_TEMP:
        value = static_cast<float>(sensorvalue) * 0.1f;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = sensorvalue * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
        value          = static_cast<float>(sensorvalue & 0x1ff) * 0.1f;
        break;
      default:
        return false;
    }
  }
  else { // sign negative
    sensorvalue |= 0xf000;

    // calculations for different sensor types
    switch (sensortype) {
      case DLbus_Sensor_DIGITAL:
        value = true;
        break;
      case DLbus_Sensor_TEMP:
        value = static_cast<float>(sensorvalue - 0x10000) * 0.1f;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue - 0x10000;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = (sensorvalue - 0x10000) * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
        value          = static_cast<float>((sensorvalue & 0x1ff) - 0x10000) * 0.1f;
        break;
      default:
        return false;
    }
  }
  ReadData->value = value;
  return true;
}

boolean P092_data_struct::P092_fetch_output(int number, sP092_ReadData *ReadData) {
  int32_t outputs;

  if (P092_DataSettings.OutputBytes > 1) {
    outputs = (DLbus_Data->ByteStream[P092_DataSettings.IdxOutput + 1] << 8) | DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
  }
  else {
    outputs = DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
  }

  if (outputs & (1 << (number - 1))) {
    ReadData->value = 1;
  }
  else {
    ReadData->value = 0;
  }
  return true;
}

boolean P092_data_struct::P092_fetch_speed(int number, sP092_ReadData *ReadData) {
  uint8_t speedbyte;

  if ((P092_DataSettings.IdxDrehzahl + (number - 1)) >= P092_DataSettings.IdxAnalog) {
    // wrong index for speed, overlapping next index (IdxAnalog)
    return false;
  }
  speedbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxDrehzahl + (number - 1)];

  if (speedbyte & 0x80) {
    return false;
  }
  ReadData->value = (speedbyte & 0x1f);
  return true;
}

boolean P092_data_struct::P092_fetch_analog(int number, sP092_ReadData *ReadData) {
  uint8_t analogbyte;

  if ((P092_DataSettings.IdxAnalog + (number - 1)) >= P092_DataSettings.IdxHmRegister) {
    // wrong index for analog, overlapping next index (IdxHmRegister)
    return false;
  }
  analogbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxAnalog + (number - 1)];

  if (analogbyte & 0x80) {
    return false;
  }
  ReadData->value = (analogbyte * 0.1);
  return true;
}

P092_data_struct::sDLbus_HMindex P092_data_struct::P092_CheckHmRegister(int number) {
  sDLbus_HMindex result;

  result.IndexIsValid = 0;

  switch (number) {
    case 1:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x1) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter1;
      result.kwh_index   = P092_DataSettings.IdxkWh1;
      result.mwh_index   = P092_DataSettings.IdxMWh1;
      break;
    case 2:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x2) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter2;
      result.kwh_index   = P092_DataSettings.IdxkWh2;
      result.mwh_index   = P092_DataSettings.IdxMWh2;
      break;
    case 3:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x4) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter3;
      result.kwh_index   = P092_DataSettings.IdxkWh3;
      result.mwh_index   = P092_DataSettings.IdxMWh3;
      break;
    default:
      return result;
  }
  result.IndexIsValid = 1;
  return result;
}

boolean P092_data_struct::P092_fetch_heatpower(int number, sP092_ReadData *ReadData) {
  // current power
  int32_t high;
  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);

  if (HMindex.IndexIsValid == 0) {
    return false;
  }
  uint8_t b1 = DLbus_Data->ByteStream[HMindex.power_index];
  uint8_t b2 = DLbus_Data->ByteStream[HMindex.power_index + 1];

  if (P092_DataSettings.CurrentHmBytes > 2) {
    uint8_t b3 = DLbus_Data->ByteStream[HMindex.power_index + 2];
    uint8_t b4 = DLbus_Data->ByteStream[HMindex.power_index + 3];
    high = 0x10000 * b4 + 0x100 * b3 + b2;
    int low = (b1 * 10) / 0x100;

    if (!(b4 & 0x80)) { // sign positive
      ReadData->value = static_cast<float>(10 * high + low) / 100.0f;
    }
    else {              // sign negative
      ReadData->value = static_cast<float>(10 * (high - 0x10000) - low) / 100.0f;
    }
  }
  else {
    high = (b2 << 8) | b1;

    if ((b2 & 0x80) == 0) { // sign positive
      ReadData->value = static_cast<float>(high) / 10.0f;
    }
    else {                  // sign negative
      ReadData->value = static_cast<float>(high - 0x10000) / 10.0f;
    }
  }
  return true;
}

boolean P092_data_struct::P092_fetch_heatmeter(int number, sP092_ReadData *ReadData) {
  // heat meter
  int32_t heat_meter;
  float   heat_meter_mwh;

  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);

  if (HMindex.IndexIsValid == 0) {
    return false;
  }
  heat_meter     = (DLbus_Data->ByteStream[HMindex.kwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.kwh_index];
  heat_meter_mwh = (heat_meter * 0.1f) / 1000.0f; // in MWh

  if (heat_meter_mwh > 1.0f) {
    // in kWh
    heat_meter      = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }

  // MWh
  heat_meter      = (DLbus_Data->ByteStream[HMindex.mwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.mwh_index];
  ReadData->value = heat_meter_mwh + heat_meter;
  return true;
}

#endif // ifdef USES_P092
#include "../PluginStructs/P006_data_struct.h"

#ifdef USES_P006


# define BMP085_I2CADDR           0x77
# define BMP085_CAL_AC1           0xAA // R   Calibration data (16 bits)
# define BMP085_CAL_AC2           0xAC // R   Calibration data (16 bits)
# define BMP085_CAL_AC3           0xAE // R   Calibration data (16 bits)
# define BMP085_CAL_AC4           0xB0 // R   Calibration data (16 bits)
# define BMP085_CAL_AC5           0xB2 // R   Calibration data (16 bits)
# define BMP085_CAL_AC6           0xB4 // R   Calibration data (16 bits)
# define BMP085_CAL_B1            0xB6 // R   Calibration data (16 bits)
# define BMP085_CAL_B2            0xB8 // R   Calibration data (16 bits)
# define BMP085_CAL_MB            0xBA // R   Calibration data (16 bits)
# define BMP085_CAL_MC            0xBC // R   Calibration data (16 bits)
# define BMP085_CAL_MD            0xBE // R   Calibration data (16 bits)
# define BMP085_CONTROL           0xF4
# define BMP085_TEMPDATA          0xF6
# define BMP085_PRESSUREDATA      0xF6
# define BMP085_READTEMPCMD       0x2E
# define BMP085_READPRESSURECMD   0x34


bool P006_data_struct::begin()
{
  if (!initialized) {
    if (I2C_read8_reg(BMP085_I2CADDR, 0xD0) != 0x55) { return false; }

    /* read calibration data */
    ac1 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC1);
    ac2 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC2);
    ac3 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC3);
    ac4 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC4);
    ac5 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC5);
    ac6 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_AC6);

    b1 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_B1);
    b2 = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_B2);

    mb = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_MB);
    mc = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_MC);
    md = I2C_read16_reg(BMP085_I2CADDR, BMP085_CAL_MD);

    initialized = true;
  }

  return true;
}

uint16_t P006_data_struct::readRawTemperature(void)
{
  I2C_write8_reg(BMP085_I2CADDR, BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
  return I2C_read16_reg(BMP085_I2CADDR, BMP085_TEMPDATA);
}

uint32_t P006_data_struct::readRawPressure(void)
{
  uint32_t raw;

  I2C_write8_reg(BMP085_I2CADDR, BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  delay(26);

  raw   = I2C_read16_reg(BMP085_I2CADDR, BMP085_PRESSUREDATA);
  raw <<= 8;
  raw  |= I2C_read8_reg(BMP085_I2CADDR, BMP085_PRESSUREDATA + 2);
  raw >>= (8 - oversampling);

  return raw;
}

int32_t P006_data_struct::readPressure(void)
{
  int32_t  UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = readRawTemperature();
  UP = readRawPressure();

  // do temperature calculations
  X1 = (UT - (int32_t)(ac6)) * ((int32_t)(ac5)) / pow(2, 15);
  X2 = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5 = X1 + X2;

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> oversampling);

  if (B7 < 0x80000000)
  {
    p = (B7 * 2) / B4;
  }
  else
  {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
  return p;
}

float P006_data_struct::readTemperature(void)
{
  int32_t UT, X1, X2, B5; // following ds convention
  float   temp;

  UT = readRawTemperature();

  // step 1
  X1    = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2, 15);
  X2    = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5    = X1 + X2;
  temp  = (B5 + 8) / pow(2, 4);
  temp /= 10;

  return temp;
}

#endif // ifdef USES_P006

#include "../PluginStructs/P062_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
# include <Adafruit_MPR121.h>

#ifdef USES_P062
#include "../Helpers/ESPEasy_Storage.h"

P062_data_struct::P062_data_struct() {
#ifdef PLUGIN_062_DEBUG
  addLog(LOG_LEVEL_INFO, F("P062_data_struct constructor"));
#endif
  clearCalibrationData(); // Reset
}

P062_data_struct::~P062_data_struct() {
  if (keypad != nullptr) {
    delete keypad;
    keypad = nullptr;
  }
}

bool P062_data_struct::init(taskIndex_t taskIndex,
                            uint8_t i2c_addr,
                            bool scancode,
                            bool keepCalibrationData) {
#ifdef PLUGIN_062_DEBUG
  addLog(LOG_LEVEL_INFO, F("P062_data_struct init()"));
#endif
  _i2c_addr            = i2c_addr;
  _use_scancode        = scancode;
  _keepCalibrationData = keepCalibrationData;

  if (!keypad) {
    keypad = new (std::nothrow) Adafruit_MPR121();
  }
  if (keypad) {
    keypad->begin(_i2c_addr);
    loadTouchObjects(taskIndex);
    return true;
  }
  return false;
}

void P062_data_struct::updateCalibration(uint8_t t) {
  if (t >= P062_MaxTouchObjects) return;
  if (_keepCalibrationData) {
    uint16_t current = keypad->filteredData(t);
    CalibrationData.CalibrationValues[t].current = current;
    if (CalibrationData.CalibrationValues[t].min == 0 || current < CalibrationData.CalibrationValues[t].min) {
      CalibrationData.CalibrationValues[t].min = current;
    }
    if (CalibrationData.CalibrationValues[t].max == 0 || current > CalibrationData.CalibrationValues[t].max) {
      CalibrationData.CalibrationValues[t].max = current;
    }
  }
}

bool P062_data_struct::readKey(uint16_t& key) {
  if (!keypad) return false;
  key = keypad->touched();

  if (key)
  {
    uint16_t colMask = 0x01;

    for (uint8_t col = 1; col <= 12; col++)
    {
      if (key & colMask) // this key pressed?
      {
        updateCalibration(col - 1);
        if (_use_scancode) {
          key = col;
          break;
        }
      }
      colMask <<= 1;
    }
  }

  if (keyLast != key)
  {
    keyLast = key;
    return true;
  }
  return false;
}

/**
 * Set all tresholds at once
 */
void P062_data_struct::setThresholds(uint8_t touch, uint8_t release) {
  keypad->setThresholds(touch, release);
}

/**
 * Set a single treshold
 */
void P062_data_struct::setThreshold(uint8_t t, uint8_t touch, uint8_t release) {
  keypad->setThreshold(t, touch, release);
}

/**
 * Load the touch objects from the settings, and initialize then properly where needed.
 */
void P062_data_struct::loadTouchObjects(taskIndex_t taskIndex) {
#ifdef PLUGIN_062_DEBUG
  String log = F("P062 DEBUG loadTouchObjects size: ");
  log += sizeof(StoredSettings);
  addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_062_DEBUG
  LoadCustomTaskSettings(taskIndex, reinterpret_cast<uint8_t *>(&StoredSettings), sizeof(StoredSettings));
}

/**
 * Get the Calibration data for 1 touch object, return false if all zeroes or invalid input for t.
 */
bool P062_data_struct::getCalibrationData(uint8_t t, uint16_t *current, uint16_t *min, uint16_t *max) {
  if (t >= P062_MaxTouchObjects) return false;
  *current = CalibrationData.CalibrationValues[t].current;
  *min     = CalibrationData.CalibrationValues[t].min;
  *max     = CalibrationData.CalibrationValues[t].max;
  return (*current + *min + *max) > 0;
}

/**
 * Reset the touch data.
 */
void P062_data_struct::clearCalibrationData() {
  for (uint8_t t = 0; t < P062_MaxTouchObjects; t++) {
    CalibrationData.CalibrationValues[t].current = 0;
    CalibrationData.CalibrationValues[t].min     = 0;
    CalibrationData.CalibrationValues[t].max     = 0;
  }
}
#endif // ifdef USES_P062

#include "../PluginStructs/P094_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter
#include <ESPeasySerial.h>

#ifdef USES_P094
#include <Regexp.h>

#include "../Globals/ESPEasy_time.h"
#include "../Helpers/StringConverter.h"


P094_data_struct::P094_data_struct() :  easySerial(nullptr) {}

P094_data_struct::~P094_data_struct() {
  reset();
}

void P094_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P094_data_struct::init(ESPEasySerialPort port, 
                            const int16_t serial_rx, 
                            const int16_t serial_tx, 
                            unsigned long baudrate) {
  if ((serial_rx < 0) && (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (isInitialized()) {
    easySerial->begin(baudrate);
    return true;
  }
  return false;
}

void P094_data_struct::post_init() {
  for (uint8_t i = 0; i < P094_FILTER_VALUE_Type_NR_ELEMENTS; ++i) {
    valueType_used[i] = false;
  }

  for (uint8_t i = 0; i < P094_NR_FILTERS; ++i) {
    size_t lines_baseindex            = P094_Get_filter_base_index(i);
    int    index                      = _lines[lines_baseindex].toInt();
    int    tmp_filter_comp            = _lines[lines_baseindex + 2].toInt();
    const bool filter_string_notempty = _lines[lines_baseindex + 3].length() > 0;
    const bool valid_index            = index >= 0 && index < P094_FILTER_VALUE_Type_NR_ELEMENTS;
    const bool valid_filter_comp      = tmp_filter_comp >= 0 && tmp_filter_comp < P094_FILTER_COMP_NR_ELEMENTS;

    valueType_index[i] = P094_not_used;

    if (valid_index && valid_filter_comp && filter_string_notempty) {
      valueType_used[index] = true;
      valueType_index[i]    = static_cast<P094_Filter_Value_Type>(index);
      filter_comp[i]        = static_cast<P094_Filter_Comp>(tmp_filter_comp);
    }
  }
}

bool P094_data_struct::isInitialized() const {
  return easySerial != nullptr;
}

void P094_data_struct::sendString(const String& data) {
  if (isInitialized()) {
    if (data.length() > 0) {
      setDisableFilterWindowTimer();
      easySerial->write(data.c_str());

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Proxy: Sending: ");
        log += data;
        addLog(LOG_LEVEL_INFO, log);
      }
    }
  }
}

bool P094_data_struct::loop() {
  if (!isInitialized()) {
    return false;
  }
  bool fullSentenceReceived = false;

  if (easySerial != nullptr) {
    int available = easySerial->available();

    unsigned long timeout = millis() + 10;

    while (available > 0 && !fullSentenceReceived) {
      // Look for end marker
      char c = easySerial->read();
      --available;

      if (available == 0) {
        if (!timeOutReached(timeout)) {
          available = easySerial->available();
        }
        delay(0);
      }

      switch (c) {
        case 13:
        {
          const size_t length = sentence_part.length();
          bool valid          = length > 0;

          for (size_t i = 0; i < length && valid; ++i) {
            if ((sentence_part[i] > 127) || (sentence_part[i] < 32)) {
              sentence_part = "";
              ++sentences_received_error;
              valid = false;
            }
          }

          if (valid) {
            fullSentenceReceived = true;
          }
          break;
        }
        case 10:

          // Ignore LF
          break;
        default:
          sentence_part += c;
          break;
      }

      if (max_length_reached()) { fullSentenceReceived = true; }
    }
  }

  if (fullSentenceReceived) {
    ++sentences_received;
    length_last_received = sentence_part.length();
  }
  return fullSentenceReceived;
}

const String& P094_data_struct::peekSentence() const {
  return sentence_part;
}

void P094_data_struct::getSentence(String& string, bool appendSysTime) {
  string = std::move(sentence_part);
  sentence_part = ""; // FIXME TD-er: Should not be needed as move already cleared it.
  if (appendSysTime) {
    // Unix timestamp = 10 decimals + separator
    if (string.reserve(sentence_part.length() + 11)) {
      string += ';';
      string += node_time.getUnixTime();
    }
  }
  sentence_part.reserve(string.length());
}

void P094_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const {
  succes      = sentences_received;
  error       = sentences_received_error;
  length_last = length_last_received;
}

void P094_data_struct::setMaxLength(uint16_t maxlenght) {
  max_length = maxlenght;
}

void P094_data_struct::setLine(uint8_t varNr, const String& line) {
  if (varNr < P94_Nlines) {
    _lines[varNr] = line;
  }
}

uint32_t P094_data_struct::getFilterOffWindowTime() const {
  return _lines[P094_FILTER_OFF_WINDOW_POS].toInt();
}

P094_Match_Type P094_data_struct::getMatchType() const {
  return static_cast<P094_Match_Type>(_lines[P094_MATCH_TYPE_POS].toInt());
}

bool P094_data_struct::invertMatch() const {
  switch (getMatchType()) {
    case P094_Regular_Match:
      break;
    case P094_Regular_Match_inverted:
      return true;
    case P094_Filter_Disabled:
      break;
  }
  return false;
}

bool P094_data_struct::filterUsed(uint8_t lineNr) const
{
  if (valueType_index[lineNr] == P094_Filter_Value_Type::P094_not_used) { return false; }
  uint8_t varNr = P094_Get_filter_base_index(lineNr);
  return _lines[varNr + 3].length() > 0;
}

String P094_data_struct::getFilter(uint8_t lineNr, P094_Filter_Value_Type& filterValueType, uint32_t& optional,
                                   P094_Filter_Comp& comparator) const
{
  uint8_t varNr = P094_Get_filter_base_index(lineNr);

  filterValueType = P094_Filter_Value_Type::P094_not_used;

  if ((varNr + 3) >= P94_Nlines) { return ""; }
  optional        = _lines[varNr + 1].toInt();
  filterValueType = valueType_index[lineNr];
  comparator      = filter_comp[lineNr];

  //  filterValueType = static_cast<P094_Filter_Value_Type>(_lines[varNr].toInt());
  //  comparator      = static_cast<P094_Filter_Comp>(_lines[varNr + 2].toInt());
  return _lines[varNr + 3];
}

void P094_data_struct::setDisableFilterWindowTimer() {
  if (getFilterOffWindowTime() == 0) {
    disable_filter_window = 0;
  }
  else {
    disable_filter_window = millis() + getFilterOffWindowTime();
  }
}

bool P094_data_struct::disableFilterWindowActive() const {
  if (disable_filter_window != 0) {
    if (!timeOutReached(disable_filter_window)) {
      // We're still in the window where filtering is disabled
      return true;
    }
  }
  return false;
}

bool P094_data_struct::parsePacket(const String& received) const {
  size_t strlength = received.length();

  if (strlength == 0) {
    return false;
  }


  if (getMatchType() == P094_Filter_Disabled) {
    return true;
  }

  bool match_result = false;

  // FIXME TD-er: For now added '$' to test with GPS.
  if ((received[0] == 'b') || (received[0] == '$')) {
    // Received a data packet in CUL format.
    if (strlength < 21) {
      return false;
    }

    // Decoded packet

    unsigned long packet_header[P094_FILTER_VALUE_Type_NR_ELEMENTS];
    packet_header[P094_packet_length] = hexToUL(received, 1, 2);
    packet_header[P094_unknown1]      = hexToUL(received, 3, 2);
    packet_header[P094_manufacturer]  = hexToUL(received, 5, 4);
    packet_header[P094_serial_number] = hexToUL(received, 9, 8);
    packet_header[P094_unknown2]      = hexToUL(received, 17, 2);
    packet_header[P094_meter_type]    = hexToUL(received, 19, 2);

    // FIXME TD-er: Is this also correct?
    packet_header[P094_rssi] = hexToUL(received, strlength - 4, 4);

    // FIXME TD-er: Is this correct?
    // match_result = packet_length == (strlength - 21) / 2;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(128);
      log  = F("CUL Reader: ");
      log += F(" length: ");
      log += packet_header[P094_packet_length];
      log += F(" (header: ");
      log += strlength - (packet_header[P094_packet_length] * 2);
      log += F(") manu: ");
      log += formatToHex_decimal(packet_header[P094_manufacturer]);
      log += F(" serial: ");
      log += formatToHex_decimal(packet_header[P094_serial_number]);
      log += F(" mType: ");
      log += formatToHex_decimal(packet_header[P094_meter_type]);
      log += F(" RSSI: ");
      log += formatToHex_decimal(packet_header[P094_rssi]);
      addLog(LOG_LEVEL_INFO, log);
    }

    bool filter_matches[P094_NR_FILTERS];

    for (unsigned int f = 0; f < P094_NR_FILTERS; ++f) {
      filter_matches[f] = false;
    }

    // Do not check for "not used" (0)
    for (unsigned int i = 1; i < P094_FILTER_VALUE_Type_NR_ELEMENTS; ++i) {
      if (valueType_used[i]) {
        for (unsigned int f = 0; f < P094_NR_FILTERS; ++f) {
          if (valueType_index[f] == i) {
            // Have a matching filter

            uint32_t optional;
            P094_Filter_Value_Type filterValueType;
            P094_Filter_Comp comparator;
            bool   match = false;
            String inputString;
            String valueString;

            if (i == P094_Filter_Value_Type::P094_position) {
              valueString = getFilter(f, filterValueType, optional, comparator);

              if (received.length() >= (optional + valueString.length())) {
                // received string is long enough to fit the expression.
                inputString = received.substring(optional, optional + valueString.length());
                match = inputString.equalsIgnoreCase(valueString);
              }
            } else {
              unsigned long value = hexToUL(getFilter(f, filterValueType, optional, comparator));
              match       = (value == packet_header[i]);
              inputString = formatToHex_decimal(packet_header[i]);
              valueString = formatToHex_decimal(value);
            }


            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log;
              log.reserve(64);
              log  = F("CUL Reader: ");
              log += P094_FilterValueType_toString(valueType_index[f]);
              log += F(":  in:");
              log += inputString;
              log += ' ';
              log += P094_FilterComp_toString(comparator);
              log += ' ';
              log += valueString;

              switch (comparator) {
                case P094_Filter_Comp::P094_Equal_OR:
                case P094_Filter_Comp::P094_Equal_MUST:

                  if (match) { log += F(" expected MATCH"); } 
                  break;
                case P094_Filter_Comp::P094_NotEqual_OR:
                case P094_Filter_Comp::P094_NotEqual_MUST:

                  if (!match) { log += F(" expected NO MATCH"); }
                  break;
              }
              addLog(LOG_LEVEL_INFO, log);
            }

            switch (comparator) {
              case P094_Filter_Comp::P094_Equal_OR:

                if (match) { filter_matches[f] = true; }
                break;
              case P094_Filter_Comp::P094_NotEqual_OR:

                if (!match) { filter_matches[f] = true; }
                break;

              case P094_Filter_Comp::P094_Equal_MUST:

                if (!match) { return false; }
                break;

              case P094_Filter_Comp::P094_NotEqual_MUST:

                if (match) { return false; }
                break;
            }
          }
        }
      }
    }

    // Now we have to check if all rows per filter line in filter_matches[f] are true or not used.
    int nrMatches = 0;
    int nrNotUsed = 0;

    for (unsigned int f = 0; !match_result && f < P094_NR_FILTERS; ++f) {
      if (f % P094_AND_FILTER_BLOCK == 0) {
        if ((nrMatches > 0) && ((nrMatches + nrNotUsed) == P094_AND_FILTER_BLOCK)) {
          match_result = true;
        }
        nrMatches = 0;
        nrNotUsed = 0;
      }

      if (filter_matches[f]) {
        ++nrMatches;
      } else {
        if (!filterUsed(f)) {
          ++nrNotUsed;
        }
      }
    }
  } else {
    switch (received[0]) {
      case 'C': // CMODE
      case 'S': // SMODE
      case 'T': // TMODE
      case 'O': // OFF
      case 'V': // Version info

        // FIXME TD-er: Must test the result of the other possible answers.
        match_result = true;
        break;
    }
  }

  return match_result;
}

const __FlashStringHelper * P094_data_struct::MatchType_toString(P094_Match_Type matchType) {
  switch (matchType)
  {
    case P094_Match_Type::P094_Regular_Match:          return F("Regular Match");
    case P094_Match_Type::P094_Regular_Match_inverted: return F("Regular Match inverted");
    case P094_Match_Type::P094_Filter_Disabled:        return F("Filter Disabled");
  }
  return F("");
}

const __FlashStringHelper * P094_data_struct::P094_FilterValueType_toString(P094_Filter_Value_Type valueType)
{
  switch (valueType) {
    case P094_Filter_Value_Type::P094_not_used:      return F("---");
    case P094_Filter_Value_Type::P094_packet_length: return F("Packet Length");
    case P094_Filter_Value_Type::P094_unknown1:      return F("unknown1");
    case P094_Filter_Value_Type::P094_manufacturer:  return F("Manufacturer");
    case P094_Filter_Value_Type::P094_serial_number: return F("Serial Number");
    case P094_Filter_Value_Type::P094_unknown2:      return F("unknown2");
    case P094_Filter_Value_Type::P094_meter_type:    return F("Meter Type");
    case P094_Filter_Value_Type::P094_rssi:          return F("RSSI");
    case P094_Filter_Value_Type::P094_position:      return F("Position");

      //    default: break;
  }
  return F("unknown");
}

const __FlashStringHelper * P094_data_struct::P094_FilterComp_toString(P094_Filter_Comp comparator)
{
  switch (comparator) {
    case P094_Filter_Comp::P094_Equal_OR:      return F("==");
    case P094_Filter_Comp::P094_NotEqual_OR:   return F("!=");
    case P094_Filter_Comp::P094_Equal_MUST:    return F("== (must)");
    case P094_Filter_Comp::P094_NotEqual_MUST: return F("!= (must)");
  }
  return F("");
}

bool P094_data_struct::max_length_reached() const {
  if (max_length == 0) { return false; }
  return sentence_part.length() >= max_length;
}

size_t P094_data_struct::P094_Get_filter_base_index(size_t filterLine) {
  return filterLine * P094_ITEMS_PER_FILTER + P094_FIRST_FILTER_POS;
}

uint32_t P094_data_struct::getDebugCounter() {
  return debug_counter++;
}

#endif // USES_P094
