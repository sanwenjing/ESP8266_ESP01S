#include "I2Cdev.h"


I2Cdev i2cdev;

#include "../Globals/Statistics.h"

#include <Arduino.h>

#ifndef BUILD_NO_RAM_TRACKER
uint32_t lowestRAM = 0;
String   lowestRAMfunction;
uint32_t lowestFreeStack = 0;
String   lowestFreeStackfunction;
#endif

uint8_t lastBootCause                           = BOOT_CAUSE_MANUAL_REBOOT;
unsigned long lastMixedSchedulerId_beforereboot = 0;

unsigned long idle_msec_per_sec = 0;
unsigned long elapsed10ps       = 0;
unsigned long elapsed10psU      = 0;
unsigned long elapsed50ps       = 0;
unsigned long loopCounter       = 0;
unsigned long loopCounterLast   = 0;
unsigned long loopCounterMax    = 1;
unsigned long lastLoopStart     = 0;
unsigned long shortestLoop      = 10000000;
unsigned long longestLoop       = 0;
unsigned long loopCounter_full  = 1;
float loop_usec_duration_total  = 0.0f;


unsigned long dailyResetCounter                   = 0;
volatile unsigned long sw_watchdog_callback_count = 0;


I2C_bus_state I2C_state = I2C_bus_state::OK;
unsigned long I2C_bus_cleared_count = 0;

#include "../Globals/ESPEasy_Scheduler.h"

ESPEasy_Scheduler Scheduler;
#include "../Globals/Plugins_other.h"


 
 void (*parseTemplate_CallBack_ptr)(String& tmpString, bool useURLencode) = nullptr;
 void (*substitute_eventvalue_CallBack_ptr)(String& line, const String& event) = nullptr;
#include "../Globals/RTC.h"

#include "../DataStructs/RTCStruct.h"


RTCStruct RTC;


#include "../Globals/Device.h"

// TODO TD-er: Move often used functions like determine valueCount to here.

DeviceVector Device;
#include "../Globals/ESPEasyWiFi.h"

#ifdef ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS
ESPEasyWiFi_t ESPEasyWiFi;
#endif // ifdef ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS

#include "../Globals/Services.h"

#ifdef FEATURE_ARDUINO_OTA
  bool ArduinoOTAtriggered=false;
#endif


#ifdef ESP8266  
  ESP8266WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP8266HTTPUpdateServer httpUpdater(true);
  #endif
#endif

#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP32HTTPUpdateServer httpUpdater(true);
  #endif
#endif


#ifdef FEATURE_DNS_SERVER
  #include <DNSServer.h>
  DNSServer  dnsServer;
  bool dnsServerActive = false;
#endif

#include "../Globals/Cache.h"

#include "../DataStructs/Caches.h"

void clearAllCaches()
{
  Cache.clearAllCaches();
}

void updateTaskCaches()
{
  Cache.updateTaskCaches();
}

void updateActiveTaskUseSerial0()
{
  Cache.updateActiveTaskUseSerial0();
}

bool activeTaskUseSerial0()
{
  return Cache.activeTaskUseSerial0;
}

Caches Cache;

#include "../Globals/TXBuffer.h"

#include "../DataStructs/Web_StreamingBuffer.h"


Web_StreamingBuffer TXBuffer;
#include "../Globals/NetworkState.h"

#include "../../ESPEasy_common.h"


// Ethernet Connection status
NetworkMedium_t active_network_medium = DEFAULT_NETWORK_MEDIUM;

bool webserverRunning(false);
bool webserver_init(false);
bool mDNS_init(false);


// NTP status
bool statusNTPInitialized = false;


// Setup DNS, only used if the ESP has no valid WiFi config
const uint8_t DNS_PORT = 53;
IPAddress  apIP(DEFAULT_AP_IP);



// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;

#include "../Globals/Settings.h"


SettingsStruct Settings;
#include "../Globals/EventQueue.h"


EventQueueStruct eventQueue;
#include "../Globals/CRCValues.h"

CRCStruct CRCValues;
#include "../Globals/TimeZone.h"

ESPEasy_time_zone time_zone;
#include "../Globals/ESPEasy_time.h"

ESPEasy_time node_time;
#include "../Globals/Logging.h"

#include "../DataStructs/LogStruct.h"


LogStruct Logging;

uint8_t highest_active_log_level = 0;
bool log_to_serial_disabled = false;

std::deque<char> serialWriteBuffer;
#include "../Globals/NPlugins.h"

#include "../DataStructs/NotificationStruct.h"
#include "../Globals/Settings.h"


nprotocolIndex_t INVALID_NPROTOCOL_INDEX = NPLUGIN_MAX;
notifierIndex_t  INVALID_NOTIFIER_INDEX  = NOTIFICATION_MAX;
npluginID_t      INVALID_N_PLUGIN_ID     = 0;


boolean (*NPlugin_ptr[NPLUGIN_MAX])(NPlugin::Function,
                                    struct EventStruct *,
                                    String&);
npluginID_t NPlugin_id[NPLUGIN_MAX];

NotificationStruct Notification[NPLUGIN_MAX];

int notificationCount = -1;


bool validNProtocolIndex(nprotocolIndex_t index) {
  return index != INVALID_NPROTOCOL_INDEX;
}

bool validNotifierIndex(notifierIndex_t index)
{
  return index < NOTIFICATION_MAX;
}

bool validNPluginID(npluginID_t npluginID)
{
  if (npluginID == INVALID_N_PLUGIN_ID) {
    return false;
  }

  // FIXME TD-er: Must search to all included plugins
  return true;
}

String getNPluginNameFromNotifierIndex(notifierIndex_t NotifierIndex) {
  String name;

  if (validNPluginID(NPlugin_id[NotifierIndex])) {
    NPlugin_ptr[NotifierIndex](NPlugin::Function::NPLUGIN_GET_DEVICENAME, nullptr, name);
  }
  return name;
}

