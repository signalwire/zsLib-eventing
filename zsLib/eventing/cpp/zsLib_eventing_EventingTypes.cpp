/*

Copyright (c) 2016, Robin Raymond
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

*/

#include <zsLib/eventing/internal/zsLib_eventing_EventingTypes.h>
#include <zsLib/eventing/internal/zsLib_eventing_Helper.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>

#include <cstdio>

namespace zsLib { namespace eventing { ZS_DECLARE_SUBSYSTEM(zsLib_eventing); } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseEventingHelper);

    namespace internal
    {
    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::OperationalTypes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IEventingTypes::toString(OperationalTypes type)
    {
      switch (type)
      {
        case OperationalType_Admin:       return "Admin";
        case OperationalType_Operational: return "Operational";
        case OperationalType_Analytic:    return "Analytic";
        case OperationalType_Debug:       return "Debug";
      }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    IEventingTypes::OperationalTypes IEventingTypes::toOperationalType(const char *type) throw (InvalidArgument)
    {
      String str(type);
      for (IEventingTypes::OperationalTypes index = IEventingTypes::OperationalType_First; index <= IEventingTypes::OperationalType_Last; index = static_cast<IEventingTypes::OperationalTypes>(static_cast<std::underlying_type<IEventingTypes::OperationalTypes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IEventingTypes::toString(index))) return index;
      }

      ZS_THROW_INVALID_ARGUMENT(String("Operational type is not valid: ") + str);
      return OperationalType_First;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::PredefinedOpCodes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IEventingTypes::toString(PredefinedOpCodes code)
    {
      switch (code)
      {
        case PredefinedOpCode_Info:       return "Info";
        case PredefinedOpCode_Start:      return "Start";
        case PredefinedOpCode_Stop:       return "Stop";
        case PredefinedOpCode_DC_Start:   return "DC_Start";
        case PredefinedOpCode_DC_Stop:    return "DC_Stop";
        case PredefinedOpCode_Extension:  return "Extension";
        case PredefinedOpCode_Reply:      return "Reply";
        case PredefinedOpCode_Resume:     return "Resume";
        case PredefinedOpCode_Suspend:    return "Suspend";
        case PredefinedOpCode_Send:       return "Send";
        case PredefinedOpCode_Receive:    return "Receive";
      }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedOpCodes IEventingTypes::toPredefinedOpCode(const char *code) throw (InvalidArgument)
    {
      String str(code);
      for (IEventingTypes::PredefinedOpCodes index = IEventingTypes::PredefinedOpCode_First; index <= IEventingTypes::PredefinedOpCode_Last; index = static_cast<IEventingTypes::PredefinedOpCodes>(static_cast<std::underlying_type<IEventingTypes::PredefinedOpCodes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IEventingTypes::toString(index))) return index;
      }

      ZS_THROW_INVALID_ARGUMENT(String("Not a predefined op code: ") + str);
      return PredefinedOpCode_First;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::PredefinedLevels
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IEventingTypes::toString(PredefinedLevels level)
    {
      switch (level)
      {
        case PredefinedLevel_Critical:      return "Critical";
        case PredefinedLevel_Error:         return "Error";
        case PredefinedLevel_Warning:       return "Warning";
        case PredefinedLevel_Informational: return "Informational";
        case PredefinedLevel_Verbose:       return "Verbose";
       }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedLevels IEventingTypes::toPredefinedLevel(const char *level) throw (InvalidArgument)
    {
      String str(level);
      for (IEventingTypes::PredefinedLevels index = IEventingTypes::PredefinedLevel_First; index <= IEventingTypes::PredefinedLevel_Last; index = static_cast<IEventingTypes::PredefinedLevels>(static_cast<std::underlying_type<IEventingTypes::PredefinedLevels>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IEventingTypes::toString(index))) return index;
      }

      ZS_THROW_INVALID_ARGUMENT(String("Not a predefined level: ") + str);
      return PredefinedLevel_First;
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedLevels IEventingTypes::toPredefinedLevel(
                                                                       Log::Severity severity,
                                                                       Log::Level level
                                                                       )
    {
      switch (severity)
      {
        case Log::Fatal:    return PredefinedLevel_Critical;
        case Log::Error:    return PredefinedLevel_Error;
        case Log::Warning:  return PredefinedLevel_Warning;
        default:            break;
      }

      switch (level)
      {
        case Log::Basic:    return PredefinedLevel_Informational;
        case Log::Detail:   return PredefinedLevel_Informational;
        case Log::Debug:    return PredefinedLevel_Informational;
        default:            break;
      }
      return PredefinedLevel_Verbose;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::BaseTypes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IEventingTypes::toString(BaseTypes type)
    {
      switch (type)
      {
        case BaseType_Boolean:    return "boolean";
        case BaseType_Integer:    return "integer";
        case BaseType_Float:      return "float";
        case BaseType_Pointer:    return "pointer";
        case BaseType_Binary:     return "binary";
        case BaseType_String:     return "string";
      }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    IEventingTypes::BaseTypes IEventingTypes::toBaseType(const char *type) throw (InvalidArgument)
    {
      String str(type);
      for (IEventingTypes::BaseTypes index = IEventingTypes::BaseType_First; index <= IEventingTypes::BaseType_Last; index = static_cast<IEventingTypes::BaseTypes>(static_cast<std::underlying_type<IEventingTypes::BaseTypes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IEventingTypes::toString(index))) return index;
      }

      ZS_THROW_INVALID_ARGUMENT(String("Not a base type: ") + str);
      return BaseType_First;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::PredefinedTypedefs
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IEventingTypes::toString(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_bool:      return "bool";

        case PredefinedTypedef_uchar:     return "uchar";
        case PredefinedTypedef_char:      return "char";
        case PredefinedTypedef_schar:     return "schar";
        case PredefinedTypedef_ushort:    return "ushort";
        case PredefinedTypedef_short:     return "short";
        case PredefinedTypedef_sshort:    return "sshort";
        case PredefinedTypedef_uint:      return "uint";
        case PredefinedTypedef_int:       return "int";
        case PredefinedTypedef_sint:      return "sint";
        case PredefinedTypedef_ulong:     return "ulong";
        case PredefinedTypedef_long:      return "long";
        case PredefinedTypedef_slong:     return "slong";
        case PredefinedTypedef_ulonglong: return "ulonglong";
        case PredefinedTypedef_longlong:  return "longlong";
        case PredefinedTypedef_slonglong: return "slonglong";

        case PredefinedTypedef_uint8:     return "uint8";
        case PredefinedTypedef_int8:      return "int8";
        case PredefinedTypedef_sint8:     return "sint8";
        case PredefinedTypedef_uint16:    return "uint16";
        case PredefinedTypedef_int16:     return "int16";
        case PredefinedTypedef_sint16:    return "sint16";
        case PredefinedTypedef_uint32:    return "uint32";
        case PredefinedTypedef_int32:     return "int32";
        case PredefinedTypedef_sint32:    return "sint32";
        case PredefinedTypedef_uint64:    return "uint64";
        case PredefinedTypedef_int64:     return "int64";
        case PredefinedTypedef_sint64:    return "sint64";

        case PredefinedTypedef_byte:      return "byte";
        case PredefinedTypedef_word:      return "word";
        case PredefinedTypedef_dword:     return "dword";
        case PredefinedTypedef_qword:     return "qword";

        case PredefinedTypedef_float:     return "float";
        case PredefinedTypedef_double:    return "double";
        case PredefinedTypedef_ldouble:   return "ldouble";
        case PredefinedTypedef_float32:   return "float32";
        case PredefinedTypedef_float64:   return "float64";

        case PredefinedTypedef_pointer:   return "pointer";

        case PredefinedTypedef_binary:    return "binary";
        case PredefinedTypedef_size:      return "size";

        case PredefinedTypedef_string:    return "string";
        case PredefinedTypedef_astring:   return "astring";
        case PredefinedTypedef_wstring:   return "wstring";
      }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedTypedefs IEventingTypes::toPredefinedTypedef(const char *type) throw (InvalidArgument)
    {
      String str(type);
      for (IEventingTypes::PredefinedTypedefs index = IEventingTypes::PredefinedTypedef_First; index <= IEventingTypes::PredefinedTypedef_Last; index = static_cast<IEventingTypes::PredefinedTypedefs>(static_cast<std::underlying_type<IEventingTypes::PredefinedTypedefs>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IEventingTypes::toString(index))) return index;
      }

      ZS_THROW_INVALID_ARGUMENT(String("Not a predefined type: ") + str);
      return PredefinedTypedef_First;
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedTypedefs IEventingTypes::toPreferredPredefinedTypedef(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_schar:     return PredefinedTypedef_char;
        case PredefinedTypedef_sshort:    return PredefinedTypedef_short;
        case PredefinedTypedef_sint:      return PredefinedTypedef_int;
        case PredefinedTypedef_slong:     return PredefinedTypedef_long;
        case PredefinedTypedef_slonglong: return PredefinedTypedef_longlong;

        case PredefinedTypedef_sint8:     return PredefinedTypedef_int8;
        case PredefinedTypedef_sint16:    return PredefinedTypedef_int16;
        case PredefinedTypedef_sint32:    return PredefinedTypedef_int32;
        case PredefinedTypedef_sint64:    return PredefinedTypedef_int64;

        case PredefinedTypedef_byte:      return PredefinedTypedef_uint8;
        case PredefinedTypedef_word:      return PredefinedTypedef_uint16;
        case PredefinedTypedef_dword:     return PredefinedTypedef_uint32;
        case PredefinedTypedef_qword:     return PredefinedTypedef_uint64;

        case PredefinedTypedef_astring:   PredefinedTypedef_string;
      }
      return type;
    }

    //-------------------------------------------------------------------------
    IEventingTypes::BaseTypes IEventingTypes::getBaseType(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_bool:      return BaseType_Boolean;

        case PredefinedTypedef_uchar:
        case PredefinedTypedef_char:
        case PredefinedTypedef_schar:
        case PredefinedTypedef_ushort:
        case PredefinedTypedef_short:
        case PredefinedTypedef_sshort:
        case PredefinedTypedef_uint:
        case PredefinedTypedef_int:
        case PredefinedTypedef_sint:
        case PredefinedTypedef_ulong:
        case PredefinedTypedef_long:
        case PredefinedTypedef_slong:
        case PredefinedTypedef_ulonglong:
        case PredefinedTypedef_longlong:
        case PredefinedTypedef_slonglong:

        case PredefinedTypedef_uint8:
        case PredefinedTypedef_int8:
        case PredefinedTypedef_sint8:
        case PredefinedTypedef_uint16:
        case PredefinedTypedef_int16:
        case PredefinedTypedef_sint16:
        case PredefinedTypedef_uint32:
        case PredefinedTypedef_int32:
        case PredefinedTypedef_sint32:
        case PredefinedTypedef_uint64:
        case PredefinedTypedef_int64:
        case PredefinedTypedef_sint64:

        case PredefinedTypedef_byte:
        case PredefinedTypedef_word:
        case PredefinedTypedef_dword:
        case PredefinedTypedef_qword:     return BaseType_Integer;

        case PredefinedTypedef_float:
        case PredefinedTypedef_double:
        case PredefinedTypedef_ldouble:
        case PredefinedTypedef_float32:
        case PredefinedTypedef_float64:   return BaseType_Float;

        case PredefinedTypedef_pointer:   return BaseType_Pointer;

        case PredefinedTypedef_binary:    return BaseType_Binary;
        case PredefinedTypedef_size:      return BaseType_Integer;

        case PredefinedTypedef_string:
        case PredefinedTypedef_astring:
        case PredefinedTypedef_wstring:   return BaseType_String;
      }

      ZS_THROW_NOT_IMPLEMENTED(String("Missing base type conversion for predefined type:") + toString(type));
    }

    //-------------------------------------------------------------------------
    bool IEventingTypes::isSigned(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_bool:

        case PredefinedTypedef_uchar:
        case PredefinedTypedef_ushort:
        case PredefinedTypedef_uint:
        case PredefinedTypedef_ulong:
        case PredefinedTypedef_ulonglong:
        case PredefinedTypedef_uint8:
        case PredefinedTypedef_uint16:
        case PredefinedTypedef_uint32:
        case PredefinedTypedef_uint64:
        case PredefinedTypedef_byte:
        case PredefinedTypedef_word:
        case PredefinedTypedef_dword:
        case PredefinedTypedef_qword:     return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool IEventingTypes::isUnsigned(PredefinedTypedefs type)
    {
      return !isSigned(type);
    }

    //-------------------------------------------------------------------------
    size_t IEventingTypes::getMinBytes(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_bool:      return sizeof(bool);

        case PredefinedTypedef_uchar:     return sizeof(char);
        case PredefinedTypedef_char:      return sizeof(char);
        case PredefinedTypedef_schar:     return sizeof(char);
        case PredefinedTypedef_ushort:    return sizeof(short);
        case PredefinedTypedef_short:     return sizeof(short);
        case PredefinedTypedef_sshort:    return sizeof(short);
        case PredefinedTypedef_uint:      return sizeof(int);
        case PredefinedTypedef_int:       return sizeof(int);
        case PredefinedTypedef_sint:      return sizeof(int);
        case PredefinedTypedef_ulong:     return sizeof(int);
        case PredefinedTypedef_long:      return sizeof(int);
        case PredefinedTypedef_slong:     return sizeof(int);
        case PredefinedTypedef_ulonglong: return sizeof(long long);
        case PredefinedTypedef_longlong:  return sizeof(long long);
        case PredefinedTypedef_slonglong: return sizeof(long long);

        case PredefinedTypedef_uint8:     return 1;
        case PredefinedTypedef_int8:      return 1;
        case PredefinedTypedef_sint8:     return 1;
        case PredefinedTypedef_uint16:    return 2;
        case PredefinedTypedef_int16:     return 2;
        case PredefinedTypedef_sint16:    return 2;
        case PredefinedTypedef_uint32:    return 4;
        case PredefinedTypedef_int32:     return 4;
        case PredefinedTypedef_sint32:    return 4;
        case PredefinedTypedef_uint64:    return 8;
        case PredefinedTypedef_int64:     return 8;
        case PredefinedTypedef_sint64:    return 8;

        case PredefinedTypedef_byte:      return 1;
        case PredefinedTypedef_word:      return 2;
        case PredefinedTypedef_dword:     return 4;
        case PredefinedTypedef_qword:     return 8;

        case PredefinedTypedef_float:     return sizeof(float);
        case PredefinedTypedef_double:    return sizeof(float);
        case PredefinedTypedef_ldouble:   return sizeof(double);
        case PredefinedTypedef_float32:   return 4;
        case PredefinedTypedef_float64:   return 8;

        case PredefinedTypedef_pointer:   return sizeof(int);

        case PredefinedTypedef_binary:    return 0;
        case PredefinedTypedef_size:      return sizeof(int);

        case PredefinedTypedef_string:    return 0;
        case PredefinedTypedef_astring:   return 0;
        case PredefinedTypedef_wstring:   return 0;
      }

      return 0;
    }

    //-------------------------------------------------------------------------
    size_t IEventingTypes::getMaxBytes(PredefinedTypedefs type)
    {
      switch (type)
      {
        case PredefinedTypedef_bool:      return sizeof(bool);

        case PredefinedTypedef_uchar:     return sizeof(char);
        case PredefinedTypedef_char:      return sizeof(char);
        case PredefinedTypedef_schar:     return sizeof(char);
        case PredefinedTypedef_ushort:    return sizeof(short);
        case PredefinedTypedef_short:     return sizeof(short);
        case PredefinedTypedef_sshort:    return sizeof(short);
        case PredefinedTypedef_uint:      return sizeof(int);
        case PredefinedTypedef_int:       return sizeof(int);
        case PredefinedTypedef_sint:      return sizeof(int);
        case PredefinedTypedef_ulong:     return sizeof(long long);
        case PredefinedTypedef_long:      return sizeof(long long);
        case PredefinedTypedef_slong:     return sizeof(long long);
        case PredefinedTypedef_ulonglong: return sizeof(long long);
        case PredefinedTypedef_longlong:  return sizeof(long long);
        case PredefinedTypedef_slonglong: return sizeof(long long);

        case PredefinedTypedef_uint8:     return 1;
        case PredefinedTypedef_int8:      return 1;
        case PredefinedTypedef_sint8:     return 1;
        case PredefinedTypedef_uint16:    return 2;
        case PredefinedTypedef_int16:     return 2;
        case PredefinedTypedef_sint16:    return 2;
        case PredefinedTypedef_uint32:    return 4;
        case PredefinedTypedef_int32:     return 4;
        case PredefinedTypedef_sint32:    return 4;
        case PredefinedTypedef_uint64:    return 8;
        case PredefinedTypedef_int64:     return 8;
        case PredefinedTypedef_sint64:    return 8;

        case PredefinedTypedef_byte:      return 1;
        case PredefinedTypedef_word:      return 2;
        case PredefinedTypedef_dword:     return 4;
        case PredefinedTypedef_qword:     return 8;

        case PredefinedTypedef_float:     return sizeof(float);
        case PredefinedTypedef_double:    return sizeof(long double);
        case PredefinedTypedef_ldouble:   return sizeof(long double);
        case PredefinedTypedef_float32:   return 4;
        case PredefinedTypedef_float64:   return 8;

        case PredefinedTypedef_pointer:   return sizeof(long long);

        case PredefinedTypedef_binary:    return 0;
        case PredefinedTypedef_size:      return sizeof(long long);

        case PredefinedTypedef_string:    return 0;
        case PredefinedTypedef_astring:   return 0;
        case PredefinedTypedef_wstring:   return 0;
      }

      return 0;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::Provider
    #pragma mark

    //-------------------------------------------------------------------------
    IEventingTypes::Provider::Provider(const ElementPtr &rootEl)
    {
      createTypesdefs(rootEl->findFirstChildElement("typedefs"), mTypedefs);
    }

    //-------------------------------------------------------------------------
    ElementPtr IEventingTypes::Provider::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "provider";

      ElementPtr rootEl = Element::create(objectName);

      rootEl->adoptAsLastChild(UseEventingHelper::createElementWithText("id", string(mID)));
      rootEl->adoptAsLastChild(UseEventingHelper::createElementWithTextAndJSONEncode("name", mName));
      rootEl->adoptAsLastChild(UseEventingHelper::createElementWithTextAndJSONEncode("resourceName", mResourceName));

      if (mTypedefs.size() > 0) {
        ElementPtr typedefsEl = Element::create("typedefs");
        for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter) {
          ElementPtr typedefEl = (*iter).second->createElement("typedef");
          typedefsEl->adoptAsLastChild(typedefEl);
        }
        rootEl->adoptAsLastChild(typedefsEl);
      }

      if (mChannels.size() > 0) {
        ElementPtr channelsEl = Element::create("channels");
        for (auto iter = mChannels.begin(); iter != mChannels.end(); ++iter) {
          ElementPtr channelEl = (*iter).second->createElement("channel");
          channelsEl->adoptAsLastChild(channelEl);
        }
        rootEl->adoptAsLastChild(channelsEl);
      }

      if (mOpCodes.size() > 0) {
        ElementPtr opCodesEl = Element::create("opcodes");
        for (auto iter = mOpCodes.begin(); iter != mOpCodes.end(); ++iter) {
          ElementPtr opCodeEl = (*iter).second->createElement("opcode");
          opCodesEl->adoptAsLastChild(opCodeEl);
        }
        rootEl->adoptAsLastChild(opCodesEl);
      }

      if (mTasks.size() > 0) {
        ElementPtr tasksEl = Element::create("tasks");
        for (auto iter = mTasks.begin(); iter != mTasks.end(); ++iter) {
          ElementPtr taskEl = (*iter).second->createElement("task");
          tasksEl->adoptAsLastChild(taskEl);
        }
        rootEl->adoptAsLastChild(tasksEl);
      }

      if (mEvents.size() > 0) {
        ElementPtr eventsEl = Element::create("events");
        for (auto iter = mEvents.begin(); iter != mEvents.end(); ++iter) {
          ElementPtr eventEl = (*iter).second->createElement("event");
          eventsEl->adoptAsLastChild(eventEl);
        }
        rootEl->adoptAsLastChild(eventsEl);
      }

      if (mTemplates.size() > 0) {
        ElementPtr templatesEl = Element::create("templates");
        for (auto iter = mTemplates.begin(); iter != mTemplates.end(); ++iter) {
          ElementPtr templateEl = (*iter).second->createElement("template");
          templatesEl->adoptAsLastChild(templateEl);
        }
        rootEl->adoptAsLastChild(templatesEl);
      }

      return rootEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::Typedef
    #pragma mark

    //-------------------------------------------------------------------------
    void IEventingTypes::createTypesdefs(
                                         ElementPtr typedefsEl,
                                         TypedefMap &outTypedefs
                                         )
    {
      typedef std::map<String, String> StringMap;

      if (!typedefsEl) return;

      StringMap typedefs;

      ElementPtr typedefEl = typedefsEl->findFirstChildElement("typedef");
      while (typedefEl) {
        String name = UseEventingHelper::getElementTextAndDecode(typedefEl->findFirstChildElement("name"));
        String type = UseEventingHelper::getElementTextAndDecode(typedefEl->findFirstChildElement("type"));

        if ((name.isEmpty()) ||
          (type.isEmpty())) continue;

        name.toLower();
        type.toLower();

        typedefs[name] = type;

        typedefEl = typedefEl->findNextSiblingElement("typedef");
      }

      StringMap tempTypedefs(typedefs);

      while (tempTypedefs.size() > 0) {
        StringMap processedTypedefs;

        auto iter = tempTypedefs.begin();

        String name = (*iter).first;
        String type = (*iter).second;

        tempTypedefs.erase(iter);

        do {
          try {
            auto predefined = IEventingTypes::toPredefinedTypedef(type);

            auto typedefObj = make_shared<IEventingTypes::Typedef>();
            typedefObj->mName = name;
            typedefObj->mType = IEventingTypes::toPreferredPredefinedTypedef(predefined);
            break;
          } catch (const InvalidArgument &) {
            // valid case - ignored
          }

          auto found = typedefs.find(type);
          if (found == typedefs.end()) {
            ZS_THROW_CUSTOM(InvalidContent, String("Typedef not mapped to predefined type: ") + type);
          }

          auto foundProcessed = processedTypedefs.find(type);
          if (foundProcessed != processedTypedefs.end()) {
            ZS_THROW_CUSTOM(InvalidContent, String("Circular typedef not mapped to predefined type: ") + type);
          }

          processedTypedefs[type] = String();
        } while (true);
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::Channel
    #pragma mark

    //-------------------------------------------------------------------------
    IEventingTypes::Channel::Channel(const ElementPtr &rootEl) throw (InvalidContent)
    {
      mID = UseEventingHelper::getElementTextAndDecode(rootEl->findLastChildElement("id"));
      mName = UseEventingHelper::getElementTextAndDecode(rootEl->findLastChildElement("name"));
      String type = UseEventingHelper::getElementTextAndDecode(rootEl->findLastChildElement("type"));
      String value = UseEventingHelper::getElementText(rootEl->findLastChildElement("value"));

      if (type.hasData()) {
        try {
          mType = IEventingTypes::toOperationalType(type);
        } catch (const InvalidArgument &) {
          ZS_THROW_CUSTOM(InvalidContent, String("Channel operational type is invalid: ") + type);
        }
      }

      if (value.hasData()) {
        try {
          mValue = Numeric<decltype(mValue)>(value);
        } catch (const Numeric<decltype(mValue)>::ValueOutOfRange &) {
          ZS_THROW_CUSTOM(InvalidContent, String("Channel value is out of range: ") + value);
        }
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr IEventingTypes::Channel::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "channel";

      ElementPtr channelEl = Element::create(objectName);
      
      if (mID.hasData()) channelEl->adoptAsLastChild(UseEventingHelper::createElementWithText("id", mID));
      if (mName.hasData()) channelEl->adoptAsLastChild(UseEventingHelper::createElementWithText("name", mName));
      channelEl->adoptAsLastChild(UseEventingHelper::createElementWithText("name", IEventingTypes::toString(mType)));
      if (0 != mValue) channelEl->adoptAsLastChild(UseEventingHelper::createElementWithNumber("name", string(mValue)));

      return channelEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes::Task
    #pragma mark

    //-------------------------------------------------------------------------
    IEventingTypes::Task::Task(const ElementPtr &rootEl)
    {
      mName = UseEventingHelper::getElementTextAndDecode(rootEl->findFirstChildElement("name"));
      String value = UseEventingHelper::getElementTextAndDecode(rootEl->findFirstChildElement("value"));

      if (value.hasData()) {
        try {
          mValue = Numeric<decltype(mValue)>(value);
        } catch (const Numeric<decltype(mValue)>::ValueOutOfRange &) {
          ZS_THROW_CUSTOM(InvalidContent, String("Task value is out of range: ") + value);
        }
      }

      createOpCodes(rootEl->findFirstChildElement("opCodes"), mOpCodes);
    }

    //-------------------------------------------------------------------------
    ElementPtr IEventingTypes::Task::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "channel";

      ElementPtr channelEl = Element::create(objectName);

      if (mName.hasData()) channelEl->adoptAsLastChild(UseEventingHelper::createElementWithTextAndJSONEncode("name", mName));
      if (0 != mValue) channelEl->adoptAsLastChild(UseEventingHelper::createElementWithNumber("value", string(mValue)));

      if (mOpCodes.size() > 0) {
        ElementPtr opCodesEl = Element::create("opCodes");

        for (auto iter = mOpCodes.begin(); iter != mOpCodes.end(); ++iter) {
          ElementPtr opCode = (*iter).second->createElement("opCode");
          opCodesEl->adoptAsLastChild(opCode);
        }
      }

      return channelEl;
    }

  } // namespace eventing
} // namespace zsLib
