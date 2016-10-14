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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Compiler.h>

#include <zsLib/eventing/IHelper.h>

#include <zsLib/Exception.h>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zsLib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseEventingHelper);

      namespace internal
      {
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark Compiler
        #pragma mark

        //-----------------------------------------------------------------------
        Compiler::Compiler(
                         const make_private &,
                         const Config &config
                         ) :
          mConfig(config)
        {
        }

        //-----------------------------------------------------------------------
        Compiler::~Compiler()
        {
        }

        //-----------------------------------------------------------------------
        CompilerPtr Compiler::create(const Config &config)
        {
          CompilerPtr pThis(std::make_shared<Compiler>(make_private{}, config));
          pThis->mThisWeak = pThis;
          return pThis;
        }

        //-----------------------------------------------------------------------
        void Compiler::process() throw (Failure)
        {
        }

      } // namespace internal

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark ICompiler
      #pragma mark

      //-------------------------------------------------------------------------
      void ICompiler::prepare(
                              const char *configFile,
                              Config &outConfig
                              )
      {
        auto rootEl = UseEventingHelper::read(UseEventingHelper::loadFile(configFile));

        ElementPtr sourcesEl = rootEl->findFirstChildElement("sources");
        if (sourcesEl) {
          ElementPtr sourceEl = sourcesEl->findFirstChildElement("source");
          while (sourceEl) {
            auto source = UseEventingHelper::getElementTextAndDecode(sourceEl);

            if (source.hasData()) {
              outConfig.mSources.push_back(source);
            }
            sourceEl = sourceEl->findNextSiblingElement("source");
          }
        }
      }

      //-------------------------------------------------------------------------
      ICompilerPtr ICompiler::create(const Config &config)
      {
        return internal::Compiler::create(config);
      }

    } // namespace tool
  } // namespace eventing
} // namespace zsLib
