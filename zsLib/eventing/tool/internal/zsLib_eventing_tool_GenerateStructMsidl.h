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
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;
          ZS_DECLARE_PTR(NamePathStructSetMap);

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
            NamePathStructSetMapPtr derives_;

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

          static bool hasAnotherCtorWithSameNumberOfArguments(
                                                              StructPtr structObj,
                                                              MethodPtr currentCtor
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