/********************************************************************************************\
   Get notificatoin protocol index (plugin index), by NPlugin_id
 \*********************************************************************************************/
nprotocolIndex_t getNProtocolIndex(npluginID_t Number)
{
  for (uint8_t x = 0; x <= notificationCount; x++) {
    if (Notification[x].Number == Number) {
      return x;
    }
  }
  return INVALID_NPROTOCOL_INDEX;
}

nprotocolIndex_t getNProtocolIndex_from_NotifierIndex(notifierIndex_t index) {
  if (validNotifierIndex(index)) {
    return getNProtocolIndex(Settings.Notification[index]);
  }
  return INVALID_NPROTOCOL_INDEX;
}

#include "../Globals/GlobalMapPortStatus.h"

MapPortStatus globalMapPortStatus;
#include "../Globals/RamTracker.h"


#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/Statistics.h"

#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"

#ifndef BUILD_NO_RAM_TRACKER
RamTracker myRamTracker;

/********************************************************************************************\
   Global convenience functions calling RamTracker
 \*********************************************************************************************/

void checkRAMtoLog(void){
  myRamTracker.getTraceBuffer();
}

void checkRAM(const String &flashString, int a ) {
  checkRAM(flashString, String(a));
}

void checkRAM(const String& flashString, const String &a ) {
  String s = flashString;
  s += " (";
  s += a;
  s += ')';
  checkRAM(s);
}

void checkRAM(const __FlashStringHelper * descr ) {
  checkRAM(String(descr));
}

void checkRAM(const String& descr ) {
  myRamTracker.registerRamState(descr);
  
  uint32_t freeRAM = FreeMem();
  if (freeRAM <= lowestRAM)
  {
    lowestRAM = freeRAM;
    lowestRAMfunction = descr;
  }
  uint32_t freeStack = getFreeStackWatermark();
  if (freeStack <= lowestFreeStack) {
    lowestFreeStack = freeStack;
    lowestFreeStackfunction = descr;
  }
}


/********************************************************************************************\
   RamTracker class
 \*********************************************************************************************/



// find highest the trace with the largest minimum memory (gets replaced by worse one)
unsigned int RamTracker::bestCaseTrace(void) {
  unsigned int lowestMemoryInTrace      = 0;
  unsigned int lowestMemoryInTraceIndex = 0;

  for (int i = 0; i < TRACES; i++) {
    if (tracesMemory[i] > lowestMemoryInTrace) {
      lowestMemoryInTrace      = tracesMemory[i];
      lowestMemoryInTraceIndex = i;
    }
  }

  // serialPrintln(lowestMemoryInTraceIndex);
  return lowestMemoryInTraceIndex;
}

RamTracker::RamTracker(void) {
  readPtr  = 0;
  writePtr = 0;

  for (int i = 0; i < TRACES; i++) {
    traces[i]       = "";
    tracesMemory[i] = 0xffffffff; // init with best case memory values, so they get replaced if memory goes lower
  }

  for (int i = 0; i < TRACEENTRIES; i++) {
    nextAction[i]            = "startup";
    nextActionStartMemory[i] = ESP.getFreeHeap(); // init with best case memory values, so they get replaced if memory goes lower
  }
}

void RamTracker::registerRamState(const String& s) {   // store function
  nextAction[writePtr]            = s;                 // name and mem
  nextActionStartMemory[writePtr] = ESP.getFreeHeap(); // in cyclic buffer.
  int bestCase = bestCaseTrace();                      // find best case memory trace

  if (ESP.getFreeHeap() < tracesMemory[bestCase]) {    // compare to current memory value
    traces[bestCase] = "";
    readPtr          = writePtr + 1;                   // read out buffer, oldest value first

    if (readPtr >= TRACEENTRIES) { 
      readPtr = 0;        // read pointer wrap around
    }
    tracesMemory[bestCase] = ESP.getFreeHeap();        // store new lowest value of that trace

    for (int i = 0; i < TRACEENTRIES; i++) {           // tranfer cyclic buffer strings and mem values to this trace
      traces[bestCase] += nextAction[readPtr];
      traces[bestCase] += "-> ";
      traces[bestCase] += String(nextActionStartMemory[readPtr]);
      traces[bestCase] += ' ';
      readPtr++;

      if (readPtr >= TRACEENTRIES) { readPtr = 0; // wrap around read pointer
      }
    }
  }
  writePtr++;

  if (writePtr >= TRACEENTRIES) { writePtr = 0; // inc write pointer and wrap around too.
  }
}

 // return giant strings, one line per trace. Add stremToWeb method to avoid large strings.
void RamTracker::getTraceBuffer() {
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String retval = "Memtrace\n";

    for (int i = 0; i < TRACES; i++) {
      retval += String(i);
      retval += ": lowest: ";
      retval += String(tracesMemory[i]);
      retval += "  ";
      retval += traces[i];
      addLog(LOG_LEVEL_DEBUG_DEV, retval);
      retval = "";
    }
  }
#endif // ifndef BUILD_NO_DEBUG
}

#else // BUILD_NO_RAM_TRACKER
/*

void checkRAMtoLog(void) {}

void checkRAM(const String& flashString,
              int           a) {}

void checkRAM(const String& flashString,
              const String& a) {}

void checkRAM(const String& descr) {}
*/

#endif // BUILD_NO_RAM_TRACKER
#include "../Globals/Plugins.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../../_Plugin_Helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/Cache.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/EventQueue.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"



int deviceCount = -1;

boolean (*Plugin_ptr[PLUGIN_MAX])(uint8_t,
                                  struct EventStruct *,
                                  String&);

pluginID_t DeviceIndex_to_Plugin_id[PLUGIN_MAX + 1];
std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;
std::vector<deviceIndex_t> DeviceIndex_sorted;


bool validDeviceIndex(deviceIndex_t index) {
  if (index < PLUGIN_MAX) {
    const pluginID_t pluginID = DeviceIndex_to_Plugin_id[index];
    return pluginID != INVALID_PLUGIN_ID;
  }
  return false;
}

