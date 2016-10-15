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

#include <zsLib/eventing/internal/zsLib_eventing_Helper.h>

#include <zsLib/Numeric.h>
#include <zsLib/Log.h>

#include <cstdio>

#include <cryptopp/hex.h>

namespace zsLib { namespace eventing { ZS_DECLARE_SUBSYSTEM(zsLib_eventing); } }


namespace zsLib
{
  namespace eventing
  {
    typedef CryptoPP::HexEncoder HexEncoder;
    typedef CryptoPP::StringSink StringSink;

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Helper
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params Helper::slog(const char *message)
      {
        return Log::Params(message, "eventing::Helper");
      }
    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IHelper
    #pragma mark

    //-------------------------------------------------------------------------
    SecureByteBlockPtr IHelper::loadFile(const char *path)
    {
      FILE *file = NULL;
      fopen_s(&file, path, "rb");
      if (NULL == file) return SecureByteBlockPtr();

      fseek(file, 0L, SEEK_END);
      auto size = ftell(file);
      fseek(file, 0L, SEEK_SET);

      auto buffer = make_shared<SecureByteBlock>(size);

      auto read = fread(buffer->BytePtr(), sizeof(BYTE), size, file);

      if (read != size) SecureByteBlockPtr();

      return buffer;
    }

    //-------------------------------------------------------------------------
    ElementPtr IHelper::read(const SecureByteBlockPtr buffer)
    {
      if (!buffer) return ElementPtr();
      return read(*buffer);
    }

    //-------------------------------------------------------------------------
    ElementPtr IHelper::read(const SecureByteBlock &buffer)
    {
      if (0 == buffer.size()) ElementPtr();
      DocumentPtr doc = Document::createFromAutoDetect(reinterpret_cast<const char *>(buffer.BytePtr()));
      ElementPtr result = doc->getFirstChildElement();
      if (!result) return ElementPtr();
      result->orphan();
      return result;
    }

    //-------------------------------------------------------------------------
    String IHelper::getElementText(const ElementPtr &el)
    {
      if (!el) return String();
      return el->getText();
    }

    //-------------------------------------------------------------------------
    String IHelper::getElementTextAndDecode(const ElementPtr &el)
    {
      if (!el) return String();
      return el->getTextDecoded();
    }

    //-----------------------------------------------------------------------
    ElementPtr IHelper::createElementWithText(
                                              const String &elName,
                                              const String &text
                                              )
    {
      ElementPtr tmp = Element::create(elName);

      if (text.isEmpty()) return tmp;

      TextPtr tmpTxt = Text::create();
      tmpTxt->setValue(text, Text::Format_JSONStringEncoded);

      tmp->adoptAsFirstChild(tmpTxt);

      return tmp;
    }

    //-----------------------------------------------------------------------
    ElementPtr IHelper::createElementWithNumber(
                                                const String &elName,
                                                const String &numberAsStringValue
                                                )
    {
      ElementPtr tmp = Element::create(elName);

      if (numberAsStringValue.isEmpty()) {
        TextPtr tmpTxt = Text::create();
        tmpTxt->setValue("0", Text::Format_JSONNumberEncoded);
        tmp->adoptAsFirstChild(tmpTxt);
        return tmp;
      }

      TextPtr tmpTxt = Text::create();
      tmpTxt->setValue(numberAsStringValue, Text::Format_JSONNumberEncoded);
      tmp->adoptAsFirstChild(tmpTxt);

      return tmp;
    }

    //-----------------------------------------------------------------------
    ElementPtr IHelper::createElementWithTime(
      const String &elName,
      Time time
    )
    {
      return createElementWithNumber(elName, IHelper::timeToString(time));
    }

    //-----------------------------------------------------------------------
    ElementPtr IHelper::createElementWithTextAndJSONEncode(
                                                           const String &elName,
                                                           const String &textVal
                                                           )
    {
      ElementPtr tmp = Element::create(elName);
      if (textVal.isEmpty()) return tmp;

      TextPtr tmpTxt = Text::create();
      tmpTxt->setValueAndJSONEncode(textVal);
      tmp->adoptAsFirstChild(tmpTxt);
      return tmp;
    }

    //-----------------------------------------------------------------------
    String IHelper::timeToString(const Time &value)
    {
      if (Time() == value) return String();
      return string(value);
    }

    //-----------------------------------------------------------------------
    Time IHelper::stringToTime(const String &str)
    {
      if (str.isEmpty()) return Time();
      if ("0" == str) return Time();

      try {
        return Numeric<Time>(str);
      } catch (const Numeric<Time>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Detail, internal::Helper::slog("unable to convert value to time") + ZS_PARAM("value", str))
      }

      return Time();
    }

    //-----------------------------------------------------------------------
    String IHelper::convertToHex(
                                 const BYTE *buffer,
                                 size_t bufferLengthInBytes,
                                 bool outputUpperCase
                                 )
    {
      String result;

      HexEncoder encoder(new StringSink(result), outputUpperCase);
      encoder.Put(buffer, bufferLengthInBytes);
      encoder.MessageEnd();

      return result;
    }

    //-----------------------------------------------------------------------
    String IHelper::convertToHex(
                                 const SecureByteBlock &input,
                                 bool outputUpperCase
                                 )
    {
      return convertToHex(input, input.size(), outputUpperCase);
    }

  } // namespace eventing
} // namespace zsLib
