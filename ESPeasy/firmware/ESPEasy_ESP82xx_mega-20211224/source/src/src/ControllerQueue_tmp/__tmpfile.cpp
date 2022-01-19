#include "../ControllerQueue/C015_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#ifdef USES_C015

C015_queue_element::C015_queue_element(C015_queue_element&& other)
  : idx(other.idx), _timestamp(other._timestamp), TaskIndex(other.TaskIndex)
  , controller_idx(other.controller_idx), valuesSent(other.valuesSent)
  , valueCount(other.valueCount)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i]  = std::move(other.txt[i]);
    vPin[i] = other.vPin[i];
  }
}

C015_queue_element::C015_queue_element(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  valuesSent(0),
  valueCount(value_count) {}

C015_queue_element& C015_queue_element::operator=(C015_queue_element&& other) {
  idx = other.idx;
  _timestamp = other._timestamp;
  TaskIndex  = other.TaskIndex;
  controller_idx = other.controller_idx;
  valuesSent = other.valuesSent;
  valueCount = other.valueCount;
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i]  = std::move(other.txt[i]);
    vPin[i] = other.vPin[i];
  }
  return *this;
}

bool C015_queue_element::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t C015_queue_element::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool C015_queue_element::isDuplicate(const C015_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.valueCount != valueCount) ||
      (other.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }

    if (other.vPin[i] != vPin[i]) {
      return false;
    }
  }
  return true;
}

#endif // ifdef USES_C015

#include "../ControllerQueue/C018_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Helpers/_CPlugin_LoRa_TTN_helper.h"

#ifdef USES_C018

C018_queue_element::C018_queue_element(struct EventStruct *event, uint8_t sampleSetCount) :
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex)
{
    # ifdef USES_PACKED_RAW_DATA
  packed = getPackedFromPlugin(event, sampleSetCount);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C018 queue element: ");
    log += packed;
    addLog(LOG_LEVEL_INFO, log);
  }
    # endif // USES_PACKED_RAW_DATA
}

size_t C018_queue_element::getSize() const {
  return sizeof(*this) + packed.length();
}

bool C018_queue_element::isDuplicate(const C018_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.packed != packed)) {
    return false;
  }
  return true;
}

#endif // ifdef USES_C018

#include "../ControllerQueue/queue_element_single_value_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

queue_element_single_value_base::queue_element_single_value_base(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  valuesSent(0),
  valueCount(value_count) {}

queue_element_single_value_base::queue_element_single_value_base(queue_element_single_value_base&& rval)
  : idx(rval.idx), _timestamp(rval._timestamp),  TaskIndex(rval.TaskIndex),
  controller_idx(rval.controller_idx),
  valuesSent(rval.valuesSent), valueCount(rval.valueCount)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(rval.txt[i]);
  }
}

queue_element_single_value_base& queue_element_single_value_base::operator=(queue_element_single_value_base&& rval) {
  idx            = rval.idx;
  _timestamp     = rval._timestamp;
  TaskIndex      = rval.TaskIndex;
  controller_idx = rval.controller_idx;
  valuesSent     = rval.valuesSent;
  valueCount     = rval.valueCount;
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(rval.txt[i]);
  }
  return *this;
}

bool queue_element_single_value_base::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t queue_element_single_value_base::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool queue_element_single_value_base::isDuplicate(const queue_element_single_value_base& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.valueCount != valueCount) ||
      (other.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}

#include "../ControllerQueue/C011_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#ifdef USES_C011


C011_queue_element::C011_queue_element(const struct EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType) {}

size_t C011_queue_element::getSize() const {
  size_t total = sizeof(*this);

  total += uri.length();
  total += HttpMethod.length();
  total += header.length();
  total += postStr.length();
  return total;
}

bool C011_queue_element::isDuplicate(const C011_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.sensorType != sensorType) ||
      (other.idx != idx)) {
    return false;
  }
  return other.uri.equals(uri) &&
         other.HttpMethod.equals(HttpMethod) &&
         other.header.equals(header) &&
         other.postStr.equals(postStr);
}

#endif // ifdef USES_C011

#include "../ControllerQueue/SimpleQueueElement_string_only.h"


simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  String&& req) :
  txt(std::move(req)), TaskIndex(TaskIndex), controller_idx(ctrl_idx) {}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const simple_queue_element_string_only& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.txt != txt)) {
    return false;
  }
  return true;
}

#include "../ControllerQueue/C016_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Helpers/ESPEasy_math.h"

#ifdef USES_C016

C016_queue_element::C016_queue_element() : _timestamp(0), TaskIndex(INVALID_TASK_INDEX), controller_idx(0), sensorType(
    Sensor_VType::SENSOR_TYPE_NONE) {}

C016_queue_element::C016_queue_element(C016_queue_element&& other)
  : _timestamp(other._timestamp)
  , TaskIndex(other.TaskIndex)
  , controller_idx(other.controller_idx)
  , sensorType(other.sensorType)
  , valueCount(other.valueCount)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
}

C016_queue_element::C016_queue_element(const struct EventStruct *event, uint8_t value_count, unsigned long unixTime) :
  _timestamp(unixTime),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType),
  valueCount(value_count)
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (i < value_count) {
      values[i] = UserVar[event->BaseVarIndex + i];
    } else {
      values[i] = 0.0f;
    }
  }
}

C016_queue_element& C016_queue_element::operator=(C016_queue_element&& other) {
  _timestamp = other._timestamp;
  TaskIndex = other.TaskIndex;
  controller_idx = other.controller_idx;
  sensorType = other.sensorType;
  valueCount = other.valueCount;
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    values[i] = other.values[i];
  }
  return *this;
}

size_t C016_queue_element::getSize() const {
  return sizeof(*this);
}

bool C016_queue_element::isDuplicate(const C016_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.sensorType != sensorType) ||
      (other.valueCount != valueCount)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (!essentiallyEqual(other.values[i] , values[i])) {
      return false;
    }
  }
  return true;
}

#endif // ifdef USES_C016

#include "../ControllerQueue/queue_element_formatted_uservar.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"
#include "../../_Plugin_Helper.h"


queue_element_formatted_uservar::queue_element_formatted_uservar(queue_element_formatted_uservar&& other)
  :
  idx(other.idx),
  _timestamp(other._timestamp),
  TaskIndex(other.TaskIndex),
  controller_idx(other.controller_idx),
  sensorType(other.sensorType),
  valueCount(other.valueCount)
{
  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(other.txt[i]);
  }
}

queue_element_formatted_uservar::queue_element_formatted_uservar(EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType)
{
  valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

queue_element_formatted_uservar& queue_element_formatted_uservar::operator=(queue_element_formatted_uservar&& other) {
  idx            = other.idx;
  _timestamp     = other._timestamp;
  TaskIndex      = other.TaskIndex;
  controller_idx = other.controller_idx;
  sensorType     = other.sensorType;
  valueCount     = other.valueCount;

  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(other.txt[i]);
  }
  return *this;
}

size_t queue_element_formatted_uservar::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool queue_element_formatted_uservar::isDuplicate(const queue_element_formatted_uservar& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.sensorType != sensorType) ||
      (other.valueCount != valueCount)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}

#include "../ControllerQueue/MQTT_queue_element.h"

#ifdef USES_MQTT

MQTT_queue_element::MQTT_queue_element(int ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       const String& topic, const String& payload, bool retained) :
  _topic(topic), _payload(payload), TaskIndex(TaskIndex), controller_idx(ctrl_idx), _retained(retained)
{
  removeEmptyTopics();
}

MQTT_queue_element::MQTT_queue_element(int         ctrl_idx,
                                       taskIndex_t TaskIndex,
                                       String   && topic,
                                       String   && payload,
                                       bool        retained)
  :
  _topic(std::move(topic)), _payload(std::move(payload)), TaskIndex(TaskIndex), controller_idx(ctrl_idx), _retained(retained)
{
  removeEmptyTopics();
}

size_t MQTT_queue_element::getSize() const {
  return sizeof(*this) + _topic.length() + _payload.length();
}

bool MQTT_queue_element::isDuplicate(const MQTT_queue_element& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other._retained != _retained) ||
      other._topic != _topic ||
      other._payload != _payload) {
    return false;
  }
  return true;
}

void MQTT_queue_element::removeEmptyTopics() {
  // some parts of the topic may have been replaced by empty strings,
  // or "/status" may have been appended to a topic ending with a "/"
  // Get rid of "//"
  while (_topic.indexOf(F("//")) != -1) {
    _topic.replace(F("//"), F("/"));
  }
}
#endif // USES_MQTT

