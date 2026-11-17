/*
The following code snippets are a draft C++ Proof Of Concept (PoC) LVGL library abstraction layer
demonstrating how The AlphaMon Platform can load, parse and interpret LVGL drawing commands from
a *.plg file stored on the M5Stack Tab5 microSD card.

If one or more plugin files are detected and successfully parsed at boot-time the AlphaMon will
automatically assign the plugin a button on the main Dashboard. Tapping on the button clears the
Tab5 screen, creates a Status Bar at the top (which includes an Exit button) and paints the glass
using abstracted LVGL API calls as documented in the code below.

This PoC has been prepared to demonstrate the proposed architecture of an AlphaMon-compatible plugin.
The intention is that, if the PoC proves to be workable, the list of supported LVGL primatives can be
expanded by modifying the code below to support more complex drawings.

A key feature of this PoC is its integgration with the AlphaMon's native, real time Expression Evaluator (EE)
which allows the computed result of one or more of the Expressions contained in the user's Configuration File
(AlphaMon.cfg) to be integrated into the plugin's output.  Thus, for example, a plugin can be designed to
display your inverter's output voltage and current simply by referencing the Alias names of those Modbus
registers in two of the Config File expressions.

*/


extern std::vector<ExpnInfo_t> vgExpnInfo; // Vector Array to store final expressions
uint16_t EE_ExecOp(RpnBinStack_t &sParsedStep);
eErrorCode EE_Evaluate(const ExpnInfo_t& sExpnInfo, EvalResult_t* spFinalResult);
void EE_Execute(ExpnInfo_t& sExpnInfo, EvalResult_t* spFinalResult);
char* Expn_FormatResult(const EvalResult_t &sResult, char* cSmallBuff);
double EE_Recurse(uint8_t ui8Token, double dPreviousResult, uint8_t ui8RequestType);
bool EE_IsFunction(eTokenOp eTokenVal);
bool EE_IsOperator(const RpnBinStack_t* spStep);
void EE_PrintInfo(const ExpnInfo_t& sExpnInfo);
void EE_Cleanup();

//Metadata for a compiled EE expression
// NOTE: The Hi Bit of ui8EDT is used to signal the last value on the RPN stack. It must
// be cleared to reveal the underlying data type of the result.
// If ui8EDT==0 it indicates an eval-only expression; LHS is absent, so no assignment step is reqd.
struct ExpnInfo_t
  {
  uint8_t   ui8Idx = 0; //Expn Idx (0=sentinel)
  uint8_t   ui8RpnSize = 0; //RPN stack size
  RpnBinStack_t* spStackPtr = nullptr; //Pointer to the base of the RPN stack
  uint8_t   ui8ConfigIdx = 0;      //Index of source expn text in cConfigValue[]
  uint8_t   ui8EDT = 0;            //LHS Expn Data Type (EDT)
  uint8_t   ui8Decimals = 2;       //Default to 2 decimal places
  uint8_t   ui8GpioPin = 0;        //The output pin no (from compiler)
  uint32_t  ui32Lockout_ms = 0;    //The lockout period (ms) after the last change (hysterisis)
  uint8_t   ui8TT = 0;  //LHS Token Type
  uint32_t  ui32TM = 0; //LHS Token Main
  uint16_t  ui16TS = 0; //LHS Token Sub
  //17Mar2026 - added execution time varables. These are flagged "mutable" because they get
  //updated, even though the other fields are const.
  mutable bool      bIsValid = false;      //Set false if execution fails
  mutable uint16_t  ui16FailLine = 0;      //Store the failure line# here for debugging
  mutable uint32_t  ui32LastExec_us = 0;   //The execution time (duration) in us of the most recent evaluation
  mutable uint64_t  ui64TotalExec_us = 0;  //The cum exec time (for calculating avg. exec time)
  mutable uint32_t  ui32ExecCount = 0;     //No. of executions since boot-up
  mutable uint32_t  ui32LastChange_ms = 0; //The millis() when the output was last updated
  };
  


/**
 * @brief Scans SD:/PLUGIN01.plg .. PLUGIN04.plg using AlphaMon's File_Check/File_Open helpers.
 */
