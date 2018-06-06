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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructCppWinrt.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructMsidl.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateHelper.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateTypesHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>

#include <sstream>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

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
        #pragma mark GenerateStructCppWinrt::HelperFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::HelperFile::HelperFile() :
          structsNeedingInterface_(make_shared<StructSet>())
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::HelperFile::~HelperFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::HelperFile::includeHeader(const String &headerFile)
        {
          auto &ss = headerIncludeSS_;

          if (headerAlreadyIncluded_.end() != headerAlreadyIncluded_.find(headerFile)) return;
          headerAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::HelperFile::includeCpp(const String &headerFile)
        {
          auto &ss = cppIncludeSS_;

          if (cppAlreadyIncluded_.end() != cppAlreadyIncluded_.find(headerFile)) return;
          cppAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructCppWinrt::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::StructFile::StructFile() :
          structsNeedingInterface_(make_shared<StructSet>())
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::StructFile::~StructFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::StructFile::includeCpp(const String &headerFile)
        {
          auto &ss = cppIncludeSS_;

          if (cppAlreadyIncluded_.end() != cppAlreadyIncluded_.find(headerFile)) return;
          cppAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructCppWinrt
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructCppWinrt::GenerateStructCppWinrt() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructCppWinrtPtr GenerateStructCppWinrt::create()
        {
          return make_shared<GenerateStructCppWinrt>();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixName(const String &originalName)
        {
          if (originalName.isEmpty()) return String();
          String firstLetter = originalName.substr(0, 1);
          String remaining = originalName.substr(1);
          firstLetter.toUpper();
          return firstLetter + remaining;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixNamePath(ContextPtr context)
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
        String GenerateStructCppWinrt::fixStructName(StructPtr structObj)
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
        String GenerateStructCppWinrt::fixMethodDeclaration(ContextPtr context)
        {
          String result = fixNamePath(context);
          if ("::" == result.substr(0, 2)) {
            return result.substr(2);
          }
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixMethodDeclaration(StructPtr derivedStruct, ContextPtr context)
        {
          String result = fixMethodDeclaration(derivedStruct);
          if (!context) return result;

          if (result.hasData()) {
            result += "::";
          }
          result += fixName(context->mName);
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixStructFileName(StructPtr structObj)
        {
          auto result = fixNamePath(structObj);
          result.replaceAll("::", ".");
          result.trim(".");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getStructInitName(StructPtr structObj)
        {
          return GenerateStructHeader::getStructInitName(structObj);
        }

        //-------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtStructInitName(StructPtr structObj)
        {
          String namePathStr = fixNamePath(structObj);
          namePathStr.replaceAll("::", "_");
          namePathStr.trim("_");
          return namePathStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixEnumName(EnumTypePtr enumObj)
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
        String GenerateStructCppWinrt::fixArgumentName(const String &originalName)
        {
          if (originalName == "delegate") return "delegateValue";
          if (originalName == "event") return "eventValue";
          return originalName;
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::processTypesNamespace(
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
            GenerateHelper::insertLast(ss, firstStruct);
          }

          indentStr = initialIndentStr;

          if (namespaceObj->mName.hasData()) {
            ss << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::processTypesStruct(
                                                  std::stringstream &ss,
                                                  const String &indentStr,
                                                  StructPtr structObj,
                                                  bool &firstFound
                                                  )
        {
          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          GenerateHelper::insertFirst(ss, firstFound);
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
        bool GenerateStructCppWinrt::processTypesEnum(
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
            ss << GenerateHelper::getDocumentation(indentStr + "/// ", enumObj, 80);
            ss << indentStr << "public enum class " << fixEnumName(enumObj) << "\n";
            ss << indentStr << "{\n";
            for (auto iterValue = enumObj->mValues.begin(); iterValue != enumObj->mValues.end(); ++iterValue)
            {
              auto valueObj = (*iterValue);
              ss << GenerateHelper::getDocumentation(indentStr + "  /// ", valueObj, 80);
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
        SecureByteBlockPtr GenerateStructCppWinrt::generateTypesHeader(ProjectPtr project) throw (Failure)
        {
          std::stringstream ss;

          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          ss << "#pragma once\n\n";
          ss << "#include \"wrapper/generated/types.h\"\n";
          ss << "\n";

          processTypesNamespace(ss, String(), project->mGlobal);

          return UseHelper::convertToBuffer(ss.str());
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::calculateRelations(
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
        void GenerateStructCppWinrt::calculateRelations(
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
        void GenerateStructCppWinrt::insertInto(
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
        void GenerateStructCppWinrt::generateSpecialHelpers(HelperFile &helperFile)
        {
          auto &derives = helperFile.derives_;

          helperFile.includeCpp("<zsLib/SafeInt.h>");
          helperFile.includeCpp("<zsLib/date.h>");
          helperFile.includeCpp("<zsLib/helpers.h>");
          helperFile.includeHeader("<zsLib/eventing/types.h>");
          helperFile.includeHeader("<zsLib/Exception.h>");

          generateBasicTypesHelper(helperFile);

          generateStringHelper(helperFile);
          generateBinaryHelper(helperFile);
          generateExceptionHelper(helperFile);

          if (derives.end() != derives.find("::zs::Days")) {
            generateDurationHelper(helperFile, "Days");
          }
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
            generateDurationHelper(helperFile, "Nanoseconds");
          }
          if (derives.end() != derives.find("::zs::Time")) {
            generateTimeHelper(helperFile);
          }
          if (derives.end() != derives.find("::zs::Any")) {
          }
          if (derives.end() != derives.find("::zs::Promise")) {
            generatePromiseHelper(helperFile);
          }
          if (derives.end() != derives.find("::zs::PromiseWith")) {
            generatePromiseWithHelper(helperFile);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateExceptionHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.headerStructSS_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          auto &derives = helperFile.derives_;
          if (derives.end() != derives.find("::zs::exceptions::Exception")) {
            ss << helperFile.headerIndentStr_ << "static Platform::FailureException^ ToCppWinrt(const ::zsLib::Exception &e) { return ref new Platform::FailureException(ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::InvalidArgument")) {
            ss << helperFile.headerIndentStr_ << "static Platform::InvalidArgumentException^ ToCppWinrt(const ::zsLib::Exceptions::InvalidArgument &e) { return ref new Platform::InvalidArgumentException(ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::BadState")) {
            ss << helperFile.headerIndentStr_ << "static Platform::COMException^ ToCppWinrt(const ::zsLib::Exceptions::BadState &e) { return ref new Platform::COMException(E_NOT_VALID_STATE, ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotImplemented")) {
            ss << helperFile.headerIndentStr_ << "static Platform::NotImplementedException^ ToCppWinrt(const ::zsLib::Exceptions::NotImplemented &e) { return ref new Platform::NotImplementedException(ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotSupported")) {
            ss << helperFile.headerIndentStr_ << "static Platform::COMException^ ToCppWinrt(const ::zsLib::Exceptions::NotSupported &e) { return ref new Platform::COMException(CO_E_NOT_SUPPORTED, ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::UnexpectedError")) {
            ss << helperFile.headerIndentStr_ << "static Platform::COMException^ ToCppWinrt(const ::zsLib::Exceptions::UnexpectedError &e) { return ref new Platform::COMException(E_UNEXPECTED, ToCppWinrt_String(e.message())); }\n";
          }
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateBasicTypesHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          struct WrapperInfo
          {
            String cppwinrtTypeCpp_;
            String cppwinrtTypeHeader_;
            String cppType_;

            WrapperInfo() {}
            WrapperInfo(const WrapperInfo &info) { (*this) = info; }
            WrapperInfo(
                        const String &cppwinrtTypeCpp,
                        const String &cppwinrtTypeHeader,
                        const String &cppType
            ) :
              cppwinrtTypeCpp_(cppwinrtTypeCpp),
              cppwinrtTypeHeader_(cppwinrtTypeHeader),
              cppType_(cppType)
            {
            }
          };
          typedef std::map<String, WrapperInfo> StringMap;

          StringMap basicTypes;

          basicTypes["Boolean"] = WrapperInfo("Platform::Boolean", "Platform::Boolean", "bool");
          basicTypes["Char16"] = WrapperInfo("char16", "char16", "signed char");
          basicTypes["Uint8"] = WrapperInfo("uint8", "uint8", "unsigned char");
          basicTypes["Int16"] = WrapperInfo("int16", "int16", "int16_t");
          basicTypes["Uint16"] = WrapperInfo("uint16", "uint16", "uint16_t");
          basicTypes["Int32"] = WrapperInfo("int32", "int32", "int32_t");
          basicTypes["Uint32"] = WrapperInfo("uint32", "uint32", "uint32_t");
          basicTypes["Int64"] = WrapperInfo("int64", "int64", "int64_t");
          basicTypes["Uint64"] = WrapperInfo("uint64", "uint64", "uint64_t");
          basicTypes["Int64"] = WrapperInfo("int64", "int64", "int64_t");
          basicTypes["Uint64"] = WrapperInfo("uint64", "uint64", "uint64_t");
          basicTypes["HelperLong"] = WrapperInfo("Internal::Helper::HelperLong", "HelperLong", "long");
          basicTypes["HelperULong"] = WrapperInfo("Internal::Helper::HelperULong", "HelperULong", "unsigned long");
          basicTypes["Float32"] = WrapperInfo("float32", "float32", "float");
          basicTypes["HelperFloat"] = WrapperInfo("Internal::Helper::HelperFloat", "HelperFloat", "float");
          basicTypes["Float64"] = WrapperInfo("float64", "float64", "double");

          for (auto iter = basicTypes.begin(); iter != basicTypes.end(); ++iter)
          {
            String wrapperName = (*iter).first;
            String cppwinrtTypeCpp = (*iter).second.cppwinrtTypeCpp_;
            String cppwinrtType = (*iter).second.cppwinrtTypeHeader_;
            String cppType = (*iter).second.cppType_;

            bool safeIntType = true;
            if ((cppwinrtType == "Platform::Boolean") ||
                (cppwinrtType == "float32") ||
                (cppwinrtType == "float64") ||
                (cppwinrtType == "HelperFloat")) {
              safeIntType = false;
            }

            bool isFloat64 = false;
            if (cppwinrtType == "float64") isFloat64 = true;

            ss << indentStr << "static " << cppwinrtType << " ToCppWinrt_" << wrapperName << "(" << cppType << " value);\n";
            if (isFloat64) {
              ss << indentStr << "static " << cppwinrtType << " ToCppWinrt_" << wrapperName << "(long double value);\n";
            }
            ss << indentStr << "static " << cppType << " FromCppWinrt_" << wrapperName << "(" << cppwinrtType << " value);\n";
            ss << indentStr << "static Platform::IBox<" << cppwinrtType << ">^ ToCppWinrt_" << wrapperName << "(const Optional<" << cppType << "> &value);\n";
            if (isFloat64) {
              ss << indentStr << "static Platform::IBox<" << cppwinrtType << ">^ ToCppWinrt_" << wrapperName << "(const Optional<long double> &value);\n";
            }
            ss << indentStr << "static Optional<" << cppType << "> FromCppWinrt_" << wrapperName << "(Platform::IBox<" << cppwinrtType << ">^ value);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << cppwinrtTypeCpp << " Internal::Helper::ToCppWinrt_" << wrapperName << "(" << cppType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  return SafeInt<" << cppwinrtType << ">(value);\n";
            } else {
              cppSS << "  return value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            if (isFloat64) {
              cppSS << dashedStr;
              cppSS << cppwinrtTypeCpp << " Internal::Helper::ToCppWinrt_"<< wrapperName << "(long double value)\n";
              cppSS << "{\n";
              cppSS << "  return value;\n";
              cppSS << "}\n";
              cppSS << "\n";
            }

            cppSS << dashedStr;
            cppSS << cppType << " Internal::Helper::FromCppWinrt_" << wrapperName << "(" << cppwinrtType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  return SafeInt<" << cppType << ">(value);\n";
            } else {
              cppSS << "  return (" << cppType << ")value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "Platform::IBox<" << cppwinrtTypeCpp << ">^ Internal::Helper::ToCppWinrt_" << wrapperName << "(const Optional<" << cppType << "> &value)\n";
            cppSS << "{\n";
            cppSS << "  if (!value.hasValue()) return nullptr;\n";
            if (safeIntType) {
              cppSS << "  return ref new Platform::Box<" << cppwinrtType << ">(SafeInt<" << cppwinrtType << ">(value.value()));\n";
            }
            else {
              cppSS << "  return ref new Platform::Box<" << cppwinrtType << ">(value.value());\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            if (isFloat64) {
              cppSS << dashedStr;
              cppSS << "Platform::IBox<" << cppwinrtTypeCpp << ">^ Internal::Helper::ToCppWinrt_" << wrapperName << "(const Optional<long double> &value)\n";
              cppSS << "{\n";
              cppSS << "  if (!value.hasValue()) return nullptr;\n";
              if (safeIntType) {
                cppSS << "  return ref new Platform::Box<" << cppwinrtType << ">(SafeInt<" << cppwinrtType << ">(value.value()));\n";
              } else {
                cppSS << "  return ref new Platform::Box<" << cppwinrtType << ">(value.value());\n";
              }
              cppSS << "}\n";
              cppSS << "\n";
            }

            cppSS << dashedStr;
            cppSS << "Optional<" << cppType << "> Internal::Helper::FromCppWinrt_" << wrapperName << "(Platform::IBox<" << cppwinrtType << ">^ value)\n";
            cppSS << "{\n";
            cppSS << "  Optional<" << cppType << "> result;\n";
            cppSS << "  if (nullptr == value) return result;\n";
            if (safeIntType) {
              cppSS << "  result = SafeInt<" << cppType << ">(value->Value);\n";
            } else {
              cppSS << "  result = (" << cppType << ")value->Value;\n";
            }
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateBinaryHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Platform::Array<byte>^ ToCppWinrt_Binary(const SecureByteBlock &value);\n";
          ss << indentStr << "static Platform::Array<byte>^ ToCppWinrt_Binary(SecureByteBlockPtr value);\n";
          ss << indentStr << "static Platform::Array<byte>^ ToCppWinrt_Binary(const Optional<SecureByteBlockPtr> &value);\n";
          ss << indentStr << "static SecureByteBlockPtr FromCppWinrt_Binary(const Platform::Array<byte>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCppWinrt_Binary(const SecureByteBlock &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.BytePtr()) return nullptr;\n";
          cppSS << "  return ref new Platform::Array<byte>((unsigned char *)value.BytePtr(), value.SizeInBytes());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCppWinrt_Binary(SecureByteBlockPtr value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return nullptr;\n";
          cppSS << "  return ToCppWinrt_Binary(*value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::Array<byte>^ Internal::Helper::ToCppWinrt_Binary(const Optional<SecureByteBlockPtr> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  if (!value.value()) return nullptr;\n";
          cppSS << "  return ToCppWinrt_Binary(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "SecureByteBlockPtr Internal::Helper::FromCppWinrt_Binary(const Platform::Array<byte>^ value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return SecureByteBlockPtr();\n";
          cppSS << "  return make_shared<SecureByteBlock>(value->Data, value->Length);\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateStringHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Platform::String^ ToCppWinrt_String(const std::string &value);\n";
          ss << indentStr << "static Platform::String^ ToCppWinrt_String(const Optional<std::string> &value);\n";
          ss << indentStr << "static Platform::String^ ToCppWinrt_String(const Optional<String> &value);\n";
          ss << indentStr << "static Platform::String^ ToCppWinrt_String(const std::wstring &value);\n";
          ss << indentStr << "static Platform::String^ ToCppWinrt_String(const Optional<std::wstring> &value);\n";
          ss << indentStr << "static String FromCppWinrt_String(Platform::String^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCppWinrt_String(const std::string &value)\n";
          cppSS << "{\n";
          cppSS << "  return ref new Platform::String(String(value).wstring().c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCppWinrt_String(const Optional< std::string > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ToCppWinrt_String(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCppWinrt_String(const Optional< String > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ToCppWinrt_String(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCppWinrt_String(const std::wstring &value)\n";
          cppSS << "{\n";
          cppSS << "  return ref new Platform::String(value.c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::String^ Internal::Helper::ToCppWinrt_String(const Optional< std::wstring > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ToCppWinrt_String(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "String Internal::Helper::FromCppWinrt_String(Platform::String^ value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return String();\n";
          cppSS << "  auto dataStr = value->Data();\n";
          cppSS << "  if (!dataStr) return String();\n";
          cppSS << "  return String(std::wstring(dataStr));\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateDurationHelper(
                                                      HelperFile &helperFile,
                                                      const String &durationType
                                                      )
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          bool isNanoseconds = "Nanoseconds" == durationType;

          ss << indentStr << "static Windows::Foundation::TimeSpan ToCppWinrt_" << durationType << "(const ::zsLib::" << durationType << " &value);\n";
          ss << indentStr << "static Platform::IBox<Windows::Foundation::TimeSpan>^ ToCppWinrt_" << durationType << "(const Optional<::zsLib::" << durationType << "> &value);\n";
          ss << indentStr << "static ::zsLib::" << durationType << " FromCppWinrt_" << durationType << "(Windows::Foundation::TimeSpan value);\n";
          ss << indentStr << "static Optional<::zsLib::" << durationType << "> FromCppWinrt_" << durationType << "(Platform::IBox<Windows::Foundation::TimeSpan>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::TimeSpan Internal::Helper::ToCppWinrt_" << durationType << "(const ::zsLib::" << durationType << " &value)\n";
          cppSS << "{\n";
          cppSS << "  Windows::Foundation::TimeSpan result {};\n";
          cppSS << "  result.Duration = " << (isNanoseconds ? "value.count()" : "::zsLib::toNanoseconds(value).count()") << " / static_cast<::zsLib::Nanoseconds::rep>(100);\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Platform::IBox<Windows::Foundation::TimeSpan>^ Internal::Helper::ToCppWinrt_" << durationType << "(const Optional<::zsLib::" << durationType << "> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Windows::Foundation::TimeSpan>(ToCppWinrt_" << durationType << "(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "::zsLib::" << durationType << " Internal::Helper::FromCppWinrt_" << durationType << "(Windows::Foundation::TimeSpan value)\n";
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
          cppSS << "Optional<::zsLib::" << durationType << "> Internal::Helper::FromCppWinrt_" << durationType << "(Platform::IBox<Windows::Foundation::TimeSpan>^ value)\n";
          cppSS << "{\n";
          cppSS << "  Optional<::zsLib::" << durationType << "> result;\n";
          cppSS << "  if (!value) return result;\n";
          cppSS << "  result = FromCppWinrt_" << durationType << "(value->Value);\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateTimeHelper(HelperFile &helperFile)
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::DateTime ToCppWinrt(const ::zsLib::Time &value);\n";
          ss << indentStr << "static Platform::IBox<Windows::Foundation::DateTime>^ ToCppWinrt(const Optional<::zsLib::Time> &value);\n";
          ss << indentStr << "static ::zsLib::Time FromCppWinrt(Windows::Foundation::DateTime value);\n";
          ss << indentStr << "static Optional<::zsLib::Time> FromCppWinrt(Platform::IBox<Windows::Foundation::DateTime>^ value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::DateTime Internal::Helper::ToCppWinrt(const ::zsLib::Time &value)\n";
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
          cppSS << "Platform::IBox<Windows::Foundation::DateTime>^ Internal::Helper::ToCppWinrt(const Optional<::zsLib::Time> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box<Windows::Foundation::DateTime>(ToCppWinrt(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "::zsLib::Time Internal::Helper::FromCppWinrt(Windows::Foundation::DateTime value)\n";
          cppSS << "{\n";
          cppSS << "  ::zsLib::Time t = day_point(jan / 1 / 1601);\n";
          cppSS << "\n";
          cppSS << "  auto nano = std::chrono::duration_cast<::zsLib::Time::duration>(zsLib::Nanoseconds(static_cast<::zsLib::Nanoseconds::rep>(value.UniversalTime) * static_cast<::zsLib::Nanoseconds::rep>(100)));\n";
          cppSS << "\n";
          cppSS << "  return t + nano;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generatePromiseHelper(HelperFile &helperFile)
        {
          helperFile.includeHeader("<ppltasks.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::IAsyncAction^ ToCppWinrt(::zsLib::PromisePtr promise);\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::IAsyncAction^ Internal::Helper::ToCppWinrt(::zsLib::PromisePtr promise)\n";
          cppSS << "{\n";
          cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
          cppSS << "  {\n";
          cppSS << "    Observer(Concurrency::task_completion_event<void> tce) : tce_(tce) {}\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override { tce_.set(); }\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override\n";
          cppSS << "    {\n";
          cppSS << "      if (!promise) {\n";
          cppSS << "        tce_.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
          cppSS << "        return;\n";
          cppSS << "      }\n";

          generateDefaultPromiseRejections(helperFile, "      ");

          cppSS << "      tce_.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
          cppSS << "    }\n";
          cppSS << "\n";
          cppSS << "  private:\n";
          cppSS << "    Concurrency::task_completion_event<void> tce_;\n";
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
          cppSS << "    auto tceTask = Concurrency::task<void>(tce);";
          cppSS << "    return tceTask.get();\n";
          cppSS << "  });\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generatePromiseWithHelper(HelperFile &helperFile)
        {
          helperFile.includeHeader("<ppltasks.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          auto foundType = helperFile.global_->toContext()->findType("::zs::PromiseWith");
          if (foundType) {
            auto promiseWithStruct = foundType->toStruct();
            if (promiseWithStruct) {
              for (auto iterPromise = promiseWithStruct->mTemplatedStructs.begin(); iterPromise != promiseWithStruct->mTemplatedStructs.end(); ++iterPromise)
              {
                auto templatedStruct = (*iterPromise).second;
                if (!templatedStruct) continue;

                auto foundArgType = templatedStruct->mTemplateArguments.begin();
                if (foundArgType == templatedStruct->mTemplateArguments.end()) continue;

                auto resolveType = (*foundArgType);
                if (!resolveType) continue;

                TypePtr rejectType;
                {
                  (++foundArgType);
                  if (foundArgType != templatedStruct->mTemplateArguments.end()) rejectType = *foundArgType;
                }

                String promiseWithStr = "PromiseWithHolderPtr";
                if (resolveType->toBasicType()) {
                  promiseWithStr = "PromiseWithHolder";
                }

                ss << indentStr << "static " << getCppWinrtType(false, templatedStruct, true) << " " << getToCppWinrtName(templatedStruct) << "(shared_ptr< " << promiseWithStr << "< " << getCppType(false, resolveType) << " > > promise);\n";

                cppSS << dashedStr;
                cppSS << getCppWinrtType(false, templatedStruct, true) << " Internal::Helper::" << getToCppWinrtName(templatedStruct) << "(shared_ptr< " << promiseWithStr << "< " << getCppType(false, resolveType) << " > > promise)\n";
                cppSS << "{\n";
                cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
                cppSS << "  {\n";
                cppSS << "    Observer(Concurrency::task_completion_event< " << getCppWinrtType(false, resolveType) << " > tce) : tce_(tce) {}\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override\n";
                cppSS << "    {\n";
                cppSS << "      if (!promise) {\n";
                cppSS << "        tce_.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
                cppSS << "        return;\n";
                cppSS << "      }\n";
                cppSS << "      {\n";
                cppSS << "        auto reasonHolder = promise->value< ::zsLib::AnyHolder< " << getCppType(false, resolveType) << " > >();\n";
                cppSS << "        if (reasonHolder) {\n";
                cppSS << "          tce_.set(::Internal::Helper::" << getToCppWinrtName(resolveType) << "(reasonHolder->value_));\n";
                cppSS << "        }\n";
                cppSS << "      }\n";
                cppSS << "      tce_.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
                cppSS << "    }\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override\n";
                cppSS << "    {\n";
                cppSS << "      if (!promise) {\n";
                cppSS << "        tce_.set_exception(ref new Platform::Exception(E_INVALIDARG));\n";
                cppSS << "        return;\n";
                cppSS << "      }\n";

                if (rejectType) {
                  auto rejectTypeStr = rejectType->getPathName();
                  if ("::void" != rejectTypeStr) {
                    generatePromiseRejection(helperFile, indentStr, rejectType);
                  }
                }
                generateDefaultPromiseRejections(helperFile, "      ");

                cppSS << "      tce_.set_exception(ref new Platform::Exception(E_NOINTERFACE));\n";
                cppSS << "    }\n";
                cppSS << "\n";
                cppSS << "  private:\n";
                cppSS << "    Concurrency::task_completion_event< " << getCppWinrtType(false, resolveType) << " > tce_;\n";
                cppSS << "  };\n";
                cppSS << "\n";
                cppSS << "  " << getCppWinrtType(false, templatedStruct) << " result = Concurrency::create_async([promise]()\n";
                cppSS << "  {\n";
                cppSS << "    Concurrency::task_completion_event< " << getCppWinrtType(false, resolveType) << " > tce;\n";
                cppSS << "\n";
                cppSS << "    auto observer = make_shared<Observer>(tce);\n";
                cppSS << "    promise->then(observer);\n";
                cppSS << "    promise->background();\n";
                cppSS << "\n";
                cppSS << "    auto tceTask = Concurrency::task< " << getCppWinrtType(false, resolveType) << " >(tce);\n";
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
        void GenerateStructCppWinrt::generateDefaultPromiseRejections(
                                                                HelperFile &helperFile,
                                                                const String &indentStr
                                                                )
        {
          auto foundType = helperFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (foundType) {
            auto rejectionStruct = foundType->toStruct();
            if (rejectionStruct) {
              for (auto iterRej = rejectionStruct->mTemplatedStructs.begin(); iterRej != rejectionStruct->mTemplatedStructs.end(); ++iterRej)
              {
                auto templatedStruct = (*iterRej).second;
                if (!templatedStruct) continue;

                auto foundArgType = templatedStruct->mTemplateArguments.begin();
                if (foundArgType == templatedStruct->mTemplateArguments.end()) continue;

                auto rejectionType = (*foundArgType);
                if (!rejectionType) continue;
                generatePromiseRejection(helperFile, indentStr, rejectionType);
              }
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generatePromiseRejection(
                                                        HelperFile &helperFile,
                                                        const String &indentStr,
                                                        TypePtr rejectionType
                                                        )
        {
          if (!rejectionType) return;
          auto &cppSS = helperFile.cppBodySS_;
          cppSS << indentStr << "{\n";
          cppSS << indentStr << "  auto reasonHolder = promise->reason< ::zsLib::AnyHolder< " << getCppType(false, rejectionType) << " > >();\n";
          cppSS << indentStr << "  if (reasonHolder) {\n";
          cppSS << indentStr << "    tce_.set_exception(::Internal::Helper::" << getToCppWinrtName(rejectionType) << "(reasonHolder->value_));\n";
          cppSS << indentStr << "  }\n";
          cppSS << indentStr << "}\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForNamespace(
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
        void GenerateStructCppWinrt::generateForStruct(
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
              (!GenerateHelper::isBuiltInType(structObj))) {
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
        void GenerateStructCppWinrt::generateForStandardStruct(
                                                         HelperFile &helperFile,
                                                         StructPtr structObj
                                                         )
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          helperFile.includeCpp(String("\"") + fixStructFileName(structObj) + ".h\"");

          ss << indentStr << "static " << getCppWinrtType(false, structObj, true) << " " << getToCppWinrtName(structObj) << "(" << getCppType(false, structObj) << " value);\n";
          ss << indentStr << "static " << getCppType(false, structObj) << " " << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(false, structObj) << " value);\n";

          ss << indentStr << "static " << getCppWinrtType(false, structObj, true) << " " << getToCppWinrtName(structObj) << "(const " << getCppType(true, structObj) << "  &value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(false, structObj, true) << " Internal::Helper::" << getToCppWinrtName(structObj) << "(" << getCppType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << fixNamePath(structObj) << "::ToCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(false, structObj) << " Internal::Helper::" << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << fixNamePath(structObj) << "::FromCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(false, structObj, true) << " Internal::Helper::" << getToCppWinrtName(structObj) << "(const " << getCppType(true, structObj) << " &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return " << getToCppWinrtName(structObj) << "(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          generateStructFile(helperFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateStructFile(
                                                  HelperFile &helperFile,
                                                  StructPtr structObj
                                                  )
        {
          if (!structObj) return;

          auto dashedStr = GenerateHelper::getDashedComment(String());

          String filename = fixStructFileName(structObj);

          StructFile structFile;
          structFile.struct_ = structObj;
          structFile.structsNeedingInterface_ = helperFile.structsNeedingInterface_;

          auto &includeSS = structFile.headerIncludeSS_;
          auto &prestructDelegateSS = structFile.mHeaderPreStructSS;
          auto &ss = structFile.mHeaderStructPrivateSS;
          auto &pubSS = structFile.mHeaderStructPublicSS;
          auto &cppSS = structFile.cppBodySS_;
          auto &cppIncludeSS = structFile.cppIncludeSS_;
          auto &observerSS = structFile.mHeaderStructObserverSS;
          auto &observerFinalSS = structFile.mHeaderStructObserverFinalSS;

          auto &indentStr = structFile.headerStructIndentStr_;

          includeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          includeSS << "#pragma once\n\n";
          includeSS << "#include \"types.h\"\n";

          String ifdefName = (structObj->hasModifier(Modifier_Special) ? "CPPWINRT_USE_GENERATED_" : "CPPWINRT_USE_CUSTOM_") + getCppWinrtStructInitName(structObj);
          ifdefName.toUpper();

          includeSS << "\n";
          includeSS << "#" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "ifdef") << " " << ifdefName << "\n";
          includeSS << "#include <wrapper/cppwinrt/" << filename << ".h>\n";
          includeSS << "#else // " << ifdefName << "\n";

          cppIncludeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";

          cppIncludeSS << "\n";
          cppIncludeSS << "#" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "ifdef") << " " << ifdefName << "\n";
          cppIncludeSS << "#include \"" << filename << ".h\"\n";
          cppIncludeSS << "#else // " << ifdefName << "\n";

          structFile.includeCpp("\"cppwinrt_Helpers.h\"");
          structFile.includeCpp("\"" + filename + ".h\"");

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
            includeSS << "#include <wrapper/generated/" << GenerateStructHeader::getStructFileName(parentStruct) << ">\n";
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

          bool hasEvents = false;
          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Method_EventHandler)) hasEvents = true;
          }

          ss << "\n";

          bool foundWebHidden = false;

          if (structObj->hasModifier(Modifier_Platform)) {
            StringList values;
            structObj->getModifierValues(Modifier_Platform, values);
            for (auto iter = values.begin(); iter != values.end(); ++iter) {
              auto &value = (*iter);
              if ("webhidden" == value) foundWebHidden = true;
            }
          }

          ss << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);

          StringList attributes;
          if (foundWebHidden) {
            attributes.push_back("Windows::Foundation::Metadata::WebHostHiddenAttribute");
          }
          if (structObj->hasModifier(Modifier_Obsolete)) {
            attributes.push_back("Windows::Foundation::Metadata::Deprecated(" + structObj->getModifierValue(Modifier_Obsolete, 0) + ", Windows::Foundation::Metadata::DeprecationType::Deprecate, 0)");
          }

          ss << getCppWinrtAttributesLine(indentStr, attributes);

          ss << indentStr << "public ref " << (structObj->hasModifier(Modifier_Struct_Dictionary) ? "struct" : "class") << " " << fixStructName(structObj) << " sealed\n";
          ss << indentStr << "{\n";
          ss << indentStr << "internal:\n";
          pubSS << indentStr << "public:\n";
          endingStrs.push_front(indentStr + "};\n");

          structFile.headerIndentStr_ = indentStr;

          indentStr += "  ";

          ss << indentStr << getCppType(false, structObj) << " native_;\n";
          if (hasEvents) {
            ss << indentStr << "wrapper" << structObj->getPathName() << "::WrapperObserverPtr observer_;\n";
          }
          ss << "\n";
          ss << indentStr << "struct WrapperCreate {};\n";
          ss << indentStr << fixStructName(structObj) << "(const WrapperCreate &) {}\n";
          ss << "\n";
          ss << indentStr << "static " << fixStructName(structObj) << "^ ToCppWinrt(" << getCppType(false, structObj) << " value);\n";
          ss << indentStr << "static " << getCppType(false, structObj) << " FromCppWinrt(" << fixStructName(structObj) << "^ value);\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(false, structObj, true) << " " << fixMethodDeclaration(structObj) << "::ToCppWinrt(" << getCppType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return nullptr;\n";
          cppSS << "  auto result = ref new " << fixStructName(structObj) << "(WrapperCreate{});\n";
          cppSS << "  result->native_ = value;\n";
          if (hasEvents) {
            cppSS << "  result->observer_ = make_shared<WrapperObserverImpl>(result);\n";
            cppSS << "  result->native_->wrapper_installObserver(result->observer_);\n";
          }
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(false, structObj) << " " << fixMethodDeclaration(structObj) << "::FromCppWinrt(" << getCppWinrtType(false, structObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (nullptr == value) return " << getCppType(false, structObj) << "();\n";
          cppSS << "  return value->native_;\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (GenerateHelper::needsDefaultConstructor(structObj)) {
            pubSS << indentStr << fixStructName(structObj) << "();\n";

            cppSS << dashedStr;
            cppSS << fixMethodDeclaration(structObj) << "::" << fixStructName(structObj) << "()\n";
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
            cppSS << "  native_->wrapper_init_" << getStructInitName(structObj) << "();\n";
            cppSS << "}\n";
            cppSS << "\n";
          }

          if (hasEvents) {
            observerSS << indentStr << "struct WrapperObserverImpl : public wrapper" << structObj->getPathName() << "::WrapperObserver \n";
            observerSS << indentStr << "{\n";
            observerSS << indentStr << "  WrapperObserverImpl(" << fixStructName(structObj) << "^ owner) : owner_(owner) {}\n";
            observerSS << "\n";
            observerFinalSS << indentStr << "  " << fixStructName(structObj) << "^ owner_;\n";
            observerFinalSS << indentStr << "};\n";
          }

          {
            bool foundCast = false;
            auto found = helperFile.derives_.find(structObj->getPathName());
            if (found != helperFile.derives_.end()) {
              auto &structSet = (*found).second;
              for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet)
              {
                auto foundStruct = (*iterSet);
                if (foundStruct != structObj) {
                  includeCppForType(structFile, foundStruct);

                  pubSS << indentStr << "/// <summary>\n";
                  pubSS << indentStr << "/// Cast from " << fixStructName(foundStruct) << " to " << fixStructName(structObj) << "\n";
                  pubSS << indentStr << "/// </summary>\n";
                  pubSS << indentStr << "[";
                  if (!foundCast) {
                    pubSS << "Windows::Foundation::Metadata::DefaultOverloadAttribute, ";
                  }
                  pubSS << "Windows::Foundation::Metadata::OverloadAttribute(\"CastAs" << fixStructName(foundStruct);
                  pubSS << "\")]\n";
                  pubSS << indentStr << "static " << fixStructName(structObj) << "^ Cast(" << getCppWinrtType(false, foundStruct) << " value);\n";
                  foundCast = true;

                  cppSS << dashedStr;
                  cppSS << getCppWinrtType(false, structObj, true) << " " << fixNamePath(structObj) << "::Cast(" << getCppWinrtType(false, foundStruct) << " value)\n";
                  cppSS << "{\n";
                  cppSS << "  if (nullptr == value) return nullptr;\n";
                  cppSS << "  auto result = std::dynamic_pointer_cast< wrapper" << structObj->getPathName() << " >(value->native_);\n";
                  cppSS << "  if (!result) return nullptr;\n";
                  cppSS << "  return ToCppWinrt(result);\n";
                  cppSS << "}\n";
                  cppSS << "\n";
                }
              }
            }
            if (foundCast) pubSS << "\n";
          }

          generateStructMethods(helperFile, structFile, structObj, structObj, true, hasEvents);

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

          includeSS << "#endif //" << (structObj->hasModifier(Modifier_Special) ? "ifdef" : "ifndef") << " " << ifdefName << "\n";


          cppIncludeSS << "\n";
          cppIncludeSS << cppSS.str();
          cppIncludeSS << "\n";
          cppIncludeSS << "#endif //" << (structObj->hasModifier(Modifier_Special) ? "ifdef" : "ifndef") << " " << ifdefName << "\n";

          String outputnameHeader = UseHelper::fixRelativeFilePath(helperFile.headerFileName_, filename + ".h");
          String outputnameCpp = UseHelper::fixRelativeFilePath(helperFile.headerFileName_, filename + ".cpp");
          writeBinary(outputnameHeader, UseHelper::convertToBuffer(includeSS.str()));
          writeBinary(outputnameCpp, UseHelper::convertToBuffer(cppIncludeSS.str()));
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateStructMethods(
                                                     HelperFile &helperFile,
                                                     StructFile &structFile,
                                                     StructPtr derivedStructObj,
                                                     StructPtr structObj,
                                                     bool createConstructors,
                                                     bool hasEvents
                                                     )
        {
          if (!structObj) return;

          auto dashedStr = GenerateHelper::getDashedComment(String());

          auto &headerMethodsSS = structFile.mHeaderStructPublicSS;
          auto &prestructDelegateSS = structFile.mHeaderPreStructSS;
          auto &observerSS = structFile.mHeaderStructObserverSS;
          auto &cppSS = structFile.cppBodySS_;
          auto &indentStr = structFile.headerStructIndentStr_;

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedType = (*iter).second;
            if (!relatedType) continue;

            {
              auto subStructObj = relatedType->toStruct();
              if (subStructObj) {
                generateStructMethods(helperFile, structFile, derivedStructObj, subStructObj, false, hasEvents);
              }
            }
          }

          bool firstOutput = true;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);

            if (method->hasModifier(Modifier_Method_Delete)) continue;

            bool isCtor = method->hasModifier(Modifier_Method_Ctor);
            bool isEvent = method->hasModifier(Modifier_Method_EventHandler);
            bool isToString = ("ToString" == fixName(method->mName)) && ("::string" == method->mResult->getPathName()) && (0 == method->mArguments.size());
            bool isStatic = method->hasModifier(Modifier_Static);

            std::stringstream implSS;

            if (!createConstructors) {
              if ((isCtor) || (isEvent)) continue;
            }

            if (firstOutput) {
              headerMethodsSS << indentStr << "// " << structObj->getPathName() << "\n\n";
              firstOutput = false;
            }

            bool hasResult = ("::void" != method->mResult->getPathName());
            if ((isCtor) || (isEvent)) hasResult = false;

            String methodImplIndentStr = "  ";

            includeCppForType(structFile, method->mResult);

            if (hasResult) {
              implSS << methodImplIndentStr << getCppWinrtType(method->hasModifier(Modifier_Optional), method->mResult, true) << " result {};\n";
            }
            if (method->mThrows.size() > 0) {
              implSS << methodImplIndentStr << "try {\n";
              methodImplIndentStr += "  ";
            }
            if (hasResult) {
              implSS << methodImplIndentStr << "result = ::Internal::Helper::" << getToCppWinrtName(method->mResult) << "(";
            } else {
              implSS << methodImplIndentStr;
            }

            if (isEvent) {
              implSS << "owner_->" << fixName(method->mName) << "(";
            } else {
              if (isCtor) {
                implSS << "native_->wrapper_init_" << getStructInitName(structObj) << "(";
              } else {
                if (isStatic) {
                  implSS << "wrapper" << method->getPathName() << "(";
                } else {
                  implSS << "native_->" << method->mName << "(";
                }
              }
            }

            StringList attributes;
            if (method->hasModifier(Modifier_Method_Default)) {
              attributes.push_back("Windows::Foundation::Metadata::DefaultOverloadAttribute");
            }
            String altName = method->getModifierValue(Modifier_AltName, 0);
            if (altName.hasData()) {
              attributes.push_back("Windows::Foundation::Metadata::OverloadAttribute(\"" + fixName(altName) + "\")");
            }
            if (method->hasModifier(Modifier_Obsolete)) {
              attributes.push_back("Windows::Foundation::Metadata::Deprecated(" + method->getModifierValue(Modifier_Obsolete, 0) + ", Windows::Foundation::Metadata::DeprecationType::Deprecate, 0)");
            }

            headerMethodsSS << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);
            for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
            {
              auto arg = (*iterArgs);
              headerMethodsSS << GenerateHelper::getDocumentation(indentStr + "/// ", arg, 80);
            }

            headerMethodsSS << getCppWinrtAttributesLine(indentStr, attributes);

            if (isEvent) {
              hasResult = false;
              observerSS << indentStr << "  virtual void " << method->mName << "(";

              cppSS << dashedStr;
              cppSS << "void " << fixNamePath(structObj) << "::WrapperObserverImpl::" << method->mName << "(";

              String delegateNameStr = fixName(method->getModifierValue(Modifier_Method_EventHandler, 0));
              if (!delegateNameStr.hasData()) {
                delegateNameStr = fixStructName(structObj) + "_";
                if (method->hasModifier(Modifier_AltName)) {
                  delegateNameStr += fixName(method->getModifierValue(Modifier_AltName, 0));
                } else {
                  delegateNameStr += fixName(method->mName);
                }
                delegateNameStr += "Delegate";
              }
              prestructDelegateSS << structFile.headerIndentStr_ << "public delegate void " << delegateNameStr << "(";
              if (method->mArguments.size() > 1) {
                observerSS << "\n";
                observerSS << indentStr << "  ";
                cppSS << "\n";
                cppSS << "  ";
                prestructDelegateSS << "\n";
                prestructDelegateSS << structFile.headerIndentStr_ << "  ";
              }

              headerMethodsSS << indentStr << "event " << delegateNameStr << "^ " << fixName(method->mName) << ";\n";
            } else {
              cppSS << dashedStr;

              headerMethodsSS << indentStr << (method->hasModifier(Modifier_Static) ? "static " : "");
              if (!isCtor) {
                includeCppForType(structFile, method->mResult);

                cppSS << getCppWinrtType(method->hasModifier(Modifier_Optional), method->mResult, true) << " ";
                headerMethodsSS << (isToString ? "virtual " : "") << getCppWinrtType(method->hasModifier(Modifier_Optional), method->mResult, true) << " ";
              }
              cppSS << fixMethodDeclaration(derivedStructObj, method) << "(";
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
                cppSS << ",\n";
                cppSS << "  ";
                if (isEvent) {
                  observerSS << ",\n";
                  observerSS << indentStr << "  ";
                  prestructDelegateSS << ",\n";
                  prestructDelegateSS << structFile.headerIndentStr_ << "  ";
                } else {
                  headerMethodsSS << ",\n";
                  headerMethodsSS << indentStr << "  ";
                }
              }
              firstArg = false;
              implSS << "::Internal::Helper::" << (isEvent ? getToCppWinrtName(arg->mType) : getFromCppWinrtName(arg->mType)) << "(" << fixArgumentName(arg->mName) << ")";
              includeCppForType(structFile, arg->mType);
              if (isEvent) {
                cppSS << getCppType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                observerSS << getCppType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                prestructDelegateSS << getCppWinrtType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
              } else {
                cppSS << getCppWinrtType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
                headerMethodsSS << getCppWinrtType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->mName);
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
              implSS << "    throw ::Internal::Helper::" << getToCppWinrtName(throwType) << "(e);\n";
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
                prestructDelegateSS << structFile.headerIndentStr_ << "  ";
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
              headerMethodsSS << ")" << (isToString ? " override" : "") << ";\n";
            }
            cppSS << ")";
            if (isCtor) {
              cppSS << "\n : native_(" << "wrapper" << structObj->getPathName() << "::wrapper_create()" << ")";
              if (hasEvents) {
                cppSS << ",\n";
                cppSS << "   observer_(make_shared<WrapperObserverImpl>(this))";
              }
            }

            cppSS << "\n";
            cppSS << "{\n";
            if (!isStatic) {
              cppSS << "  if (" << (isEvent ? "nullptr == owner_" : "!native_") << ") {throw ref new Platform::NullReferenceException();}\n";
            }
            if ((isCtor) && (hasEvents)) {
              cppSS << "  native_->wrapper_installObserver(observer_);\n";
            }
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
            bool isStatic = property->hasModifier(Modifier_Static);

            if ((!structObj->hasModifier(Modifier_Struct_Dictionary)) || (isStatic))
            {
              if ((!hasGetter) && (!hasSetter)) hasGetter = hasSetter = true;
            }

            if (hasGetter) {
              if (!hasSetter) hasSet = false;
            } else {
              if (hasSetter) hasGet = false;
            }

            includeCppForType(structFile, property->mType);

            headerMethodsSS << GenerateHelper::getDocumentation(indentStr + "/// ", property, 80);

            StringList attributes;
            if (property->hasModifier(Modifier_Obsolete)) {
              attributes.push_back("Windows::Foundation::Metadata::Deprecated(" + property->getModifierValue(Modifier_Obsolete, 0) + ", Windows::Foundation::Metadata::DeprecationType::Deprecate, 0)");
            }

            headerMethodsSS << getCppWinrtAttributesLine(indentStr, attributes);

            headerMethodsSS << indentStr << (isStatic ? "static " : "") << "property " << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType, true) << " " << fixName(property->mName);
            if (hasGet) {
              if (hasSet) {
                headerMethodsSS << "\n";
                headerMethodsSS << indentStr << "{\n";
                headerMethodsSS << indentStr << "  " << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType) << " get();\n";
                headerMethodsSS << indentStr << "  void set(" << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType) << " value);\n";
                headerMethodsSS << indentStr << "}\n";
              } else {
                headerMethodsSS << " { " << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType, true) << " get(); }\n";
              }
            } else {
              if (hasSet) {
                headerMethodsSS << " { void set(" << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType) << " value); }\n";
              }
            }

            if (hasGet) {
              cppSS << dashedStr;
              cppSS << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType, true) << " " << fixMethodDeclaration(derivedStructObj, property) << "::get()\n";
              cppSS << "{\n";
              if (!isStatic) {
                cppSS << "  if (!native_) {throw ref new Platform::NullReferenceException();}\n";
              }
              cppSS << "  return ::Internal::Helper::" << getToCppWinrtName(property->mType) << "(" << (isStatic ? String("wrapper" + structObj->getPathName() + "::") : String("native_->"));
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
              cppSS << "void " << fixMethodDeclaration(derivedStructObj, property) << "::set(" << getCppWinrtType(property->hasModifier(Modifier_Optional), property->mType) << " value)\n";
              cppSS << "{\n";
              if (!isStatic) {
                cppSS << "  if (!native_) {throw ref new Platform::NullReferenceException();}\n";
                cppSS << "  native_->";
              } else {
                cppSS << "  wrapper" << structObj->getPathName() << "::";
              }

              if (hasSetter) {
                cppSS << "set_" << property->mName << "(::Internal::Helper::" << getFromCppWinrtName(property->mType) << "(value));";
              } else {
                cppSS << property->mName << " = " << "::Internal::Helper::" << getFromCppWinrtName(property->mType) << "(value);";
              }
              cppSS << "\n";
              cppSS << "}\n";
              cppSS << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForEnum(
                                               HelperFile &helperFile,
                                               EnumTypePtr enumObj
                                               )
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static " << getCppWinrtType(false, enumObj, true) << " " << getToCppWinrtName(enumObj) << "(" << getCppType(false, enumObj) << " value) { return (" << getCppWinrtType(false, enumObj) << ")value; }\n";
          ss << indentStr << "static " << getCppType(false, enumObj) << " " << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(false, enumObj) << " value) { return (" << getCppType(false, enumObj) << ")value; }\n";

          ss << indentStr << "static " << getCppWinrtType(true, enumObj, true) << " " << getToCppWinrtName(enumObj) << "(" << getCppType(true, enumObj) << "  &value);\n";
          ss << indentStr << "static " << getCppType(true, enumObj) << " " << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(true, enumObj) << " value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(true, enumObj, true) << " Internal::Helper::" << getToCppWinrtName(enumObj) << "(" << getCppType(true, enumObj) << "  &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return ref new Platform::Box< " << getCppWinrtType(false, enumObj) << " >(" << getToCppWinrtName(enumObj) << "(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(true, enumObj) << " Internal::Helper::" << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(true, enumObj) << " value)\n";
          cppSS << "{\n";
          cppSS << "  " << getCppType(true, enumObj) << " result;\n";
          cppSS << "  if (!value) return result;\n";
          cppSS << "  result = " << getFromCppWinrtName(enumObj) << "(value->Value);\n";
          cppSS << "  return result\n;";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForList(
                                               HelperFile &helperFile,
                                               StructPtr structObj
                                               )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }

            TypePtr foundType = (*found);
            ss << indentStr << "static Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(false, foundType, true) << " >^ " << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::list< " << getCppType(false, foundType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::list< " << getCppType(false, foundType) << "> > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(false, foundType) << " >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(false, foundType, true) << " >^ Internal::Helper::" << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::list< " << getCppType(false, foundType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Vector< " << getCppWinrtType(false, foundType) << " >();\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Append(" << getToCppWinrtName(foundType) << "(*iter));\n";
            cppSS << "  }\n";
            cppSS << "  return result->GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::list<" << getCppType(false, foundType) << "> > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(false, foundType) << " >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::list< " << getCppType(false, foundType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::list< " << getCppType(false, foundType) << " > >();\n";
            cppSS << "  for (" << getCppWinrtType(false, foundType) << " value : values)\n";
            cppSS << "  {\n";
            cppSS << "    result->push_back(" << getFromCppWinrtName(foundType) << "(value));\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForMap(
                                              HelperFile &helperFile,
                                              StructPtr structObj
                                              )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

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

            ss << indentStr << "static Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType, true) << ", " << getCppWinrtType(false, valueType, true)  << " >^ " << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMap< " << getCppWinrtType(false, keyType) << ", " << getCppWinrtType(false, valueType) << " >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType, true) << ", " << getCppWinrtType(false, valueType, true) << " >^ Internal::Helper::" << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Map< " << getCppWinrtType(false, keyType) << ", " << getCppWinrtType(false, valueType)  << " >();\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Insert(" << getToCppWinrtName(keyType) << "((*iter).first), " << getToCppWinrtName(valueType) << "((*iter).second));\n";
            cppSS << "  }\n";
            cppSS << "  return result->GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType) << ", " << getCppWinrtType(false, valueType) << " >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::map< " << getCppType(false, keyType) << ", " << getCppType(false, valueType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::map<" << getCppType(false, keyType) << ", " << getCppType(false, valueType) << "> >();\n";
            cppSS << "  std::for_each(Windows::Foundation::Collections::begin(values), Windows::Foundation::Collections::end(values), [](Windows::Foundation::Collections::IKeyValuePair< " << getCppWinrtType(false, keyType) << ", " << getCppWinrtType(false, valueType) << " >^ pair)\n";
            cppSS << "  {\n";
            cppSS << "    result[" << getFromCppWinrtName(keyType) << "(pair->Key)] = " << getFromCppWinrtName(valueType) << "[pair->Value];\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForSet(
                                              HelperFile &helperFile,
                                              StructPtr structObj
                                              )
        {
          helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Unexpected template missing type: \"" + templatedStruct->getPathName());
            }

            TypePtr keyType = (*found);

            ss << indentStr << "static Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType, true) << ", Platform::Object^ >^ " << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::set< " << getCppType(false, keyType) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::set< " << getCppType(false, keyType) << " > > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType) << ", Platform::Object^ >^ values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType, true) << ", Platform::Object^ >^ Internal::Helper::" << getToCppWinrtName(templatedStruct) << "(shared_ptr< std::set< " << getCppType(false, keyType) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  auto result = ref new Platform::Collections::Map< " << getCppWinrtType(false, keyType) << ", Platform::Object^ >();\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result->Insert(" << getToCppWinrtName(keyType) << "(*iter), nullptr);\n";
            cppSS << "  }\n";
            cppSS << "  return result->GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::set< " << getCppType(false, keyType) << " > > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(false, keyType) << ", Platform::Object^ >^ values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::set< " << getCppType(false, keyType) << " > >();\n";
            cppSS << "  auto result = make_shared< std::set<" << getCppType(false, keyType) << "> >();\n";
            cppSS << "  std::for_each(Windows::Foundation::Collections::begin(values), Windows::Foundation::Collections::end(values), [result](Windows::Foundation::Collections::IKeyValuePair< " << getCppWinrtType(false, keyType) << ", Platform::Object^ >^ pair)\n";
            cppSS << "  {\n";
            cppSS << "    result->insert(" << getFromCppWinrtName(keyType) << "(pair->Key));\n";
            cppSS << "  });\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getBasicCppWinrtTypeString(
                                                      bool isOptional,
                                                      BasicTypePtr type,
                                                      bool isReturnType
                                                      )
        {
          if (!type) return String();

          switch (type->mBaseType) {
            case IEventingTypes::PredefinedTypedef_void:      return "void";

            case IEventingTypes::PredefinedTypedef_bool:      return makeCppWinrtOptional(isOptional, "Platform::Boolean");

            case IEventingTypes::PredefinedTypedef_uchar:     return makeCppWinrtOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_char:
            case IEventingTypes::PredefinedTypedef_schar:     return makeCppWinrtOptional(isOptional, "char16");
            case IEventingTypes::PredefinedTypedef_ushort:    return makeCppWinrtOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_short:
            case IEventingTypes::PredefinedTypedef_sshort:    return makeCppWinrtOptional(isOptional, "int16");
            case IEventingTypes::PredefinedTypedef_uint:      return makeCppWinrtOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_int:
            case IEventingTypes::PredefinedTypedef_sint:      return makeCppWinrtOptional(isOptional, "int32");
            case IEventingTypes::PredefinedTypedef_ulong:     return makeCppWinrtOptional(isOptional, "Internal::Helper::HelperULong");
            case IEventingTypes::PredefinedTypedef_long:
            case IEventingTypes::PredefinedTypedef_slong:     return makeCppWinrtOptional(isOptional, "Internal::Helper::HelperLong");
            case IEventingTypes::PredefinedTypedef_ulonglong: return makeCppWinrtOptional(isOptional, "int64");
            case IEventingTypes::PredefinedTypedef_longlong:
            case IEventingTypes::PredefinedTypedef_slonglong: return makeCppWinrtOptional(isOptional, "int64");
            case IEventingTypes::PredefinedTypedef_uint8:     return makeCppWinrtOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_int8:
            case IEventingTypes::PredefinedTypedef_sint8:     return makeCppWinrtOptional(isOptional, "int8");
            case IEventingTypes::PredefinedTypedef_uint16:    return makeCppWinrtOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_int16:
            case IEventingTypes::PredefinedTypedef_sint16:    return makeCppWinrtOptional(isOptional, "int16");
            case IEventingTypes::PredefinedTypedef_uint32:    return makeCppWinrtOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_int32:
            case IEventingTypes::PredefinedTypedef_sint32:    return makeCppWinrtOptional(isOptional, "int32");
            case IEventingTypes::PredefinedTypedef_uint64:    return makeCppWinrtOptional(isOptional, "uint64");
            case IEventingTypes::PredefinedTypedef_int64:
            case IEventingTypes::PredefinedTypedef_sint64:    return makeCppWinrtOptional(isOptional, "int64");

            case IEventingTypes::PredefinedTypedef_byte:      return makeCppWinrtOptional(isOptional, "uint8");
            case IEventingTypes::PredefinedTypedef_word:      return makeCppWinrtOptional(isOptional, "uint16");
            case IEventingTypes::PredefinedTypedef_dword:     return makeCppWinrtOptional(isOptional, "uint32");
            case IEventingTypes::PredefinedTypedef_qword:     return makeCppWinrtOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_float:     return makeCppWinrtOptional(isOptional, "Internal::Helper::HelperFloat");
            case IEventingTypes::PredefinedTypedef_double:
            case IEventingTypes::PredefinedTypedef_ldouble:   return makeCppWinrtOptional(isOptional, "float64");
            case IEventingTypes::PredefinedTypedef_float32:   return makeCppWinrtOptional(isOptional, "float32");
            case IEventingTypes::PredefinedTypedef_float64:   return makeCppWinrtOptional(isOptional, "float64");

            case IEventingTypes::PredefinedTypedef_pointer:   return makeCppWinrtOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_binary:    return isReturnType ? "Platform::Array<byte>^" : "const Platform::Array<byte>^";
            case IEventingTypes::PredefinedTypedef_size:      return makeCppWinrtOptional(isOptional, "uint64");

            case IEventingTypes::PredefinedTypedef_string:
            case IEventingTypes::PredefinedTypedef_astring:
            case IEventingTypes::PredefinedTypedef_wstring:   return makeCppWinrtOptional(false, "Platform::String^");
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::makeCppWinrtOptional(bool isOptional, const String &value)
        {
          if (!isOptional) return value;
          return "Platform::IBox< " + value + " >^";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppType(
                                            bool isOptional,
                                            TypePtr type
                                            )
        {
          return GenerateStructHeader::getWrapperTypeString(isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtType(
                                           bool isOptional,
                                           TypePtr type,
                                           bool isReturnType
                                           )
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
              return getBasicCppWinrtTypeString(isOptional, basicType, isReturnType);
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              if (structType->mGenerics.size() > 0) return String();
              if (structType->hasModifier(Modifier_Special)) {
                String specialName = structType->getPathName();
                if ("::zs::Any" == specialName) return "Platform::Object^";
                if ("::zs::Promise" == specialName) return "Windows::Foundation::IAsyncAction^";
                if ("::zs::exceptions::Exception" == specialName) return "Platform::FailureException^";
                if ("::zs::exceptions::InvalidArgument" == specialName) return "Platform::InvalidArgumentException^";
                if ("::zs::exceptions::BadState" == specialName) return "Platform::COMException^";
                if ("::zs::exceptions::NotImplemented" == specialName) return "Platform::NotImplementedException^";
                if ("::zs::exceptions::NotSupported" == specialName) return "Platform::COMException^";
                if ("::zs::exceptions::UnexpectedError" == specialName) return "Platform::COMException^";
                if ("::zs::Time" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::DateTime");
                if ("::zs::Milliseconds" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Microseconds" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Nanoseconds" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Seconds" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Minutes" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Hours" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
                if ("::zs::Days" == specialName) return makeCppWinrtOptional(isOptional, "Windows::Foundation::TimeSpan");
              }
              return makeCppWinrtOptional(false, fixNamePath(structType) + "^");
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return makeCppWinrtOptional(isOptional, fixNamePath(enumType));
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              String templatedTypeStr;
              String specialName;
              String specialTemplatePost;

              {
                auto parentStruct = templatedType->getParentStruct();
                if (parentStruct) {
                  if (parentStruct->hasModifier(Modifier_Special)) {
                    specialName = parentStruct->getPathName();
                    if ("::std::set" == specialName) {
                      templatedTypeStr = "Windows::Foundation::Collections::IMapView< ";
                      specialTemplatePost = ", Platform::Object^";
                    }
                    if ("::std::list" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IVectorView< ";
                    if ("::std::map" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IMapView< ";
                    if ("::zs::PromiseWith" == specialName) templatedTypeStr = "Windows::Foundation::IAsyncOperation< ";
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
                String typeStr = getCppWinrtType(false, templateArg, isReturnType);
                if ("void" == typeStr) continue;
                if (!first) templatedTypeStr += ", ";
                templatedTypeStr += typeStr;
                first = false;
              }
              templatedTypeStr += specialTemplatePost + " >^";
              return makeCppWinrtOptional(false, templatedTypeStr);
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtAttributes(const StringList &attributes)
        {
          if (attributes.size() < 1) return String();

          std::stringstream ss;

          ss << "[";

          bool firstAttribute = true;
          for (auto iter = attributes.begin(); iter != attributes.end(); ++iter)
          {
            String attributeStr = (*iter);
            if (!firstAttribute) ss << ", ";
            firstAttribute = false;
            ss << attributeStr;
          }
          ss << "]";

          return ss.str();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtAttributesLine(
                                                     const String &linePrefix,
                                                     const StringList &attributes
                                                     )
        {
          if (attributes.size() < 1) return String();
          return linePrefix + getCppWinrtAttributes(attributes) + "\n";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getToFromCppWinrtName(TypePtr type)
        {
          if (!type) return String();

          type = type->getOriginalType();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case IEventingTypes::PredefinedTypedef_bool:      return "Boolean";

                case IEventingTypes::PredefinedTypedef_binary:    return "Binary";

                case IEventingTypes::PredefinedTypedef_ulong:     return "HelperULong";
                case IEventingTypes::PredefinedTypedef_long:
                case IEventingTypes::PredefinedTypedef_slong:     return "HelperLong";

                case IEventingTypes::PredefinedTypedef_float:     return "HelperFloat";

                case IEventingTypes::PredefinedTypedef_string:
                case IEventingTypes::PredefinedTypedef_astring:
                case IEventingTypes::PredefinedTypedef_wstring:   return "String";
                default:                                          break;
              }
              auto result = getBasicCppWinrtTypeString(false, basicType);
              return fixName(result);
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              String specialName = type->getPathName();

              if ("::zs::Any" == specialName) return String();
              if ("::zs::Promise" == specialName) return String();
              if ("::zs::exceptions::Exception" == specialName) return String();
              if ("::zs::exceptions::InvalidArgument" == specialName) return String();
              if ("::zs::exceptions::BadState" == specialName) return String();
              if ("::zs::exceptions::NotImplemented" == specialName) return String();
              if ("::zs::exceptions::NotSupported" == specialName) return String();
              if ("::zs::exceptions::UnexpectedError" == specialName) return String();
              if ("::zs::Time" == specialName) return String();
              if ("::zs::Milliseconds" == specialName) return String("Milliseconds");
              if ("::zs::Microseconds" == specialName) return String("Microseconds");
              if ("::zs::Nanoseconds" == specialName) return String("Nanoseconds");
              if ("::zs::Seconds" == specialName) return String("Seconds");
              if ("::zs::Minutes" == specialName) return String("Minutes");
              if ("::zs::Hours" == specialName) return String("Hours");
              if ("::zs::Days" == specialName) return String("Days");

              if ("::zs::PromiseWith" == specialName) return String("PromiseWith");
              if ("::std::set" == specialName) return String("Set");
              if ("::std::list" == specialName) return String("List");
              if ("::std::map" == specialName) return String("Map");

              String namePath = fixNamePath(structType);
              namePath.replaceAll("::", "_");
              namePath.trim("_");
              return namePath;
            }
          }

          {
            String result;
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              auto parent = templatedType->getParent();
              if (parent) {
                result = getToFromCppWinrtName(parent->toType());
              }

              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                String typeResult = getToFromCppWinrtName(*iter);
                if (typeResult.isEmpty()) continue;
                if (result.hasData()) result += "_";
                result += typeResult;
              }
            }
            return result;
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getToCppWinrtName(TypePtr type)
        {
          if (!type) return "ToCppWinrt";
          String result = getToFromCppWinrtName(type);
          if (result.hasData()) return "ToCppWinrt_" + result;
          return "ToCppWinrt";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getFromCppWinrtName(TypePtr type)
        {
          if (!type) return "FromCppWinrt";
          String result = getToFromCppWinrtName(type);
          if (result.hasData()) return "FromCppWinrt_" + result;
          return "FromCppWinrt";
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::IncludeProcessedInfo::IncludeProcessedInfo()
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::IncludeProcessedInfo::~IncludeProcessedInfo()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeCppForType(
                                                 StructFile &structFile,
                                                 TypePtr type
                                                 )
        {
          IncludeProcessedInfo info;
          includeCppForType(info, structFile, type);
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeCppForType(
                                                 IncludeProcessedInfo &processed,
                                                 StructFile &structFile,
                                                 TypePtr type
                                                 )
        {
          if (!type) return;

          type = type->getOriginalType();

          String path = type->getPathName();
          auto found = processed.processedTypes_.find(path);
          if (found != processed.processedTypes_.end()) return;

          processed.processedTypes_.insert(path);

          {
            auto structObj = type->toStruct();
            if (structObj) {
              String specialName = type->getPathName();

              includeTemplatedStructForType(processed, structFile, structObj);

              if ("::zs::Any" == specialName) return;
              if ("::zs::Promise" == specialName) return;
              if ("::zs::exceptions::Exception" == specialName) return;
              if ("::zs::exceptions::InvalidArgument" == specialName) return;
              if ("::zs::exceptions::BadState" == specialName) return;
              if ("::zs::exceptions::NotImplemented" == specialName) return;
              if ("::zs::exceptions::NotSupported" == specialName) return;
              if ("::zs::exceptions::UnexpectedError" == specialName) return;
              if ("::zs::Time" == specialName) return;
              if ("::zs::Milliseconds" == specialName) return;
              if ("::zs::Microseconds" == specialName) return;
              if ("::zs::Nanoseconds" == specialName) return;
              if ("::zs::Seconds" == specialName) return;
              if ("::zs::Minutes" == specialName) return;
              if ("::zs::Hours" == specialName) return;
              if ("::zs::Days" == specialName) return;

              if ("::std::list" == specialName) return;
              if ("::std::map" == specialName) return;
              if ("::std::set" == specialName) return;
              if ("::zs::PromiseWith" == specialName) return;

              structFile.includeCpp(String("\"") + fixStructFileName(structObj) + ".h\"");
              return;
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                includeCppForType(processed, structFile, *iter);
              }
              auto parent = templatedType->getParent();
              if (!parent) return;
              includeCppForType(processed, structFile, parent->toStruct());
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeTemplatedStructForType(
                                                             IncludeProcessedInfo &processed,
                                                             StructFile &structFile,
                                                             StructPtr structObj
                                                             )
        {
          if (!structObj) return;

          String path = structObj->getPathName();
          auto found = processed.structProcessedTypes_.find(path);
          if (found != processed.structProcessedTypes_.end()) return;

          processed.structProcessedTypes_.insert(path);

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedType = (*iter).second;
            if (!templatedType) continue;
            includeTemplatedStructForType(processed, structFile, templatedType->toTemplatedStructType());
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedType = (*iter).second;
            if (!relatedType) continue;
            includeTemplatedStructForType(processed, structFile, relatedType->toStruct());
            includeTemplatedStructForType(processed, structFile, relatedType->toTemplatedStructType());
          }

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);

            if (method->mResult) {
              includeTemplatedStructForType(processed, structFile, method->mResult->toStruct());
              includeTemplatedStructForType(processed, structFile, method->mResult->toTemplatedStructType());
            }
            for (auto iterArg = method->mArguments.begin(); iterArg != method->mArguments.end(); ++iterArg)
            {
              auto argType = (*iterArg);
              if (!argType) continue;
              if (!argType->mType) continue;
              includeTemplatedStructForType(processed, structFile, argType->mType->toStruct());
              includeTemplatedStructForType(processed, structFile, argType->mType->toTemplatedStructType());
            }
          }

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter) {
            auto property = (*iter);
            if (!property->mType) continue;

            includeTemplatedStructForType(processed, structFile, property->mType->toStruct());
            includeTemplatedStructForType(processed, structFile, property->mType->toTemplatedStructType());
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeTemplatedStructForType(
                                                             IncludeProcessedInfo &processed,
                                                             StructFile &structFile,
                                                             TemplatedStructTypePtr templatedStructObj
                                                             )
        {
          if (!templatedStructObj) return;

          String path = templatedStructObj->getPathName();
          auto found = processed.templatedProcessedTypes_.find(path);
          if (found != processed.templatedProcessedTypes_.end()) return;

          processed.templatedProcessedTypes_.insert(path);

          for (auto iter = templatedStructObj->mTemplateArguments.begin(); iter != templatedStructObj->mTemplateArguments.end(); ++iter) {
            auto argType = (*iter);
            if (!argType) continue;

            auto argStructType = argType->toStruct();

            includeCppForType(processed, structFile, argStructType);
            includeTemplatedStructForType(processed, structFile, argStructType);
            includeTemplatedStructForType(processed, structFile, argType->toTemplatedStructType());
          }

          auto parent = templatedStructObj->getParent();
          if (!parent) return;

          includeTemplatedStructForType(processed, structFile, parent->toStruct());
          includeTemplatedStructForType(processed, structFile, parent->toTemplatedStructType());
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::targetKeyword()
        {
          return String("cppwinrt");
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::targetKeywordHelp()
        {
          return String("Generate CPP/WINRT UWP wrapper");
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::targetOutput(
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

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("cppwinrt"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          HelperFile helperFile;
          helperFile.global_ = project->mGlobal;

          GenerateStructMsidl::scanNamespaceForStructsNeedingToBeInterfaces(*(helperFile.structsNeedingInterface_), helperFile.global_);

          writeBinary(UseHelper::fixRelativeFilePath(pathStr, String("types.h")), generateTypesHeader(project));

          calculateRelations(project->mGlobal, helperFile.derives_);

          helperFile.headerFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("cppwinrt_Helpers.h"));
          helperFile.cppFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("cppwinrt_Helpers.cpp"));

          {
            auto &ss = helperFile.headerIncludeSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "#pragma once\n\n";
            ss << "#include \"types.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppIncludeSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "#include \"cppwinrt_Helpers.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppBodySS_;
            GenerateStructHeader::generateUsingTypes(ss, "");
            ss << "using namespace date;\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.headerStructSS_;
            auto &finalSS = helperFile.headerFinalSS_;
            auto &indentStr = helperFile.headerIndentStr_;

            ss << "namespace Internal {\n";
            indentStr += "  ";
            GenerateStructHeader::generateUsingTypes(ss, indentStr);
            ss << "\n";
            ss << indentStr << "struct Helper {\n";
            finalSS << indentStr << "};\n\n";
            finalSS << "} // namespace Internal\n";

            indentStr += "  ";

            ss << "\n";
            ss << "#ifdef _WIN64\n";
            ss << indentStr << "typedef float64 HelperFloat;\n";
            ss << indentStr << "typedef int64 HelperLong;\n";
            ss << indentStr << "typedef uint64 HelperULong;\n";
            ss << "#else //_WIN64\n";
            ss << indentStr << "typedef float32 HelperFloat;\n";
            ss << indentStr << "typedef int32 HelperLong;\n";
            ss << indentStr << "typedef uint32 HelperULong;\n";
            ss << "#endif //_WIN64\n";
            ss << "\n";
            generateSpecialHelpers(helperFile);
          }

          generateForNamespace(helperFile, project->mGlobal, String());

          helperFile.headerIncludeSS_ << "\n";
          helperFile.headerIncludeSS_ << helperFile.headerStructSS_.str();
          helperFile.headerIncludeSS_ << helperFile.headerFinalSS_.str();

          helperFile.cppIncludeSS_ << "\n";
          helperFile.cppIncludeSS_ << helperFile.cppBodySS_.str();

          writeBinary(helperFile.headerFileName_, UseHelper::convertToBuffer(helperFile.headerIncludeSS_.str()));
          writeBinary(helperFile.cppFileName_, UseHelper::convertToBuffer(helperFile.cppIncludeSS_.str()));
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
