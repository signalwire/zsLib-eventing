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
        //
        // GenerateStructC
        //

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
            StringSet boxings_;

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

            HelperFile() noexcept;
            ~HelperFile() noexcept;

            void headerIncludeC(const String &headerFile) noexcept;
            void headerIncludeCpp(const String &headerFile) noexcept;
            void includeC(const String &headerFile) noexcept;
            void includeCpp(const String &headerFile) noexcept;
            bool hasBoxing(const String &namePathStr) noexcept;
          };

          struct StructFile
          {
            StructPtr struct_;

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

            StructFile() noexcept;
            ~StructFile() noexcept;

            void headerIncludeC(const String &headerFile) noexcept;
            void headerIncludeCpp(const String &headerFile) noexcept;
            void includeC(const String &headerFile) noexcept;
            void includeCpp(const String &headerFile) noexcept;
          };

          GenerateStructC() noexcept;

          static GenerateStructCPtr create() noexcept;

          static String fixBasicType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String fixCType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String fixCType(TypePtr type) noexcept;
          static String fixCType(
                                 bool isOptional,
                                 TypePtr type
                                 ) noexcept;
          static String fixType(TypePtr type) noexcept;
          static String getApiImplementationDefine(ContextPtr context) noexcept;
          static String getApiCastRequiredDefine(ContextPtr context) noexcept;
          static String getApiExportDefine(ContextPtr context) noexcept;
          static String getApiExportCastedDefine(ContextPtr context) noexcept;
          static String getApiCallingDefine(ContextPtr context) noexcept;
          static String getApiGuardDefine(
                                          ContextPtr context,
                                          bool endGuard = false
                                          ) noexcept;
          static String getToHandleMethod(
                                          bool isOptional, 
                                          TypePtr type
                                          ) noexcept;
          static String getFromHandleMethod(
                                            bool isOptional,
                                            TypePtr type
                                            ) noexcept;

          static void includeType(
                                  HelperFile &helperFile,
                                  TypePtr type
                                  ) noexcept;
          static void includeType(
                                  StructFile &structFile,
                                  TypePtr type
                                  ) noexcept;

          static void calculateRelations(
                                         NamespacePtr namespaceObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         ) noexcept;
          static void calculateRelations(
                                         StructPtr structObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         ) noexcept;

          static void calculateBoxings(
                                       NamespacePtr namespaceObj,
                                       StringSet &ioBoxings
                                       ) noexcept;
          static void calculateBoxings(
                                       StructPtr structObj,
                                       StringSet &ioBoxings
                                       ) noexcept;
          static void calculateBoxings(
                                       TemplatedStructTypePtr templatedStructObj,
                                       StringSet &ioBoxings
                                       ) noexcept;
          static void calculateBoxings(
                                       MethodPtr method,
                                       StringSet &ioBoxings,
                                       TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                       ) noexcept;
          static void calculateBoxings(
                                       PropertyPtr property,
                                       StringSet &ioBoxings,
                                       TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                       ) noexcept;
          static void calculateBoxingType(
                                          bool isOptional,
                                          TypePtr type,
                                          StringSet &ioBoxings,
                                          TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                          ) noexcept;

          static void insertInto(
                                 StructPtr structObj,
                                 const NamePath &namePath,
                                 NamePathStructSetMap &ioDerivesInfo
                                 ) noexcept;

          static void appendStream(
                                   std::stringstream &output,
                                   std::stringstream &source,
                                   bool appendEol = true
                                   ) noexcept;

          static void prepareHelperFile(HelperFile &helperFile) noexcept;
          static void prepareHelperCallback(HelperFile &helperFile) noexcept;
          static void prepareHelperExceptions(HelperFile &helperFile) noexcept;
          static void prepareHelperExceptions(
                                              HelperFile &helperFile,
                                              const String &exceptionName
                                              ) noexcept;
          static void prepareHelperBoxing(HelperFile &helperFile) noexcept;
          static void prepareHelperBoxing(
                                          HelperFile &helperFile,
                                          const IEventingTypes::PredefinedTypedefs basicType
                                          ) noexcept;
          static void prepareHelperNamespace(
                                             HelperFile &helperFile,
                                             NamespacePtr namespaceObj
                                             ) noexcept;
          static void prepareHelperStruct(
                                          HelperFile &helperFile,
                                          StructPtr structObj
                                          ) noexcept;
          static void prepareHelperEnum(
                                        HelperFile &helperFile,
                                        EnumTypePtr enumObj
                                        ) noexcept;
          static void prepareHelperEnumBoxing(
                                              HelperFile &helperFile,
                                              EnumTypePtr enumObj
                                              ) noexcept;
          static void prepareHelperString(HelperFile &helperFile) noexcept;
          static void prepareHelperBinary(HelperFile &helperFile) noexcept;
          static void prepareHelperDuration(
                                            HelperFile &helperFile,
                                            const String &durationType
                                            ) noexcept;
          static void prepareHelperList(
                                        HelperFile &helperFile,
                                        const String &listOrSetStr
                                        ) noexcept;
          static void prepareHelperSpecial(
                                           HelperFile &helperFile,
                                           const String &specialName
                                           ) noexcept;
          static void preparePromiseWithValue(HelperFile &helperFile) noexcept;
          static void preparePromiseWithRejectionReason(HelperFile &helperFile) noexcept;

          static void finalizeHelperFile(HelperFile &helperFile) noexcept;

          static void processNamespace(
                                       HelperFile &helperFile,
                                       NamespacePtr namespaceObj
                                       ) noexcept;
          static void processStruct(
                                    HelperFile &helperFile,
                                    StructPtr structObj
                                    ) noexcept;
          static void processStruct(
                                    HelperFile &helperFile,
                                    StructFile &structFile,
                                    StructPtr rootStructObj,
                                    StructPtr structObj
                                    ) noexcept;
          static void processMethods(
                                     HelperFile &helperFile,
                                     StructFile &structFile,
                                     StructPtr rootStructObj,
                                     StructPtr structObj
                                     ) noexcept;
          static void processProperties(
                                        HelperFile &helperFile,
                                        StructFile &structFile,
                                        StructPtr rootStructObj,
                                        StructPtr structObj
                                        ) noexcept;
          static void processEventHandlers(
                                           HelperFile &helperFile,
                                           StructFile &structFile,
                                           StructPtr structObj
                                           ) noexcept;
          static void processEventHandlersStart(
                                                HelperFile &helperFile,
                                                StructFile &structFile,
                                                StructPtr structObj
                                                ) noexcept;
          static void processEventHandlersEnd(
                                              HelperFile &helperFile,
                                              StructFile &structFile,
                                              StructPtr structObj
                                              ) noexcept;

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) noexcept(false); // throws Failure
          
          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            NamespacePtr namespaceObj
                                            ) noexcept;
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         StructPtr structObj
                                         ) noexcept;
          static void processTypesEnum(
                                       std::stringstream &ss,
                                       ContextPtr context
                                       ) noexcept;
          static void processTypesTemplatesAndSpecials(
                                                       std::stringstream &ss,
                                                       ProjectPtr project
                                                       ) noexcept;
          static void processTypesTemplate(
                                           std::stringstream &ss,
                                           ContextPtr structContextObj
                                           ) noexcept;
          static void processTypesSpecialStruct(
                                                std::stringstream &ss,
                                                ContextPtr structContextObj
                                                ) noexcept;

          //-------------------------------------------------------------------
          //
          // GenerateStructC::IIDLCompilerTarget
          //

          //-------------------------------------------------------------------
          String targetKeyword() noexcept override;
          String targetKeywordHelp() noexcept override;
          void targetOutput(
                            const String &inPathStr,
                            const ICompilerTypes::Config &config
                            ) noexcept(false) override; // throws Failure
        };
         
      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
