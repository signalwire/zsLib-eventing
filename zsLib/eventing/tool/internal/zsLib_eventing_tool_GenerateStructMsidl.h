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
        // GenerateStructMsidl
        //

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

            IDLFile() noexcept;
            ~IDLFile() noexcept;

            String indent_;

            String fileName_;

            std::stringstream importSS_;
            std::stringstream enumSS_;
            std::stringstream bodySS_;

            StringSet alreadyImported_;

            StructSetPtr structsNeedingInterface_;
            NamePathStructSetMapPtr derives_;

            void import(const String &file) noexcept;
            bool isStructNeedingInterface(StructPtr structObj) const noexcept;

            void finalize(std::stringstream &ss) const noexcept;
          };

          struct GenerationOptions
          {
            struct Optional {
              Optional(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              bool value_{};
            };
            struct Interface {
              Interface(bool value) noexcept : value_(value) {}
              operator bool() const noexcept { return value_; }
              bool value_{};
            };

            GenerationOptions() noexcept {}
            GenerationOptions(const Optional &value) noexcept : optional_(value) {}
            GenerationOptions(const Interface &value) noexcept : interface_(value) {}
            GenerationOptions(
                              const Optional &value1,
                              const Interface &value2
                              ) noexcept : optional_(value1), interface_(value2) {}

            bool optional_ {false};
            bool interface_ {false};
          };

          GenerateStructMsidl() noexcept;

          static GenerateStructMsidlPtr create() noexcept;

          static String fixName(const String &originalName) noexcept;
          static String fixName(ContextPtr structObj) noexcept;
          static String fixNamePath(
                                    ContextPtr context,
                                    const GenerationOptions &options
                                    ) noexcept;
          static String toIdlType(BasicTypePtr basicType) noexcept;
          static String toIdlType(
                                  IDLFile &idl,
                                  const GenerationOptions &options,
                                  BasicTypePtr basicType
                                  ) noexcept;
          static String toIdlSimpleType(
                                        IDLFile &idl,
                                        const GenerationOptions &options,
                                        TypePtr type
                                        ) noexcept;
          static String toIdlSimpleType(
                                        IDLFile &idl,
                                        const GenerationOptions &options,
                                        const String &typeName
                                        ) noexcept;
          static String toIdlType(
                                  IDLFile &idl,
                                  const GenerationOptions &options,
                                  TypePtr type
                                  ) noexcept;

          static void fixHiddenAttribute(
                                         IDLFile &idl,
                                         ContextPtr context,
                                         const String &indentStr,
                                         std::stringstream &ss
                                         ) noexcept;
          static void fixDeprecatedAttribute(
                                             IDLFile &idl,
                                             ContextPtr context,
                                             const String &indentStr,
                                             std::stringstream &ss
                                             ) noexcept;
          static void fixMethodNameAttribute(
                                             IDLFile &idl,
                                             ContextPtr context,
                                             const String &indentStr,
                                             std::stringstream &ss
                                             ) noexcept;
          static void fixDefaultAttribute(
                                          IDLFile &idl,
                                          ContextPtr context,
                                          const String &indentStr,
                                          std::stringstream &ss
                                          ) noexcept;

          static void scanNamespaceForStructsNeedingToBeInterfaces(
                                                                   StructSet &needingInterfaceSet,
                                                                   NamespacePtr namespaceObj,
                                                                   int scanPass = 0
                                                                   ) noexcept;

          static void scanStructForStructsNeedingToBeInterfaces(
                                                                StructSet &needingInterfaceSet,
                                                                StructPtr structObj,
                                                                int scanPass
                                                                ) noexcept;

          static bool doesAnyRelationHaveInterface(
                                                   const StructSet &needingInterfaceSet,
                                                   StructPtr structObj,
                                                   StructSet &alreadyScanned
                                                   ) noexcept;

          static void markAllRelatedStructsAsNeedingInterface(
                                                              StructSet &needingInterfaceSet,
                                                              StructPtr structObj
                                                              ) noexcept;
          
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

          static bool ctorNeedsToBecomeStaticMethod(
                                                    StructPtr structObj,
                                                    MethodPtr currentCtor,
                                                    String &ioOverrideMethodName
                                                    ) noexcept;

          void processNamespace(
                                IDLFile &forwardIdl,
                                IDLFile &outputIdl,
                                NamespacePtr namespaceObj
                                ) noexcept;

          void processStruct(
                             IDLFile &forwardIdl,
                             IDLFile &outputIdl,
                             StructPtr structObj
                             ) noexcept;
          void processProperties(
                                 IDLFile &idl,
                                 StructPtr structObj,
                                 std::stringstream &ss,
                                 std::stringstream &staticsSS,
                                 bool &outFoundProperty
                                 ) noexcept;
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
                              ) noexcept;

          void processEnum(
                           IDLFile &forwardIdl,
                           IDLFile &outputIdl,
                           EnumTypePtr enumObj
                           ) noexcept;

          //-------------------------------------------------------------------
          //
          // GenerateStructMsidl::IIDLCompilerTarget
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
