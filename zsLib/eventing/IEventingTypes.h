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

#include <map>

namespace zsLib
{
  namespace eventing
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IEventingTypes
    //

    interaction IEventingTypes
    {
      ZS_DECLARE_STRUCT_PTR(Provider);
      ZS_DECLARE_STRUCT_PTR(Typedef);
      ZS_DECLARE_STRUCT_PTR(Channel);
      ZS_DECLARE_STRUCT_PTR(Task);
      ZS_DECLARE_STRUCT_PTR(Keyword);
      ZS_DECLARE_STRUCT_PTR(OpCode);
      ZS_DECLARE_STRUCT_PTR(DataTemplate);
      ZS_DECLARE_STRUCT_PTR(DataType);
      ZS_DECLARE_STRUCT_PTR(Event);
      ZS_DECLARE_STRUCT_PTR(Subsystem);

      typedef String ChannelID;
      typedef std::map<ChannelID, ChannelPtr> ChannelMap;
      ZS_DECLARE_PTR(ChannelMap);

      typedef String Name;
      typedef std::map<Name, TaskPtr> TaskMap;
      ZS_DECLARE_PTR(TaskMap);

      typedef std::map<Name, KeywordPtr> KeywordMap;
      ZS_DECLARE_PTR(KeywordMap);

      typedef std::map<Name, OpCodePtr> OpCodeMap;
      ZS_DECLARE_PTR(OpCodeMap);

      typedef String TypedefName;
      typedef std::map<TypedefName, TypedefPtr> TypedefMap;
      
      typedef String AliasName;
      typedef String AliasValue;
      typedef std::map<String, String> AliasMap;

      typedef String EventName;
      typedef std::map<EventName, EventPtr> EventMap;

      typedef String SubsystemName;
      typedef std::map<SubsystemName, SubsystemPtr> SubsystemMap;

      typedef String Hash;
      typedef std::map<Hash, DataTemplatePtr> DataTemplateMap;

      typedef std::list<DataTypePtr> DataTypeList;

      typedef String FileName;
      typedef std::list<FileName> FileNameList;

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::OperationalTypes
      //

      enum OperationalTypes
      {
        OperationalType_First,

        OperationalType_Admin = OperationalType_First,
        OperationalType_Operational,
        OperationalType_Analytic,
        OperationalType_Debug,

        OperationalType_Last = OperationalType_Debug,
      };

      static const char *toString(OperationalTypes type) noexcept;
      static OperationalTypes toOperationalType(const char *type) noexcept(false); // throws InvalidArgument

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::PredefinedOpCodes
      //

      enum PredefinedOpCodes
      {
        PredefinedOpCode_First = 0,

        PredefinedOpCode_Info = PredefinedOpCode_First,
        PredefinedOpCode_Start = 1,
        PredefinedOpCode_Stop = 2,
        PredefinedOpCode_DC_Start = 3,
        PredefinedOpCode_DC_Stop = 4,
        PredefinedOpCode_Extension = 5,
        PredefinedOpCode_Reply = 6,
        PredefinedOpCode_Resume = 7,
        PredefinedOpCode_Suspend = 8,
        PredefinedOpCode_Send = 9,
        PredefinedOpCode_Receive = 240,

        PredefinedOpCode_Last = PredefinedOpCode_Receive,
      };

      static const char *toString(PredefinedOpCodes code) noexcept;
      static PredefinedOpCodes toPredefinedOpCode(const char *code) noexcept(false); // throws InvalidArgument

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::PredefinedLevels
      //

      enum PredefinedLevels
      {
        PredefinedLevel_First = 1,

        PredefinedLevel_Critical = PredefinedLevel_First,
        PredefinedLevel_Error = 2,
        PredefinedLevel_Warning = 3,
        PredefinedLevel_Informational = 4,
        PredefinedLevel_Verbose = 5,

        PredefinedLevel_Last = PredefinedLevel_Verbose,
      };

      static const char *toString(PredefinedLevels level) noexcept;
      static PredefinedLevels toPredefinedLevel(const char *level) noexcept(false); // throws InvalidArgument;
      static PredefinedLevels toPredefinedLevel(
                                                Log::Severity severity,
                                                Log::Level level
                                                ) noexcept;

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::BaseTypes
      //

      enum BaseTypes
      {
        BaseType_First,

        BaseType_Boolean = BaseType_First,
        BaseType_Integer,
        BaseType_Float,
        BaseType_Pointer,
        BaseType_Binary,
        BaseType_String,

        BaseType_Last = BaseType_String,
      };

      static const char *toString(BaseTypes type) noexcept;
      static BaseTypes toBaseType(const char *type) noexcept(false); // throws InvalidArgument

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::PredefinedTypedefs
      //

      enum PredefinedTypedefs
      {
        PredefinedTypedef_First,
        
        PredefinedTypedef_void = PredefinedTypedef_First,

        PredefinedTypedef_bool,

        PredefinedTypedef_uchar,
        PredefinedTypedef_char,
        PredefinedTypedef_schar,
        PredefinedTypedef_ushort,
        PredefinedTypedef_short,
        PredefinedTypedef_sshort,
        PredefinedTypedef_uint,
        PredefinedTypedef_int,
        PredefinedTypedef_sint,
        PredefinedTypedef_ulong,
        PredefinedTypedef_long,
        PredefinedTypedef_slong,
        PredefinedTypedef_ulonglong,
        PredefinedTypedef_longlong,
        PredefinedTypedef_slonglong,
        PredefinedTypedef_uint8,
        PredefinedTypedef_int8,
        PredefinedTypedef_sint8,
        PredefinedTypedef_uint16,
        PredefinedTypedef_int16,
        PredefinedTypedef_sint16,
        PredefinedTypedef_uint32,
        PredefinedTypedef_int32,
        PredefinedTypedef_sint32,
        PredefinedTypedef_uint64,
        PredefinedTypedef_int64,
        PredefinedTypedef_sint64,

        PredefinedTypedef_byte,
        PredefinedTypedef_word,
        PredefinedTypedef_dword,
        PredefinedTypedef_qword,

        PredefinedTypedef_float,
        PredefinedTypedef_double,
        PredefinedTypedef_ldouble,
        PredefinedTypedef_float32,
        PredefinedTypedef_float64,

        PredefinedTypedef_pointer,

        PredefinedTypedef_binary,
        PredefinedTypedef_size,

        PredefinedTypedef_string,
        PredefinedTypedef_astring,
        PredefinedTypedef_wstring,

        PredefinedTypedef_Last = PredefinedTypedef_wstring,
      };

      static const char *toString(PredefinedTypedefs type) noexcept;
      static PredefinedTypedefs toPredefinedTypedef(const char *type) noexcept(false); // throws InvalidArgument
      static PredefinedTypedefs toPreferredPredefinedTypedef(PredefinedTypedefs type) noexcept;

      static BaseTypes getBaseType(PredefinedTypedefs type) noexcept;
      static bool isSigned(PredefinedTypedefs type) noexcept;
      static bool isUnsigned(PredefinedTypedefs type) noexcept;
      static bool isAString(PredefinedTypedefs type) noexcept;
      static bool isWString(PredefinedTypedefs type) noexcept;
      static size_t getMinBytes(PredefinedTypedefs type) noexcept;
      static size_t getMaxBytes(PredefinedTypedefs type) noexcept;

      static String aliasLookup(const AliasMap *aliases, const String &value) noexcept;
      static String aliasLookup(const AliasMap &aliases, const String &value) noexcept;

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Provider
      //

      struct Provider
      {
        UUID mID;
        String mName;         // Company-Product-Component
        String mSymbolName;   // e.g. "zsLib"
        String mDescription;
        String mResourceName;
        String mUniqueHash;

        AliasMap mAliases;
        TypedefMap mTypedefs;
        ChannelMap mChannels;
        OpCodeMap mOpCodes;
        TaskMap mTasks;
        KeywordMap mKeywords;
        EventMap mEvents;
        DataTemplateMap mDataTemplates;
        SubsystemMap mSubsystems;

        Provider() noexcept;
        Provider(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent
        ~Provider() noexcept;

        static ProviderPtr create() noexcept                            { return make_shared<Provider>(); }
        static ProviderPtr create(const ElementPtr &el) noexcept(false) { if (!el) return ProviderPtr(); return make_shared<Provider>(el); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;
        void parse(const ElementPtr &rootEl) noexcept(false); // throws InvalidContent

        String hash() const noexcept;
        String uniqueEventingHash() const noexcept;

        String aliasLookup(const String &value) noexcept;
      };

      static void createAliases(
                                ElementPtr aliasesEl,
                                AliasMap &ioAliases
                                ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Typedef
      //

      struct Typedef
      {
        TypedefName        mName;
        PredefinedTypedefs mType {PredefinedTypedef_First};

        Typedef() noexcept {}
        Typedef(
                const ElementPtr &rootEl,
                const AliasMap *aliases = NULL
                ) noexcept(false); // throws InvalidArgument

        static TypedefPtr create() noexcept                             { return make_shared<Typedef>(); }
        static TypedefPtr create(const ElementPtr &el) noexcept(false)  { if (!el) return TypedefPtr(); return make_shared<Typedef>(el); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };

      static void createTypesdefs(
                                  ElementPtr typedefsEl,
                                  TypedefMap &ioTypedefs,
                                  const AliasMap *aliases = NULL
                                  ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Channel
      //

      struct Channel
      {
        ChannelID         mID;
        Name              mName; // Company-Product-Component
        OperationalTypes  mType {OperationalType_First};

        size_t            mValue {};

        Channel() noexcept {}
        Channel(
                const ElementPtr &rootEl,
                const AliasMap *aliases = NULL
                )  noexcept(false); // throws InvalidContent

        static ChannelPtr create() noexcept { return make_shared<Channel>(); }
        static ChannelPtr create(
                                 const ElementPtr &el,
                                 const AliasMap *aliases = NULL
                                 ) noexcept(false) { if (!el) return ChannelPtr(); return make_shared<Channel>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };

      static void createChannels(
                                 ElementPtr channelsEl,
                                 ChannelMap &outChannels,
                                 const AliasMap *aliases = NULL
                                 ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Task
      //

      struct Task
      {
        Name      mName;
        OpCodeMap mOpCodes;

        size_t    mValue {};

        Task() noexcept;
        Task(
             const ElementPtr &rootEl,
             const AliasMap *aliases = NULL
             ) noexcept(false); // throws InvalidContent
        ~Task() noexcept;

        static TaskPtr create() noexcept { return make_shared<Task>(); }
        static TaskPtr create(
                              const ElementPtr &el,
                              const AliasMap *aliases = NULL
                              ) noexcept(false) { if (!el) return TaskPtr(); return make_shared<Task>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };

      static void createTasks(
                              ElementPtr tasksEl,
                              TaskMap &outTasks,
                              const AliasMap *aliases = NULL
                              ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Keyword
      //
      
      struct Keyword
      {
        Name      mName;

        uint64_t  mMask {};

        Keyword() noexcept {}
        Keyword(
                const ElementPtr &rootEl,
                const AliasMap *aliases = NULL
                ) noexcept(false); // throws InvalidContent

        static KeywordPtr create() noexcept { return make_shared<Keyword>(); }
        static KeywordPtr create(
                                 const ElementPtr &el,
                                 const AliasMap *aliases = NULL
                                 ) noexcept(false) { if (!el) return KeywordPtr(); return make_shared<Keyword>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };

      static void createKeywords(
                                 ElementPtr tasksEl,
                                 KeywordMap &outKeywords,
                                 const AliasMap *aliases = NULL
                                 ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::OpCode
      //

      struct OpCode
      {
        Name        mName;
        TaskWeakPtr mTask;

        size_t      mValue {};

        OpCode() noexcept;
        OpCode(
               const ElementPtr &rootEl,
               const AliasMap *aliases = NULL
               ) noexcept(false); // throws InvalidContent
        ~OpCode() noexcept;

        static OpCodePtr create() noexcept { return make_shared<OpCode>(); }
        static OpCodePtr create(
                                const ElementPtr &el,
                                const AliasMap *aliases = NULL
                                ) noexcept(false) { if (!el) return OpCodePtr(); return make_shared<OpCode>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash(bool calledFromTask = false) const noexcept;
      };

      static void createOpCodes(
                                ElementPtr opCodesEl,
                                OpCodeMap &outOpCodes,
                                const AliasMap *aliases = NULL
                                ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::Event
      //

      struct Event
      {
        Name            mName;

        String          mSubsystem;
        Log::Severity   mSeverity {Log::Informational};
        Log::Level      mLevel {Log::None};

        ChannelPtr      mChannel;
        TaskPtr         mTask;
        OpCodePtr       mOpCode;
        DataTemplatePtr mDataTemplate;
        KeywordMap      mKeywords;

        size_t          mValue {};

        String hash() const noexcept;

        Event() noexcept;
        Event(
              const ElementPtr &rootEl,
              const AliasMap *aliases = NULL
              ) noexcept(false); // throws InvalidContent
        ~Event() noexcept;

        static EventPtr create() noexcept { return make_shared<Event>(); }
        static EventPtr create(
                               const ElementPtr &el,
                               const AliasMap *aliases = NULL
                               ) noexcept(false) { if (!el) return EventPtr(); return make_shared<Event>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;
      };

      static void createEvents(
                               ElementPtr eventsEl,
                               EventMap &outEvents,
                               const ChannelMap &channels,
                               OpCodeMap &opCodes,
                               const TaskMap &tasks,
                               const KeywordMap &keywords,
                               const DataTemplateMap &dataTemplates,
                               const AliasMap *aliases = NULL
                               ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::DataTemplate
      //

      struct DataTemplate
      {
        String mID;
        DataTypeList mDataTypes;

        DataTemplate() noexcept;
        DataTemplate(
                     const ElementPtr &rootEl,
                     const AliasMap *aliases = NULL
                     ) noexcept(false); // throw (InvalidContent);
        ~DataTemplate() noexcept;

        static DataTemplatePtr create() noexcept { return make_shared<DataTemplate>(); }
        static DataTemplatePtr create(
                                      const ElementPtr &el,
                                      const AliasMap *aliases = NULL
                                      ) noexcept(false) { if (!el) return DataTemplatePtr(); return make_shared<DataTemplate>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String uniqueID() const noexcept;
        String hash() const noexcept;
      };

      static void createDataTemplates(
                                      ElementPtr templatesEl,
                                      DataTemplateMap &outDataTemplates,
                                      const TypedefMap &typedefs,
                                      const AliasMap *aliases = NULL
                                      ) noexcept(false); // throws InvalidContent

      //-----------------------------------------------------------------------
      //
      // IEventingTypes::DataType
      //

      struct DataType
      {
        PredefinedTypedefs mType {PredefinedTypedef_First};
        String             mValueName;

        DataType() noexcept;
        DataType(
                 const ElementPtr &rootEl,
                 const TypedefMap *typedefs = NULL,
                 const AliasMap *aliases = NULL
                 ) noexcept(false); // throws InvalidContent
        ~DataType() noexcept;

        static DataTypePtr create() noexcept { return make_shared<DataType>(); }
        static DataTypePtr create(
                                const ElementPtr &el,
                                const TypedefMap *typedefs = NULL,
                                const AliasMap *aliases = NULL
                                ) noexcept(false) { if (!el) return DataTypePtr(); return make_shared<DataType>(el, typedefs, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };

      static void createDataTypes(
                                  ElementPtr dataTypesEl,
                                  DataTypeList &outDataTypes,
                                  const TypedefMap &typedefs,
                                  const AliasMap *aliases = NULL
                                  ) noexcept(false); // throws InvalidContent
      
      //-----------------------------------------------------------------------
      //
      // IEventingTypes::DataType
      //
      
      struct Subsystem
      {
        SubsystemName mName;
        Log::Level mLevel {Log::Level::Level_First};

        Subsystem() noexcept;
        Subsystem(
                  const ElementPtr &rootEl,
                  const AliasMap *aliases = NULL
                  ) noexcept(false); // throws InvalidContent
        ~Subsystem() noexcept;

        static SubsystemPtr create() noexcept { return make_shared<Subsystem>(); }
        static SubsystemPtr create(
                                   const ElementPtr &el,
                                   const AliasMap *aliases = NULL
                                   ) noexcept(false) { if (!el) return SubsystemPtr(); return make_shared<Subsystem>(el, aliases); }

        ElementPtr createElement(const char *objectName = NULL) const noexcept;

        String hash() const noexcept;
      };
      
      static void createSubsystems(
                                   ElementPtr dataTypesEl,
                                   SubsystemMap &outSubsystems,
                                   const AliasMap *aliases = NULL
                                   ) noexcept(false); // throws InvalidContent
    };

  } // namespace eventing
} // namespace zsLib
