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

namespace zsLib
{
  namespace eventing
  {

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRemoteEventingTypes
    //

    interaction IRemoteEventingTypes
    {
      typedef zsLib::Log::Level Level;

      enum States
      {
        State_First,

        State_Pending         = State_First,
        State_Listening,
        State_Connecting,
        State_Connected,
        State_ShuttingDown,
        State_Shutdown,
        
        State_Last            = State_Shutdown
      };

      enum Ports : WORD
      {
        Port_Default = 63311
      };
      
      static const char *toString(States state) noexcept;
      States toState(const char *state) noexcept(false); // throws InvalidArgument
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRemoteEventing
    //

    interaction IRemoteEventing : public IRemoteEventingTypes
    {
      static IRemoteEventingPtr connectToRemote(
                                                IRemoteEventingDelegatePtr connectionDelegate,
                                                const IPAddress &serverIP,
                                                const char *connectionSharedSecret
                                                ) noexcept;

      static IRemoteEventingPtr listenForRemote(
                                                IRemoteEventingDelegatePtr connectionDelegate,
                                                WORD localPort,
                                                const char *connectionSharedSecret,
                                                Seconds maxWaitToBindTimeInSeconds = Seconds(60)
                                                ) noexcept;
      virtual PUID getID() const noexcept = 0;

      virtual void shutdown() noexcept = 0;

      virtual States getState() const noexcept = 0;

      virtual void setRemoteLevel(
                                  const char *remoteSubsystemName,
                                  Level level,
                                  bool setOnlyDefaultLevel
                                  ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRemoteEventingDelegate
    //

    interaction IRemoteEventingDelegate
    {
      typedef IRemoteEventingTypes::States States;
      typedef zsLib::Log::Level Level;
      typedef zsLib::Log::KeywordBitmaskType KeywordBitmaskType;
      
      virtual void onRemoteEventingStateChanged(
                                                ZS_MAYBE_USED() IRemoteEventingPtr connection,
                                                ZS_MAYBE_USED() States state
                                                ) { ZS_MAYBE_USED(connection); ZS_MAYBE_USED(state); };
      
      virtual void onRemoteEventingRemoteSubsystem(
                                                   ZS_MAYBE_USED() IRemoteEventingPtr connection,
                                                   ZS_MAYBE_USED() const char *subsystemName
                                                   ) { ZS_MAYBE_USED(connection); ZS_MAYBE_USED(subsystemName); };
      
      virtual void onRemoteEventingRemoteProvider(
                                                  ZS_MAYBE_USED() UUID providerID,
                                                  ZS_MAYBE_USED() const char *providerName,
                                                  ZS_MAYBE_USED() const char *providerUniqueHash
                                                  ) { ZS_MAYBE_USED(providerID); ZS_MAYBE_USED(providerName); ZS_MAYBE_USED(providerUniqueHash); }
      virtual void onRemoteEventingRemoteProviderGone(ZS_MAYBE_USED() const char *providerName) { ZS_MAYBE_USED(providerName); }
      
      virtual void onRemoteEventingRemoteProviderStateChange(
                                                             ZS_MAYBE_USED() const char *providerName,
                                                             ZS_MAYBE_USED() KeywordBitmaskType keywords
                                                             ) { ZS_MAYBE_USED(providerName); ZS_MAYBE_USED(keywords); }

      virtual void onRemoteEventingLocalDroppedEvents(
                                                      ZS_MAYBE_USED() IRemoteEventingPtr connection,
                                                      ZS_MAYBE_USED() size_t totalDropped
                                                      ) { ZS_MAYBE_USED(connection); ZS_MAYBE_USED(totalDropped); }
      virtual void onRemoteEventingRemoteDroppedEvents(
                                                       ZS_MAYBE_USED() IRemoteEventingPtr connection,
                                                       ZS_MAYBE_USED() size_t totalDropped
                                                       ) { ZS_MAYBE_USED(connection); ZS_MAYBE_USED(totalDropped); }
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(zsLib::eventing::IRemoteEventingDelegate)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::eventing::IRemoteEventingPtr, IRemoteEventingPtr)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::eventing::IRemoteEventingTypes::States, States)
ZS_DECLARE_PROXY_TYPEDEF(std::size_t, size_t)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::UUID, UUID)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingStateChanged, IRemoteEventingPtr, States)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingRemoteSubsystem, IRemoteEventingPtr, const char *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingRemoteProvider, UUID, const char *, const char *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingRemoteProviderGone, const char *)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingRemoteProviderStateChange, const char *, KeywordBitmaskType)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingLocalDroppedEvents, IRemoteEventingPtr, size_t)
ZS_DECLARE_PROXY_METHOD(onRemoteEventingRemoteDroppedEvents, IRemoteEventingPtr, size_t)
ZS_DECLARE_PROXY_END()
