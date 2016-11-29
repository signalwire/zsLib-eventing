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
    #pragma mark
    #pragma mark IRemoteServerTypes
    #pragma mark

    interaction IRemoteServerTypes
    {
      enum States
      {
        State_Pending,
        State_Listening,
        State_Connected,
        State_ShuttingDown,
        State_Shutdown
      };
      static const char *toString(States state);
      static States toState(const char *state);
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteServer
    #pragma mark

    interaction IRemoteServer : public IRemoteServerTypes
    {
      static IRemoteServerPtr createServer(
                                           IRemoteServerDelegatePtr serverDelegate,
                                           WORD listenPort,
                                           const char *connectionSharedSecret,
                                           Seconds maxWaitToBindTimeInSeconds
                                           );

      virtual PUID getID() const = 0;

      virtual void shutdown() = 0;

      virtual States getState() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteServerDelegate
    #pragma mark

    interaction IRemoteServerDelegate
    {
      typedef IRemoteServerTypes::States States;

      virtual void onRemoteServerStateChanged(
                                              IRemoteServerPtr connection,
                                              States state
                                              ) {};
    };

  }
}

ZS_DECLARE_PROXY_BEGIN(zsLib::eventing::IRemoteServerDelegate)
ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IRemoteServerPtr, IRemoteServerPtr)
ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IRemoteServerTypes::States, States)
ZS_DECLARE_PROXY_METHOD_2(onRemoteServerStateChanged, IRemoteServerPtr, States)
ZS_DECLARE_PROXY_END()
