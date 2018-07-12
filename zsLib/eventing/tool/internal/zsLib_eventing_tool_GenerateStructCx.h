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
        //
        // GenerateStructCx
        //

        struct GenerateStructCx : public IIDLCompilerTarget,
                                  public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          typedef std::set<TypePtr> TypeSet;

          struct HelperFile
          {
            NamespacePtr mGlobal;
            NamePathStructSetMap mDerives;

            String mHeaderFileName;
            String mCppFileName;

            String mHeaderIndentStr;

            std::stringstream mHeaderIncludeSS;
            std::stringstream mHeaderThrowersSS;
            std::stringstream mHeaderStructSS;
            std::stringstream mHeaderFinalSS;
            std::stringstream mCppIncludeSS;
            std::stringstream mCppBodySS;

            StringSet mHeaderAlreadyIncluded;
            StringSet mCppAlreadyIncluded;
            TypeSet mAlreadyThrows;

            HelperFile() noexcept;
            ~HelperFile() noexcept;

            void includeHeader(const String &headerFile) noexcept;
            void includeCpp(const String &headerFile) noexcept;
            void specialThrow(TypePtr rejectionType) noexcept;
          };

          struct StructFile
          {
            StructPtr mStruct;
            std::stringstream mHeaderIncludeSS;
            std::stringstream mHeaderPreStructSS;
            std::stringstream mHeaderStructPrivateSS;
            std::stringstream mHeaderStructObserverSS;
            std::stringstream mHeaderStructObserverFinalSS;
            std::stringstream mHeaderStructPublicSS;
            std::stringstream mCppIncludeSS;
            std::stringstream mCppBodySS;
            String mHeaderIndentStr;
            String mHeaderStructIndentStr;

            StringSet mCppAlreadyIncluded;

            StructFile() noexcept;
            ~StructFile() noexcept;

            void includeCpp(const String &headerFile) noexcept;
          };

          GenerateStructCx() noexcept;

          static GenerateStructCxPtr create() noexcept;

          static String fixName(
                                const String &originalName,
                                const char *splitter = ".",
                                const char *combiner = "."
                                ) noexcept;
          static String fixNamePath(ContextPtr context) noexcept;
          static String fixStructName(StructPtr structObj) noexcept;
          static String fixMethodDeclaration(ContextPtr context) noexcept;
          static String fixMethodDeclaration(
                                             StructPtr derivedStruct,
                                             ContextPtr context
                                             ) noexcept;
          static String fixStructFileName(StructPtr structObj) noexcept;
          static String getStructInitName(StructPtr structObj) noexcept;
          static String getCxStructInitName(StructPtr structObj) noexcept;
          static String fixEnumName(EnumTypePtr enumObj) noexcept;
          static String fixArgumentName(const String &originalName) noexcept;

          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            const String &inIndentStr,
                                            NamespacePtr namespaceObj
                                            ) noexcept;
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         const String &inIndentStr,
                                         StructPtr structObj,
                                         bool &firstFound
                                         ) noexcept;
          static bool processTypesEnum(
                                       std::stringstream &ss,
                                       const String &inIndentStr,
                                       ContextPtr context
                                       ) noexcept;

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) noexcept(false); // throws Failure

          static void calculateRelations(
                                         NamespacePtr namespaceObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         ) noexcept;
          static void calculateRelations(
                                         StructPtr structObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         ) noexcept;

          static void insertInto(
                                 StructPtr structObj,
                                 const NamePath &namePath,
                                 NamePathStructSetMap &ioDerivesInfo
                                 ) noexcept;

          static void generateSpecialHelpers(HelperFile &helperFile) noexcept;
          static void generateBasicTypesHelper(HelperFile &helperFile) noexcept;
          static void generateExceptionHelper(HelperFile &helperFile) noexcept;
          static void generateStringHelper(HelperFile &helperFile) noexcept;
          static void generateBinaryHelper(HelperFile &helperFile) noexcept;
          static void generateDurationHelper(
                                             HelperFile &helperFile,
                                             const String &durationType
                                             ) noexcept;
          static void generateTimeHelper(HelperFile &helperFile) noexcept;
          static void generatePromiseHelper(HelperFile &helperFile) noexcept;
          static void generatePromiseWithHelper(HelperFile &helperFile) noexcept;
          static void generateDefaultPromiseRejections(
                                                       HelperFile &helperFile,
                                                       const String &indentStr
                                                       ) noexcept;
          static void generatePromiseRejection(
                                               HelperFile &helperFile,
                                               const String &indentStr,
                                               TypePtr rejectionType
                                               ) noexcept;

          static void generateForNamespace(
                                           HelperFile &helperFile,
                                           NamespacePtr namespaceObj,
                                           const String &inIndentStr
                                           ) noexcept;

          static void generateForStruct(
                                        HelperFile &helperFile,
                                        StructPtr structObj,
                                        const String &inIndentStr
                                        ) noexcept;
          static void generateForEnum(
                                      HelperFile &helperFile,
                                      EnumTypePtr enumObj
                                      ) noexcept;
          static void generateForStandardStruct(
                                                HelperFile &helperFile,
                                                StructPtr structObj
                                                ) noexcept;
          static void generateStructFile(
                                         HelperFile &helperFile,
                                         StructPtr structObj
                                         ) noexcept;
          static void generateStructMethods(
                                            HelperFile &helperFile, 
                                            StructFile &structFile,
                                            StructPtr derivedStructObj,
                                            StructPtr structObj,
                                            bool hasEvents
                                            ) noexcept;
          static void generateForList(
                                      HelperFile &helperFile,
                                      StructPtr structObj
                                      ) noexcept;
          static void generateForMap(
                                     HelperFile &helperFile,
                                     StructPtr structObj
                                     ) noexcept;
          static void generateForSet(
                                     HelperFile &helperFile,
                                     StructPtr structObj
                                     ) noexcept;

          static String getBasicCxTypeString(
                                             bool isOptional,
                                             BasicTypePtr type,
                                             bool isReturnType = false
                                             ) noexcept;
          static String makeCxOptional(
                                       bool isOptional,
                                       const String &value
                                       ) noexcept;
          static bool isDefaultExceptionType(TypePtr type);
          static String getCppType(
                                   bool isOptional,
                                   TypePtr type
                                   ) noexcept;
          static String getCxType(
                                  bool isOptional,
                                  TypePtr type,
                                  bool isReturnType = false
                                  ) noexcept;
          static String getCxAttributes(const StringList &attributes) noexcept;
          static String getCxAttributesLine(
                                            const String &linePrefix,
                                            const StringList &attributes
                                            ) noexcept;
          static String getToFromCxName(TypePtr type) noexcept;
          static String getToCxName(TypePtr type) noexcept;
          static String getFromCxName(TypePtr type) noexcept;
          static void includeCppForType(
                                        StructFile &structFile,
                                        TypePtr type
                                        ) noexcept;

          struct IncludeProcessedInfo
          {
            StringSet processedTypes_;
            StringSet structProcessedTypes_;
            StringSet templatedProcessedTypes_;

            IncludeProcessedInfo() noexcept;
            ~IncludeProcessedInfo() noexcept;
          };

          static void includeCppForType(
                                        IncludeProcessedInfo &processed,
                                        StructFile &structFile,
                                        TypePtr type
                                        ) noexcept;
          static void includeTemplatedStructForType(
                                                    IncludeProcessedInfo &processed,
                                                    StructFile &structFile,
                                                    StructPtr structObj
                                                    ) noexcept;
          static void includeTemplatedStructForType(
                                                    IncludeProcessedInfo &processed,
                                                    StructFile &structFile,
                                                    TemplatedStructTypePtr templatedStructObj
                                                    ) noexcept;

          //-------------------------------------------------------------------
          //
          // GenerateStructCx::IIDLCompilerTarget
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
