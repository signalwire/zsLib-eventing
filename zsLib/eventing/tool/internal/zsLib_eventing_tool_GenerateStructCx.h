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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_IDLCompiler.h>

#include <sstream>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
        
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructCx
        #pragma mark

        struct GenerateStructCx : public IIDLCompilerTarget,
                                  public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          GenerateStructCx();

          static GenerateStructCxPtr create();

          static void insertFirst(
                                  std::stringstream &ss,
                                  bool &first
                                  );

          static void insertLast(
                                 std::stringstream &ss,
                                 bool &first
                                 );

          static String fixName(const String &originalName);

          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            const String &inIndentStr,
                                            NamespacePtr namespaceObj,
                                            bool outputEnums
                                            );

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) throw (Failure);

          static void caclculateDerives(
                                        StructPtr structObj,
                                        NamePathStructSetMap &ioDerivesInfo
                                        );

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark GenerateStructCx::IIDLCompilerTarget
          #pragma mark

          //-------------------------------------------------------------------
          virtual String targetKeyword() override;
          virtual String targetKeywordHelp() override;
          virtual void targetOutput(
                                    const String &inPathStr,
                                    const ICompilerTypes::Config &config
                                    ) throw (Failure) override;

        protected:
          struct HelperFile
          {
            std::stringstream mHeaderIncludeSS;
            std::stringstream mHeaderStructSS;
            std::stringstream mCppIncludeSS;
            std::stringstream mCppBodySS;
          };

        };
         
      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
