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

#include <zsLib/eventing/internal/types.h>

#include <zsLib/eventing/IRemoteEventing.h>

#include <zsLib/IPAddress.h>
#include <zsLib/ITimer.h>
#include <zsLib/IWakeDelegate.h>
#include <zsLib/Log.h>
#include <zsLib/Socket.h>

#include <cryptopp/queue.h>

#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_MAX_DATA_SIZE                                    "zsLib/eventing/remote-eventing/max-data-size-in-bytes"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_MAX_PACKED_SIZE                                  "zsLib/eventing/remote-eventing/max-packed-data-size-in-bytes"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_MAX_OUTSTANDING_EVENTS                           "zsLib/eventing/remote-eventing/max-outstanding-events-in-bytes"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_MAX_QUEUED_ASYNC_DATA_BEFORED_EVENTS_DROPPED     "zsLib/eventing/remote-eventing/max-queued-async-data-before-events-dropped"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_MAX_QUEUED_OUTGOING_DATA_BEFORED_EVENTS_DROPPED  "zsLib/eventing/remote-eventing/max-queued-outgoing-data-before-events-dropped"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_NOTIFY_TIMER                                     "zsLib/eventing/remote-eventing/notify-timer-in-seconds"
#define ZSLIB_EVENTING_SETTING_REMOTE_EVENTING_USE_IPV6                                         "zsLib/eventing/remote-eventing/use-ipv6"

namespace zsLib
{
  namespace eventing
  {
    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IRemoteEventingInternalTypes
      //
      