bool validTaskIndex(taskIndex_t index) {
  return index < TASKS_MAX;
}

bool validPluginID(pluginID_t pluginID) {
  return (pluginID != INVALID_PLUGIN_ID);
}

bool validPluginID_fullcheck(pluginID_t pluginID) {
  if (!validPluginID(pluginID)) {
    return false;
  }
  auto it = Plugin_id_to_DeviceIndex.find(pluginID);
  return (it != Plugin_id_to_DeviceIndex.end());
}

bool validUserVarIndex(userVarIndex_t index) {
  return index < USERVAR_MAX_INDEX;
}

bool validTaskVarIndex(taskVarIndex_t index) {
  return index < VARS_PER_TASK;
}

bool supportedPluginID(pluginID_t pluginID) {
  return validDeviceIndex(getDeviceIndex(pluginID));
}

deviceIndex_t getDeviceIndex_from_TaskIndex(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    return getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
  }
  return INVALID_DEVICE_INDEX;
}

/*********************************************************************************************
 * get the taskPluginID with required checks, INVALID_PLUGIN_ID when invalid
 ********************************************************************************************/
pluginID_t getPluginID_from_TaskIndex(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

    if (validDeviceIndex(DeviceIndex)) {
      return DeviceIndex_to_Plugin_id[DeviceIndex];
    }
  }
  return INVALID_PLUGIN_ID;
}

deviceIndex_t getDeviceIndex(pluginID_t pluginID)
{
  if (pluginID != INVALID_PLUGIN_ID) {
    auto it = Plugin_id_to_DeviceIndex.find(pluginID);

    if (it != Plugin_id_to_DeviceIndex.end())
    {
      if (!validDeviceIndex(it->second)) { return INVALID_DEVICE_INDEX; }

      if (Device[it->second].Number != pluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        addLog(LOG_LEVEL_ERROR, F("getDeviceIndex error in Device Vector"));
      }
      return it->second;
    }
  }
  return INVALID_DEVICE_INDEX;
}

/********************************************************************************************\
   Find name of plugin given the plugin device index..
 \*********************************************************************************************/
String getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex) {
  String deviceName = "";

  if (validDeviceIndex(deviceIndex)) {
    Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, nullptr, deviceName);
  }
  return deviceName;
}

String getPluginNameFromPluginID(pluginID_t pluginID) {
  deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) {
    String name = F("Plugin ");
    name += String(static_cast<int>(pluginID));
    name += F(" not included in build");
    return name;
  }
  return getPluginNameFromDeviceIndex(deviceIndex);
}

#if USE_I2C_DEVICE_SCAN
bool checkPluginI2CAddressFromDeviceIndex(deviceIndex_t deviceIndex, uint8_t i2cAddress) {
  bool hasI2CAddress = false;

  if (validDeviceIndex(deviceIndex)) {
    String dummy;
    struct EventStruct TempEvent;
    TempEvent.Par1 = i2cAddress;
    hasI2CAddress = Plugin_ptr[deviceIndex](PLUGIN_I2C_HAS_ADDRESS, &TempEvent, dummy);
  }
  return hasI2CAddress;
}
#endif // if USE_I2C_DEVICE_SCAN

// ********************************************************************************
// Device Sort routine, compare two array entries
// ********************************************************************************
bool arrayLessThan(const String& ptr_1, const String& ptr_2)
{
  unsigned int i = 0;

  while (i < ptr_1.length()) // For each character in string 1, starting with the first:
  {
    if (ptr_2.length() < i)  // If string 2 is shorter, then switch them
    {
      return true;
    }
    const char check1 = static_cast<char>(ptr_1[i]); // get the same char from string 1 and string 2
    const char check2 = static_cast<char>(ptr_2[i]);

    if (check1 == check2) {
      // they're equal so far; check the next char !!
      i++;
    } else {
      return check2 > check1;
    }
  }
  return false;
}

// ********************************************************************************
// Device Sort routine, actual sorting alfabetically by plugin name.
// Sorting does happen case sensitive.
// ********************************************************************************
void sortDeviceIndexArray() {
  // First fill the existing number of the DeviceIndex.
  DeviceIndex_sorted.resize(deviceCount + 1);

  for (deviceIndex_t x = 0; x <= deviceCount; x++) {
    if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
      DeviceIndex_sorted[x] = x;
    } else {
      DeviceIndex_sorted[x] = INVALID_DEVICE_INDEX;
    }
  }

  // Do the sorting.
  int innerLoop;
  int mainLoop;

  for (mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;

    while (innerLoop  >= 1)
    {
      if (arrayLessThan(
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop]),
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop - 1])))
      {
        deviceIndex_t temp = DeviceIndex_sorted[innerLoop - 1];
        DeviceIndex_sorted[innerLoop - 1] = DeviceIndex_sorted[innerLoop];
        DeviceIndex_sorted[innerLoop]     = temp;
      }
      innerLoop--;
    }
  }
}

// ********************************************************************************
// Functions to assist changing I2C multiplexer port or clock speed 
// when addressing a task
// ********************************************************************************

bool prepare_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex) {
  if (!validTaskIndex(taskIndex) || !validDeviceIndex(DeviceIndex)) {
    return false;
  }
  if (Device[DeviceIndex].Type != DEVICE_TYPE_I2C) {
    return true; // No I2C task, so consider all-OK
  }
  if (I2C_state != I2C_bus_state::OK) {
    return false; // Bus state is not OK, so do not consider task runnable
  }
#ifdef FEATURE_I2CMULTIPLEXER
  I2CMultiplexerSelectByTaskIndex(taskIndex);
  // Output is selected after this write, so now we must make sure the
  // frequency is set before anything else is sent.
#endif

  if (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED)) {
    I2CSelectLowClockSpeed(); // Set to slow
  }
  return true;
}


