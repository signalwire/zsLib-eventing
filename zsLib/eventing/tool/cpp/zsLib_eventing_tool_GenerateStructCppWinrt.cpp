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
        //
        // GenerateStructCppWinrt::HelperFile
        //

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::HelperFile::HelperFile() noexcept :
          structsNeedingInterface_(make_shared<StructSet>())
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::HelperFile::~HelperFile() noexcept
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::HelperFile::includeHeader(const String &headerFile) noexcept
        {
          auto &ss = headerIncludeSS_;

          if (headerAlreadyIncluded_.end() != headerAlreadyIncluded_.find(headerFile)) return;
          headerAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::HelperFile::includeCpp(const String &headerFile) noexcept
        {
          auto &ss = cppIncludeSS_;

          if (cppAlreadyIncluded_.end() != cppAlreadyIncluded_.find(headerFile)) return;
          cppAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        bool GenerateStructCppWinrt::HelperFile::isStructNeedingInterface(StructPtr structObj) const noexcept
        {
          return (structsNeedingInterface_->end() != structsNeedingInterface_->find(structObj));
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // GenerateStructCppWinrt::StructFile
        //

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::StructFile::StructFile() noexcept :
          structsNeedingInterface_(make_shared<StructSet>())
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::StructFile::~StructFile() noexcept
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::StructFile::includeCpp(const String &headerFile) noexcept
        {
          auto &ss = cppIncludeSS_;

          if (cppAlreadyIncluded_.end() != cppAlreadyIncluded_.find(headerFile)) return;
          cppAlreadyIncluded_.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }

        //---------------------------------------------------------------------
        bool GenerateStructCppWinrt::StructFile::isStructNeedingInterface(StructPtr structObj) const noexcept
        {
          return (structsNeedingInterface_->end() != structsNeedingInterface_->find(structObj));
        }


        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // GenerateStructCppWinrt
        //


        //-------------------------------------------------------------------
        GenerateStructCppWinrt::GenerateStructCppWinrt() noexcept : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructCppWinrtPtr GenerateStructCppWinrt::create() noexcept
        {
          return make_shared<GenerateStructCppWinrt>();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixName(const String &originalName) noexcept
        {
          return GenerateStructMsidl::fixName(originalName);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixNamePath(ContextPtr context) noexcept
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
        String GenerateStructCppWinrt::fixNamePathNoPrefix(ContextPtr context) noexcept
        {
          String result = fixNamePath(context);
          if (0 != result.find("::")) return result;
          return result.substr(strlen("::"));
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixStructName(StructPtr structObj) noexcept
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
        String GenerateStructCppWinrt::fixMethodDeclaration(ContextPtr context) noexcept
        {
          String result = fixNamePath(context);
          if ("::" == result.substr(0, 2)) {
            return result.substr(2);
          }
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixMethodDeclaration(StructPtr derivedStruct, ContextPtr context) noexcept
        {
          return fixMethodDeclaration(derivedStruct, nullptr != context ? context->mName : String());
        }
        
        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixMethodDeclaration(StructPtr derivedStruct, const String &name) noexcept
        {
          String result = fixMethodDeclaration(derivedStruct);
          if (name.isEmpty()) return result;

          if (result.hasData()) {
            result += "::";
          }
          result += fixName(name);
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixStructFileName(StructPtr structObj) noexcept
        {
          auto result = fixNamePath(structObj);
          result.replaceAll("::", ".");
          result.trim(".");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixStructFileNameAsPath(StructPtr structObj) noexcept
        {
          auto result = fixNamePath(structObj);
          result.replaceAll("::", "/");
          result.trim("/");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getStructInitName(StructPtr structObj) noexcept
        {
          return GenerateStructHeader::getStructInitName(structObj);
        }

        //-------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtStructInitName(StructPtr structObj) noexcept
        {
          String namePathStr = fixNamePath(structObj);
          namePathStr.replaceAll("::", "_");
          namePathStr.trim("_");
          return namePathStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::fixEnumName(EnumTypePtr enumObj) noexcept
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
        String GenerateStructCppWinrt::fixArgumentName(const String &originalName) noexcept
        {
          if (originalName == "delegate") return "delegateValue";
          if (originalName == "event") return "eventValue";
          return originalName;
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::processTypesNamespace(
                                                           std::stringstream &iss,
                                                           std::stringstream &ss,
                                                           const String &inIndentStr,
                                                           NamespacePtr namespaceObj
                                                           ) noexcept
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

            String namePath = fixNamePath(namespaceObj);
            namePath.replaceAll("::", ".");
            namePath.trim(".");

            if ((namespaceObj->mEnums.size() > 0) ||
                (namespaceObj->mStructs.size() > 0)) {
              iss << "#include \"winrt/" << namePath << ".h\"\n";
            }

            ss << indentStr << "namespace " << fixName(namespaceObj->mName) << " {\n";
            auto parent = namespaceObj->getParent();
            if (parent) {
              auto parentNamespace = parent->toNamespace();
              if (parentNamespace->isGlobal()) {
                GenerateStructHeader::generateUsingTypes(ss, indentStr + "  ");
              }
            }
          } else {
            ss << indentStr << "namespace winrt {\n";
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
            processTypesNamespace(iss, ss, namespaceObj->isGlobal() ? "  " : inIndentStr, subNamespaceObj);
          }

          ss << indentStr << "namespace implementation {\n";
          String preStructIndentStr = indentStr;
          indentStr += "  ";

          bool firstStruct = true;
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto structObj = (*iter).second;
            processTypesStruct(ss, indentStr, structObj, firstStruct);
          }
          if (namespaceObj->mStructs.size() > 0) {
            GenerateHelper::insertLast(ss, firstStruct);
          }

          indentStr = preStructIndentStr;
          ss << indentStr << "} // namespace implementation\n";

          indentStr = initialIndentStr;

          if (namespaceObj->mName.hasData()) {
            ss << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n";
          } else {
            ss << "} // namespace winrt\n";
          }
        }
        
        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::processTypesStruct(
                                                        std::stringstream &ss,
                                                        const String &indentStr,
                                                        StructPtr structObj,
                                                        bool &firstFound
                                                        ) noexcept
        {
          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          GenerateHelper::insertFirst(ss, firstFound);

          ss << indentStr << "struct " << fixStructName(structObj) << ";\n";

          //bool found = processTypesEnum(ss, indentStr, structObj);
          //if (found) firstFound = true;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processTypesStruct(ss, indentStr, subStructObj, firstFound);
          }
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr GenerateStructCppWinrt::generateTypesHeader(ProjectPtr project) noexcept(false)
        {
          std::stringstream iss;
          std::stringstream ss;

          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          iss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          iss << "#pragma once\n\n";
          iss << "#include \"wrapper/generated/types.h\"\n";
          iss << "\n";

          processTypesNamespace(iss, ss, String(), project->mGlobal);

          std::stringstream resultSS;
          resultSS << iss.str();
          resultSS << "\n";
          resultSS << ss.str();

          return UseHelper::convertToBuffer(resultSS.str());
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateSpecialHelpers(HelperFile &helperFile) noexcept
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
        void GenerateStructCppWinrt::generateExceptionHelper(HelperFile &helperFile) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          auto &derives = helperFile.derives_;
          if (derives.end() != derives.find("::zs::exceptions::Exception")) {
            ss << helperFile.headerIndentStr_ << "static hresult_error ToCppWinrt(const ::zsLib::Exception &e) { return hresult_error(E_FAIL, ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::InvalidArgument")) {
            ss << helperFile.headerIndentStr_ << "static hresult_invalid_argument ToCppWinrt(const ::zsLib::Exceptions::InvalidArgument &e) { return hresult_invalid_argument(ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::BadState")) {
            ss << helperFile.headerIndentStr_ << "static hresult_error ToCppWinrt(const ::zsLib::Exceptions::BadState &e) { return hresult_error(E_NOT_VALID_STATE, ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotImplemented")) {
            ss << helperFile.headerIndentStr_ << "static hresult_not_implemented ToCppWinrt(const ::zsLib::Exceptions::NotImplemented &e) { return hresult_not_implemented(ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::NotSupported")) {
            ss << helperFile.headerIndentStr_ << "static hresult_error ToCppWinrt(const ::zsLib::Exceptions::NotSupported &e) { return hresult_error(CO_E_NOT_SUPPORTED, ToCppWinrt_String(e.message())); }\n";
          }
          if (derives.end() != derives.find("::zs::exceptions::UnexpectedError")) {
            ss << helperFile.headerIndentStr_ << "static hresult_error ToCppWinrt(const ::zsLib::Exceptions::UnexpectedError &e) { return hresult_error(E_UNEXPECTED, ToCppWinrt_String(e.message())); }\n";
          }
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateBasicTypesHelper(HelperFile &helperFile) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (IEventingTypes::PredefinedTypedefs index = IEventingTypes::PredefinedTypedef_First; index <= IEventingTypes::PredefinedTypedef_Last; index = static_cast<IEventingTypes::PredefinedTypedefs>(static_cast<std::underlying_type<IEventingTypes::PredefinedTypedefs>::type>(index) + 1)) {
            if (requiresSpecialConversion(index)) continue;

            String wrapperName = fixName(GenerateHelper::getConverstionNameString(index));
            String cppwinrtType = getBasicCppWinrtTypeString(index, GO{});
            String cppType = GenerateHelper::getBasicTypeString(index);
            bool safeIntType = GenerateHelper::isSafeIntType(index);
            bool isFloat = GenerateHelper::isFloat(index);

            ss << indentStr << "static " << cppwinrtType << " ToCppWinrt_" << wrapperName << "(" << cppType << " value);\n";
            ss << indentStr << "static Windows::Foundation::IReference<" << cppwinrtType << "> ToCppWinrt_" << wrapperName << "(const Optional<" << cppType << "> &value);\n";
            ss << "\n";

            ss << indentStr << "static " << cppType << " FromCppWinrt_" << wrapperName << "(" << cppwinrtType << " value);\n";
            ss << indentStr << "static Optional<" << cppType << "> FromCppWinrt_" << wrapperName << "(Windows::Foundation::IReference<" << cppwinrtType << "> const & value);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << cppwinrtType << " Internal::Helper::ToCppWinrt_" << wrapperName << "(" << cppType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  auto safeInt = SafeInt<" << cppwinrtType << ">(value);\n";
              cppSS << "  return safeInt.Ref();\n";
            } else if (isFloat) {
              cppSS << "  return (" << cppwinrtType << ")value;\n";
            } else {
              cppSS << "  return value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::IReference<" << cppwinrtType << "> Internal::Helper::ToCppWinrt_" << wrapperName << "(const Optional<" << cppType << "> &value)\n";
            cppSS << "{\n";
            cppSS << "  if (!value.hasValue()) return nullptr;\n";
            if (safeIntType) {
              cppSS << "  auto safeInt = SafeInt<" << cppwinrtType << ">(value.value());\n";
              cppSS << "  return Windows::Foundation::IReference<" << cppwinrtType << ">(safeInt.Ref());\n";
            } else if (isFloat) {
              cppSS << "  return Windows::Foundation::IReference<" << cppwinrtType << ">((" << cppwinrtType << ")(value.value()));\n";
            } else {
              cppSS << "  return Windows::Foundation::IReference<" << cppwinrtType << ">(value.value());\n";
            }
            cppSS << "}\n";
            cppSS << "\n";


            cppSS << dashedStr;
            cppSS << cppType << " Internal::Helper::FromCppWinrt_" << wrapperName << "(" << cppwinrtType << " value)\n";
            cppSS << "{\n";
            if (safeIntType) {
              cppSS << "  auto safeInt = SafeInt<" << cppType << ">(value);\n";
              cppSS << "  return safeInt.Ref();\n";
            } else if (isFloat) {
              cppSS << "  return (" << cppType << ")value;\n";
            } else {
              cppSS << "  return value;\n";
            }
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "Optional<" << cppType << "> Internal::Helper::FromCppWinrt_" << wrapperName << "(Windows::Foundation::IReference<" << cppwinrtType << "> const & value)\n";
            cppSS << "{\n";
            cppSS << "  Optional<" << cppType << "> result;\n";
            cppSS << "  if (!value) return result;\n";
            if (safeIntType) {
              cppSS << "  auto safeInt = SafeInt<" << cppType << ">(value.Value());\n";
              cppSS << "  result = safeInt.Ref();\n";
            } else if (isFloat) {
              cppSS << "  result = (" << cppType << ")(value.Value());\n";
            } else {
              cppSS << "  result = value.Value();\n";
            }
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateBinaryHelper(HelperFile &helperFile) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static com_array<uint8_t> ToCppWinrt_Binary(const SecureByteBlock &value);\n";
          ss << indentStr << "static com_array<uint8_t> ToCppWinrt_Binary(SecureByteBlockPtr value);\n";
          ss << indentStr << "static com_array<uint8_t> ToCppWinrt_Binary(const Optional<SecureByteBlockPtr> &value);\n";
          ss << indentStr << "static SecureByteBlockPtr FromCppWinrt_Binary(array_view<uint8_t const> const &value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "com_array<uint8_t> Internal::Helper::ToCppWinrt_Binary(const SecureByteBlock &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.BytePtr()) return com_array<uint8_t>();\n";
          cppSS << "  com_array<uint8_t> result(static_cast<array_view<uint8_t>::size_type>(value.SizeInBytes()));\n";
          cppSS << "  memcpy(result.data(), value.BytePtr(), sizeof(uint8_t)*value.SizeInBytes());\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "com_array<uint8_t> Internal::Helper::ToCppWinrt_Binary(SecureByteBlockPtr value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return com_array<uint8_t>();\n";
          cppSS << "  return ToCppWinrt_Binary(*value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "com_array<uint8_t> Internal::Helper::ToCppWinrt_Binary(const Optional<SecureByteBlockPtr> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return com_array<uint8_t>();\n";
          cppSS << "  if (!value.value()) return com_array<uint8_t>();\n";
          cppSS << "  return ToCppWinrt_Binary(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "SecureByteBlockPtr Internal::Helper::FromCppWinrt_Binary(array_view<uint8_t const> const &value)\n";
          cppSS << "{\n";
          cppSS << "  if (value.empty()) return SecureByteBlockPtr();\n";
          cppSS << "  return make_shared<SecureByteBlock>(value.data(), static_cast<SecureByteBlock::size_type>(value.size()));\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateStringHelper(HelperFile &helperFile) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static hstring ToCppWinrt_String(const std::string &value);\n";
          ss << indentStr << "static hstring ToCppWinrt_String(const Optional<std::string> &value);\n";
          ss << indentStr << "static hstring ToCppWinrt_String(const Optional<String> &value);\n";
          ss << indentStr << "static hstring ToCppWinrt_Astring(const std::string &value) {return ToCppWinrt_String(value);}\n";
          ss << indentStr << "static hstring ToCppWinrt_Astring(const Optional<std::string> &value) {return ToCppWinrt_String(value);}\n";
          ss << indentStr << "static hstring ToCppWinrt_Astring(const Optional<String> &value) {return ToCppWinrt_String(value);}\n";
          ss << indentStr << "static hstring ToCppWinrt_Wstring(const std::wstring &value);\n";
          ss << indentStr << "static hstring ToCppWinrt_Wstring(const Optional<std::wstring> &value);\n";
          ss << "\n";

          ss << indentStr << "static String FromCppWinrt_String(hstring const & value);\n";
          ss << indentStr << "static String FromCppWinrt_Astring(hstring const & value) {return FromCppWinrt_String(value);}\n";
          ss << indentStr << "static std::wstring FromCppWinrt_Wstring(hstring const & value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "hstring Internal::Helper::ToCppWinrt_String(const std::string &value)\n";
          cppSS << "{\n";
          cppSS << "  return hstring(String(value).wstring().c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "hstring Internal::Helper::ToCppWinrt_String(const Optional< std::string > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return hstring();\n";
          cppSS << "  return ToCppWinrt_String(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "hstring Internal::Helper::ToCppWinrt_String(const Optional< String > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return hstring();\n";
          cppSS << "  return ToCppWinrt_String(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "hstring Internal::Helper::ToCppWinrt_Wstring(const std::wstring &value)\n";
          cppSS << "{\n";
          cppSS << "  return hstring(value.c_str());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "hstring Internal::Helper::ToCppWinrt_Wstring(const Optional< std::wstring > &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return hstring();\n";
          cppSS << "  return ToCppWinrt_Wstring(value.value());\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "String Internal::Helper::FromCppWinrt_String(hstring const & value)\n";
          cppSS << "{\n";
          cppSS << "  if (value.size() < 1) return String();\n";
          cppSS << "  auto dataStr = value.data();\n";
          cppSS << "  if (!dataStr) return String();\n";
          cppSS << "  return String(std::wstring(dataStr));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "std::wstring Internal::Helper::FromCppWinrt_Wstring(hstring const & value)\n";
          cppSS << "{\n";
          cppSS << "  if (value.size() < 1) return std::wstring();\n";
          cppSS << "  auto dataStr = value.data();\n";
          cppSS << "  if (!dataStr) return std::wstring();\n";
          cppSS << "  return std::wstring(dataStr);\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateDurationHelper(
                                                            HelperFile &helperFile,
                                                            const String &durationType
                                                            ) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::TimeSpan ToCppWinrt_" << durationType << "(const ::zsLib::" << durationType << " &value);\n";
          ss << indentStr << "static Windows::Foundation::IReference<Windows::Foundation::TimeSpan> ToCppWinrt_" << durationType << "(const Optional<::zsLib::" << durationType << "> &value);\n";
          ss << indentStr << "static ::zsLib::" << durationType << " FromCppWinrt_" << durationType << "(Windows::Foundation::TimeSpan value);\n";
          ss << indentStr << "static Optional<::zsLib::" << durationType << "> FromCppWinrt_" << durationType << "(Windows::Foundation::IReference<Windows::Foundation::TimeSpan> const & value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::TimeSpan Internal::Helper::ToCppWinrt_" << durationType << "(const ::zsLib::" << durationType << " &value)\n";
          cppSS << "{\n";
          cppSS << "  return std::chrono::duration_cast<Windows::Foundation::TimeSpan>(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::IReference<Windows::Foundation::TimeSpan> Internal::Helper::ToCppWinrt_" << durationType << "(const Optional<::zsLib::" << durationType << "> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return Windows::Foundation::IReference<Windows::Foundation::TimeSpan>(ToCppWinrt_" << durationType << "(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "::zsLib::" << durationType << " Internal::Helper::FromCppWinrt_" << durationType << "(Windows::Foundation::TimeSpan value)\n";
          cppSS << "{\n";
          cppSS << "  return std::chrono::duration_cast<::zsLib::" << durationType << ">(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Optional<::zsLib::" << durationType << "> Internal::Helper::FromCppWinrt_" << durationType << "(Windows::Foundation::IReference<Windows::Foundation::TimeSpan> const & value)\n";
          cppSS << "{\n";
          cppSS << "  Optional<::zsLib::" << durationType << "> result;\n";
          cppSS << "  if (!value) return result;\n";
          cppSS << "  result = FromCppWinrt_" << durationType << "(value.Value());\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateTimeHelper(HelperFile &helperFile) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::DateTime ToCppWinrt(const ::zsLib::Time &value);\n";
          ss << indentStr << "static Windows::Foundation::IReference<Windows::Foundation::DateTime> ToCppWinrt(const Optional<::zsLib::Time> &value);\n";
          ss << indentStr << "static ::zsLib::Time FromCppWinrt(Windows::Foundation::DateTime value);\n";
          ss << indentStr << "static Optional<::zsLib::Time> FromCppWinrt(Windows::Foundation::IReference<Windows::Foundation::DateTime> const & value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::DateTime Internal::Helper::ToCppWinrt(const ::zsLib::Time &value)\n";
          cppSS << "{\n";
          cppSS << "  return Windows::Foundation::DateTime(std::chrono::duration_cast<Windows::Foundation::TimeSpan>(value.time_since_epoch()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::IReference<Windows::Foundation::DateTime> Internal::Helper::ToCppWinrt(const Optional<::zsLib::Time> &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return Windows::Foundation::IReference<Windows::Foundation::DateTime>(ToCppWinrt(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << "::zsLib::Time Internal::Helper::FromCppWinrt(Windows::Foundation::DateTime value)\n";
          cppSS << "{\n";
          cppSS << "  return ::zsLib::Time(std::chrono::duration_cast<::zsLib::Time::duration>(value.time_since_epoch()));\n";
          cppSS << "}\n";
          cppSS << "\n";


          cppSS << dashedStr;
          cppSS << "Optional<::zsLib::Time> Internal::Helper::FromCppWinrt(Windows::Foundation::IReference<Windows::Foundation::DateTime> const & value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return Optional<::zsLib::Time>();\n";
          cppSS << "  return Optional<::zsLib::Time>(FromCppWinrt(value.Value()));\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generatePromiseHelper(HelperFile &helperFile) noexcept
        {
          //helperFile.includeHeader("<ppltasks.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static Windows::Foundation::IAsyncOperation<Windows::Foundation::IInspectable> ToCppWinrt(::zsLib::PromisePtr promise);\n";

          cppSS << dashedStr;
          cppSS << "Windows::Foundation::IAsyncOperation<Windows::Foundation::IInspectable> Internal::Helper::ToCppWinrt(::zsLib::PromisePtr promise)\n";
          cppSS << "{\n";
          cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
          cppSS << "  {\n";
          cppSS << "    Observer(HANDLE handle) : handle_(handle) {}\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override { ::SetEvent(handle_); }\n";
          cppSS << "\n";
          cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override { ::SetEvent(handle_); }\n";
          cppSS << "\n";
          cppSS << "  private:\n";
          cppSS << "    HANDLE handle_;\n";
          cppSS << "  };\n";
          cppSS << "\n";
          cppSS << "  if (!promise) co_return Windows::Foundation::IInspectable {nullptr};\n";
          cppSS << "\n";
          cppSS << "  HANDLE handle = ::CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);\n";
          cppSS << "  auto observer = std::make_shared<Observer>(handle);";
          cppSS << "  promise->then(observer);\n";
          cppSS << "  promise->background();\n";
          cppSS << "\n";
          cppSS << "  co_await winrt::resume_on_signal(handle);\n";
          cppSS << "  ::CloseHandle(handle);\n";
          cppSS << "  handle = NULL;\n";
          cppSS << "\n";
          cppSS << "  if (promise->isRejected()) {\n";
          cppSS << "    PromisePtr promiseBase = promise;\n";

          generateDefaultPromiseRejections(helperFile, "    ");

          cppSS << "\n";
          cppSS << "    throw hresult_no_interface();\n";
          cppSS << "  }\n";
          cppSS << "  co_return Windows::Foundation::IInspectable {nullptr};\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generatePromiseWithHelper(HelperFile &helperFile) noexcept
        {
          //helperFile.includeHeader("<ppltasks.h>");

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

                ss << indentStr << "static " << getCppWinrtType(helperFile, templatedStruct, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< " << promiseWithStr << "< " << getCppType(resolveType, GO{}) << " > > promise);\n";

                cppSS << dashedStr;
                cppSS << getCppWinrtType(helperFile, templatedStruct, GO::MakeReturnResult()) << " Internal::Helper::" << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< " << promiseWithStr << "< " << getCppType(resolveType, GO{}) << " > > promise)\n";

                cppSS << "{\n";
                cppSS << "  struct Observer : public ::zsLib::IPromiseResolutionDelegate\n";
                cppSS << "  {\n";
                cppSS << "    Observer(HANDLE handle) : handle_(handle) {}\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseResolved(PromisePtr promise) override { ::SetEvent(handle_); }\n";
                cppSS << "\n";
                cppSS << "    virtual void onPromiseRejected(PromisePtr promise) override { ::SetEvent(handle_); }\n";
                cppSS << "\n";
                cppSS << "  private:\n";
                cppSS << "    HANDLE handle_;\n";
                cppSS << "  };\n";
                cppSS << "\n";
                cppSS << "  if (!promise) co_return Windows::Foundation::IInspectable {nullptr};\n";
                cppSS << "\n";
                cppSS << "  HANDLE handle = ::CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);\n";
                cppSS << "  auto observer = std::make_shared<Observer>(handle);";
                cppSS << "  promise->then(observer);\n";
                cppSS << "  promise->background();\n";
                cppSS << "\n";
                cppSS << "  co_await winrt::resume_on_signal(handle);\n";
                cppSS << "  ::CloseHandle(handle);\n";
                cppSS << "  handle = NULL;\n";
                cppSS << "\n";
                cppSS << "  if (promise->isResolved()) {\n";
                cppSS << "    auto value = promise->value();\n";
                cppSS << "    if (value) {\n";
                cppSS << "      auto result = (::Internal::Helper::" << getToCppWinrtName(helperFile, resolveType, GO{}) << "(value));\n";
                cppSS << "      co_return result.as<Windows::Foundation::IInspectable>();\n";
                cppSS << "    }\n";
                cppSS << "  }\n";

                cppSS << "  if (promise->isRejected()) {\n";
                cppSS << "    PromisePtr promiseBase = promise;\n";

                if (rejectType) {
                  auto rejectTypeStr = rejectType->getPathName();
                  if ("::void" != rejectTypeStr) {
                    generatePromiseRejection(helperFile, indentStr, rejectType);
                  }
                }
                generateDefaultPromiseRejections(helperFile, "    ");

                cppSS << "\n";
                cppSS << "    throw hresult_no_interface();\n";
                cppSS << "  }\n";
                cppSS << "  co_return Windows::Foundation::IInspectable {nullptr};\n";
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
                                                                      ) noexcept
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
                                                              ) noexcept
        {
          if (!rejectionType) return;

          String specialName = rejectionType->getPathName();

          auto &cppSS = helperFile.cppBodySS_;
          cppSS << indentStr << "{\n";
          cppSS << indentStr << "  auto reasonHolder = promiseBase->reason< ::zsLib::AnyHolder< " << getCppType(rejectionType, GO{}) << " > >();\n";
          cppSS << indentStr << "  if (reasonHolder) {\n";
          cppSS << indentStr << "    auto comResult = (::Internal::Helper::" << getToCppWinrtName(helperFile, rejectionType, GO{}) << "(reasonHolder->value_));\n";
          cppSS << indentStr << "    co_return comResult.as<Windows::Foundation::IInspectable>();\n";
          cppSS << indentStr << "  }\n";
          cppSS << indentStr << "}\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForNamespace(
                                                          HelperFile &helperFile,
                                                          NamespacePtr namespaceObj,
                                                          const String &inIndentStr
                                                          ) noexcept
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
                                                       ) noexcept
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
                                                               ) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          helperFile.includeCpp(String("\"") + fixStructFileName(structObj) + ".h\"");
          bool requiresInterface = helperFile.isStructNeedingInterface(structObj);

          ss << indentStr << "\n";
          ss << indentStr << "// " << getToCppWinrtName(helperFile, structObj, GO{}) << "\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          }

          ss << indentStr << "\n";
          ss << indentStr << "// " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          }

          if (requiresInterface) {
            ss << indentStr << "\n";
            ss << indentStr << "// " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppType(structObj, GO{}) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          }

          ss << indentStr << "\n";
          ss << indentStr << "// " << getFromCppWinrtName(structObj) << "\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " " << getFromCppWinrtName(structObj) << "(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " " << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " " << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppType(structObj, GO{}) << " " << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          }


          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{}) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrt(value);\n";
            cppSS << "}\n";
            cppSS << "\n";
          }


          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtImpl(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtImpl(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeImplementation() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtImpl(value);\n";
            cppSS << "}\n";
            cppSS << "\n";
          }


          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppType(structObj, GO{}) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtInterface(value);\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtInterface(value);\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::ToCppWinrtInterface(value);\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, structObj, GO{ GO::MakeInterface() }) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return value;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " Internal::Helper::" << getFromCppWinrtName(structObj) << "(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " Internal::Helper::" << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::FromCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " Internal::Helper::" << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::FromCppWinrt(value);\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppType(structObj, GO{}) << " Internal::Helper::" << getFromCppWinrtName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return " << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << "::FromCppWinrt(value);\n";
            cppSS << "}\n";
            cppSS << "\n";
          }

          generateStructFile(helperFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateStructFile(
                                                        HelperFile &helperFile,
                                                        StructPtr structObj
                                                        ) noexcept
        {
          if (!structObj) return;

          auto dashedStr = GenerateHelper::getDashedComment(String());

          String filename = fixStructFileName(structObj);
          String filenameAsPath = fixStructFileNameAsPath(structObj);

          StructFile structFile;
          structFile.struct_ = structObj;
          structFile.structsNeedingInterface_ = helperFile.structsNeedingInterface_;

          auto &includeSS = structFile.headerIncludeSS_;
          auto &delegateSS = structFile.headerStructEventHandlersSS_;
          auto &ss = structFile.headerStructPrivateSS_;
          auto &pubSS = structFile.headerStructPublicSS_;
          auto &cppSS = structFile.cppBodySS_;
          auto &cppIncludeSS = structFile.cppIncludeSS_;
          auto &observerSS = structFile.headerStructObserverSS_;
          auto &observerFinalSS = structFile.headerStructObserverFinalSS_;

          auto &indentStr = structFile.headerStructIndentStr_;

          bool requiresInterface = helperFile.isStructNeedingInterface(structObj);

          includeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          includeSS << "#pragma once\n\n";

          String ifdefName = (structObj->hasModifier(Modifier_Special) ? "CPPWINRT_USE_GENERATED_" : "CPPWINRT_USE_CUSTOM_") + getCppWinrtStructInitName(structObj);
          ifdefName.toUpper();

          includeSS << "\n";
          includeSS << "#" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "ifdef") << " " << ifdefName << "\n";
          includeSS << "#include <wrapper/override/cppwinrt/" << filename << ".h>\n";
          includeSS << "#else // " << ifdefName << "\n";

          includeSS << "\n";
          includeSS << "#include \"types.h\"\n";
          includeSS << "\n";

          cppIncludeSS << "// " ZS_EVENTING_GENERATED_BY "\n\n";

          cppIncludeSS << "#include \"pch.h\"\n";
          cppIncludeSS << "\n";
          cppIncludeSS << "#" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "ifdef") << " " << ifdefName << "\n";
          cppIncludeSS << "#include <wrapper/override/cppwinrt/" << filename << ".cpp>\n";
          cppIncludeSS << "#else // " << ifdefName << "\n";

          structFile.includeCpp("\"cppwinrt_Helpers.h\"");
          structFile.includeCpp("\"" + filename + ".h\"");

          cppSS << "using namespace winrt;\n\n";

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
            includeSS << "#include \"" << filenameAsPath << ".g.h\"\n";
            includeSS << "#include <wrapper/generated/" << GenerateStructHeader::getStructFileName(parentStruct) << ">\n";
            includeSS << "\n";
          }

          // insert namespace
          {
            includeSS << indentStr << "namespace winrt {\n";
            endingStrs.push_back(indentStr + "} // namespace winrt");
            indentStr += "  ";
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

          includeSS << indentStr << "namespace implementation {\n";
          ss << "\n";

          indentStr += "  ";

          ss << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);

          ss << indentStr << "struct " << fixStructName(structObj) << " : " << fixStructName(structObj) << "T<" << fixStructName(structObj) << ">\n";
          ss << indentStr << "{\n";
          pubSS << indentStr << "public:\n";

          structFile.headerIndentStr_ = indentStr;

          indentStr += "  ";

          ss << indentStr << "// internal\n";
          ss << indentStr << getCppType(structObj, GO{}) << " native_;\n";
          if (hasEvents) {
            ss << indentStr << "wrapper" << structObj->getPathName() << "::WrapperObserverPtr observer_;\n";
          }
          ss << "\n";
          ss << indentStr << "struct WrapperCreate {};\n";
          ss << indentStr << fixStructName(structObj) << "(const WrapperCreate &) {}\n";
          ss << "\n";

          ss << indentStr << "// ToCppWinrtImpl\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " ToCppWinrtImpl(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          }
          ss << "\n";

          ss << indentStr << "// ToCppWinrt\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrt(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeImplementation(), GO::MakeComPtr() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult()}) << " ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value);\n";
          }
          ss << "\n";

          ss << indentStr << "// ToCppWinrtInterface\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " ToCppWinrtInterface(" << getCppType(structObj, GO{}) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value);\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
          } else {
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrtInterface(" << getCppType(structObj, GO{}) << " value) { return ToCppWinrt(value); }\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value) { return value; }\n";
            ss << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value) { return ToCppWinrt(value); }\n";
          }
          ss << "\n";

          ss << indentStr << "// FromCppWinrt\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " FromCppWinrt(" << getCppType(structObj, GO{}) << " value);\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeImplementation(), GO::MakeComPtr() }) << " value);\n";
          ss << indentStr << "static " << getCppType(structObj, GO{}) << " FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value);\n";
          if (requiresInterface) {
            ss << indentStr << "static " << getCppType(structObj, GO{}) << " FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value);\n";
          }
          ss << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtImpl(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return nullptr;\n";
          cppSS << "  auto result = winrt::make_self<" << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << ">(WrapperCreate{});\n";
          cppSS << "  result->native_ = value;\n";
          if (hasEvents) {
            cppSS << "  result->observer_ = make_shared<WrapperObserverImpl>(result);\n";
            cppSS << "  result->native_->wrapper_installObserver(result->observer_);\n";
          }
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " impl {nullptr};\n";
          cppSS << "  impl.copy_from(winrt::from_abi<" << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << ">(value));\n";
          cppSS << "  return impl;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtImpl(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeImplementation(), GO::MakeComPtr() }) << " impl {nullptr};\n";
            cppSS << "  impl.copy_from(winrt::from_abi<" << getCppWinrtType(helperFile, structObj, GO::MakeImplementation()) << ">(value));\n";
            cppSS << "  return impl;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }


          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrt(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  auto result = ToCppWinrtImpl(value);\n";
          cppSS << "  return result.as< " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " >();\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeImplementation(), GO::MakeComPtr() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value.as< " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " >();\n";
          cppSS << "}\n";
          cppSS << "\n";

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return value.as< " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " >();\n";
            cppSS << "}\n";
            cppSS << "\n";
          }

          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtInterface(" << getCppType(structObj, GO{}) << " value)\n";
            cppSS << "{\n";
            cppSS << "  auto result = ToCppWinrtImpl(value);\n";
            cppSS << "  return result.as< " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " >();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return value.as< " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " >();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return value.as< " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " >();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::ToCppWinrtInterface(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return value;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeImplementation(), GO::MakeComPtr() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value) return " << getCppType(structObj, GO{}) << "();\n";
          cppSS << "  return value->native_;\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return FromCppWinrt(ToCppWinrtImpl(value));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(structObj, GO{}) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::FromCppWinrt(" << getCppType(structObj, GO{}) << " value)\n";
          cppSS << "{\n";
          cppSS << "  return value;\n";
          cppSS << "}\n";
          cppSS << "\n";


          if (requiresInterface) {
            cppSS << dashedStr;
            cppSS << getCppType(structObj, GO{}) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::FromCppWinrt(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  return FromCppWinrt(ToCppWinrtImpl(value));\n";
            cppSS << "}\n";
            cppSS << "\n";
          }


          if (GenerateHelper::needsDefaultConstructor(structObj)) {
            pubSS << indentStr << fixStructName(structObj) << "();\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::" << fixStructName(structObj) << "()\n";
            cppSS << "  : native_(" << "wrapper" << structObj->getPathName() << "::wrapper_create()" << ")";
            if (hasEvents) {
              cppSS << ",\n";
              cppSS << "    observer_(make_shared<WrapperObserverImpl>(get_strong()))";
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
            observerSS << indentStr << "  WrapperObserverImpl(" << getCppWinrtType(helperFile, structObj, GO{GO::MakeImplementation(), GO::MakeReference(), GO::MakeComPtr()}) << " owner) : owner_(owner) {}\n";
            observerSS << "\n";
            observerFinalSS << indentStr << "  " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation(), GO::MakeComPtr() }) << " owner_;\n";
            observerFinalSS << indentStr << "};\n";
          }

          bool foundCast = false;
          if (requiresInterface) {
            pubSS << indentStr << "/// <summary>\n";
            pubSS << indentStr << "/// Cast from " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface() }) << " to " << fixStructName(structObj) << "\n";
            pubSS << indentStr << "/// </summary>\n";
            pubSS << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " CastFromI" << fixStructName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";

            cppSS << dashedStr;
            cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::CastFromI" << fixStructName(structObj) << "(" << getCppWinrtType(helperFile, structObj, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
            cppSS << "{\n";
            cppSS << "  if (!value) return nullptr;\n";
            cppSS << "  auto nativeObject = ::Internal::Helper::" << getFromCppWinrtName(structObj) << "(value);  \n";
            cppSS << "  if (!nativeObject) return nullptr;\n";
            cppSS << "  auto result = std::dynamic_pointer_cast< wrapper" << structObj->getPathName() << " >(nativeObject);\n";
            cppSS << "  if (!result) return nullptr;\n";
            cppSS << "  return ToCppWinrt(result);\n";
            cppSS << "}\n";
            cppSS << "\n";
            foundCast = true;
          }

          {
            auto found = helperFile.derives_.find(structObj->getPathName());
            if (found != helperFile.derives_.end()) {
              auto &structSet = (*found).second;
              for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet)
              {
                auto foundStruct = (*iterSet);
                if (foundStruct != structObj) {
                  includeCppForType(structFile, foundStruct);

                  bool foundRequiresInterface = helperFile.isStructNeedingInterface(foundStruct);

                  pubSS << indentStr << "/// <summary>\n";
                  pubSS << indentStr << "/// Cast from " << getCppWinrtType(helperFile, foundStruct, GO{}) << " to " << fixStructName(structObj) << "\n";
                  pubSS << indentStr << "/// </summary>\n";
                  pubSS << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " CastFrom" << fixStructName(foundStruct) << "(" << getCppWinrtType(helperFile, foundStruct, GO::MakeReference()) << " value);\n";
                  if (foundRequiresInterface) {
                    pubSS << indentStr << "/// <summary>\n";
                    pubSS << indentStr << "/// Cast from " << getCppWinrtType(helperFile, foundStruct, GO{ GO::MakeInterface() }) << " to " << fixStructName(structObj) << "\n";
                    pubSS << indentStr << "/// </summary>\n";
                    pubSS << indentStr << "static " << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " CastFromI" << fixStructName(foundStruct) << "(" << getCppWinrtType(helperFile, foundStruct, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value);\n";
                  }
                  foundCast = true;

                  cppSS << dashedStr;
                  cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::CastFrom" << fixStructName(foundStruct) << "(" << getCppWinrtType(helperFile, foundStruct, GO::MakeReference()) << " value)\n";
                  cppSS << "{\n";
                  cppSS << "  if (!value) return nullptr;\n";
                  cppSS << "  auto nativeObject = ::Internal::Helper::" << getFromCppWinrtName(foundStruct) << "(value);  \n";
                  cppSS << "  if (!nativeObject) return nullptr;\n";
                  cppSS << "  auto result = std::dynamic_pointer_cast< wrapper" << structObj->getPathName() << " >(nativeObject);\n";
                  cppSS << "  if (!result) return nullptr;\n";
                  cppSS << "  return ToCppWinrt(result);\n";
                  cppSS << "}\n";
                  cppSS << "\n";

                  if (foundRequiresInterface) {
                    cppSS << dashedStr;
                    cppSS << getCppWinrtType(helperFile, structObj, GO::MakeReturnResult()) << " " << getCppWinrtType(helperFile, structObj, GO{ GO::MakeImplementation() }) << "::CastFromI" << fixStructName(foundStruct) << "(" << getCppWinrtType(helperFile, foundStruct, GO{ GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
                    cppSS << "{\n";
                    cppSS << "  if (!value) return nullptr;\n";
                    cppSS << "  auto nativeObject = ::Internal::Helper::" << getFromCppWinrtName(foundStruct) << "(value);  \n";
                    cppSS << "  if (!nativeObject) return nullptr;\n";
                    cppSS << "  auto result = std::dynamic_pointer_cast< wrapper" << structObj->getPathName() << " >(nativeObject);\n";
                    cppSS << "  if (!result) return nullptr;\n";
                    cppSS << "  return ToCppWinrt(result);\n";
                    cppSS << "}\n";
                    cppSS << "\n";
                  }
                }
              }
            }
          }
          if (foundCast) pubSS << "\n";

          generateStructMethods(helperFile, structFile, structObj, structObj, hasEvents);

          includeSS << "\n";
          includeSS << ss.str();
          includeSS << "\n";

          std::string delegateStr = delegateSS.str();
          includeSS << delegateStr;
          if (delegateStr.length() > 0) {
            includeSS << "\n";
          }

          includeSS << observerSS.str() << "\n";
          includeSS << observerFinalSS.str() << "\n";
          includeSS << pubSS.str();

          includeSS << "\n";

          indentStr = indentStr.substr(0, indentStr.length() - 2);

          includeSS << indentStr << "};\n\n";

          indentStr = indentStr.substr(0, indentStr.length() - 2);

          includeSS << indentStr << "} // namepsace implementation\n\n";

          includeSS << indentStr << "namespace factory_implementation {\n";

          includeSS << "\n";
          includeSS << indentStr << "  struct " << fixStructName(structObj) << " : " << fixStructName(structObj) << "T<" << fixStructName(structObj) << ", implementation::" << fixStructName(structObj) << ">\n";
          includeSS << indentStr << "  {\n";
          includeSS << indentStr << "  };\n";
          includeSS << "\n";

          endingStrs.push_front(indentStr + "} // namespace factory_implementation\n");

          // insert ending namespaces
          {
            for (auto iter = endingStrs.begin(); iter != endingStrs.end(); ++iter) {
              includeSS << (*iter) << "\n";
            }
          }

          includeSS << "#endif //" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "") << " " << ifdefName << "\n";

          cppIncludeSS << "\n";
          cppIncludeSS << cppSS.str();
          cppIncludeSS << "\n";
          cppIncludeSS << "#endif //" << (structObj->hasModifier(Modifier_Special) ? "ifndef" : "") << " " << ifdefName << "\n";

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
                                                           bool hasEvents
                                                           ) noexcept
        {
          if (!structObj) return;

          auto dashedStr = GenerateHelper::getDashedComment(String());

          auto &headerMethodsSS = structFile.headerStructPublicSS_;
          auto &delegateSS = structFile.headerStructEventHandlersSS_;
          auto &observerSS = structFile.headerStructObserverSS_;
          auto &cppSS = structFile.cppBodySS_;
          auto &indentStr = structFile.headerStructIndentStr_;

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedType = (*iter).second;
            if (!relatedType) continue;

            {
              auto subStructObj = relatedType->toStruct();
              if (subStructObj) {
                generateStructMethods(helperFile, structFile, derivedStructObj, subStructObj, hasEvents);
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
            String methodName = method->mName;
            bool isStatic = method->hasModifier(Modifier_Static);
            bool isDefault = method->hasModifier(Modifier_Method_Default);
            bool foundAnotherCtorWithSameNumberOfArguments{ false };

            //if (method->hasModifier(Modifier_AltName)) {
            //  methodName = method->getModifierValue(Modifier_AltName, 0);
            //}

            std::stringstream implSS;

            if (derivedStructObj != structObj) {
              if ((isCtor) || (isEvent) || isStatic) continue;
            }

            if (isCtor) {
              if (!isDefault) {
                foundAnotherCtorWithSameNumberOfArguments = GenerateStructMsidl::hasAnotherCtorWithSameNumberOfArguments(derivedStructObj, method);
                if (foundAnotherCtorWithSameNumberOfArguments) {
                  String altName = method->getModifierValue(Modifier_AltName);
                  if (altName.hasData()) methodName = altName;
                }
              }
            }

            if (firstOutput) {
              headerMethodsSS << indentStr << "// " << structObj->getPathName() << "\n\n";
              firstOutput = false;
            }

            bool hasResult = ("::void" != method->mResult->getPathName());
            if ((isCtor) || (isEvent)) hasResult = false;

            String methodImplIndentStr = "  ";

            includeCppForType(structFile, method->mResult);

            if (foundAnotherCtorWithSameNumberOfArguments) {
              implSS << methodImplIndentStr << "auto result = winrt::make_self< " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << " >(WrapperCreate{});\n";
              implSS << methodImplIndentStr << "result->native_ = wrapper" << derivedStructObj->getPathName() << "::wrapper_create()" << ";\n";
              implSS << methodImplIndentStr << "if (!result->native_) {throw hresult_error(E_POINTER);}\n";
              if (hasEvents) {
                implSS << methodImplIndentStr << "result->observer_ = make_shared<WrapperObserverImpl>(result);\n";
                implSS << methodImplIndentStr << "result->native_->wrapper_installObserver(result->observer_);\n";
              }
            } else if (hasResult) {
              implSS << methodImplIndentStr << getCppWinrtType(helperFile, method->mResult, GO{ GO::Optional(method->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReturnResult()}) << " result {" << getCppWinrtResultTypeInitializer(method->mResult) << "};\n";
            }
            if (method->mThrows.size() > 0) {
              implSS << methodImplIndentStr << "try {\n";
              methodImplIndentStr += "  ";
            }
            if (hasResult) {
              implSS << methodImplIndentStr << "result = ::Internal::Helper::" << getToCppWinrtName(helperFile, method->mResult, GO{}) << "(";
            } else {
              implSS << methodImplIndentStr;
            }

            if (isEvent) {
              implSS << "owner_->" << methodName << "Event_(";
            } else {
              if (isCtor) {
                implSS << (foundAnotherCtorWithSameNumberOfArguments ? "result->" : "") << "native_->wrapper_init_" << getStructInitName(derivedStructObj) << "(";
              } else {
                if (isStatic) {
                  implSS << "wrapper" << method->getPathName() << "(";
                } else {
                  implSS << "native_->" << methodName << "(";
                }
              }
            }

            headerMethodsSS << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);
            for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
            {
              auto arg = (*iterArgs);
              headerMethodsSS << GenerateHelper::getDocumentation(indentStr + "/// ", arg, 80);
            }

            if (isEvent) {
              hasResult = false;
              observerSS << indentStr << "  virtual void " << methodName << "(";

              String delegateNameStr = fixName(method->getModifierValue(Modifier_Method_EventHandler, 0));
              if (!delegateNameStr.hasData()) {
                delegateNameStr = getCppWinrtType(helperFile, derivedStructObj, GO{}) + "_";
                if (method->hasModifier(Modifier_AltName)) {
                  delegateNameStr += fixName(methodName);
                }
                else {
                  delegateNameStr += fixName(methodName);
                }
                delegateNameStr += "Delegate";
              }

              cppSS << dashedStr;
              cppSS << "winrt::event_token " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::" << fixName(methodName) << "(" << delegateNameStr << " const &handler)\n";
              cppSS << "{\n";
              cppSS << methodImplIndentStr << "return " << methodName << "Event_.add(handler);\n";
              cppSS << "}\n";
              cppSS << "\n";

              cppSS << dashedStr;
              cppSS << "void " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::" << fixName(methodName) << "(winrt::event_token const &token)\n";
              cppSS << "{\n";
              cppSS << methodImplIndentStr << methodName << "Event_.remove(token);\n";
              cppSS << "}\n";
              cppSS << "\n";

              cppSS << dashedStr;
              cppSS << "void " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::WrapperObserverImpl::" << methodName << "(";

              delegateSS << indentStr << "winrt::event< " << delegateNameStr << " > " << methodName << "Event_;\n";
              if (method->mArguments.size() > 1) {
                observerSS << "\n";
                observerSS << indentStr << "  ";
                cppSS << "\n";
                cppSS << "  ";
              }

              headerMethodsSS << indentStr << "event_token " << fixName(methodName) << "(" << delegateNameStr << " const &handler);\n";
              headerMethodsSS << indentStr << "void " << fixName(methodName) << "(event_token const& token);\n";
            } else {
              cppSS << dashedStr;

              headerMethodsSS << indentStr << ((foundAnotherCtorWithSameNumberOfArguments || method->hasModifier(Modifier_Static)) ? "static " : "");
              if (!isCtor) {
                includeCppForType(structFile, method->mResult);

                cppSS << getCppWinrtType(helperFile, method->mResult, GO{ GO::Optional(method->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReturnResult()}) << " ";
                headerMethodsSS << getCppWinrtType(helperFile, method->mResult, GO{ GO::Optional(method->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReturnResult()}) << " ";
              }
              if (foundAnotherCtorWithSameNumberOfArguments) {
                cppSS << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeInterface(), GO::MakeReturnResult() }) << " ";
                headerMethodsSS << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeInterface(), GO::MakeReturnResult() }) << " ";
              }
              cppSS << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::" << fixName(methodName) << "(";
              headerMethodsSS << fixName(methodName) << "(";
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
                } else {
                  headerMethodsSS << ",\n";
                  headerMethodsSS << indentStr << "  ";
                }
              }
              firstArg = false;
              implSS << "::Internal::Helper::" << (isEvent ? getToCppWinrtName(helperFile, arg->mType, GO {}) : getFromCppWinrtName(arg->mType)) << "(" << fixArgumentName(arg->mName) << ")";
              includeCppForType(structFile, arg->mType);
              if (isEvent) {
                cppSS << getCppType(arg->mType, GO::Optional(arg->hasModifier(Modifier_Optional))) << " " << fixArgumentName(arg->mName);
                observerSS << getCppType(arg->mType, GO::Optional(arg->hasModifier(Modifier_Optional))) << " " << fixArgumentName(arg->mName);
              } else {
                cppSS << getCppWinrtType(helperFile, arg->mType, GO{ GO::Optional(arg->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReference() }) << " " << fixArgumentName(arg->mName);
                headerMethodsSS << getCppWinrtType(helperFile, arg->mType, GO{ GO::Optional(arg->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReference() }) << " " << fixArgumentName(arg->mName);
              }
            }

            if (hasResult) {
              implSS << ")";
            }
            implSS << ");\n";
            for (auto iterThrows = method->mThrows.begin(); iterThrows != method->mThrows.end(); ++iterThrows) {
              auto throwType = (*iterThrows);
              if (!throwType) continue;
              implSS << "  } catch(const " << getCppType(throwType, GO{}) << " &e) {\n";
              implSS << "    throw ::Internal::Helper::" << getToCppWinrtName(helperFile, throwType, GO {}) << "(e);\n";
            }
            if (method->mThrows.size() > 0) {
              implSS << "  }\n";
            }
            if (foundAnotherCtorWithSameNumberOfArguments) {
              implSS << "  return ToCppWinrtInterface(result);\n";
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
              }
              observerSS << ") override;\n";
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
            if ((isCtor) && (!foundAnotherCtorWithSameNumberOfArguments)) {
              cppSS << "\n : native_(" << "wrapper" << derivedStructObj->getPathName() << "::wrapper_create()" << ")";
              if (hasEvents) {
                cppSS << ",\n";
                cppSS << "   observer_(make_shared<WrapperObserverImpl>(get_strong()))";
              }
            }

            cppSS << "\n";
            cppSS << "{\n";
            if ((!isStatic) && (!foundAnotherCtorWithSameNumberOfArguments)) {
              cppSS << "  if (" << (isEvent ? "nullptr == owner_" : "!native_") << ") {throw hresult_error(E_POINTER);}\n";
            }
            if (((isCtor) && (!foundAnotherCtorWithSameNumberOfArguments)) && (hasEvents)) {
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

            if (derivedStructObj != structObj) {
              if (isStatic) continue;
            }

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

            if (hasGet) {
              headerMethodsSS << indentStr << (isStatic ? "static " : "") << getCppWinrtType(helperFile, property->mType, GO {GO::Optional(property->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReturnResult()}) << " " << fixName(property->mName) << "();\n";
            }
            if (hasSet) {
              headerMethodsSS << indentStr << (isStatic ? "static " : "") << "void " << fixName(property->mName) << "(" << getCppWinrtType(helperFile, property->mType, GO {GO::Optional(property->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReference()}) << " value);\n";
            }

            if (hasGet) {
              cppSS << dashedStr;
              cppSS << getCppWinrtType(helperFile, property->mType, GO {GO::Optional(property->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReturnResult()}) << " " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::" << fixName(property->mName) << "()\n";
              cppSS << "{\n";
              if (!isStatic) {
                cppSS << "  if (!native_) {throw hresult_error(E_POINTER);}\n";
              }
              cppSS << "  return ::Internal::Helper::" << getToCppWinrtName(helperFile, property->mType, GO { }) << "(" << (isStatic ? String("wrapper" + derivedStructObj->getPathName() + "::") : String("native_->"));
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
              cppSS << "void " << getCppWinrtType(helperFile, derivedStructObj, GO{ GO::MakeImplementation() }) << "::" << fixName(property->mName) << "(" << getCppWinrtType(helperFile, property->mType, GO { GO::Optional(property->hasModifier(Modifier_Optional)), GO::MakeInterface(), GO::MakeReference() }) << " value)\n";
              cppSS << "{\n";
              if (!isStatic) {
                cppSS << "  if (!native_) {throw hresult_error(E_POINTER);}\n";
                cppSS << "  native_->";
              } else {
                cppSS << "  wrapper" << derivedStructObj->getPathName() << "::";
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
                                                     ) noexcept
        {
          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          ss << indentStr << "static " << getCppWinrtType(helperFile, enumObj, GO::MakeReturnResult()) << " " << getToCppWinrtName(helperFile, enumObj, GO{}) << "(" << getCppType(enumObj, GO{}) << " value) { return (" << getCppWinrtType(helperFile, enumObj, GO{}) << ")value; }\n";
          ss << indentStr << "static " << getCppType(enumObj, GO{}) << " " << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(helperFile, enumObj, GO{}) << " value) { return (" << getCppType(enumObj, GO{}) << ")value; }\n";

          ss << indentStr << "static " << getCppWinrtType(helperFile, enumObj, GO{ GO::MakeOptional(), GO::MakeReturnResult() }) << " " << getToCppWinrtName(helperFile, enumObj, GO{}) << "(" << getCppType(enumObj, GO::MakeOptional()) << "  &value);\n";
          ss << indentStr << "static " << getCppType(enumObj, GO::MakeOptional()) << " " << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(helperFile, enumObj, GO{ GO::MakeOptional(), GO::MakeReference() }) << " value);\n";
          ss << "\n";

          cppSS << dashedStr;
          cppSS << getCppWinrtType(helperFile, enumObj, GO{ GO::MakeOptional(), GO::MakeReturnResult() }) << " Internal::Helper::" << getToCppWinrtName(helperFile, enumObj, GO{}) << "(" << getCppType(enumObj, GO::MakeOptional()) << "  &value)\n";
          cppSS << "{\n";
          cppSS << "  if (!value.hasValue()) return nullptr;\n";
          cppSS << "  return Windows::Foundation::IReference< " << getCppWinrtType(helperFile, enumObj, GO{}) << " >(" << getToCppWinrtName(helperFile, enumObj, GO{}) << "(value.value()));\n";
          cppSS << "}\n";
          cppSS << "\n";

          cppSS << dashedStr;
          cppSS << getCppType(enumObj, GO::MakeOptional()) << " Internal::Helper::" << getFromCppWinrtName(enumObj) << "(" << getCppWinrtType(helperFile, enumObj, GO{ GO::MakeOptional(), GO::MakeReference() }) << " value)\n";
          cppSS << "{\n";
          cppSS << "  " << getCppType(enumObj, GO::MakeOptional()) << " result;\n";
          cppSS << "  if (!value) return result;\n";
          cppSS << "  result = " << getFromCppWinrtName(enumObj) << "(value.Value());\n";
          cppSS << "  return result;\n";
          cppSS << "}\n";
          cppSS << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::generateForList(
                                                     HelperFile &helperFile,
                                                     StructPtr structObj
                                                     ) noexcept
        {
          //helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_ASSERT_FAIL("unexpected template missing type");
            }

            TypePtr foundType = (*found);
            ss << indentStr << "static Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(helperFile, foundType, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " > " << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::list< " << getCppType(foundType, GO{}) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::list< " << getCppType(foundType, GO{}) << "> > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(helperFile, foundType, GO{ GO::MakeInterface() }) << " > const & values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(helperFile, foundType, GO{ GO::MakeReturnResult(), GO::MakeInterface() }) << " > Internal::Helper::" << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::list< " << getCppType(foundType, GO{}) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  Windows::Foundation::Collections::IVector< " << getCppWinrtType(helperFile, foundType, GO::MakeInterface()) << " > result;\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result.Append(" << getToCppWinrtName(helperFile, foundType, GO{ GO::MakeInterface() }) << "(*iter));\n";
            cppSS << "  }\n";
            cppSS << "  return result.GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::list<" << getCppType(foundType, GO{}) << "> > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IVectorView< " << getCppWinrtType(helperFile, foundType, GO{ GO::MakeInterface() }) << " > const & values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::list< " << getCppType(foundType, GO{}) << " > >();\n";
            cppSS << "  auto result = make_shared< std::list< " << getCppType(foundType, GO{}) << " > >();\n";
            cppSS << "  for (" << getCppWinrtType(helperFile, foundType, GO{ GO::MakeReference(), GO::MakeInterface() }) << " value : values)\n";
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
                                                    ) noexcept
        {
          //helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_ASSERT_FAIL("unexpected template missing type");
            }

            TypePtr keyType = (*found);

            ++found;
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_ASSERT_FAIL("unexpected template missing type");
            }
            TypePtr valueType = (*found);

            ss << indentStr << "static Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO::MakeReturnResult()) << ", " << getCppWinrtType(helperFile, valueType, GO::MakeReturnResult()) << " > " << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::map< " << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::map<" << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << " > > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO{}) << ", " << getCppWinrtType(helperFile, valueType, GO{ GO::MakeInterface() }) << " > const & values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO::MakeReturnResult()) << ", " << getCppWinrtType(helperFile, valueType, GO::MakeReturnResult()) << " > Internal::Helper::" << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::map< " << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  Windows::Foundation::Collections::IMap< " << getCppWinrtType(helperFile, keyType, GO{}) << ", " << getCppWinrtType(helperFile, valueType, GO{}) << " > result;\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result.Insert(" << getToCppWinrtName(helperFile, keyType, GO{ GO::MakeInterface() }) << "((*iter).first), " << getToCppWinrtName(helperFile, valueType, GO{ GO::MakeInterface() }) << "((*iter).second));\n";
            cppSS << "  }\n";
            cppSS << "  return result->GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::map<" << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << " > > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO{}) << ", " << getCppWinrtType(helperFile, valueType, GO{ GO::MakeInterface() }) << " > const & values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::map< " << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << " > >();\n";
            cppSS << "  auto result = make_shared< std::map<" << getCppType(keyType, GO{}) << ", " << getCppType(valueType, GO{}) << "> >();\n";
            cppSS << "  for (Windows::Foundation::Collections::IKeyValuePair< " << getCppWinrtType(helperFile, keyType, GO{}) << ", " << getCppWinrtType(helperFile, valueType, GO{}) << " > const & pair : values)\n";
            cppSS << "  {\n";
            cppSS << "    result[" << getFromCppWinrtName(keyType) << "(pair.Key())] = " << getFromCppWinrtName(valueType) << "(pair.Value());\n";
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
                                                    ) noexcept
        {
          //helperFile.includeHeader("<collection.h>");

          auto &ss = helperFile.headerStructSS_;
          auto &cppSS = helperFile.cppBodySS_;
          auto &indentStr = helperFile.headerIndentStr_;
          auto dashedStr = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStruct = (*iter).second;

            auto found = templatedStruct->mTemplateArguments.begin();
            if (found == templatedStruct->mTemplateArguments.end()) {
              ZS_ASSERT_FAIL("unexpected template missing type");
            }

            TypePtr keyType = (*found);

            ss << indentStr << "static Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO::MakeReturnResult()) << ", Windows::Foundation::IInspectable > " << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::set< " << getCppType(keyType, GO{}) << " > > values);\n";
            ss << indentStr << "static shared_ptr< std::set< " << getCppType(keyType, GO{}) << " > > " << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO{ GO::MakeInterface() }) << ", Windows::Foundation::IInspectable > const & values);\n";
            ss << "\n";

            cppSS << dashedStr;
            cppSS << "Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO::MakeReturnResult()) << ", Windows::Foundation::IInspectable > Internal::Helper::" << getToCppWinrtName(helperFile, templatedStruct, GO{}) << "(shared_ptr< std::set< " << getCppType(keyType, GO{}) << " > > values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return nullptr;\n";
            cppSS << "  Windows::Foundation::Collections::IMap< " << getCppWinrtType(helperFile, keyType, GO{}) << ", Windows::Foundation::IInspectable > result;\n";
            cppSS << "  for (auto iter = values->begin(); iter != values->end(); ++iter)\n";
            cppSS << "  {\n";
            cppSS << "    result.Insert(" << getToCppWinrtName(helperFile, keyType, GO{ GO::MakeInterface() }) << "(*iter), Windows::Foundation::IInspectable {nullptr});\n";
            cppSS << "  }\n";
            cppSS << "  return result.GetView();\n";
            cppSS << "}\n";
            cppSS << "\n";

            cppSS << dashedStr;
            cppSS << "shared_ptr< std::set< " << getCppType(keyType, GO{}) << " > > Internal::Helper::" << getFromCppWinrtName(templatedStruct) << "(Windows::Foundation::Collections::IMapView< " << getCppWinrtType(helperFile, keyType, GO{ GO::MakeInterface() }) << ", Windows::Foundation::IInspectable > const & values)\n";
            cppSS << "{\n";
            cppSS << "  if (!values) return shared_ptr< std::set< " << getCppType(keyType, GO{}) << " > >();\n";
            cppSS << "  auto result = make_shared< std::set<" << getCppType(keyType, GO{}) << "> >();\n";
            cppSS << "  for (Windows::Foundation::Collections::IKeyValuePair< " << getCppWinrtType(helperFile, keyType, GO{ GO::MakeInterface() }) << ", Windows::Foundation::IInspectable > const & pair : values)\n";
            cppSS << "  {\n";
            cppSS << "    result->insert(" << getFromCppWinrtName(keyType) << "(pair.Key()));\n";
            cppSS << "  }\n";
            cppSS << "  return result;\n";
            cppSS << "}\n";
            cppSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructCppWinrt::requiresSpecialConversion(IEventingTypes::PredefinedTypedefs basicType) noexcept
        {
          switch (basicType) {
            case IEventingTypes::PredefinedTypedef_void:      

            case IEventingTypes::PredefinedTypedef_binary:    

            case IEventingTypes::PredefinedTypedef_string:
            case IEventingTypes::PredefinedTypedef_astring:
            case IEventingTypes::PredefinedTypedef_wstring:   return true;
          }
          return false;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getBasicCppWinrtTypeString(
                                                                  IEventingTypes::PredefinedTypedefs basicType,
                                                                  const GenerationOptions &options
                                                                  ) noexcept
        {
          switch (basicType) {
            case IEventingTypes::PredefinedTypedef_void:      return "void";

            case IEventingTypes::PredefinedTypedef_bool:      return makeCppWinrtReferenceAndOptionalIfOptional("bool", options);

            case IEventingTypes::PredefinedTypedef_uchar:     return makeCppWinrtReferenceAndOptionalIfOptional("uint8_t", options);
            case IEventingTypes::PredefinedTypedef_char:
            case IEventingTypes::PredefinedTypedef_schar:     return makeCppWinrtReferenceAndOptionalIfOptional("char16_t", options);
            case IEventingTypes::PredefinedTypedef_ushort:    return makeCppWinrtReferenceAndOptionalIfOptional("uint16_t", options);
            case IEventingTypes::PredefinedTypedef_short:
            case IEventingTypes::PredefinedTypedef_sshort:    return makeCppWinrtReferenceAndOptionalIfOptional("int16_t", options);
            case IEventingTypes::PredefinedTypedef_uint:      return makeCppWinrtReferenceAndOptionalIfOptional("uint32_t", options);
            case IEventingTypes::PredefinedTypedef_int:
            case IEventingTypes::PredefinedTypedef_sint:      return makeCppWinrtReferenceAndOptionalIfOptional("int32_t", options);
            case IEventingTypes::PredefinedTypedef_ulong:     return makeCppWinrtReferenceAndOptionalIfOptional("uint64_t", options);
            case IEventingTypes::PredefinedTypedef_long:
            case IEventingTypes::PredefinedTypedef_slong:     return makeCppWinrtReferenceAndOptionalIfOptional("int64_t", options);
            case IEventingTypes::PredefinedTypedef_ulonglong: return makeCppWinrtReferenceAndOptionalIfOptional("uint64_t", options);
            case IEventingTypes::PredefinedTypedef_longlong:
            case IEventingTypes::PredefinedTypedef_slonglong: return makeCppWinrtReferenceAndOptionalIfOptional("int64_t", options);
            case IEventingTypes::PredefinedTypedef_uint8:     return makeCppWinrtReferenceAndOptionalIfOptional("uint8_t", options);
            case IEventingTypes::PredefinedTypedef_int8:
            case IEventingTypes::PredefinedTypedef_sint8:     return makeCppWinrtReferenceAndOptionalIfOptional("int8_t", options);
            case IEventingTypes::PredefinedTypedef_uint16:    return makeCppWinrtReferenceAndOptionalIfOptional("uint16_t", options);
            case IEventingTypes::PredefinedTypedef_int16:
            case IEventingTypes::PredefinedTypedef_sint16:    return makeCppWinrtReferenceAndOptionalIfOptional("int16_t", options);
            case IEventingTypes::PredefinedTypedef_uint32:    return makeCppWinrtReferenceAndOptionalIfOptional("uint32_t", options);
            case IEventingTypes::PredefinedTypedef_int32:
            case IEventingTypes::PredefinedTypedef_sint32:    return makeCppWinrtReferenceAndOptionalIfOptional("int32_t", options);
            case IEventingTypes::PredefinedTypedef_uint64:    return makeCppWinrtReferenceAndOptionalIfOptional("uint64_t", options);
            case IEventingTypes::PredefinedTypedef_int64:
            case IEventingTypes::PredefinedTypedef_sint64:    return makeCppWinrtReferenceAndOptionalIfOptional("int64_t", options);

            case IEventingTypes::PredefinedTypedef_byte:      return makeCppWinrtReferenceAndOptionalIfOptional("uint8_t", options);
            case IEventingTypes::PredefinedTypedef_word:      return makeCppWinrtReferenceAndOptionalIfOptional("uint16_t", options);
            case IEventingTypes::PredefinedTypedef_dword:     return makeCppWinrtReferenceAndOptionalIfOptional("uint32_t", options);
            case IEventingTypes::PredefinedTypedef_qword:     return makeCppWinrtReferenceAndOptionalIfOptional("uint64_t", options);

            case IEventingTypes::PredefinedTypedef_float:     return makeCppWinrtReferenceAndOptionalIfOptional("float", options);
            case IEventingTypes::PredefinedTypedef_double:
            case IEventingTypes::PredefinedTypedef_ldouble:   return makeCppWinrtReferenceAndOptionalIfOptional("double", options);
            case IEventingTypes::PredefinedTypedef_float32:   return makeCppWinrtReferenceAndOptionalIfOptional("float", options);
            case IEventingTypes::PredefinedTypedef_float64:   return makeCppWinrtReferenceAndOptionalIfOptional("double", options);

            case IEventingTypes::PredefinedTypedef_pointer:   return makeCppWinrtReferenceAndOptionalIfOptional("uintptr_t", options);

            case IEventingTypes::PredefinedTypedef_binary:    return makeCppWinrtReference(options.isReturnResult() ? "com_array<uint8_t>" : "array_view<uint8_t const>", options);
            case IEventingTypes::PredefinedTypedef_size:      return makeCppWinrtReferenceAndOptionalIfOptional("uint64_t", options);

            case IEventingTypes::PredefinedTypedef_string:
            case IEventingTypes::PredefinedTypedef_astring:
            case IEventingTypes::PredefinedTypedef_wstring:   return makeCppWinrtReference("hstring", options);
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getBasicCppWinrtTypeString(
                                                                  BasicTypePtr type,
                                                                  const GenerationOptions &options
                                                                  ) noexcept
        {
          if (!type) return String();

          return getBasicCppWinrtTypeString(type->mBaseType, options);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::makeCppWinrtOptional(const String &value, const GenerationOptions &options) noexcept
        {
          if (!options.getOptional()) return value;
          return "Windows::Foundation::IReference< " + value + " >";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::makeCppWinrtReference(const String &value, const GenerationOptions &options) noexcept
        {
          if (!options.getReference()) return value;
          return value + " const &";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::makeCppWinrtReferenceAndOptional(const String &value, const GenerationOptions &options) noexcept
        {
          return makeCppWinrtReference(makeCppWinrtOptional(value, options), options);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::makeCppWinrtReferenceAndOptionalIfOptional(const String &value, const GenerationOptions &options) noexcept
        {
          if (!options.getOptional()) return value;
          if (!options.getReference()) return makeCppWinrtOptional(value, options);
          return makeCppWinrtReference(makeCppWinrtOptional(value, options), options);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppType(
                                                  TypePtr type,
                                                  const GenerationOptions &options
                                                  ) noexcept
        {
          return GenerateStructHeader::getWrapperTypeString(options.isOptional(), type);
        }
        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtType(
                                                       const HelperFile &helperFile,
                                                       TypePtr type,
                                                       const GenerationOptions &options
                                                       ) noexcept
        {
          return getCppWinrtType(type, *helperFile.structsNeedingInterface_, options);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtType(
                                                       TypePtr type,
                                                       const StructSet &structsNeedingInterface,
                                                       const GenerationOptions &options
                                                       ) noexcept
        {
          if (!type) return String();

          type = type->getOriginalType();

          {
            auto typedefType = type->toTypedefType();
            if (typedefType) {
              ZS_ASSERT_FAIL("typedef failed to resolve to original type");
            }
          }

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return getBasicCppWinrtTypeString(basicType, options);
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              if (structType->mGenerics.size() > 0) return String();
              if (structType->hasModifier(Modifier_Special)) {
                String specialName = structType->getPathName();
                if ("::zs::Any" == specialName) return makeCppWinrtReferenceAndOptional("Windows::Foundation::IInspectable", options.getAmmended(GO::MakeNotOptional()));
                if ("::zs::Promise" == specialName) return makeCppWinrtReferenceAndOptional("Windows::Foundation::IAsyncOperation< Windows::Foundation::IInspectable >", options.getAmmended(GO::MakeNotOptional()));
                if ("::zs::exceptions::Exception" == specialName) return makeCppWinrtReferenceAndOptional("hresult_error", options.getAmmended(GO::MakeNotOptional())); // E_FAIL
                if ("::zs::exceptions::InvalidArgument" == specialName) return makeCppWinrtReferenceAndOptional("hresult_invalid_argument", options.getAmmended(GO::MakeNotOptional()));
                if ("::zs::exceptions::BadState" == specialName) return makeCppWinrtReferenceAndOptional("hresult_error", options.getAmmended(GO::MakeNotOptional()));   // E_NOT_VALID_STATE
                if ("::zs::exceptions::NotImplemented" == specialName) return makeCppWinrtReferenceAndOptional("hresult_not_implemented", options.getAmmended(GO::MakeNotOptional()));
                if ("::zs::exceptions::NotSupported" == specialName) return makeCppWinrtReferenceAndOptional("hresult_error", options.getAmmended(GO::MakeNotOptional()));  // CO_E_NOT_SUPPORTED
                if ("::zs::exceptions::UnexpectedError" == specialName) return makeCppWinrtReferenceAndOptional("hresult_error", options.getAmmended(GO::MakeNotOptional())); //E_UNEXPECTED
                if ("::zs::Time" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::DateTime", options);
                if ("::zs::Milliseconds" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Microseconds" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Nanoseconds" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Seconds" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Minutes" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Hours" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
                if ("::zs::Days" == specialName) return makeCppWinrtReferenceAndOptionalIfOptional("Windows::Foundation::TimeSpan", options);
              }

              String namePath = fixNamePathNoPrefix(structType);

              GO newOptions = options;

              if (options.isInterface()) {
                if (structsNeedingInterface.end() == structsNeedingInterface.find(structType)) newOptions = newOptions.getAmmended(GO::MakeNotInterface());
              }

              String::size_type index = namePath.rfind("::");
              if (String::npos != index) {
                if (newOptions.isInterface()) {
                  namePath.insert(index + strlen("::"), "I");
                } else if (newOptions.isImplementation()) {
                  namePath.insert(index, "::implementation");
                  if (newOptions.isComPtr()) {
                    namePath = "winrt::com_ptr< " + namePath + " >";
                  }
                }
              }

              newOptions = options.getAmmended(GO::MakeNotOptional()).getAmmended(GO::MakeNotInterface()).getAmmended(GO::MakeNotImplementation());
              return makeCppWinrtReference(namePath, newOptions);
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return makeCppWinrtReferenceAndOptionalIfOptional(fixNamePathNoPrefix(enumType), options);
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
                      specialTemplatePost = ", Windows::Foundation::IInspectable";
                    }
                    if ("::std::list" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IVectorView< ";
                    if ("::std::map" == specialName) templatedTypeStr = "Windows::Foundation::Collections::IMapView< ";
                    if ("::zs::PromiseWith" == specialName) return makeCppWinrtReferenceAndOptional("Windows::Foundation::IAsyncOperation< Windows::Foundation::IInspectable >", options.getAmmended(GO::MakeNotOptional()));
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
                String typeStr = getCppWinrtType(templateArg, structsNeedingInterface, GO{ GO::Optional(templateArg->hasModifier(Modifier_Optional)), GO::MakeInterface() });
                if ("void" == typeStr) continue;
                if (!first) templatedTypeStr += ", ";
                templatedTypeStr += typeStr;
                first = false;
              }
              templatedTypeStr += specialTemplatePost + " >";
              return makeCppWinrtReferenceAndOptional(templatedTypeStr, options.getAmmended(GO::MakeNotOptional()));
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtResultTypeInitializer(TypePtr type) noexcept
        {
          if (!type) return String();

          String nullptrStr("nullptr");

          type = type->getOriginalType();

          {
            auto typedefType = type->toTypedefType();
            if (typedefType) {
              ZS_ASSERT_FAIL("typedef failed to resolve to original type");
            }
          }

          {
            auto basicType = type->toBasicType();
            if (basicType) return String();
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              if (structType->mGenerics.size() > 0) return String();
              if (structType->hasModifier(Modifier_Special)) {
                String specialName = structType->getPathName();
                if ("::zs::Any" == specialName) return nullptrStr;
                if ("::zs::Promise" == specialName) return nullptrStr;
                if ("::zs::exceptions::Exception" == specialName) return String();
                if ("::zs::exceptions::InvalidArgument" == specialName) return String();
                if ("::zs::exceptions::BadState" == specialName) return String();
                if ("::zs::exceptions::NotImplemented" == specialName) return String();
                if ("::zs::exceptions::NotSupported" == specialName) return String();
                if ("::zs::exceptions::UnexpectedError" == specialName) return String();
                if ("::zs::Time" == specialName) return String();
                if ("::zs::Milliseconds" == specialName) return String();
                if ("::zs::Microseconds" == specialName) return String();
                if ("::zs::Nanoseconds" == specialName) return String();
                if ("::zs::Seconds" == specialName) return String();
                if ("::zs::Minutes" == specialName) return String();
                if ("::zs::Hours" == specialName) return String();
                if ("::zs::Days" == specialName) return String();
              }

              return nullptrStr;
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) return String();
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) return nullptrStr;
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getCppWinrtAttributes(const StringList &attributes) noexcept
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
                                                                 ) noexcept
        {
          if (attributes.size() < 1) return String();
          return linePrefix + getCppWinrtAttributes(attributes) + "\n";
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getToFromCppWinrtName(
                                                             TypePtr type,
                                                             const StructSet &structsNeedingInterface,
                                                             const GenerationOptions &options,
                                                             const String &prefixName,
                                                             const String &prefixNameIfImpl,
                                                             const String &prefixIfInterface
                                                             ) noexcept
        {
          if (!type) return prefixName;

          String simpleResult = prefixName;
          simpleResult.trim("_");

          type = type->getOriginalType();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              String result = GenerateHelper::getConverstionNameString(basicType);
              return prefixName + fixName(result);
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              String specialName = type->getPathName();

              if ("::zs::Any" == specialName) return simpleResult;
              if ("::zs::Promise" == specialName) return simpleResult;
              if ("::zs::exceptions::Exception" == specialName) return simpleResult;
              if ("::zs::exceptions::InvalidArgument" == specialName) return simpleResult;
              if ("::zs::exceptions::BadState" == specialName) return simpleResult;
              if ("::zs::exceptions::NotImplemented" == specialName) return simpleResult;
              if ("::zs::exceptions::NotSupported" == specialName) return simpleResult;
              if ("::zs::exceptions::UnexpectedError" == specialName) return simpleResult;
              if ("::zs::Time" == specialName) return simpleResult;
              if ("::zs::Milliseconds" == specialName) return prefixName + "Milliseconds";
              if ("::zs::Microseconds" == specialName) return prefixName + "Microseconds";
              if ("::zs::Nanoseconds" == specialName) return prefixName + "Nanoseconds";
              if ("::zs::Seconds" == specialName) return prefixName + "Seconds";
              if ("::zs::Minutes" == specialName) return prefixName + "Minutes";
              if ("::zs::Hours" == specialName) return prefixName + "Hours";
              if ("::zs::Days" == specialName) return prefixName + "Days";

              if ("::zs::PromiseWith" == specialName) return prefixName + "PromiseWith";
              if ("::std::set" == specialName) return prefixName + "Set";
              if ("::std::list" == specialName) return prefixName + "List";
              if ("::std::map" == specialName) return prefixName + "Map";

              String namePath = fixNamePath(structType);
              namePath.replaceAll("::", "_");
              namePath.trim("_");

              bool hasInterface = (structsNeedingInterface.end() != structsNeedingInterface.find(structType));

              if ((hasInterface) && (options.isInterface())) return prefixIfInterface + namePath;
              return (options.isImplementation() ? prefixNameIfImpl : prefixName) + namePath;
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              String result;
              auto parent = templatedType->getParent();
              if (parent) {
                result = getToFromCppWinrtName(parent->toType(), structsNeedingInterface, options.getInterface(), prefixName, prefixNameIfImpl, prefixIfInterface);
              }

              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                String typeResult = getToFromCppWinrtName(*iter, structsNeedingInterface, options.getInterface(), prefixName, prefixNameIfImpl, prefixIfInterface);
                if (typeResult.isEmpty()) continue;
                if (result.hasData()) result += "_";
                result += typeResult;
              }
              return result;
            }
          }
          return simpleResult;
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getToCppWinrtName(
                                                         TypePtr type,
                                                         const StructSet &structsNeedingInterface,
                                                         const GenerationOptions &options
                                                         ) noexcept
        {
          if (!type) return "ToCppWinrt_";
          return getToFromCppWinrtName(type, structsNeedingInterface, options, "ToCppWinrt_", "ToCppWinrtImpl_", "ToCppWinrtInterface_");
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getToCppWinrtName(
                                                         const HelperFile &helperFile,
                                                         TypePtr type,
                                                         const GenerationOptions &options
                                                         ) noexcept
        {
          return getToCppWinrtName(type, *helperFile.structsNeedingInterface_, options);
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::getFromCppWinrtName(TypePtr type) noexcept
        {
          if (!type) return "FromCppWinrt_";
          return getToFromCppWinrtName(type, StructSet{}, GO::MakeCppBaseType(), "FromCppWinrt_", "FromCppWinrt_", "FromCppWinrt_");
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::IncludeProcessedInfo::IncludeProcessedInfo() noexcept
        {
        }

        //---------------------------------------------------------------------
        GenerateStructCppWinrt::IncludeProcessedInfo::~IncludeProcessedInfo() noexcept
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeCppForType(
                                                       StructFile &structFile,
                                                       TypePtr type
                                                       ) noexcept
        {
          IncludeProcessedInfo info;
          includeCppForType(info, structFile, type);
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::includeCppForType(
                                                       IncludeProcessedInfo &processed,
                                                       StructFile &structFile,
                                                       TypePtr type
                                                       ) noexcept
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
                                                                   ) noexcept
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
                                                                   ) noexcept
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
        //
        // GenerateStructHeader::IIDLCompilerTarget
        //

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::targetKeyword() noexcept
        {
          return String("cppwinrt");
        }

        //---------------------------------------------------------------------
        String GenerateStructCppWinrt::targetKeywordHelp() noexcept
        {
          return String("Generate CPP/WINRT UWP wrapper");
        }

        //---------------------------------------------------------------------
        void GenerateStructCppWinrt::targetOutput(
                                                  const String &inPathStr,
                                                  const ICompilerTypes::Config &config
                                                  ) noexcept(false)
        {
          typedef std::stack<NamespacePtr> NamespaceStack;

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

          GenerateStructMsidl::calculateRelations(project->mGlobal, helperFile.derives_);

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
            ss << "#include \"pch.h\"\n";
            ss << "#include \"cppwinrt_Helpers.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppBodySS_;
            GenerateStructHeader::generateUsingTypes(ss, "");
            ss << "using namespace winrt;\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.headerStructSS_;
            auto &finalSS = helperFile.headerFinalSS_;
            auto &indentStr = helperFile.headerIndentStr_;

            ss << "namespace Internal {\n";
            indentStr += "  ";
            GenerateStructHeader::generateUsingTypes(ss, indentStr);
            ss << indentStr << "using namespace winrt;\n";
            ss << "\n";
            ss << indentStr << "struct Helper {\n";
            finalSS << indentStr << "};\n\n";
            finalSS << "} // namespace Internal\n";

            indentStr += "  ";

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
