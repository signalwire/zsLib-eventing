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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructMsidl.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateHelper.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>
#include <zsLib/Numeric.h>

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
        // GenerateStructMsidl::IDLFile
        //

        //---------------------------------------------------------------------
        GenerateStructMsidl::IDLFile::IDLFile() noexcept :
          structsNeedingInterface_(make_shared<StructSet>()),
          derives_(make_shared<NamePathStructSetMap>())
        {
        }

        //---------------------------------------------------------------------
        GenerateStructMsidl::IDLFile::~IDLFile() noexcept
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::IDLFile::import(const String &fileName) noexcept
        {
          auto &ss = importSS_;

          if (alreadyImported_.end() != alreadyImported_.find(fileName)) return;
          alreadyImported_.insert(fileName);

          if (0 == (fileName.compareNoCase("windows.foundation.idl"))) {
            ss << "//import \"" << fileName << "\";\n";
            return;
          }

          ss << "import \"" << fileName << "\";\n";
        }

        //---------------------------------------------------------------------
        bool GenerateStructMsidl::IDLFile::isStructNeedingInterface(StructPtr structObj) const noexcept
        {
          return (structsNeedingInterface_->end() != structsNeedingInterface_->find(structObj));
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::IDLFile::finalize(std::stringstream &ss) const noexcept
        {
          ss << importSS_.str();
          ss << "\n";
          ss << enumSS_.str();
          ss << "\n";
          ss << bodySS_.str();
          ss << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // GenerateStructMsidl
        //


        //---------------------------------------------------------------------
        GenerateStructMsidl::GenerateStructMsidl() noexcept : IDLCompiler(Noop{})
        {
        }

        //---------------------------------------------------------------------
        GenerateStructMsidlPtr GenerateStructMsidl::create() noexcept
        {
          return make_shared<GenerateStructMsidl>();
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::fixName(const String &originalName) noexcept
        {
          if (originalName.isEmpty()) return String();
          String firstLetter = originalName.substr(0, 1);
          String remaining = originalName.substr(1);
          firstLetter.toUpper();
          return firstLetter + remaining;
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::fixName(ContextPtr contextPtr) noexcept
        {
          auto parent = contextPtr->getParent();
          if (!parent) return fixName(contextPtr->mName);

          String name;

          auto parentStruct = parent->toStruct();
          while (parentStruct)
          {
            if (name.hasData()) {
              name = "_" + name;
            }
            name = fixName(parentStruct->mName) + name;
            parent = contextPtr->getParent();
            if (!parent) break;
            parentStruct = parent->toStruct();
          }

          if (name.hasData()) { 
            return name + "_" + fixName(contextPtr->mName);
          }
          return fixName(contextPtr->mName);
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::fixNamePath(
                                                ContextPtr context,
                                                const GenerationOptions &options
                                                ) noexcept
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

            bool isFinal = false;
            {
              auto check = iter;
              ++check;
              isFinal = check == parents.end();
            }

            auto structObj = context->toStruct();
            if ((structObj) && (lastWasStruct)) {
              path += "_";
            }
            else {
              path += "::";
            }

            if ((structObj) && (options.interface_) && (isFinal)) {
              path += "I";
            }

            path += fixName(context->mName);
            lastWasStruct = ((bool)structObj);
          }

          path.replaceAll("::", ".");
          if (path.substr(0, 1) == ".") {
            path = path.substr(1);
          }
          return path;
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::toIdlType(BasicTypePtr basicType) noexcept
        {
          switch (basicType->mBaseType) {
            case PredefinedTypedef_void:        return "void";

            case PredefinedTypedef_bool:        return "Boolean";

            case PredefinedTypedef_uchar:       return "UInt8";
            case PredefinedTypedef_char:        return "Char";
            case PredefinedTypedef_schar:       return "Char";
            case PredefinedTypedef_ushort:      return "UInt16";
            case PredefinedTypedef_short:
            case PredefinedTypedef_sshort:      return "Int16";
            case PredefinedTypedef_uint:        return "UInt32";
            case PredefinedTypedef_int:
            case PredefinedTypedef_sint:        return "Int32";
            case PredefinedTypedef_ulong:       return "UInt64";
            case PredefinedTypedef_long:
            case PredefinedTypedef_slong:       return "Int64";
            case PredefinedTypedef_ulonglong:   return "UInt64";
            case PredefinedTypedef_longlong:
            case PredefinedTypedef_slonglong:   return "Int64";
            case PredefinedTypedef_uint8:       return "UInt8";
            case PredefinedTypedef_int8:
            case PredefinedTypedef_sint8:       return "Int8";
            case PredefinedTypedef_uint16:      return "UInt16";
            case PredefinedTypedef_int16:
            case PredefinedTypedef_sint16:      return "Int16";
            case PredefinedTypedef_uint32:      return "UInt32";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "Int32";
            case PredefinedTypedef_uint64:      return "UInt64";
            case PredefinedTypedef_int64:
            case PredefinedTypedef_sint64:      return "UInt64";

            case PredefinedTypedef_byte:        return "UInt8";
            case PredefinedTypedef_word:        return "UInt16";
            case PredefinedTypedef_dword:       return "UInt32";
            case PredefinedTypedef_qword:       return "UInt64";

            case PredefinedTypedef_float:       return "Single";
            case PredefinedTypedef_double:      return "Double";
            case PredefinedTypedef_ldouble:     return "Double";
            case PredefinedTypedef_float32:     return "Single";
            case PredefinedTypedef_float64:     return "Double";

            case PredefinedTypedef_pointer:     return "UInt64";

            case PredefinedTypedef_binary:      return "UInt8[]";
            case PredefinedTypedef_size:        return "UInt64";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:
            case PredefinedTypedef_wstring:     return "String";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::toIdlType(
                                              IDLFile &idl,
                                              const GenerationOptions &options,
                                              BasicTypePtr basicType
                                              ) noexcept
        {
          String result = toIdlType(basicType);

          if (options.optional_) {
            switch (basicType->mBaseType)
            {
              case PredefinedTypedef_void:      
              case PredefinedTypedef_binary:    
              case PredefinedTypedef_string:
              case PredefinedTypedef_astring:
              case PredefinedTypedef_wstring:     return result;
              default: break;
            }
            idl.import("windows.foundation.idl");
            result = "Windows.Foundation.IReference< " + result + " >";
          }
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::toIdlSimpleType(
                                                    IDLFile &idl,
                                                    const GenerationOptions &options,
                                                    TypePtr type
                                                    ) noexcept
        {
          String result = fixNamePath(type, options);
          if (options.optional_) {
            idl.import("windows.foundation.idl");
            result = "Windows.Foundation.IReference< " + result + " >";
          }
          return result;
        }
        
        //---------------------------------------------------------------------
        String GenerateStructMsidl::toIdlSimpleType(
                                                    IDLFile &idl,
                                                    const GenerationOptions &options,
                                                    const String &typeName
                                                    ) noexcept
        {
          String result = typeName;
          if (options.optional_) {
            idl.import("windows.foundation.idl");
            result = "Windows.Foundation.IReference< " + result + " >";
          }
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::toIdlType(
                                              IDLFile &idl,
                                              const GenerationOptions &options,
                                              TypePtr type
                                              ) noexcept
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return toIdlType(idl, options, basicType);
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) return toIdlSimpleType(idl, options, type);
          }
          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) {
                String specialName = structType->getPathName();
                if ("::zs::Any" == specialName) return toIdlSimpleType(idl, options, "Object");
                if ("::zs::Promise" == specialName) {
                  idl.import("windows.foundation.idl");
                  return "Windows.Foundation.IAsyncOperation< Object >";  // should be IAsyncAction but need a way to return promise rejection reasons
                }
                if ("::zs::exceptions::Exception" == specialName) return "Object";
                if ("::zs::exceptions::InvalidArgument" == specialName) return "Object";
                if ("::zs::exceptions::BadState" == specialName) return "Object";
                if ("::zs::exceptions::NotImplemented" == specialName) return "Object";
                if ("::zs::exceptions::NotSupported" == specialName) return "Object";
                if ("::zs::exceptions::UnexpectedError" == specialName) return "Object";

                idl.import("windows.foundation.idl");
                if ("::zs::Time" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.DateTime");
                if ("::zs::Milliseconds" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Microseconds" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Nanoseconds" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Seconds" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Minutes" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Hours" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::zs::Days" == specialName) return toIdlSimpleType(idl, options, "Windows.Foundation.TimeSpan");
                if ("::std::set" == specialName) return String();
                if ("::std::list" == specialName) return String();
                if ("::std::map" == specialName) return String();
              }
              if (idl.isStructNeedingInterface(structType)) return toIdlSimpleType(idl, GenerationOptions{ GenerationOptions::Interface(options.interface_) }, type);
              return toIdlSimpleType(idl, GenerationOptions{ }, type);
            }
          }
          {
            auto templatedStruct = type->toTemplatedStructType();
            if (templatedStruct) {
              if (GenerateHelper::isBuiltInType(templatedStruct)) {
                auto parent = templatedStruct->getParent();
                if (!parent) return String();

                auto parentStruct = parent->toStruct();
                if (!parentStruct) return String();

                String specialName = parentStruct->getPathName();

                String name = fixNamePath(parentStruct, options);

                auto maxParams = templatedStruct->mTemplateArguments.size();

                if ("::zs::PromiseWith" == specialName) {
                  idl.import("windows.foundation.idl");
                  name = "Windows.Foundation.IAsyncOperation";
                  maxParams = 1;
                  return name + "< Object >"; // should use actual reason but cannot return exception directly
                }
                if ("::zs::PromiseRejectionReason" == specialName) return "Object";

                bool specialSet = false;
                if ("::std::set" == specialName) {
                  idl.import("windows.foundation.idl");
                  name = "Windows.Foundation.Collections.IMapView";
                  specialSet = true;
                }
                if ("::std::list" == specialName) {
                  idl.import("windows.foundation.idl");
                  name = "Windows.Foundation.Collections.IVectorView";
                }
                if ("::std::map" == specialName) {
                  idl.import("windows.foundation.idl");
                  name = "Windows.Foundation.Collections.IMapView";
                }

                String result = name + "< ";

                bool first {true};
                decltype(maxParams) loop = 0;
                for (auto iter = templatedStruct->mTemplateArguments.begin(); iter != templatedStruct->mTemplateArguments.end() && loop < maxParams; ++iter, ++loop)
                {
                  auto &templateArg = (*iter);
                  if (!first) result += ", ";
                  result += toIdlType(idl, GenerationOptions{ GenerationOptions::Optional(templateArg->hasModifier(Modifier_Optional)), GenerationOptions::Interface(true) }, templateArg);
                  first = false;
                }
                if (specialSet) {
                  result += ", Object";
                }

                result += " >";
                return result;
              }
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::scanNamespaceForStructsNeedingToBeInterfaces(
                                                                               StructSet &needingInterfaceSet,
                                                                               NamespacePtr namespaceObj,
                                                                               int scanPass
                                                                               ) noexcept
        {
          if (namespaceObj->hasModifier(Modifier_Special)) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            scanNamespaceForStructsNeedingToBeInterfaces(needingInterfaceSet, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto subStructObj = (*iter).second;
            scanStructForStructsNeedingToBeInterfaces(needingInterfaceSet, subStructObj, scanPass);
          }

          if (0 == scanPass) {
            if (namespaceObj->mName.isEmpty()) scanNamespaceForStructsNeedingToBeInterfaces(needingInterfaceSet, namespaceObj, 1);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::scanStructForStructsNeedingToBeInterfaces(
                                                                            StructSet &needingInterfaceSet,
                                                                            StructPtr structObj,
                                                                            int scanPass
                                                                            ) noexcept
        {
          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto subStructObj = (*iter).second;
            scanStructForStructsNeedingToBeInterfaces(needingInterfaceSet, subStructObj, scanPass);
          }

          if (0 == scanPass) {
            if (!structObj->hasModifier(Modifier_Struct_Dictionary)) {
              markAllRelatedStructsAsNeedingInterface(needingInterfaceSet, structObj);
            }

            if (structObj->mIsARelationships.size() > 0) {
              markAllRelatedStructsAsNeedingInterface(needingInterfaceSet, structObj);
              return;
            }
          }

          if (1 == scanPass) {
            StructSet alreadyScanned;
            if (doesAnyRelationHaveInterface(needingInterfaceSet, structObj, alreadyScanned)) {
              markAllRelatedStructsAsNeedingInterface(needingInterfaceSet, structObj);
            }
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructMsidl::doesAnyRelationHaveInterface(
                                                               const StructSet &needingInterfaceSet,
                                                               StructPtr structObj,
                                                               StructSet &alreadyScanned
                                                               ) noexcept
        {
          if (alreadyScanned.find(structObj) != alreadyScanned.end()) return false;
          alreadyScanned.insert(structObj);

          if (needingInterfaceSet.find(structObj) != needingInterfaceSet.end()) return true;

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedObj = (*iter).second;
            auto relatedStructObj = relatedObj->toStruct();
            if (!relatedStructObj) continue;

            if (doesAnyRelationHaveInterface(needingInterfaceSet, relatedStructObj, alreadyScanned)) return true;
          }
          return false;
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::markAllRelatedStructsAsNeedingInterface(
                                                                          StructSet &needingInterfaceSet,
                                                                          StructPtr structObj
                                                                          ) noexcept
        {
          if (needingInterfaceSet.find(structObj) != needingInterfaceSet.end()) return;
          needingInterfaceSet.insert(structObj);

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedObj = (*iter).second;
            auto relatedStructObj = relatedObj->toStruct();
            if (!relatedStructObj) continue;

            markAllRelatedStructsAsNeedingInterface(needingInterfaceSet, relatedStructObj);
          }
        }

        
        //---------------------------------------------------------------------
        void GenerateStructMsidl::calculateRelations(
                                                     NamespacePtr namespaceObj,
                                                     NamePathStructSetMap &ioDerivesInfo
                                                     ) noexcept
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
        void GenerateStructMsidl::calculateRelations(
                                                     StructPtr structObj,
                                                     NamePathStructSetMap &ioDerivesInfo
                                                     ) noexcept
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
        void GenerateStructMsidl::insertInto(
                                             StructPtr structObj,
                                             const NamePath &namePath,
                                             NamePathStructSetMap &ioDerivesInfo
                                             ) noexcept
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
        bool GenerateStructMsidl::hasAnotherCtorWithSameNumberOfArguments(
                                                                          StructPtr structObj,
                                                                          MethodPtr currentCtor
                                                                          ) noexcept
        {
          if (!structObj) return false;
          if (!currentCtor) return false;

          // scan for other methods with the same number of arguments
          for (auto iterCheck = structObj->mMethods.begin(); iterCheck != structObj->mMethods.end(); ++iterCheck) {
            auto &methodCheck = (*iterCheck);
            if (methodCheck == currentCtor) continue;
            if (!methodCheck->hasModifier(Modifier_Method_Ctor)) continue;
            if (methodCheck->hasModifier(Modifier_Method_Delete)) continue;

            if (currentCtor->mArguments.size() != methodCheck->mArguments.size()) continue;
            return true;
          }
          return false;
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::processNamespace(
                                                   IDLFile &forwardsIdl,
                                                   IDLFile &outputIdl,
                                                   NamespacePtr namespaceObj
                                                   ) noexcept
        {
          if (namespaceObj->hasModifier(Modifier_Special)) return;

          String initialIndentStr = outputIdl.indent_;
          String &indentStr = outputIdl.indent_;

          if (namespaceObj->mName.hasData()) {
            forwardsIdl.bodySS_ << GenerateHelper::getDocumentation(indentStr + "/// ", namespaceObj, 80);
            forwardsIdl.bodySS_ << indentStr << "namespace " << fixName(namespaceObj->mName) << "\n";
            forwardsIdl.bodySS_ << indentStr << "{\n";
            outputIdl.enumSS_ << indentStr << "namespace " << fixName(namespaceObj->mName) << "\n";
            outputIdl.enumSS_ << indentStr << "{\n";
            outputIdl.bodySS_ << GenerateHelper::getDocumentation(indentStr + "/// ", namespaceObj, 80);
            outputIdl.bodySS_ << indentStr << "namespace " << fixName(namespaceObj->mName) << "\n";
            outputIdl.bodySS_ << indentStr << "{\n";
            indentStr += "    ";
            forwardsIdl.indent_ = indentStr;
          }

          bool firstEnumItem = true;
          bool firstForwardItem = true;
          bool firstBodyItem = true;
          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            if (!firstEnumItem) {
              outputIdl.enumSS_ << "\n";
            }
            if (!firstForwardItem) {
              forwardsIdl.bodySS_ << "\n";
            }
            if (!firstBodyItem) {
              outputIdl.bodySS_ << "\n";
            }
            processNamespace(forwardsIdl, outputIdl, subNamespaceObj);
            firstEnumItem = false;
            firstForwardItem = false;
            firstBodyItem = false;
          }

          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter)
          {
            auto subEnumObj = (*iter).second;
            if (!firstEnumItem) {
              outputIdl.enumSS_ << "\n";
            }
            processEnum(forwardsIdl, outputIdl, subEnumObj);
            firstEnumItem = false;
          }

          bool firstStruct = true;
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            if ((!firstForwardItem) && (firstStruct)) {
              forwardsIdl.bodySS_ << "\n";
            }
            auto subStructObj = (*iter).second;
            processStruct(forwardsIdl, outputIdl, subStructObj);
            firstStruct = false;
            firstForwardItem = false;
          }

          if (namespaceObj->mName.hasData()) {
            indentStr = initialIndentStr;
            forwardsIdl.indent_ = initialIndentStr;
            forwardsIdl.bodySS_ << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n";
            outputIdl.enumSS_ << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n";
            outputIdl.bodySS_ << indentStr << "} // namespace " << fixName(namespaceObj->mName) << "\n\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::fixHiddenAttribute(
                                                     IDLFile &idl,
                                                     ContextPtr context,
                                                     const String &indentStr,
                                                     std::stringstream &ss
                                                     ) noexcept
        {
          bool foundWebHidden {false};

          if (context->hasModifier(Modifier_Platform)) {
            StringList values;
            context->getModifierValues(Modifier_Platform, values);
            for (auto iter = values.begin(); iter != values.end(); ++iter) {
              auto &value = (*iter);
              if ("webhidden" == value) foundWebHidden = true;
            }
          }
          if (!foundWebHidden) return;

          idl.import("windows.foundation.customattributes.idl");
          ss << indentStr << "[webhosthidden]\n";
        }


        //---------------------------------------------------------------------
        void GenerateStructMsidl::fixDeprecatedAttribute(
                                                         IDLFile &idl,
                                                         ContextPtr context,
                                                         const String &indentStr,
                                                         std::stringstream &ss
                                                         ) noexcept
        {
          if (!context->hasModifier(Modifier_Obsolete)) return;

          idl.import("windows.foundation.idl");
          ss << indentStr << "[deprecated(" << context->getModifierValue(Modifier_Obsolete, 0) << ", deprecate, Windows.Foundation.UniversalApiContract, 1.0)]\n";
        }

        
        //---------------------------------------------------------------------
        void GenerateStructMsidl::fixMethodNameAttribute(
                                                         IDLFile &idl,
                                                         ContextPtr context,
                                                         const String &indentStr,
                                                         std::stringstream &ss
                                                         ) noexcept
        {
          if (!context->hasModifier(Modifier_AltName)) return;

          String methodName = context->getModifierValue(Modifier_AltName);

          idl.import("windows.foundation.idl");
          ss << indentStr << "[method_name(\"" << fixName(methodName) << "\")]\n";
        }
        
        //---------------------------------------------------------------------
        void GenerateStructMsidl::fixDefaultAttribute(
                                                      IDLFile &idl,
                                                      ContextPtr context,
                                                      const String &indentStr,
                                                      std::stringstream &ss
                                                      ) noexcept
        {
          if (!context->hasModifier(Modifier_Method_Default)) return;

          idl.import("windows.foundation.idl");
          ss << indentStr << "[default_overload]\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::processStruct(
                                                IDLFile &forwardsIdl,
                                                IDLFile &outputIdl,
                                                StructPtr structObj
                                                ) noexcept
        {
          if (structObj->mTemplatedStructs.size() > 0) return;

          NamespaceList namespaces;

          bool isSpecial = structObj->hasModifier(Modifier_Special);
          if (isSpecial) {
            if (GenerateHelper::isBuiltInType(structObj)) return;

            auto parent = structObj->getParent();
            while (parent) {
              auto namespaceObj = parent->toNamespace();
              if (namespaceObj) {
                if (namespaceObj->mName.hasData()) {
                  namespaces.push_front(namespaceObj);
                }
              }
              parent = parent->getParent();
            }
          }

          std::stringstream forwardSS;

          std::stringstream preISS;
          std::stringstream iss;

          std::stringstream delegatesSS;
          std::stringstream preSS;
          std::stringstream ss;

          String initialIndentStr = outputIdl.indent_;
          String &indentStr = outputIdl.indent_;

          bool requiresInterface = outputIdl.isStructNeedingInterface(structObj);

          IDLFile templateIDL;
          templateIDL.global_ = outputIdl.global_;
          templateIDL.structsNeedingInterface_ = outputIdl.structsNeedingInterface_;  // reference same base class set
          templateIDL.derives_ = outputIdl.derives_;  // reference same base class set
          templateIDL.indent_ = outputIdl.indent_;
          templateIDL.fileName_ = UseHelper::fixRelativeFilePath(outputIdl.fileName_, fixNamePath(structObj, GenerationOptions{}) + ".idl.template");

          IDLFile &useIDL = isSpecial ? templateIDL : outputIdl;

          preISS << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);
          preSS << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);

          fixHiddenAttribute(useIDL, structObj, indentStr, iss);
          fixDeprecatedAttribute(useIDL, structObj, indentStr, iss);

          fixHiddenAttribute(useIDL, structObj, indentStr, ss);
          fixDeprecatedAttribute(useIDL, structObj, indentStr, ss);

          if (requiresInterface) {
            forwardSS << indentStr << "interface " << "I" << fixName(structObj) << ";\n";
            forwardSS << indentStr << "runtimeclass " << fixName(structObj) << ";\n";
            preISS << indentStr << "[version(1.0)]\n";
            iss << indentStr << "interface I" << fixName(structObj) << " : IInspectable";
            ss << indentStr << "runtimeclass " << fixName(structObj);
            ss << " : [default] I" + fixName(structObj);

            if (structObj->mIsARelationships.size() > 0) {
              iss << " requires ";

              bool first {true};
              for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
              {
                auto &type = (*iter).second;

                if (!first) iss << ", ";
                iss << fixNamePath(type, GenerationOptions { GenerationOptions::Interface(true) });
                first = false;
              }
            }
          } else {
            forwardSS << indentStr << "runtimeclass " << fixName(structObj) << ";\n";
            ss << indentStr << "runtimeclass " << fixName(structObj);

            if (structObj->mIsARelationships.size() > 0) {
              ss << " : ";
              bool first{ true };
              for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
              {
                auto &type = (*iter).second;

                if (!first) ss << ", ";
                ss << fixNamePath(type, GenerationOptions{ });
                first = false;
              }
            }
          }

          iss << "\n";
          ss << "\n";
          iss << indentStr << "{\n";
          ss << indentStr << "{\n";
          indentStr += "    ";
          templateIDL.indent_ = indentStr;
          forwardsIdl.indent_ = indentStr;

          std::stringstream propsSS;
          std::stringstream staticsPropsSS;

          bool foundProperty {false};
          processProperties(useIDL, structObj, propsSS, staticsPropsSS, foundProperty);

          std::stringstream methodsSS;
          std::stringstream interfaceStaticMethodsSS;
          std::stringstream ctorSS;
          std::stringstream interfaceDelegateEventHandlersSS;

          bool foundMethod {false};
          bool foundCtor {false};
          processMethods(useIDL, structObj, methodsSS, requiresInterface ? interfaceStaticMethodsSS : methodsSS, delegatesSS, interfaceDelegateEventHandlersSS, ctorSS, requiresInterface, foundMethod, foundCtor);

          if (GenerateHelper::isConstructable(structObj)) {
            if (structObj->hasModifier(Modifier_Struct_Dictionary)) {
              if (!foundCtor) {
                ctorSS << indentStr << fixName(structObj) << "();\n\n";
                foundCtor = true;
              }
            }
          }

          ss << ctorSS.str();

          if (requiresInterface) {
            String hash = structObj->hash();
            hash = hash.substr(0, sizeof(UUID) * 2);
            hash.insert(20, "-");
            hash.insert(16, "-");
            hash.insert(12, "-");
            hash.insert(8, "-");

            preISS << initialIndentStr << "[uuid(";
            preISS << hash;
            preISS << ")]\n";
          }

          if ((!foundProperty) && (!foundMethod) && (!foundCtor)) {
            preSS << initialIndentStr << "[default_interface]\n";
          }

          (requiresInterface ? iss : ss) << methodsSS.str();
          ss << interfaceStaticMethodsSS.str();
          ss << interfaceDelegateEventHandlersSS.str();

          (requiresInterface ? iss : ss) << propsSS.str();
          ss << staticsPropsSS.str();

          indentStr = initialIndentStr;
          forwardsIdl.indent_ = indentStr;
          templateIDL.indent_ = indentStr;
          iss << indentStr << "};\n";
          ss << indentStr << "};\n";

          forwardsIdl.bodySS_ << forwardSS.str();

          iss << "\n";
          ss << "\n";

          if (isSpecial) {
            useIDL.import("forwards.idl");
            useIDL.import("output.idl");
            outputIdl.import(fixNamePath(structObj, GenerationOptions{} ) + ".idl");

            useIDL.importSS_ << "\n";

            StringList reverseList;

            String tempIndent;

            for (auto iter = namespaces.begin(); iter != namespaces.end(); ++iter) {
              auto &namespaceObj = (*iter);

              std::stringstream reverseSS;

              useIDL.importSS_ << tempIndent << "namespace " << fixName(namespaceObj->mName) << "\n";
              useIDL.importSS_ << tempIndent << "{\n";
              reverseSS << tempIndent << "} // namespace " << fixName(namespaceObj) << "\n";
              reverseList.push_front(reverseSS.str());
              tempIndent += "    ";
            }

            for (auto iter = reverseList.begin(); iter != reverseList.end(); ++iter) {
              auto &str = (*iter);
              ss << str;
            }
          }

          if (requiresInterface) {
            useIDL.bodySS_ << preISS.str();
            useIDL.bodySS_ << iss.str();
          }

          useIDL.bodySS_ << delegatesSS.str();
          useIDL.bodySS_ << preSS.str();
          useIDL.bodySS_ << ss.str();

          if (isSpecial) {
            std::stringstream outSS;
            useIDL.finalize(outSS);
            writeBinary(useIDL.fileName_, UseHelper::convertToBuffer(outSS.str()));
          }

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto subStructObj = (*iter).second;
            processStruct(forwardsIdl, outputIdl, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::processProperties(
                                                    IDLFile &idl,
                                                    StructPtr structObj,
                                                    std::stringstream &ss,
                                                    std::stringstream &staticsSS,
                                                    bool &outFoundProperty
                                                    ) noexcept
        {
          outFoundProperty = false;

          auto &indentStr = idl.indent_;

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter)
          {
            auto &prop = (*iter);
            if (prop->hasModifier(Modifier_Special)) continue;

            bool isStatic = prop->hasModifier(Modifier_Static);

            std::stringstream &useSS = (isStatic ? staticsSS : ss);

            outFoundProperty = true;

            bool isOptional = prop->hasModifier(Modifier_Optional);
            auto type = prop->mType;

            useSS << "\n";
            useSS << GenerateHelper::getDocumentation(indentStr + "/// ", prop, 80);

            fixHiddenAttribute(idl, prop, indentStr, useSS);
            fixDeprecatedAttribute(idl, prop, indentStr, useSS);

            useSS << indentStr;
            if (isStatic) {
              useSS << "static ";
            }
            useSS << toIdlType(idl, GenerationOptions{ GenerationOptions::Optional(isOptional), GenerationOptions::Interface(true) }, type) << " " << fixName(prop->mName);

            bool hasGet = prop->hasModifier(Modifier_Property_Getter);
            bool hasSet = prop->hasModifier(Modifier_Property_Setter);
            bool readOnly = prop->hasModifier(Modifier_Property_ReadOnly);
            bool writeOnly = prop->hasModifier(Modifier_Property_WriteOnly);

            if ((!hasGet) && (!hasSet)) {
              hasGet = true;
              hasSet = true;
            }
            if (readOnly) {
              hasSet = false;
            }
            if (writeOnly) {
              hasGet = false;
            }

            if ((hasGet) || (hasSet)) {
              useSS << " {";
              if (hasGet) {
                useSS << " get;";
              }
              if (hasSet) {
                useSS << " set;";
              }
              useSS << " }";
            }
            useSS << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::processMethods(
                                                 IDLFile &idl,
                                                 StructPtr structObj,
                                                 std::stringstream &methodsSS,
                                                 std::stringstream &staticMethodsSS,
                                                 std::stringstream &delegatesSS,
                                                 std::stringstream &delegateEventHandlersSS,
                                                 std::stringstream &ctorSS,
                                                 bool requiredInterface,
                                                 bool &outFoundMethod,
                                                 bool &outFoundCtor
                                                 ) noexcept
        {
          outFoundMethod = false;

          auto &indentStr = idl.indent_;
          String actualDelegateIndentStr = indentStr.substr(0, indentStr.length() - strlen("    "));

          bool foundEvent {false};
          bool firstCtor {false};
          bool firstMethod {true};

          if (requiredInterface) {
            firstMethod = false;

            staticMethodsSS << "\n";
            staticMethodsSS << indentStr << "/// <summary>\n";
            staticMethodsSS << indentStr << "/// Cast from " << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, structObj) << " to " << toIdlType(idl, GenerationOptions{}, structObj) << "\n";
            staticMethodsSS << indentStr << "/// </summary>\n";
            staticMethodsSS << indentStr << "static " << toIdlType(idl, GenerationOptions{}, structObj)
              << " CastFromI" << fixName(structObj) << "("
              << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, structObj) << " source);\n";
          }

          {
            bool foundCast = false;
            auto found = idl.derives_->find(structObj->getPathName());
            if (found != idl.derives_->end()) {
              auto &structSet = (*found).second;
              for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet)
              {
                auto foundStruct = (*iterSet);
                if (foundStruct != structObj) {

                  staticMethodsSS << "\n";
                  firstMethod = false;

                  bool foundNeedsInterface = idl.isStructNeedingInterface(foundStruct);

                  if (foundNeedsInterface) {
                    staticMethodsSS << indentStr << "/// <summary>\n";
                    staticMethodsSS << indentStr << "/// Cast from " << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, foundStruct) << " to " << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, structObj) << "\n";
                    staticMethodsSS << indentStr << "/// </summary>\n";
                    staticMethodsSS << indentStr << "static " << toIdlType(idl, GenerationOptions{}, structObj)
                      << " CastFromI" << fixName(foundStruct) << "("
                      << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(foundNeedsInterface) }, foundStruct) << " source);\n\n";
                  }

                  staticMethodsSS << indentStr << "/// <summary>\n";
                  staticMethodsSS << indentStr << "/// Cast from " << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, foundStruct) << " to " << toIdlType(idl, GenerationOptions{ GenerationOptions::Interface(requiredInterface) }, structObj) << "\n";
                  staticMethodsSS << indentStr << "/// </summary>\n";
                  staticMethodsSS << indentStr << "static " << toIdlType(idl, GenerationOptions{ }, structObj)
                    << " CastFrom" << fixName(foundStruct) << "("
                    << toIdlType(idl, GenerationOptions{}, foundStruct) << " source);\n";
                }
              }
            }
            if (foundCast) staticMethodsSS << "\n";
          }

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto &method = (*iter);
            bool isCtor = method->hasModifier(Modifier_Method_Ctor);
            bool isDefault = method->hasModifier(Modifier_Method_Default);
            bool isStatic = method->hasModifier(Modifier_Static);
            bool isEvent = method->hasModifier(Modifier_Method_EventHandler);

            if (isCtor) outFoundCtor = true;
            if (method->hasModifier(Modifier_Special)) continue;
            if (method->hasModifier(Modifier_Method_Delete)) continue;

            foundEvent = foundEvent || isEvent;

            if (!isCtor) outFoundMethod = true;

            bool isOptional = method->hasModifier(Modifier_Optional);
            auto resultType = method->mResult;

            bool &useFirst = (isCtor ? firstCtor : firstMethod);
            std::stringstream &useSS = (isCtor ? ctorSS : (isEvent ? delegatesSS : (isStatic ? staticMethodsSS : methodsSS)));
            std::stringstream &handlerSS = (isEvent ? delegateEventHandlersSS : useSS);
            String &useIndentStrForMethod = (isEvent ? actualDelegateIndentStr : indentStr);

            if ((!firstCtor) || (!firstMethod)) {
              handlerSS << "\n";
            }

            handlerSS << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);

            fixHiddenAttribute(idl, method, indentStr, handlerSS);
            fixDeprecatedAttribute(idl, method, indentStr, handlerSS);
            fixMethodNameAttribute(idl, method, indentStr, handlerSS);
            fixDefaultAttribute(idl, method, indentStr, handlerSS);

            bool foundAnotherCtorWithSameNumberOfArguments{false};

            useSS << useIndentStrForMethod;
            if (!isCtor) {
              if (isStatic) {
                useSS << "static ";
              }
              if (isEvent) {
                useSS << "delegate ";
              }
              useSS << toIdlType(idl, GenerationOptions{ GenerationOptions::Optional(isOptional), GenerationOptions::Interface(true) }, resultType) << " ";
            }
            if (isCtor) {
              if (!isDefault) {
                foundAnotherCtorWithSameNumberOfArguments = hasAnotherCtorWithSameNumberOfArguments(structObj, method);
              }
            }
            if (foundAnotherCtorWithSameNumberOfArguments) {
              String altMethodName = method->getModifierValue(Modifier_AltName);

              useSS << "static " << toIdlType(idl, GenerationOptions{ GenerationOptions::Optional(isOptional), GenerationOptions::Interface(requiredInterface) }, structObj) << " ";
              useSS << (altMethodName.hasData() ? fixName(altMethodName) : fixName(method->mName));
            } else {
              if (isEvent) {
                handlerSS << indentStr << "event " << fixName(structObj) << "_" << fixName(method->mName) << "Delegate " << fixName(method->mName) << ";\n";
                useSS << fixName(structObj) << "_";
              }
              useSS << (isCtor ? fixName(structObj) : fixName(method->mName));
              if (isEvent) {
                useSS << "Delegate";
              }
            }
            useSS << "(";

            bool foundParam {false};

            auto totalParams = method->mArguments.size();

            for (auto iterParam = method->mArguments.begin(); iterParam != method->mArguments.end(); ++iterParam) {
              auto &paramType = (*iterParam)->mType;
              auto paramName = fixName((*iterParam)->mName);

              if (foundParam) {
                useSS << ",";
              }

              if (totalParams > 1) {
                useSS << "\n" << indentStr << "    ";
              }

              useSS << toIdlType(idl, GenerationOptions{ GenerationOptions::Optional(isOptional), GenerationOptions::Interface(true) }, paramType) << " " << paramName;
              foundParam = true;
            }
            if ((totalParams > 1) && (foundParam)) {
              useSS << "\n" << indentStr;
            }

            useSS << ");\n";
            useFirst = false;
          }

          if (foundEvent) {
            delegatesSS << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::processEnum(
                                              IDLFile &forwardsIdl,
                                              IDLFile &outputIdl,
                                              EnumTypePtr enumObj
                                              ) noexcept
        {
          if (enumObj->hasModifier(Modifier_Special)) return;

          String &indentStr = outputIdl.indent_;
          std::stringstream &ss = outputIdl.enumSS_;

          ss << GenerateHelper::getDocumentation(indentStr + "/// ", enumObj, 80);

          forwardsIdl.bodySS_ << indentStr << "typedef enum " << fixName(enumObj) << " " << fixName(enumObj) << ";\n";

          ss << indentStr << "enum " << fixName(enumObj) << "\n";
          ss << indentStr << "{\n";

          bool firstValue = true;
          for (auto iter = enumObj->mValues.begin(); iter != enumObj->mValues.end(); ++iter)
          {
            auto &prop = (*iter);
            if (prop->hasModifier(Modifier_Special)) continue;
            if (!firstValue) {
              ss << ",\n";
            }
            ss << GenerateHelper::getDocumentation(indentStr + "    /// ", prop, 80);
            outputIdl.enumSS_ << indentStr << "    " << fixName(prop->mName);
            if (prop->mValue.hasData()) {
              ss << " = " << prop->mValue;
            }
            firstValue = false;
          }
          ss << "\n";
          ss << indentStr << "};\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // GenerateStructHeader::IIDLCompilerTarget
        //

        //---------------------------------------------------------------------
        String GenerateStructMsidl::targetKeyword() noexcept
        {
          return String("msidl");
        }

        //---------------------------------------------------------------------
        String GenerateStructMsidl::targetKeywordHelp() noexcept
        {
          return String("Generate Microsoft IDL");
        }

        //---------------------------------------------------------------------
        void GenerateStructMsidl::targetOutput(
                                               const String &inPathStr,
                                               const ICompilerTypes::Config &config
                                               ) noexcept(false)
        {
          String pathStr(UseHelper::fixRelativeFilePath(inPathStr, String("wrapper")));

          try {
            UseHelper::mkdir(pathStr);
          }
          catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("generated"));
          try {
            UseHelper::mkdir(pathStr);
          }
          catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("msidl"));
          try {
            UseHelper::mkdir(pathStr);
          }
          catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;

          IDLFile outputIdlFile;
          outputIdlFile.global_ = project->mGlobal;
          outputIdlFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("output.idl"));

          scanNamespaceForStructsNeedingToBeInterfaces(*(outputIdlFile.structsNeedingInterface_), outputIdlFile.global_);
          calculateRelations(project->mGlobal, *outputIdlFile.derives_);

          IDLFile forwardIdlFile;
          forwardIdlFile.global_ = project->mGlobal;
          forwardIdlFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("forwards.idl"));
          forwardIdlFile.structsNeedingInterface_ = outputIdlFile.structsNeedingInterface_;   // reference previously scanned base class set
          forwardIdlFile.derives_ = outputIdlFile.derives_;   // reference previously scanned base class set

          forwardIdlFile.importSS_ << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          outputIdlFile.importSS_ << "// " ZS_EVENTING_GENERATED_BY "\n\n";

          outputIdlFile.import("forwards.idl");

          processNamespace(forwardIdlFile, outputIdlFile, outputIdlFile.global_);

          {
            std::stringstream ss;
            outputIdlFile.finalize(ss);
            writeBinary(outputIdlFile.fileName_, UseHelper::convertToBuffer(ss.str()));
          }

          {
            std::stringstream ss;
            forwardIdlFile.finalize(ss);
            writeBinary(forwardIdlFile.fileName_, UseHelper::convertToBuffer(ss.str()));
          }

        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