void post_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex) {
  if (!validTaskIndex(taskIndex) || !validDeviceIndex(DeviceIndex)) {
    return;
  }
  if (Device[DeviceIndex].Type != DEVICE_TYPE_I2C) {
    return;
  }
#ifdef FEATURE_I2CMULTIPLEXER
  I2CMultiplexerOff();
#endif

  if (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED)) {
    I2CSelectHighClockSpeed();  // Reset
  }
}

// Add an event to the event queue.
// event value 1 = taskIndex (first task = 1)
// event value 2 = return value of the plugin function
// Example:  TaskInit#bme=1,0    (taskindex = 0, return value = 0)
void queueTaskEvent(const String& eventName, taskIndex_t taskIndex, int value1) {
  if (Settings.UseRules) {
    String event;
    event.reserve(eventName.length() + 32);
    event  = eventName;
    event += '#';
    event += getTaskDeviceName(taskIndex);
    event += '=';
    event += taskIndex + 1;
    event += ',';
    event += value1;
    eventQueue.addMove(std::move(event));
  }
}


/**
 * Call the plugin of 1 task for 1 function, with standard EventStruct and optional command string
 */
bool PluginCallForTask(taskIndex_t taskIndex, uint8_t Function, EventStruct *TempEvent, String& command, EventStruct *event = nullptr) {
  bool retval = false;
  if (Settings.TaskDeviceEnabled[taskIndex] && validPluginID_fullcheck(Settings.TaskDeviceNumber[taskIndex]))
  {
    if (Settings.TaskDeviceDataFeed[taskIndex] == 0) // these calls only to tasks with local feed
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);
      if (validDeviceIndex(DeviceIndex)) {
        TempEvent->setTaskIndex(taskIndex);
        TempEvent->sensorType   = Device[DeviceIndex].VType;
        if (event != nullptr) {
          TempEvent->OriginTaskIndex = event->TaskIndex;
        }

        if (!prepare_I2C_by_taskIndex(taskIndex, DeviceIndex)) {
          return false;
        }
        switch (Function) {
          case PLUGIN_WRITE:          // First set
          case PLUGIN_REQUEST:
          case PLUGIN_ONCE_A_SECOND:  // Second set
          case PLUGIN_TEN_PER_SECOND:
          case PLUGIN_FIFTY_PER_SECOND:
          case PLUGIN_INIT:           // Second set, instead of PLUGIN_INIT_ALL
          case PLUGIN_CLOCK_IN:
          case PLUGIN_EVENT_OUT:
          case PLUGIN_TIME_CHANGE:
            {
              #ifndef BUILD_NO_RAM_TRACKER
              checkRAM(F("PluginCall_s"), taskIndex);
              #endif
              break;
            }
        }
        START_TIMER;
        retval = (Plugin_ptr[DeviceIndex](Function, TempEvent, command));
        STOP_TIMER_TASK(DeviceIndex, Function);

        if (Function == PLUGIN_INIT) {
          // Schedule the plugin to be read.
          Scheduler.schedule_task_device_timer_at_init(TempEvent->TaskIndex);
          queueTaskEvent(F("TaskInit"), taskIndex, retval);
        }

        post_I2C_by_taskIndex(taskIndex, DeviceIndex);
        delay(0); // SMY: call delay(0) unconditionally
      }
    }
  }
  return retval;
}

