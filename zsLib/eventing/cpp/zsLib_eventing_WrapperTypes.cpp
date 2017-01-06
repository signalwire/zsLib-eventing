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

#include <zsLib/eventing/internal/zsLib_eventing_WrapperTypes.h>
#include <zsLib/eventing/internal/zsLib_eventing_Helper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>


#include <cstdio>

namespace zsLib { namespace eventing { ZS_DECLARE_SUBSYSTEM(zsLib_eventing); } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseHelper);

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark
      
      //-----------------------------------------------------------------------
      bool splitToNamePath(
                           const String &typeNameWithPath,
                           String &outPath,
                           String &outName
                           )
      {
        outPath = String();
        outName = String();
        
        UseHelper::SplitMap splits;
        UseHelper::split(typeNameWithPath, splits, "::");
        UseHelper::splitTrim(splits);
        UseHelper::splitPruneEmpty(splits);
        
        if (splits.size() < 1) return false;
        
        {
          auto found = splits.find(splits.size()-1);
          ZS_THROW_INVALID_ASSUMPTION_IF(found == splits.end());
          
          outName = (*found).second;
          
          splits.erase(found);
        }

        outPath = UseHelper::combine(splits, "::");

        // put back global prefix if has global prefix
        if ("::" == typeNameWithPath.substr(0, 2)) {
          outPath = "::" + outPath;
        }

        return true;
      }
    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IWrapperTypes::toString(Visibilities value)
    {
      switch (value)
      {
        case Visibility_Public:     return "public";
        case Visibility_Protected:  return "protected";
        case Visibility_Private:    return "private";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::Visibilities IWrapperTypes::toVisibility(const char *value) throw (InvalidArgument)
    {
      String str(value);
      for (IWrapperTypes::Visibilities index = IWrapperTypes::Visibility_First; index <= IWrapperTypes::Visibility_Last; index = static_cast<IWrapperTypes::Visibilities>(static_cast<std::underlying_type<IWrapperTypes::Visibilities>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IWrapperTypes::toString(index))) return index;
      }
      
      ZS_THROW_INVALID_ARGUMENT(String("Not a visible type: ") + str);
      return Visibility_First;
    }
    
    //-------------------------------------------------------------------------
    static IWrapperTypes::TypeModifiers *getTypeModifiers()
    {
      static IWrapperTypes::TypeModifiers modifierList[] =
      {
        IWrapperTypes::TypeModifier_Generic,
        IWrapperTypes::TypeModifier_Const,
        IWrapperTypes::TypeModifier_Reference,
        IWrapperTypes::TypeModifier_Array,
        IWrapperTypes::TypeModifier_Ptr,
        IWrapperTypes::TypeModifier_RawPtr,
        IWrapperTypes::TypeModifier_SharedPtr,
        IWrapperTypes::TypeModifier_WeakPtr,
        IWrapperTypes::TypeModifier_UniPtr,
        IWrapperTypes::TypeModifier_CollectionPtr,
        IWrapperTypes::TypeModifier_None
      };
      return &(modifierList[0]);
    }

    //-------------------------------------------------------------------------
    static const char *toStringValue(IWrapperTypes::TypeModifiers value)
    {
      switch (value)
      {
        case IWrapperTypes::TypeModifier_None:          return "none";
        case IWrapperTypes::TypeModifier_Generic:       return "generic";
        case IWrapperTypes::TypeModifier_Const:         return "const";
        case IWrapperTypes::TypeModifier_Reference:     return "reference";
        case IWrapperTypes::TypeModifier_Array:         return "array";
        case IWrapperTypes::TypeModifier_Ptr:           return "ptr";
        case IWrapperTypes::TypeModifier_RawPtr:        return "raw";
        case IWrapperTypes::TypeModifier_SharedPtr:     return "shared";
        case IWrapperTypes::TypeModifier_WeakPtr:       return "weak";
        case IWrapperTypes::TypeModifier_UniPtr:        return "uni";
        case IWrapperTypes::TypeModifier_CollectionPtr: return "collection";
      }

      return "unknown";
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::toString(TypeModifiers value)
    {
      auto modifierList = getTypeModifiers();
      
      String result;

      for (int loop = 0; TypeModifier_None != modifierList[loop]; ++loop)
      {
        if (0 != (value & modifierList[loop])) {
          if (result.hasData()) result += ",";
          result += toStringValue(modifierList[loop]);
        }
      }

      return result;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypeModifiers IWrapperTypes::toTypeModifier(const char *value) throw (InvalidArgument)
    {
      TypeModifiers result {TypeModifier_None};
      
      String str(value);
      
      UseHelper::SplitMap splits;
      UseHelper::split(str, splits, ",");
      UseHelper::splitTrim(splits);
      UseHelper::splitPruneEmpty(splits);
      
      auto modifierList = getTypeModifiers();

      for (auto iter = splits.begin(); iter != splits.end(); ++iter) {
        auto &check = (*iter).second;
        
        for (int loop = 0; TypeModifier_None != modifierList[loop]; ++loop)
        {
          if (0 == check.compareNoCase(toStringValue(modifierList[loop]))) {
            result = addModifiers(result, modifierList[loop]);
            break;
          }
        }
      }

      return result;
    }
    
    //-------------------------------------------------------------------------
    bool IWrapperTypes::hasModifier(
                                    TypeModifiers value,
                                    TypeModifiers checkFor
                                    )
    {
      return 0 != (value & checkFor);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::TypeModifiers IWrapperTypes::addModifiers(
                                                             TypeModifiers value,
                                                             TypeModifiers addModifiers
                                                             )
    {
      return static_cast<TypeModifiers>(value | addModifiers);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::TypeModifiers IWrapperTypes::removeModifiers(
                                                                TypeModifiers value,
                                                                TypeModifiers addModifiers
                                                                )
    {
      return static_cast<TypeModifiers>((0xFFFFFFFF ^ addModifiers) & value);
    }

    //-------------------------------------------------------------------------
    static IWrapperTypes::StructModifiers *getStructModifiers()
    {
      static IWrapperTypes::StructModifiers modifierList[] =
      {
        IWrapperTypes::StructModifier_Generic,
        IWrapperTypes::StructModifier_Delegate,
        IWrapperTypes::StructModifier_Subscription,
        IWrapperTypes::StructModifier_None
      };
      return &(modifierList[0]);
    }
    
    //-------------------------------------------------------------------------
    static const char *toStringValue(IWrapperTypes::StructModifiers value)
    {
      switch (value)
      {
        case IWrapperTypes::StructModifier_None:          return "none";
        case IWrapperTypes::StructModifier_Generic:       return "generic";
        case IWrapperTypes::StructModifier_Delegate:      return "delegate";
        case IWrapperTypes::StructModifier_Subscription:  return "subscription";
      }

      return "unknown";
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::toString(StructModifiers value)
    {
      auto modifierList = getStructModifiers();
      
      String result;
      
      for (int loop = 0; StructModifier_None != modifierList[loop]; ++loop)
      {
        if (0 != (value & modifierList[loop])) {
          if (result.hasData()) result += ",";
          result += toStringValue(modifierList[loop]);
        }
      }
      
      return result;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::StructModifiers IWrapperTypes::toStructModifier(const char *value) throw (InvalidArgument)
    {
      StructModifiers result {StructModifier_None};
      
      String str(value);
      
      UseHelper::SplitMap splits;
      UseHelper::split(str, splits, ",");
      UseHelper::splitTrim(splits);
      UseHelper::splitPruneEmpty(splits);
      
      auto modifierList = getStructModifiers();
      
      for (auto iter = splits.begin(); iter != splits.end(); ++iter) {
        auto &check = (*iter).second;

        for (int loop = 0; StructModifier_None != modifierList[loop]; ++loop)
        {
          if (0 == check.compareNoCase(toStringValue(modifierList[loop]))) {
            result = addModifiers(result, modifierList[loop]);
            break;
          }
        }
      }
      
      return result;
    }

    //-------------------------------------------------------------------------
    bool IWrapperTypes::hasModifier(
                                    StructModifiers value,
                                    StructModifiers checkFor
                                    )
    {
      return 0 != (value & checkFor);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::StructModifiers IWrapperTypes::addModifiers(
                                                               StructModifiers value,
                                                               StructModifiers addModifiers
                                                               )
    {
      return static_cast<StructModifiers>(value | addModifiers);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::StructModifiers IWrapperTypes::removeModifiers(
                                                                  StructModifiers value,
                                                                  StructModifiers addModifiers
                                                                  )
    {
      return static_cast<StructModifiers>((0xFFFFFFFF ^ addModifiers) & value);
    }

    //-------------------------------------------------------------------------
    static IWrapperTypes::MethodModifiers *getMethodModifiers()
    {
      static IWrapperTypes::MethodModifiers modifierList[] =
      {
        IWrapperTypes::MethodModifier_Ctor,
        IWrapperTypes::MethodModifier_Static,
        IWrapperTypes::MethodModifier_Dynamic,
        IWrapperTypes::MethodModifier_Const,
        IWrapperTypes::MethodModifier_None
      };
      return &(modifierList[0]);
    }
    
    //-------------------------------------------------------------------------
    static const char *toStringValue(IWrapperTypes::MethodModifiers value)
    {
      switch (value)
      {
        case IWrapperTypes::MethodModifier_None:      return "none";
        case IWrapperTypes::MethodModifier_Ctor:      return "ctor";
        case IWrapperTypes::MethodModifier_Static:    return "static";
        case IWrapperTypes::MethodModifier_Dynamic:   return "dynamic";
        case IWrapperTypes::MethodModifier_Const:     return "const";
      }

      return "unknown";
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::toString(MethodModifiers value)
    {
      auto modifierList = getMethodModifiers();

      String result;
      for (int loop = 0; MethodModifier_None != modifierList[loop]; ++loop)
      {
        if (0 != (value & modifierList[loop])) {
          if (result.hasData()) result += ",";
          result += toStringValue(modifierList[loop]);
        }
      }
      
      return result;
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::MethodModifiers IWrapperTypes::toMethodModifier(const char *value) throw (InvalidArgument)
    {
      MethodModifiers result {MethodModifier_None};

      String str(value);

      UseHelper::SplitMap splits;
      UseHelper::split(str, splits, ",");
      UseHelper::splitTrim(splits);
      UseHelper::splitPruneEmpty(splits);
      
      auto modifierList = getMethodModifiers();
      
      for (auto iter = splits.begin(); iter != splits.end(); ++iter) {
        auto &check = (*iter).second;

        for (int loop = 0; MethodModifier_None != modifierList[loop]; ++loop)
        {
          if (0 == check.compareNoCase(toStringValue(modifierList[loop]))) {
            result = static_cast<MethodModifiers>(result | modifierList[loop]);
            break;
          }
        }
      }
      
      return result;
    }

    //-------------------------------------------------------------------------
    bool IWrapperTypes::hasModifier(
                                    MethodModifiers value,
                                    MethodModifiers checkFor
                                    )
    {
      return 0 != (value & checkFor);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::MethodModifiers IWrapperTypes::addModifiers(
                                                               MethodModifiers value,
                                                               MethodModifiers addModifiers
                                                               )
    {
      return static_cast<MethodModifiers>(value | addModifiers);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::MethodModifiers IWrapperTypes::removeModifiers(
                                                                  MethodModifiers value,
                                                                  MethodModifiers addModifiers
                                                                  )
    {
      return static_cast<MethodModifiers>((0xFFFFFFFF ^ addModifiers) & value);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Context
    #pragma mark

    //-------------------------------------------------------------------------
    String IWrapperTypes::Context::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(mName);
      hasher->update(":");

      if (mDocumentation) {
        hasher->update(UseHelper::toString(mDocumentation));
      }

      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::ContextPtr IWrapperTypes::Context::getParent() const
    {
      return mContext.lock();
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::ContextPtr IWrapperTypes::Context::getRoot() const
    {
      auto parent = getParent();
      if (!parent) return ContextPtr();
      
      while (true)
      {
        auto temp = parent->mContext.lock();
        if (!temp) return parent;

        parent = temp;
      }
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::ProjectPtr IWrapperTypes::Context::getProject() const
    {
      auto project = getRoot();
      if (!project) return ProjectPtr();
      return project->toProject();
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Context::getPath() const
    {
      auto parent = getParent();
      if (!parent) return String();
      
      ContextList parents;

      while (parent) {
        parents.push_front(parent);
        
        parent = parent->getParent();
      }

      String pathStr;

      for (auto iter = parents.begin(); iter != parents.end(); ++iter)
      {
        parent = (*iter);
        
        {
          auto namespaceObj = parent->toNamespace();
          if (namespaceObj) {
            pathStr += "::";
            pathStr += namespaceObj->getMappingName();
            goto next;
          }
        }

        {
          auto structObj = parent->toStruct();
          if (structObj) {
            pathStr += "::";
            pathStr += structObj->getMappingName();
            goto next;
          }
        }
        
      next:
        {
        }
      }

      if (pathStr.isEmpty()) return String("::");

      return pathStr;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Context::findType(
                                                            const String &typeNameWithPath,
                                                            const FindTypeOptions *options
                                                            ) const
    {
      FindTypeOptions defaultOptions;
      if (!options) options = &defaultOptions;
      
      String path;
      String name;

      if (!internal::splitToNamePath(typeNameWithPath, path, name)) return TypePtr();

      return findType(path, name, *options);
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Context::findType(
                                                            const String &pathStr,
                                                            const String &typeName,
                                                            const FindTypeOptions &options
                                                            ) const
    {
      if (options.mSearchParents) {
        auto parent = getParent();
        if (!parent) return TypePtr();

        return parent->findType(pathStr, typeName, options);
      }
      return TypePtr();
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::Context::aliasLookup(const String &value)
    {
      auto project = getProject();
      if (!project) return value;
      return project->aliasLookup(value);
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Context::init()
    {
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Context::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;

      mName = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("name")));
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Context::write(ElementPtr &rootEl) const
    {
      if (mName.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("name", mName));
      }

      if (mDocumentation) {
        rootEl->adoptAsLastChild(mDocumentation->clone());
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Context::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;
      
      auto docEl = rootEl->findFirstChildElement("documentation");
      if (docEl) {
        mDocumentation = docEl->clone()->toElement();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Project
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::Project::init()
    {
      Context::init();
      createBaseTypes();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Project::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      createBaseTypes();
      parse(rootEl);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::ProjectPtr IWrapperTypes::Project::create()
    {
      auto pThis(make_shared<Project>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::ProjectPtr IWrapperTypes::Project::create(const ElementPtr &el) throw (InvalidContent)
    {
      auto pThis(make_shared<Project>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Project::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "project";

      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);

      rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("name", mName));

      if (mAliases.size() > 0) {
        auto aliasesEl = Element::create("aliases");
        for (auto iter = mAliases.begin(); iter != mAliases.end(); ++iter) {
          auto aliasEl = Element::create("alias");
          aliasEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("in", (*iter).first));
          aliasEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("out", (*iter).second));
          aliasesEl->adoptAsLastChild(aliasEl);
        }
      }
      
      if (mDefinedExclusives.size() > 0) {
        auto exclusivesEl = Element::create("exclusives");
        for (auto iter = mDefinedExclusives.begin(); iter != mDefinedExclusives.end(); ++iter) {
          auto exclusiveStr = (*iter);
          exclusivesEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("exclusive", exclusiveStr));
        }
      }

      if (mGlobal) {
        rootEl->adoptAsLastChild(mGlobal->createElement());
      }

      return rootEl;
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Project::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;
      
      Context::parse(rootEl);

      createAliases(rootEl->findFirstChildElement("aliases"), mAliases);
      
      auto exclusivesEl = rootEl->findFirstChildElement("exclusives");
      if (exclusivesEl) {
        auto exclusiveEl = exclusivesEl->findFirstChildElement("exclusive");
        while (exclusiveEl) {
          auto exclusiveStr = UseHelper::getElementTextAndDecode(exclusiveEl);
          mDefinedExclusives.insert(exclusiveStr);
          exclusiveEl = exclusiveEl->findNextSiblingElement("exclusive");
        }
      }

      auto context = toContext();
      
      Context::parse(rootEl);
      
      ElementPtr namespaceEl = rootEl->findFirstChildElement("namespace");
      if (namespaceEl) {
        mGlobal = Namespace::createForwards(toContext(), namespaceEl);
        mGlobal->parse(namespaceEl);
      } else {
        mGlobal = Namespace::create(context);
      }
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Project::hash() const
    {
      auto hasher = IHasher::sha256();

      hasher->update(Context::hash());

      hasher->update(":list:aliases");
      for (auto iter = mAliases.begin(); iter != mAliases.end(); ++iter)
      {
        auto &aliasIn = (*iter).first;
        auto &aliasOut = (*iter).second;

        hasher->update(":next:");
        hasher->update(aliasIn);
        hasher->update(":");
        hasher->update(aliasOut);
      }
      
      hasher->update(":list:exclusives");
      for (auto iter = mDefinedExclusives.begin(); iter != mDefinedExclusives.end(); ++iter)
      {
        auto exclusiveStr = (*iter);
        hasher->update(":next:");
        hasher->update(exclusiveStr);
      }

      hasher->update(":global:");
      
      if (mGlobal) {
        hasher->update(mGlobal->hash());
      }

      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Project::findType(
                                                            const String &pathStr,
                                                            const String &typeName,
                                                            const FindTypeOptions &options
                                                            ) const
    {
      if (pathStr.hasData()) {
        if (!mGlobal) return TypePtr();
        return mGlobal->findType(pathStr, typeName, options);
      }

      auto found = mBasicTypes.find(typeName);
      if (found == mBasicTypes.end()) return TypePtr();

      return (*found).second;
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Project::resolveTypedefs() throw (InvalidContent)
    {
      if (mGlobal) {
        mGlobal->resolveTypedefs();
      }

      for (auto iter = mBasicTypes.begin(); iter != mBasicTypes.end(); ++iter) {
        auto basicType = (*iter).second;
        basicType->resolveTypedefs();
      }
    }

    //-------------------------------------------------------------------------
    bool IWrapperTypes::Project::fixTemplateHashMapping()
    {
      bool didFix = false;
      if (mGlobal) {
        do {
          didFix = mGlobal->fixTemplateHashMapping();
        } while (didFix);
      }
      return didFix;
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Project::aliasLookup(const String &value)
    {
      return IWrapperTypes::aliasLookup(mAliases, value);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::BasicTypePtr IWrapperTypes::Project::findBasicType(IEventingTypes::PredefinedTypedefs findType) const
    {
      for (auto iter = mBasicTypes.begin(); iter != mBasicTypes.end(); ++iter)
      {
        auto checkType = (*iter).second;
        if (findType == checkType->mBaseType) return checkType;
      }
      return BasicTypePtr();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Project::createBaseTypes()
    {
      auto context = toContext();

      for (IEventingTypes::PredefinedTypedefs index = IEventingTypes::PredefinedTypedef_First; index <= IEventingTypes::PredefinedTypedef_Last; index = static_cast<IEventingTypes::PredefinedTypedefs>(static_cast<std::underlying_type<IEventingTypes::PredefinedTypedefs>::type>(index) + 1)) {
        auto type = BasicType::create(context);
        type->mBaseType = index;
        type->mName = IEventingTypes::toString(index);
        mBasicTypes[type->getMappingName()] = type;
      }
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Namespace
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::Namespace::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Namespace::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      
      if (!rootEl) return;
      
      auto context = toContext();

      // scan for other nested namespaces, enums, structs and typedefs
      createNamespaceForwards(context, rootEl->findFirstChildElement("namespaces"), mNamespaces);
      createEnumForwards(context, rootEl->findFirstChildElement("enums"), mEnums);
      createStructForwards(context, rootEl->findFirstChildElement("structs"), mStructs);
      createTypedefForwards(context, rootEl->findFirstChildElement("typedefs"), mTypedefs);
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::NamespacePtr IWrapperTypes::Namespace::create(ContextPtr context)
    {
      auto pThis(make_shared<Namespace>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::NamespacePtr IWrapperTypes::Namespace::createForwards(
                                                                         ContextPtr context,
                                                                         const ElementPtr &el
                                                                         ) throw (InvalidContent)
    {
      auto pThis(make_shared<Namespace>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Namespace::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "namespace";

      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);

      if (mNamespaces.size() > 0) {
        auto namespacesEl = Element::create("namespaces");
        
        for (auto iter = mNamespaces.begin(); iter != mNamespaces.end(); ++iter)
        {
          auto namespaceObj = (*iter).second;
          namespacesEl->adoptAsLastChild(namespaceObj->createElement());
        }
        rootEl->adoptAsLastChild(namespacesEl);
      }
      
      if (mEnums.size() > 0) {
        auto enumsEl = Element::create("enums");
        
        for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter)
        {
          auto enumObj = (*iter).second;
          enumsEl->adoptAsLastChild(enumObj->createElement());
        }
        rootEl->adoptAsLastChild(enumsEl);
      }
      
      if (mStructs.size() > 0) {
        auto structsEl = Element::create("structs");
        
        for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter)
        {
          auto structObj = (*iter).second;
          structsEl->adoptAsLastChild(structObj->createElement());
        }
        rootEl->adoptAsLastChild(structsEl);
      }
      
      if (mTypedefs.size() > 0) {
        auto typedefsEl = Element::create("typedefs");
        
        for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter)
        {
          auto typedefObj = (*iter).second;
          typedefsEl->adoptAsLastChild(typedefObj->createElement());
        }
        rootEl->adoptAsLastChild(typedefsEl);
      }

      return rootEl;
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Namespace::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;
      
      auto context = toContext();

      Context::parse(rootEl);

      // scan for other nested namespaces, enums, structs and typedefs
      parseNamespaces(context, rootEl->findFirstChildElement("namespaces"), mNamespaces);
      parseEnums(context, rootEl->findFirstChildElement("enums"), mEnums);
      parseStructs(context, rootEl->findFirstChildElement("structs"), mStructs);
      parseTypedefs(context, rootEl->findFirstChildElement("typedefs"), mTypedefs);
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::Namespace::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());

      hasher->update(":namespaces:");
      for (auto iter = mNamespaces.begin(); iter != mNamespaces.end(); ++iter)
      {
        auto namespaceObj = (*iter).second;
        hasher->update(namespaceObj->hash());
        hasher->update(":next:");
      }

      hasher->update(":enums:");
      for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter)
      {
        auto enumObj = (*iter).second;
        hasher->update(enumObj->hash());
        hasher->update(":next:");
      }
      
      hasher->update(":structs:");
      for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter)
      {
        auto structObj = (*iter).second;
        hasher->update(structObj->hash());
        hasher->update(":next:");
      }
      
      hasher->update(":typedefs:");
      for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter)
      {
        auto typedefObj = (*iter).second;
        hasher->update(typedefObj->hash());
        hasher->update(":next:");
      }

      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Namespace::findType(
                                                              const String &pathStr,
                                                              const String &typeName,
                                                              const FindTypeOptions &options
                                                              ) const
    {
      String checkPath = pathStr;
      
      if ("::" == checkPath.substr(0, 2)) {
        auto parent = getParent();
        if (!parent) return TypePtr();

        if (!parent->toProject()) {
          if (!options.mSearchParents) return TypePtr();
          return parent->findType(pathStr, typeName, options);
        }
        
        // strip the global namespace if at the global namespace
        checkPath = pathStr.substr(2);
      }
      
      if (pathStr.hasData()) {
        UseHelper::SplitMap splitPaths;
        UseHelper::split(pathStr, splitPaths, "::");
        
        if (splitPaths.size() < 1) return TypePtr();
        
        String searchPath = splitPaths[0];

        splitPaths.erase(splitPaths.begin());

        checkPath = UseHelper::combine(splitPaths, "::");

        {
          auto found = mNamespaces.find(searchPath);
          if (found != mNamespaces.end()) {
            auto namespaceObj = (*found).second;
            return namespaceObj->findType(checkPath, typeName, options);
          }
        }
        
        {
          auto found = mStructs.find(searchPath);
          if (found != mStructs.end()) {
            auto structObj = (*found).second;
            return structObj->findType(checkPath, typeName, options);
          }
        }
        
        auto parent = getParent();
        if (parent) return parent->findType(pathStr, typeName, options);

        // type not found
        return TypePtr();
      }

      {
        auto found = mEnums.find(typeName);
        if (found != mEnums.end()) return (*found).second;
      }
      
      {
        auto found = mStructs.find(typeName);
        if (found != mStructs.end()) return (*found).second;
      }

      {
        auto found = mTypedefs.find(typeName);
        if (found != mTypedefs.end()) return (*found).second;
      }

      if (options.mSearchParents) {
        auto parent = getParent();
        if (parent) return parent->findType(pathStr, typeName, options);
      }

      return TypePtr();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Namespace::resolveTypedefs() throw (InvalidContent)
    {
      for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }
      for (auto iter = mNamespaces.begin(); iter != mNamespaces.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }
      for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }
      for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }
    }

    //-------------------------------------------------------------------------
    bool IWrapperTypes::Namespace::fixTemplateHashMapping()
    {
      bool didFix = false;
      for (auto iter_doNotUse = mNamespaces.begin(); iter_doNotUse != mNamespaces.end(); ) {
        auto current = iter_doNotUse;

        auto obj = (*current).second;
        if (obj->fixTemplateHashMapping()) didFix = true;
      }
      for (auto iter_doNotUse = mStructs.begin(); iter_doNotUse != mStructs.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto obj = (*current).second;
        if (obj->fixTemplateHashMapping()) didFix = true;
      }
      return didFix;
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::NamespacePtr IWrapperTypes::Namespace::findNamespace(const String &nameWithPath) const
    {
      String path;
      String name;
      
      if (!internal::splitToNamePath(nameWithPath, path, name)) return NamespacePtr();

      return findNamespace(path, name);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::NamespacePtr IWrapperTypes::Namespace::findNamespace(
                                                                        const String &pathStr,
                                                                        const String &name
                                                                        ) const
    {
      NamespacePtr parentNamespace;
      
      String checkPath = pathStr;
      
      if ("::" == checkPath.substr(0, 2)) {
        auto parent = getParent();
        if (!parent) return NamespacePtr();
        
        parentNamespace = parent->toNamespace();

        if (parentNamespace) {
          return parentNamespace->findNamespace(pathStr, name);
        }

        // strip the global namespace if at the global namespace
        checkPath = pathStr.substr(2);
      }
      
      if (pathStr.hasData()) {
        UseHelper::SplitMap splitPaths;
        UseHelper::split(pathStr, splitPaths, "::");

        if (splitPaths.size() < 1) return NamespacePtr();
        
        String searchPath = splitPaths[0];
        
        splitPaths.erase(splitPaths.begin());
        
        checkPath = UseHelper::combine(splitPaths, "::");
        
        {
          auto found = mNamespaces.find(searchPath);
          if (found != mNamespaces.end()) {
            auto namespaceObj = (*found).second;
            return namespaceObj->findNamespace(checkPath, name);
          }
        }

        auto parent = getParent();
        if (!parent) return NamespacePtr();
        
        parentNamespace = parent->toNamespace();
        if (!parentNamespace) NamespacePtr();

        return parentNamespace->findNamespace(pathStr, name);
      }
      
      {
        auto found = mNamespaces.find(name);
        if (found != mNamespaces.end()) return (*found).second;
      }

      {
        auto parent = getParent();
        if (!parent) return NamespacePtr();
        
        parentNamespace = parent->toNamespace();
        if (!parentNamespace) NamespacePtr();

      }

      return parentNamespace->findNamespace(pathStr, name);
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::createNamespaceForwards(
                                                ContextPtr context,
                                                ElementPtr namespacesEl,
                                                NamespaceMap &outNamespaces
                                                ) throw (InvalidContent)
    {
      if (!namespacesEl) return;

      auto namespaceEl = namespacesEl->findFirstChildElement("namespace");

      while (namespaceEl) {
        auto namespaceObj = Namespace::createForwards(context, namespaceEl);
        outNamespaces[namespaceObj->getMappingName()] = namespaceObj;

        namespaceEl = namespaceEl->findNextSiblingElement("namespace");
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::parseNamespaces(
                                        ContextPtr context,
                                        ElementPtr namespacesEl,
                                        NamespaceMap &ioNamespaces
                                        ) throw (InvalidContent)
    {
      if (!namespacesEl) return;
      
      auto namespaceEl = namespacesEl->findFirstChildElement("namespace");

      while (namespaceEl) {
        auto name = context->aliasLookup(UseHelper::getElementTextAndDecode(namespaceEl->findFirstChildElement("name")));
        
        NamespacePtr namespaceObj;

        auto found = ioNamespaces.find(name);
        if (found == ioNamespaces.end()) {
          namespaceObj = Namespace::createForwards(context, namespaceEl);
          ioNamespaces[namespaceObj->getMappingName()] = namespaceObj;
        } else {
          namespaceObj = (*found).second;
        }
        namespaceObj->parse(namespaceEl);

        namespaceEl = namespaceEl->findNextSiblingElement("namespace");
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Type
    #pragma mark

    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Type::createReferencedType(
                                                                     ContextPtr context,
                                                                     ElementPtr parentEl
                                                                     ) throw (InvalidContent)
    {
      FindTypeOptions options;

      {
        auto typedefEl = parentEl->findFirstChildElement("typedef");
        if (typedefEl) {
          auto typedefObj = TypedefType::createForwards(context, typedefEl);
          typedefObj->parse(typedefEl);
          return typedefObj;
        }
      }

      {
        auto basicEl = parentEl->findFirstChildElement("basic");
        if (basicEl) {
          auto baseStr = context->aliasLookup(UseHelper::getElementTextAndDecode(basicEl->findFirstChildElement("base")));
          auto foundType = context->findType(String(), baseStr, options);
          if (!foundType) {
            ZS_THROW_CUSTOM(InvalidContent, String("Basic type was not found: ") + baseStr);
          }
          return foundType;
        }
      }

      {
        auto enumEl = parentEl->findFirstChildElement("enum");
        if (enumEl) {
          auto pathStr = context->aliasLookup(UseHelper::getElementTextAndDecode(enumEl->findFirstChildElement("path")));
          auto baseStr = context->aliasLookup(UseHelper::getElementTextAndDecode(enumEl->findFirstChildElement("base")));

          auto foundType = context->findType(pathStr, baseStr, options);
          if (!foundType) {
            ZS_THROW_CUSTOM(InvalidContent, String("Enum type was not found, path=") + pathStr + ", base=" + baseStr);
          }
          return foundType;
        }
      }

      {
        auto structEl = parentEl->findFirstChildElement("struct");
        if (structEl) {
          auto pathStr = context->aliasLookup(UseHelper::getElementTextAndDecode(structEl->findFirstChildElement("path")));
          auto baseStr = context->aliasLookup(UseHelper::getElementTextAndDecode(structEl->findFirstChildElement("base")));

          auto foundType = context->findType(pathStr, baseStr, options);
          if (!foundType) {
            ZS_THROW_CUSTOM(InvalidContent, String("Struct type was not found, path=") + pathStr + ", base=" + baseStr);
          }
          return foundType;
        }
      }

      ZS_THROW_CUSTOM(InvalidContent, "Referenced type was not found for context, context=" + context->getMappingName());
      return TypePtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Type::createReferenceTypeElement() const
    {
      {
        auto typedefType = toTypedefType();
        if (typedefType) return typedefType->createElement();
      }

      {
        auto basicType = toBasicType();
        if (basicType) {
          auto typeEl = Element::create("basic");
          typeEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", IEventingTypes::toString(basicType->mBaseType)));
          return typeEl;
        }
      }

      {
        auto enumType = toEnumType();
        if (enumType) {
          auto typeEl = Element::create("enum");
          auto pathStr = enumType->getPath();
          typeEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("path", pathStr));
          typeEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", enumType->getMappingName()));
          return typeEl;
        }
      }

      {
        auto structType = toStruct();
        if (structType) {
          auto typeEl = Element::create("struct");

          auto pathStr = structType->getPath();
          typeEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("path", pathStr));
          typeEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", structType->getMappingName()));
          return typeEl;
        }
      }

      ZS_THROW_INVALID_ASSUMPTION("Unexpected type reference found: " + getMappingName());
      
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::BasicType
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::BasicType::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::BasicType::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      
      if (!rootEl) return;

      Context::parse(rootEl);

      String baseTypeStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("type")));
      
      if (baseTypeStr.hasData()) {
        try {
          mBaseType = IEventingTypes::toPredefinedTypedef(baseTypeStr);
        } catch (const InvalidArgument &) {
          ZS_THROW_CUSTOM(InvalidContent, String("Invalid base type") + baseTypeStr);
        }
      }

      if (mName.isEmpty()) {
        mName = IEventingTypes::toString(mBaseType);
      }
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::BasicTypePtr IWrapperTypes::BasicType::create(ContextPtr context)
    {
      auto pThis(make_shared<BasicType>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::BasicType::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "basic";

      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);

      rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("type", IEventingTypes::toString(mBaseType)));

      return rootEl;
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::BasicType::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());
      hasher->update(":");
      hasher->update(IEventingTypes::toString(mBaseType));
      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }
    

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::EnumType
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::EnumType::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::EnumType::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      
      if (!rootEl) return;
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::EnumTypePtr IWrapperTypes::EnumType::create(ContextPtr context)
    {
      auto pThis(make_shared<EnumType>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::EnumTypePtr IWrapperTypes::EnumType::createForwards(
                                                                       ContextPtr context,
                                                                       const ElementPtr &el
                                                                       ) throw (InvalidContent)
    {
      auto pThis(make_shared<EnumType>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::EnumType::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "enum";
      
      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);
      
      rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("type", IEventingTypes::toString(mBaseType)));
      
      if (mValues.size() > 0) {
        auto valuesEl = Element::create("values");
        for (auto iter = mValues.begin(); iter != mValues.end(); ++iter) {
          auto nameValuePair = (*iter);
          
          auto valueEl = Element::create("value");
          
          valueEl->adoptAsFirstChild(UseHelper::createElementWithTextAndJSONEncode("name", nameValuePair.first));
          valueEl->adoptAsFirstChild(UseHelper::createElementWithTextAndJSONEncode("value", nameValuePair.second));

          valuesEl->adoptAsFirstChild(valueEl);
        }

        rootEl->adoptAsLastChild(valuesEl);
      }

      return rootEl;
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::EnumType::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;

      Context::parse(rootEl);

      String baseTypeStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("type")));

      if (baseTypeStr.hasData()) {
        try {
          mBaseType = IEventingTypes::toPredefinedTypedef(baseTypeStr);
        } catch (const InvalidArgument &) {
          ZS_THROW_CUSTOM(InvalidContent, String("Invalid base type") + baseTypeStr);
        }
      }
      
      auto valuesEl = rootEl->findFirstChildElement("values");
      if (valuesEl) {
        auto valueEl = valuesEl->findFirstChildElement("value");
        while (valueEl) {
          String name = aliasLookup(UseHelper::getElementTextAndDecode(valueEl->findFirstChildElement("name")));
          String value = aliasLookup(UseHelper::getElementTextAndDecode(valueEl->findFirstChildElement("value")));
          mValues.push_back(NameValuePair(name, value));
          valueEl = valueEl->findNextSiblingElement("value");
        }
      }
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::EnumType::hash() const
    {
      auto hasher = IHasher::sha256();

      hasher->update(Context::hash());
      
      hasher->update(":values:");
      for (auto iter = mValues.begin(); iter != mValues.end(); ++iter)
      {
        auto nameValuePair = (*iter);
        hasher->update(nameValuePair.first);
        hasher->update(":value:");
        hasher->update(nameValuePair.second);
        hasher->update(":next:");
      }
      
      hasher->update(":end");

      return hasher->finalizeAsString();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::createEnumForwards(
                                           ContextPtr context,
                                           ElementPtr enumsEl,
                                           EnumMap &outEnums
                                           ) throw (InvalidContent)
    {
      if (!enumsEl) return;

      auto enumEl = enumsEl->findFirstChildElement("enum");
      while (enumEl) {
        auto enumObj = EnumType::createForwards(context, enumEl);
        outEnums[enumObj->getMappingName()] = enumObj;
        enumEl = enumEl->findNextSiblingElement("enum");
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::parseEnums(
                                   ContextPtr context,
                                   ElementPtr enumsEl,
                                   EnumMap &ioEnums
                                   ) throw (InvalidContent)
    {
      if (!enumsEl) return;
      
      auto enumEl = enumsEl->findFirstChildElement("enum");
      while (enumEl) {
        auto name = context->aliasLookup(UseHelper::getElementTextAndDecode(enumEl->findFirstChildElement("name")));
        
        EnumTypePtr enumObj;
        
        auto found = ioEnums.find(name);
        if (found == ioEnums.end()) {
          enumObj = EnumType::createForwards(context, enumEl);
          ioEnums[enumObj->getMappingName()] = enumObj;
        } else {
          enumObj = (*found).second;
        }
        enumObj->parse(enumEl);

        enumEl = enumEl->findNextSiblingElement("enum");
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::TypedefType
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::TypedefType::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::TypedefType::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;

      Context::init(rootEl);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::TypedefTypePtr IWrapperTypes::TypedefType::create(ContextPtr context)
    {
      auto pThis(make_shared<TypedefType>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypedefTypePtr IWrapperTypes::TypedefType::createForwards(
                                                                             ContextPtr context,
                                                                             const ElementPtr &el
                                                                             ) throw (InvalidContent)
    {
      auto pThis(make_shared<TypedefType>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::TypedefType::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "typedef";
      
      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);
      
      String modifiersStr = toString(mModifiers);

      if (modifiersStr.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("modifiers", modifiersStr));
      }
      
      if (0 != mArraySize) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithNumber("arraySize", string(mArraySize)));
      }
      
      auto originalType = mOriginalType.lock();

      if (originalType) {

        // attempt to figure out type of original
        {
          auto basicType = originalType->toBasicType();
          if (basicType) {
            rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", IEventingTypes::toString(basicType->mBaseType)));
            goto done;
          }
        }

        {
          auto enumType = originalType->toEnumType();
          if (enumType) {
            auto pathStr = enumType->getPath();
            rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("path", pathStr));
            rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", enumType->getMappingName()));
            goto done;
          }
        }

        {
          auto structType = originalType->toStruct();
          if (structType) {
            auto pathStr = structType->getPath();
            rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("path", pathStr));
            rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", structType->getMappingName()));
            goto done;
          }
        }
        
        ZS_THROW_BAD_STATE("typedef is not resolved to a proper type");

      done:
        {
        }
      } else {
        // must point to something if not generic template typedef
        ZS_THROW_BAD_STATE_IF(!hasModifier(mModifiers, TypeModifier_Generic));
      }

      return rootEl;
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::TypedefType::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;
      
      Context::parse(rootEl);

      String modifiersStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("modifiers")));
      if (modifiersStr.hasData()) {
        mModifiers = toTypeModifier(modifiersStr);
      }
      
      String arrayStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("arraySize")));
      if (arrayStr.hasData()) {
        try {
          mArraySize = Numeric<decltype(mArraySize)>(arrayStr);
        } catch(const Numeric<decltype(mArraySize)>::ValueOutOfRange &) {
          ZS_THROW_CUSTOM(InvalidContent, String("array size is not valid: ") + arrayStr);
        }
      }

      if (!hasModifier(mModifiers, TypeModifier_Generic)) {
        String pathStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findLastChildElement("path")));
        String baseStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findLastChildElement("base")));
        
        FindTypeOptions options;
        auto foundType = findType(pathStr, baseStr, options);

        if (!foundType) {
          ZS_THROW_CUSTOM(InvalidContent, String("Type was not found, path=") + pathStr + ", base=" + baseStr);
        }

        mOriginalType = foundType;
      }
    }
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::TypedefType::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());

      hasher->update(":");
      hasher->update(toString(mModifiers));
      hasher->update(":");

      auto original = mOriginalType.lock();
      if (original) {
        auto name = original->getMappingName();
        auto path = original->getPath();

        hasher->update(path);
        hasher->update(":");
        hasher->update(name);
        hasher->update(":");
      } else {
        hasher->update("__generic__:");
      }

      hasher->update(mArraySize);
      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::TypedefType::resolveTypedefs() throw (InvalidContent)
    {
      auto originalType = mOriginalType.lock();

      TypePtr baseType;
      TypedefTypeList linkedTypedefs;
      TypedefTypeSet typedefsSet;

      while (originalType) {
        auto typedefType = originalType->toTypedefType();
        if (typedefType) {
          if (typedefsSet.find(typedefType) != typedefsSet.end()) {
            ZS_THROW_CUSTOM(InvalidContent, String("Typedef is circular: ") + typedefType->getMappingName());
          }

          typedefsSet.insert(typedefType);
          linkedTypedefs.push_front(typedefType);
          continue;
        }
        baseType = originalType;
        break;
      }

      if (linkedTypedefs.size() < 1) return;  // nothing to do

      if (!baseType) {
        ZS_THROW_CUSTOM(InvalidContent, String("No linked base typedef found: ") + getMappingName());
      }

      for (auto iter = linkedTypedefs.begin(); iter != linkedTypedefs.end(); ++iter) {
        auto typedefObj = (*iter);

        if (0 != typedefObj->mArraySize) {
          if (0 != mArraySize) {
            ZS_THROW_CUSTOM(InvalidContent, String("Multi-dimensional array not support: ") + getMappingName());
          }
          mArraySize = typedefObj->mArraySize;
        }

        TypeModifiers originalModifiers = typedefObj->mModifiers;

        // process safe to combine modifiers
        {
          if (hasModifier(typedefObj->mModifiers, TypeModifier_Generic)) {
            mModifiers = addModifiers(mModifiers, TypeModifier_Generic);
            originalModifiers = removeModifiers(originalModifiers, TypeModifier_Generic);
          }
          if (hasModifier(typedefObj->mModifiers, TypeModifier_Const)) {
            mModifiers = addModifiers(mModifiers, TypeModifier_Const);
            originalModifiers = removeModifiers(originalModifiers, TypeModifier_Const);
          }
          if (hasModifier(typedefObj->mModifiers, TypeModifier_Ptr)) {
            mModifiers = addModifiers(mModifiers, TypeModifier_Ptr);
            originalModifiers = removeModifiers(originalModifiers, TypeModifier_Ptr);
          }
        }

        if (hasModifier(mModifiers, typedefObj->mModifiers)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers) + ", linked modifiers=" + toString(originalModifiers));
        }
      }

      if (hasModifier(mModifiers, TypeModifier_SharedPtr)) {
        if (hasModifier(mModifiers, TypeModifier_WeakPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_UniPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_CollectionPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
      }
      if (hasModifier(mModifiers, TypeModifier_WeakPtr)) {
        if (hasModifier(mModifiers, TypeModifier_SharedPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_UniPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_CollectionPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
      }
      if (hasModifier(mModifiers, TypeModifier_UniPtr)) {
        if (hasModifier(mModifiers, TypeModifier_SharedPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_WeakPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_CollectionPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
      }
      if (hasModifier(mModifiers, TypeModifier_CollectionPtr)) {
        if (hasModifier(mModifiers, TypeModifier_SharedPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_WeakPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
        if (hasModifier(mModifiers, TypeModifier_UniPtr)) {
          ZS_THROW_CUSTOM(InvalidContent, String("Not allowed to combine these type modifiers, current modifiers=") + toString(mModifiers));
        }
      }

      mOriginalType = baseType;
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::TypedefType::getTypeBypassingTypedefIfNoop() const
    {
      if (0 != mArraySize) return toType();

      if (TypeModifier_None == mModifiers) {
        auto originalType = mOriginalType.lock();
        if (originalType) return originalType;
      }

      return toType();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::createTypedefForwards(
                                              ContextPtr context,
                                              ElementPtr typedefsEl,
                                              TypedefTypeMap &outTypedefs
                                              ) throw (InvalidContent)
    {
      if (!typedefsEl) return;
      
      auto typedefEl = typedefsEl->findFirstChildElement("typedef");
      while (typedefEl) {
        auto typedefObj = TypedefType::createForwards(context, typedefsEl);
        outTypedefs[typedefObj->getMappingName()] = typedefObj;
        typedefEl = typedefEl->findNextSiblingElement("typedef");
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::parseTypedefs(
                                      ContextPtr context,
                                      ElementPtr typedefsEl,
                                      TypedefTypeMap &ioTypedefs
                                      ) throw (InvalidContent)
    {
      if (!typedefsEl) return;

      auto typedefEl = typedefsEl->findFirstChildElement("typedef");
      while (typedefEl) {
        auto name = context->aliasLookup(UseHelper::getElementTextAndDecode(typedefEl->findFirstChildElement("name")));
        
        TypedefTypePtr typedefObj;
        
        auto found = ioTypedefs.find(name);
        if (found == ioTypedefs.end()) {
          typedefObj = TypedefType::createForwards(context, typedefEl);
          ioTypedefs[typedefObj->getMappingName()] = typedefObj;
        } else {
          typedefObj = (*found).second;
        }

        typedefObj->parse(typedefEl);

        typedefEl = typedefEl->findNextSiblingElement("typedef");
      }
    }
    

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Struct
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::Struct::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Struct::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      
      if (!rootEl) return;

      mTemplateID = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("templateId")));

      auto context = toContext();

      createEnumForwards(context, rootEl->findFirstChildElement("enums"), mEnums);
      createStructForwards(context, rootEl->findFirstChildElement("structs"), mStructs);
      createTypedefForwards(context, rootEl->findFirstChildElement("typedefs"), mTypedefs);
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::StructPtr IWrapperTypes::Struct::create(ContextPtr context)
    {
      auto pThis(make_shared<Struct>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::StructPtr IWrapperTypes::Struct::createForwards(
                                                                   ContextPtr context,
                                                                   const ElementPtr &el
                                                                   ) throw (InvalidContent)
    {
      auto pThis(make_shared<Struct>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Struct::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "struct";
      
      ElementPtr rootEl = Element::create(objectName);

      Context::write(rootEl);

      String modifiersStr = toString(mModifiers);

      if (modifiersStr.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("modifiers", modifiersStr));
      }

      String lastVisibilityStr = toString(mCurrentVisbility);
      if (lastVisibilityStr.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("visibility", lastVisibilityStr));
      }

      if (mTemplateID.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("templateId", mTemplateID));
      }

      if (mTemplates.size() > 0) {
        ElementPtr templatesEl = Element::create("templates");

        for (auto iter = mTemplates.begin(); iter != mTemplates.end(); ++iter) {
          auto templateType = (*iter);

          ElementPtr templateEl = Element::create("template");

          templateEl->adoptAsLastChild(templateType->createReferenceTypeElement());
          templatesEl->adoptAsLastChild(templateEl);
        }
      }

      if (mIsARelationships.size() > 0) {
        ElementPtr relationshipsEl = Element::create("relationships");
        for (auto iter = mIsARelationships.begin(); iter != mIsARelationships.end(); ++iter) {
          auto visibility = (*iter).second.first;
          auto structType = (*iter).second.second;

          auto relationshipEl = Element::create("relationsip");

          String visibilityStr = toString(visibility);
          if (visibilityStr.hasData()) {
            relationshipEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("visibility", visibilityStr));
          }

          auto pathStr = structType->getPath();
          if (pathStr.hasData()) {
            relationshipEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("path", pathStr));
          }

          auto nameStr = structType->getMappingName();
          if (nameStr.hasData()) {
            relationshipEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("base", nameStr));
          }

          relationshipsEl->adoptAsLastChild(relationshipEl);
        }
        rootEl->adoptAsLastChild(relationshipsEl);
      }

      if (mEnums.size() > 0) {
        auto enumsEl = Element::create("enums");

        for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter)
        {
          auto enumObj = (*iter).second;
          enumsEl->adoptAsLastChild(enumObj->createElement());
        }
        rootEl->adoptAsLastChild(enumsEl);
      }

      if (mStructs.size() > 0) {
        auto structsEl = Element::create("structs");

        for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter)
        {
          auto structObj = (*iter).second;
          structsEl->adoptAsLastChild(structObj->createElement());
        }
        rootEl->adoptAsLastChild(structsEl);
      }

      if (mTypedefs.size() > 0) {
        auto typedefsEl = Element::create("typedefs");

        for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter)
        {
          auto typedefObj = (*iter).second;
          typedefsEl->adoptAsLastChild(typedefObj->createElement());
        }
        rootEl->adoptAsLastChild(typedefsEl);
      }

      if (mProperties.size() > 0) {
        auto propertiesEl = Element::create("properties");

        for (auto iter = mProperties.begin(); iter != mProperties.end(); ++iter)
        {
          auto propertyObj = (*iter);
          propertiesEl->adoptAsLastChild(propertyObj->createElement());
        }
        rootEl->adoptAsLastChild(propertiesEl);
      }

      if (mMethods.size() > 0) {
        auto methodsEl = Element::create("methods");

        for (auto iter = mMethods.begin(); iter != mMethods.end(); ++iter)
        {
          auto methodObj = (*iter);
          methodsEl->adoptAsLastChild(methodObj->createElement());
        }
        rootEl->adoptAsLastChild(methodsEl);
      }

      return rootEl;
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Struct::parse(const ElementPtr &rootEl) throw (InvalidContent)
    {
      if (!rootEl) return;

      Context::parse(rootEl);

      auto context = toContext();

      {
        auto modifiersStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("modifiers")));
        if (modifiersStr.hasData()) {
          mModifiers = toStructModifier(modifiersStr);
        }
      }

      {
        auto visibilityStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("visibility")));
        if (visibilityStr.hasData()) {
          mCurrentVisbility = toVisibility(visibilityStr);
        }
      }

      mTemplateID = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("templateId")));

      {
        auto templatesEl = rootEl->findFirstChildElement("templates");
        if (templatesEl) {
          auto templateEl = templatesEl->findFirstChildElement("template");

          mTemplates.push_back(createReferencedType(context, templateEl));
          templateEl = templateEl->findNextSiblingElement("template");
        }
      }


      {
        auto relationshipsEl = rootEl->findFirstChildElement("relationships");
        if (relationshipsEl) {

          FindTypeOptions options;

          auto relationshipEl = relationshipsEl->findFirstChildElement("relationship");
          while (relationshipEl) {

            Visibilities visible {Visibility_First};

            {
              auto visibiltyStr = aliasLookup(UseHelper::getElementTextAndDecode(relationshipEl->findFirstChildElement("visibility")));
              if (visibiltyStr.hasData()) {
                visible = toVisibility(visibiltyStr);
              }
            }

            {
              auto pathStr = aliasLookup(UseHelper::getElementTextAndDecode(relationshipEl->findFirstChildElement("path")));
              auto baseStr = aliasLookup(UseHelper::getElementTextAndDecode(relationshipEl->findFirstChildElement("base")));

              auto foundType = findType(pathStr, baseStr, options);
              if (!foundType) {
                ZS_THROW_CUSTOM(InvalidContent, String("Relationship struct type was not found, path=") + pathStr + ", base=" + baseStr);
              }

              auto structObj = foundType->toStruct();
              if (!structObj) {
                ZS_THROW_CUSTOM(InvalidContent, String("Relationship struct type was found but was not a struct, path=") + pathStr + ", base=" + baseStr);
              }

              mIsARelationships[structObj->getMappingName()] = IWrapperTypes::VisibleStructPair(visible, structObj);
            }

            relationshipEl = relationshipEl->findNextSiblingElement("relationship");
          }
        }
      }

      // scan for other nested namespaces, enums, structs and typedefs
      parseEnums(context, rootEl->findFirstChildElement("enums"), mEnums);
      parseStructs(context, rootEl->findFirstChildElement("structs"), mStructs);
      parseTypedefs(context, rootEl->findFirstChildElement("typedefs"), mTypedefs);

      createProperties(context, rootEl->findFirstChildElement("properties"), mProperties);
      createMethods(context, rootEl->findFirstChildElement("properties"), mMethods);
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Struct::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());

      hasher->update(":");
      hasher->update(toString(mModifiers));
      hasher->update(":");
      hasher->update(toString(mCurrentVisbility));
      hasher->update(":");
      hasher->update(mTemplateID);

      hasher->update(":templates:");
      for (auto iter = mTemplates.begin(); iter != mTemplates.end(); ++iter) {
        auto templateObj = (*iter);

        String pathStr = templateObj->getPath();
        String baseStr = templateObj->getMappingName();

        hasher->update(pathStr);
        hasher->update(":");
        hasher->update(baseStr);
        hasher->update(":next:");
      }

      hasher->update(":relationships:");
      for (auto iter = mIsARelationships.begin(); iter != mIsARelationships.end(); ++iter) {
        auto visibility = (*iter).second.first;
        auto structObj = (*iter).second.second;

        String pathStr = structObj->getPath();
        String baseStr = structObj->getMappingName();

        hasher->update(toString(visibility));
        hasher->update(":");
        hasher->update(pathStr);
        hasher->update(":");
        hasher->update(baseStr);
        hasher->update(":next:");
      }

      hasher->update(":enums:");
      for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter)
      {
        auto enumObj = (*iter).second;
        hasher->update(enumObj->hash());
        hasher->update(":next:");
      }

      hasher->update(":structs:");
      for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter)
      {
        auto structObj = (*iter).second;
        hasher->update(structObj->hash());
        hasher->update(":next:");
      }

      hasher->update(":typedefs:");
      for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter)
      {
        auto typedefObj = (*iter).second;
        hasher->update(typedefObj->hash());
        hasher->update(":next:");
      }

      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Struct::getMappingName() const
    {
      if (mTemplateID.hasData()) {
        return mName + "/" + mTemplateID;
      }
      return mName;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::TypePtr IWrapperTypes::Struct::findType(
                                                           const String &pathStr,
                                                           const String &typeName,
                                                           const FindTypeOptions &options
                                                           ) const
    {
      String checkPath = pathStr;
      
      if ("::" == checkPath.substr(0, 2)) {
        auto parent = getParent();
        if (!parent) return TypePtr();
        
        if (!parent->toProject()) {
          if (!options.mSearchParents) return TypePtr();
          return parent->findType(pathStr, typeName, options);
        }

        // strip the global namespace if at the global namespace
        checkPath = pathStr.substr(2);
      }
      
      if (pathStr.hasData()) {
        UseHelper::SplitMap splitPaths;
        UseHelper::split(pathStr, splitPaths, "::");
        
        if (splitPaths.size() < 1) return TypePtr();
        
        String searchPath = splitPaths[0];
        
        splitPaths.erase(splitPaths.begin());
        
        checkPath = UseHelper::combine(splitPaths, "::");

        {
          auto found = mStructs.find(searchPath);
          if (found != mStructs.end()) {
            auto structObj = (*found).second;
            return structObj->findType(checkPath, typeName, options);
          }
        }

        {
          FindTypeOptions baseOptions = options;
          baseOptions.mSearchParents = false;

          for (auto iter = mIsARelationships.begin(); iter != mIsARelationships.end(); ++iter)
          {
            auto checkName = (*iter).first;
            auto baseType = (*iter).second.second;
            
            baseType->findType(pathStr, typeName, baseOptions);
            if (checkName == searchPath) {
              baseType->findType(checkPath, typeName, baseOptions);
            }
          }
        }

        if (options.mSearchParents) {
          auto parent = getParent();
          if (parent) return parent->findType(pathStr, typeName, options);
        }
        
        // type not found
        return TypePtr();
      }

      {
        auto found = mEnums.find(typeName);
        if (found != mEnums.end()) return (*found).second;
      }
      
      {
        auto found = mStructs.find(typeName);
        if (found != mStructs.end()) return (*found).second;
      }
      
      {
        auto found = mTypedefs.find(typeName);
        if (found != mTypedefs.end()) return (*found).second;
      }

      {
        for (auto iter = mTemplates.begin(); iter != mTemplates.end(); ++iter) {
          auto type = (*iter);
          auto name = type->getMappingName();

          if (name == typeName) return type;
        }
      }

      {
        FindTypeOptions baseOptions = options;
        baseOptions.mSearchParents = false;

        for (auto iter = mIsARelationships.begin(); iter != mIsARelationships.end(); ++iter)
        {
          auto checkName = (*iter).first;
          auto baseType = (*iter).second.second;

          baseType->findType(String(), typeName, baseOptions);
        }
      }

      if (options.mSearchParents) {
        auto parent = getParent();
        if (parent) return parent->findType(pathStr, typeName, options);
      }
      
      return TypePtr();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Struct::resolveTypedefs() throw (InvalidContent)
    {
      for (auto iter = mTypedefs.begin(); iter != mTypedefs.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }

      for (auto iter = mEnums.begin(); iter != mEnums.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }
      for (auto iter = mStructs.begin(); iter != mStructs.end(); ++iter) {
        auto obj = (*iter).second;
        obj->resolveTypedefs();
      }

      for (auto iter = mProperties.begin(); iter != mProperties.end(); ++iter) {
        auto obj = (*iter);
        obj->resolveTypedefs();
      }
      for (auto iter = mMethods.begin(); iter != mMethods.end(); ++iter) {
        auto obj = (*iter);
        obj->resolveTypedefs();
      }

      for (auto iter_doNotUse = mTemplates.begin(); iter_doNotUse != mTemplates.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        TypePtr obj = (*current);
        if (!obj) continue;

        TypePtr bypassType = obj->getTypeBypassingTypedefIfNoop();
        if (bypassType == obj) continue;

        mTemplates.insert(current, bypassType);
        mTemplates.erase(current);
      }
    }

    //-------------------------------------------------------------------------
    bool IWrapperTypes::Struct::fixTemplateHashMapping()
    {
      bool didFix = false;
      for (auto iter_doNotUse = mStructs.begin(); iter_doNotUse != mStructs.end(); ) {

        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto obj = (*current).second;
        if (obj->fixTemplateHashMapping()) didFix = true;
      }

      for (auto iter_doNotUse = mStructs.begin(); iter_doNotUse != mStructs.end(); ) {
        
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto obj = (*current).second;

        String calculatedID = calculateTemplateID();

        if (mTemplateID == calculatedID) continue;

        auto parent = mContext.lock();
        if (!parent) continue;

        {
          auto parentNamespace = parent->toNamespace();
          if (parentNamespace) {
            auto found = parentNamespace->mStructs.find(getMappingName());
            if (found != parentNamespace->mStructs.end()) {
              parentNamespace->mStructs.erase(found);
            }
            mTemplateID = calculatedID;
            parentNamespace->mStructs[getMappingName()] = toStruct();
            return true;
          }
        }

        {
          auto parentStruct = parent->toStruct();
          if (parentStruct) {
            auto found = parentStruct->mStructs.find(getMappingName());
            if (found != parentStruct->mStructs.end()) {
              parentStruct->mStructs.erase(found);
            }
            mTemplateID = calculatedID;
            parentStruct->mStructs[getMappingName()] = toStruct();
            return true;
          }
        }
      }

      return didFix;
    }

    //-------------------------------------------------------------------------
    String IWrapperTypes::Struct::calculateTemplateID() const
    {
      if (hasModifier(mModifiers, StructModifier_Generic)) return String();
      if (mTemplates.size() < 1) return String();

      auto hasher = IHasher::sha256();

      for (auto iter = mTemplates.begin(); iter != mTemplates.end(); ++iter)
      {
        auto obj = (*iter);

        hasher->update(obj->hash());
        hasher->update(":");
      }

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::createStructForwards(
                                             ContextPtr context,
                                             ElementPtr structsEl,
                                             StructMap &outStructs
                                             ) throw (InvalidContent)
    {
      if (!structsEl) return;

      auto structEl = structsEl->findFirstChildElement("struct");

      while (structEl) {
        auto structObj = Struct::createForwards(context, structEl);
        outStructs[structObj->getMappingName()] = structObj;

        structEl = structEl->findNextSiblingElement("struct");
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::parseStructs(
                                     ContextPtr context,
                                     ElementPtr structsEl,
                                     StructMap &ioStructs
                                     ) throw (InvalidContent)
    {
      if (!structsEl) return;

      auto structEl = structsEl->findFirstChildElement("struct");

      while (structEl) {
        auto name = context->aliasLookup(UseHelper::getElementTextAndDecode(structEl->findFirstChildElement("name")));

        StructPtr structObj;

        auto found = ioStructs.find(name);
        if (found == ioStructs.end()) {
          structObj = Struct::createForwards(context, structEl);
          ioStructs[structObj->getMappingName()] = structObj;
        } else {
          structObj = (*found).second;
        }
        structObj->parse(structEl);

        structEl = structEl->findNextSiblingElement("struct");
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Property
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::Property::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Property::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      Context::parse(rootEl);

      if (!rootEl) return;

      mDefaultValue = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("default")));

      auto context = toContext();
      mType = Type::createReferencedType(context, rootEl);
    }

    //-------------------------------------------------------------------------
    IWrapperTypes::PropertyPtr IWrapperTypes::Property::create(ContextPtr context)
    {
      auto pThis(make_shared<Property>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::PropertyPtr IWrapperTypes::Property::create(
                                                               ContextPtr context,
                                                               const ElementPtr &el
                                                               ) throw (InvalidContent)
    {
      auto pThis(make_shared<Property>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Property::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "property";
      
      ElementPtr rootEl = Element::create(objectName);

      if (mDefaultValue.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("default", mDefaultValue));
      }

      if (mType) {
        rootEl->adoptAsLastChild(mType->createReferenceTypeElement());
      }

      return rootEl;
    }
    
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::Property::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());
      hasher->update(":");
      hasher->update(mDefaultValue);
      if (mType) {
        hasher->update(":");
        hasher->update(mType->getPath());
        hasher->update(":");
        hasher->update(mType->getMappingName());
      } else {
        hasher->update(":__");
      }
      hasher->update(":end");

      return hasher->finalizeAsString();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Property::resolveTypedefs() throw (InvalidContent)
    {
      if (mType) {
        mType = mType->getTypeBypassingTypedefIfNoop();
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::createProperties(
                                         ContextPtr context,
                                         ElementPtr propertiesEl,
                                         PropertyList &outProperties
                                         ) throw (InvalidContent)
    {
      if (!propertiesEl) return;

      auto propertyEl = propertiesEl->findFirstChildElement("property");

      while (propertyEl) {
        auto propertyObj = Property::create(context, propertyEl);
        outProperties.push_back(propertyObj);
        propertyEl = propertyEl->findNextSiblingElement("property");
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes::Method
    #pragma mark

    //-------------------------------------------------------------------------
    void IWrapperTypes::Method::init()
    {
      Context::init();
    }
    
    //-------------------------------------------------------------------------
    void IWrapperTypes::Method::init(const ElementPtr &rootEl) throw (InvalidContent)
    {
      Context::init(rootEl);
      Context::parse(rootEl);

      auto context = toContext();

      auto modifiersStr = aliasLookup(UseHelper::getElementTextAndDecode(rootEl->findFirstChildElement("modifiers")));
      if (modifiersStr.hasData()) {
        mModifiers = toMethodModifier(modifiersStr);
      }

      {
        auto propertiesEl = rootEl->findFirstChildElement("arguments");
        if (propertiesEl) {
          auto propertyEl = propertiesEl->findFirstChildElement("argument");
          while (propertyEl) {
            auto propertyObj = Property::create(context, propertyEl);
            mArguments.push_back(propertyObj);
            propertyEl = propertyEl->findNextSiblingElement("argument");
          }
        }
      }

      {
        auto throwsEl = rootEl->findFirstChildElement("throws");
        if (throwsEl) {
          auto throwEl = throwsEl->findFirstChildElement("throw");
          while (throwEl) {
            auto typeObj = Type::createReferencedType(context, throwEl);
            if (!typeObj) {
              ZS_THROW_CUSTOM(InvalidContent, String("Thrown object was not found for: ") + getMappingName());
            }
            mThrows.push_back(typeObj);
            throwEl = throwEl->findNextSiblingElement("throw");
          }
        }
      }
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::MethodPtr IWrapperTypes::Method::create(ContextPtr context)
    {
      auto pThis(make_shared<Method>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    IWrapperTypes::MethodPtr IWrapperTypes::Method::create(
                                                           ContextPtr context,
                                                           const ElementPtr &el
                                                           ) throw (InvalidContent)
    {
      auto pThis(make_shared<Method>(make_private{}, context));
      pThis->mThisWeak = pThis;
      pThis->init(el);
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    ElementPtr IWrapperTypes::Method::createElement(const char *objectName) const
    {
      if (NULL == objectName) objectName = "method";
      
      ElementPtr rootEl = Element::create(objectName);

      String modifieresStr = toString(mModifiers);
      if (modifieresStr.hasData()) {
        rootEl->adoptAsLastChild(UseHelper::createElementWithTextAndJSONEncode("modifiers", modifieresStr));
      }

      if (mArguments.size() > 0) {
        auto argumentsEl = Element::create("arguments");
        for (auto iter = mArguments.begin(); iter != mArguments.end(); ++iter) {
          auto propertyObj = (*iter);
          argumentsEl->adoptAsLastChild(propertyObj->createElement("argument"));
        }
      }

      if (mThrows.size() > 0) {
        auto throwsEl = Element::create("throws");
        for (auto iter = mThrows.begin(); iter != mThrows.end(); ++iter) {
          auto throwObj = (*iter);
          auto throwEl = Element::create("throw");
          throwEl->adoptAsLastChild(throwObj->createReferenceTypeElement());
          throwsEl->adoptAsLastChild(throwEl);
        }
      }

      return rootEl;
    }
    
    
    //-------------------------------------------------------------------------
    String IWrapperTypes::Method::hash() const
    {
      auto hasher = IHasher::sha256();
      
      hasher->update(Context::hash());
      hasher->update(":");
      hasher->update(toString(mModifiers));

      hasher->update(":arguments:");
      for (auto iter = mArguments.begin(); iter != mArguments.end(); ++iter) {
        auto propertyObj = (*iter);
        hasher->update(propertyObj->getPath());
        hasher->update(":");
        hasher->update(propertyObj->getMappingName());
        hasher->update(":next:");
      }

      hasher->update(":throws:");
      for (auto iter = mThrows.begin(); iter != mThrows.end(); ++iter) {
        auto throwObj = (*iter);
        hasher->update(throwObj->getPath());
        hasher->update(":");
        hasher->update(throwObj->getMappingName());
        hasher->update(":next:");
      }

      hasher->update(":end");
      
      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::Method::resolveTypedefs() throw (InvalidContent)
    {
      for (auto iter = mArguments.begin(); iter != mArguments.end(); ++iter) {
        auto obj = (*iter);
        obj->resolveTypedefs();
      }

      for (auto iter_doNotUse = mThrows.begin(); iter_doNotUse != mThrows.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        TypePtr type = (*current);
        if (!type) continue;

        TypePtr bypassType = type->getTypeBypassingTypedefIfNoop();
        if (type == bypassType) continue;

        mThrows.insert(current, bypassType);
        mThrows.erase(current);
      }
    }

    //-------------------------------------------------------------------------
    void IWrapperTypes::createMethods(
                                      ContextPtr context,
                                      ElementPtr methodsEl,
                                      MethodList &outMethods
                                      ) throw (InvalidContent)
    {
      if (!methodsEl) return;

      auto methodEl = methodsEl->findFirstChildElement("method");

      while (methodEl) {
        auto methodObj = Method::create(context, methodEl);
        outMethods.push_back(methodObj);
        methodEl = methodEl->findNextSiblingElement("method");
      }
    }

  } // namespace eventing
} // namespace zsLib
