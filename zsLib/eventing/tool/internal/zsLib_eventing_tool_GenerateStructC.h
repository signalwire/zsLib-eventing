/*

Copyright (c) 2017, Robin Raymond
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
        #pragma mark GenerateStructC
        #pragma mark

        struct GenerateStructC : public IIDLCompilerTarget,
                                  public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          struct HelperFile
          {
            NamespacePtr global_;
            NamePathStructSetMap derives_;

            String headerFileName_;
            String cppFileName_;

            std::stringstream headerCIncludeSS_;
            std::stringstream headerCFunctionsSS_;
            std::stringstream headerCppIncludeSS_;
            std::stringstream headerCppFunctionsSS_;

            std::stringstream cIncludeSS_;
            std::stringstream cFunctionsSS_;

            std::stringstream cppIncludeSS_;
            std::stringstream cppFunctionsSS_;

            StringSet headerCAlreadyIncluded_;
            StringSet headerCppAlreadyIncluded_;
            StringSet cAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            void headerIncludeC(const String &headerFile);
            void headerIncludeCpp(const String &headerFile);
            void includeC(const String &headerFile);
            void includeCpp(const String &headerFile);
          };

          struct StructFile
          {
            StructPtr struct_;

            std::stringstream headerCIncludeSS_;
            std::stringstream headerCFunctionsSS_;
            std::stringstream headerCppIncludeSS_;
            std::stringstream headerCppFunctionsSS_;

            std::stringstream cIncludeSS_;
            std::stringstream cFunctionsSS_;
            std::stringstream cppIncludeSS_;
            std::stringstream cppFunctionsSS_;

            StringSet headerCAlreadyIncluded_;
            StringSet headerCppAlreadyIncluded_;
            StringSet cAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            void headerIncludeC(const String &headerFile);
            void headerIncludeCpp(const String &headerFile);
            void includeC(const String &headerFile);
            void includeCpp(const String &headerFile);
          };

          GenerateStructC();

          static GenerateStructCPtr create();

          static String fixCType(IEventingTypes::PredefinedTypedefs type);
          static String fixCType(TypePtr type);
          static String fixType(TypePtr type);
          static String getApiImplementationDefine(ContextPtr context);
          static String getApiExportDefine(ContextPtr context);
          static String getApiCallingDefine(ContextPtr context);
          static String getApiGuardDefine(
                                          ContextPtr context,
                                          bool endGuard = false
                                          );
          static void includeType(
                                  HelperFile &helperFile,
                                  TypePtr type
                                  );

          static void calculateRelations(
                                         NamespacePtr namespaceObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         );
          static void calculateRelations(
                                         StructPtr structObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         );

          static void insertInto(
                                 StructPtr structObj,
                                 const NamePath &namePath,
                                 NamePathStructSetMap &ioDerivesInfo
                                 );

          static void appendStream(
                                   std::stringstream &output,
                                   std::stringstream &source,
                                   bool appendEol = true
                                   );

          static void prepareHelperFile(HelperFile &helperFile);
          static void prepareHelperCallback(HelperFile &helperFile);
          static void prepareHelperString(HelperFile &helperFile);
          static void prepareHelperDuration(
                                            HelperFile &helperFile,
                                            const String &durationType
                                            );
          static void prepareHelperList(
                                        HelperFile &helperFile,
                                        const String &listOrSetStr
                                        );
          static void prepareHelperSpecial(
                                           HelperFile &helperFile,
                                           const String specialName
                                           );
          static void preparePromiseWithValue(HelperFile &helperFile);
          static void preparePromiseWithRejectionReason(HelperFile &helperFile);

          static void finalizeHelperFile(HelperFile &helperFile);

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) throw (Failure);
          
          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            NamespacePtr namespaceObj
                                            );
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         StructPtr structObj
                                         );
          static void processTypesEnum(
                                       std::stringstream &ss,
                                       ContextPtr context
                                       );
          static void processTypesTemplatesAndSpecials(
                                                       std::stringstream &ss,
                                                       ProjectPtr project
                                                       );
          static void processTypesTemplate(
                                           std::stringstream &ss,
                                           ContextPtr structContextObj
                                           );
          static void processTypesSpecialStruct(
                                                std::stringstream &ss,
                                                ContextPtr structContextObj
                                                );

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark GenerateStructC::IIDLCompilerTarget
          #pragma mark

          //-------------------------------------------------------------------
          virtual String targetKeyword() override;
          virtual String targetKeywordHelp() override;
          virtual void targetOutput(
                                    const String &inPathStr,
                                    const ICompilerTypes::Config &config
                                    ) throw (Failure) override;
        };
         
      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