/*********************************************************************************************\
* Function call to all or specific plugins
\*********************************************************************************************/
bool PluginCall(uint8_t Function, struct EventStruct *event, String& str)
{
  struct EventStruct TempEvent;

  if (event == nullptr) {
    event = &TempEvent;
  }
  else {
    TempEvent.deep_copy(*event);
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("PluginCall"), Function);
  #endif

  switch (Function)
  {
    // Unconditional calls to all plugins
    case PLUGIN_DEVICE_ADD:
    case PLUGIN_UNCONDITIONAL_POLL:    // FIXME TD-er: PLUGIN_UNCONDITIONAL_POLL is not being used at the moment

      for (deviceIndex_t x = 0; x < PLUGIN_MAX; x++) {
        if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
          if (Function == PLUGIN_DEVICE_ADD) {
            if ((deviceCount + 2) > static_cast<int>(Device.size())) {
              // Increase with 16 to get some compromise between number of resizes and wasted space
              unsigned int newSize = Device.size();
              newSize = newSize + 16 - (newSize % 16);
              Device.resize(newSize);

              // FIXME TD-er: Also resize DeviceIndex_to_Plugin_id ?
            }
          }
          START_TIMER;
          Plugin_ptr[x](Function, event, str);
          STOP_TIMER_TASK(x, Function);
          delay(0); // SMY: call delay(0) unconditionally
        }
      }
      return true;

    case PLUGIN_MONITOR:

      for (auto it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it) {
        // only call monitor function if there the need to
        if (it->second.monitor || it->second.command || it->second.init) {
          TempEvent.Par1 = getPortFromKey(it->first);

          // initialize the "x" variable to synch with the pluginNumber if second.x == -1
          if (!validDeviceIndex(it->second.x)) { it->second.x = getDeviceIndex(getPluginFromKey(it->first)); }

          const deviceIndex_t DeviceIndex = it->second.x;
          if (validDeviceIndex(DeviceIndex))  {
            START_TIMER;
            Plugin_ptr[DeviceIndex](Function, &TempEvent, str);
            STOP_TIMER_TASK(DeviceIndex, Function);
          }
        }
      }
      return true;


    // Call to all plugins. Return at first match
    case PLUGIN_WRITE:
//    case PLUGIN_REQUEST: @giig1967g: replaced by new function getGPIOPluginValues()
    {
      taskIndex_t firstTask = 0;
      taskIndex_t lastTask = TASKS_MAX;
      String command = String(str);                           // Local copy to avoid warning in ExecuteCommand
      int dotPos = command.indexOf('.');                      // Find first period
      if (Function == PLUGIN_WRITE                            // Only applicable on PLUGIN_WRITE function
        && dotPos > -1) {                                     // First precondition is just a quick check for a period (fail-fast strategy)
        String arg0 = parseString(command, 1);                // Get first argument
        dotPos = arg0.indexOf('.');
        if (dotPos > -1) {
          String thisTaskName = command.substring(0, dotPos); // Extract taskname prefix
          thisTaskName.replace(F("["), EMPTY_STRING);                      // Remove the optional square brackets
          thisTaskName.replace(F("]"), EMPTY_STRING);
          if (thisTaskName.length() > 0) {                    // Second precondition
            taskIndex_t thisTask = findTaskIndexByName(thisTaskName);
            if (!validTaskIndex(thisTask)) {                  // Taskname not found or invalid, check for a task number?
              thisTask = static_cast<taskIndex_t>(atoi(thisTaskName.c_str()));
              if (thisTask == 0 || thisTask > TASKS_MAX) {
                thisTask = INVALID_TASK_INDEX;
              } else {
                thisTask--;                                   // 0-based
              }
            }
            if (validTaskIndex(thisTask)) {                   // Known taskindex?
#ifdef USES_P022                                              // Exclude P022 as it has rather explicit differences in commands when used with the [<TaskName>]. prefix
              if (Settings.TaskDeviceEnabled[thisTask]        // and internally needs to know wether it was called with the taskname prefixed
                && validPluginID_fullcheck(Settings.TaskDeviceNumber[thisTask])
                && Settings.TaskDeviceDataFeed[thisTask] == 0) {
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(thisTask);
                if (validDeviceIndex(DeviceIndex) && Device[DeviceIndex].Number == 22 /* PLUGIN_ID_022 define no longer available, 'assume' 22 for now */) {
                  thisTask = INVALID_TASK_INDEX;
                }
              }
              if (validTaskIndex(thisTask)) {
#endif
                firstTask = thisTask;
                lastTask  = thisTask + 1;                     // Add 1 to satisfy the for condition
                command   = command.substring(dotPos + 1);    // Remove [<TaskName>]. prefix
#ifdef USES_P022
              }
#endif
            }
          }
        }
      }
  // String info = F("PLUGIN_WRITE first: "); // To remove
  // info += firstTask;
  // info += F(" last: ");
  // info += lastTask;
  // addLog(LOG_LEVEL_INFO, info);

      for (taskIndex_t task = firstTask; task < lastTask; task++)
      {
        bool retval = PluginCallForTask(task, Function, &TempEvent, command);

        if (retval) {
          CPluginCall(CPlugin::Function::CPLUGIN_ACKNOWLEDGE, &TempEvent, command);
          return true;
        }
      }

      if (Function == PLUGIN_REQUEST) {
        // @FIXME TD-er: work-around as long as gpio command is still performed in P001_switch.
        for (deviceIndex_t deviceIndex = 0; deviceIndex < PLUGIN_MAX; deviceIndex++) {
          if (validPluginID(DeviceIndex_to_Plugin_id[deviceIndex])) {
            if (Plugin_ptr[deviceIndex](Function, event, str)) {
              delay(0); // SMY: call delay(0) unconditionally
              CPluginCall(CPlugin::Function::CPLUGIN_ACKNOWLEDGE, event, str);
              return true;
            }
          }
        }
      }
      break;
    }

    // Call to all plugins used in a task. Return at first match
    case PLUGIN_SERIAL_IN:
    case PLUGIN_UDP_IN:
    {
      for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
      {
        bool retval = PluginCallForTask(taskIndex, Function, &TempEvent, str);

        if (retval) {
          #ifndef BUILD_NO_RAM_TRACKER
          checkRAM(F("PluginCallUDP"), taskIndex);
          #endif
          return true;
        }
      }
      return false;
    }

    // Call to all plugins that are used in a task
    case PLUGIN_ONCE_A_SECOND:
    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_FIFTY_PER_SECOND:
    case PLUGIN_INIT_ALL:
    case PLUGIN_CLOCK_IN:
    case PLUGIN_EVENT_OUT:
    case PLUGIN_TIME_CHANGE:
    {
      if (Function == PLUGIN_INIT_ALL) {
        Function = PLUGIN_INIT;
      }

      for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
      {
        #ifndef BUILD_NO_DEBUG
        const int freemem_begin = ESP.getFreeHeap();
        #endif

        PluginCallForTask(taskIndex, Function, &TempEvent, str, event);

        #ifndef BUILD_NO_DEBUG
        if (Function == PLUGIN_INIT) {
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            // See also logMemUsageAfter()
            const int freemem_end = ESP.getFreeHeap();
            String log;
            log.reserve(128);
            log  = F("After PLUGIN_INIT ");
            log += F(" task: ");
            if (taskIndex < 9) log += ' ';
            log += taskIndex + 1;
            while (log.length() < 30) log += ' ';
            log += F("Free mem after: ");
            log += freemem_end;
            while (log.length() < 53) log += ' ';
            log += F("plugin: ");
            log += freemem_begin - freemem_end;
            while (log.length() < 67) log += ' ';

            log += Settings.TaskDeviceEnabled[taskIndex] ? F("[ena]") : F("[dis]");
            while (log.length() < 73) log += ' ';
            log += getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(taskIndex));

            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        #endif
      }
      if (Function == PLUGIN_INIT) {
        updateTaskCaches();
      }

      return true;
    }

    // Call to specific task which may interact with the hardware
    case PLUGIN_INIT:
    case PLUGIN_EXIT:
    case PLUGIN_WEBFORM_LOAD:
    case PLUGIN_READ:
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

      if (validDeviceIndex(DeviceIndex)) {
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex) {
          // LoadTaskSettings may call PLUGIN_GET_DEVICEVALUENAMES.
          LoadTaskSettings(event->TaskIndex);
        }
        event->BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
        {
          #ifndef BUILD_NO_RAM_TRACKER
          String descr;
          descr.reserve(20);
          descr  = F("PluginCall_task_");
          descr += event->TaskIndex;
          checkRAM(descr, String(Function));
          #endif
        }
        if (!prepare_I2C_by_taskIndex(event->TaskIndex, DeviceIndex)) {
          return false;
        }
        START_TIMER;
        bool retval =  Plugin_ptr[DeviceIndex](Function, event, str);

        if (retval && (Function == PLUGIN_READ)) {
          saveUserVarToRTC();
        }
        if (Function == PLUGIN_INIT) {
          // Schedule the plugin to be read.
          Scheduler.schedule_task_device_timer_at_init(TempEvent.TaskIndex);
          updateTaskCaches();
          queueTaskEvent(F("TaskInit"), event->TaskIndex, retval);
        }
        if (Function == PLUGIN_EXIT) {
          clearPluginTaskData(event->TaskIndex);
          updateTaskCaches();
          initSerial();
          queueTaskEvent(F("TaskExit"), event->TaskIndex, retval);
        }
        STOP_TIMER_TASK(DeviceIndex, Function);
        post_I2C_by_taskIndex(event->TaskIndex, DeviceIndex);
        delay(0); // SMY: call delay(0) unconditionally

        return retval;
      }
      return false;
    }

    // Call to specific task not interacting with hardware
    case PLUGIN_GET_CONFIG:
    case PLUGIN_GET_DEVICEVALUENAMES:
    case PLUGIN_GET_DEVICEVALUECOUNT:
    case PLUGIN_GET_DEVICEVTYPE:
    case PLUGIN_GET_DEVICEGPIONAMES:
    case PLUGIN_WEBFORM_SAVE:
    case PLUGIN_WEBFORM_SHOW_VALUES:
    case PLUGIN_WEBFORM_SHOW_CONFIG:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    case PLUGIN_FORMAT_USERVAR:
    case PLUGIN_SET_CONFIG:
    case PLUGIN_SET_DEFAULTS:
    case PLUGIN_I2C_HAS_ADDRESS:

    // PLUGIN_MQTT_xxx functions are directly called from the scheduler.
    //case PLUGIN_MQTT_CONNECTION_STATE:
    //case PLUGIN_MQTT_IMPORT:
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

      if (validDeviceIndex(DeviceIndex)) {
        // LoadTaskSettings may call PLUGIN_GET_DEVICEVALUENAMES.
        LoadTaskSettings(event->TaskIndex);
        event->BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
        {
          #ifndef BUILD_NO_RAM_TRACKER
          String descr;
          descr.reserve(20);
          descr  = F("PluginCall_task_");
          descr += event->TaskIndex;
          checkRAM(descr, String(Function));
          #endif
        }
        if (Function == PLUGIN_SET_DEFAULTS) {
          for (int i = 0; i < VARS_PER_TASK; ++i) {
            UserVar[event->BaseVarIndex + i] = 0.0f;
          }
        }
        if (Function == PLUGIN_GET_DEVICEVALUECOUNT) {
          event->Par1 = Device[DeviceIndex].ValueCount;
        }
        if (Function == PLUGIN_GET_DEVICEVTYPE) {
          event->sensorType = Device[DeviceIndex].VType;
        }

        START_TIMER;
        bool retval =  Plugin_ptr[DeviceIndex](Function, event, str);
        if (Function == PLUGIN_SET_DEFAULTS) {
          saveUserVarToRTC();
        }
        if (Function == PLUGIN_GET_DEVICEVALUECOUNT) {
          // Check if we have a valid value count.
          if (Output_Data_type_t::Simple == Device[DeviceIndex].OutputDataType) {
            if (event->Par1 < 1 || event->Par1 > 4) {
              // Output_Data_type_t::Simple only allows for 1 .. 4 output types.
              // Apparently the value is not correct, so use the default.
              event->Par1 = Device[DeviceIndex].ValueCount;
            }
          }
        }

        // Calls may have updated ExtraTaskSettings, so validate them.
        ExtraTaskSettings.validate();
        
        STOP_TIMER_TASK(DeviceIndex, Function);
        delay(0); // SMY: call delay(0) unconditionally
        return retval;
      }
      return false;
    }

  } // case
  return false;
}

