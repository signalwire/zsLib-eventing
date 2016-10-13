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

#pragma once

#include <zsLib/eventing/types.h>

#include <vector>
#include <map>

#include <iostream>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IOutputDelegate
      #pragma mark

      interaction IOutputDelegate
      {
        virtual void output(const char *str) const = 0;
        virtual void output(const wchar_t *str) const = 0;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StdOutputStream
      #pragma mark

      class StdOutputStream : IOutputDelegate
      {
        virtual void output(const char *str) const
        {
          std::cout << str;
        }
        virtual void output(const wchar_t *str) const
        {
          std::cout << str;
        }
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DebugOutputStream
      #pragma mark

      class DebugOutputStream : IOutputDelegate
      {
        //---------------------------------------------------------------------
        virtual void output(const char *str) const
        {
#ifdef _WIN32
          OutputDebugStringA(str);
#endif //_WIN32
        }

        //---------------------------------------------------------------------
        virtual void output(const wchar_t *str) const
        {
#ifdef _WIN32
          OutputDebugStringW(str);
#endif //_WIN32
        }
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark tool_basic_streambuf
      #pragma mark

      template < typename T, class CharTraits = std::char_traits< T > >
      class tool_basic_streambuf : private std::basic_streambuf< T, CharTraits >
      {
      public:
        typedef typename CharTraits::off_type off_type;
        typedef typename CharTraits::char_type char_type;
        typedef typename CharTraits::int_type int_type;
        typedef std::vector< typename T > BufferType;

      public:
        //---------------------------------------------------------------------
        tool_basic_streambuf(IOutputDelegate &outputer) :
          mOutputer(outputer)
        {
          _Init(NULL, NULL, NULL, &pBegin, &pCurrent, &pLength);
          m_outputBuffer.reserve(32);
        }

      protected:
        //---------------------------------------------------------------------
        virtual int_type overflow(int_type c = CharTraits::eof())
        {
          AutoRecursiveLock lock(mLock);

          if (c == CharTraits::eof())
            return CharTraits::not_eof(c);

          m_outputBuffer.push_back(c);
          if (c == TEXT('\n'))
            sync();

          return c;
        }

        //---------------------------------------------------------------------
        int sync()
        {
          AutoRecursiveLock lock(mLock);

          m_outputBuffer.push_back(TEXT('\0'));

          mOutputer.output(reinterpret_cast<const T*>(&m_outputBuffer[0]));

          m_outputBuffer.clear();
          m_outputBuffer.reserve(32);
          return 0;
        }

      protected:
        //---------------------------------------------------------------------
        RecursiveLock mLock;

        // put begin, put current and put length
        char_type* pBegin;
        char_type *pCurrent;
        int_type pLength;
        BufferType m_outputBuffer;
        IOutputDelegate &mOutputer;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark tool_basic_ostream
      #pragma mark

      template < typename T, class CharTraits = std::char_traits< T > >
      class tool_basic_ostream : public std::basic_ostream< T, CharTraits >,
                                 public IOutputDelegate
      {
      public:
        typedef tool_basic_streambuf< T, CharTraits > ToolStreamBuf;
        typedef std::basic_streambuf< T, CharTraits > StreamBuf;
        typedef std::basic_ostream< T, CharTraits > OStream;
        typedef std::map<PUID, IOutputDelegate> OutputDelegateMap;
        ZS_DECLARE_PTR(OutputDelegateMap);

      public:
        //---------------------------------------------------------------------
        tool_basic_ostream() : OStream((StreamBuf*)&m_streamBuffer) :
          mOutputs(make_shared<OutputDelegateMap>())
        {
          clear();
        }

        //---------------------------------------------------------------------
        void install(
                     PUID installID,
                     IOutputDelegatePtr delegate
                     )
        {
          if (0 == installID) return;
          AutoRecursiveLock lock(mLock);
          OutputDelegateMapPtr replacements(make_shared<OutputDelegateMap>(*mOutputs));
          replacements[installID] = delegate;
          mOutputs = replacements;
        }

        //---------------------------------------------------------------------
        void remove(PUID installID)
        {
          if (0 == installID) return;
          AutoRecursiveLock lock(mLock);
          OutputDelegateMapPtr replacements(make_shared<OutputDelegateMap>(*mOutputs));
          auto found = replacements->find(installID);
          if (found != replacements->end())
            replacements->erase(found);
          mOutputs = replacements;
        }

        //---------------------------------------------------------------------
        void installStdOutput()
        {
          AutoRecursiveLock lock(mLock);
          remove(mStdOutputInstall);
          auto stream = make_shared<StdOutputStream>();
          mStdOutputInstall = createPUID();
          install(mStdOutputInstall, stream);
        }

        //---------------------------------------------------------------------
        void installDebugger()
        {
#ifdef _DEBUG
          AutoRecursiveLock lock(mLock);
          remove(mDebugOutputInstall);
          auto stream = make_shared<DebugOutputStream>();
          mDebugOutputInstall = createPUID();
          install(mDebugOutputInstall, stream);
#endif //_DEBUG
        }

      protected:
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark tool_basic_ostream => IOutputDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void output(const char *str) const
        {
          OutputDelegateMapPtr temp;
          {
            AutoRecursiveLock lock(mLock);
            temp = mOutputs;
          }
          for (auto iter = mOutputs->begin(); iter != mOutputs->end(); ++iter)
          {
            (*iter).second->output(str);
          }
        }

        //---------------------------------------------------------------------
        virtual void output(const wchar_t *str) const
        {
          OutputDelegateMapPtr temp;
          {
            AutoRecursiveLock lock(mLock);
            temp = mOutputs;
          }
          for (auto iter = mOutputs->begin(); iter != mOutputs->end(); ++iter)
          {
            (*iter).second->output(str);
          }
        }

      protected:
        //---------------------------------------------------------------------
        RecursiveLock mLock;

        ToolStreamBuf m_streamBuffer;
        OutputDelegateMapPtr mOutputs;  // contents are non-mutable

        PUID mStdOutputInstall {};
        PUID mDebugOutputInstall {};
      };

      typedef tool_basic_ostream< char > tool_ostream;
      //typedef tool_basic_ostream< wchar_t > wdebugostream;
      //typedef tool_basic_ostream< TCHAR > tdebugostream; 

      tool_ostream &output();
    }
  }
}
