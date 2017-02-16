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
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructHeader.h>
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
        #pragma mark GenerateStructCx::HelperFile
        #pragma mark

        //---------------------------------------------------------------------
        void GenerateStructCx::HelperFile::includeHeader(const String &headerFile)
        {
          auto &ss = mHeaderIncludeSS;

          if (mHeaderAlreadyIncluded.end() != mHeaderAlreadyIncluded.find(headerFile)) return;
          mHeaderAlreadyIncluded.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::HelperFile::includeCpp(const String &headerFile)
        {
          auto &ss = mCppIncludeSS;

          if (mCppAlreadyIncluded.end() != mCppAlreadyIncluded.find(headerFile)) return;
          mCppAlreadyIncluded.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

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
        String GenerateStructCx::fixNamePath(ContextPtr context)
        {
          ContextList parents;
          while (context) {
            if (context->toProject()) break;
            auto namespaceObj = context->toNamespace();
            if (namespaceObj) {
              if (namespaceObj->mName.isEmpty()) break;
            }
            parents.push_front(context);
            context = context->getParent();
          }

          bool lastWasStruct = false;
          String path;
          for (auto iter = parents.begin(); iter != parents.end(); ++iter)
          {
            context = (*iter);

            auto structObj = context->toStruct();
            if ((structObj) && (lastWasStruct)) {
              path += "_";
            } else {
              path += "::";
            }

            path += fixName(context->mName);
            lastWasStruct = ((bool)structObj);
          }
          return path;
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::fixStructName(StructPtr structObj)
        {
          typedef std::list<StructPtr> StructList;

          StructList parents;
          while (structObj) {
            parents.push_front(structObj);
            auto parentObj = structObj->getParent();
            if (!parentObj) break;

            structObj = parentObj->toStruct();
          }

          bool first = true;
          String name;
          for (auto iter = parents.begin(); iter != parents.end(); ++iter)
          {
            structObj = (*iter);

            if (!first) {
              name += "_";
            }
            first = false;

            name += fixName(structObj->mName);
          }
          return name;
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::fixStructFileName(StructPtr structObj)
        {
          auto result = fixNamePath(structObj);
          result.replaceAll("::", "_");
          result.trim("_");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::getStructInitName(StructPtr structObj)
        {
          return GenerateStructHeader::getStructInitName(structObj);
        }

        //-------------------------------------------------------------------
        String GenerateStructCx::getCxStructInitName(StructPtr structObj)
        {
          String namePathStr = fixNamePath(structObj);
          namePathStr.replaceAll("::", "_");
          namePathStr.trim("_");
          return namePathStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::fixEnumName(EnumTypePtr enumObj)
        {
          StructPtr structObj;
          auto parent = enumObj->getParent();
          if (parent) structObj = parent->toStruct();

          ContextList parents;
          parents.push_front(enumObj);

          while (structObj) {
            parents.push_front(structObj);
            auto parentObj = structObj->getParent();
            if (!parentObj) break;

            structObj = parentObj->toStruct();
          }

          bool first = true;
          String name;
          for (auto iter = parents.begin(); iter != parents.end(); ++iter)
          {
            auto context = (*iter);

            if (!first) {
              name += "_";
            }
            first = false;

            name += fixName(context->mName);
          }
          return name;
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::fixArgumentName(const String &originalName)
        {
          if (originalName == "delegate") return "delegateValue";
          if (originalName == "event") return "eventValue";
          return originalName;
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::processTypesNamespace(
                                                     std::stringstream &ss,
                                                     const String &inIndentStr,
                                                     NamespacePtr namespaceObj
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

          if (!namespaceObj->isGlobal()) {
            ss << indentStr << "namespace " << fixName(namespaceObj->mName) << " {\n";
            auto parent = namespaceObj->getParent();
            if (parent) {
              auto parentNamespace = parent->toNamespace();
              if (parentNamespace->isGlobal()) {
                GenerateStructHeader::generateUsingTypes(ss, indentStr + "  ");
              }
            }
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
            processTypesNamespace(ss, inIndentStr, subNamespaceObj);
          }

          processTypesEnum(ss, indentStr, namespaceObj);

          bool firstStruct = true;
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto structObj = (*iter).second;
            processTypesStruct(ss, indentStr, structObj, firstStruct);
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
        void GenerateStructCx::processTypesStruct(
                                                  std::stringstream &ss,
                                                  const String &indentStr,
                                                  StructPtr structObj,
                                                  bool &firstFound
                                                  )
        {
          if (!structObj) return;
          if (structObj->hasModifier(Modifier_Special)) return;
          if (structObj->mGenerics.size() > 0) return;

          insertFirst(ss, firstFound);
          if (structObj->hasModifier(Modifier_Struct_Dictionary)) {
            ss << indentStr << "ref struct " << fixStructName(structObj) << ";\n";
          } else {
            ss << indentStr << "ref class " << fixStructName(structObj) << ";\n";
          }

          bool found = processTypesEnum(ss, indentStr, structObj);
          if (found) firstFound = true;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processTypesStruct(ss, indentStr, subStructObj, firstFound);
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructCx::processTypesEnum(
                                                std::stringstream &ss,
                                                const String &indentStr,
                                                ContextPtr context
                                                )
        {
          auto namespaceObj = context->toNamespace();
          auto structObj = context->toStruct();
          if ((!namespaceObj) && (!structObj)) return false;

          bool found = false;

          auto &enums = namespaceObj ? (namespaceObj->mEnums) : (structObj->mEnums);
          for (auto iter = enums.begin(); iter != enums.end(); ++iter)
          {
            auto enumObj = (*iter).second;
            found = true;
            ss << "\n";
            ss << indentStr << "public enum class " << fixEnumName(enumObj) << "\n";
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

          return found;
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr GenerateStructCx::generateTypesHeader(ProjectPtr project) throw (Failure)
        {
          std::stringstream ss;

          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          ss << "#pragma once\n\n";
          ss << "#include \"../types.h\"\n";
          ss << "\n";

          processTypesNamespace(ss, String(), project->mGlobal);

          return UseHelper::convertToBuffer(ss.str());
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::calculateRelations(
                                                  NamespacePtr namespaceObj,
                                                  NamePathStructSetMap &ioDerivesInfo
                                                  )
        {
          if (!namespaceObj) return;
          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            calculateRelations(subNamespaceObj, ioDerivesInfo);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto structObj = (*iter).second;
            calculateRelations(structObj, ioDerivesInfo);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::calculateRelations(
                                                  StructPtr structObj,
                                                  NamePathStructSetMap &ioDerivesInfo
                                                  )
        {
          if (!structObj) return;

          String currentNamePath = structObj->getPathName();

          StructSet allParents;
          allParents.insert(structObj);

          while (allParents.size() > 0)
          {
            auto top = allParents.begin();
            StructPtr parentStructObj = (*top);
            allParents.erase(top);
            
            if (structObj != parentStructObj) {
              insertInto(parentStructObj, currentNamePath, ioDerivesInfo);
            }
            insertInto(structObj, parentStructObj->getPathName(), ioDerivesInfo);

            for (auto iter = parentStructObj->mIsARelationships.begin(); iter != parentStructObj->mIsARelationships.end(); ++iter)
            {
              auto foundObj = (*iter).second;
              if (!foundObj) continue;
              auto foundStructObj = foundObj->toStruct();
              if (!foundStructObj) continue;
              allParents.insert(foundStructObj);
            }
          }

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto foundStruct = (*iter).second;
            calculateRelations(foundStruct, ioDerivesInfo);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::insertInto(
                                          StructPtr structObj,
                                          const NamePath &namePath,
                                          NamePathStructSetMap &ioDerivesInfo
                                          )
        {
          if (!structObj) return;

          auto found = ioDerivesInfo.find(namePath);
          if (found == ioDerivesInfo.end()) {
            StructSet newSet;
            newSet.insert(structObj);
            ioDerivesInfo[namePath] = newSet;
            return;
          }

          auto &existingSet = (*found).second;
          existingSet.insert(structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateSpecialHelpers(HelperFile &helperFile)
        {
          auto &derives = helperFile.mDerives;

          helperFile.includeCpp("<zsLib/SafeInt.h>");
          helperFile.includeCpp("<zsLib/date.h>");
          helperFile.includeCpp("<zsLib/helpers.h>");
          helperFile.includeHeader("<zsLib/eventing/types.h>");

          generateBasicTypesHelper(helperFile);

          generateStringHelper(helperFile);
          generateBinaryHelper(helperFile);
          generateExceptionHelper(helperFile);

          if (derives.end() != derives.find("::zs::Hours")) {
            generateDurationHelper(helperFile, "Hours");
          }
          if (derives.end() != derives.find("::zs::Minutes")) {
            generateDurationHelper(helperFile, "Minutes");
          }
          if (derives.end() != derives.find("::zs::Seconds")) {
            generateDurationHelper(helperFile, "Seconds");
          }
          if (derives.end() != derives.find("::zs::Milliseconds")) {
            generateDurationHelper(helperFile, "Milliseconds");
          }
          if (derives.end() != derives.find("::zs::Microseconds")) {
            generateDurationHelper(helperFile, "Microseconds");
          }
          if (derives.end() != derives.find("::zs::Nanoseconds")) {
            generateDurationHelper(helperFile, "Nanoseconds", true);
          }
          if (derives.end() != derives.find("::zs::Time")) {
            generateTimeHelper(helperFile);
          }
          if (derives.end() != derives.find("::zs::Promise")) {
            generatePromiseHelper(helperFile);
          }
          if (derives.end() != derives.find("::zs::PromiseWith")) {
            generatePromiseWithHelper(helperFile);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateExceptionHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          auto &derives = helperFile.mDerives;
          if (derives.end() != derives.find("::zs::exceptions::Exception")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::FailureException^ ToCx(const ::zsLib::Exception &e) { return ref new Platform::FailureException(ToCx(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::InvalidParameters")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::InvalidArgumentException^ ToCx(const ::zsLib::InvalidParameters &e) { return ref new Platform::InvalidArgumentException(ToCx(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::InvalidState")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::COMException^ ToCx(const ::zsLib::InvalidState &e) { return ref new Platform::COMException(E_NOT_VALID_STATE, ToCx(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotImplemented")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::NotImplementedException^ ToCx(const ::zsLib::NotImplemented &e) { return ref new Platform::NotImplementedException(ToCx(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotSupported")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::COMException^ ToCx(const ::zsLib::NotSupported &e) { return ref new Platform::COMException(CO_E_NOT_SUPPORTED, ToCx(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::Unexpected")) {
            ss << helperFile.mHeaderIndentStr << "static Platform::COMException^ ToCx(const ::zsLib::Unexpected &e) { return ref new Platform::COMException(E_UNEXPECTED, ToCx(e.message())); }\n";
          }
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateBasicTypesHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          typedef std::map<String, String> StringMap;

          StringMap basicTypes;

          basicTypes["Platform::Boolean"] = "bool";
          basicTypes["int8"] = "signed char";
          basicTypes["uint8"] = "unsigned char";
          basicTypes["int16"] = "signed short";
          basicTypes["uint16"] = "unsigned short";
          basicTypes["int32"] = "signed int";
          basicTypes["uint32"] = "unsigned int";
          basicTypes["int64"] = "signed long";
          basicTypes["uint64"] = "unsigned long";
          basicTypes["int64"] = "signed long long";
          basicTypes["uint64"] = "unsigned long long";
          basicTypes["float32"] = "float";
          basicTypes["float64"] = "double";

          for (auto iter = basicTypes.begin(); iter != basicTypes.end(); ++iter)
          {
            String cxType = (*iter).first;
            String cppType = (*iter).second;

            bool safeIntType = true;
            if ((cxType == "Platform::Boolean") ||
              (cxType == "float32") ||
              (cxType == "float64")) {
              safeIntType = false;
            }

            bool isFloat64 = false;
            if (cxType == "float64") isFloat64 = true;

            ss << indentStr << "static " << cxType << " ToCx(" << cppType << " value);\n";
            if (isFloat64) {
              ss << indentStr << "static " << cxType << " ToCx(long double value);\n";
            }
            ss << indentStr << "static " << cppType << " FromCx(" << cxType << " value);\n";
            ss << indentStr << "static Platform::IBox<" << cxType << ">^ ToCx(const Optional<" << cppType << "> &value);\n";
            if (isFloat64) {
              ss << indentStr << "static Platform::IBox<" << cxType << ">^ ToCx(const Optional<long double> &value);\n";
            }
            ss << indentStr << "static Optional<" << cppType << "> FromCx(Platform::IBox<" << cxType << ">^ value);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << cxType << " Internal::Helper::ToCx(" << cppType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  return SafeInt<" << cxType << ">(value)\n";
            }
            else {
              cppSS << "  return value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            if (isFloat64) {
              cppSS << dashedStr;
              cppSS << cxType << " Internal::Helper::ToCx(long double value)\n";
              cppSS << "{\n";
              cppSS << "  return value;\n";
              cppSS << "}\n";
              cppSS << "\n";
            }

            cppSS << dashedStr;
            cppSS << cppType << " Internal::Helper::FromCx(" << cxType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  return SafeInt<" << cppType << ">(value);\n";
            }
            else {
              cppSS << "  return (" << cppType << ")value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "Platform::IBox<" << cxType << ">^ Internal::Helper::ToCx(const Optional<" << cppType << "> &value)\n";
            cppSS << "{\n";
            cppSS << "  if (!value.hasValue()) return nullptr;\n";
            if (safeIntType) {
              cppSS << "  return ref new Platform::Box<" << cxType << ">(SafeInt<" << cxType << ">(value.value()));\n";
            }
            else {
              cppSS << "  return ref new Platform::Box<" << cxType << ">(value.value());\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            if (isFloat64) {
              cppSS << dashedStr;
              cppSS << "Platform::IBox<" << cxType << ">^ Internal::Helper::ToCx(const Optional<long double> &value)\n";
              cppSS << "{\n";
              cppSS << "  if (!value.hasValue()) return nullptr;\n";
              if (safeIntType) {
                cppSS << "  return ref new Platform::Box<" << cxType << ">(SafeInt<" << cxType << ">(value.value()));\n";
              }
              else {
                cppSS << "  return ref new Platform::Box<" << cxType << ">(value.value());\n";
              }
              cppSS << "}\n";
              cppSS << "\n";
            }

            cppSS << dashedStr;
            cppSS << "Optional<" << cppType << "> Internal::Helper::FromCx(Platform::IBox<" << cxType << ">^ value)\n";
            cppSS << "{\n";
            cppSS << "  Optional<" << cppType << "> result;\n";
            cppSS << "  if (nullptr == value) return result;\n";
            if (safeIntType) {
              cppSS << "  result = SafeInt<" << cppType << ">(input->Value);\n";
            }
            else {
              cppSS << "  result = (" << cppType << ")input->Value;\n";
            }
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateBinaryHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          ss << indentStr << "static Platform::Array<byte>^ ToCx(const SecureByteBlock &value);\n";
          ss << indentStr << "static Platform::Array<byte>^ ToCx(SecureByteBlockPtr value);\n";
          ss << indentStr << "static Platform::Array<byte>^ ToCx(const Optional<SecureByteBlockPtr> &value);\n";
          ss << indentStr << "static SecureByteBlockPtr FromCx(Platform::Array<byte>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCx(const SecureByteBlock &value)\n";
          cppSS << "{\n";
          cppSS << "  return ref new Platform::Array(value->BytePtr(), value->SizeInBytes());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCx(SecureByteBlockPtr value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return nullptr;\n";
          cppSS << "  return ToCx(*value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCx(const Optional<SecureByteBlockPtr> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  if (!value.value()) return nullptr;\n";
          cppSS << "  return ToCx(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "SecureByteBlockPtr Internal::Helper::FromCx(Platform::Array<byte>^ value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return SecureByteBlockPtr();\n";
          cppSS << "  return make_shared<SecureByteBlock>(value->Data, value->Length);\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateStringHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          ss << indentStr << "static Platform::String^ ToCx(const std::string &value);\n";
          ss << indentStr << "static Platform::String^ ToCx(const Optional<std::string> &value);\n";
          ss << indentStr << "static Platform::String^ ToCx(const Optional<String> &value);\n";
          ss << indentStr << "static Platform::String^ ToCx(const std::wstring &value);\n";
          ss << indentStr << "static Platform::String^ ToCx(const Optional<std::wstring> &value);\n";
          ss << indentStr << "static std::string FromCx(Platform::String^ value);\n";
          ss << indentStr << "static Optional<String> FromCx(Platform::IBox<Platform::String^>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCx(const std::string &value)\n";
          cppSS << "{\n";
          cppSS << "  return ref new Platform::String(String(value).wstring().c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Platform::String^>^ Internal::Helper::ToCx(const Optional< std::string > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Platform::Sring^>(ToCx(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Platform::String^>^ Internal::Helper::ToCx(const Optional< String > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Platform::Sring^>(ToCx(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCx(const std::wstring &value)\n";
          cppSS << "{\n";
          cppSS << "  return ref new Platform::String(value.wstring().c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Platform::String^>^ Internal::Helper::ToCx(const Optional< std::wstring > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Platform::Sring^>(ToCx(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "std::string Internal::Helper::FromCx(Platform::String^ value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return std::string();\n";
          cppSS << "  auto dataStr = value->Data();\n";
          cppSS << "  if (!dataStr) return std::string();\n";
          cppSS << "  return String(std::wstring(dataStr));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Optional<String> Internal::Helper::FromCx(Platform::IBox<Platform::String^>^ value)\n";
          cppSS << "{\n";
          cppSS << "  Optional<String> result;\n";
          cppSS << "  if (!value) return result;\n";
          cppSS << "  result = value->Value;\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateDurationHelper(
                                                      HelperFile &helperFile,
                                                      const String &durationType,
                                                      bool generateFromCx
                                                      )
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          bool isNanoseconds = "Nanoseconds" == durationType;

          ss << indentStr << "static Windows::Foundation::TimeSpan ToCx(const ::zsLib::" << durationType << " &value);\n";
          ss << indentStr << "static Platform::IBox<Windows::Foundation::TimeSpan>^ ToCx(const Optional<::zsLib::" << durationType << "> &value);\n";
          if (generateFromCx) {
            ss << indentStr << "static ::zsLib::" << durationType << " FromCx(Windows::Foundation::TimeSpan value);\n";
            ss << indentStr << "static Optional<::zsLib::" << durationType << "> FromCx(Platform::IBox<Windows::Foundation::TimeSpan>^ value);\n";
          }
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::TimeSpan Internal::Helper::ToCx(const ::zsLib::" << durationType << " &value)\n";
          cppSS << "{\n";
          cppSS << "  Windows::Foundation::TimeSpan result {};\n";
          cppSS << "  result.Duration = " << (isNanoseconds ? "value.count()" : "::zsLib::toNanoseconds(value).count()") << " / static_cast<::zsLib::Nanoseconds::rep>(100);\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Windows::Foundation::TimeSpan>^ Internal::Helper::ToCx(const Optional<::zsLib::" << durationType << "> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Windows::Foundation::TimeSpan>(ToCx(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (generateFromCx) {
            cppSS << dashedStr;
            cppSS << "::zsLib::" << durationType << " Internal::Helper::FromCx(Windows::Foundation::TimeSpan value)\n";
            cppSS << "{\n";
            cppSS << "  ::zsLib::Nanoseconds::rep result {};\n";
            cppSS << "  result = SafeInt<::zsLib::Nanoseconds::rep>(value.Duration) * static_cast<::zsLib::Nanoseconds::rep>(100);\n";
            if (isNanoseconds) {
              cppSS << "  return ::zsLib::Nanoseconds(result);\n";
            } else {
              cppSS << "  return ::zsLib::to" << durationType << "(::zsLib::Nanoseconds(result));\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "Optional<::zsLib::" << durationType << "> Internal::Helper::FromCx(Platform::IBox<Windows::Foundation::TimeSpan>^ value)\n";
            cppSS << "{\n";
            cppSS << "  Optional<::zsLib::" << durationType << "> result;\n";
            cppSS << "  if (!value) return result;\n";
            cppSS << "  result = ToCx(value->Value);\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateTimeHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::DateTime ToCx(const ::zsLib::Time &value);\n";
          ss << indentStr << "static Platform::IBox<Windows::Foundation::DateTime>^ ToCx(const Optional<::zsLib::Time> &value);\n";
          ss << indentStr << "static ::zsLib::Time FromCx(Windows::Foundation::DateTime value);\n";
          ss << indentStr << "static Optional<::zsLib::Time> FromCx(Platform::IBox<Windows::Foundation::DateTime>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::DateTime Internal::Helper::ToCx(const ::zsLib::Time &value)\n";
          cppSS << "{\n";
          cppSS << "  Windows::Foundation::DateTime result {};\n";
          cppSS << "  auto t = day_point(jan / 1 / 1601);\n";
          cppSS << "\n";
          cppSS << "  auto diff = value - t;\n";
          cppSS << "  auto nano = ::zsLib::toNanoseconds(diff);\n";
          cppSS << "\n";
          cppSS << "  result.UniversalTime = SafeInt<decltype(result.UniversalTime)>(nano.count() / static_cast<::zsLib::Nanoseconds::rep>(100));\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Windows::Foundation::DateTime>^ Internal::Helper::ToCx(const Optional<::zsLib::Time> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Windows::Foundation::DateTime>(ToCx(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "::zsLib::Time Internal::Helper::FromCx(Windows::Foundation::DateTime value)\n";
          cppSS << "{\n";
          cppSS << "  ::zsLib::Time t = day_point(jan / 1 / 1601);\n";
          cppSS << "\n";
          cppSS << "  auto nano = ::zsLib::toMilliseconds(zsLib::Nanoseconds(static_cast<::zsLib::Nanoseconds::rep>(value.UniversalTime) * static_cast<::zsLib::Nanoseconds::rep>(100)));\n";
          cppSS << "\n";
          cppSS << "  auto result = t + nano;\n";
          cppSS << "  return zsLib::timeSinceEpoch(result);\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generatePromiseHelper(HelperFile &helperFile)
        {
          helperFile.includeHeader("<ppltasks.h>");

          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::IAsyncAction^ ToCx(::zsLib::PromisePtr promise);\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::IAsyncAction^ Internal::Helper::ToCx(::zsLib::PromisePtr promise)\n";
          cppSS << "{\n";
          cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
          cppSS << "  {\n";
          cppSS << "    Observer(Concurrency::task_completion_event<void> tce) : mTce(tce) {}\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override { mTce.set(); }\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override\n";
          cppSS << "    {\n";
          cppSS << "      if (!promise) {\n";
          cppSS << "        mTce.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
          cppSS << "        return;\n";
          cppSS << "      }\n";

          generateDefaultPromiseRejections(helperFile, "      ");

          cppSS << "      mTce.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
          cppSS << "    }\n";
          cppSS << "\n";
          cppSS << "  private:\n";
          cppSS << "    Concurrency::task_completion_event<void> mTce;\n";
          cppSS << "  };\n";
          cppSS << "\n";
          cppSS << "  Windows::Foundation::IAsyncAction^ result = Concurrency::create_async([promise]()\n";
          cppSS << "  {\n";
          cppSS << "    Concurrency::task_completion_event<void> tce;\n";
          cppSS << "\n";
          cppSS << "    auto observer = make_shared<Observer>(tce);\n";
          cppSS << "    promise->then(observer);\n";
          cppSS << "    promise->background();\n";
          cppSS << "\n";
          cppSS << "    return tceTask.get();\n";
          cppSS << "  });\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }


        //---------------------------------------------------------------------
        void GenerateStructCx::generatePromiseWithHelper(HelperFile &helperFile)
        {
          helperFile.includeHeader("<ppltasks.h>");

          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          auto foundType = helperFile.mGlobal->toContext()->findType("::zs::PromiseWith");
          if (foundType) {
            auto promiseWithStruct = foundType->toStruct();
            if (promiseWithStruct) {
              for (auto iterPromise = promiseWithStruct->mTemplatedStructs.begin(); iterPromise != promiseWithStruct->mTemplatedStructs.end(); ++iterPromise)
              {
                auto templatedStruct = (*iterPromise).second;
                if (!templatedStruct) continue;

                auto foundType = templatedStruct->mTemplateArguments.begin();
                if (foundType == templatedStruct->mTemplateArguments.end()) continue;

                auto resolveType = (*foundType);
                if (!resolveType) continue;

                TypePtr rejectType;
                {
                  (++foundType);
                  if (foundType != templatedStruct->mTemplateArguments.end()) rejectType = *foundType;
                }

                ss << indentStr << "static " << getCxType(false, templatedStruct) << " ToCx(PromiseWith< " << getCppType(false, resolveType) << " > promise);\n";

                cppSS << dashedStr;
                cppSS << getCxType(false, templatedStruct) << " Internal::Helper::ToCx(PromiseWith< " << getCppType(false, resolveType) << " > promise)\n";
                cppSS << "{\n";
                cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
                cppSS << "  {\n";
                cppSS << "    Observer(Concurrency::task_completion_event<void> tce) : mTce(tce) {}\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override\n";
                cppSS << "    {\n";
                cppSS << "      if (!promise) {\n";
                cppSS << "        mTce.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
                cppSS << "        return;\n";
                cppSS << "      }\n";
                cppSS << "      {\n";
                cppSS << "        auto reasonHolder = promise->value< ::zsLib::AnyHolder< " << getCppType(false, resolveType) << " > >();\n";
                cppSS << "        if (reasonHolder) {\n";
                cppSS << "          mTce.set(::Internal::Helper::ToCx(reasonHolder->mValue));\n";
                cppSS << "      }\n";
                cppSS << "      mTce.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
                cppSS << "    }\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override\n";
                cppSS << "    {\n";
                cppSS << "      if (!promise) {\n";
                cppSS << "        mTce.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
                cppSS << "        return;\n";
                cppSS << "      }\n";

                if (rejectType) {
                  auto rejectTypeStr = rejectType->getPathName();
                  if ("::void" != rejectTypeStr) {
                    generatePromiseRejection(helperFile, indentStr, rejectType);
                  }
                }
                generateDefaultPromiseRejections(helperFile, "      ");

                cppSS << "      mTce.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
                cppSS << "    }\n";
                cppSS << "\n";
                cppSS << "  private:\n";
                cppSS << "    Concurrency::task_completion_event< " << getCxType(false, resolveType) << " > mTce;\n";
                cppSS << "  };\n";
                cppSS << "\n";
                cppSS << "  " << getCxType(false, templatedStruct) << " result = Concurrency::create_async([promise]()\n";
                cppSS << "  {\n";
                cppSS << "    Concurrency::task_completion_event< " << getCxType(false, resolveType) << " > tce;\n";
                cppSS << "\n";
                cppSS << "    auto observer = make_shared<Observer>(tce);\n";
                cppSS << "    promise->then(observer);\n";
                cppSS << "    promise->background();\n";
                cppSS << "\n";
                cppSS << "    return tceTask.get();\n";
                cppSS << "  });\n";
                cppSS << "  return result;\n";
                cppSS << "}\n";
                cppSS << "\n";
              }
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateDefaultPromiseRejections(
                                                                HelperFile &helperFile,
                                                                const String &indentStr
                                                                )
        {
          auto &cppSS = helperFile.mCppBodySS;
          auto foundType = helperFile.mGlobal->toContext()->findType("::zs::PromiseRejectionReason");
          if (foundType) {
            auto rejectionStruct = foundType->toStruct();
            if (rejectionStruct) {
              for (auto iterRej = rejectionStruct->mTemplatedStructs.begin(); iterRej != rejectionStruct->mTemplatedStructs.end(); ++iterRej)
              {
                auto templatedStruct = (*iterRej).second;
                if (!templatedStruct) continue;

                auto foundType = templatedStruct->mTemplateArguments.begin();
                if (foundType == templatedStruct->mTemplateArguments.end()) continue;

                auto rejectionType = (*foundType);
                if (!rejectionType) continue;
                generatePromiseRejection(helperFile, indentStr, rejectionType);
              }
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generatePromiseRejection(
                                                        HelperFile &helperFile,
                                                        const String &indentStr,
                                                        TypePtr rejectionType
                                                        )
        {
          if (!rejectionType) return;
          auto &cppSS = helperFile.mCppBodySS;
          cppSS << indentStr << "{\n";
          cppSS << indentStr << "  auto reasonHolder = promise->reason< ::zsLib::AnyHolder< " << getCppType(false, rejectionType) << " > >();\n";
          cppSS << indentStr << "  if (reasonHolder) {\n";
          cppSS << indentStr << "    mTce.set_exception(::Internal::Helper::ToCx(reasonHolder->mValue));\n";
          cppSS << indentStr << "  }\n";
          cppSS << indentStr << "}\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForNamespace(
                                                    HelperFile &helperFile,
                                                    NamespacePtr namespaceObj,
                                                    const String &inIndentStr
                                                    )
        {
          if (!namespaceObj) return;

          String indentStr = inIndentStr;
          if (!namespaceObj->isGlobal()) {
            indentStr += "  ";
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            generateForNamespace(helperFile, subNamespaceObj, indentStr);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto subStructObj = (*iter).second;
            generateForStruct(helperFile, subStructObj, inIndentStr);
          }
          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter)
          {
            auto enumObj = (*iter).second;
            generateForEnum(helperFile, enumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForStruct(
                                                 HelperFile &helperFile,
                                                 StructPtr structObj,
                                                 const String &inIndentStr
                                                 )
        {
          if (!structObj) return;

          auto namePath = structObj->getPathName();
          if (namePath == "::std::list") {
            generateForList(helperFile, structObj);
          }
          if (namePath == "::std::map") {
            generateForMap(helperFile, structObj);
          }
          if (namePath == "::std::set") {
            generateForSet(helperFile, structObj);
          }

          if ((structObj->mGenerics.size() < 1) &&
              (!structObj->hasModifier(Modifier_Special))) {
            generateForStandardStruct(helperFile, structObj);
          }

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto subStructObj = (*iter).second;
            generateForStruct(helperFile, subStructObj, inIndentStr);
          }
          for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter)
          {
            auto enumObj = (*iter).second;
            generateForEnum(helperFile, enumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForStandardStruct(
                                                         HelperFile &helperFile,
                                                         StructPtr structObj
                                                         )
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          helperFile.includeCpp(String("\"") + fixStructFileName(structObj) + ".h\"");

          ss << indentStr << "static " << getCxType(false, structObj) << " ToCx(" << getCppType(false, structObj) << " value);\n";
          ss << indentStr << "static " << getCppType(false, structObj) << " FromCx(" << getCxType(false, structObj) << " value);\n";

          ss << indentStr << "static " << getCxType(true, structObj) << ">^ ToCx(" << getCppType(true, structObj) << "  &value) { if (!value.hasValue()) return nullptr; return ref new Platform::Box< " << getCxType(false, structObj) << " >(ToCx(value.value())); }\n";
          ss << indentStr << "static " << getCppType(true, structObj) << " FromCx(" << getCxType(true, structObj) << " >^ value) { " << getCppType(true, structObj) << " > result; if (!value) return result; result = FromCx(value->Value); return result; }\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << getCxType(false, structObj) << " Internal::Helper::ToCx(" << getCppType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << fixNamePath(structObj) << "::ToCx(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(false, structObj) << " Internal::Helper::FromCx(" << getCxType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << fixNamePath(structObj) << "::FromCx(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          generateStructFile(helperFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateStructFile(
                                                  HelperFile &helperFile,
                                                  StructPtr structObj
                                                  )
        {
          if (!structObj) return;

          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          String filename = fixStructFileName(structObj);

          StructFile structFile;
          structFile.mStruct = structObj;

          auto &includeSS = structFile.mHeaderIncludeSS;
          auto &prestructDelegateSS = structFile.mHeaderPreStructSS;
          auto &ss = structFile.mHeaderStructPrivateSS;
          auto &pubSS = structFile.mHeaderStructPublicSS;
          auto &cppSS = structFile.mCppBodySS;
          auto &cppIncludeSS = structFile.mCppIncludeSS;
          auto &observerSS = structFile.mHeaderStructObserverSS;
          auto &observerFinalSS = structFile.mHeaderStructObserverFinalSS;

          auto &indentStr = structFile.mHeaderStructIndentStr;

          includeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          includeSS << "#pragma once\n\n";
          includeSS << "#include \"types.h\"\n";

          cppIncludeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          cppIncludeSS << "#include \"cx_Helpers.h\"\n";
          cppIncludeSS << "#include \"" << filename << ".h\"\n";
          cppIncludeSS << "\n";

          NamespaceList namespaceParents;
          StringList endingStrs;

          // scope - figure out namespace parents
          {
            StructPtr parentStruct = structObj;

            auto parent = structObj->getParent();
            while (parent) {
              if (!parent) break;

              auto parentNamespace = parent->toNamespace();
              auto parentStructCheck = parent->toStruct();

              if (parentNamespace) {
                if (parentNamespace->isGlobal()) break;
                namespaceParents.push_front(parentNamespace);
              }
              if (parentStructCheck) {
                parentStruct = parentStructCheck;
              }
              parent = parent->getParent();
            }
            includeSS << "#include \"../" << GenerateStructHeader::getStructFileName(parentStruct) << "\"\n";
            includeSS << "\n";
          }

          // insert namespace
          {
            for (auto iter = namespaceParents.begin(); iter != namespaceParents.end(); ++iter)
            {
              auto namespaceObj = (*iter);
              auto nameStr = fixName(namespaceObj->mName);
              includeSS << indentStr << "namespace " << nameStr << " {\n";
              endingStrs.push_front(indentStr + "} // namespace " + nameStr);

              indentStr += "  ";
            }
          }

          bool hasConstructor = false;
          bool hasEvents = false;
          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Method_EventHandler)) hasEvents = true;
            if (method->hasModifier(Modifier_Method_Ctor)) hasConstructor = true;
          }

          ss << "\n";
          ss << indentStr << "public ref " << (structObj->hasModifier(Modifier_Struct_Dictionary) ? "struct" : "class") << " " << fixStructName(structObj) << " sealed\n";
          ss << indentStr << "{\n";
          ss << indentStr << "internal:\n";
          pubSS << indentStr << "public:\n";
          endingStrs.push_front(indentStr + "};\n");

          structFile.mHeaderIndentStr = indentStr;

          indentStr += "  ";

          ss << indentStr << getCppType(false, structObj) << " native_;\n";
          if (hasEvents) {
            ss << indentStr << "wrapper" << structObj->getPathName() << "::WrapperObserverPtr observer_;\n";
          }
          ss << "\n";
          ss << indentStr << "struct WrapperCreate {};\n";
          ss << indentStr << fixStructName(structObj) << "(const WrapperCreate &) {}\n";
          ss << "\n";
          ss << indentStr << "static " << fixStructName(structObj) << "^ ToCx(" << getCppType(false, structObj) << " value);\n";
          ss << indentStr << "static " << getCppType(false, structObj) << " FromCx(" << fixStructName(structObj) << "^ value);\n";

          cppSS << dashedStr;
          cppSS << getCxType(false, structObj) << " " << fixStructName(structObj) << "::ToCx(" << getCppType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return nullptr;\n";
          cppSS << "  auto result = ref new " << fixStructName(structObj) << "(WrapperCreate{});\n";
          cppSS << "  result->observer_ = make_shared<WrapperObserverImpl>(result);\n";
          cppSS << "  result->native_ = value;\n";
          if (hasEvents) {
            cppSS << "  result->native_->wrapper_installObserver(observer_);\n";
          }
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(false, structObj) << " " << fixStructName(structObj) << "::FromCx(" << getCxType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return " << getCppType(false, structObj) << "();\n";
          cppSS << "  return value->native_;\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (!hasConstructor) {
            bool staticOnlyMethods = GenerateStructHeader::hasOnlyStaticMethods(structObj);

            if (!staticOnlyMethods) {
              pubSS << indentStr << fixStructName(structObj) << "();";

              cppSS << dashedStr;
              cppSS << fixNamePath(structObj) << "::" << fixStructName(structObj) << "\n";
              cppSS << "  : native_(" << "wrapper" << structObj->getPathName() << "::wrapper_create()" << ")";
              if (hasEvents) {
                cppSS << ",\n";
                cppSS << "    observer_(make_shared<WrapperObserverImpl>(this))";
              }
              cppSS << "\n";
              cppSS << "{\n";
              if (hasEvents) {
                cppSS << "  native_->wrapper_installObserver(observer_);\n";
              }
              cppSS << "  wrapper_init_" << getStructInitName(structObj) << "();\n";
              cppSS << "}\n";
              cppSS << "\n";
            }
          }

          if (hasEvents) {
            observerSS << indentStr << "struct WrapperObserverImpl : public wrapper" << structObj->getPathName() << "::WrapperObserver \n";
            observerSS << indentStr << "{\n";
            observerSS << indentStr << "  WrapperObserverImpl(" << fixStructName(structObj) << " owner) : owner_(owner) {}\n";
            observerSS << "\n";
            observerFinalSS << indentStr << "  " << fixStructName(structObj) << "^ owner_;\n";
            observerFinalSS << indentStr << "};\n";
          }

          {
            bool foundCast = false;
            auto found = helperFile.mDerives.find(structObj->getPathName());
            if (found != helperFile.mDerives.end()) {
              auto &structSet = (*found).second;
              for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet)
              {
                auto foundStruct = (*iterSet);
                if (foundStruct != structObj) {
                  pubSS << indentStr << "static " << fixStructName(structObj) << "^ Cast(" << getCxType(false, foundStruct) << " value);\n";
                  foundCast = true;

                  cppSS << dashedStr;
                  cppSS << getCxType(false, structObj) << " " << fixNamePath(structObj) << "::Cast(" << getCxType(false, foundStruct) << " value)\n";
                  cppSS << "{\n";
                  cppSS << "  if (nullptr == value) return nullptr;\n";
                  cppSS << "  auto result = std::dynamic_pointer_cast< wrapper" << structObj->getPathName() << " >(value);\n";
                  cppSS << "  if (!result) return nullptr;\n";
                  cppSS << "  return ToCx(result);\n";
                  cppSS << "}\n";
                  cppSS << "\n";
                }
              }
            }
            if (foundCast) pubSS << "\n";
          }

          generateStructMethods(helperFile, structFile, structObj, true, hasEvents);

          includeSS << "\n";
          includeSS << prestructDelegateSS.str();
          includeSS << "\n";
          includeSS << ss.str();
          includeSS << "\n";
          includeSS << observerSS.str() << "\n";
          includeSS << observerFinalSS.str() << "\n";
          includeSS << pubSS.str();

          includeSS << "\n";
          // insert ending namespaces
          {
            for (auto iter = endingStrs.begin(); iter != endingStrs.end(); ++iter)
            {
              includeSS << (*iter) << "\n";
            }
          }

          cppIncludeSS << "\n";
          cppIncludeSS << cppSS.str();
          cppIncludeSS << "\n";

          String outputnameHeader = UseHelper::fixRelativeFilePath(helperFile.mHeaderFileName, filename + ".h");
          String outputnameCpp = UseHelper::fixRelativeFilePath(helperFile.mHeaderFileName, filename + ".cpp");
          writeBinary(outputnameHeader, UseHelper::convertToBuffer(includeSS.str()));
          writeBinary(outputnameCpp, UseHelper::convertToBuffer(cppIncludeSS.str()));
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateStructMethods(
                                                     HelperFile &helperFile,
                                                     StructFile &structFile,
                                                     StructPtr structObj,
                                                     bool createConstructors,
                                                     bool hasEvents
                                                     )
        {
          if (!structObj) return;

          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          auto &headerMethodsSS = structFile.mHeaderStructPublicSS;
          auto &prestructDelegateSS = structFile.mHeaderPreStructSS;
          auto &observerSS = structFile.mHeaderStructObserverSS;
          auto &cppSS = structFile.mCppBodySS;
          auto &indentStr = structFile.mHeaderStructIndentStr;

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedType = (*iter).second;
            if (!relatedType) continue;
            
            {
              auto subStructObj = relatedType->toStruct();
              if (subStructObj) {
                generateStructMethods(helperFile, structFile, subStructObj, false, hasEvents);
              }
            }
          }

          bool firstOutput = true;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            
            bool isCtor = method->hasModifier(Modifier_Method_Ctor);
            bool isEvent = method->hasModifier(Modifier_Method_EventHandler);
            String delegateName;

            std::stringstream implSS;

            if (!createConstructors) {
              if ((isCtor) || (isEvent)) continue;
            }

            if (firstOutput) {
              headerMethodsSS << indentStr << "// " << structObj->getPathName() << "\n";
              firstOutput = false;
            }

            bool hasResult = ("::void" != method->mResult->getPathName());
            if ((isCtor) || (isEvent)) hasResult = false;

            String methodImplIndentStr = "  ";

            if (hasResult) {
              implSS << methodImplIndentStr << getCxType(method->hasModifier(Modifier_Optional), method->mResult) << " result {};\n";
            }
            if (method->mThrows.size() > 0) {
              implSS << methodImplIndentStr << "try {\n";
              methodImplIndentStr += "  ";
            }
            if (hasResult) {
              implSS << methodImplIndentStr << "result = ::Internal::Helper::ToCx(";
            } else {
              implSS << methodImplIndentStr;
            }

            if (isEvent) {
              implSS << "owner_->" << fixName(method->mName) << "(";
            } else {
              if (isCtor) {
                implSS << "native_->wrapper_init_" << getStructInitName(structObj) << "(";
              } else {
                implSS << "native_->" << method->mName << "(";
              }
            }

            StringList attributes;
            if (method->hasModifier(Modifier_Method_Default)) {
              attributes.push_back("Windows::Foundation::Metadata::DefaultOverloadAttribute");
            }
            String altName = method->getModifierValue(Modifier_AltName, 0);
            if (altName.hasData()) {
              attributes.push_back("Windows::Foundation::Metadata::OverloadAttribute(\"" + fixName(altName) + "\"))");
            }

            if (attributes.size() > 0) {
              headerMethodsSS << indentStr << "[";
              bool firstAttribute = true;
              while (attributes.size() > 0) {
                String attributeStr = attributes.front();
                attributes.pop_front();
                if (!firstAttribute) headerMethodsSS << ", ";
                firstAttribute = false;
                headerMethodsSS << attributeStr;
              }
              headerMethodsSS << "]\n";
            }

            if (isEvent) {
              hasResult = false;
              observerSS << indentStr << "  virtual void " << method->mName << "(";

              cppSS << dashedStr;
              cppSS << "void " << fixNamePath(structObj) << "::WrapperObserverImpl::" << method->mName << "(";

              delegateName = fixStructName(structObj) + "_";
              if (method->hasModifier(Modifier_AltName)) {
                delegateName += fixName(method->getModifierValue(Modifier_AltName, 0));
              } else {
                delegateName += fixName(method->mName);
              }
              delegateName += "Delegate";
              prestructDelegateSS << structFile.mHeaderIndentStr << "public delegate void " << delegateName << "(";
              if (method->mArguments.size() > 1) {
                observerSS << "\n";
                observerSS << indentStr << "  ";
                cppSS << "\n";
                cppSS << "  ";
                prestructDelegateSS << "\n";
                prestructDelegateSS << structFile.mHeaderIndentStr << "  ";
              }

              headerMethodsSS << indentStr << "event " << delegateName << "^ " << fixName(method->mName) << ";\n";
            } else {
              cppSS << dashedStr;

              headerMethodsSS << indentStr;
              if (!isCtor) {
                cppSS << getCxType(method->hasModifier(Modifier_Optional), method->mResult) << " ";
                headerMethodsSS << getCxType(method->hasModifier(Modifier_Optional), method->mResult) << " ";
              }
              cppSS << fixNamePath(method) << "(";
              headerMethodsSS << fixName(method->mName) << "(";
              if (method->mArguments.size() > 1) {
                cppSS << "\n";
                cppSS << "  ";
                headerMethodsSS << "\n";
                headerMethodsSS << indentStr << "  ";
              }
            }

            bool firstArg = true;
            for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
            {
              auto arg = (*iterArgs);
              if (!firstArg) {
                implSS << ", ";
                if (isEvent) {
                  cppSS << ",\n";
                  cppSS << "  ";
                  observerSS << ",\n";
                  observerSS << indentStr << "  ";
                  prestructDelegateSS << ",\n";
                  prestructDelegateSS << structFile.mHeaderIndentStr << "  ";
                } else {
                  headerMethodsSS << ",\n";
                  headerMethodsSS << indentStr << "  ";
                }
              }
              firstArg = false;
              implSS << "::Internal::Helper::" << (isEvent ? "ToCx" : "FromCx") << "(" << fixArgumentName(arg->mName) << ")";
              if (isEvent) {
                cppSS << getCppType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                observerSS << getCppType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                prestructDelegateSS << getCxType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
              } else {
                cppSS << getCxType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                headerMethodsSS << getCxType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
              }
            }

            if (hasResult) {
              implSS << ")";
            }
            implSS << ");\n";
            for (auto iterThrows = method->mThrows.begin(); iterThrows != method->mThrows.end(); ++iterThrows) {
              auto throwType = (*iterThrows);
              if (!throwType) continue;
              implSS << "  } catch(const " << getCppType(false, throwType) << " &e) {\n";
              implSS << "    throw ToCx(e);\n";
            }
            if (method->mThrows.size() > 0) {
              implSS << "  }\n";
            }
            if (hasResult) {
              implSS << "  return result;\n";
            }

            if (isEvent) {
              if (method->mArguments.size() > 1) {
                cppSS << "\n";
                cppSS << "  ";
                observerSS << "\n";
                observerSS << indentStr << "  ";
                prestructDelegateSS << "\n";
                prestructDelegateSS << structFile.mHeaderIndentStr << "  ";
              }
              observerSS << ") override;\n";
              prestructDelegateSS << ");\n";
            } else {
              if (method->mArguments.size() > 1) {
                cppSS << "\n";
                cppSS << "  ";
                headerMethodsSS << "\n";
                headerMethodsSS << indentStr << "  ";
              }
              headerMethodsSS << ");\n";
            }
            cppSS << ")";
            if (isCtor) {
              cppSS << " : native_(" << "wrapper" << structObj->getPathName() << "::wrapper_create()" << ")";
              if (hasEvents) {
                cppSS << ",\n";
                cppSS << "      observer_(make_shared<WrapperObserverImpl>(this))";
              }
            }

            cppSS << "\n";
            cppSS << "{\n";
            if ((isCtor) && (hasEvents)) {
              cppSS << "  wrapper_installObserver(observer_);\n";
            }
            cppSS << "  if (" << (isEvent ? "nullptr == owner_" : "!native_") << ") {throw ref new Platform::NullReferenceException();}\n";
            cppSS << implSS.str();
            cppSS << "}\n";
            cppSS << "\n";
          }

          if (!firstOutput) headerMethodsSS << "\n";

          for (auto iterProperties = structObj->mProperties.begin(); iterProperties != structObj->mProperties.end(); ++iterProperties)
          {
            auto property = (*iterProperties);

            bool hasGet = true;
            bool hasSet = true;
            bool hasGetter = property->hasModifier(Modifier_Property_Getter);
            bool hasSetter = property->hasModifier(Modifier_Property_Setter);

            if (hasGetter) {
              if (!hasSetter) hasSet = false;
            } else {
              if (hasSetter) hasGet = false;
            }

            headerMethodsSS << indentStr << "property " << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " " << fixName(property->mName);
            if (hasGet) {
              if (hasSet) {
                headerMethodsSS << "\n";
                headerMethodsSS << indentStr << "{\n";
                headerMethodsSS << indentStr << "  " << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " get();\n";
                headerMethodsSS << indentStr << "  void set(" << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " value);\n";
                headerMethodsSS << indentStr << "}\n";
              } else {
                headerMethodsSS << " { " << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " get(); }\n";
              }
            } else {
              if (hasGet) {
                headerMethodsSS << " { void set(" << getCxType(property->hasModifier(Modifier_Optional), property->mType) << "); }\n";
              }
            }

            if (hasGet) {
              cppSS << dashedStr;
              cppSS << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " " << fixNamePath(property) << "::get()\n";
              cppSS << "{\n";
              cppSS << "  if (!native_) {throw ref new Platform::NullReferenceException();}\n";
              cppSS << "  return ::Internal::Helper::ToCx(native_->";
              if (hasGetter) {
                cppSS << "get_" << property->mName << "()";
              } else {
                cppSS << property->mName;
              }
              cppSS << ");\n";
              cppSS << "}\n";
              cppSS << "\n";
            }
            if (hasSet) {
              cppSS << dashedStr;
              cppSS << "void " << fixNamePath(property) << "::set(" << getCxType(property->hasModifier(Modifier_Optional), property->mType) << " value)\n";
              cppSS << "{\n";
              cppSS << "  if (!native_) {throw ref new Platform::NullReferenceException();}\n";
              cppSS << "  native_->";
              if (hasSetter) {
                cppSS << "set_" << property->mName << "(::Internal::Helper::FromCx(value));";
              } else {
                cppSS << property->mName << " = " << "::Internal::Helper::FromCx(value);";
              }
              cppSS << "\n";
              cppSS << "}\n";
              cppSS << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForEnum(
                                               HelperFile &helperFile,
                                               EnumTypePtr enumObj
                                               )
        {
          auto &ss = helperFile.mHeaderStructSS;
          auto &indentStr = helperFile.mHeaderIndentStr;

          ss << indentStr << "static " << getCxType(false, enumObj) << " ToCx(" << getCppType(false, enumObj) << " value) { return (" << getCxType(false, enumObj) << ")value; }\n";
          ss << indentStr << "static " << getCppType(false, enumObj) << " FromCx(" << getCxType(false, enumObj) << " value) { return (" << getCppType(false, enumObj) << ")value; }\n";

          ss << indentStr << "static " << getCxType(true, enumObj) << ">^ ToCx(" << getCppType(true, enumObj) << "  &value) { if (!value.hasValue()) return nullptr; return ref new Platform::Box< " << getCxType(false, enumObj) << " >(ToCx(value.value())); }\n";
          ss << indentStr << "static " << getCppType(true, enumObj) << " FromCx(" << getCxType(true, enumObj) << " >^ value) { " << getCppType(true, enumObj) << " > result; if (!value) return result; result = FromCx(value->Value); return result; }\n";
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForList(
                                               HelperFile &helperFile,
                                               StructPtr structObj
                                               )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }

            TypePtr foundType = (*found);
            ss << indentStr << "static Windows::Foundation::Collections::IVector< " << getCxType(false, foundType) << " >^ ToCx(shared_ptr< std::list< " << getCppType(false, foundType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::list<" << getCppType(false, foundType) << "> > FromCx(Windows::Foundation::Collections::IVector< " << getCxType(false, foundType) << " >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IVector< " << getCxType(false, foundType) << " >^ Internal::Helper::ToCx(shared_ptr< std::list< " << getCppType(false, foundType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Vector< " << getCxType(false, foundType) << " >();\n";
            cppSS << "  for (auto iter = value->begin(); iter != value.end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Append(ToCx(*iter));\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::list<" << getCppType(false, foundType) << "> > Internal::Helper::FromCx(Windows::Foundation::Collections::IVector< " << getCxType(false, foundType) << " >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return shared_ptr< std::list< " << getCppType(false, foundType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::list< " << getCppType(false, foundType) << " > >();\n";
            cppSS << "  for (" << getCxType(false, foundType) << " value : values)\n";
            cppSS << "  {\n";
            cppSS << "    result->push_back(FromCx(value));\n";
            cppSS << "  }\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForMap(
                                              HelperFile &helperFile,
                                              StructPtr structObj
                                              )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }

            TypePtr keyType = (*found);

            ++found;
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }
            TypePtr valueType = (*found);

            ss << indentStr << "static Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", " << getCxType(false, valueType)  << " >^ ToCx(shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > FromCx(Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", " << getCxType(false, valueType) << " >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", " << getCxType(false, valueType) << " >^ Internal::Helper::ToCx(shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Map< " << getCxType(false, keyType) << ", " << getCxType(false, valueType)  << " >();\n";
            cppSS << "  for (auto iter = value->begin(); iter != value.end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Insert(ToCx((*iter).first), ToCx((*iter).second));\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > Internal::Helper::FromCx(Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", " << getCxType(false, valueType) << " >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << "> >();\n";
            cppSS << "  std::for_each(Windows::Foundation::Collections::begin(values), Windows::Foundation::Collections::end(values), [](Windows::Foundation::Collections::IKeyValuePair< " << getCxType(false, keyType) << ", " << getCxType(false, valueType) << " >^ pair)\n";
            cppSS << "  {\n";
            cppSS << "    result[FromCx(pair->Key)] = FromCx[pair->Value];\n";
            cppSS << "  }\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCx::generateForSet(
                                              HelperFile &helperFile,
                                              StructPtr structObj
                                              )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.mHeaderStructSS;
          auto &cppSS = helperFile.mCppBodySS;
          auto &indentStr = helperFile.mHeaderIndentStr;
          auto dashedStr = GenerateTypesHeader::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }

            TypePtr keyType = (*found);

            ss << indentStr << "static Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", Platform::Object^ >^ ToCx(shared_ptr< std::set< " << getCppType(false, keyType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::set<" << getCppType(false, keyType) << " > > FromCx(Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", Platform::Object^ >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", Platform::Object^ >^ Internal::Helper::ToCx(shared_ptr< std::set< " << getCppType(false, keyType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Map< " << getCxType(false, keyType) << ", Platform::Object^ >();\n";
            cppSS << "  for (auto iter = value->begin(); iter != value.end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Insert(ToCx(*iter), nullptr);\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::set<" << getCppType(false, keyType) << " > > Internal::Helper::FromCx(Windows::Foundation::Collections::IMap< " << getCxType(false, keyType) << ", Platform::Object^ >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return shared_ptr< std::set< " << getCppType(false, keyType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::set<" << getCppType(false, keyType) << "> >();\n";
            cppSS << "  std::for_each(Windows::Foundation::Collections::begin(values), Windows::Foundation::Collections::end(values), [](Windows::Foundation::Collections::IKeyValuePair< " << getCxType(false, keyType) << ", Platform::Object^ >^ pair)\n";
            cppSS << "  {\n";
            cppSS << "    result.insert(FromCx(pair->Key));\n";
            cppSS << "  }\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::getBasicCxTypeString(
                                                      bool isOptional,
                                                      BasicTypePtr type
                                                      )
        {
          if (!type) return String();

          switch (type->mBaseType) {
            case IEventingTypes::PredefinedTypedef_void:      return "void";

            case IEventingTypes::PredefinedTypedef_bool:      return makeCxOptional(isOptional, "Platform::Boolean");

            case IEventingTypes::PredefinedTypedef_uchar:     return makeCxOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_char:
            case IEventingTypes::PredefinedTypedef_schar:     return makeCxOptional(isOptional, "int8");
            case IEventingTypes::PredefinedTypedef_ushort:    return makeCxOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_short:
            case IEventingTypes::PredefinedTypedef_sshort:    return makeCxOptional(isOptional, "int16");
            case IEventingTypes::PredefinedTypedef_uint:      return makeCxOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_int:
            case IEventingTypes::PredefinedTypedef_sint:      return makeCxOptional(isOptional, "int32");
            case IEventingTypes::PredefinedTypedef_ulong:     return makeCxOptional(isOptional, "uint64");
            case IEventingTypes::PredefinedTypedef_long:
            case IEventingTypes::PredefinedTypedef_slong:     return makeCxOptional(isOptional, "int64");
            case IEventingTypes::PredefinedTypedef_ulonglong: return makeCxOptional(isOptional, "uint64");
            case IEventingTypes::PredefinedTypedef_longlong:
            case IEventingTypes::PredefinedTypedef_slonglong: return makeCxOptional(isOptional, "int64");
            case IEventingTypes::PredefinedTypedef_uint8:     return makeCxOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_int8:
            case IEventingTypes::PredefinedTypedef_sint8:     return makeCxOptional(isOptional, "int8");
            case IEventingTypes::PredefinedTypedef_uint16:    return makeCxOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_int16:
            case IEventingTypes::PredefinedTypedef_sint16:    return makeCxOptional(isOptional, "int16");
            case IEventingTypes::PredefinedTypedef_uint32:    return makeCxOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_int32:
            case IEventingTypes::PredefinedTypedef_sint32:    return makeCxOptional(isOptional, "int32");
            case IEventingTypes::PredefinedTypedef_uint64:    return makeCxOptional(isOptional, "uint64");
            case IEventingTypes::PredefinedTypedef_int64:
            case IEventingTypes::PredefinedTypedef_sint64:    return makeCxOptional(isOptional, "int64");

            case IEventingTypes::PredefinedTypedef_byte:      return makeCxOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_word:      return makeCxOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_dword:     return makeCxOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_qword:     return makeCxOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_float:     return makeCxOptional(isOptional, "float32");
            case IEventingTypes::PredefinedTypedef_double:
            case IEventingTypes::PredefinedTypedef_ldouble:   return makeCxOptional(isOptional, "float64");
            case IEventingTypes::PredefinedTypedef_float32:   return makeCxOptional(isOptional, "float32");
            case IEventingTypes::PredefinedTypedef_float64:   return makeCxOptional(isOptional, "float64");

            case IEventingTypes::PredefinedTypedef_pointer:   return makeCxOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_binary:    return "Platform::Array<byte>^";
            case IEventingTypes::PredefinedTypedef_size:      return makeCxOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_string:
            case IEventingTypes::PredefinedTypedef_astring:
            case IEventingTypes::PredefinedTypedef_wstring:   return makeCxOptional(isOptional, "Platform::String^");
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::makeCxOptional(bool isOptional, const String &value)
        {
          if (!isOptional) return value;
          return "Platform::IBox< " + value + " >^";
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::getCppType(
                                            bool isOptional,
                                            TypePtr type
                                            )
        {
          return GenerateStructHeader::getWrapperTypeString(isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructCx::getCxType(bool isOptional, TypePtr type)
        {
          if (!type) return String();

          type = type->getOriginalType();

          {
            auto typedefType = type->toTypedefType();
            if (typedefType) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Typedef failed to resolve to original type: " + typedefType->getPathName());
            }
          }

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return getBasicCxTypeString(isOptional, basicType);
            }
          }


          {
            auto structType = type->toStruct();
            if (structType) {
              if (structType->mGenerics.size() > 0) return String();
              if (structType->hasModifier(Modifier_Special)) {
                String specialName = structType->getPathName();
                if ("::zs::Promise" == specialName) return "Windows::Foundation::IAsyncAction^";
                if ("::zs::exceptions::Exception" == specialName) return "Platform::FailureException^";
                if ("::zs::exceptions::InvalidParameters" == specialName) return "Platform::InvalidArgumentException^";
                if ("::zs::exceptions::InvalidState" == specialName) return "Platform::COMException^";
                if ("::zs::exceptions::NotImplemented" == specialName) return "Platform::NotImplementedException^";
                if ("::zs::exceptions::NotSupported" == specialName) return "Platform::COMException^";
                if ("::zs::exceptions::Unexpected" == specialName) return "Platform::COMException^";
                if ("::zs::Time" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::DateTime");
                if ("::zs::Milliseconds" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Microseconds" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Nanoseconds" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Seconds" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Minutes" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Hours" == specialName) return makeCxOptional(isOptional, "Windows::Foundation::TimeSpan");
              }
              return makeCxOptional(isOptional, fixNamePath(structType) + "^");
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return makeCxOptional(isOptional, fixNamePath(enumType));
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              String templatedTypeStr;
              String specialName;
              String specialTemplatePost;

              {
                auto parent = type->getParent();
                if (parent) {
                  auto parentStruct = parent->toStruct();
                  if (parentStruct) {
                    if (parentStruct->hasModifier(Modifier_Special)) {
                      specialName = parentStruct->getPathName();
                      if ("::std::set" == specialName) {
                        templatedTypeStr = "Windows::Foundation::Collections::IMap< ";
                        specialTemplatePost = ", Platform::Boolean";
                      }
                      if ("::std::list" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IVector< ";
                      if ("::std::map" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IMap< ";
                      if ("::zs::PromiseWith" == specialName) templatedTypeStr = "Windows::Foundation::IAsyncOperation< ";
                    }
                  }
                }
              }

              if (templatedTypeStr.isEmpty()) {
                templatedTypeStr = fixNamePath(templatedType) + "< ";
              }
              bool first = true;
              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter)
              {
                auto templateArg = (*iter);
                String typeStr = getCxType(false, templateArg);
                if ("void" == typeStr) continue;
                if (!first) templatedTypeStr += ", ";
                templatedTypeStr += typeStr;
                first = false;
              }
              templatedTypeStr += specialTemplatePost + " >^";
              return makeCxOptional(isOptional, templatedTypeStr);
            }
          }
          return String();
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
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("generated"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
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

          HelperFile helperFile;
          helperFile.mGlobal = project->mGlobal;

          calculateRelations(project->mGlobal, helperFile.mDerives);

          helperFile.mHeaderFileName = UseHelper::fixRelativeFilePath(pathStr, String("cx_Helpers.h"));
          helperFile.mCppFileName = UseHelper::fixRelativeFilePath(pathStr, String("cx_Helpers.cpp"));

          {
            auto &ss = helperFile.mHeaderIncludeSS;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "#pragma once\n\n";
            ss << "#include \"types.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.mCppIncludeSS;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "#include \"cx_Helpers.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.mCppBodySS;
            GenerateStructHeader::generateUsingTypes(ss, "");
            ss << "\n";
          }

          {
            auto &ss = helperFile.mHeaderStructSS;
            auto &finalSS = helperFile.mHeaderFinalSS;
            auto &indentStr = helperFile.mHeaderIndentStr;

            ss << "namespace Internal {\n";
            indentStr += "  ";
            GenerateStructHeader::generateUsingTypes(ss, indentStr);
            ss << "\n";
            ss << indentStr << "struct Helper {\n";
            ss << indentStr << "internal:\n";
            finalSS << indentStr << "};\n\n";
            finalSS << "} // namespace Internal\n";

            indentStr += "  ";
            generateSpecialHelpers(helperFile);
          }

          generateForNamespace(helperFile, project->mGlobal, String());

          helperFile.mHeaderIncludeSS << "\n";
          helperFile.mHeaderIncludeSS << helperFile.mHeaderStructSS.str();
          helperFile.mHeaderIncludeSS << helperFile.mHeaderFinalSS.str();

          helperFile.mCppIncludeSS << "\n";
          helperFile.mCppIncludeSS << helperFile.mCppBodySS.str();

          writeBinary(helperFile.mHeaderFileName, UseHelper::convertToBuffer(helperFile.mHeaderIncludeSS.str()));
          writeBinary(helperFile.mCppFileName, UseHelper::convertToBuffer(helperFile.mCppIncludeSS.str()));
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