bool addPlugin(pluginID_t pluginID, deviceIndex_t x) {
  if (x < PLUGIN_MAX) { 
    DeviceIndex_to_Plugin_id[x] = pluginID; 
    Plugin_id_to_DeviceIndex[pluginID] = x;
    return true;
  }
  String log = F("System: Error - Too many Plugins. PLUGIN_MAX = ");
  log += PLUGIN_MAX;
  addLog(LOG_LEVEL_ERROR, log);
  return false;
}

#include "../Globals/Protocol.h"

ProtocolStruct Protocol[CPLUGIN_MAX];
int protocolCount = -1;

#include "../Globals/MainLoopCommand.h"

uint8_t cmd_within_mainloop = 0;
#include "../Globals/Nodes.h"


NodesMap Nodes;
#include "../Globals/WiFi_AP_Candidates.h"

WiFi_AP_CandidatesList WiFi_AP_Candidates;

#include "../Globals/ESPEasyWiFiEvent.h"

#include "../../ESPEasy_common.h"

// FIXME TD-er: Rename this to ESPEasyNetworkEvent


#ifdef ESP8266
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
WiFiEventHandler stationGotIpHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;
WiFiEventHandler stationModeAuthModeChangeHandler;
WiFiEventHandler APModeStationConnectedHandler;
WiFiEventHandler APModeStationDisconnectedHandler;
#endif // ifdef ESP8266

WiFiEventData_t WiFiEventData;

#ifdef HAS_ETHERNET
EthernetEventData_t EthEventData;
#endif


#ifdef ESP32
WiFiEventId_t wm_event_id;
#endif // ifdef ESP32

#include "../Globals/CPlugins.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"



