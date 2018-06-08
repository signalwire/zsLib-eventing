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
        #pragma mark GenerateStructCppWinrt
        #pragma mark

        struct GenerateStructCppWinrt : public IIDLCompilerTarget,
                                        public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          ZS_DECLARE_PTR(StructSet);
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
            std::stringstream headerStructSS_;
            std::stringstream headerFinalSS_;
            std::stringstream cppIncludeSS_;
            std::stringstream cppBodySS_;

            StringSet headerAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            StructSetPtr structsNeedingInterface_;

            HelperFile();
            ~HelperFile();

            void includeHeader(const String &headerFile);
            void includeCpp(const String &headerFile);

            bool isStructNeedingInterface(StructPtr structObj) const;
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

            StructFile();
            ~StructFile();

            void includeCpp(const String &headerFile);
            bool isStructNeedingInterface(StructPtr structObj) const;
          };

          struct GenerationOptions
          {
            struct BaseOption;
            struct Optional;
            struct Interface;
            struct ReturnResult;
            struct Implementation;

            friend struct BaseOption;
            friend struct Optional;
            friend struct Interface;
            friend struct ReturnResult;
            friend struct Implementation;

            struct BaseOption
            {
              virtual void apply(GenerationOptions *options) const = 0;
            };

            struct Optional : public BaseOption {
              Optional(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->optional_ = value_; }
            };
            struct Interface : public BaseOption {
              Interface(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->interface_ = value_; }
            };
            struct ReturnResult : public BaseOption {
              ReturnResult(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->returnResult_ = value_; }
            };
            struct Reference : public BaseOption {
              Reference(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->reference_ = value_; }
            };
            struct Implementation : public BaseOption {
              Implementation(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->implementation_ = value_; }
            };
            struct ComPtr : public BaseOption {
              ComPtr(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
              virtual void apply(GenerationOptions *options) const override { options->comPtr_ = value_; }
            };

            GenerationOptions() {}
            GenerationOptions(const BaseOption &value) { value.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2
                              ) { value1.apply(this); value2.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3
                              ) { value1.apply(this); value2.apply(this); value3.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4
                              ) { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4,
                              const BaseOption &value5
                              ) { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); value5.apply(this); }
            GenerationOptions(
                              const BaseOption &value1,
                              const BaseOption &value2,
                              const BaseOption &value3,
                              const BaseOption &value4,
                              const BaseOption &value5,
                              const BaseOption &value6
                              ) { value1.apply(this); value2.apply(this); value3.apply(this); value4.apply(this); value5.apply(this); value6.apply(this); }

            GenerationOptions(const GenerationOptions &source) = default;
            GenerationOptions &operator=(const GenerationOptions &source) = default;

            bool isOptional() const { return optional_; }
            bool isInterface() const { return interface_; }
            bool isReturnResult() const { return returnResult_; }
            bool isReference() const { return reference_; }
            bool isImplementation() const { return implementation_; }
            bool isComPtr() const { return comPtr_; }

            Optional getOptional() const { return Optional(optional_); }
            Interface getInterface() const { return Interface(interface_); }
            ReturnResult getReturnResult() const { return ReturnResult(returnResult_); }
            Reference getReference() const { return Reference(reference_); }
            Implementation getImplementation() const { return Implementation(implementation_); }
            ComPtr getComPtr() const { return ComPtr(comPtr_); }

            GenerationOptions getAmmended(const BaseOption &value) const { GenerationOptions result(*this); value.apply(&result); return result; }

            static Optional MakeOptional() { return Optional(true); }
            static Optional MakeNotOptional() { return Optional(false); }
            static Interface MakeInterface() { return Interface(true); }
            static Interface MakeNotInterface() { return Interface(false); }
            static ReturnResult MakeReturnResult() { return ReturnResult(true); }
            static ReturnResult MakeNotReturnResult() { return ReturnResult(false); }
            static Reference MakeReference() { return Reference(true); }
            static Reference MakeNotReference() { return Reference(false); }
            static Implementation MakeImplementation() { return Implementation(true); }
            static Implementation MakeNotImplementation() { return Implementation(false); }
            static ComPtr MakeComPtr() { return ComPtr(true); }
            static ComPtr MakeNotComPtr() { return ComPtr(false); }

          protected:
            bool optional_{};
            bool interface_{};
            bool returnResult_{};
            bool reference_{};
            bool implementation_{};
            bool comPtr_{};
          };

          GenerateStructCppWinrt();

          static GenerateStructCppWinrtPtr create();

          static String fixName(const String &originalName);
          static String fixNamePath(ContextPtr context);
          static String fixNamePathNoPrefix(ContextPtr context);
          static String fixStructName(StructPtr structObj);
          static String fixMethodDeclaration(ContextPtr context);
          static String fixMethodDeclaration(
                                             StructPtr derivedStruct,
                                             ContextPtr context
                                             );
          static String fixStructFileName(StructPtr structObj);
          static String fixStructFileNameAsPath(StructPtr structObj);
          static String getStructInitName(StructPtr structObj);
          static String getCppWinrtStructInitName(StructPtr structObj);
          static String fixEnumName(EnumTypePtr enumObj);
          static String fixArgumentName(const String &originalName);

          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            const String &inIndentStr,
                                            NamespacePtr namespaceObj
                                            );
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         const String &inIndentStr,
                                         StructPtr structObj,
                                         bool &firstFound
                                         );
          static bool processTypesEnum(
                                       std::stringstream &ss,
                                       const String &inIndentStr,
                                       ContextPtr context
                                       );

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) throw (Failure);

          static void generateSpecialHelpers(HelperFile &helperFile);
          static void generateBasicTypesHelper(HelperFile &helperFile);
          static void generateExceptionHelper(HelperFile &helperFile);
          static void generateStringHelper(HelperFile &helperFile);
          static void generateBinaryHelper(HelperFile &helperFile);
          static void generateDurationHelper(
                                             HelperFile &helperFile,
                                             const String &durationType
                                             );
          static void generateTimeHelper(HelperFile &helperFile);
          static void generatePromiseHelper(HelperFile &helperFile);
          static void generatePromiseWithHelper(HelperFile &helperFile);
          static void generateDefaultPromiseRejections(
                                                       HelperFile &helperFile,
                                                       const String &indentStr
                                                       );
          static void generatePromiseRejection(
                                               HelperFile &helperFile,
                                               const String &indentStr,
                                               TypePtr rejectionType
                                               );

          static void generateForNamespace(
                                           HelperFile &helperFile,
                                           NamespacePtr namespaceObj,
                                           const String &inIndentStr
                                           );

          static void generateForStruct(
                                        HelperFile &helperFile,
                                        StructPtr structObj,
                                        const String &inIndentStr
                                        );
          static void generateForEnum(
                                      HelperFile &helperFile,
                                      EnumTypePtr enumObj
                                      );
          static void generateForStandardStruct(
                                                HelperFile &helperFile,
                                                StructPtr structObj
                                                );
          static void generateStructFile(
                                         HelperFile &helperFile,
                                         StructPtr structObj
                                         );
          static void generateStructMethods(
                                            HelperFile &helperFile, 
                                            StructFile &structFile,
                                            StructPtr derivedStructObj,
                                            StructPtr structObj,
                                            bool createConstructors,
                                            bool hasEvents
                                            );
          static void generateForList(
                                      HelperFile &helperFile,
                                      StructPtr structObj
                                      );
          static void generateForMap(
                                     HelperFile &helperFile,
                                     StructPtr structObj
                                     );
          static void generateForSet(
                                     HelperFile &helperFile,
                                     StructPtr structObj
                                     );

          static String getBasicCppWinrtTypeString(
                                                   BasicTypePtr type,
                                                   const GenerationOptions &options
                                                   );
          static String makeCppWinrtOptional(
                                             const String &value,
                                             const GenerationOptions &options
                                             );
          static String makeCppWinrtReference(
                                              const String &value,
                                              const GenerationOptions &options
                                              );
          static String makeCppWinrtReferenceAndOptionalIfOptional(
                                                                  const String &value,
                                                                  const GenerationOptions &options
                                                                  );
          static String getCppType(
                                   TypePtr type,
                                   const GenerationOptions &options
                                   );
          static String getCppWinrtType(
                                        TypePtr type,
                                        const GenerationOptions &options
                                        );
          static String getCppWinrtAttributes(const StringList &attributes);
          static String getCppWinrtAttributesLine(
                                                  const String &linePrefix,
                                                  const StringList &attributes
                                                  );
          static String getToFromCppWinrtName(
                                              TypePtr type,
                                              const GenerationOptions &options,
                                              const String &prefixName,
                                              const String &prefixNameIfImpl,
                                              const String &prefixIfInterface
                                              );
          static String getToCppWinrtName(
                                          TypePtr type,
                                          const GenerationOptions &options
                                          );
          static String getFromCppWinrtName(TypePtr type);
          static void includeCppForType(
                                        StructFile &structFile,
                                        TypePtr type
                                        );

          struct IncludeProcessedInfo
          {
            StringSet processedTypes_;
            StringSet structProcessedTypes_;
            StringSet templatedProcessedTypes_;

            IncludeProcessedInfo();
            ~IncludeProcessedInfo();
          };

          static void includeCppForType(
                                        IncludeProcessedInfo &processed,
                                        StructFile &structFile,
                                        TypePtr type
                                        );
          static void includeTemplatedStructForType(
                                                    IncludeProcessedInfo &processed,
                                                    StructFile &structFile,
                                                    StructPtr structObj
                                                    );
          static void includeTemplatedStructForType(
                                                    IncludeProcessedInfo &processed,
                                                    StructFile &structFile,
                                                    TemplatedStructTypePtr templatedStructObj
                                                    );

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark GenerateStructCppWinrt::IIDLCompilerTarget
          #pragma mark

          //-------------------------------------------------------------------
          String targetKeyword() override;
          String targetKeywordHelp() override;
          void targetOutput(
                            const String &inPathStr,
                            const ICompilerTypes::Config &config
                            ) throw (Failure) override;
        };
         
      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