#include "../ControllerQueue/DelayQueueElements.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Helpers/PeriodicalActions.h"

#ifdef USES_MQTT
ControllerDelayHandlerStruct<MQTT_queue_element> *MQTTDelayHandler = nullptr;

bool init_mqtt_delay_queue(controllerIndex_t ControllerIndex, String& pubname, bool& retainFlag) {
  MakeControllerSettings(ControllerSettings); //-V522
  if (!AllocatedControllerSettings()) {
    return false;
  }
  LoadControllerSettings(ControllerIndex, ControllerSettings);
  if (MQTTDelayHandler == nullptr) {
    MQTTDelayHandler = new (std::nothrow) ControllerDelayHandlerStruct<MQTT_queue_element>;
  }
  if (MQTTDelayHandler == nullptr) {
    return false;
  }
  MQTTDelayHandler->configureControllerSettings(ControllerSettings);
  pubname = ControllerSettings.Publish;
  retainFlag = ControllerSettings.mqtt_retainFlag();
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
  scheduleNextMQTTdelayQueue();
  return true;
}

void exit_mqtt_delay_queue() {
  if (MQTTDelayHandler != nullptr) {
    delete MQTTDelayHandler;
    MQTTDelayHandler = nullptr;
  }
}

#endif // USES_MQTT


/*********************************************************************************************\
* C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
#ifdef USES_C001
# define C001_queue_element simple_queue_element_string_only
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  1)
#endif // ifdef USES_C001

/*********************************************************************************************\
* C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
#ifdef USES_C003
# define C003_queue_element simple_queue_element_string_only
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  3)
#endif // ifdef USES_C003

#ifdef USES_C004
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  4)
#endif // ifdef USES_C004

#ifdef USES_C007
# define C007_queue_element queue_element_formatted_uservar
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  7)
#endif // ifdef USES_C007



/*********************************************************************************************\
* C008_queue_element for queueing requests for 008: Generic HTTP
* Using queue_element_single_value_base
\*********************************************************************************************/
#ifdef USES_C008
# define C008_queue_element queue_element_single_value_base
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  8)
#endif // ifdef USES_C008

#ifdef USES_C009
# define C009_queue_element queue_element_formatted_uservar
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00,  9)
#endif // ifdef USES_C009


/*********************************************************************************************\
* C010_queue_element for queueing requests for 010: Generic UDP
* Using queue_element_single_value_base
\*********************************************************************************************/
#ifdef USES_C010
# define C010_queue_element queue_element_single_value_base
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP( 0, 10)
#endif // ifdef USES_C010



/*********************************************************************************************\
* C011_queue_element for queueing requests for 011: Generic HTTP Advanced
\*********************************************************************************************/
#ifdef USES_C011
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP( 0, 11)
#endif // ifdef USES_C011


/*********************************************************************************************\
* C012_queue_element for queueing requests for 012: Blynk
* Using queue_element_single_value_base
\*********************************************************************************************/
#ifdef USES_C012
# define C012_queue_element queue_element_single_value_base
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP( 0, 12)
#endif // ifdef USES_C012

/*
 #ifdef USES_C013
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 13)
 #endif
 */

/*
 #ifdef USES_C014
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 14)
 #endif
 */


#ifdef USES_C015
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 15)
#endif // ifdef USES_C015



#ifdef USES_C016
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 16)
#endif // ifdef USES_C016


#ifdef USES_C017
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 17)
#endif // ifdef USES_C017

#ifdef USES_C018
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 18)
#endif // ifdef USES_C018


/*
 #ifdef USES_C019
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 19)
 #endif
 */

/*
 #ifdef USES_C020
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 20)
 #endif
 */

/*
 #ifdef USES_C021
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 21)
 #endif
 */

/*
 #ifdef USES_C022
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 22)
 #endif
 */

/*
 #ifdef USES_C023
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 23)
 #endif
 */

/*
 #ifdef USES_C024
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 24)
 #endif
 */

/*
 #ifdef USES_C025
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 25)
 #endif
 */



// When extending this, search for EXTEND_CONTROLLER_IDS 
// in the code to find all places that need to be updated too.

