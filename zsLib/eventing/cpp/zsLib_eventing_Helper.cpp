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
  } // namespace eventing
} // namespace zsLib
