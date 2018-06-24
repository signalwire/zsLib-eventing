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
        // GenerateStructPython
        //

        struct GenerateStructPython : public IIDLCompilerTarget,
                                      public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          struct BaseFile
          {
            ProjectPtr project_;
            NamespacePtr global_;
            NamePathStructSetMap derives_;
            StringSet boxings_;

            String fileName_;
            String indent_;

            std::stringstream importSS_;
            std::stringstream usingAliasSS_;

            std::stringstream namespaceSS_;
            std::stringstream preStructSS_;
            std::stringstream structDeclationsSS_;
            std::stringstream structSS_;
            std::stringstream postStructSS_;

            std::stringstream preStructEndSS_;
            std::stringstream structEndSS_;
            std::stringstream postStructEndSS_;
            std::stringstream namespaceEndSS_;

            StringSet alreadyUsing_;

            BaseFile() noexcept;
            ~BaseFile() noexcept;

            void indentMore() noexcept { indent_ += "    "; }
            void indentLess() noexcept { indent_ = indent_.substr(0, indent_.length() - 4); }

            void usingTypedef(
                              const String &usingType,
                              const String &originalType = String(),
                              const String &asType = String()
                              ) noexcept;
            void usingTypedef(IEventingTypes::PredefinedTypedefs type) noexcept;
            void usingTypedef(TypePtr type) noexcept;

            bool hasBoxing(const String &namePathStr) noexcept;

            void startRegion(const String &region) noexcept;
            void endRegion(const String &region) noexcept;

            void startOtherRegion(
                                  const String &region,
                                  bool preStruct
                                  ) noexcept;
            void endOtherRegion(
                                const String &region,
                                bool preStruct
                                ) noexcept;
          };

          struct SetupFile : public BaseFile
          {
          };

          struct ApiFile : public BaseFile
          {
            std::stringstream &helpersSS_;
            std::stringstream &helpersEndSS_;

            ApiFile() noexcept;
            ~ApiFile() noexcept;

            void startHelpersRegion(const String &region) noexcept { startOtherRegion(region, false); }
            void endHelpersRegion(const String &region) noexcept { endOtherRegion(region, false); }
          };

          struct EnumFile : public BaseFile
          {
          };

          struct InitFile : public BaseFile
          {
          };

          struct StructFile : public ApiFile
          {
            std::stringstream &interfaceSS_;
            std::stringstream &interfaceEndSS_;
            std::stringstream &delegateSS_;

            StructFile(
                       BaseFile &baseFile,
                       StructPtr structObj
                       ) noexcept;
            ~StructFile() noexcept;

            StructPtr struct_;
            bool isStaticOnly_ {};
            bool isDictionary {};
            bool hasEvents_ {};
            bool shouldDefineInterface_ {};
            bool shouldInheritException_ {};
          };

          GenerateStructPython() noexcept;

          static GenerateStructPythonPtr create() noexcept;

          static String fixName(const String &originalName) noexcept;
          static String fixNamespace(NamespacePtr namespaceObj) noexcept;
          static String fixNamePath(ContextPtr context) noexcept;
          static String flattenPath(const String &originalName) noexcept;

          static String getPythonConvertedCType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String getPythonConvertedCType(TypePtr type) noexcept;
          static String getPythonCType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String getPythonToCType(
                                         const String &inputValue,
                                         IEventingTypes::PredefinedTypedefs type
                                         ) noexcept;
          static String getCToPythonType(
                                         const String &inputValue,
                                         IEventingTypes::PredefinedTypedefs type
                                         ) noexcept;
          static String fixArgumentName(const String &value) noexcept;
          static String fixCPythonType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String fixCPythonType(TypePtr type) noexcept;
          static String fixPythonType(
                                      TypePtr type,
                                      bool isInterface = false
                                      ) noexcept;
          static String fixPythonPathType(
                                          TypePtr type,
                                          bool isInterface = false
                                          ) noexcept;
          static String fixPythonType(
                                      bool isOptional,
                                      TypePtr type,
                                      bool isInterface = false
                                      ) noexcept;
          static String fixPythonPathType(
                                          bool isOptional,
                                          TypePtr type,
                                          bool isInterface = false
                                          ) noexcept;
          static String fixCsSystemType(IEventingTypes::PredefinedTypedefs type) noexcept;
          static String combineIf(
                                  const String &combinePreStr,
                                  const String &combinePostStr,
                                  const String &ifHasValue
                                  ) noexcept;
          static String getHelpersMethod(
                                         BaseFile &baseFile,
                                         bool useApiHelper,
                                         const String &methodName,
                                         bool isOptional,
                                         TypePtr type
                                         ) noexcept;
          static String getToCMethod(
                                     BaseFile &baseFile,
                                     bool isOptional,
                                     TypePtr type
                                     ) noexcept;
          static String getFromCMethod(
                                       BaseFile &baseFile,
                                       bool isOptional,
                                       TypePtr type
                                       ) noexcept;
          static String getAdoptFromCMethod(
                                            BaseFile &baseFile,
                                            bool isOptional, 
                                            TypePtr type
                                            ) noexcept;
          static String getDestroyCMethod(
                                          BaseFile &baseFile,
                                          bool isOptional, 
                                          TypePtr type
                                          ) noexcept;

          static bool hasInterface(StructPtr structObj) noexcept;

          static String getApiCastRequiredDefine(BaseFile &baseFile) noexcept;
          static String getApiPath() noexcept;
          static String getHelperPath() noexcept;

          static bool shouldDeriveFromException(
                                                BaseFile &baseFile,
                                                StructPtr structObj
                                                ) noexcept;

          static void finalizeBaseFile(BaseFile &apiFile) noexcept;

          static void prepareSetupFile(SetupFile &apiFile) noexcept;
          static void finalizeSetupFile(SetupFile &apiFile) noexcept;

          static void prepareApiInitFile(InitFile &apiFile) noexcept;
          static void finalizeApiInitFile(InitFile &apiFile) noexcept;

          static void prepareApiFile(ApiFile &apiFile) noexcept;
          static void finalizeApiFile(ApiFile &apiFile) noexcept;

          static void prepareApiCallback(ApiFile &apiFile) noexcept;

          static void prepareApiExceptions(ApiFile &apiFile) noexcept;
          static void prepareApiExceptions(
                                           ApiFile &apiFile,
                                           const String &exceptionName
                                           ) noexcept;

          static void prepareApiBasicTypes(ApiFile &apiFile) noexcept;
          static void prepareApiBasicTypes(
                                           ApiFile &apiFile,
                                           const IEventingTypes::PredefinedTypedefs basicType
                                           ) noexcept;
          static void prepareApiBoxing(ApiFile &apiFile) noexcept;
          static void prepareApiBoxing(
                                       ApiFile &apiFile,
                                       const IEventingTypes::PredefinedTypedefs basicType
                                       ) noexcept;

          static void prepareApiString(ApiFile &apiFile) noexcept;
          static void prepareApiBinary(ApiFile &apiFile) noexcept;

          static void prepareApiDuration(ApiFile &apiFile) noexcept;
          static void prepareApiDuration(
                                         ApiFile &apiFile,
                                         const String &durationType
                                         ) noexcept;
          static void prepareApiList(
                                     ApiFile &apiFile,
                                     const String &listOrSetStr
                                     ) noexcept;
          static void prepareApiSpecial(
                                        ApiFile &apiFile,
                                        const String &specialName
                                        ) noexcept;
          static void preparePromiseWithValue(ApiFile &apiFile) noexcept;
          static void preparePromiseWithRejectionReason(ApiFile &apiFile) noexcept;

          static void prepareApiNamespace(
                                          ApiFile &apiFile,
                                          NamespacePtr namespaceObj
                                          ) noexcept;
          static void prepareApiStruct(
                                       ApiFile &apiFile,
                                       StructPtr structObj
                                       ) noexcept;
          static void prepareApiEnum(
                                     ApiFile &apiFile,
                                     EnumTypePtr enumObj
                                     ) noexcept;

          static void prepareInitFile(
                                      InitFile &initFile,
                                      NamespacePtr namespaceObj
                                      ) noexcept;
          static void finalizeInitFile(InitFile &initFile) noexcept { finalizeBaseFile(initFile); }

          static void prepareEnumFile(
                                      EnumFile &enumFile,
                                      NamespacePtr namespaceObj
                                      ) noexcept;
          static void finalizeEnumFile(EnumFile &enumFile) noexcept { finalizeBaseFile(enumFile); }

          static void prepareEnumNamespace(
                                           EnumFile &enumFile,
                                           NamespacePtr namespaceObj
                                           ) noexcept;

          static void processNamespace(
                                       const String &pathStr,
                                       SetupFile &setupFile,
                                       ApiFile &apiFile,
                                       NamespacePtr namespaceObj
                                       ) noexcept (false);
          static void processStruct(
                                    const String &pathStr,
                                    ApiFile &apiFile,
                                    InitFile &initFile,
                                    StructPtr structObj
                                    ) noexcept;
          static void processInheritance(
                                         ApiFile &apiFile,
                                         StructFile &structFile,
                                         StructFile &interfaceFile,
                                         StructPtr rootStructObj,
                                         StructPtr structObj,
                                         bool &first
                                         ) noexcept;
          static void processStruct(
                                    const String &pathStr,
                                    ApiFile &apiFile,
                                    InitFile &initFile,
                                    StructFile &structFile,
                                    StructFile &interfaceFile,
                                    StructPtr rootStructObj,
                                    StructPtr structObj
                                    ) noexcept;
          static void processEnum(
                                  ApiFile &apiFile,
                                  StructFile &structFile,
                                  StructPtr structObj,
                                  EnumTypePtr enumObj
                                  ) noexcept;
          static void processMethods(
                                     ApiFile &apiFile,
                                     StructFile &structFile,
                                     StructFile &interfaceFile,
                                     StructPtr rootStructObj,
                                     StructPtr structObj
                                     ) noexcept;
          static void processProperties(
                                        ApiFile &apiFile,
                                        StructFile &structFile,
                                        StructFile &interfaceFile,
                                        StructPtr rootStructObj,
                                        StructPtr structObj
                                        ) noexcept;
          static void processEventHandlers(
                                           ApiFile &apiFile,
                                           StructFile &structFile,
                                           StructFile &interfaceFile,
                                           StructPtr structObj
                                           ) noexcept;
          static void processEventHandlersStart(
                                                ApiFile &apiFile,
                                                StructFile &structFile,
                                                StructFile &interfaceFile,
                                                StructPtr structObj
                                                ) noexcept;
          static void processEventHandlersEnd(
                                              ApiFile &apiFile,
                                              StructFile &structFile,
                                              StructFile &interfaceFile,
                                              StructPtr structObj
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
