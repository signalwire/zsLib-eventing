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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructCx.h>
//#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructImplCpp.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateTypesHeader.h>
//#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructHeader.h>
//#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructImplHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>

//#include <zsLib/eventing/IHelper.h>
//#include <zsLib/eventing/IHasher.h>
//#include <zsLib/eventing/IEventingTypes.h>
//
//#include <zsLib/Exception.h>
//#include <zsLib/Numeric.h>
//
#include <sstream>
//#include <list>
//#include <set>
//#include <cctype>

#define ZS_WRAPPER_COMPILER_DIRECTIVE_EXCLUSIZE "EXCLUSIVE"

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zsLib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IIDLTypes::Project, Project);

    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::tool::internal::Helper, UseHelper);
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHasher, UseHasher);
      typedef std::set<String> HashSet;

      namespace internal
      {

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructCx
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructCx::GenerateStructCx() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructCxPtr GenerateStructCx::create()
        {
          return make_shared<GenerateStructCx>();
        }

        //-------------------------------------------------------------------
        void GenerateStructCx::insertFirst(
                                           std::stringstream &ss,
                                           bool &first
                                           )
        {
          GenerateTypesHeader::insertFirst(ss, first);
        }

        //-------------------------------------------------------------------
        void GenerateStructCx::insertLast(
                                          std::stringstream &ss,
                                          bool &first
                                          )
        {
          GenerateTypesHeader::insertLast(ss, first);
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::fixName(const String &originalName)
        {
          if (originalName.isEmpty()) return String();
          String firstLetter = originalName.substr(0, 1);
          String remaining = originalName.substr(1);
          firstLetter.toUpper();
          return firstLetter + remaining;
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::processTypesNamespace(
                                                     std::stringstream &ss,
                                                     const String &inIndentStr,
                                                     NamespacePtr namespaceObj,
                                                     bool outputEnums
                                                     )
        {
          if (!namespaceObj) return;
          if (namespaceObj->hasModifier(Modifier_Special)) return;

          int parentCount = 0;
          String initialIndentStr;
          String indentStr(inIndentStr);

          {
            auto parent = namespaceObj->getParent();
            while (parent)
            {
              auto checkObj = parent->toNamespace();
              if (!checkObj) break;
              if (!checkObj->mName.hasData()) break;
              ++parentCount;
              indentStr += "  ";
              parent = parent->getParent();
            }
          }

          initialIndentStr = indentStr;

          if (namespaceObj->mName.hasData()) {
            ss << indentStr << "namespace " << fixName(namespaceObj->mName) << " {\n";
          }

          indentStr += "  ";

          bool firstNamespace {true};
          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            if (subNamespaceObj->hasModifier(Modifier_Special)) continue;

            if (!firstNamespace) {
              ss << "\n";
            }
            firstNamespace = false;
            processTypesNamespace(ss, inIndentStr, subNamespaceObj, outputEnums);
          }

          bool firstEnum {true};
          if (outputEnums) {
            for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter)
            {
              auto enumObj = (*iter).second;
              firstEnum = false;
              ss << "\n";
              ss << indentStr << "public enum class " << fixName(enumObj->mName) << "\n";
              ss << indentStr << "{\n";
              for (auto iterValue = enumObj->mValues.begin(); iterValue != enumObj->mValues.end(); ++iterValue)
              {
                auto valueObj = (*iterValue);
                ss << indentStr << "  " << fixName(valueObj->mName);
                if (valueObj->mValue.hasData()) {
                  ss << " = " << valueObj->mValue;
                }
                ss << ",\n";
              }
              ss << indentStr << "};\n";
            }
            insertLast(ss, firstEnum);
          }

          bool firstStruct {firstEnum};
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto structObj = (*iter).second;
            if (structObj->hasModifier(Modifier_Special)) continue;
            if (structObj->mGenerics.size() > 0) continue;

            insertFirst(ss, firstStruct);
            if (structObj->hasModifier(Modifier_Struct_Dictionary)) {
              ss << indentStr << "ref struct " << fixName(structObj->mName) << ";\n";
            } else {
              ss << indentStr << "ref class " << fixName(structObj->mName) << ";\n";
            }
          }
          if (namespaceObj->mStructs.size() > 0) {
            insertLast(ss, firstStruct);
          }

          indentStr = initialIndentStr;

          if (namespaceObj->mName.hasData()) {
            ss << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n";
          }
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr GenerateStructCx::generateTypesHeader(ProjectPtr project) throw (Failure)
        {
          std::stringstream ss;

          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          ss << "#pragma once\n\n";
          ss << "#include \"../generated/types.h\"\n";
          ss << "\n";

          processTypesNamespace(ss, String(), project->mGlobal, true);

          return UseHelper::convertToBuffer(ss.str());
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::caclculateDerives(
                                                 StructPtr structObj,
                                                 NamePathStructSetMap &ioDerivesInfo
                                                 )
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructCx::targetKeyword()
        {
          return String("cx");
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::targetKeywordHelp()
        {
          return String("Generate C++/CX UWP wrapper");
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::targetOutput(
                                            const String &inPathStr,
                                            const ICompilerTypes::Config &config
                                            ) throw (Failure)
        {
          typedef std::stack<NamespacePtr> NamespaceStack;
          typedef std::stack<String> StringList;

          String pathStr(UseHelper::fixRelativeFilePath(inPathStr, String("wrapper")));

          try {
            UseHelper::mkdir(pathStr);
          }
          catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";
          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("cx"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          writeBinary(UseHelper::fixRelativeFilePath(pathStr, String("types.h")), generateTypesHeader(project));

          NamespaceStack namespaceStack;

          namespaceStack.push(project->mGlobal);

          while (namespaceStack.size() > 0)
          {
            auto namespaceObj = namespaceStack.top();
            namespaceStack.pop();
            if (!namespaceObj) continue;
            if (namespaceObj->hasModifier(Modifier_Special)) continue;

            for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
            {
              auto subNamespaceObj = (*iter).second;
              namespaceStack.push(subNamespaceObj);
            }

            for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
            {
              auto structObj = (*iter).second;
              if (structObj->hasModifier(Modifier_Special)) continue;
              if (structObj->mGenerics.size() > 0) continue;

              String filename;// = GenerateStructHeader::getStructFileName(structObj);

              String outputname = UseHelper::fixRelativeFilePath(pathStr, filename);

              std::stringstream ss;
              std::stringstream includeSS;
              std::stringstream structSS;
              StringList endStrings;

              ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
              ss << "#pragma once\n\n";
              ss << "#include \"types.h\"\n";

              structSS << "namespace wrapper {\n";

              NamespaceStack parentStack;
              auto parent = structObj->getParent();
              while (parent) {
                auto parentNamespace = parent->toNamespace();
                if (parentNamespace) {
                  parentStack.push(parentNamespace);
                }
                parent = parent->getParent();
              }

              String indentStr = "  ";

              while (parentStack.size() > 0)
              {
                auto parentNamespace = parentStack.top();
                parentStack.pop();

                if (parentNamespace->mName.hasData()) {
                  structSS << indentStr << "namespace " << parentNamespace->mName << " {\n";
                  {
                    std::stringstream endSS;
                    endSS << indentStr << "} // " << parentNamespace->mName << "\n";
                    endStrings.push(endSS.str());
                  }

                  indentStr += "  ";
                }
              }

              {
                //GenerateStructHeader::StringSet processedHeaders;
                //GenerateStructHeader::generateStruct(structObj, indentStr, processedHeaders, includeSS, structSS);
              }

              ss << includeSS.str();
              ss << "\n";
              ss << structSS.str();
              ss << "\n";
              while (endStrings.size() > 0) {
                ss << endStrings.top();
                endStrings.pop();
              }
              ss << "} // namespace wrapper\n\n";

              //writeBinary(outputname, UseHelper::convertToBuffer(ss.str()));
            }
          }
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
