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
        // GenerateStructCppWinrt
        //

        struct GenerateStructCppWinrt : public IIDLCompilerTarget,
                                        public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          ZS_DECLARE_PTR(StructSet);
          typedef std::set<TypePtr> TypeSet;
          ZS_DECLARE_PTR(TypeSet);
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;
          struct GenerationOptions;
          typedef GenerationOptions GO;

          struct HelperFile
          {
            NamespacePtr global_;
            NamePathStructSetMap derives_;

            String headerFileName_;
            String cppFileName_;

            String headerIndentStr_;

            std::stringstream headerIncludeSS_;
            std::stringstream headerThrowersSS_;
            std::stringstream headerStructSS_;
            std::stringstream headerFinalSS_;
            std::stringstream cppIncludeSS_;
            std::stringstream cppBodySS_;

            StringSet headerAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            TypeSet alreadyThrows_;

            StructSetPtr structsNeedingInterface_;

            HelperFile() noexcept;
            ~HelperFile() noexcept;

            void includeHeader(const String &headerFile) noexcept;
            void includeCpp(const String &headerFile) noexcept;
            void specialThrow(TypePtr type) noexcept;

            bool isStructNeedingInterface(StructPtr structObj) const noexcept;
          };

          struct StructFile
          {
            StructPtr struct_;
            std::stringstream headerIncludeSS_;
            std::stringstream headerStructPrivateSS_;
            std::stringstream headerStructEventHandlersSS_;
            std::stringstream headerStructObserverSS_;
            std::stringstream headerStructObserverFinalSS_;
            std::stringstream headerStructPublicSS_;
            std::stringstream cppIncludeSS_;
            std::stringstream cppBodySS_;
            String headerIndentStr_;
            String headerStructIndentStr_;

            StringSet cppAlreadyIncluded_;

            StructSetPtr structsNeedingInterface_;

            StructFile() noexcept;
            ~StructFile() noexcept;

            void includeCpp(const String &headerFile) noexcept;
            bool isStructNeedingInterface(StructPtr structObj) const noexcept;
          };

          struct GenerationOptions
          {
            struct BaseOption;
            struct Optional;
            struct Interface;
            struct ReturnResult;
            struct Implementation;
            struct CppBaseType;

            friend struct BaseOption;
            friend struct Optional;
            friend struct Interface;
            friend struct ReturnResult;
            friend struct Implementation;
            friend struct CppBaseType;

            struct BaseOption
            {
              virtual void apply(GenerationOptions *options) const noexcept = 0;
            };

            struct Optional : public BaseOption {
              Optional(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->optional_ = value_; }
              bool value_{};
            };
            struct Interface : public BaseOption {
              Interface(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->interface_ = value_; }
              bool value_{};
            };
            struct ReturnResult : public BaseOption {
              ReturnResult(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->returnResult_ = value_; }
              bool value_{};
            };
            struct Reference : public BaseOption {
              Reference(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->reference_ = value_; }
              bool value_{};
            };
            struct Implementation : public BaseOption {
              Implementation(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->implementation_ = value_; }
              bool value_{};
            };
            struct ComPtr : public BaseOption {
              ComPtr(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->comPtr_ = value_; }
              bool value_{};
            };
            struct CppBaseType : public BaseOption {
              CppBaseType(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              virtual void apply(GenerationOptions *options) const noexcept override { options->cppBaseType_ = value_; }
              bool value_{};
            };

            GenerationOptions() noexcept {}
            GenerationOptions(const BaseOption &value) noexcept { value.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2
                              ) noexcept { value1.apply(this); value2.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3
                              ) noexcept { value1.apply(this); value2.apply(this); value3.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4
                              ) noexcept { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4,
                              const BaseOption &value5
                              ) noexcept { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); value5.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4,
                              const BaseOption &value5,
                              const BaseOption &value6
                              ) noexcept { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); value5.apply(this); value6.apply(this); }

            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4,
                              const BaseOption &value5,
                              const BaseOption &value6,
                              const BaseOption &value7
                              ) noexcept { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); value5.apply(this); value6.apply(this); value7.apply(this); }

            GenerationOptions(const GenerationOptions &source) noexcept = default;
            GenerationOptions &operator=(const GenerationOptions &source) noexcept = default;

            bool isOptional() const noexcept { return optional_; }
            bool isInterface() const noexcept { return interface_; }
            bool isReturnResult() const noexcept { return returnResult_; }
            bool isReference() const noexcept { return reference_; }
            bool isImplementation() const noexcept { return implementation_; }
            bool isComPtr() const noexcept { return comPtr_; }
            bool isCppBaseType() const noexcept { return cppBaseType_; }

            Optional getOptional() const noexcept { return Optional(optional_); }
            Interface getInterface() const noexcept { return Interface(interface_); }
            ReturnResult getReturnResult() const noexcept { return ReturnResult(returnResult_); }
            Reference getReference() const noexcept { return Reference(reference_); }
            Implementation getImplementation() const noexcept { return Implementation(implementation_); }
            ComPtr getComPtr() const noexcept { return ComPtr(comPtr_); }
            CppBaseType getCppBaseType() const noexcept { return CppBaseType(cppBaseType_); }

            GenerationOptions getAmmended(const BaseOption &value) const noexcept { GenerationOptions result(*this); value.apply(&result); return result; }

            static Optional MakeOptional() noexcept { return Optional(true); }
            static Optional MakeNotOptional() noexcept { return Optional(false); }
            static Interface MakeInterface() noexcept { return Interface(true); }
            static Interface MakeNotInterface() noexcept { return Interface(false); }
            static ReturnResult MakeReturnResult() noexcept { return ReturnResult(true); }
            static ReturnResult MakeNotReturnResult() noexcept { return ReturnResult(false); }
            static Reference MakeReference() noexcept { return Reference(true); }
            static Reference MakeNotReference() noexcept { return Reference(false); }
            static Implementation MakeImplementation() noexcept { return Implementation(true); }
            static Implementation MakeNotImplementation() noexcept { return Implementation(false); }
            static ComPtr MakeComPtr() noexcept { return ComPtr(true); }
            static ComPtr MakeNotComPtr() noexcept { return ComPtr(false); }
            static CppBaseType MakeCppBaseType() noexcept { return CppBaseType(true); }
            static CppBaseType MakeNotCppBaseType() noexcept { return CppBaseType(false); }

          protected:
            bool optional_{};
            bool interface_{};
            bool returnResult_{};
            bool reference_{};
            bool implementation_{};
            bool comPtr_{};
            bool cppBaseType_{};
          };

          GenerateStructCppWinrt() noexcept;

          static GenerateStructCppWinrtPtr create() noexcept;

          static String fixName(const String &originalName) noexcept;
          static String fixNamePath(ContextPtr context) noexcept;
          static String fixNamePathNoPrefix(ContextPtr context) noexcept;
          static String fixStructName(StructPtr structObj) noexcept;
          static String fixMethodDeclaration(ContextPtr context) noexcept;
          static String fixMethodDeclaration(
                                             StructPtr derivedStruct,
                                             ContextPtr context
                                             ) noexcept;
          static String fixMethodDeclaration(
                                             StructPtr derivedStruct,
                                             const String &name
                                             ) noexcept;
          static String fixStructFileName(StructPtr structObj) noexcept;
          static String fixStructFileNameAsPath(StructPtr structObj) noexcept;
          static String getStructInitName(StructPtr structObj) noexcept;
          static String getCppWinrtStructInitName(StructPtr structObj) noexcept;
          static String fixEnumName(EnumTypePtr enumObj) noexcept;
          static String fixArgumentName(const String &originalName) noexcept;

          static void processTypesNamespace(
                                            std::stringstream &iss,
                                            std::stringstream &ss,
                                            const String &inIndentStr,
                                            NamespacePtr namespaceObj
                                            ) noexcept;
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         const String &indentStr,
                                         StructPtr structObj,
                                         bool &firstFound
                                         ) noexcept;

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) noexcept(false); // throws Failure

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
          static bool requiresSpecialConversion(IEventingTypes::PredefinedTypedefs basicType) noexcept;
          static String getBasicCppWinrtTypeString(
                                                   IEventingTypes::PredefinedTypedefs type,
                                                   const GenerationOptions &options
                                                   ) noexcept;

          static String getBasicCppWinrtTypeString(
                                                   BasicTypePtr type,
                                                   const GenerationOptions &options
                                                   ) noexcept;
          static String makeCppWinrtOptional(
                                             const String &value,
                                             const GenerationOptions &options
                                             ) noexcept;
          static String makeCppWinrtReference(
                                              const String &value,
                                              const GenerationOptions &options
                                              ) noexcept;
          static String makeCppWinrtReferenceAndOptional(
                                                         const String &value,
                                                         const GenerationOptions &options
                                                         ) noexcept;
          static String makeCppWinrtReferenceAndOptionalIfOptional(
                                                                  const String &value,
                                                                  const GenerationOptions &options
                                                                  ) noexcept;
          static bool isDefaultExceptionType(TypePtr type);
          static String getCppType(
                                   TypePtr type,
                                   const GenerationOptions &options
                                   ) noexcept;
          static String getCppWinrtType(
                                        const HelperFile &helperFile,
                                        TypePtr type,
                                        const GenerationOptions &options
                                        ) noexcept;
          static String getCppWinrtType(
                                        TypePtr type,
                                        const StructSet &structsNeedingInterface,
                                        const GenerationOptions &options
                                        ) noexcept;
          static String getCppWinrtResultTypeInitializer(TypePtr type) noexcept;
          static String getCppWinrtAttributes(const StringList &attributes) noexcept;
          static String getCppWinrtAttributesLine(
                                                  const String &linePrefix,
                                                  const StringList &attributes
                                                  ) noexcept;
          static String getToFromCppWinrtName(
                                              TypePtr type,
                                              const StructSet &structsNeedingInterface,
                                              const GenerationOptions &options,
                                              const String &prefixName,
                                              const String &prefixNameIfImpl,
                                              const String &prefixIfInterface
                                              ) noexcept;
          static String getToCppWinrtName(
                                          TypePtr type,
                                          const StructSet &structsNeedingInterface,
                                          const GenerationOptions &options
                                          ) noexcept;
          static String getToCppWinrtName(
                                          const HelperFile &helperFile,
                                          TypePtr type,
                                          const GenerationOptions &options
                                          ) noexcept;
          static String getFromCppWinrtName(TypePtr type) noexcept;
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
          // GenerateStructCppWinrt::IIDLCompilerTarget
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