      interaction IRemoteEventingInternalTypes
      {
        typedef zsLib::Log::ProviderHandle ProviderHandle;
        typedef zsLib::Log::KeywordBitmaskType KeywordBitmaskType;
        
        struct ProviderInfo
        {
          ProviderHandle mHandle {};
          ProviderHandle mRemoteHandle {};
          bool mSelfRegistered {};
          PUID mRelatedToRemoteEventingObjectID {};
          UUID mProviderID {};
          String mProviderName;
          String mProviderHash;
          String mProviderJMAN;
          KeywordBitmaskType mBitmask {};

          ProviderInfo() noexcept;
          ~ProviderInfo() noexcept;
        };
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IRemoteEventingAsyncDelegate
      //
      
      interaction IRemoteEventingAsyncDelegate : public IRemoteEventingInternalTypes
      {
        ZS_DECLARE_TYPEDEF_PTR(CryptoPP::ByteQueue, ByteQueue);
        typedef zsLib::Log::KeywordBitmaskType KeywordBitmaskType;
        
        virtual void onRemoteEventingSubscribeLogger() = 0;
        virtual void onRemoteEventingUnsubscribeLogger() = 0;
        
        virtual void onRemoteEventingNewSubsystem(const char *subsystemName) = 0;

        virtual void onRemoteEventingProviderRegistered(ProviderInfo *info) = 0;
        virtual void onRemoteEventingProviderUnregistered(ProviderInfo *info) = 0;
        virtual void onRemoteEventingProviderLoggingStateChanged(
                                                                 ProviderInfo *info,
                                                                 KeywordBitmaskType keyword
                                                                 ) = 0;

        virtual void onRemoteEventingWriteEvent(
                                                ByteQueuePtr message,
                                                size_t currentSize
                                                ) = 0;
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // RemoteEventing
      //

      class RemoteEventing : public MessageQueueAssociator,
                             public IRemoteEventing,
                             public IWakeDelegate,
                             public ITimerDelegate,
                             public ISocketDelegate,
                             public ILogEventingProviderDelegate,
                             public ILogEventingDelegate,
                             public IRemoteEventingAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        friend interaction IRemoteEventing;
        ZS_DECLARE_TYPEDEF_PTR(CryptoPP::ByteQueue, ByteQueue);
        ZS_DECLARE_STRUCT_PTR(SubsystemInfo);
        typedef zsLib::Log::Level Level;

        typedef zsLib::Log::ProviderHandle ProviderHandle;
        typedef zsLib::Log::EventingAtomDataArray EventingAtomDataArray;
        typedef zsLib::Log::EventingAtomIndex EventingAtomIndex;
        typedef zsLib::Log::KeywordBitmaskType KeywordBitmaskType;

        enum MessageTypes
        {
          MessageType_First           = 1,
          
          MessageType_Hello           = MessageType_First,
          MessageType_Challenge       = 2,
          MessageType_ChallengeReply  = 3,
          MessageType_Welcome         = 4,

          MessageType_Goodbye         = 5,
          
          MessageType_Notify          = 8,
          MessageType_Request         = 16,
          MessageType_RequestAck      = 17,
          
          MessageType_TraceEvent      = 32,
          
          MessageType_Last            = MessageType_TraceEvent
        };
        
        static const char *toString(MessageTypes messageType) noexcept;
        MessageTypes toMessageType(const char *messageType) noexcept(false); // throws InvalidArgument
        
        struct SubsystemInfo
        {
          String mName;
          Log::Level mLevel {Log::Level_First};
        };
        
        typedef std::set<ProviderInfo *> ProviderInfoSet;
        typedef std::map<UUID, ProviderInfo *> ProviderInfoUUIDMap;
        typedef std::map<ProviderHandle, ProviderInfo *> ProviderInfoHandleMap;
        typedef std::map<String, SubsystemInfoPtr> SubsystemMap;
        typedef std::map<String, KeywordBitmaskType> KeywordLogLevelMap;
        
      public:
        RemoteEventing(
                       const make_private &,
                       IMessageQueuePtr queue,
                       IRemoteEventingDelegatePtr connectionDelegate,
                       const char *connectionSharedSecret,
                       const IPAddress &serverIP,
                       WORD listenPort,
                       Seconds maxWaitToBindTime
                       ) noexcept;
        ~RemoteEventing() noexcept;

      protected:
        void init() noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // RemoteEventing => IRemoteEventing
        //

        static RemoteEventingPtr connectToRemote(
                                                 IRemoteEventingDelegatePtr connectionDelegate,
                                                 const IPAddress &serverIP,
                                                 const char *connectionSharedSecret
                                                 ) noexcept;
        
        static RemoteEventingPtr listenForRemote(
                                                 IRemoteEventingDelegatePtr connectionDelegate,
                                                 WORD localPort,
                                                 const char *connectionSharedSecret,
                                                 Seconds maxWaitToBindTimeInSeconds
                                                 ) noexcept;

        PUID getID() const noexcept override;

        void shutdown() noexcept override;

        States getState() const noexcept override;

        void setRemoteLevel(
                            const char *remoteSubsystemName,
                            Level level,
                            bool setOnlyDefaultLevel
                            ) noexcept override;

        //---------------------------------------------------------------------
        //
        // RemoteEventing => IWakeDelegate
        //

        void onWake() override;

        //---------------------------------------------------------------------
        //
        // RemoteEventing => ITimerDelegate
        //

        void onTimer(ITimerPtr timer) override;
        
        //---------------------------------------------------------------------
        //
        // RemoteEventing => ISocketDelegate
        //

        void onReadReady(SocketPtr socket) override;
        void onWriteReady(SocketPtr socket) override;
        void onException(SocketPtr socket) override;
        
        //---------------------------------------------------------------------
        //
        // RemoteEventing => ILogEventingProviderDelegate
        //

        void notifyNewSubsystem(zsLib::Subsystem &inSubsystem) noexcept override;
        
        void notifyEventingProviderRegistered(
                                              ProviderHandle handle,
                                              EventingAtomDataArray eventingAtomDataArray
                                              ) noexcept override;
        void notifyEventingProviderUnregistered(
                                                ProviderHandle handle,
                                                EventingAtomDataArray eventingAtomDataArray
                                                ) noexcept override;

        void notifyEventingProviderLoggingStateChanged(
                                                       ProviderHandle handle,
                                                       EventingAtomDataArray eventingAtomDataArray,
                                                       KeywordBitmaskType keywords
                                                       ) noexcept override;
        
        //---------------------------------------------------------------------
        //
        // RemoteEventing => ILogEventingDelegate
        //
        
        // (duplicate) virtual void notifyNewSubsystem(zsLib::Subsystem &inSubsystem) override;
        
        void notifyWriteEvent(
                              ProviderHandle handle,
                              EventingAtomDataArray eventingAtomDataArray,
                              Severity severity,
                              Level level,
                              EVENT_DESCRIPTOR_HANDLE descriptor,
                              EVENT_PARAMETER_DESCRIPTOR_HANDLE parameterDescriptor,
                              EVENT_DATA_DESCRIPTOR_HANDLE dataDescriptor,
                              size_t dataDescriptorCount
                              ) noexcept override;
        
        //---------------------------------------------------------------------
        //
        // RemoteEventing => IRemoteEventingAsyncDelegate
        //

        void onRemoteEventingSubscribeLogger() override;
        void onRemoteEventingUnsubscribeLogger() override;
        void onRemoteEventingNewSubsystem(const char *subsystemName) override;

        void onRemoteEventingProviderRegistered(ProviderInfo *info) override;
        void onRemoteEventingProviderUnregistered(ProviderInfo *info) override;
        void onRemoteEventingProviderLoggingStateChanged(
                                                         ProviderInfo *info,
                                                         KeywordBitmaskType keyword
                                                         ) override;

        void onRemoteEventingWriteEvent(
                                        ByteQueuePtr message,
                                        size_t currentSize
                                        ) override;
        
      protected:
        //---------------------------------------------------------------------
        //
        // RemoteEventing (internal)
        //

        static Log::Params slog(const char *message) noexcept;
        Log::Params log(const char *message) noexcept;
        
        bool isShuttingDown() const noexcept        { return State_ShuttingDown == mState; }
        bool isShutdown() const noexcept            { return State_Shutdown == mState; }
        bool isListeningMode() const noexcept       { return 0 != mListenPort; }
        bool isConnectingMode() const noexcept      { return 0 == mListenPort; }
        SocketPtr getActiveSocket() const noexcept  { if (isListeningMode()) return mAcceptedSocket; return mConnectSocket; }
        bool isAuthorized() const noexcept          { return MessageType_Welcome == mHandshakeState; }

        void disconnect() noexcept;
        void cancel() noexcept;
        void step() noexcept;
        
        bool stepSocketBind() noexcept;
        bool stepWaitForAccept() noexcept;
        
        bool stepSocketConnect() noexcept;
        bool stepWaitConnected() noexcept;
        bool stepHello() noexcept;
        
        bool stepNotifyTimer() noexcept;
        bool stepAuthorized() noexcept;
        
        void setState(States state) noexcept;
        void resetConnection() noexcept;
        void prepareNewConnection() noexcept;
        void readIncomingMessage() noexcept;
        void sendOutgoingData() noexcept;

        void sendData(
                      MessageTypes messageType,
                      const SecureByteBlock &buffer
                      ) noexcept;
        void sendData(
                      MessageTypes messageType,
                      const ElementPtr &rootEl
                      ) noexcept;
        void sendData(
                      MessageTypes messageType,
                      const std::string &message
                      ) noexcept;
        void sendAck(
                     const String &requestID,
                     int errorNumber = 0,
                     const char *reason = NULL
                     ) noexcept;

        void handleHandshakeMessage(
                                    MessageTypes messageType,
                                    SecureByteBlock &buffer
                                    ) noexcept;
        void handleAuthorizedMessage(
                                     MessageTypes messageType,
                                     SecureByteBlock &buffer
                                     ) noexcept;
        
        void handleHello(const ElementPtr &rootEl) noexcept;
        void handleChallenge(const ElementPtr &rootEl) noexcept;
        void handleChallengeReply(const ElementPtr &rootEl) noexcept;
        void handleWelcome(const ElementPtr &rootEl) noexcept;
        
        void handleNotify(const ElementPtr &rootEl) noexcept;
        void handleNotifyGeneralInfo(const ElementPtr &rootEl) noexcept;
        void handleNotifyRemoteSubsystem(const ElementPtr &rootEl) noexcept;
        void handleNotifyRemoteProvider(const ElementPtr &rootEl) noexcept;
        void handleNotifyRemoteProviderKeywordLogging(const ElementPtr &rootEl) noexcept;
        void handleRequest(const ElementPtr &rootEl) noexcept;
        void handleRequestAck(const ElementPtr &rootEl) noexcept;
        
        void handleEvent(SecureByteBlock &buffer) noexcept;
        
        void sendWelcome() noexcept;
        void sendNotify() noexcept;
        void requestSetRemoteSubsystemLevel(
                                            SubsystemInfoPtr info,
                                            bool setOnlyDefaultLevel
                                            ) noexcept;
        void requestSetRemoteEventProviderLogging(
                                                  const String &providerName,
                                                  KeywordBitmaskType bitmask
                                                  ) noexcept;
        void announceProviderToRemote(
                                      ProviderInfo *info,
                                      bool announceNew = true
                                      ) noexcept;
        void announceProviderLoggingStateChangedToRemote(
                                                         ProviderInfo *info,
                                                         KeywordBitmaskType bitmask
                                                         ) noexcept;
        
        void announceSubsystemToRemote(SubsystemInfoPtr info) noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // RemoteEventing (data)
        //

        mutable RecursiveLock mLock;
        AutoPUID mID;
        RemoteEventingWeakPtr mThisWeak;
        RemoteEventingPtr mGracefulShutdownReference;

        IRemoteEventingDelegatePtr mDelegate;
        
        size_t mMaxDataSize {};
        size_t mMaxPackedSize {};
        size_t mMaxOutstandingEvents {};
        size_t mMaxQueuedAsyncDataBeforeEventsDropped {};
        size_t mMaxQueuedOutgoingDataBeforeEventsDropped {};
        bool mUseIPv6 {};
        
        EventingAtomIndex mEventingAtomIndex {};

        States mState {State_Pending};

        IPAddress mServerIP;
        WORD mListenPort {};
        String mSharedSecret;
        Seconds mMaxWaitToBindTime {};
        Time mBindFailureTime {};
        
        ITimerPtr mRebindTimer;
        SocketPtr mBindSocket;
        SocketPtr mAcceptedSocket;
        IPAddress mRemoteIP;

        SocketPtr mConnectSocket;
        bool mConnected {false};
        
        ITimerPtr mNotifyTimer;
        size_t mAnnouncedLocalDropped {};
        size_t mAnnouncedRemoteDropped {};

        ByteQueue mIncomingQueue;
        ByteQueue mOutgoingQueue;
        bool mWriteReady {false};
        
        MessageTypes mHandshakeState {MessageType_First};
        String mHelloSalt;
        String mExpectingHelloProofInChallenge;
        
        String mChallengeSalt;
        String mExpectingChallengeProofInReply;
        
        bool mFlipEndianInt {false};
        bool mFlipEndianFloat {false};
        
        SubsystemMap mLocalSubsystems;
        SubsystemMap mRemoteSubsystems;
        SubsystemMap mSetRemoteSubsystemsLevels;
        SubsystemMap mSetDefaultRemoteSubsystemsLevels;

        ProviderInfoUUIDMap mLocalAnnouncedProviders;
        ProviderInfoUUIDMap mRemoteRegisteredProvidersByUUID;
        ProviderInfoHandleMap mRemoteRegisteredProvidersByRemoteHandle;
        KeywordLogLevelMap mRequestRemoteProviderKeywordLevel;
        ProviderInfoHandleMap mRequestedRemoteProviderKeywordLevel;

        ProviderInfoSet mCleanUpProviderInfos;

        mutable RecursiveLock mAsyncSelfLock;
        IRemoteEventingAsyncDelegatePtr mAsyncSelf;

        std::atomic<size_t> mTotalDroppedEvents {};
        std::atomic<size_t> mOutstandingEvents {};
        std::atomic<size_t> mEventDataInAsyncQueue {};
        std::atomic<size_t> mEventDataInOutgoingQueue {};
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(zsLib::eventing::internal::IRemoteEventingAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::eventing::internal::IRemoteEventingInternalTypes::ProviderInfo, ProviderInfo)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::eventing::internal::IRemoteEventingAsyncDelegate::KeywordBitmaskType, KeywordBitmaskType)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::eventing::internal::IRemoteEventingAsyncDelegate::ByteQueuePtr, ByteQueuePtr)
ZS_DECLARE_PROXY_TYPEDEF(std::size_t, size_t)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingSubscribeLogger)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingUnsubscribeLogger)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingNewSubsystem, const char *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingProviderRegistered, ProviderInfo *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingProviderUnregistered, ProviderInfo *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingProviderLoggingStateChanged, ProviderInfo *, KeywordBitmaskType)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingWriteEvent, ByteQueuePtr, size_t)
ZS_DECLARE_PROXY_END()