// FIXME TD-er: Make these private and add functions to access its content.
std::map<cpluginID_t, protocolIndex_t> CPlugin_id_to_ProtocolIndex;
cpluginID_t ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX + 1];

bool (*CPlugin_ptr[CPLUGIN_MAX])(CPlugin::Function,
                                 struct EventStruct *,
                                 String&);


/********************************************************************************************\
   Call CPlugin functions
 \*********************************************************************************************/
bool CPluginCall(CPlugin::Function Function, struct EventStruct *event) {
  String dummy;

  return CPluginCall(Function, event, dummy);
}

bool CPluginCall(CPlugin::Function Function, struct EventStruct *event, String& str)
{
  struct EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    // Unconditional calls to all included controller in the build
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:

      for (protocolIndex_t x = 0; x < CPLUGIN_MAX; x++) {
        const cpluginID_t cpluginID = ProtocolIndex_to_CPlugin_id[x];
        if (validCPluginID(cpluginID)) {
          #ifndef BUILD_NO_RAM_TRACKER
          checkRAM(F("CPluginCallADD"), x);
          #endif

          /*
          #ifndef BUILD_NO_DEBUG
          const int freemem_begin = ESP.getFreeHeap();
          #endif
          */

          String dummy;
          CPluginCall(x, Function, event, dummy);
          /*
          #ifndef BUILD_NO_DEBUG
          if (Function == CPlugin::Function::CPLUGIN_PROTOCOL_ADD) {
            // See also logMemUsageAfter()
            const int freemem_end = ESP.getFreeHeap();
            String log = F("After CPLUGIN_ADD ");
            log += cpluginID;
            while (log.length() < 30) log += ' ';
            log += F("Free mem after: ");
            log += freemem_end;
            while (log.length() < 54) log += ' ';
            log += F("ctrlr: ");
            log += freemem_begin - freemem_end;

            addLog(LOG_LEVEL_INFO, log);
          }
          #endif
          */
        }
      }
      return true;


    // calls to all active controllers
    case CPlugin::Function::CPLUGIN_INIT_ALL:
    case CPlugin::Function::CPLUGIN_UDP_IN:
    case CPlugin::Function::CPLUGIN_INTERVAL:      // calls to send stats information
    case CPlugin::Function::CPLUGIN_GOT_CONNECTED: // calls to send autodetect information
    case CPlugin::Function::CPLUGIN_GOT_INVALID:   // calls to mark unit as invalid
    case CPlugin::Function::CPLUGIN_FLUSH:
    case CPlugin::Function::CPLUGIN_TEN_PER_SECOND:
    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:

      if (Function == CPlugin::Function::CPLUGIN_INIT_ALL) {
        Function = CPlugin::Function::CPLUGIN_INIT;
      }

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if ((Settings.Protocol[x] != 0) && Settings.ControllerEnabled[x]) {
          protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);
          event->ControllerIndex = x;
          String dummy;
          CPluginCall(ProtocolIndex, Function, event, dummy);
        }
      }
      return true;

    // calls to specific controller
    case CPlugin::Function::CPLUGIN_INIT:
    case CPlugin::Function::CPLUGIN_EXIT:
    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:
    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      controllerIndex_t controllerindex = event->ControllerIndex;

      if (Settings.ControllerEnabled[controllerindex] && supportedCPluginID(Settings.Protocol[controllerindex]))
      {
        if (Function == CPlugin::Function::CPLUGIN_PROTOCOL_SEND) {
          checkDeviceVTypeForTask(event);
        }
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerindex);
        CPluginCall(ProtocolIndex, Function, event, str);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE: // calls to send acknowledge back to controller

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
          protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);
          CPluginCall(ProtocolIndex, Function, event, str);
        }
      }
      return true;
  }

  return false;
}

bool CPluginCall(protocolIndex_t protocolIndex, CPlugin::Function Function, struct EventStruct *event, String& str) {
  if (validProtocolIndex(protocolIndex)) {
    #ifndef BUILD_NO_DEBUG
    const int freemem_begin = ESP.getFreeHeap();
    #endif

    START_TIMER;
    bool ret = CPlugin_ptr[protocolIndex](Function, event, str);
    STOP_TIMER_CONTROLLER(protocolIndex, Function);
    #ifndef BUILD_NO_DEBUG
    if (Function == CPlugin::Function::CPLUGIN_INIT) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        // See also logMemUsageAfter()
        const int freemem_end = ESP.getFreeHeap();
        String log = F("After CPLUGIN_INIT ");
        while (log.length() < 30) log += ' ';
        log += F("Free mem after: ");
        log += freemem_end;
        while (log.length() < 54) log += ' ';
        log += F("ctrlr: ");
        log += freemem_begin - freemem_end;
        while (log.length() < 73) log += ' ';
        log += getCPluginNameFromProtocolIndex(protocolIndex);

        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
    #endif
    return ret;
  }
  return false;
}

// Check if there is any controller enabled.
bool anyControllerEnabled() {
  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
      return true;
    }
  }
  return false;
}

