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

#include <cstdio>

namespace zsLib { namespace eventing { ZS_DECLARE_SUBSYSTEM(zsLib_eventing); } }


namespace zsLib
{
  namespace eventing
  {
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
    IEventingTypes::OperationalTypes IEventingTypes::toOperationalTypes(const char *type) throw (InvalidArgument)
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
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedTypedefs IEventingTypes::toPredefinedTypedef(const char *type)
    {
    }

    //-------------------------------------------------------------------------
    IEventingTypes::PredefinedTypedefs IEventingTypes::toPreferredPredefinedTypedef(PredefinedTypedefs type)
    {
    }

    //-------------------------------------------------------------------------
    IEventingTypes::BaseTypes IEventingTypes::getBaseType(PredefinedTypedefs type)
    {
    }

    //-------------------------------------------------------------------------
    bool IEventingTypes::isSigned(PredefinedTypedefs type)
    {
    }

    //-------------------------------------------------------------------------
    size_t IEventingTypes::getMinBits(PredefinedTypedefs type)
    {
    }

    //-------------------------------------------------------------------------
    size_t IEventingTypes::getMaxBits(PredefinedTypedefs type)
    {
    }

  } // namespace eventing
} // namespace zsLib