void Plugin_Scan()
  {
  if (!bgSdCardFound)
    return;

  char cPath[32];
  for (uint8_t i = 0; i < PLUGIN_SLOT_COUNT; i++)
    {
    bPluginExists[i] = false;
    snprintf(cPluginTitles[i], sizeof(cPluginTitles[i]), "PLUGIN%02u", i + 1);
    snprintf(cPath, sizeof(cPath), "/PLUGIN%02u.plg", i + 1);

    if (File_Check(cPath, true))
      {
      bPluginExists[i] = true;
      File f = File_Open(__func__, cPath, (char*)FILE_READ, true);
      if (f)
        {
        while (f.available())
          {
          String line = f.readStringUntil('\n');
          line.trim();
          if (line.length() == 0 || line.startsWith("//") || line.startsWith("#"))
            continue;
          if (line.startsWith("TITLE,"))
            {
            strncpy(cPluginTitles[i], line.substring(6).c_str(), sizeof(cPluginTitles[i]) - 1);
            cPluginTitles[i][sizeof(cPluginTitles[i]) - 1] = '\0';
            _plg_trim(cPluginTitles[i]);
            }
          break;
          }
        f.close();
        }
      }
    }
  }

/**
 * @brief Parses a .plg layout file from SD and instantiates its widgets onto oPluginCanvas.
 * @param ui8Slot 0-based plug-in slot index (0..3 -> PLUGIN01..PLUGIN04).
 * @return true if loaded successfully, false otherwise.
 */