// Find first enabled controller index with this protocol
controllerIndex_t findFirstEnabledControllerWithId(cpluginID_t cpluginid) {
  if (!supportedCPluginID(cpluginid)) {
    return INVALID_CONTROLLER_INDEX;
  }

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++) {
    if ((Settings.Protocol[i] == cpluginid) && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

bool validProtocolIndex(protocolIndex_t index)
{
  return getCPluginID_from_ProtocolIndex(index) != INVALID_C_PLUGIN_ID;
}

bool validControllerIndex(controllerIndex_t index)
{
  return index < CONTROLLER_MAX;
}

bool validCPluginID(cpluginID_t cpluginID)
{
  if (cpluginID == INVALID_C_PLUGIN_ID) {
    return false;
  }
  auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);
  return it != CPlugin_id_to_ProtocolIndex.end();
}

bool supportedCPluginID(cpluginID_t cpluginID)
{
  return validProtocolIndex(getProtocolIndex(cpluginID));
}

protocolIndex_t getProtocolIndex_from_ControllerIndex(controllerIndex_t index) {
  if (validControllerIndex(index)) {
    return getProtocolIndex(Settings.Protocol[index]);
  }
  return INVALID_PROTOCOL_INDEX;
}

cpluginID_t getCPluginID_from_ProtocolIndex(protocolIndex_t index) {
  if (index < CPLUGIN_MAX) {
    const cpluginID_t cpluginID = ProtocolIndex_to_CPlugin_id[index];
    return cpluginID;
  }
  return INVALID_C_PLUGIN_ID;
}

cpluginID_t getCPluginID_from_ControllerIndex(controllerIndex_t index) {
  const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(index);

  return getCPluginID_from_ProtocolIndex(protocolIndex);
}

protocolIndex_t getProtocolIndex(cpluginID_t cpluginID)
{
  if (cpluginID != INVALID_C_PLUGIN_ID) {
    auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);

    if (it != CPlugin_id_to_ProtocolIndex.end())
    {
      if (!validProtocolIndex(it->second)) { return INVALID_PROTOCOL_INDEX; }
      #ifndef BUILD_NO_DEBUG
      if (Protocol[it->second].Number != cpluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        String log = F("getProtocolIndex error in Protocol Vector. CPluginID: ");
        log += String(cpluginID);
        log += F(" p_index: ");
        log += String(it->second);
        addLog(LOG_LEVEL_ERROR, log);
      }
      #endif
      return it->second;
    }
  }
  return INVALID_PROTOCOL_INDEX;
}

String getCPluginNameFromProtocolIndex(protocolIndex_t ProtocolIndex) {
  String controllerName;

  if (validProtocolIndex(ProtocolIndex)) {
    CPlugin_ptr[ProtocolIndex](CPlugin::Function::CPLUGIN_GET_DEVICENAME, nullptr, controllerName);
  }
  return controllerName;
}

String getCPluginNameFromCPluginID(cpluginID_t cpluginID) {
  protocolIndex_t protocolIndex = getProtocolIndex(cpluginID);

  if (!validProtocolIndex(protocolIndex)) {
    String name = F("CPlugin ");
    name += String(static_cast<int>(cpluginID));
    name += F(" not included in build");
    return name;
  }
  return getCPluginNameFromProtocolIndex(protocolIndex);
}


bool addCPlugin(cpluginID_t cpluginID, protocolIndex_t x) {
  if (x < CPLUGIN_MAX) { 
    ProtocolIndex_to_CPlugin_id[x] = cpluginID; 
    CPlugin_id_to_ProtocolIndex[cpluginID] = x;
    return true;
  }
  String log = F("System: Error - Too many C-Plugins. CPLUGIN_MAX = ");
  log += CPLUGIN_MAX;
  addLog(LOG_LEVEL_ERROR, log);
  return false;
}

#include "../Globals/SecuritySettings.h"


SecurityStruct SecuritySettings;
ExtendedControllerCredentialsStruct ExtendedControllerCredentials;
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../DataStructs/FactoryDefaultPref.h"

ResetFactoryDefaultPreference_struct ResetFactoryDefaultPreference;
#include "../Globals/ExtraTaskSettings.h"


// TODO TD-er: Move fuctions to manage the ExtraTaskSettings cache to this file.

ExtraTaskSettingsStruct ExtraTaskSettings;
#include "../Globals/RuntimeData.h"


std::map<uint32_t, double> customFloatVar;

//float UserVar[VARS_PER_TASK * TASKS_MAX];

UserVarStruct UserVar;


double getCustomFloatVar(uint32_t index) {
  auto it = customFloatVar.find(index);

  if (it != customFloatVar.end()) {
    return it->second;
  }
  return 0.0;
}

void setCustomFloatVar(uint32_t index, const double& value) {
  customFloatVar[index] = value;
}

bool getNextCustomFloatVar(uint32_t& index, double& value) {
  auto it = customFloatVar.find(index);

  if (it == customFloatVar.end()) { return false; }
  ++it;

  if (it == customFloatVar.end()) { return false; }
  index = it->first;
  value = it->second;
  return true;
}

#include "../Globals/MQTT.h"

#ifdef USES_MQTT

// MQTT client
WiFiClient   mqtt;
PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect        = true;
bool MQTTclient_must_send_LWT_connected = false;
bool MQTTclient_connected               = false;
int  mqtt_reconnect_count               = 0;
#endif // USES_MQTT

#ifdef USES_P037

// mqtt import status
bool P037_MQTTImport_connected = false;
#endif // ifdef USES_P037

#include "../Globals/C016_ControllerCache.h"

#ifdef USES_C016


#include "../ControllerQueue/C016_queue_element.h"

ControllerCache_struct ControllerCache;

bool C016_startCSVdump() {
  ControllerCache.resetpeek();
  return ControllerCache.isInitialized();
}

String C016_getCacheFileName(bool& islast) {
  return ControllerCache.getPeekCacheFileName(islast);
}

bool C016_deleteOldestCacheBlock() {
  return ControllerCache.deleteOldestCacheBlock();
}

bool C016_getCSVline(
  unsigned long& timestamp,
  uint8_t& controller_idx,
  uint8_t& TaskIndex,
  Sensor_VType& sensorType,
  uint8_t& valueCount,
  float& val1,
  float& val2,
  float& val3,
  float& val4)
{
  C016_queue_element element;
  bool result = ControllerCache.peek((uint8_t*)&element, sizeof(element));
  timestamp = element._timestamp;
  controller_idx = element.controller_idx;
  TaskIndex = element.TaskIndex;
  sensorType = element.sensorType;
  valueCount = element.valueCount;
  val1 = element.values[0];
  val2 = element.values[1];
  val3 = element.values[2];
  val4 = element.values[3];
  return result;
}

#endif
