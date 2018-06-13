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

#include <zsLib/eventing/types.h>
#include <zsLib/eventing/IEventingTypes.h>

#include <map>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
        class IDLCompiler;
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IIDLTypes
    //

    interaction IIDLTypes : public IEventingTypes
    {
      ZS_DECLARE_STRUCT_PTR(Project);
      ZS_DECLARE_STRUCT_PTR(Context);
      ZS_DECLARE_STRUCT_PTR(Namespace);
      ZS_DECLARE_STRUCT_PTR(Type);
      ZS_DECLARE_STRUCT_PTR(BasicType);
      ZS_DECLARE_STRUCT_PTR(EnumType);
      ZS_DECLARE_STRUCT_PTR(EnumTypeValue);
      ZS_DECLARE_STRUCT_PTR(TypedefType);
      ZS_DECLARE_STRUCT_PTR(Struct);
      ZS_DECLARE_STRUCT_PTR(GenericType);
      ZS_DECLARE_STRUCT_PTR(TemplatedStructType);
      ZS_DECLARE_STRUCT_PTR(Property);
      ZS_DECLARE_STRUCT_PTR(Method);

      enum Modifiers : ULONG
      {
        Modifier_First,

        Modifier_Struct_Dictionary = Modifier_First,  // struct is treated as simple structured dictionary data; properties default without getters/setters
        Modifier_Struct_Exception,                    // struct is meant for throws declarations

        Modifier_Method_Ctor,
        Modifier_Method_EventHandler,
        Modifier_Method_Default,
        Modifier_Method_Delete,

        Modifier_Method_Argument_In,
        Modifier_Method_Argument_Out,
        Modifier_Method_Argument_Grouping,  // grouped arguments for languages (e.g. objective-C) that allow "with" argument groupings

        Modifier_Property_ReadOnly,         // value can be fetched but not set
        Modifier_Property_WriteOnly,        // value can be set but not fetched
        Modifier_Property_Getter,           // value is not stored in wrapper, fetched from code
        Modifier_Property_Setter,           // value is not set in wrapper, set in code

        Modifier_Static,                    // method or property is static
        Modifier_AltName,
        Modifier_Special,                   // namespace is not output, struct wrapper is created through special / custom processing
        Modifier_Platform,                  // platform specific extensions
        Modifier_Nullable,                  // value of null is legal
        Modifier_Optional,                  // optional type whose value may not be set
        Modifier_Dynamic,                   // type might be of derived type
        Modifier_Obsolete,                  // method, property, namespace or struct is marked as obsolete

        Modifier_Last = Modifier_Obsolete,
      };

      static const char *toString(Modifiers value) noexcept;
      static int getTotalParams(Modifiers value) noexcept;
      static Modifiers toModifier(const char *value) noexcept(false); // throws InvalidArgument

      static bool isValidForAll(Modifiers value) noexcept;
      static bool isValidForNamespace(Modifiers value) noexcept;
      static bool isValidForStruct(Modifiers value) noexcept;
      static bool isValidForMethod(Modifiers value) noexcept;
      static bool isValidForMethodArgument(Modifiers value) noexcept;
      static bool isValidForProperty(Modifiers value) noexcept;

      typedef String Name;
      typedef String Value;
      typedef std::list<ContextPtr> ContextList;
      typedef std::list<NamespacePtr> NamespaceList;
      typedef std::map<Name, NamespacePtr> NamespaceMap;
      typedef std::map<Name, TypePtr> RelatedStructMap;
      typedef std::list<TypePtr> TypeList;
      typedef std::map<Name, TypePtr> TypeMap;
      typedef std::map<Name, BasicTypePtr> BasicTypeMap;
      typedef std::set<TypedefTypePtr> TypedefTypeSet;
      typedef std::list<TypedefTypePtr> TypedefTypeList;
      typedef std::map<Name, TypedefTypePtr> TypedefTypeMap;
      typedef std::list<GenericTypePtr> GenericTypeList;
      typedef std::map<Name, GenericTypePtr> GenericTypeMap;
      typedef std::list<TemplatedStructTypePtr> TemplatedStructTypeList;
      typedef std::map<Name, TemplatedStructTypePtr> TemplatedStructTypeMap;
      typedef std::map<Name, StructPtr> StructMap;
      typedef std::map<Name, EnumTypePtr> EnumMap;
      typedef std::list<EnumTypeValuePtr> EnumTypeValueList;
      typedef std::list<PropertyPtr> PropertyList;
      typedef std::map<Name, PropertyPtr> PropertyMap;
      typedef std::list<MethodPtr> MethodList;
      typedef std::map<Name, MethodPtr> MethodMap;
      typedef std::pair<Name, Value> NameValuePair;
      typedef std::list<NameValuePair> NameValueList;
      typedef std::set<Value> ValueSet;
      typedef std::map<Name, StringList> StringListMap;

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Context
      //

      struct Context
      {
      public:
        friend class tool::internal::IDLCompiler;

      protected:
        struct make_private {};

      public:
        struct FindTypeOptions
        {
          bool mSearchParents {true};
        };

      public:
        ContextWeakPtr mContext;
        String mName;

        ElementPtr mDocumentation;
        StringListMap mModifiers;

      public:
        virtual ~Context();

        virtual ContextPtr toContext() const noexcept;
        virtual ProjectPtr toProject() const noexcept;
        virtual NamespacePtr toNamespace() const noexcept;
        virtual TypePtr toType() const noexcept;
        virtual BasicTypePtr toBasicType() const noexcept;
        virtual EnumTypePtr toEnumType() const noexcept;
        virtual EnumTypeValuePtr toEnumTypeValue() const noexcept;
        virtual TypedefTypePtr toTypedefType() const noexcept;
        virtual GenericTypePtr toGenericType() const noexcept;
        virtual TemplatedStructTypePtr toTemplatedStructType() const noexcept;
        virtual StructPtr toStruct() const noexcept;
        virtual PropertyPtr toProperty() const noexcept;
        virtual MethodPtr toMethod() const noexcept;

        virtual ElementPtr createElement(const char *objectName = NULL) const noexcept = 0;
        virtual String hash() const noexcept;

        virtual String getMappingName() const noexcept;

        ContextPtr getParent() const noexcept;
        ContextPtr getRoot() const noexcept;
        ProjectPtr getProject() const noexcept;
        String getPath() const noexcept;
        String getPathName() const noexcept;

        virtual TypePtr findType(
                                 const String &typeNameWithPath,
                                 const FindTypeOptions *options = NULL
                                 ) const noexcept;
        virtual TypePtr findType(
                                 const String &pathStr,
                                 const String &typeName,
                                 const FindTypeOptions &options
                                 ) const noexcept;
        
        virtual bool hasModifier(Modifiers modifier) const noexcept;
        virtual String getModifierValue(
                                        Modifiers modifier,
                                        size_t index = 0
                                        ) const noexcept;
        virtual void getModifierValues(
                                       Modifiers modifier,
                                       StringList &outValues
                                       ) const noexcept;

        virtual void clearModifier(Modifiers modifier) noexcept;
        virtual void setModifier(Modifiers modifier) noexcept;
        virtual void setModifier(
                                 Modifiers modifier,
                                 const String &value
                                 ) noexcept;
        virtual void setModifier(
                                 Modifiers modifier,
                                 const StringList &values
                                 ) noexcept;

        virtual void resolveTypedefs() noexcept(false); // throws InvalidContent
        virtual bool fixTemplateHashMapping() noexcept;

        virtual String aliasLookup(const String &value) noexcept;

      protected:
        Context(
                const make_private &,
                ContextPtr context
                ) noexcept;

        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false);

        virtual void write(ElementPtr &rootEl) const noexcept;
        virtual void parse(const ElementPtr &rootEl) noexcept(false); // throw InvalidContent
        virtual void copyContentsFrom(ContextPtr originalContext) noexcept;

      protected:
        ContextWeakPtr mThisWeak;
      };

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Project
      //

      struct Project : public Context
      {
        AliasMap mAliases;

        NamespacePtr mGlobal;
        BasicTypeMap mBasicTypes;

        ValueSet mDefinedExclusives;

        Project(const make_private &v) noexcept;
        ~Project() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static ProjectPtr create() noexcept;
        static ProjectPtr create(const ElementPtr &el) noexcept(false); // throws InvalidContent

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; // throws InvalidContent
        String hash() const noexcept override;
        TypePtr findType(
                          const String &typeNameWithPath,
                          const FindTypeOptions *options = NULL
                          ) const noexcept override {return Context::findType(typeNameWithPath, options);  }
        TypePtr findType(
                         const String &pathStr,
                         const String &typeName,
                         const FindTypeOptions &options
                         ) const noexcept override;
        void resolveTypedefs() noexcept(false) override; // throws InvalidContent
        bool fixTemplateHashMapping() noexcept override;
        String aliasLookup(const String &value) noexcept override;

        BasicTypePtr findBasicType(IEventingTypes::PredefinedTypedefs basicType) const noexcept;

        ProjectPtr toProject() const noexcept override;

      protected:
        void createBaseTypes() noexcept;
      };

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Namespace
      //

      struct Namespace : public Context
      {
        NamespaceMap mNamespaces;
        EnumMap mEnums;
        StructMap mStructs;
        TypedefTypeMap mTypedefs;

        Namespace(
                  const make_private &v,
                  ContextPtr context
                  ) noexcept;
        ~Namespace() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static NamespacePtr create(ContextPtr context) noexcept;
        static NamespacePtr createForwards(
                                           ContextPtr context,
                                           const ElementPtr &el
                                           ) noexcept(false); //throw InvalidContent

        NamespacePtr toNamespace() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; //  throws InvalidContent
        String hash() const noexcept override;
        TypePtr findType(
                         const String &typeNameWithPath,
                         const FindTypeOptions *options = NULL
                         ) const noexcept override {return Context::findType(typeNameWithPath, options);  }
        TypePtr findType(
                         const String &pathStr,
                         const String &typeName,
                         const FindTypeOptions &options
                         ) const noexcept override;
        void resolveTypedefs() noexcept(false) override; // throws InvalidContent
        bool fixTemplateHashMapping() noexcept override;

        virtual NamespacePtr findNamespace(const String &nameWithPath) const noexcept;
        virtual NamespacePtr findNamespace(
                                           const String &pathStr,
                                           const String &name
                                           ) const noexcept;

        bool isGlobal() const noexcept;
      };

      static void createNamespaceForwards(
                                          ContextPtr context,
                                          ElementPtr namespacesEl,
                                          NamespaceMap &outNamespaces
                                          ) noexcept(false); // throws InvalidContent

      static void parseNamespaces(
                                  ContextPtr context,
                                  ElementPtr namespacesEl,
                                  NamespaceMap &ioNamespaces
                                  ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Type
      //

      struct Type : public Context
      {
        Type(
             const make_private &v,
             ContextPtr context
             ) noexcept;
        ~Type() noexcept override;

        TypePtr toType() const noexcept override;

        static TypePtr createReferencedType(
                                            ContextPtr context,
                                            ElementPtr parentEl
                                            ) noexcept(false); // throws InvalidContent

        virtual TypePtr getOriginalType() const noexcept;

        ElementPtr createReferenceTypeElement() const noexcept;
      };

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::BasicType
      //

      struct BasicType : public Type
      {
        PredefinedTypedefs mBaseType {PredefinedTypedef_First};

        BasicType(
                  const make_private &v,
                  ContextPtr context
                  ) noexcept;
        ~BasicType() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static BasicTypePtr create(ContextPtr context) noexcept;

        BasicTypePtr toBasicType() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        String hash() const noexcept override;
      };

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::EnumType
      //

      struct EnumType : public Type
      {
        PredefinedTypedefs mBaseType {PredefinedTypedef_int};

        EnumTypeValueList mValues;

        EnumType(
                 const make_private &v,
                 ContextPtr context
                 ) noexcept;
        ~EnumType() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static EnumTypePtr create(ContextPtr context) noexcept;
        static EnumTypePtr createForwards(
                                          ContextPtr context,
                                          const ElementPtr &el
                                          ) noexcept(false); // throws InvalidContent

        EnumTypePtr toEnumType() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; // throws InvalidContent
        String hash() const noexcept override;
      };

      static void createEnumForwards(
                                     ContextPtr context,
                                     ElementPtr enumsEl,
                                     EnumMap &outEnums
                                     ) noexcept(false); // throws InvalidContent

      static void parseEnums(
                             ContextPtr context,
                             ElementPtr enumsEl,
                             EnumMap &ioEnums
                             ) noexcept(false); // throws InvalidContent


      //-----------------------------------------------------------------------
      //
      // IIDLTypes::EnumTypeValue
      //

      struct EnumTypeValue : public Context
      {
        String mValue;

        EnumTypeValue(
                      const make_private &v,
                      ContextPtr context
                      ) noexcept;
        ~EnumTypeValue() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static EnumTypeValuePtr create(ContextPtr context) noexcept;
        static EnumTypeValuePtr create(ContextPtr context, const ElementPtr &el) noexcept(false); // throws InvalidContent

        EnumTypeValuePtr toEnumTypeValue() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; //  throws InvalidContent
        String hash() const noexcept override;
      };

      static void createEnumValues(
                                   ContextPtr context,
                                   ElementPtr enumsEl,
                                   EnumTypeValueList &outEnumValues
                                   ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::TypedefType
      //

      struct TypedefType : public Type
      {
        TypeWeakPtr mOriginalType;

        TypedefType(
                    const make_private &v,
                    ContextPtr context
                    ) noexcept;
        ~TypedefType() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static TypedefTypePtr create(ContextPtr context) noexcept;
        static TypedefTypePtr createForwards(
                                             ContextPtr context,
                                             const ElementPtr &el
                                             ) noexcept(false); // throw (InvalidContent);

        TypedefTypePtr toTypedefType() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; // throws InvalidContent
        String hash() const noexcept override;

        void resolveTypedefs() noexcept(false) override; // throws InvalidContent

        TypePtr getOriginalType() const noexcept override;
      };

      static void createTypedefForwards(
                                        ContextPtr context,
                                        ElementPtr typedefsEl,
                                        TypedefTypeMap &outTypedefs
                                        ) noexcept(false); // throws InvalidContent

      static void parseTypedefs(
                                ContextPtr context,
                                ElementPtr typedefsEl,
                                TypedefTypeMap &ioTypedefs
                                ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Struct
      //

      struct Struct : public Type
      {
        GenericTypeList mGenerics;
        TypeList mGenericDefaultTypes;

        TemplatedStructTypeMap mTemplatedStructs;

        RelatedStructMap mIsARelationships;

        EnumMap mEnums;
        StructMap mStructs;
        TypedefTypeMap mTypedefs;

        PropertyList mProperties;
        MethodList mMethods;

        Struct(
               const make_private &v,
               ContextPtr context
               ) noexcept;
        ~Struct() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static StructPtr create(ContextPtr context) noexcept;
        static StructPtr createForwards(
                                        ContextPtr context,
                                        const ElementPtr &el
                                        ) noexcept(false); // throws InvalidContent

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; // throws InvalidContent
        String hash() const noexcept override;

        TypePtr findType(
                          const String &typeNameWithPath,
                          const FindTypeOptions *options = NULL
                          ) const noexcept override {return Context::findType(typeNameWithPath, options);  }
        TypePtr findType(
                         const String &pathStr,
                         const String &typeName,
                         const FindTypeOptions &options
                         ) const noexcept override;
        void resolveTypedefs() noexcept(false) override; // throws InvalidContent
        bool fixTemplateHashMapping() noexcept override;

        StructPtr toStruct() const noexcept override;

        bool hasExistingNonForwardedData() const noexcept;

        StructPtr getRootStruct() const noexcept;
      };

      static void createStructForwards(
                                       ContextPtr context,
                                       ElementPtr structsEl,
                                       StructMap &outStructs
                                       ) noexcept(false); // throws InvalidContent

      static void parseStructs(
                               ContextPtr context,
                               ElementPtr structsEl,
                               StructMap &ioStructs
                               ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::TypedefType
      //

      struct GenericType : public Type
      {
      public:
        GenericType(
                    const make_private &v,
                    ContextPtr context
                    ) noexcept;
        ~GenericType() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static GenericTypePtr create(ContextPtr context) noexcept;
        static GenericTypePtr createForward(
                                            ContextPtr context,
                                            const ElementPtr &el
                                            ) noexcept(false); // throws InvalidContent

        GenericTypePtr toGenericType() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; //  throws InvalidContent
        String hash() const noexcept override;
      };

      static void createGenericForwards(
                                        ContextPtr context,
                                        ElementPtr genericsEl,
                                        GenericTypeList &outGenerics
                                        ) noexcept(false); // throws InvalidContent

      static void parseGenerics(
                                ContextPtr context,
                                ElementPtr structsEl,
                                GenericTypeList &ioGenerics
                                ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::TypedefType
      //

      struct TemplatedStructType : public Type
      {
        TypeList mTemplateArguments;

      public:
        TemplatedStructType(
                            const make_private &v,
                            ContextPtr context
                            ) noexcept;
        ~TemplatedStructType() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static TemplatedStructTypePtr create(ContextPtr context) noexcept;
        static TemplatedStructTypePtr createForwards(
                                                     ContextPtr context,
                                                     const ElementPtr &el
                                                     ) noexcept(false); // throws InvalidContent

        TemplatedStructTypePtr toTemplatedStructType() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        void parse(const ElementPtr &rootEl) noexcept(false) override; // throws InvalidContent
        String hash() const noexcept override;

        void resolveTypedefs() noexcept(false) override; //  throws InvalidContent

        String calculateTemplateID() const noexcept;
        StructPtr getParentStruct() const noexcept;
      };

      static void createTemplatedStructTypeForwards(
                                                    ContextPtr context,
                                                    ElementPtr templatedStructsEl,
                                                    TemplatedStructTypeMap &outTemplatedStruct
                                                    ) noexcept(false); // throw (InvalidContent);

      static void parseTemplatedStructTypes(
                                            ContextPtr context,
                                            ElementPtr templatedStructsEl,
                                            TemplatedStructTypeMap &ioTemplatedStruct
                                            ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Property
      //

      struct Property : public Context
      {
        TypePtr mType;
        String mDefaultValue;

        Property(
                 const make_private &v,
                 ContextPtr context
                 ) noexcept;
        ~Property() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static PropertyPtr create(ContextPtr context) noexcept;
        static PropertyPtr create(
                                  ContextPtr context,
                                  const ElementPtr &el
                                  ) noexcept(false); // throws InvalidContent

        PropertyPtr toProperty() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        String hash() const noexcept override;

        void resolveTypedefs() noexcept(false) override; // throws InvalidContent
      };

      static void createProperties(
                                   ContextPtr context,
                                   ElementPtr propertiesEl,
                                   PropertyList &outProperties
                                   ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IIDLTypes::Method
      //

      struct Method : public Context
      {
        TypePtr mResult;
        PropertyList mArguments;

        TypeList mThrows;

        Method(
               const make_private &v,
               ContextPtr context
               ) noexcept;
        ~Method() noexcept override;

      protected:
        void init() noexcept;
        void init(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

      public:
        static MethodPtr create(ContextPtr context) noexcept;
        static MethodPtr create(
                                ContextPtr context,
                                const ElementPtr &el
                                ) noexcept(false); // throws InvalidContent

        MethodPtr toMethod() const noexcept override;

        ElementPtr createElement(const char *objectName = NULL) const noexcept override;
        String hash() const noexcept override;

        void resolveTypedefs() noexcept(false) override; //  throws InvalidContent
      };

      static void createMethods(
                                ContextPtr context,
                                ElementPtr methodsEl,
                                MethodList &outMethods
                                ) noexcept(false); // throws InvalidContent
    };

  } // namespace eventing
} // namespace zsLib
