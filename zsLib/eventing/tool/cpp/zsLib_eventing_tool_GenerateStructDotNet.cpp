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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructDotNet.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructC.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructCx.h>
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
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet::ApiFile
        #pragma mark

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::usingTypedef(
                                                          const String &usingType,
                                                          const String &originalType
                                                          )
        {
          if (alreadyUsing_.end() != alreadyUsing_.find(usingType)) return;
          alreadyUsing_.insert(usingType);

          auto &ss = (originalType.hasData() ? usingAliasSS_ : usingNamespaceSS_);
          ss << "using " << usingType;
          if (originalType.hasData()) {
            ss << " = " << originalType;
          }
          ss << ";\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::usingTypedef(TypePtr type)
        {
          if (!type) return;

          {
            auto basicType = type->toBasicType();
            if (basicType) return;
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) return;
              usingTypedef(GenerateStructC::fixCType(type), "System.IntPtr");
              return;
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              usingTypedef(GenerateStructC::fixCType(type), "System.IntPtr");
              return;
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto systemType = fixCsSystemType(enumType->mBaseType);
              if (systemType.hasData()) {
                usingTypedef(GenerateStructC::fixCType(type), systemType);
              }
            }
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructDotNet::BaseFile::hasBoxing(const String &namePathStr)
        {
          auto found = boxings_.find(namePathStr);
          return found != boxings_.end();
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::startRegion(const String &region)
        {
          auto dash = GenerateHelper::getDashedComment(indent_);

          auto &ss = structSS_;
          ss << "\n";
          ss << indent_ << "#region " << region << "\n";
          ss << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "// " << region << "\n";
          ss << dash;
          ss << dash;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::endRegion(const String &region)
        {
          auto &ss = structSS_;
          ss << "\n";
          ss << indent_ << "#endregion // " << region << "\n";
          ss << "\n";
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::startOtherRegion(
                                                              const String &region,
                                                              bool preStruct
                                                              )
        {
          auto dash = GenerateHelper::getDashedComment(indent_);

          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << "\n";
          ss << indent_ << "#region " << region << "\n";
          ss << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "// " << region << "\n";
          ss << dash;
          ss << dash;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::endOtherRegion(
                                                            const String &region,
                                                            bool preStruct
                                                            )
        {
          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << "\n";
          ss << indent_ << "#endregion // " << region << "\n";
          ss << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructDotNet::StructFile::StructFile(StructPtr structObj) :
          interfaceSS_(preStructSS_),
          interfaceEndSS_(preStructEndSS_),
          delegateSS_(structDeclationsSS_),
          struct_(structObj),
          isStaticOnly_(GenerateHelper::hasOnlyStaticMethods(structObj)),
          hasEvents_(GenerateHelper::hasEventHandlers(structObj)),
          isDictionary(structObj->hasModifier(Modifier_Struct_Dictionary))
        {
          if ((!isStaticOnly_) &&
              (!isDictionary)) {
            auto parent = structObj->getParent();
            if (parent) {
              Context::FindTypeOptions options;
              options.mSearchParents = false;
              auto found = parent->findType("I" + GenerateStructCx::fixStructName(structObj), &options);
              shouldDefineInterface_ = (!((bool)found));
            }
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructDotNet::GenerateStructDotNet() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructDotNetPtr GenerateStructDotNet::create()
        {
          return make_shared<GenerateStructDotNet>();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getMarshalAsType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "I1";
            case PredefinedTypedef_uchar:       
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       
            case PredefinedTypedef_ushort:      
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      
            case PredefinedTypedef_uint:        
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        
            case PredefinedTypedef_ulong:       
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       
            case PredefinedTypedef_ulonglong:   
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   
            case PredefinedTypedef_uint8:       
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       
            case PredefinedTypedef_uint16:      
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      
            case PredefinedTypedef_uint32:      
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      
            case PredefinedTypedef_uint64:      
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      

            case PredefinedTypedef_byte:        
            case PredefinedTypedef_word:        
            case PredefinedTypedef_dword:       
            case PredefinedTypedef_qword:       

            case PredefinedTypedef_float:       
            case PredefinedTypedef_double:      
            case PredefinedTypedef_ldouble:     
            case PredefinedTypedef_float32:     
            case PredefinedTypedef_float64:     

            case PredefinedTypedef_pointer:     

            case PredefinedTypedef_binary:      
            case PredefinedTypedef_size:        

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     break;
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCCsType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        return "void";
            case PredefinedTypedef_bool:        return "bool";
            case PredefinedTypedef_uchar:       return "byte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "sbyte";
            case PredefinedTypedef_ushort:      return "ushort";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "short";
            case PredefinedTypedef_uint:        return "uint";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "int";
            case PredefinedTypedef_ulong:       return "ulong";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "long";
            case PredefinedTypedef_ulonglong:   return "ulong";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "long";
            case PredefinedTypedef_uint8:       return "byte";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "sbyte";
            case PredefinedTypedef_uint16:      return "ushort";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "short";
            case PredefinedTypedef_uint32:      return "uint";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "int";
            case PredefinedTypedef_uint64:      return "ulong";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "long";

            case PredefinedTypedef_byte:        return "byte";
            case PredefinedTypedef_word:        return "ushort";
            case PredefinedTypedef_dword:       return "uint";
            case PredefinedTypedef_qword:       return "ulong";

            case PredefinedTypedef_float:       return "float";
            case PredefinedTypedef_double:      return "double";
            case PredefinedTypedef_ldouble:     return "double";
            case PredefinedTypedef_float32:     return "float";
            case PredefinedTypedef_float64:     return "double";

            case PredefinedTypedef_pointer:     return "raw_pointer_t";

            case PredefinedTypedef_binary:      return "binary_t";
            case PredefinedTypedef_size:        return "binary_size_t";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "string_t";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCCsType(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return fixCCsType(basicType->mBaseType);
          }
          return GenerateStructC::fixCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsType(
                                               TypePtr type,
                                               bool isInterface
                                               )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case PredefinedTypedef_pointer:     return "System.IntPtr";

                case PredefinedTypedef_binary:      return "byte[]";
                case PredefinedTypedef_size:        return "ulong";

                case PredefinedTypedef_string:
                case PredefinedTypedef_astring:
                case PredefinedTypedef_wstring:     return "string";
                default:                            break;
              }
              return fixCCsType(basicType->toBasicType());
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) return GenerateStructCx::fixName(enumType->getMappingName());
          }
          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) {
                String specialName = structType->getPathName();
                if ("::zs::Time" == specialName) return "System.DateTimeOffset";
                if ("::zs::Nanoseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Microseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Milliseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Seconds" == specialName) return "System.TimeSpan";
                if ("::zs::Minutes" == specialName) return "System.TimeSpan";
                if ("::zs::Hours" == specialName) return "System.TimeSpan";

                if ("::zs::exceptions::Exception" == specialName) return "System.Exception";
                if ("::zs::exceptions::InvalidArgument" == specialName) return "System.ArgumentException";
                if ("::zs::exceptions::BadState" == specialName) return "System.Runtime.InteropServices.COMException";
                if ("::zs::exceptions::NotImplemented" == specialName) return "System.NotImplementedException";
                if ("::zs::exceptions::NotSupported" == specialName) return "System.NotSupportedException";
                if ("::zs::exceptions::UnexpectedError" == specialName) return "System.Runtime.InteropServices.COMException";

                if ("::zs::Any" == specialName) return "object";
                if ("::zs::Promise" == specialName) return "System.Threading.Tasks.Task";
              }

              if (isInterface) {
                if (hasInterface(structType)) {
                  return "I" + GenerateStructCx::fixStructName(structType);
                }
              }
              return GenerateStructCx::fixStructName(structType);
            }
          }
          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              auto parent = templatedType->getParent();
              if (parent) {
                auto parentStruct = parent->toStruct();
                if (parentStruct) {
                  String prefixStr;
                  String postFixStr = ">";
                  String specialName = parentStruct->getPathName();
                  if ("::zs::PromiseWith" == specialName) prefixStr = "System.Threading.Tasks.Task<";
                  if ("::std::list" == specialName) prefixStr = "System.Collections.Generic.IReadOnlyList<";
                  if ("::std::set" == specialName) {
                    prefixStr = "System.Collections.Generic.IReadOnlyDictionary<";
                    postFixStr = ", object>";
                  }
                  if ("::std::map" == specialName) prefixStr = "System.Collections.Generic.IReadOnlyDictionary<";

                  String argsStr;
                  for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                    auto subType = (*iter);
                    if (argsStr.hasData()) {
                      argsStr += ", ";
                    }
                    argsStr += fixCsPathType(subType, isInterface);
                  }

                  if ((">" == postFixStr.substr(0, 1)) &&
                      (argsStr.length() > 0) &&
                      (">" == argsStr.substr(argsStr.length() - 1, 1))) {
                    argsStr += " ";
                  }

                  return prefixStr + argsStr + postFixStr;
                }
              }
            }
          }

          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsPathType(
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          if (!type) return String();

          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) return fixCsType(type, isInterface);

              if (isInterface) {
                if (hasInterface(structType)) {
                  auto parent = structType->getParent();
                  auto result = GenerateStructCx::fixNamePath(parent);
                  if (result.substr(0, 2) == "::") result = result.substr(2);
                  result.replaceAll("::", ".");
                  if (result.hasData()) {
                    result += ".";
                  }
                  result += "I" + GenerateStructCx::fixStructName(structType);
                  return result;
                }
              }

              auto result = GenerateStructCx::fixNamePath(structType);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::",".");
              return result;
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto parent = enumType->getParent();
              auto result = GenerateStructCx::fixNamePath(parent);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::", ".");
              if (result.hasData()) {
                result += ".";
              }
              result += GenerateStructCx::fixName(enumType->getMappingName());
              return result;
            }
          }

          return fixCsType(type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsType(
                                               bool isOptional,
                                               TypePtr type,
                                               bool isInterface
                                               )
        {
          auto result = fixCsType(type, isInterface);
          if (!isOptional) return result;

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case PredefinedTypedef_binary:      return result;
                case PredefinedTypedef_string:
                case PredefinedTypedef_astring:
                case PredefinedTypedef_wstring:     return result;
                default:                            break;
              }
              return result + "?";
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return result + "?";
            }
          }

          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsPathType(
                                                   bool isOptional,
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          if (!isOptional) return fixCsPathType(type, isInterface);
          if (type->toStruct()) return fixCsPathType(type, isInterface);
          return fixCsType(isOptional, type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsSystemType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        return String();
            case PredefinedTypedef_bool:        return "System.Boolean";
            case PredefinedTypedef_uchar:       return "System.Byte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "System.SByte";
            case PredefinedTypedef_ushort:      return "System.UInt16";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "System.Int16";
            case PredefinedTypedef_uint:        return "System.UInt32";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "System.Int32";
            case PredefinedTypedef_ulong:       return "System.UInt64";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "System.Int64";
            case PredefinedTypedef_ulonglong:   return "System.UInt64";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "System.Int64";
            case PredefinedTypedef_uint8:       return "System.Byte";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "System.SByte";
            case PredefinedTypedef_uint16:      return "System.UInt16";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "System.Int16";
            case PredefinedTypedef_uint32:      return "System.UInt32";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "System.Int32";
            case PredefinedTypedef_uint64:      return "System.UInt64";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "System.Int64";

            case PredefinedTypedef_byte:        return "System.Byte";
            case PredefinedTypedef_word:        return "System.UInt16";
            case PredefinedTypedef_dword:       return "System.UInt32";
            case PredefinedTypedef_qword:       return "System.UInt64";

            case PredefinedTypedef_float:       return "System.Single";
            case PredefinedTypedef_double:      return "System.Double";
            case PredefinedTypedef_ldouble:     return "System.Double";
            case PredefinedTypedef_float32:     return "System.Single";
            case PredefinedTypedef_float64:     return "System.Double";

            case PredefinedTypedef_pointer:     return "System.IntPtr";

            case PredefinedTypedef_binary:      return "System.IntPtr";
            case PredefinedTypedef_size:        return "System.UInt64";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "System.IntPtr";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::combineIf(
                                               const String &combinePreStr,
                                               const String &combinePostStr,
                                               const String &ifHasValue
                                               )
        {
          if (ifHasValue.isEmpty()) return String();
          return combinePreStr + ifHasValue + combinePostStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getReturnMarshal(
                                                      IEventingTypes::PredefinedTypedefs type,
                                                      const String &indentStr
                                                      )
        {
          return combineIf(indentStr + "[return: MarshalAs(UnmanagedType.", ")]\n", getMarshalAsType(type));
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getParamMarshal(IEventingTypes::PredefinedTypedefs type)
        {
          return combineIf("[MarshalAs(UnmanagedType.", ")] ", getMarshalAsType(type));
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getReturnMarshal(
                                                      TypePtr type,
                                                      const String &indentStr
                                                      )
        {
          if (!type) return String();
          {
            auto basicType = type->toBasicType();
            if (basicType) return getReturnMarshal(basicType->mBaseType, indentStr);
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getParamMarshal(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return getParamMarshal(basicType->mBaseType);
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getHelpersMethod(
                                                      BaseFile &baseFile,
                                                      const String &methodName,
                                                      bool isOptional,
                                                      TypePtr type
                                                      )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              String cTypeStr = GenerateStructC::fixBasicType(basicType->mBaseType);
              if (isOptional) {
                return getApiPath(baseFile) + ".box_" + cTypeStr + methodName;
              }
              if ("string_t" == cTypeStr) return getApiPath(baseFile) + "." + cTypeStr + methodName;
              if ("binary_t" == cTypeStr) return getApiPath(baseFile) + "." + cTypeStr + methodName;
              return String();
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              String cTypeStr = GenerateStructC::fixCType(enumType);
              if (isOptional) {
                return getApiPath(baseFile) + ".box_" + cTypeStr + methodName;
              }
              return getApiPath(baseFile) + "." + cTypeStr + methodName;
            }
          }

          return getHelperPath(baseFile) + "." + GenerateStructC::fixCType(type) + methodName;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getToCMethod(
                                                  BaseFile &baseFile,
                                                  bool isOptional,
                                                  TypePtr type
                                                  )
        {
          return getHelpersMethod(baseFile, "_ToC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getFromCMethod(
                                                    BaseFile &baseFile,
                                                    bool isOptional, 
                                                    TypePtr type
                                                    )
        {
          return getHelpersMethod(baseFile, "_FromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getAdoptFromCMethod(
                                                         BaseFile &baseFile,
                                                         bool isOptional, 
                                                         TypePtr type
                                                         )
        {
          return getHelpersMethod(baseFile, "_AdoptFromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getDestroyCMethod(
                                                       BaseFile &baseFile,
                                                       bool isOptional,
                                                       TypePtr type
                                                       )
        {
          return getHelpersMethod(baseFile, "_wrapperDestroy", isOptional, type);
        }

        //---------------------------------------------------------------------
        bool GenerateStructDotNet::hasInterface(StructPtr structObj)
        {
          if (!structObj) return false;
          if (structObj->hasModifier(Modifier_Struct_Dictionary)) return false;

          auto parent = structObj->getParent();
          if (!parent) return false;

          Context::FindTypeOptions options;
          auto found = parent->findType("I" + GenerateStructCx::fixStructName(structObj), &options);
          return !((bool)found);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getApiPath(BaseFile &apiFile)
        {
          return "Wrapper." + GenerateStructCx::fixName(apiFile.project_->getMappingName()) + ".Api";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getHelperPath(BaseFile &apiFile)
        {
          return "Wrapper." + GenerateStructCx::fixName(apiFile.project_->getMappingName()) + ".Helpers";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::finalizeBaseFile(BaseFile &apiFile)
        {
          std::stringstream ss;

          GenerateStructC::appendStream(ss, apiFile.usingNamespaceSS_);
          GenerateStructC::appendStream(ss, apiFile.usingAliasSS_);
          GenerateStructC::appendStream(ss, apiFile.namespaceSS_);
          GenerateStructC::appendStream(ss, apiFile.preStructSS_);
          GenerateStructC::appendStream(ss, apiFile.preStructEndSS_);
          GenerateStructC::appendStream(ss, apiFile.structDeclationsSS_);
          GenerateStructC::appendStream(ss, apiFile.structSS_);
          GenerateStructC::appendStream(ss, apiFile.structEndSS_);
          GenerateStructC::appendStream(ss, apiFile.postStructSS_);
          GenerateStructC::appendStream(ss, apiFile.postStructEndSS_);
          GenerateStructC::appendStream(ss, apiFile.namespaceEndSS_);

          writeBinary(apiFile.fileName_, UseHelper::convertToBuffer(ss.str()));
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiFile(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          {
            auto &ss = apiFile.usingNamespaceSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
          }

          {
            auto &ss = apiFile.namespaceSS_;

            ss << "namespace Wrapper\n";
            ss << "{\n";
            apiFile.indentMore();
            ss << indentStr << "namespace " << GenerateStructCx::fixName(apiFile.project_->getMappingName()) << "\n";
            ss << indentStr << "{\n";
            apiFile.indentMore();
          }

          {
            auto &ss = apiFile.helpersSS_;

            ss << indentStr << "internal static class Helpers\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    private const System.UInt32 E_NOT_VALID_STATE = 5023;\n";
            ss << indentStr << "    private const System.UInt32 CO_E_NOT_SUPPORTED = 0x80004021;\n";
            ss << indentStr << "    private const System.UInt32 E_UNEXPECTED = 0x8000FFFF;\n";
          }

          {
            auto &ss = apiFile.structSS_;

            String libNameStr = "lib" + GenerateStructCx::fixName(apiFile.project_->getMappingName());
            ss << indentStr << "internal static class Api\n";
            ss << indentStr << "{\n";
            apiFile.indentMore();
            ss << "#if __TVOS__ && __UNIFIED__\n";
            ss << indentStr << "private const string UseDynamicLib = \"@rpath/" << libNameStr << ".framework/libSkiaSharp\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __IOS__ && __UNIFIED__\n";
            ss << indentStr << "private const string UseDynamicLib = \"@rpath/" << libNameStr << ".framework/libSkiaSharp\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __ANDROID__\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".so\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __MACOS__\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dylib\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif DESKTOP\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dll\"; // redirect using .dll.config to '" << libNameStr << ".dylib' on OS X\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif WINDOWS_UWP\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dll\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif NET_STANDARD\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << "\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#else\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << "\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#endif\n";
            ss << "\n";
            ss << indentStr << "private const UnmanagedType UseBoolMashal = UnmanagedType.I1;\n";
            ss << indentStr << "private const UnmanagedType UseStringMarshal = UnmanagedType.LPStr;\n";
            ss << "\n";
          }

          apiFile.usingTypedef("binary_t", "System.IntPtr");
          apiFile.usingTypedef("string_t", "System.IntPtr");
          apiFile.usingTypedef("raw_pointer_t", "System.IntPtr");
          apiFile.usingTypedef("binary_size_t", "System.UInt64");

          prepareApiCallback(apiFile);
          prepareApiExceptions(apiFile);
          prepareApiBoxing(apiFile);
          prepareApiString(apiFile);
          prepareApiBinary(apiFile);
          prepareApiDuration(apiFile);

          prepareApiList(apiFile, "list");
          prepareApiList(apiFile, "set");
          prepareApiList(apiFile, "map");

          prepareApiSpecial(apiFile, "Any");
          prepareApiSpecial(apiFile, "Promise");

          preparePromiseWithValue(apiFile);
          preparePromiseWithRejectionReason(apiFile);

          prepareApiNamespace(apiFile, apiFile.global_);

          apiFile.startRegion("Struct API helpers");
          apiFile.startHelpersRegion("Struct helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::finalizeApiFile(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.endRegion("Struct API helpers");
          apiFile.endHelpersRegion("Struct helpers");
          apiFile.indentLess();

          {
            auto &ss = apiFile.helpersEndSS_;
            ss << "\n";
            ss << indentStr << "}\n";
          }

          {
            auto &ss = apiFile.structEndSS_;
            ss << "\n";
            ss << indentStr << "}\n";
          }

          {
            auto &ss = apiFile.namespaceEndSS_;

            ss << "\n";
            apiFile.indentLess();
            ss << "    } // namespace " << GenerateStructCx::fixName(apiFile.project_->getMappingName()) << "\n";
            apiFile.indentLess();
            ss << "} // namespace Wrapper\n";
          }

          finalizeBaseFile(apiFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiCallback(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("System.Runtime.InteropServices");
          apiFile.usingTypedef("generic_handle_t", "System.IntPtr");
          apiFile.usingTypedef("instance_id_t", "System.IntPtr");
          apiFile.usingTypedef("callback_event_t", "System.IntPtr");
          apiFile.usingTypedef("event_observer_t", "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;

            apiFile.startRegion("Callback and Event API helpers");

            static const char *callbackHelpers = 
              "// void wrapperCallbackFunction(callback_event_t handle);\n"
              "[UnmanagedFunctionPointer(UseCallingConvention)]\n"
              "public delegate void WrapperCallbackFunction(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_wrapperInstall([MarshalAs(UnmanagedType.FunctionPtr)] WrapperCallbackFunction function);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_wrapperObserverDestroy(event_observer_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_event_wrapperDestroy(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t callback_event_wrapperInstanceId(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static event_observer_t callback_event_get_observer(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseStringMarshal)]\n"
              "public extern static string callback_event_get_namespace(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseStringMarshal)]\n"
              "public extern static string callback_event_get_class(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseStringMarshal)]\n"
              "public extern static string callback_event_get_method(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static generic_handle_t callback_event_get_source(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t callback_event_get_source_instance_id(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static generic_handle_t callback_event_get_data(callback_event_t handle, int argumentIndex);\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, callbackHelpers);

            apiFile.endRegion("Callback and Event API helpers");
          }

          {
            auto &ss = apiFile.helpersSS_;

            apiFile.startHelpersRegion("Callback and Event helpers");

            static const char *callbackHelpers =
              "public delegate void FireEventDelegate(object target, string methodName, callback_event_t handle);\n"
              "\n"
              "private class EventManager\n"
              "{\n"
              "    private static EventManager singleton_ = new EventManager();\n"
              "\n"
              "    private object lock_ = new System.Object();\n"
              "\n"
              "    private class Target\n"
              "    {\n"
              "        public System.WeakReference<object> target_;\n"
              "        public FireEventDelegate delegate_;\n"
              "\n"
              "        public Target(object target, FireEventDelegate fireDelegate)\n"
              "        {\n"
              "            this.target_ = new System.WeakReference<object>(target);\n"
              "            this.delegate_ = fireDelegate;\n"
              "        }\n"
              "        public void FireEvent(string methodName, callback_event_t handle)\n"
              "        {\n"
              "            object target = null;\n"
              "            if (!this.target_.TryGetTarget(out target)) return;\n"
              "            this.delegate_(target, methodName, handle);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private class InstanceObservers\n"
              "    {\n"
              "        public System.Collections.Generic.List<Target> targets_ = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            var oldList = targets_;\n"
              "            var newList = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "            foreach(Target value in oldList)\n"
              "            {\n"
              "                object existingTarget = null;\n"
              "                if (!value.target_.TryGetTarget(out existingTarget)) continue;\n"
              "                if (System.Object.ReferenceEquals(existingTarget, target)) continue;\n"
              "                newList.Add(value);\n"
              "            }\n"
              "            newList.Add(new Target(target, targetCallback));\n"
              "            targets_ = newList;\n"
              "        }\n"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            object target)\n"
              "        {\n"
              "            var oldList = targets_;\n"
              "            var newList = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "            foreach (Target value in oldList)\n"
              "            {\n"
              "                object existingTarget = null;\n"
              "                if (!value.target_.TryGetTarget(out existingTarget)) continue;\n"
              "                if (System.Object.ReferenceEquals(existingTarget, target)) continue;\n"
              "                newList.Add(value);\n"
              "            }\n"
              "            targets_ = newList;\n"
              "        }\n"
              "\n"
              "        public System.Collections.Generic.List<Target> Targets { get { return targets_; } }\n"
              "    }\n"
              "\n"
              "    private class StructObservers\n"
              "    {\n"
              "        public System.Collections.Generic.Dictionary<instance_id_t, InstanceObservers> observers_ = new System.Collections.Generic.Dictionary<instance_id_t, InstanceObservers>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            instance_id_t source,\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            InstanceObservers observer = null;\n"
              "            if (!observers_.TryGetValue(source, out observer))\n"
              "            {\n"
              "                observer = new InstanceObservers();\n"
              "                this.observers_[source] = observer;\n"
              "            }\n"
              "            observer.ObserveEvents(target, targetCallback);\n"
              "        }\n"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            instance_id_t source,\n"
              "            object target)\n"
              "        {\n"
              "            InstanceObservers observer = null;\n"
              "            if (!observers_.TryGetValue(source, out observer)) return; \n"
              "            observer.ObserveEventsCancel(target);\n"
              "        }\n"
              "\n"
              "        public System.Collections.Generic.List<Target> GetTargets(instance_id_t source)\n"
              "        {\n"
              "            InstanceObservers observer;\n"
              "            if (observers_.TryGetValue(source, out observer)) { return observer.Targets; }\n"
              "            return null;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private class Structs\n"
              "    {"
              "        public System.Collections.Generic.Dictionary<string, StructObservers> observers_ = new System.Collections.Generic.Dictionary<string, StructObservers>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            string className,\n"
              "            instance_id_t source,\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            StructObservers observer = null;\n"
              "            if (!observers_.TryGetValue(className, out observer))\n"
              "            {\n"
              "                observer = new StructObservers();\n"
              "                this.observers_[className] = observer;\n"
              "            }"
              "            observer.ObserveEvents(source, target, targetCallback);\n"
              "        }"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            string className,\n"
              "            instance_id_t source,\n"
              "            object target)\n"
              "        {\n"
              "            StructObservers observer = null;\n"
              "            if (!observers_.TryGetValue(className, out observer)) return;\n"
              "            observer.ObserveEventsCancel(source, target);\n"
              "        }"
              "\n"
              "        public System.Collections.Generic.List<Target> GetTargets(\n"
              "            string className,\n"
              "            instance_id_t source)\n"
              "        {\n"
              "            StructObservers observer;\n"
              "            if (observers_.TryGetValue(className, out observer)) {\n"
              "                return observer.GetTargets(source);\n"
              "            }\n"
              "            return null;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private System.Collections.Generic.Dictionary<string, Structs> observers_;\n"
              "\n"
              "    public static EventManager Singleton { get { return singleton_; } }\n"
              "\n"
              "    private EventManager()\n"
              "    {\n"
              "        Wrapper.Ortc.Api.callback_wrapperInstall(HandleEvent);\n"
              "    }\n"
              "\n"
              "    public void ObserveEvents(\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        instance_id_t source,\n"
              "        object target,\n"
              "        FireEventDelegate targetCallback)\n"
              "    {\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer = null;\n"
              "            if (!observers_.TryGetValue(namespaceName, out observer))\n"
              "            {\n"
              "                observer = new Structs();\n"
              "                this.observers_[namespaceName] = observer;\n"
              "            }\n"
              "            observer.ObserveEvents(className, source, target, targetCallback);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    public void ObserveEventsCancel(\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        instance_id_t source,\n"
              "        object target)\n"
              "    {\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer = null;\n"
              "            if (!observers_.TryGetValue(namespaceName, out observer)) return;\n"
              "            observer.ObserveEventsCancel(className, source, target);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    public void HandleEvent(\n"
              "        callback_event_t handle,\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        string methodName,\n"
              "        instance_id_t source)\n"
              "    {\n"
              "        System.Collections.Generic.List<Target> targets = null;\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer;\n"
              "            if (observers_.TryGetValue(namespaceName, out observer)) {\n"
              "                targets = observer.GetTargets(className, source);\n"
              "            }\n"
              "        }\n"
              "        if (null == targets) return;\n"
              "        foreach (Target target in targets) { target.FireEvent(methodName, handle); }\n"
              "    }\n"
              "\n"
              "    private static void HandleEvent(callback_event_t handle)\n"
              "    {\n"
              "        if (System.IntPtr.Zero == handle) return;\n"
              "        string namespaceName = Wrapper.Ortc.Api.callback_event_get_namespace(handle);\n"
              "        string className = Wrapper.Ortc.Api.callback_event_get_class(handle);\n"
              "        string methodName = Wrapper.Ortc.Api.callback_event_get_method(handle);\n"
              "        var instanceId = Wrapper.Ortc.Api.callback_event_get_source_instance_id(handle);\n"
              "        Singleton.HandleEvent(handle, namespaceName, className, methodName, instanceId);\n"
              "        Wrapper.Ortc.Api.callback_event_wrapperDestroy(handle);\n"
              "    }\n"
              "}\n"
              "\n"
              "public static void ObserveEvents(\n"
              "    string namespaceName,\n"
              "    string className,\n"
              "    instance_id_t source,\n"
              "    object target,\n"
              "    FireEventDelegate targetCallback\n"
              "    )\n"
              "{\n"
              "    EventManager.Singleton.ObserveEvents(namespaceName, className, source, target, targetCallback);\n"
              "}\n"
              "\n"
              "public static void ObserveEventsCancel(\n"
              "    string namespaceName,\n"
              "    string className,\n"
              "    instance_id_t source,\n"
              "    object target)\n"
              "{\n"
              "    EventManager.Singleton.ObserveEventsCancel(namespaceName, className, source, target);\n"
              "}\n"
              ;

              GenerateHelper::insertBlob(ss, indentStr, callbackHelpers);

              apiFile.endHelpersRegion("Callback and Event helpers");
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiExceptions(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("exception_handle_t", "System.IntPtr");
          apiFile.usingTypedef("instance_id_t", "System.IntPtr");

          {
            apiFile.startRegion("Exception API helpers");

            auto &ss = apiFile.structSS_;

            static const char *exceptionHelpers = 
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static exception_handle_t exception_wrapperCreate_exception();\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void exception_wrapperDestroy(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t exception_wrapperInstanceId(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseBoolMashal)]\n"
              "public extern static bool exception_hasException(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseStringMarshal)]\n"
              "public extern static string exception_what(exception_handle_t handle);\n"
              "\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, exceptionHelpers);
          }

          {
            apiFile.startHelpersRegion("Exception helpers");

            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "public static System.Exception exception_FromC(exception_handle_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (!" << getApiPath(apiFile) << ".exception_hasException(handle)) return null;\n";
          }

          prepareApiExceptions(apiFile, "InvalidArgument");
          prepareApiExceptions(apiFile, "BadState");
          prepareApiExceptions(apiFile, "NotImplemented");
          prepareApiExceptions(apiFile, "NotSupported");
          prepareApiExceptions(apiFile, "UnexpectedError");
          prepareApiExceptions(apiFile, "Exception");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "    return new System.Exception(" << getApiPath(apiFile) << ".exception_what(handle));\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static System.Exception exception_AdoptFromC(exception_handle_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = exception_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".exception_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";

            apiFile.endHelpersRegion("Exception helpers");
          }

          {
            auto &ss = apiFile.structSS_;

            apiFile.endRegion("Exception API helpers");
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiExceptions(
                                                        ApiFile &apiFile,
                                                        const String &exceptionName
                                                        )
        {
          auto &indentStr = apiFile.indent_;
         
          auto context = apiFile.global_->toContext()->findType("::zs::exceptions::" + exceptionName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
            ss << indentStr << "public extern static bool exception_is_" << exceptionName << "(exception_handle_t handle);\n";
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "    if (" << getApiPath(apiFile) << ".exception_is_" << exceptionName << "(handle)) return new " << fixCsType(contextStruct) << "(" << getApiPath(apiFile) << ".exception_what(handle)";
            if ("BadState" == exceptionName)  ss << ", unchecked((System.Int32)E_NOT_VALID_STATE)";
            if ("UnexpectedError" == exceptionName)  ss << ", unchecked((System.Int32)E_UNEXPECTED)";
            ss << ");\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBoxing(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion("Boxing API helpers");
          apiFile.startHelpersRegion("Boxing helpers");

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_bool);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uchar);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_schar);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ushort);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sshort);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ulong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_slong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ulonglong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_slonglong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint8);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint8);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint16);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint16);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint32);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint32);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint64);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint64);

          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_byte);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_word);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_dword);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_qword);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_double);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ldouble);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float32);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float64);

          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_pointer);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_binary);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_size);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_string);

          apiFile.endRegion("Boxing API helpers");
          apiFile.endHelpersRegion("Boxing helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBoxing(
                                                    ApiFile &apiFile,
                                                    const IEventingTypes::PredefinedTypedefs basicType
                                                    )
        {
          auto &indentStr = apiFile.indent_;

          String cTypeStr = GenerateStructC::fixCType(basicType);
          String csTypeStr = fixCCsType(basicType);
          String boxedTypeStr = "box_" + cTypeStr;

          apiFile.usingTypedef(boxedTypeStr, "System.IntPtr");

          auto pathStr = GenerateStructC::fixBasicType(basicType);
          if (!apiFile.hasBoxing(pathStr)) return;

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(" << getParamMarshal(basicType) << csTypeStr << " value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void box_" << cTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t box_" << cTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static bool box_" << cTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << getReturnMarshal(basicType, indentStr);
            ss << indentStr << "public extern static " << csTypeStr << " box_" << cTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void box_" << cTypeStr << "_set_value(" << boxedTypeStr << " handle, " << getParamMarshal(basicType) << csTypeStr << " value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            if (IEventingTypes::PredefinedTypedef_binary == basicType) {
              ss << indentStr << "public static byte[] box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    var binaryHandle = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "    var result = " << cTypeStr << "_FromC(binaryHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(binaryHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static byte[] box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(byte[] value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    var tempHandle = " << cTypeStr << "_ToC(value);\n";
              ss << indentStr << "    var result = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
            } else if (IEventingTypes::getBaseType(basicType) == IEventingTypes::BaseType_String) {
              ss << indentStr << "public static string box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    var tempHandle = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "    var result = " << cTypeStr << "_FromC(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static string box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(string value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    var tempHandle = " << cTypeStr << "_ToC(value);\n";
              ss << indentStr << "    var result = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
            } else {
              ss << indentStr << "public static " << fixCCsType(basicType) << "? box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << fixCCsType(basicType) << " box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(" << fixCCsType(basicType) << "? value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr <<"WithValue(value.Value);\n";
              ss << indentStr << "}\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiString(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("string_t", "System.IntPtr");

          apiFile.startRegion("String API helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static string_t string_t_wrapperCreate_string_t();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static string_t string_t_wrapperCreate_string_tWithValue([MarshalAs(UseStringMarshal)] string value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void string_t_wrapperDestroy(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t string_t_wrapperInstanceId(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "[return: MarshalAs(UseStringMarshal)]\n";
            ss << indentStr << "public extern static string string_t_get_value(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void string_t_set_value(string_t handle, [MarshalAs(UseStringMarshal)] string value);\n";
            ss << "\n";
          }
          apiFile.endRegion("String API helpers");

          apiFile.startHelpersRegion("String helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static string string_t_FromC(string_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".string_t_get_value(handle);\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static string string_t_AdoptFromC(string_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = string_t_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".string_t_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static string_t string_t_ToC(string value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".string_t_wrapperCreate_string_tWithValue(value);\n";
            ss << indentStr << "}\n";
          }

          apiFile.endHelpersRegion("String helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBinary(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion("Binary API helpers");

          apiFile.usingTypedef("binary_t", "System.IntPtr");
          apiFile.usingTypedef("ConstBytePtr", "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_t binary_t_wrapperCreate_binary_t();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_t binary_t_wrapperCreate_binary_tWithValue(byte [] value, binary_size_t size);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void binary_t_wrapperDestroy(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t binary_t_wrapperInstanceId(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static ConstBytePtr binary_t_get_value(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_size_t binary_t_get_size(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void binary_t_set_value(binary_t handle, byte [] value, binary_size_t size);\n";
            ss << "\n";
          }
          apiFile.endRegion("Binary API helpers");

          apiFile.startHelpersRegion("Binary helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static byte[] binary_t_FromC(binary_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
            ss << indentStr << "    var length = " << getApiPath(apiFile) << ".binary_t_get_size(handle);\n";
            ss << indentStr << "    byte[] buffer = new byte[length];\n";
            ss << indentStr << "    Marshal.Copy(" << getApiPath(apiFile) << ".binary_t_get_value(handle), buffer, 0, checked((int)length));\n";
            ss << indentStr << "    return buffer;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static byte[] binary_t_AdoptFromC(binary_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = binary_t_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".binary_t_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static binary_t binary_t_ToC(byte[] value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".binary_t_wrapperCreate_binary_tWithValue(value, checked((ulong)value.Length));\n";
            ss << indentStr << "}\n";
          }

          apiFile.endHelpersRegion("Binary helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiDuration(ApiFile &apiFile)
        {
          apiFile.startRegion("Time API helpers");
          apiFile.startHelpersRegion("Time helpers");

          prepareApiDuration(apiFile, "Time");
          prepareApiDuration(apiFile, "Days");
          prepareApiDuration(apiFile, "Hours");
          prepareApiDuration(apiFile, "Seconds");
          prepareApiDuration(apiFile, "Minutes");
          prepareApiDuration(apiFile, "Milliseconds");
          prepareApiDuration(apiFile, "Microseconds");
          prepareApiDuration(apiFile, "Nanoseconds");

          apiFile.endRegion("Time helpers");
          apiFile.endHelpersRegion("Time helpers");
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiDuration(
                                                      ApiFile &apiFile,
                                                      const String &durationType
                                                      )
        {
          auto &indentStr = apiFile.indent_;

          bool isTime = "Time" == durationType;

          auto durationContext = apiFile.global_->toContext()->findType("::zs::" + durationType);
          if (!durationContext) return;

          auto durationStruct = durationContext->toStruct();
          if (!durationStruct) return;

          String cTypeStr = GenerateStructC::fixCType(durationStruct);

          apiFile.usingTypedef(cTypeStr, "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << cTypeStr << " zs_" << durationType << "_wrapperCreate_" << durationType << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << cTypeStr << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(long value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void zs_" << durationType << "_wrapperDestroy(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t zs_" << durationType << "_wrapperInstanceId(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "long zs_" << durationType << "_get_value(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void zs_" << durationType << "_set_value(" << cTypeStr << " handle, long value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static " << fixCsPathType(durationContext) << " zs_" << durationType << "_FromC(" << cTypeStr << " handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    ";
            if ("Time" == durationType) {
              ss << "return System.DateTimeOffset.FromFileTime(" << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle));\n";
            } else if ("Nanoseconds" == durationType) {
              ss << "return System.TimeSpan.FromTicks(System.TimeSpan.TicksPerMillisecond * " << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle) / 1000 / 1000);\n";
            } else if ("Microseconds" == durationType) {
              ss << "return System.TimeSpan.FromTicks(System.TimeSpan.TicksPerMillisecond * " << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle) / 1000);\n";
            } else {
              ss << "return System.TimeSpan.From" << durationType << "(" << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle));\n";
            }
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << fixCsPathType(durationContext) << " zs_" << durationType << "_AdoptFromC(" << cTypeStr << " handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "  var result = zs_" << durationType << "_FromC(handle);\n";
            ss << indentStr << "  " << getApiPath(apiFile) << "zs_" << durationType << "_wrapperDestroy(handle);\n";
            ss << indentStr << "  return result;\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << cTypeStr << " zs_" << durationType << "_ToC(" << fixCsPathType(durationContext) << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    ";
            if ("Time" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.ToFileTime());\n";
            } else if ("Nanoseconds" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.Ticks * 1000 * 1000 / System.TimeSpan.TicksPerMillisecond);\n";
            } else if ("Microseconds" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.Ticks * 1000 / System.TimeSpan.TicksPerMillisecond);\n";
            } else {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(unchecked((long)(value.Total" << durationType << ")));\n";
            }
            ss << indentStr << "}\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiList(
                                                  ApiFile &apiFile,
                                                  const String &listOrSetStr
                                                  )
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion(listOrSetStr + " helpers");

          bool isMap = ("map" == listOrSetStr);
          bool isList = ("list" == listOrSetStr);
          auto context = apiFile.global_->toContext()->findType("::std::" + listOrSetStr);
          if (!context) return;

          auto structType = context->toStruct();
          if (!structType) return;

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

            
            apiFile.usingTypedef("iterator_handle_t", "System.IntPtr");
            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(keyType);
            apiFile.usingTypedef(listType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(templatedStructType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "();\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static void " << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static instance_id_t " << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              if (isMap) {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentStr << "public extern static " << "void " << GenerateStructC::fixType(templatedStructType) << "_insert(" << GenerateStructC::fixCType(templatedStructType) << " handle, " << getParamMarshal(keyType) << fixCCsType(keyType) << " key, " << getParamMarshal(listType) << fixCCsType(listType) << " value);\n";
              } else {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentStr << "public extern static " << "void " << GenerateStructC::fixType(templatedStructType) << "_insert(" << GenerateStructC::fixCType(templatedStructType) << " handle, " << getParamMarshal(listType) << fixCCsType(listType) << " value);\n";
              }

              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static iterator_handle_t " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static void " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterator_handle_t iterHandle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(iterator_handle_t iterHandle);\n";
              if (isMap) {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << getReturnMarshal(keyType, indentStr);
                ss << indentStr << "public extern static " << fixCCsType(keyType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey(iterator_handle_t iterHandle);\n";
              }
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << getReturnMarshal(listType, indentStr);
              ss << indentStr << "public extern static " << fixCCsType(listType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue(iterator_handle_t iterHandle);\n";
              ss << "\n";
            }
          }
          apiFile.endRegion(listOrSetStr + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiSpecial(
                                                     ApiFile &apiFile,
                                                     const String &specialName
                                                     )
        {
          auto &indentStr = apiFile.indent_;

          bool isPromise = "Promise" == specialName;

          auto context = apiFile.global_->toContext()->findType("::zs::" + specialName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.usingTypedef(GenerateStructC::fixCType(contextStruct), "System.IntPtr");

          apiFile.startRegion(specialName + " helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void zs_" << specialName << "_wrapperDestroy(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t zs_" << specialName << "_wrapperInstanceId(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            if (isPromise) {
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static event_observer_t zs_" << specialName << "_wrapperObserveEvents(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static ulong zs_" << specialName << "_get_id(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isSettled(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isResolved(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isRejected(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            }
            ss << "\n";
          }
          apiFile.endRegion(specialName + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::preparePromiseWithValue(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          auto context = apiFile.global_->toContext()->findType("::zs::PromiseWith");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.startRegion("PromiseWith helpers");

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(promiseType) << " zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
          }

          apiFile.endRegion("PromiseWith helpers");
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::preparePromiseWithRejectionReason(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          auto context = apiFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.startRegion("PromiseWithRejectionReason helpers");

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(promiseType) << " zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
          }
          apiFile.endRegion("PromiseWithRejectionReason helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiNamespace(
                                                       ApiFile &apiFile,
                                                       NamespacePtr namespaceObj
                                                       )
        {
          if (!namespaceObj) return;

          if (namespaceObj == apiFile.global_) {
            apiFile.startRegion("enum API helpers");
            apiFile.startHelpersRegion("enum helpers");
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            prepareApiNamespace(apiFile, subNamespaceObj);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareApiStruct(apiFile, subStructObj);
          }
          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareApiEnum(apiFile, subEnumObj);
          }

          if (namespaceObj == apiFile.global_) {
            apiFile.endRegion("enum API helpers");
            apiFile.endHelpersRegion("enum helpers");
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiStruct(
                                                    ApiFile &apiFile,
                                                    StructPtr structObj
                                                    )
        {
          if (!structObj) return;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareApiStruct(apiFile, subStructObj);
          }
          for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareApiEnum(apiFile, subEnumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiEnum(
                                                  ApiFile &apiFile,
                                                  EnumTypePtr enumObj
                                                  )
        {
          auto &indentStr = apiFile.indent_;

          if (!enumObj) return;
          bool hasBoxing = apiFile.hasBoxing(enumObj->getPathName());

          String cTypeStr = GenerateStructC::fixCType(enumObj);
          String boxedTypeStr = "box_" + cTypeStr;
          String csTypeStr = fixCsPathType(enumObj);

          apiFile.usingTypedef(cTypeStr, fixCsSystemType(enumObj->mBaseType));
          apiFile.usingTypedef(boxedTypeStr, "System.IntPtr");

          if (hasBoxing)
          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(" << fixCCsType(enumObj->mBaseType) << " value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void " << boxedTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t " << boxedTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
            ss << indentStr << "public extern static " << "bool " << boxedTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << fixCCsType(enumObj->mBaseType) << " " << boxedTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void " << boxedTypeStr << "_set_value(" << boxedTypeStr << " handle, " << fixCCsType(enumObj->mBaseType) << " value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static " << csTypeStr << " " << cTypeStr << "_FromC(" << cTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << csTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << csTypeStr << " " << cTypeStr << "_AdoptFromC(" << cTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << csTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << cTypeStr << " " << cTypeStr << "_ToC(" << csTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << cTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            if (hasBoxing) {
              ss << "\n";
              ss << indentStr << "public static " << csTypeStr << "? " << boxedTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << "." << boxedTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    return unchecked((" << csTypeStr << ")" << getApiPath(apiFile) << "." << boxedTypeStr << "_get_value(handle));\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << csTypeStr << "? " << boxedTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = " << boxedTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << boxedTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " " << boxedTypeStr << "_ToC(" << csTypeStr << "? value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << "." << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(unchecked((" << cTypeStr << ")value.Value));\n";
              ss << indentStr << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareEnumFile(EnumFile &enumFile)
        {
          auto &indentStr = enumFile.indent_;

          {
            auto &ss = enumFile.usingNamespaceSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
          }

          prepareEnumNamespace(enumFile, enumFile.global_);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareEnumNamespace(
                                                        EnumFile &enumFile,
                                                        NamespacePtr namespaceObj
                                                        )
        {
          auto &indentStr = enumFile.indent_;
          auto &ss = enumFile.namespaceSS_;

          if (!namespaceObj) return;

          if (!namespaceObj->isGlobal()) {
            ss << "\n";
            ss << indentStr << "namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            ss << indentStr << "{\n";
            enumFile.indentMore();
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;

            if (namespaceObj->isGlobal()) {
              auto name = subNamespaceObj->getMappingName();
              if ("std" == name) continue;
              if ("zs" == name) continue;
            }

            prepareEnumNamespace(enumFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto enumObj = (*iter).second;
            if (!enumObj) continue;

            ss << "\n";
            ss << indentStr << "public enum " << GenerateStructCx::fixName(enumObj->getMappingName());
            if (enumObj->mBaseType != PredefinedTypedef_int) {
              ss << " : " << fixCCsType(enumObj->mBaseType);
            }
            ss << "\n";
            ss << indentStr << "{\n";
            enumFile.indentMore();

            bool first = true;
            for (auto iter = enumObj->mValues.begin(); iter != enumObj->mValues.end(); ++iter) {
              auto valueObj = (*iter);

              if (!first) ss << ",\n";
              first = false;
              ss << indentStr << GenerateStructCx::fixName(valueObj->getMappingName());
              if (valueObj->mValue.hasData()) {
                ss << " = " << valueObj->mValue;
              }
            }
            if (!first) ss << "\n";

            enumFile.indentLess();
            ss << indentStr << "}\n";
          }

          if (!namespaceObj->isGlobal()) {
            enumFile.indentLess();
            auto &ss = enumFile.namespaceSS_;
            ss << indentStr << "} // namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processNamespace(
                                                    ApiFile &apiFile,
                                                    NamespacePtr namespaceObj
                                                    )
        {
          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            processNamespace(apiFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processStruct(apiFile, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processStruct(
                                                 ApiFile &apiFile,
                                                 StructPtr structObj
                                                 )
        {
          typedef std::list<NamespacePtr> NamespaceList;

          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          StructFile structFile(structObj);
          structFile.project_ = apiFile.project_;
          structFile.global_ = apiFile.global_;
          structFile.fileName_ = UseHelper::fixRelativeFilePath(apiFile.fileName_, "net_" + GenerateStructC::fixType(structObj) + ".cs");

          auto &indentStr = structFile.indent_;

          NamespaceList namespaceList;

          {
            auto parent = structObj->getParent();
            while (parent) {
              auto namespaceObj = parent->toNamespace();
              if (namespaceObj) {
                if (!namespaceObj->isGlobal()) {
                  namespaceList.push_front(namespaceObj);
                }
              }
              parent = parent->getParent();
            }
          }

          {
            auto &ss = structFile.namespaceSS_;

            for (auto iter = namespaceList.begin(); iter != namespaceList.end(); ++iter) {
              auto namespaceObj = (*iter);
              ss << indentStr << "namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
              ss << indentStr << "{\n";
              structFile.indentMore();
            }
          }

          if (structFile.shouldDefineInterface_) {
            auto &ss = structFile.interfaceSS_;
            ss << indentStr << "public interface I" << GenerateStructCx::fixStructName(structObj) << "\n";
            ss << indentStr << "{\n";
          }

          String cTypeStr = GenerateStructC::fixCType(structObj);
          String csTypeStr = GenerateStructCx::fixStructName(structObj);

          {
            auto &ss = structFile.structSS_;
            ss << indentStr << "public " << (structFile.isStaticOnly_ ? "static " : "sealed ") << "class " << csTypeStr << (structFile.isStaticOnly_ ? "" : " : System.IDisposable");

            if (!structFile.isStaticOnly_) {
              String indentPlusStr;
              size_t spaceLength = indentStr.length() + strlen("public sealed class  : ") + csTypeStr.length();
              indentPlusStr.reserve(spaceLength);
              for (size_t index = 0; index < spaceLength; ++index) { indentPlusStr += " "; }
              processInheritance(apiFile, structFile, structObj, indentPlusStr);
            }
            ss << "\n";
            ss << indentStr << "{\n";
          }

          structFile.indentMore();

          if (!structFile.isStaticOnly_)
          {
            structFile.usingTypedef(structObj);
            structFile.usingTypedef("instance_id_t", "System.IntPtr");

            {
              auto &ss = structFile.structSS_;
              structFile.startRegion("To / From C routines");
              ss << indentStr << "private class WrapperMakePrivate {}\n";
              ss << indentStr << "private " << cTypeStr << " native_ = System.IntPtr.Zero;\n";
              ss << "\n";
              ss << indentStr << "private " << csTypeStr << "(WrapperMakePrivate ignored, " << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    this.native_ = handle;\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    WrapperObserveEvents();\n";
              }
              ss << indentStr << "}\n";
              ss << "\n";
              ss << "\n";
              ss << indentStr << "public void Dispose()\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    Dispose(true);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "private void Dispose(bool disposing)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == this.native_) return;\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    WrapperObserveEventsCancel();\n";
              }
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(this.native_);\n";
              ss << indentStr << "    this.native_ = System.IntPtr.Zero;\n";
              ss << indentStr << "    if (disposing) System.GC.SuppressFinalize(this);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "~" << csTypeStr << "()\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    Dispose(false);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << csTypeStr << " " << cTypeStr << "_FromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    return new " << csTypeStr << "((WrapperMakePrivate)null, " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperClone(handle));\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << csTypeStr << " " << cTypeStr << "_AdoptFromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    return new " << csTypeStr << "((WrapperMakePrivate)null, handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << cTypeStr << " " << cTypeStr << "_ToC(" << csTypeStr << " value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return value.native_;\n";
              ss << indentStr << "}\n";
              structFile.endRegion("To / From C routines");
            }
            {
              apiFile.usingTypedef(structObj);
              apiFile.usingTypedef("instance_id_t", "System.IntPtr");

              auto &indentStr = apiFile.indent_;
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << cTypeStr << " " << cTypeStr << "_wrapperClone(" << cTypeStr << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static void " << cTypeStr << "_wrapperDestroy(" << cTypeStr << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static instance_id_t " << cTypeStr << "_wrapperInstanceId(" << cTypeStr << " handle);\n";
            }
            {
              auto &indentStr = apiFile.indent_;
              auto &ss = apiFile.helpersSS_;
              ss << "\n";
              ss << indentStr << "public static " << fixCsPathType(structObj) << " " << cTypeStr << "_FromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    return " << fixCsPathType(structObj) << "." << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << fixCsPathType(structObj) << " " << cTypeStr << "_AdoptFromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    return " << fixCsPathType(structObj) << "." << cTypeStr << "_AdoptFromC(handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << cTypeStr << " " << cTypeStr << "_ToC(" << fixCsPathType(structObj) << " value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    return " << fixCsPathType(structObj) << "." << cTypeStr << "_ToC(value);\n";
              ss << indentStr << "}\n";
            }
          }

          processStruct(apiFile, structFile, structObj, structObj);

          structFile.indentLess();

          if (structFile.shouldDefineInterface_) {
            auto &ss = structFile.interfaceEndSS_;
            ss << indentStr << "}\n";
          }
          {
            auto &ss = structFile.structEndSS_;
            ss << indentStr << "}\n";
          }

          {
            auto &ss = structFile.namespaceEndSS_;
            for (auto iter = namespaceList.rbegin(); iter != namespaceList.rend(); ++iter) {
              auto namespaceObj = (*iter);
              structFile.indentLess();
              ss << indentStr << "} //" << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            }
          }

          finalizeBaseFile(structFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processInheritance(
                                                      ApiFile &apiFile,
                                                      StructFile &structFile,
                                                      StructPtr structObj,
                                                      const String &indentStr
                                                      )
        {
          if (!structObj) return;

          if (hasInterface(structObj)) {
            auto &ss = structFile.structSS_;
            ss << ",\n" << indentStr << fixCsPathType(structObj, true);
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedObj = (*iter).second;
            if (!relatedObj) continue;
            processInheritance(apiFile, structFile, relatedObj->toStruct(), indentStr);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processStruct(
                                                 ApiFile &apiFile,
                                                 StructFile &structFile,
                                                 StructPtr rootStructObj,
                                                 StructPtr structObj
                                                 )
        {
          if (!structObj) return;

          auto &indentStr = structFile.indent_;

          if (rootStructObj == structObj) {
            for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
              auto subStructObj = (*iter).second;
              processStruct(apiFile, subStructObj);
            }
            for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
              auto enumObj = (*iter).second;
              processEnum(apiFile, structFile, structObj, enumObj);
            }
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter) {
            auto relatedTypeObj = (*iter).second;
            if (!relatedTypeObj) continue;
            processStruct(apiFile, structFile, rootStructObj, relatedTypeObj->toStruct());
          }

          structFile.startRegion(fixCsPathType(structObj));

          processMethods(apiFile, structFile, rootStructObj, structObj);
          processProperties(apiFile, structFile, rootStructObj, structObj);
          if (rootStructObj == structObj) {
            processEventHandlers(apiFile, structFile, structObj);
          }

          structFile.endRegion(fixCsPathType(structObj));
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEnum(
                                               ApiFile &apiFile,
                                               StructFile &structFile,
                                               StructPtr structObj,
                                               EnumTypePtr enumObj
                                               )
        {
          if (!enumObj) return;

          auto &indentStr = structFile.indent_;
          auto &ss = structFile.structSS_;

          ss << "\n";
          ss << indentStr << "public enum " << GenerateStructCx::fixName(enumObj->getMappingName());
          if (enumObj->mBaseType != PredefinedTypedef_int) {
            ss << " : " << fixCCsType(enumObj->mBaseType);
          }
          ss << "\n";
          ss << indentStr << "{\n";
          structFile.indentMore();

          bool first = true;
          for (auto iter = enumObj->mValues.begin(); iter != enumObj->mValues.end(); ++iter) {
            auto valueObj = (*iter);

            if (!first) ss << ",\n";
            first = false;
            ss << indentStr << GenerateStructCx::fixName(valueObj->getMappingName());
            if (valueObj->mValue.hasData()) {
              ss << " = " << valueObj->mValue;
            }
          }
          if (!first) ss << "\n";

          structFile.indentLess();
          ss << indentStr << "}\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processMethods(
                                                  ApiFile &apiFile,
                                                  StructFile &structFile,
                                                  StructPtr rootStructObj,
                                                  StructPtr structObj
                                                  )
        {
          auto &indentStr = structFile.indent_;
          bool foundConstructor = false;

          std::stringstream headerCSS;
          std::stringstream headerCppSS;
          std::stringstream cSS;
          std::stringstream cppSS;

          auto cTypeStr = GenerateStructC::fixCType(structObj);
          auto csTypeStr = fixCsType(structObj);

#if 0
          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (!method) continue;
            if (method->hasModifier(Modifier_Method_EventHandler)) continue;

            bool isConstructor = method->hasModifier(Modifier_Method_Ctor);
            bool isStatic = method->hasModifier(Modifier_Static);
            bool hasThis = ((!isStatic) && (!isConstructor));

            if (isConstructor) foundConstructor = true;

            if (rootStructObj != structObj) {
              if ((isStatic) || (isConstructor)) continue;
            }
            if (method->hasModifier(Modifier_Method_Delete)) continue;

            String name = method->mName;
            if (method->hasModifier(Modifier_AltName)) {name = method->getModifierValue(Modifier_AltName);}

            String resultCTypeStr = (isConstructor ? fixCType(structObj) : fixCType(method->hasModifier(Modifier_Optional), method->mResult));
            bool hasResult = resultCTypeStr != "void";

            {
              auto &ss = headerCSS;
              ss << exportStr << " " << resultCTypeStr << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_" << (isConstructor ? "wrapperCreate_" : "")  << name << "(";
            }
            {
              auto &ss = cSS;
              ss << dash;
              ss << resultCTypeStr << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_" << (isConstructor ? "wrapperCreate_" : "") << name << "(";
            }

            std::stringstream argSS;

            size_t totalArgs = method->mArguments.size();
            if (hasThis) ++totalArgs;
            if (method->mThrows.size() > 0) ++totalArgs;

            if (totalArgs > 1) argSS << "\n  ";

            bool first = true;

            if (method->mThrows.size() > 0) {
              argSS << "exception_handle_t wrapperExceptionHandle";
              first = false;
            }

            if (hasThis) {
              if (!first) argSS << ",\n  ";
              argSS << fixCType(structObj) << " " << "wrapperThisHandle";
              first = false;
            }

            for (auto iterArg = method->mArguments.begin(); iterArg != method->mArguments.end(); ++iterArg) {
              auto argPropertyObj = (*iterArg);
              includeType(structFile, argPropertyObj->mType);
              if (!first) argSS << ",\n  ";
              first = false;
              argSS << fixCType(argPropertyObj->mType) << " " << argPropertyObj->mName;
            }
            argSS << ")";

            {
              auto &ss = headerCSS;
              ss << argSS.str() << ";\n";
            }
            {
              auto &ss = cSS;
              ss << argSS.str() << "\n";
              ss << "{\n";
              String indentStr = "  ";
              if (method->mThrows.size() > 0) {
                indentStr += "  ";
                if (hasResult) {
                  ss << "  " << resultCTypeStr << " wrapperResult {};\n";
                }
                ss << "  try {\n";
              }
              ss << indentStr;
              if (isConstructor) {
                ss << "auto wrapperThis = wrapper" << structObj->getPathName() << "::wrapper_create();\n";
                ss << indentStr << "wrapperThis->wrapper_init_" << GenerateStructHeader::getStructInitName(structObj) << "(";
              } else {
                if (hasThis) {
                  ss << "auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << indentStr << "if (!wrapperThis) return";
                  if ("void" != resultCTypeStr) {
                    ss << " " << resultCTypeStr << "()";
                  }
                  ss << ";\n";
                  ss << indentStr;
                }
                if (hasResult) {
                  ss << (method->mThrows.size() > 0 ? "wrapperResult = " : "return ") << getToHandleMethod(method->hasModifier(Modifier_Optional), method->mResult) << "(";
                }
                if (hasThis) {
                  ss << "wrapperThis->" << method->getMappingName() << "(";
                } else {
                  ss << "wrapper" << structObj->getPathName() << "::" << method->getMappingName() << "(";
                }
              }

              first = true;
              for (auto iterNamedArgs = method->mArguments.begin(); iterNamedArgs != method->mArguments.end(); ++iterNamedArgs) {
                auto propertyObj = (*iterNamedArgs);
                if (!first) ss << ", ";
                first = false;
                ss << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(" << propertyObj->getMappingName() << ")";
              }

              if (isConstructor) {
                ss << ");\n";
                ss << indentStr << "return " << getToHandleMethod(method->hasModifier(Modifier_Optional), structObj) << "(wrapperThis);\n";
              } else {
                if (hasResult) {
                  ss << ")";
                }
                ss << ");\n";
              }

              if (method->mThrows.size() > 0) {
                for (auto iterThrow = method->mThrows.begin(); iterThrow != method->mThrows.end(); ++iterThrow) {
                  auto throwType = (*iterThrow);
                  includeType(structFile, throwType);
                  ss << "  } catch (const " << GenerateStructHeader::getWrapperTypeString(false, throwType) << " &e) {\n";
                  ss << "    wrapper::exception_set_Exception(wrapperExceptionHandle, make_shared<::zsLib::" << ("Exception" == throwType->getMappingName() ? "" : "Exceptions::") << throwType->getMappingName() << ">(e));\n";
                }
                ss << "  }\n";
                if (hasResult) {
                  ss << "  return wrapperResult;\n";
                }
              }
              ss << "}\n";
              ss << "\n";
            }
          }

          bool onlyStatic = GenerateHelper::hasOnlyStaticMethods(structObj) || structObj->hasModifier(Modifier_Static);

          if (rootStructObj == structObj) {
            if (!onlyStatic) {
              {
                auto found = apiFile.derives_.find(structObj->getPathName());
                if (found != apiFile.derives_.end()) {
                  auto &structSet = (*found).second;

                  bool foundRelated = false;
                  for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet) {
                    auto relatedStruct = (*iterSet);
                    if (!relatedStruct) continue;
                    if (relatedStruct == structObj) continue;

                    foundRelated = true;
                    includeType(structFile, relatedStruct);

                    structFile.includeC("\"../" + fixType(relatedStruct) + ".h\"");

                    {
                      auto &ss = structFile.headerCFunctionsSS_;
                      ss << exportStr << " " << fixCType(relatedStruct) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCastAs_" << fixType(relatedStruct) << "(" << fixCType(structObj) << " handle);\n";
                    }
                    {
                      auto &ss = structFile.cFunctionsSS_;
                      ss << dash;
                      ss << fixCType(relatedStruct) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCastAs_" << fixType(relatedStruct) << "(" << fixCType(structObj) << " handle)\n";
                      ss << "{\n";
                      ss << "  typedef wrapper" << relatedStruct->getPathName() << " RelatedWrapperType;\n";
                      ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                      ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                      ss << "  if (0 == handle) return 0;\n";
                      ss << "  auto originalType = *reinterpret_cast<WrapperTypePtrRawPtr>(handle);\n";
                      ss << "  auto castType = std::dynamic_pointer_cast<RelatedWrapperType>(originalType);\n";
                      ss << "  if (!castType) return 0;\n";
                      ss << "  return " << getToHandleMethod(false, relatedStruct) << "(castType);\n";
                      ss << "}\n";
                      ss << "\n";
                    }
                  }

                  if (foundRelated) {
                    auto &ss = structFile.headerCFunctionsSS_;
                    ss << "\n";
                  }
                }
              }

              if (!foundConstructor) {
                {
                  auto &ss = structFile.headerCFunctionsSS_;
                  ss << exportStr << " " << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCreate_" << structObj->getMappingName() << "();\n";
                }
                {
                  auto &ss = structFile.cFunctionsSS_;
                  ss << dash;
                  ss << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCreate_" << structObj->getMappingName() << "()\n";
                  ss << "{\n";
                  ss << "  typedef " << fixCType(structObj) << " CType;\n";
                  ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                  ss << "  auto result = wrapper" << structObj->getPathName() << "::wrapper_create();\n";
                  ss << "  result->wrapper_init_" << GenerateStructHeader::getStructInitName(structObj) << "();\n";
                  ss << "  return reinterpret_cast<CType>(new WrapperTypePtr(result));\n";
                  ss << "}\n";
                  ss << "\n";
                }
              }

              {
                auto &ss = structFile.headerCFunctionsSS_;
                ss << exportStr << " void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperDestroy(" << fixCType(structObj) << " handle);\n";
                ss << exportStr << " instance_id_t " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperInstanceId(" << fixCType(structObj) << " handle);\n";
              }
              {
                auto &ss = structFile.cFunctionsSS_;
                ss << dash;
                ss << "void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperDestroy(" << fixCType(structObj) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                ss << "  if (0 == handle) return;\n";
                ss << "  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "instance_id_t " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperInstanceId(" << fixCType(structObj) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());\n";
                ss << "}\n";
                ss << "\n";
              }
            }

            {
              auto &ss = structFile.headerCppFunctionsSS_;
              ss << "  " << fixCType(structObj) << " " << fixType(rootStructObj) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value);\n";
              ss << "  " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " " << fixType(rootStructObj) << "_wrapperFromHandle(" << fixCType(structObj) << " handle);\n";
            }
            {
              auto &ss = structFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << fixCType(structObj) << " " << fixType(rootStructObj) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value)\n";
              ss << "  {\n";
              ss << "    typedef " << fixCType(structObj) << " CType;\n";
              ss << "    typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
              ss << "    typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
              ss << "    if (!value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new WrapperTypePtr(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " " << fixType(rootStructObj) << "_wrapperFromHandle(" << fixCType(structObj) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
              ss << "    typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
              ss << "    if (0 == handle) return WrapperTypePtr();\n";
              ss << "    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }

          structFile.headerCFunctionsSS_ << headerCSS.str();
          structFile.headerCppFunctionsSS_ << headerCppSS.str();
          structFile.cFunctionsSS_ << cSS.str();
          structFile.cppFunctionsSS_ << cppSS.str();
#endif //0
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processProperties(
                                                     ApiFile &apiFile,
                                                     StructFile &structFile,
                                                     StructPtr rootStructObj,
                                                     StructPtr structObj
                                                     )
        {
#if 0
          bool onlyStatic = GenerateHelper::hasOnlyStaticMethods(structObj) || structObj->hasModifier(Modifier_Static);

          if (onlyStatic) {
            if (rootStructObj != structObj) return;
          }

          auto exportStr = (rootStructObj == structObj ? getApiExportDefine(apiFile.global_) : getApiExportCastedDefine(apiFile.global_));

          bool isDictionary = structObj->hasModifier(Modifier_Struct_Dictionary);

          auto dash = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter) {
            auto propertyObj = (*iter);
            if (!propertyObj) continue;

            bool isStatic = propertyObj->hasModifier(Modifier_Static);
            bool hasGetter = propertyObj->hasModifier(Modifier_Property_Getter);
            bool hasSetter = propertyObj->hasModifier(Modifier_Property_Setter);

            if (!isDictionary) {
              if ((!hasGetter) && (!hasSetter)) {
                hasGetter = hasSetter = true;
              }
            }

            includeType(structFile, propertyObj->mType);

            bool hasGet = true;
            bool hasSet = true;

            if ((hasGetter) && (!hasSetter)) hasSet = false;
            if ((hasSetter) && (!hasGetter)) hasGet = false;

            if (isStatic) {
              if (hasGet) hasGetter = true;
              if (hasSet) hasSetter = true;
            }

            {
              auto &ss = structFile.headerCFunctionsSS_;
              if (hasGet) {
                ss << exportStr << " " << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle")) << ");\n";
              }
              if (hasSet) {
                ss << exportStr << " void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle, ")) << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " value);\n";
              }
            }
            {
              auto &ss = structFile.cFunctionsSS_;
              if (hasGet) {
                ss << dash;
                ss << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle")) << ")\n";
                ss << "{\n";
                if (!isStatic) {
                  ss << "  auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << "  return " << getToHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(wrapperThis->" << (hasGetter ? "get_" : "") << propertyObj->getMappingName() << (hasGetter ? "()" : "") << ");\n";
                } else {
                  ss << "  return " << getToHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(wrapper" << structObj->getPathName() << "::get_" << propertyObj->getMappingName() << "());\n";
                }
                ss << "}\n";
                ss << "\n";
              }
              if (hasSet) {
                ss << dash;
                ss << "void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle, ")) << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " value)\n";
                ss << "{\n";
                if (!isStatic) {
                  ss << "  auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << "  wrapperThis->" << (hasSetter ? "set_" : "") << propertyObj->getMappingName() << (hasSetter ? "(" : " = ") << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value)" << (hasSetter ? ")" : "") << ";\n";
                } else {
                  ss << "  wrapper" << structObj->getPathName() << "::set_" << propertyObj->getMappingName() << "(" << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value));\n";
                }
                ss << "}\n";
                ss << "\n";
              }
            }
          }
#endif // 0
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlers(
                                                        ApiFile &apiFile,
                                                        StructFile &structFile,
                                                        StructPtr structObj
                                                        )
        {
          auto &indentStr = structFile.indent_;

          if (!structObj) return;
          if (!structFile.hasEvents_) return;

          structFile.usingTypedef("callback_event_t", "System.IntPtr");

          processEventHandlersStart(apiFile, structFile, structObj);

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            if (!method->hasModifier(Modifier_Method_EventHandler)) continue;

            {
              auto &ss = structFile.delegateSS_;
              structFile.indentLess();
              ss << indentStr << "public delegate void " << fixCsType(structObj) << "_" << GenerateStructCx::fixName(method->getMappingName()) << "(";
              bool first {true};
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << fixCsPathType(arg->mType) << " " << arg->getMappingName();
              }
              ss << ");\n";
              structFile.indentMore();
            }

            {
              auto &ss = structFile.structSS_;

              ss << indentStr << "public event " << fixCsType(structObj) << "_" << GenerateStructCx::fixName(method->getMappingName()) << " " << GenerateStructCx::fixName(method->getMappingName()) << ";\n";
            }
          }

          processEventHandlersEnd(apiFile, structFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlersStart(
                                                             ApiFile &apiFile,
                                                             StructFile &structFile,
                                                             StructPtr structObj
                                                             )
        {
          auto &indentStr = structFile.indent_;

          structFile.startRegion("Events");

          {
            auto &ss = structFile.structSS_;
            ss << "\n";
            ss << indentStr << "private void WrapperObserveEvents()\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == native_) return;\n";
            ss << indentStr << "    " << getHelperPath(apiFile) << ".ObserveEvents(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath(apiFile) << "." << GenerateStructC::fixCType(structObj) << "_wrapperInstanceId(this.native_), (object)this, (object target, string method, callback_event_t handle) => {\n";
            structFile.indentMore();
            structFile.indentMore();

            ss << indentStr << "if (null == target) return;\n";

            for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
              auto method = (*iter);
              if (!method) continue;
              if (!method->hasModifier(Modifier_Method_EventHandler)) continue;
              
              ss << indentStr << "if (\"" << method->getMappingName() << "\" == method) {\n";
              ss << indentStr << "    ((" << fixCsType(structObj) << ")target)." << GenerateStructCx::fixName(method->getMappingName()) << "(";
              bool first {true};
              size_t index = 0;
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << getAdoptFromCMethod(apiFile, method->hasModifier(Modifier_Optional), arg->mType) << "(" << getApiPath(apiFile) << ".callback_event_get_data(handle, " << index << "))";
              }
              ss << ");\n";
              ss << indentStr << "}\n";
            }

            structFile.indentLess();
            structFile.indentLess();

            ss << indentStr << "    });\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "private void WrapperObserveEventsCancel()\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    " << getHelperPath(apiFile) << ".ObserveEventsCancel(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath(apiFile) << "." << GenerateStructC::fixCType(structObj) << "_wrapperInstanceId(this.native_), (object)this);\n";
            ss << indentStr << "}\n";
            ss << "\n";

          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlersEnd(
                                                           ApiFile &apiFile,
                                                           StructFile &structFile,
                                                           StructPtr structObj
                                                           )
        {
          structFile.endRegion("Events");
        }

#if 0
        
        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCType(TypePtr type)
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
        String GenerateStructDotNet::fixCType(
                                         bool isOptional,
                                         TypePtr type
                                         )
        {
          if (!isOptional) return fixCType(type);
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return String("box_") + fixCType(basicType->mBaseType);
            }
          }
          {
            auto enumObj = type->toEnumType();
            if (enumObj) {
              return String("box_") + fixCType(enumObj);
            }
          }
          return fixCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixType(TypePtr type)
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
        String GenerateStructDotNet::getApiImplementationDefine(ContextPtr context)
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
        String GenerateStructDotNet::getApiCastRequiredDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_GENERATED_REQUIRES_CAST";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_REQUIRES_CAST";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getApiExportDefine(ContextPtr context)
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
        String GenerateStructDotNet::getApiExportCastedDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_CASTED_EXPORT_API";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_CASTED_EXPORT_API";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getApiCallingDefine(ContextPtr context)
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
        String GenerateStructDotNet::getApiGuardDefine(
                                                  ContextPtr context,
                                                  bool endGuard
                                                  )
        {
          String result = (!endGuard ? "WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "WRAPPER_C_PLUS_PLUS_END_GUARD");
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + (!endGuard ? "_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "_WRAPPER_C_PLUS_PLUS_END_GUARD");
        }
        
        //---------------------------------------------------------------------
        String GenerateStructDotNet::getToHandleMethod(
                                                  bool isOptional,
                                                  TypePtr type
                                                  )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              if (isOptional) {
                return "wrapper::box_" + fixBasicType(basicType->mBaseType) + "_wrapperToHandle";
              }
              String cTypeStr = fixCType(basicType);
              if ("string_t" == cTypeStr) return "wrapper::string_t_wrapperToHandle";
              if ("binary_t" == cTypeStr) return "wrapper::binary_t_wrapperToHandle";
              return String();
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              if (isOptional) {
                return "box_" + fixCType(enumType) + "_wrapperToHandle";
              }
              return String("static_cast<") + fixCType(enumType->mBaseType) + ">";
            }
          }
          {
            if (GenerateHelper::isBuiltInType(type)) {
              auto structObj = type->toStruct();
              if (!structObj) {
                auto templatedStructObj = type->toTemplatedStructType();
                if (templatedStructObj) {
                  auto parentObj = templatedStructObj->getParent();
                  if (parentObj) structObj = parentObj->toStruct();
                }
              }

              if (!structObj) return String();

              String specialName = structObj->getPathName();

              if ("::zs::Any" == specialName) return "wrapper::zs_Any_wrapperToHandle";
              if ("::zs::Promise" == specialName) return "wrapper::zs_Promise_wrapperToHandle";
              if ("::zs::PromiseWith" == specialName) return "wrapper::zs_Promise_wrapperToHandle";
              if ("::zs::PromiseRejectionReason" == specialName) return String();
              if ("::zs::exceptions::Exception" == specialName) return "wrapper::exception_Exception_wrapperToHandle";
              if ("::zs::exceptions::InvalidArgument" == specialName) return "wrapper::exception_InvalidArgument_wrapperToHandle";
              if ("::zs::exceptions::BadState" == specialName) return "wrapper::exception_BadState_wrapperToHandle";
              if ("::zs::exceptions::NotImplemented" == specialName) return "wrapper::exception_NotImplemented_wrapperToHandle";
              if ("::zs::exceptions::NotSupported" == specialName) return "wrapper::exception_NotSupported_wrapperToHandle";
              if ("::zs::exceptions::UnexpectedError" == specialName) return "wrapper::exception_UnexpectedError_wrapperToHandle";
              if ("::zs::Time" == specialName) return "wrapper::zs_Time_wrapperToHandle";
              if ("::zs::Milliseconds" == specialName) return "wrapper::zs_Milliseconds_wrapperToHandle";
              if ("::zs::Microseconds" == specialName) return "wrapper::zs_Microseconds_wrapperToHandle";
              if ("::zs::Nanoseconds" == specialName) return "wrapper::zs_Nanoseconds_wrapperToHandle";
              if ("::zs::Seconds" == specialName) return "wrapper::zs_Seconds_wrapperToHandle";
              if ("::zs::Minutes" == specialName) return "wrapper::zs_Minutes_wrapperToHandle";
              if ("::zs::Hours" == specialName) return "wrapper::zs_Hours_wrapperToHandle";
              if ("::std::set" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
              if ("::std::list" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
              if ("::std::map" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
            }
          }
          {
            auto structObj = type->toStruct();
            if (structObj) {
              return String("wrapper::") + fixType(structObj) + "_wrapperToHandle";
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getFromHandleMethod(
                                                    bool isOptional,
                                                    TypePtr type
                                                    )
        {
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              if (!isOptional) {
                return String("static_cast<wrapper") + enumType->getPathName() + ">";
              }
            }
          }
          auto result = getToHandleMethod(isOptional, type);
          result.replaceAll("_wrapperToHandle", "_wrapperFromHandle");
          return result;
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::calculateRelations(
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
        void GenerateStructDotNet::calculateRelations(
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
        void GenerateStructDotNet::insertInto(
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
        SecureByteBlockPtr GenerateStructDotNet::generateTypesHeader(ProjectPtr project) throw (Failure)
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

          ss << "#ifndef " << getApiExportCastedDefine(project) << "\n";
          ss << "/* By defining " << getApiCastRequiredDefine(project) << " the wrapper will not export\n";
          ss << "   any base class methods and instead will expect the caller to cast the C object handle\n";
          ss << "   type to the base C object type to access base object methods and properties. */\n";
          ss << "#ifdef " << getApiCastRequiredDefine(project) << "\n";
          ss << "#define " << getApiExportCastedDefine(project) << "\n";
          ss << "#else /* " << getApiCastRequiredDefine(project) << " */\n";
          ss << "#define " << getApiExportCastedDefine(project) << " " << getApiExportDefine(project) << "\n";
          ss << "#endif /* " << getApiCastRequiredDefine(project) << " */\n";
          ss << "#endif /* ndef " << getApiExportCastedDefine(project) << " */\n";
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
          ss << "typedef signed short sshort_t;\n";
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
          ss << "typedef uintptr_t box_bool_t;\n";
          ss << "typedef uintptr_t box_schar_t;\n";
          ss << "typedef uintptr_t box_uchar_t;\n";
          ss << "typedef uintptr_t box_sshort_t;\n";
          ss << "typedef uintptr_t box_ushort_t;\n";
          ss << "typedef uintptr_t box_sint_t;\n";
          ss << "typedef uintptr_t box_uint_t;\n";
          ss << "typedef uintptr_t box_slong_t;\n";
          ss << "typedef uintptr_t box_ulong_t;\n";
          ss << "typedef uintptr_t box_sllong_t;\n";
          ss << "typedef uintptr_t box_ullong_t;\n";
          ss << "typedef uintptr_t box_float_t;\n";
          ss << "typedef uintptr_t box_double_t;\n";
          ss << "typedef uintptr_t box_float32_t;\n";
          ss << "typedef uintptr_t box_float64_t;\n";
          ss << "typedef uintptr_t box_ldouble_t;\n";
          ss << "typedef uintptr_t box_int8_t;\n";
          ss << "typedef uintptr_t box_uint8_t;\n";
          ss << "typedef uintptr_t box_int16_t;\n";
          ss << "typedef uintptr_t box_uint16_t;\n";
          ss << "typedef uintptr_t box_int32_t;\n";
          ss << "typedef uintptr_t box_uint32_t;\n";
          ss << "typedef uintptr_t box_int64_t;\n";
          ss << "typedef uintptr_t box_uint64_t;\n";
          ss << "typedef uintptr_t box_uintptr_t;\n";
          ss << "typedef uintptr_t box_binary_t;\n";
          ss << "typedef uintptr_t box_string_t;\n";
          ss << "\n";
          ss << "typedef uintptr_t instance_id_t;\n";
          ss << "typedef uintptr_t event_observer_t;\n";
          ss << "typedef uintptr_t callback_event_t;\n";
          ss << "typedef uintptr_t generic_handle_t;\n";
          ss << "typedef uintptr_t exception_handle_t;\n";
          ss << "\n";

          processTypesNamespace(ss, project->mGlobal);

          ss << "\n";
          processTypesTemplatesAndSpecials(ss, project);
          ss << "\n";
          ss << getApiGuardDefine(project, true) << "\n";

          ss << "\n";
          ss << "#ifdef __cplusplus\n";
          ss << "#include \"../types.h\"\n";
          ss << "\n";

          ss << "namespace wrapper\n";
          ss << "{\n";
          ss << "  struct IWrapperObserver;\n";
          ss << "  typedef shared_ptr<IWrapperObserver> IWrapperObserverPtr;\n";
          ss << "\n";
          ss << "  struct IWrapperObserver\n";
          ss << "  {\n";
          ss << "    virtual event_observer_t getObserver() = 0;\n";
          ss << "    virtual void observerCancel() = 0;\n";
          ss << "  };\n";
          ss << "\n";

          ss << "  struct IWrapperCallbackEvent;\n";
          ss << "  typedef shared_ptr<IWrapperCallbackEvent> IWrapperCallbackEventPtr;\n";
          ss << "\n";
          ss << "  struct IWrapperCallbackEvent\n";
          ss << "  {\n";
          ss << "    static void fireEvent(IWrapperCallbackEventPtr event);\n";
          ss << "\n";
          ss << "    virtual event_observer_t getObserver() = 0;\n";
          ss << "    virtual const char *getNamespace() = 0;\n";
          ss << "    virtual const char *getClass() = 0;\n";
          ss << "    virtual const char *getMethod() = 0;\n";
          ss << "    virtual generic_handle_t getSource() = 0;\n";
          ss << "    virtual generic_handle_t getEventData(int argumentIndex) = 0;\n";
          ss << "  };\n";
          ss << "\n";
          ss << "} /* namespace wrapper */\n";
          ss << "\n";
          ss << "#endif /* __cplusplus */\n";

          return UseHelper::convertToBuffer(ss.str());
        }
        
        //---------------------------------------------------------------------
        void GenerateStructDotNet::processTypesNamespace(
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
        void GenerateStructDotNet::processTypesStruct(
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
        void GenerateStructDotNet::processTypesEnum(
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
            ss << "typedef uintptr_t box_" << fixCType(enumObj) << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processTypesTemplatesAndSpecials(
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
        void GenerateStructDotNet::processTypesTemplate(
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
        void GenerateStructDotNet::processTypesSpecialStruct(
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

#endif //0

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructDotNet::targetKeyword()
        {
          return String("dotnet");
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::targetKeywordHelp()
        {
          return String("Generate C# DotNet wrapper using C API");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::targetOutput(
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

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("dotnet"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          EnumFile enumFile;
          enumFile.project_ = project;
          enumFile.global_ = project->mGlobal;

          ApiFile apiFile;
          apiFile.project_ = project;
          apiFile.global_ = project->mGlobal;

          GenerateStructC::calculateRelations(project->mGlobal, apiFile.derives_);
          GenerateStructC::calculateBoxings(project->mGlobal, apiFile.boxings_);

          enumFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("net_enums.cs"));
          apiFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("net_internal_apis.cs"));

          prepareEnumFile(enumFile);
          prepareApiFile(apiFile);

          processNamespace(apiFile, apiFile.global_);

          finalizeEnumFile(enumFile);
          finalizeApiFile(apiFile);
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
