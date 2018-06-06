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
        #pragma mark GenerateStructMsidl
        #pragma mark

        struct GenerateStructMsidl : public IIDLCompilerTarget,
                                     public IDLCompiler
        {

          typedef std::set<String> StringSet;
          typedef String NamePath;
          typedef std::set<StructPtr> StructSet;
          ZS_DECLARE_PTR(StructSet);

          typedef std::list<NamespacePtr> NamespaceList;

          struct IDLFile
          {
            NamespacePtr global_;

            IDLFile();
            ~IDLFile();

            String indent_;

            String fileName_;

            std::stringstream importSS_;
            std::stringstream enumSS_;
            std::stringstream bodySS_;

            StringSet alreadyImported_;

            StructSetPtr structsNeedingInterface_;

            void import(const String &file);
            bool isStructNeedingInterface(StructPtr structObj) const;

            void finalize(std::stringstream &ss) const;
          };

          struct GenerationOptions
          {
            struct Optional {
              Optional(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
            };
            struct Interface {
              Interface(bool value) : value_(value) {}
              operator bool() const { return value_; }
              bool value_{};
            };

            GenerationOptions() {}
            GenerationOptions(const Optional &value) : optional_(value) {}
            GenerationOptions(const Interface &value) : interface_(value) {}
            GenerationOptions(
              const Optional &value1,
              const Interface &value2
            ) : optional_(value1), interface_(value2) {}

            bool optional_ {false};
            bool interface_ {false};
          };

          GenerateStructMsidl();

          static GenerateStructMsidlPtr create();

          static String fixName(const String &originalName);
          static String fixName(ContextPtr structObj);
          static String fixNamePath(
                                    ContextPtr context,
                                    const GenerationOptions &options
                                    );
          static String toIdlType(BasicTypePtr basicType);
          static String toIdlType(
                                  IDLFile &idl,
                                  const GenerationOptions &options,
                                  BasicTypePtr basicType
                                  );
          static String toIdlSimpleType(
                                        IDLFile &idl,
                                        const GenerationOptions &options,
                                        TypePtr type
                                        );
          static String toIdlSimpleType(
                                        IDLFile &idl,
                                        const GenerationOptions &options,
                                        const String &typeName
                                        );
          static String toIdlType(
                                  IDLFile &idl,
                                  const GenerationOptions &options,
                                  TypePtr type
                                  );

          static void fixHiddenAttribute(
                                         IDLFile &idl,
                                         ContextPtr context,
                                         const String &indentStr,
                                         std::stringstream &ss
                                         );
          static void fixDeprecatedAttribute(
                                             IDLFile &idl,
                                             ContextPtr context,
                                             const String &indentStr,
                                             std::stringstream &ss
                                             );
          static void fixMethodNameAttribute(
                                             IDLFile &idl,
                                             ContextPtr context,
                                             const String &indentStr,
                                             std::stringstream &ss
                                             );
          static void fixDefaultAttribute(
                                          IDLFile &idl,
                                          ContextPtr context,
                                          const String &indentStr,
                                          std::stringstream &ss
                                          );

          static void scanNamespaceForStructsNeedingToBeInterfaces(
                                                                   StructSet &needingInterfaceSet,
                                                                   NamespacePtr namespaceObj,
                                                                   int scanPass = 0
                                                                   );

          static void scanStructForStructsNeedingToBeInterfaces(
                                                                StructSet &needingInterfaceSet,
                                                                StructPtr structObj,
                                                                int scanPass
                                                                );

          static bool doesAnyRelationHaveInterface(
                                                   StructSet &needingInterfaceSet,
                                                   StructPtr structObj,
                                                   StructSet &alreadyScanned
                                                   );

          static void markAllRelatedStructsAsNeedingInterface(
                                                              StructSet &needingInterfaceSet,
                                                              StructPtr structObj
                                                              );

          void processNamespace(
                                IDLFile &forwardIdl,
                                IDLFile &outputIdl,
                                NamespacePtr namespaceObj
                                );

          void processStruct(
                             IDLFile &forwardIdl,
                             IDLFile &outputIdl,
                             StructPtr structObj
                             );
          void processProperties(
                                 IDLFile &idl,
                                 StructPtr structObj,
                                 std::stringstream &ss,
                                 std::stringstream &staticsSS,
                                 bool &outFoundProperty
                                 );
          void processMethods(
                              IDLFile &idl,
                              StructPtr structObj,
                              std::stringstream &methodSS,
                              std::stringstream &staticMethodsSS,
                              std::stringstream &delegatesSS,
                              std::stringstream &delegateEventHandlersSS,
                              std::stringstream &ctorSS,
                              bool requiredInterface,
                              bool &outFoundMethod,
                              bool &outFoundCtor
                              );

          void processEnum(
                           IDLFile &forwardIdl,
                           IDLFile &outputIdl,
                           EnumTypePtr enumObj
                           );

#if 0
          static String fixStructName(StructPtr structObj);
          static String fixMethodDeclaration(ContextPtr context);
          static String fixMethodDeclaration(
                                             StructPtr derivedStruct,
                                             ContextPtr context
                                             );
          static String fixStructFileName(StructPtr structObj);
          static String getStructInitName(StructPtr structObj);
          static String getCxStructInitName(StructPtr structObj);
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

          static String getBasicCxTypeString(
                                             bool isOptional,
                                             BasicTypePtr type,
                                             bool isReturnType = false
                                             );
          static String makeCxOptional(
                                       bool isOptional,
                                       const String &value
                                       );
          static String getCppType(
                                   bool isOptional,
                                   TypePtr type
                                   );
          static String getCxType(
                                  bool isOptional,
                                  TypePtr type,
                                  bool isReturnType = false
                                  );
          static String getCxAttributes(const StringList &attributes);
          static String getCxAttributesLine(
                                            const String &linePrefix,
                                            const StringList &attributes
                                            );
          static String getToFromCxName(TypePtr type);
          static String getToCxName(TypePtr type);
          static String getFromCxName(TypePtr type);
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
#endif //0

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark GenerateStructMsidl::IIDLCompilerTarget
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
