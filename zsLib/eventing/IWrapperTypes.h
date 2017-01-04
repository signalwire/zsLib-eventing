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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IWrapperTypes
    #pragma mark

    interaction IWrapperTypes : public IEventingTypes
    {
      ZS_DECLARE_STRUCT_PTR(Project);
      ZS_DECLARE_STRUCT_PTR(Context);
      ZS_DECLARE_STRUCT_PTR(Namespace);
      ZS_DECLARE_STRUCT_PTR(Type);
      ZS_DECLARE_STRUCT_PTR(BasicType);
      ZS_DECLARE_STRUCT_PTR(EnumType);
      ZS_DECLARE_STRUCT_PTR(TypedefType);
      ZS_DECLARE_STRUCT_PTR(Struct);
      ZS_DECLARE_STRUCT_PTR(Property);
      ZS_DECLARE_STRUCT_PTR(Method);

      enum Visibilities
      {
        Visibility_First,
        
        Visibility_Public = Visibility_First,
        Visibility_Protected,
        Visibility_Private,
        
        Visibility_Last = Visibility_Private,
      };
      
      static const char *toString(Visibilities value);
      static Visibilities toVisibility(const char *value) throw (InvalidArgument);

      enum TypeModifiers : ULONG
      {
        TypeModifier_None =       0,
        TypeModifier_Generic =    (1 << 0),
        TypeModifier_Const =      (1 << 1),
        TypeModifier_Reference =  (1 << 2),
        TypeModifier_Array =      (1 << 3),
        TypeModifier_Ptr =        (1 << 8),
        TypeModifier_RawPtr =     (1 << 9),
        TypeModifier_SharedPtr =  (1 << 10),
        TypeModifier_WeakPtr =    (1 << 11),
        TypeModifier_UniPtr =     (1 << 12),
      };

      static String toString(TypeModifiers value);
      static TypeModifiers toTypeModifier(const char *value) throw (InvalidArgument);
      static bool hasModifier(
                              TypeModifiers value,
                              TypeModifiers checkFor
                              );
      static TypeModifiers addModifiers(
                                        TypeModifiers value,
                                        TypeModifiers addModifiers
                                        );
      static TypeModifiers removeModifiers(
                                           TypeModifiers value,
                                           TypeModifiers addModifiers
                                           );

      enum StructModifiers : ULONG
      {
        StructModifier_None =           0,
        StructModifier_Generic =        (1 << 0),
        StructModifier_Delegate =       (1 << 4),
        StructModifier_Subscription =   (1 << 5),
      };

      static String toString(StructModifiers value);
      static StructModifiers toStructModifier(const char *value) throw (InvalidArgument);
      static bool hasModifier(
                              StructModifiers value,
                              StructModifiers checkFor
                              );
      static StructModifiers addModifiers(
                                          StructModifiers value,
                                          StructModifiers addModifiers
                                          );
      static StructModifiers removeModifiers(
                                             StructModifiers value,
                                             StructModifiers addModifiers
                                             );

      enum MethodModifiers : ULONG
      {
        MethodModifier_None =     0,
        MethodModifier_Ctor =     (1 << 0),

        MethodModifier_Static =   (1 << 1),
        MethodModifier_Dynamic =  (1 << 2),

        MethodModifier_Const =    (1 << 4),
      };

      static String toString(MethodModifiers value);
      static MethodModifiers toMethodModifier(const char *value) throw (InvalidArgument);
      static bool hasModifier(
                              MethodModifiers value,
                              MethodModifiers checkFor
                              );
      static MethodModifiers addModifiers(
                                          MethodModifiers value,
                                          MethodModifiers addModifiers
                                          );
      static MethodModifiers removeModifiers(
                                             MethodModifiers value,
                                             MethodModifiers addModifiers
                                             );

      typedef String Name;
      typedef String Value;
      typedef std::list<ContextPtr> ContextList;
      typedef std::list<NamespacePtr> NamespaceList;
      typedef std::map<Name, NamespacePtr> NamespaceMap;
      typedef std::pair<Visibilities, StructPtr> VisibleStructPair;
      typedef std::map<Name, VisibleStructPair> RelatedStructMap;
      typedef std::list<TypePtr> TypeList;
      typedef std::map<Name, TypePtr> TypeMap;
      typedef std::map<Name, BasicTypePtr> BasicTypeMap;
      typedef std::list<TypedefTypePtr> TypedefTypeList;
      typedef std::map<Name, TypedefTypePtr> TypedefTypeMap;
      typedef std::map<Name, StructPtr> StructMap;
      typedef std::map<Name, EnumTypePtr> EnumMap;
      typedef std::list<PropertyPtr> PropertyList;
      typedef std::map<Name, PropertyPtr> PropertyMap;
      typedef std::list<MethodPtr> MethodList;
      typedef std::map<Name, MethodPtr> MethodMap;
      typedef std::pair<Name, Value> NameValuePair;
      typedef std::list<NameValuePair> NameValueList;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Context
      #pragma mark

      struct Context
      {
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
        
      public:
        virtual ~Context() {mThisWeak.reset();}
        
        virtual ContextPtr toContext() const          {return mThisWeak.lock();}
        virtual ProjectPtr toProject() const          {return ProjectPtr();}
        virtual NamespacePtr toNamespace() const      {return NamespacePtr();}
        virtual TypePtr toType() const                {return TypePtr();}
        virtual BasicTypePtr toBasicType() const      {return BasicTypePtr();}
        virtual EnumTypePtr toEnumType() const        {return EnumTypePtr();}
        virtual TypedefTypePtr toTypedefType() const  {return TypedefTypePtr();}
        virtual StructPtr toStruct() const            {return StructPtr();}
        virtual PropertyPtr toProperty() const        {return PropertyPtr();}
        virtual MethodPtr toMethod() const            {return MethodPtr();}
        
        virtual ElementPtr createElement(const char *objectName = NULL) const = 0;
        virtual String hash() const;
        
        virtual String getMappingName() const         {return mName;}

        ContextPtr getParent() const;
        ContextPtr getRoot() const;
        ProjectPtr getProject() const;
        String getPath() const;

        virtual TypePtr findType(
                                 const String &typeNameWithPath,
                                 const FindTypeOptions *options = NULL
                                 ) const;
        virtual TypePtr findType(
                                 const String &pathStr,
                                 const String &typeName,
                                 const FindTypeOptions &options
                                 ) const;

        virtual void resolveTypedefs() throw (InvalidContent) {}

        virtual String aliasLookup(const String &value);
        
      protected:
        Context(
                const make_private &,
                ContextPtr context
                ) : mContext(context) {}

        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);

        virtual void write(ElementPtr &rootEl) const;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent);

      protected:
        ContextWeakPtr mThisWeak;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Project
      #pragma mark

      struct Project : public Context
      {
        AliasMap mAliases;

        NamespacePtr mGlobal;
        BasicTypeMap mBasicTypes;

        Project(const make_private &v) : Context(v, ContextPtr()) {}
        
      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);

      public:
        static ProjectPtr create();
        static ProjectPtr create(const ElementPtr &el) throw (InvalidContent);

        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent) override;
        virtual String hash() const override;

        virtual TypePtr findType(
                                 const String &pathStr,
                                 const String &typeName,
                                 const FindTypeOptions &options
                                 ) const override;
        virtual void resolveTypedefs() throw (InvalidContent) override;
        virtual String aliasLookup(const String &value) override;

        BasicTypePtr findBasicType(IEventingTypes::PredefinedTypedefs basicType) const;

        virtual ProjectPtr toProject() const override {return ZS_DYNAMIC_PTR_CAST(Project, toContext());}

      protected:
        void createBaseTypes();
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Namespace
      #pragma mark

      struct Namespace : public Context
      {
        NamespaceMap mNamespaces;
        EnumMap mEnums;
        StructMap mStructs;
        TypedefTypeMap mTypedefs;

        Namespace(
                  const make_private &v,
                  ContextPtr context
                  ) : Context(v, context) {}
        
      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);

      public:
        static NamespacePtr create(ContextPtr context);
        static NamespacePtr createForwards(
                                           ContextPtr context,
                                           const ElementPtr &el
                                           ) throw (InvalidContent);

        virtual NamespacePtr toNamespace() const override {return ZS_DYNAMIC_PTR_CAST(Namespace, toContext());}
        
        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent) override;
        virtual String hash() const override;
        virtual TypePtr findType(
                                 const String &pathStr,
                                 const String &typeName,
                                 const FindTypeOptions &options
                                 ) const override;
        virtual void resolveTypedefs() throw (InvalidContent) override;
      };

      static void createNamespaceForwards(
                                          ContextPtr context,
                                          ElementPtr namespacesEl,
                                          NamespaceMap &outNamespaces
                                          ) throw (InvalidContent);

      static void parseNamespaces(
                                  ContextPtr context,
                                  ElementPtr namespacesEl,
                                  NamespaceMap &ioNamespaces
                                  ) throw (InvalidContent);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Type
      #pragma mark

      struct Type : public Context
      {
        Type(
             const make_private &v,
             ContextPtr context
             ) : Context(v, context) {}

        virtual TypePtr toType() const override {return ZS_DYNAMIC_PTR_CAST(Type, toContext());}

        static TypePtr createReferencedType(
                                            ContextPtr context,
                                            ElementPtr parentEl
                                            ) throw (InvalidContent);

        virtual TypePtr getTypeBypassingTypedefIfNoop() const {return toType();}

        ElementPtr createReferenceTypeElement() const;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::BasicType
      #pragma mark

      struct BasicType : public Type
      {
        PredefinedTypedefs mBaseType {PredefinedTypedef_First};
        
        BasicType(
                  const make_private &v,
                  ContextPtr context
                  ) : Type(v, context) {}

      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static BasicTypePtr create(ContextPtr context);
        
        virtual BasicTypePtr toBasicType() const override {return ZS_DYNAMIC_PTR_CAST(BasicType, toContext());}

        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual String hash() const override;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::EnumType
      #pragma mark

      struct EnumType : public Type
      {
        PredefinedTypedefs mBaseType {PredefinedTypedef_int};

        NameValueList mValues;

        EnumType(
                 const make_private &v,
                 ContextPtr context
                 ) : Type(v, context) {}

      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static EnumTypePtr create(ContextPtr context);
        static EnumTypePtr createForwards(
                                          ContextPtr context,
                                          const ElementPtr &el
                                          ) throw (InvalidContent);
        
        virtual EnumTypePtr toEnumType() const override {return ZS_DYNAMIC_PTR_CAST(EnumType, toContext());}
        
        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent) override;
        virtual String hash() const override;
      };

      static void createEnumForwards(
                                     ContextPtr context,
                                     ElementPtr enumsEl,
                                     EnumMap &outEnums
                                     ) throw (InvalidContent);

      static void parseEnums(
                             ContextPtr context,
                             ElementPtr enumsEl,
                             EnumMap &ioEnums
                             ) throw (InvalidContent);
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::TypedefType
      #pragma mark

      struct TypedefType : public Type
      {
        TypeModifiers mModifiers {};
        TypeWeakPtr mOriginalType;    // not set if generic
        size_t mArraySize {};         // allow for one dimensional array

        TypedefType(
                    const make_private &v,
                    ContextPtr context
                    ) : Type(v, context) {}
        
      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static TypedefTypePtr create(ContextPtr context);
        static TypedefTypePtr createForwards(
                                             ContextPtr context,
                                             const ElementPtr &el
                                             ) throw (InvalidContent);

        virtual TypedefTypePtr toTypedefType() const override {return ZS_DYNAMIC_PTR_CAST(TypedefType, toContext());}

        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent) override;
        virtual String hash() const override;

        virtual void resolveTypedefs() throw (InvalidContent) override;

        virtual TypePtr getTypeBypassingTypedefIfNoop() const override;
      };

      static void createTypedefForwards(
                                        ContextPtr context,
                                        ElementPtr typedefsEl,
                                        TypedefTypeMap &outTypedefs
                                        ) throw (InvalidContent);

      static void parseTypedefs(
                                ContextPtr context,
                                ElementPtr typedefsEl,
                                TypedefTypeMap &ioTypedefs
                                ) throw (InvalidContent);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Struct
      #pragma mark

      struct Struct : public Type
      {
        StructModifiers mModifiers {};
        TypeList mTemplates;

        String mTemplateID;

        RelatedStructMap mIsARelationships;

        Visibilities mCurrentVisbility {Visibility_First};

        EnumMap mEnums;
        StructMap mStructs;
        TypedefTypeMap mTypedefs;

        PropertyList mProperties;
        MethodList mMethods;

        Struct(
               const make_private &v,
               ContextPtr context
               ) : Type(v, context) {}

      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static StructPtr create(ContextPtr context);
        static StructPtr createForwards(
                                        ContextPtr context,
                                        const ElementPtr &el
                                        ) throw (InvalidContent);
        
        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual void parse(const ElementPtr &rootEl) throw (InvalidContent) override;
        virtual String hash() const override;

        virtual String getMappingName() const override;
        
        virtual TypePtr findType(
                                 const String &pathStr,
                                 const String &typeName,
                                 const FindTypeOptions &options
                                 ) const override;
        virtual void resolveTypedefs() throw (InvalidContent) override;

        virtual StructPtr toStruct() const override {return ZS_DYNAMIC_PTR_CAST(Struct, toContext());}
      };

      static void createStructForwards(
                                       ContextPtr context,
                                       ElementPtr structsEl,
                                       StructMap &outStructs
                                       ) throw (InvalidContent);

      static void parseStructs(
                               ContextPtr context,
                               ElementPtr structsEl,
                               StructMap &ioStructs
                               ) throw (InvalidContent);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Property
      #pragma mark

      struct Property : public Context
      {
        TypePtr mType;
        String mDefaultValue;
        
        Property(
                 const make_private &v,
                 ContextPtr context
                 ) : Context(v, context) {}
        
      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static PropertyPtr create(ContextPtr context);
        static PropertyPtr create(
                                  ContextPtr context,
                                  const ElementPtr &el
                                  ) throw (InvalidContent);
        
        virtual PropertyPtr toProperty() const override {return ZS_DYNAMIC_PTR_CAST(Property, toContext());}
        
        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual String hash() const override;

        virtual void resolveTypedefs() throw (InvalidContent) override;
      };

      static void createProperties(
                                   ContextPtr context,
                                   ElementPtr propertiesEl,
                                   PropertyList &outProperties
                                   ) throw (InvalidContent);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IWrapperTypes::Method
      #pragma mark

      struct Method : public Context
      {
        MethodModifiers mModifiers {};
        PropertyList mArguments;

        TypeList mThrows;
        
        Method(
               const make_private &v,
               ContextPtr context
               ) : Context(v, context) {}

      protected:
        void init();
        void init(const ElementPtr &rootEl) throw (InvalidContent);
        
      public:
        static MethodPtr create(ContextPtr context);
        static MethodPtr create(
                                ContextPtr context,
                                const ElementPtr &el
                                ) throw (InvalidContent);
        
        virtual MethodPtr toMethod() const override {return ZS_DYNAMIC_PTR_CAST(Method, toContext());}
        
        virtual ElementPtr createElement(const char *objectName = NULL) const override;
        virtual String hash() const override;

        virtual void resolveTypedefs() throw (InvalidContent) override;
      };

      static void createMethods(
                                ContextPtr context,
                                ElementPtr methodsEl,
                                MethodList &outMethods
                                ) throw (InvalidContent);
    };

  } // namespace eventing
} // namespace zsLib
