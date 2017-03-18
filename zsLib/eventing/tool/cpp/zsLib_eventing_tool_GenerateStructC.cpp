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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructC.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateHelper.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateTypesHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>

#include <sstream>

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
        #pragma mark (helpers)
        #pragma mark
        //---------------------------------------------------------------------
        static void doInclude(
                              const String &headerFile,
                              std::stringstream &ss,
                              GenerateStructC::StringSet &alreadyIncluded
                              )
        {
          if (alreadyIncluded.end() != alreadyIncluded.find(headerFile)) return;
          alreadyIncluded.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC::HelperFile
        #pragma mark

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::headerIncludeC(const String &headerFile)
        {
          doInclude(headerFile, headerCIncludeSS_, headerCAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::headerIncludeCpp(const String &headerFile)
        {
          doInclude(headerFile, headerCppIncludeSS_, headerCppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::includeC(const String &headerFile)
        {
          doInclude(headerFile, cIncludeSS_, cAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::includeCpp(const String &headerFile)
        {
          doInclude(headerFile, cppIncludeSS_, cppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::headerIncludeC(const String &headerFile)
        {
          doInclude(headerFile, headerCIncludeSS_, headerCAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::headerIncludeCpp(const String &headerFile)
        {
          doInclude(headerFile, headerCppIncludeSS_, headerCppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::includeC(const String &headerFile)
        {
          doInclude(headerFile, cIncludeSS_, cAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::includeCpp(const String &headerFile)
        {
          doInclude(headerFile, cppIncludeSS_, cppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructC::GenerateStructC() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructCPtr GenerateStructC::create()
        {
          return make_shared<GenerateStructC>();
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixCType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_void:      return "void";

            case PredefinedTypedef_bool:      return "bool_t";

            case PredefinedTypedef_uchar:     return "schar_t";
            case PredefinedTypedef_char:
            case PredefinedTypedef_schar:     return "uchar_t";
            case PredefinedTypedef_ushort:    return "ushort_t";
            case PredefinedTypedef_short:
            case PredefinedTypedef_sshort:    return "sshort_t";
            case PredefinedTypedef_uint:      return "uint_t";
            case PredefinedTypedef_int:
            case PredefinedTypedef_sint:      return "sint_t";
            case PredefinedTypedef_ulong:     return "ulong_t";
            case PredefinedTypedef_long:
            case PredefinedTypedef_slong:     return "slong_t";
            case PredefinedTypedef_ulonglong: return "ullong_t";
            case PredefinedTypedef_longlong:
            case PredefinedTypedef_slonglong: return "sllong_t";
            case PredefinedTypedef_uint8:     return "uint8_t";
            case PredefinedTypedef_int8:
            case PredefinedTypedef_sint8:     return "int8_t";
            case PredefinedTypedef_uint16:    return "uint16_t";
            case PredefinedTypedef_int16:
            case PredefinedTypedef_sint16:    return "int16_t";
            case PredefinedTypedef_uint32:    return "uint32_t";
            case PredefinedTypedef_int32:
            case PredefinedTypedef_sint32:    return "int32_t";
            case PredefinedTypedef_uint64:    return "uint64_t";
            case PredefinedTypedef_int64:
            case PredefinedTypedef_sint64:    return "uint64_t";

            case PredefinedTypedef_byte:      return "uint8_t";
            case PredefinedTypedef_word:      return "uint16_t";
            case PredefinedTypedef_dword:     return "uint32_t";
            case PredefinedTypedef_qword:     return "uint64_t";

            case PredefinedTypedef_float:     return "float_t";
            case PredefinedTypedef_double:    return "double_t";
            case PredefinedTypedef_ldouble:   return "ldouble_t";
            case PredefinedTypedef_float32:   return "float32_t";
            case PredefinedTypedef_float64:   return "float64_t";

            case PredefinedTypedef_pointer:   return "uintptr_t";

            case PredefinedTypedef_binary:    return "binary_t";
            case PredefinedTypedef_size:      return "uintptr_t";

            case PredefinedTypedef_string:
            case PredefinedTypedef_astring:
            case PredefinedTypedef_wstring:   return "string_t";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixCType(TypePtr type)
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return fixCType(basicType->mBaseType);
            }
          }

          auto result = fixType(type);
          return result + "_t";
        }


        //---------------------------------------------------------------------
        String GenerateStructC::fixType(TypePtr type)
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return fixCType(basicType->mBaseType);
            }
          }

          {
            auto templateType = type->toTemplatedStructType();
            if (templateType) {
              auto parent = type->getParent();
              if (parent) {
                auto result = fixType(parent->toStruct());
                for (auto iter = templateType->mTemplateArguments.begin(); iter != templateType->mTemplateArguments.end(); ++iter) {
                  auto typeArgument = (*iter);
                  String temp = fixType(typeArgument);
                  if (temp.hasData()) {
                    if (result.hasData()) {
                      result += "_";
                    }
                    result += temp;
                  }
                }
                return result;
              }
            }
          }

          auto result = type->getPathName();
          if ("::" == result.substr(0, 2)) {
            result = result.substr(2);
          }
          result.replaceAll("::", "_");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiImplementationDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_GENERATED_IMPLEMENTATION";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_IMPLEMENTATION";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiExportDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_EXPORT_API";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_EXPORT_API";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiCallingDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_CALLING_CONVENTION";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_CALLING_CONVENTION";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiGuardDefine(
                                                  ContextPtr context,
                                                  bool endGuard
                                                  )
        {
          String result = (!endGuard ? "WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "WRAPPER_C_PLUS_PLUS_END_GUARD");
          if (!context) return result;
          auto root = context->getRoot();
          if (!root) return result;
          auto project = context->toProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + (!endGuard ? "_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "_WRAPPER_C_PLUS_PLUS_END_GUARD");
        }
        
        //---------------------------------------------------------------------
        void GenerateStructC::includeType(
                                          HelperFile &helperFile,
                                          TypePtr type
                                          )
        {
          if (!type) return;
          if (type->toBasicType()) return;
          if (type->toEnumType()) return;
          
          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                auto subType = (*iter);
                includeType(helperFile, subType);
              }
              return;
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              String fileName = "\"" + fixType(type) + ".h\"";
              helperFile.includeC(fileName);
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateRelations(
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
        void GenerateStructC::calculateRelations(
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
        void GenerateStructC::insertInto(
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
        void GenerateStructC::appendStream(
                                           std::stringstream &output,
                                           std::stringstream &source,
                                           bool appendEol
                                           )
        {
          String str = source.str();
          if (str.isEmpty()) return;

          if (appendEol) {
            output << "\n";
          }
          output << str;
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperFile(HelperFile &helperFile)
        {
          {
            auto &ss = helperFile.headerCIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "#pragma once\n\n";
            ss << "#include \"types.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << getApiGuardDefine(helperFile.global_) << "\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "\n";
            ss << "#include \"c_helpers.h\"\n";
            ss << "#include <zsLib/types.h>\n";
            ss << "#include <zsLib/eventing/types.h>\n";
            ss << "#include <zsLib/SafeInt.h>\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;
            ss << "using namespace wrapper;\n\n";
          }

          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "\n";
            ss << getApiGuardDefine(helperFile.global_, true) << "\n";
            ss << "\n";

            ss << "#ifdef __cplusplus\n";
            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;

            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          prepareHelperCallback(helperFile);
          prepareHelperString(helperFile);

          prepareHelperDuration(helperFile, "Time");
          prepareHelperDuration(helperFile, "Days");
          prepareHelperDuration(helperFile, "Hours");
          prepareHelperDuration(helperFile, "Seconds");
          prepareHelperDuration(helperFile, "Minutes");
          prepareHelperDuration(helperFile, "Milliseconds");
          prepareHelperDuration(helperFile, "Microseconds");
          prepareHelperDuration(helperFile, "Nanoseconds");

          prepareHelperList(helperFile, "list");
          prepareHelperList(helperFile, "set");
          prepareHelperList(helperFile, "map");

          prepareHelperSpecial(helperFile, "Any");
          prepareHelperSpecial(helperFile, "Promise");
          preparePromiseWithValue(helperFile);
          preparePromiseWithRejectionReason(helperFile);

          {
            auto &ss = helperFile.headerCppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
            ss << "#endif /* __cplusplus */\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperCallback(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << getApiGuardDefine(helperFile.global_) << "\n";
            ss << "\n";
            ss << "typedef void (ORTC_WRAPPER_C_CALLING_CONVENTION *wrapperCallback)(uintptr_t);\n";
            ss << "\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperInstall(wrapperCallback function);\n";
            ss << "\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperDestroy(uintptr_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const char * " << getApiCallingDefine(helperFile.global_) << " callback_get_class(uintptr_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const char * " << getApiCallingDefine(helperFile.global_) << " callback_get_method(uintptr_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " uintptr_t " << getApiCallingDefine(helperFile.global_) << " callback_get_event_source(uintptr_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " uintptr_t " << getApiCallingDefine(helperFile.global_) << " callback_get_event_data(uintptr_t handle, int argumentIndex);\n";

            ss << "\n";
          }
          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "  struct IWrapperCallback;\n";
            ss << "  typedef shared_ptr<IWrapperCallback> IWrapperCallbackPtr;\n";
            ss << "\n";
            ss << "  struct IWrapperCallback\n";
            ss << "  {\n";
            ss << "    static void fireEvent(IWrapperCallbackPtr event);\n";
            ss << "\n";
            ss << "    virtual const char *getClass() = 0;\n";
            ss << "    virtual const char *getMethod() = 0;\n";
            ss << "    virtual uintptr_t getSource() = 0;\n";
            ss << "    virtual uintptr_t getEventData(int argumentIndex) = 0;\n";
            ss << "  };\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;
            ss << dash;
            ss << "static wrapperCallback &callback_get_singleton()\n";
            ss << "{\n";
            ss << "  static wrapperCallback function {};\n";
            ss << "  return function;\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void callback_wrapperInstall(wrapperCallback function)\n";
            ss << "{\n";
            ss << "  callback_get_singleton() = function;\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void callback_wrapperDestroy(uintptr_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackPtr * IWrapperCallbackPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  delete reinterpret_cast<IWrapperCallbackPtrRawPtr>(handle);\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const char *callback_get_class(uintptr_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackPtr * IWrapperCallbackPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackPtrRawPtr>(handle))->getClass();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const char *callback_get_method(uintptr_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackPtr * IWrapperCallbackPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackPtrRawPtr>(handle))->getMethod();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "uintptr_t callback_get_event_source(uintptr_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackPtr * IWrapperCallbackPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackPtrRawPtr>(handle))->getSource();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "uintptr_t callback_get_event_data(uintptr_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackPtr * IWrapperCallbackPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackPtrRawPtr>(handle))->getEventData();\n";
            ss << "}\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;
            ss << dash2;
            ss << "  void IWrapperCallback::fireEvent(IWrapperCallbackPtr event)\n";
            ss << "  {\n";
            ss << "    if (!event) return;\n";
            ss << "    auto singleton = callback_get_singleton();\n";
            ss << "    if (!singleton) return;\n";
            ss << "    uintptr_t handle = reinterpret_cast<uintptr_t>(new IWrapperCallbackPtr(event));\n";
            ss << "    singleton(handle);\n";
            ss << "  }\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperString(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_string_t();\n";
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_string_tWithValue(const char *value);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperDestroy(string_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " const char * " << getApiCallingDefine(helperFile.global_) << " string_t_get_value(string_t_ handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " string_t_set_value(string_t_ handle, const char *value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  string_t string_t_wrapperToHandle(const ::std::string &value);\n";
              ss << "  ::zsLib::String string_t_wrapperFromHandle(string_t handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_string_t()\n";
              ss << "{\n";
              ss << "  return reinterpret_cast<string_t>(new ::zsLib::String());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "string_t string_t_wrapperCreate_string_tWithValue(const char *value)\n";
              ss << "{\n";
              ss << "  return reinterpret_cast<string_t>(new ::zsLib::String(value));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void string_t_wrapperDestroy(string_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<::zsLib::String *>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "const char *string_t_get_value(string_t_ handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return reinterpret_cast<const char *>(NULL);\n";
              ss << "  return (*reinterpret_cast<::zsLib::String *>(handle));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void string_t_set_value(string_t_ handle, const char *value)\n";
              ss << "{\n";
              ss << "  if (0 == handle) throw std::runtime_error(\"null pointer exception\");\n";
              ss << "  (*reinterpret_cast<::zsLib::String *>(handle)) = ::zsLib::String(value);\n";
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  string_t string_t_wrapperToHandle(const ::std::string &value)\n";
              ss << "  {\n";
              ss << "    return reinterpret_cast<string_t>(new ::zsLib::String(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  ::zsLib::String string_t_wrapperFromHandle(string_t handle)\n";
              ss << "  {\n";
              ss << "    if (0 == handle) return ::zsLib::String();\n";
              ss << "    return (*reinterpret_cast<::zsLib::String *>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperDuration(
                                                    HelperFile &helperFile,
                                                    const String &durationType
                                                    )
        {
          bool isTime = "Time" == durationType;

          auto durationContext = helperFile.global_->toContext()->findType("::zs::" + durationType);
          if (!durationContext) return;

          auto durationStruct = durationContext->toStruct();
          if (!durationStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));
          String zsDurationType = "::zsLib::" + durationType;

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(durationStruct) << " " << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "();\n";
              ss << getApiExportDefine(durationStruct) << " " << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(int64_t value);\n";
              ss << getApiExportDefine(durationStruct) << " " << "void " << getApiCallingDefine(durationStruct) <<" zs_" << durationType << "_wrapperDestroy(" << fixCType(durationStruct) << " handle);\n";
              ss << getApiExportDefine(durationStruct) << " " << "int64_t " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_get_value(" << fixCType(durationStruct) << " handle);\n";
              ss << getApiExportDefine(durationStruct) << " " << "void " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_set_value(" << fixCType(durationStruct) << " handle, int64_t value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  " << fixCType(durationStruct) << " zs_" << durationType << "_wrapperToHandle(" << durationType << " value);\n";
              ss << "  " << zsDurationType << " zs_" << durationType << "_wrapperFromHandle(" << fixCType(durationStruct) << " handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << fixCType(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "()\n";
              ss << "{\n";
              ss << "  typedef " << fixCType(durationStruct) << " CType;\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  return reinterpret_cast<CType>(new DurationType());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << fixCType(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(int64_t value)\n";
              ss << "{\n";
              ss << "  typedef " << fixCType(durationStruct) << " CType;\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType::rep DurationTypeRep;\n";
              if (isTime) {
                ss << "  typedef DurationType::duration TimeDurationType;\n";
                ss << "  return reinterpret_cast<CType>(new DurationType(TimeDurationType(SafeInt<DurationTypeRep>(value))));\n";
              } else {
                ss << "  return reinterpret_cast<CType>(new DurationType(static_cast<DurationTypeRep>(SafeInt<DurationTypeRep>(value))));\n";
              }
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void zs_" << durationType << "_wrapperDestroy(" << fixCType(durationStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<DurationTypeRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "int64_t zs_" << durationType << "_get_value(" << fixCType(durationStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              if (isTime) {
                ss << "  return SafeInt<int64_t>(reinterpret_cast<DurationTypeRawPtr>(handle)->time_since_epoch().count());\n";
              } else {
                ss << "  return SafeInt<int64_t>(reinterpret_cast<DurationTypeRawPtr>(handle)->count());\n";
              }
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void zs_" << durationType << "_set_value(" << fixCType(durationStruct) << " handle, int64_t value)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  typedef DurationType::rep DurationTypeRep;\n";
              ss << "  if (0 == handle) throw std::runtime_error(\"null pointer exception\");\n";
              if (isTime) {
                ss << "  typedef DurationType::duration TimeDurationType;\n";
                ss << "  (*reinterpret_cast<DurationTypeRawPtr>(handle)) = DurationType(TimeDurationType(SafeInt<DurationTypeRep>(value)));\n";
              } else {
                ss << "  (*reinterpret_cast<DurationTypeRawPtr>(handle)) = DurationType(SafeInt<DurationTypeRep>(value));\n";
              }
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << fixCType(durationStruct) << " zs_" << durationType << "_wrapperToHandle(" << durationType << " value)\n";
              ss << "  {\n";
              ss << "    typedef " << fixCType(durationStruct) << " CType;\n";
              ss << "    typedef " << zsDurationType << " DurationType;\n";
              ss << "    typedef DurationType * DurationTypeRawPtr;\n";
              ss << "    if (DurationType() == value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new DurationType(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << zsDurationType << " zs_" << durationType << "_wrapperFromHandle(" << fixCType(durationStruct) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << zsDurationType << " DurationType;\n";
              ss << "    typedef DurationType * DurationTypeRawPtr;\n";
              ss << "    if (0 == handle) return DurationType();\n";
              ss << "    return (*reinterpret_cast<DurationTypeRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperList(
                                                HelperFile &helperFile,
                                                const String &listOrSetStr
                                                )
        {
          bool isMap = ("map" == listOrSetStr);
          auto context = helperFile.global_->toContext()->findType("::std::" + listOrSetStr);
          if (!context) return;

          auto structType = context->toStruct();
          if (!structType) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          for (auto iter = structType->mTemplatedStructs.begin(); iter != structType->mTemplatedStructs.end(); ++iter) {
            auto templatedStructType = (*iter).second;

            TypePtr keyType;
            TypePtr listType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              listType = (*iterArg);
              if (isMap) {
                ++iterArg;
                if (iterArg != templatedStructType->mTemplateArguments.end()) {
                  keyType = listType;
                  listType = (*iterArg);
                }
              }
            }

            includeType(helperFile, listType);

            {
              {
                auto &ss = helperFile.headerCFunctionsSS_;
                ss << getApiExportDefine(structType) << " " << fixCType(templatedStructType) << " " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "();\n";
                ss << getApiExportDefine(structType) << " " << "void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperDestroy(" << fixCType(templatedStructType) << " handle);\n";
                if (isMap) {
                  ss << getApiExportDefine(structType) << " " << "void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(keyType) << " key, " << fixCType(listType) << " value);\n";
                }
                else {
                  ss << getApiExportDefine(structType) << " " << "void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(listType) << " value);\n";
                }

                ss << getApiExportDefine(structType) << " uintptr_t " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterBegin(" << fixCType(templatedStructType) << " handle);\n";
                ss << getApiExportDefine(structType) << " void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterNext(uintptr_t iterHandle);\n";
                ss << getApiExportDefine(structType) << " bool " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterIsEnd(uintptr_t iterHandle);\n";
                if (isMap) {
                  ss << getApiExportDefine(structType) << " " << fixCType(keyType) << " " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterKey(uintptr_t iterHandle);\n";
                }
                ss << getApiExportDefine(structType) << " " << fixCType(listType) << " " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterValue(uintptr_t iterHandle);\n";
                ss << "\n";
              }
              {
                auto &ss = helperFile.headerCppFunctionsSS_;
                ss << "  " << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " " << fixType(templatedStructType) << "_wrapperFromHandle(" << fixCType(templatedStructType) << " handle);\n";
                ss << "  " << fixCType(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " value);\n";
                ss << "\n";
              }
            }

            std::stringstream typedefsSS;
            std::stringstream typedefsWithIterSS;
            std::stringstream typedefsSS2;
            {
              auto &ss = typedefsSS;
              ss << "  typedef " << fixCType(templatedStructType) << " CType;\n";
              if (isMap) {
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, keyType) << " WrapperKeyType;\n";
              }
              ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, listType) << " WrapperType;\n";
              ss << "  typedef ::std::" << listOrSetStr << "<" << (isMap ? "WrapperKeyType, " : "") << "WrapperType> WrapperTypeList;\n";
              ss << "  typedef shared_ptr<WrapperTypeList> WrapperTypeListPtr;\n";
              ss << "  typedef WrapperTypeListPtr * WrapperTypeListPtrRawPtr;\n";
            }
            {
              auto &ss = typedefsWithIterSS;
              ss << typedefsSS.str();
              ss << "  typedef WrapperTypeList::iterator WrapperTypeListIterator;\n";
              ss << "  typedef WrapperTypeListIterator * WrapperTypeListIteratorRawPtr;\n";
            }
            {
              String tmp = typedefsSS.str();
              tmp.replaceAll("  ", "    ");
              typedefsSS2 << tmp;
            }

            {
              {
                auto &ss = helperFile.cFunctionsSS_;
                ss << dash;
                ss << fixCType(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "()\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  return reinterpret_cast<CType>(new WrapperTypeListPtr(make_shared<WrapperTypeList>()));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << fixType(templatedStructType) << "_wrapperDestroy(" << fixCType(templatedStructType) << " handle)\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  if (0 == handle) return;\n";
                ss << "  delete reinterpret_cast<WrapperTypeListPtrRawPtr>(handle);\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(listType) << " value)\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  if (0 == handle) throw std::runtime_error(\"null pointer exception\");\n";

                String keyTypeStr;
                String listTypeStr;

                if (isMap) {
                  if (((keyType->toBasicType()) ||
                      (keyType->toEnumType())) &&
                     ("string_t" != fixCType(keyType))) {
                    keyTypeStr = "key";
                  } else {
                    keyTypeStr = fixType(keyType) + "_wrapperFromHandle(key)";
                  }
                }
                if (((listType->toBasicType()) ||
                    (listType->toEnumType())) &&
                    ("string_t" != fixCType(listType))) {
                  listTypeStr = "value";
                } else {
                  listTypeStr = fixType(listType) + "_wrapperFromHandle(value)";
                }

                if (isMap) {
                  ss << "  (*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle)))[" << keyTypeStr << "] = " << listTypeStr << ";\n";
                } else {
                  ss << "  (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->insert(" << listTypeStr << ");\n";
                }
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "uintptr_t " << fixType(templatedStructType) << "_wrapperIterBegin(" << fixCType(templatedStructType) << " handle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == handle) throw std::runtime_error(\"null pointer exception\");\n";
                ss << "  return reinterpret_cast<uintptr_t>(new WrapperTypeListIterator((*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->begin()));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << fixType(templatedStructType) << "_wrapperIterNext(uintptr_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == iterHandle) throw std::runtime_error(\"null pointer exception\");\n";
                ss << "  ++(*reinterpret_cast<WrapperTypeListPtrRawPtr>(iterHandle));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool " << fixType(templatedStructType) << "_wrapperIterIsEnd(" << fixCType(templatedStructType) << " handle, uintptr_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == handle) throw std::runtime_error(\"null pointer exception\");\n";
                ss << "  if (0 == iterHandle) throw std::runtime_error(\"null pointer exception\");\n";
                ss << "  auto iterRawPtr = reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle);\n";
                ss << "  bool isEnd = (*iterRawPtr) == (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->end();\n";
                ss << "  if (isEnd) delete iterRawPtr;\n";
                ss << "  return isEnd;\n";
                ss << "}\n";
                ss << "\n";

                if (isMap) {
                  ss << dash;
                  ss << fixCType(keyType) << " " << fixType(templatedStructType) << "_wrapperIterKey(uintptr_t iterHandle)\n";
                  ss << "{\n";
                  ss << typedefsWithIterSS.str();
                  ss << "  if (0 == iterHandle) throw std::runtime_error(\"null pointer exception\");\n";
                  if (((keyType->toBasicType()) ||
                    (keyType->toEnumType())) &&
                    ("string_t" != fixCType(keyType))) {
                    ss << "  return (*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(iterHandle))).first;\n";
                  }
                  else {
                    ss << "  return " << fixType(keyType) << "_wrapperToHandle(*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(iterHandle)).first);\n";
                  }
                  ss << "}\n";
                  ss << "\n";
                }

                ss << dash;
                ss << fixCType(listType) << " " << fixType(templatedStructType) << "_wrapperIterValue(uintptr_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == iterHandle) throw std::runtime_error(\"null pointer exception\");\n";
                if (((listType->toBasicType()) ||
                     (listType->toEnumType())) &&
                   ("string_t" != fixCType(listType))) {
                  ss << "  return (*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(iterHandle)))" << (isMap ? ".second" : "") << ";\n";
                } else {
                  ss << "  return " << fixType(listType) << "_wrapperToHandle(*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(iterHandle))" << (isMap ? ".second" : "") << ");\n";
                }
                ss << "}\n";
                ss << "\n";
              }
              {
                auto &ss = helperFile.cppFunctionsSS_;
                ss << dash2;
                ss << "  " << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " " << fixType(templatedStructType) << "_wrapperFromHandle(" << fixCType(templatedStructType) << " handle)\n";
                ss << "  {\n";
                ss << typedefsSS2.str();
                ss << "    if (0 == handle) return WrapperTypeListPtr();\n";
                ss << "    return (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle));\n";
                ss << "  }\n";
                ss << "\n";

                ss << dash2;
                ss << "  " << fixCType(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " value)\n";
                ss << "  {\n";
                ss << typedefsSS2.str();
                ss << "    if (!value) return 0;\n";
                ss << "    return reinterpret_cast<CType>(new WrapperTypeListPtr(value));\n";
                ss << "  }\n";
                ss << "\n";
              }
            }

          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperSpecial(
                                                   HelperFile &helperFile,
                                                   const String specialName
                                                   )
        {
          bool isPromise = "Promise" == specialName;

          auto context = helperFile.global_->toContext()->findType("::zs::" + specialName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));
          String zsSpecialType = "::zsLib::" + specialName;

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " void " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperDestroy(" << fixCType(contextStruct) << " handle);\n";
              if (isPromise) {
                ss << getApiExportDefine(contextStruct) << " void " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperObserveEvents(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " uint64_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_get_id(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isSettled(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isResolved(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isRejected(" << fixCType(contextStruct) << " handle);\n";
              }
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              if (isPromise) {
                ss << "  void zs_" << specialName << "_wrapperObserveEvents(" << specialName << "Ptr value);\n";
              }
              ss << "  " << fixCType(contextStruct) << " zs_" << specialName << "_wrapperToHandle(" << specialName << "Ptr value);\n";
              ss << "  " << specialName << "Ptr zs_" << specialName << "_wrapperFromHandle(" << fixCType(contextStruct) << " handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << "void zs_" << specialName << "_wrapperDestroy(" << fixCType(contextStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << specialName << "Ptr WrapperType;\n";
              ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<WrapperTypeRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              if (isPromise) {
                ss << dash;
                ss << "void zs_" << specialName << "_wrapperObserveEvents(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  wrapper::zs_" << specialName << "_wrapperObserveEvents((*reinterpret_cast<WrapperTypeRawPtr>(handle)));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "uint64_t zs_" << specialName << "_get_id(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return SafeInt<uint64_t>((*reinterpret_cast<WrapperTypeRawPtr>(handle))->getID());\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool zs_" << specialName << "_isSettled(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isSettled();\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool zs_" << specialName << "_isResolved(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isResolved();\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool zs_" << specialName << "_isRejected(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isRejected();\n";
                ss << "}\n";
                ss << "\n";
              }
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              if (isPromise) {
                ss << dash2;
                ss << dash2;
                ss << dash2;
                ss << dash2;
                ss << "  //\n";
                ss << "  // PromiseCallback\n";
                ss << "  //\n";
                ss << "\n";
                ss << "  ZS_DECLARE_CLASS_PTR(PromiseCallback);\n";
                ss << "\n";
                ss << "  class PromiseCallback : public IWrapperCallback,\n";
                ss << "                          public zsLib::IPromiseSettledDelegate\n";
                ss << "  {\n";
                ss << "  public:\n";
                ss << "    PromiseCallback(PromisePtr promise) : promise_(promise) {}\n";
                ss << "\n";
                ss << "    PromiseCallbackPtr create(PromisePtr promise)\n";
                ss << "    {\n";
                ss << "      if (!promise) return PromiseCallbackPtr();\n";
                ss << "\n";
                ss << "      auto pThis = make_shared<PromiseCallback>(promise);\n";
                ss << "      thisWeak_ = pThis;\n";
                ss << "      promise->then(pThis);\n";
                ss << "      promise->background();\n";
                ss << "      return pThis;\n";
                ss << "    }\n";
                ss << "\n";
                ss << "    virtual const char *getClass()  {return \"zs_Promise\";}\n";
                ss << "    virtual const char *getMethod() {return \"onSettled\";}\n";
                ss << "    virtual uintptr_t getSource()   {return zs_Promise_wrapperToHandle(promise_);}\n";
                ss << "    virtual uintptr_t getEventData(int argumentIndex) {return 0;}\n";
                ss << "\n";
                ss << "    virtual void onPromiseSettled(PromisePtr promise)\n";
                ss << "    {\n";
                ss << "      IWrapperCallback::fireEvent(thisWeak_.lock());\n";
                ss << "    }\n";
                ss << "\n";
                ss << "  private:\n";
                ss << "    PromiseCallbackWeakPtr thisWeak_;\n";
                ss << "    PromisePtr promise_;\n";
                ss << "  };\n";
                ss << "\n";

                ss << dash2;
                ss << "  void zs_" << specialName << "_wrapperObserveEvents(" << specialName << "Ptr value)\n";
                ss << "  {\n";
                ss << "    PromiseCallback::create(value);\n";
                ss << "  }\n";
                ss << "\n";
              }
              ss << dash2;
              ss << "  " << fixCType(contextStruct) << " zs_" << specialName << "_wrapperToHandle(" << specialName << "Ptr value)\n";
              ss << "  {\n";
              ss << "    typedef " << specialName << "Ptr WrapperType;\n";
              ss << "    typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "    if (0 == handle) return 0;\n";
              ss << "    if (!value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new DurationType(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << specialName << "Ptr zs_" << specialName << "_wrapperFromHandle(" << fixCType(contextStruct) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << specialName << "Ptr WrapperType;\n";
              ss << "    typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "    if (0 == handle) return WrapperType();\n";
              ss << "    return (*reinterpret_cast<WrapperTypeRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::preparePromiseWithValue(HelperFile &helperFile)
        {
          auto context = helperFile.global_->toContext()->findType("::zs::PromiseWith");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            includeType(helperFile, promiseType);

            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " " << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_resolveValue_" << fixType(promiseType) << "(zs_Promise handle);\n";
            }
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << fixCType(promiseType) << " zs_PromiseWith_resolveValue_" << fixType(promiseType) << "(zs_Promise handle)\n";
              ss << "{\n";
              ss << "  typedef ::zsLib::AnyHolder< " << GenerateStructHeader::getWrapperTypeString(false, promiseType) << " > AnyHolderWrapper;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  PromisePtr promise = (*reinterpret_cast<PromisePtrRawPtr>(handle));\n";
              ss << "  if (!promise) return 0;\n";
              ss << "  auto holder = promise->value<AnyHolderWrapper>();\n";
              ss << "  if (!holder) return 0;\n";
              ss << "  return " << fixType(promiseType) << "_wrapperToHandle(holder->value_);\n";
              ss << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::preparePromiseWithRejectionReason(HelperFile &helperFile)
        {
          auto context = helperFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            includeType(helperFile, promiseType);

            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " " << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_rejectReason_" << fixType(promiseType) << "(zs_Promise handle);\n";
            }
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << fixCType(promiseType) << " zs_PromiseWith_rejectReason_" << fixType(promiseType) << "(zs_Promise handle)\n";
              ss << "{\n";
              ss << "  typedef ::zsLib::AnyHolder< " << GenerateStructHeader::getWrapperTypeString(false, promiseType) << " > AnyHolderWrapper;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  PromisePtr promise = (*reinterpret_cast<PromisePtrRawPtr>(handle));\n";
              ss << "  if (!promise) return 0;\n";
              ss << "  auto holder = promise->value<AnyHolderWrapper>();\n";
              ss << "  if (!holder) return 0;\n";
              ss << "  return " << fixType(promiseType) << "_wrapperToHandle(holder->value_);\n";
              ss << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::finalizeHelperFile(HelperFile &helperFile)
        {
          {
            std::stringstream ss;

            appendStream(ss, helperFile.headerCIncludeSS_);
            appendStream(ss, helperFile.headerCFunctionsSS_);
            appendStream(ss, helperFile.headerCppIncludeSS_);
            appendStream(ss, helperFile.headerCppFunctionsSS_);

            writeBinary(helperFile.headerFileName_, UseHelper::convertToBuffer(ss.str()));
          }
          {
            std::stringstream ss;

            appendStream(ss, helperFile.cIncludeSS_);
            appendStream(ss, helperFile.cFunctionsSS_);
            appendStream(ss, helperFile.cppIncludeSS_);
            appendStream(ss, helperFile.cppFunctionsSS_);

            writeBinary(helperFile.cppFileName_, UseHelper::convertToBuffer(ss.str()));
          }
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr GenerateStructC::generateTypesHeader(ProjectPtr project) throw (Failure)
        {
          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          std::stringstream ss;

          ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
          ss << "#pragma once\n\n";
          ss << "\n";
          ss << "#include <stdint.h>\n\n";
          ss << "\n";

          ss << "#ifdef __cplusplus\n";
          ss << "#define " << getApiGuardDefine(project) << "    extern \"C\" {\n";
          ss << "#define " << getApiGuardDefine(project, true) << "      }\n";
          ss << "#else /* __cplusplus */\n";
          ss << "#include <stdbool.h>\n";
          ss << "#define " << getApiGuardDefine(project) << "\n";
          ss << "#define " << getApiGuardDefine(project, true) << "\n";
          ss << "#endif /* __cplusplus */\n";
          ss << "\n";

          ss << "#ifndef " << getApiExportDefine(project) << "\n";
          ss << "#ifdef " << getApiImplementationDefine(project) << "\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiExportDefine(project) << " __declspec(dllexport)\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiExportDefine(project) << " __attribute__((visibility(\"default\")))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#else /* "<< getApiImplementationDefine(project) << " */\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiExportDefine(project) << " __declspec(dllimport)\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiExportDefine(project) << " __attribute__((visibility(\"default\")))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#endif /* " << getApiImplementationDefine(project) << " */\n";
          ss << "#endif /* ndef " << getApiExportDefine(project) << " */\n";
          ss << "\n";

          ss << "#ifndef " << getApiCallingDefine(project) << "\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiCallingDefine(project) << " __stdcall\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiCallingDefine(project) << " __attribute__((cdecl))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#endif /* ndef " << getApiCallingDefine(project) << " */\n";
          ss << "\n";

          ss << getApiGuardDefine(project) << "\n";
          ss << "\n";
          ss << "typedef bool bool_t;\n";
          ss << "typedef signed char schar_t;\n";
          ss << "typedef unsigned char uchar_t;\n";
          ss << "typedef signed short short_t;\n";
          ss << "typedef unsigned short ushort_t;\n";
          ss << "typedef signed int sint_t;\n";
          ss << "typedef unsigned int uint_t;\n";
          ss << "typedef signed long slong_t;\n";
          ss << "typedef unsigned long ulong_t;\n";
          ss << "typedef signed long long sllong_t;\n";
          ss << "typedef unsigned long long ullong_t;\n";
          ss << "typedef float float_t;\n";
          ss << "typedef double double_t;\n";
          ss << "typedef float float32_t;\n";
          ss << "typedef double float64_t;\n";
          ss << "typedef long double ldouble_t;\n";
          ss << "typedef uintptr_t binary_t;\n";
          ss << "typedef uintptr_t string_t;\n";
          ss << "\n";

          processTypesNamespace(ss, project->mGlobal);

          ss << "\n";
          processTypesTemplatesAndSpecials(ss, project);
          ss << "\n";
          ss << getApiGuardDefine(project, true) << "\n";

          ss << "\n";
          ss << "#ifdef __cplusplus\n";
          ss << "#include \"../types.h\"\n";
          ss << "#endif /* __cplusplus */\n";

          return UseHelper::convertToBuffer(ss.str());
        }
        
        //---------------------------------------------------------------------
        void GenerateStructC::processTypesNamespace(
                                                    std::stringstream &ss,
                                                    NamespacePtr namespaceObj
                                                    )
        {
          if (!namespaceObj) return;
          if (namespaceObj->hasModifier(Modifier_Special)) return;

          bool firstNamespace {true};
          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            processTypesNamespace(ss, subNamespaceObj);
          }

          processTypesEnum(ss, namespaceObj);

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto structObj = (*iter).second;
            processTypesStruct(ss, structObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesStruct(
                                                  std::stringstream &ss,
                                                  StructPtr structObj
                                                  )
        {
          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          ss << "typedef uintptr_t " << fixCType(structObj) << ";\n";

          processTypesEnum(ss, structObj);

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processTypesStruct(ss, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesEnum(
                                               std::stringstream &ss,
                                               ContextPtr context
                                               )
        {
          auto namespaceObj = context->toNamespace();
          auto structObj = context->toStruct();
          if ((!namespaceObj) && (!structObj)) return;

          bool found = false;

          auto &enums = namespaceObj ? (namespaceObj->mEnums) : (structObj->mEnums);
          for (auto iter = enums.begin(); iter != enums.end(); ++iter)
          {
            auto enumObj = (*iter).second;
            ss << "typedef " << fixCType(enumObj->mBaseType) << " " << fixCType(enumObj) << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesTemplatesAndSpecials(
                                                               std::stringstream &ss,
                                                               ProjectPtr project
                                                               )
        {
          if (!project) return;

          ContextPtr context = project;
          processTypesTemplate(ss, context->findType("::std::list"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::std::map"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::std::set"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::zs::PromiseWith"));
          ss << "\n";

          processTypesSpecialStruct(ss, context->findType("::zs::Any"));
          processTypesSpecialStruct(ss, context->findType("::zs::Promise"));
          processTypesSpecialStruct(ss, context->findType("::zs::Exception"));
          processTypesSpecialStruct(ss, context->findType("::zs::InvalidArgument"));
          processTypesSpecialStruct(ss, context->findType("::zs::BadState"));
          processTypesSpecialStruct(ss, context->findType("::zs::NotImplemented"));
          processTypesSpecialStruct(ss, context->findType("::zs::NotSupported"));
          processTypesSpecialStruct(ss, context->findType("::zs::UnexpectedError"));
          ss << "\n";

          processTypesSpecialStruct(ss, context->findType("::zs::Time"));
          processTypesSpecialStruct(ss, context->findType("::zs::Milliseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Microseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Nanoseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Seconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Minutes"));
          processTypesSpecialStruct(ss, context->findType("::zs::Hours"));
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesTemplate(
                                                   std::stringstream &ss,
                                                   ContextPtr structContextObj
                                                   )
        {
          if (!structContextObj) return;

          auto structObj = structContextObj->toStruct();
          if (!structObj) return;

          if (structObj->mGenerics.size() < 1) return;

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter) {
            auto templatedStruct = (*iter).second;
            ss << "typedef uintptr_t " << fixCType(templatedStruct) << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesSpecialStruct(
                                                        std::stringstream &ss,
                                                        ContextPtr structContextObj
                                                        )
        {
          if (!structContextObj) return;

          auto structObj = structContextObj->toStruct();
          if (!structObj) return;

          if (!structObj->hasModifier(Modifier_Special)) return;

          ss << "typedef uintptr_t " << fixCType(structObj) << ";\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructC::targetKeyword()
        {
          return String("c");
        }

        //---------------------------------------------------------------------
        String GenerateStructC::targetKeywordHelp()
        {
          return String("Generate C wrapper");
        }

        //---------------------------------------------------------------------
        void GenerateStructC::targetOutput(
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

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("c"));
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
          helperFile.global_ = project->mGlobal;

          calculateRelations(project->mGlobal, helperFile.derives_);

          helperFile.cppFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("c_helpers.cpp"));
          helperFile.headerFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("c_helpers.h"));

          prepareHelperFile(helperFile);

          finalizeHelperFile(helperFile);
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