bool Plugin_Load(uint8_t ui8Slot)
  {
  if (!bgSdCardFound || oPluginCanvas == nullptr || ui8Slot >= PLUGIN_SLOT_COUNT || !bPluginExists[ui8Slot])
    return false;

  char cPath[32];
  snprintf(cPath, sizeof(cPath), "/PLUGIN%02u.plg", ui8Slot + 1);
  File f = File_Open(__func__, cPath, (char*)FILE_READ, true);
  if (!f)
    return false;

  ui8PluginBindingCount = 0;
  bPluginActive         = true;

  char cLine[160];
  while (f.available())
    {
    size_t len = f.readBytesUntil('\n', cLine, sizeof(cLine) - 1);
    cLine[len] = '\0';
    _plg_trim(cLine);
    if (cLine[0] == '\0' || strncmp(cLine, "//", 2) == 0 || cLine[0] == '#' || strncmp(cLine, "TITLE", 5) == 0)
      continue;

    char* cpTok[9] = { nullptr };
    uint8_t ui8TokCnt = 0;
    char* p = strtok(cLine, ",");
    while (p != nullptr && ui8TokCnt < 9)
      {
      _plg_trim(p);
      cpTok[ui8TokCnt++] = p;
      p = strtok(nullptr, ",");
      }

    if (ui8TokCnt < 7)
      continue;

    const char* cpType = cpTok[0];
    int32_t x          = atoi(cpTok[1]);
    int32_t y          = atoi(cpTok[2]);
    int32_t w          = atoi(cpTok[3]);
    int32_t h          = atoi(cpTok[4]);
    uint32_t ui32Color = (uint32_t)strtoul(cpTok[5], nullptr, 0);
    const char* cpArg  = cpTok[6];

    if (strcasecmp(cpType, "BOX") == 0)
      {
      lv_obj_t* box = lv_obj_create(oPluginCanvas);
      lv_obj_set_pos(box, x, y);
      lv_obj_set_size(box, w, h);
      lv_obj_set_style_bg_color(box, lv_color_hex(ui32Color), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_radius(box, 14, LV_PART_MAIN);
      lv_obj_set_style_border_width(box, 0, LV_PART_MAIN);
      lv_obj_set_clickable(box, false);

      if (cpArg[0] != '\0')
        {
        lv_obj_t* hdr = lv_label_create(box);
        lv_label_set_text(hdr, cpArg);
        lv_obj_set_style_text_font(hdr, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(hdr, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, 16, 12);
        }
      }
    else if (strcasecmp(cpType, "BAR") == 0 && ui8PluginBindingCount < PLUGIN_MAX_BINDINGS)
      {
      lv_obj_t* bar = lv_bar_create(oPluginCanvas);
      lv_obj_set_pos(bar, x, y);
      lv_obj_set_size(bar, w, h);
      int32_t i32Min = (ui8TokCnt >= 8) ? atoi(cpTok[7]) : 0;
      int32_t i32Max = (ui8TokCnt >= 9) ? atoi(cpTok[8]) : 100;
      lv_bar_set_range(bar, i32Min, i32Max);
      lv_bar_set_value(bar, i32Min, LV_ANIM_OFF);
      lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
      lv_obj_set_style_bg_color(bar, lv_color_hex(ui32Color), LV_PART_INDICATOR);
      lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
      lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
      lv_obj_set_clickable(bar, false);

      PluginBinding_t& b = sPluginBindings[ui8PluginBindingCount++];
      b.pObj       = bar;
      b.eType      = PLG_W_BAR;
      b.ui8ExpnIdx = _plg_parseExpn(cpArg);
      b.i32Min     = i32Min;
      b.i32Max     = i32Max;
      b.cFormat[0] = '\0';
      }
    else if (strcasecmp(cpType, "VAL") == 0)
      {
      lv_obj_t* lbl = lv_label_create(oPluginCanvas);
      lv_obj_set_pos(lbl, x, y);
      lv_obj_set_width(lbl, w);
      lv_obj_set_style_text_font(lbl, &lv_font_montserrat_48, 0);
      lv_obj_set_style_text_color(lbl, lv_color_hex(ui32Color), 0);
      lv_label_set_text(lbl, cpArg);

      uint8_t ui8Expn = _plg_parseExpn(cpArg);
      if (ui8Expn > 0 && ui8PluginBindingCount < PLUGIN_MAX_BINDINGS)
        {
        PluginBinding_t& b = sPluginBindings[ui8PluginBindingCount++];
        b.pObj       = lbl;
        b.eType      = PLG_W_VAL;
        b.ui8ExpnIdx = ui8Expn;
        strncpy(b.cFormat, cpArg, sizeof(b.cFormat) - 1);
        b.cFormat[sizeof(b.cFormat) - 1] = '\0';
        }
      }
    }
  f.close();

  Plugin_Refresh();
  lv_obj_invalidate(oPluginCanvas);
  return true;
  }

/**
 * @brief Updates all active LVGL plug-in widgets directly via ExpnEval's existing API.
 */
void Plugin_Refresh()
  {
  if (!bPluginActive || currentView != VIEW_PLUGIN)
    return;

  char cValStr[SMALL_BUFF_MAX];
  char cDisplay[96];

  for (uint8_t i = 0; i < ui8PluginBindingCount; i++)
    {
    PluginBinding_t& b = sPluginBindings[i];
    if (b.pObj == nullptr || b.ui8ExpnIdx == 0 || b.ui8ExpnIdx >= vgExpnInfo.size())
      continue;

    ExpnInfo_t& sExpnInfo = vgExpnInfo[b.ui8ExpnIdx];
    if (!sExpnInfo.bIsValid || sExpnInfo.spStackPtr == nullptr)
      continue;

    EvalResult_t sResult = {};
    sResult.ui8EvalIdx = b.ui8ExpnIdx;
    if (EE_Evaluate(sExpnInfo, &sResult) != EXIT_OK)
      continue;

    if (b.eType == PLG_W_BAR)
      {
      if (Token2Num(sResult))
        {
        int32_t i32Clamped = constrain((int32_t)sResult.dNum, b.i32Min, b.i32Max);
        lv_bar_set_value(b.pObj, i32Clamped, LV_ANIM_OFF);
        }
      }
    else if (b.eType == PLG_W_VAL)
      {
      Expn_FormatResult(sResult, cValStr);
      const char* pOpen  = strstr(b.cFormat, "{EXPN:");
      const char* pClose = (pOpen != nullptr) ? strchr(pOpen, '}') : nullptr;
      if (pOpen != nullptr && pClose != nullptr)
        {
        size_t prefixLen = pOpen - b.cFormat;
        snprintf(cDisplay, sizeof(cDisplay), "%.*s%s%s", (int)prefixLen, b.cFormat, cValStr, pClose + 1);
        lv_label_set_text(b.pObj, cDisplay);
        }
      else
        {
        lv_label_set_text(b.pObj, cValStr);
        }
      }
    }
  }

// AlphaMon plugin source file code trimmer
static void _plg_trim(char* s)
  {
  if (s == nullptr) return;
  size_t len = strlen(s);
  while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n' || s[len - 1] == '"'))
    s[--len] = '\0';
  size_t start = 0;
  while (s[start] == ' ' || s[start] == '\t' || s[start] == '"')
    start++;
  if (start > 0)
    memmove(s, s + start, len - start + 1);
  return;
  }

// Plugin hook into AlphaMon's Expression Evaluator (EE)
static uint8_t _plg_parseExpn(const char* cpToken)
  {
  if (cpToken == nullptr) 
    return 0;
  const char* p = strstr(cpToken, "EXPN:");
  if (p == nullptr) 
    return 0;
  return (uint8_t)atoi(p + 5);
  }

// ===========================================================================
// W3C / Wikipedia Web Colors (Single Source of Truth via X-Macro)
// ===========================================================================
// Format: X(Name, Hex24)
#define WEB_COLOR_LIST(X) \
  X(White,          0xFFFFFF) \
  X(Silver,         0xC0C0C0) \
  X(Gray,           0x808080) \
  X(Black,          0x000000) \
  X(Red,            0xFF0000) \
  X(Maroon,         0x800000) \
  X(Yellow,         0xFFFF00) \
  X(Olive,          0x808000) \
  X(Lime,           0x00FF00) \
  X(Green,          0x008000) \
  X(Aqua,           0x00FFFF) \
  X(Cyan,           0x00FFFF) \
  X(Teal,           0x008080) \
  X(Blue,           0x0000FF) \
  X(Navy,           0x000080) \
  X(Fuchsia,        0xFF00FF) \
  X(Magenta,        0xFF00FF) \
  X(Purple,         0x800080) \
  X(Orange,         0xFFA500) \
  X(DarkOrange,     0xFF8C00) \
  X(MidnightBlue,   0x191970) \
  X(DarkGreen,      0x006400) \
  X(DarkSlateGray,  0x2F4F4F) \
  X(DarkCyan,       0x008B8B) \
  X(SlateGray,      0x708090) \
  X(LightSlateGray, 0x778899) \
  X(DimGray,        0x696969) \
  X(DarkGray,       0xA9A9A9) \
  X(LightGray,      0xD3D3D3) \
  X(Gold,           0xFFD700) \
  X(Crimson,        0xDC143C) \
  X(DodgerBlue,     0x1E90FF) \
  X(RoyalBlue,      0x4169E1) \
  X(SteelBlue,      0x4682B4) \
  X(TrackDarkGray,  0x333333) \
  X(BtnActiveTop,   0x546E7A) \
  X(BtnActiveBase,  0x263238) \
  X(BtnActiveEdge,  0x90A4AE) \
  X(BtnSpareBase,   0x141A1D) \
  X(BtnSpareEdge,   0x37474F)

// 1. Auto-generate COLOR_<Name> constants for C++ code
enum WebColorHex : uint32_t {
  #define X_COLOR_ENUM(name, hex) COLOR_##name = hex,
  WEB_COLOR_LIST(X_COLOR_ENUM)
  #undef X_COLOR_ENUM
};

// 2. Lookup table struct for JSON .plg colour names
struct WebColorEntry_t {
  const char* cpName;
  uint32_t    ui32Hex;
};

static const WebColorEntry_t sWebColors[] = {
  #define X_COLOR_ROW(name, hex) { #name, hex },
  WEB_COLOR_LIST(X_COLOR_ROW)
  #undef X_COLOR_ROW
};
static const uint16_t ui16WebColorCount = sizeof(sWebColors) / sizeof(sWebColors[0]);

// 3. Helper: Resolves either a Web Color name ("DodgerBlue") or hex ("0x1E90FF" / "#1E90FF")
static inline uint32_t Color_Parse(const char* cpColorStr)
  {
  if (cpColorStr == nullptr || cpColorStr[0] == '\0')
    return COLOR_White;
  if (cpColorStr[0] == '#')
    return (uint32_t)strtoul(cpColorStr + 1, nullptr, 16);
  if (strncmp(cpColorStr, "0x", 2) == 0 || strncmp(cpColorStr, "0X", 2) == 0)
    return (uint32_t)strtoul(cpColorStr, nullptr, 16);

  for (uint16_t i = 0; i < ui16WebColorCount; i++)
    {
    if (strcasecmp(cpColorStr, sWebColors[i].cpName) == 0)
      return sWebColors[i].ui32Hex;
    }
  return COLOR_White;
  }

