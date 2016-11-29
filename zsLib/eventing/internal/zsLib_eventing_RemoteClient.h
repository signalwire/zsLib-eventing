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

#include <zsLib/eventing/IRemoteClient.h>

#include <zsLib/IPAddress.h>
#include <zsLib/IWakeDelegate.h>
#include <zsLib/Log.h>

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
      #pragma mark
      #pragma mark RemoteClient
      #pragma mark

      class RemoteClient : public RecursiveLock,
                           public MessageQueueAssociator,
                           public IRemoteClient,
                           public IWakeDelegate
      {
      protected:
        struct make_private {};

      public:
        friend interaction IRemoteClient;

      public:
        RemoteClient(
                     const make_private &,
                     IMessageQueuePtr queue,
                     IRemoteClientDelegatePtr connectionDelegate,
                     const IPAddress &serverIP,
                     const char *connectionSharedSecret
                     );
        ~RemoteClient();

      protected:
        void init();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RemoteClient => IRemoteClient
        #pragma mark

        static RemoteClientPtr connectServer(
                                             IRemoteClientDelegatePtr connectionDelegate,
                                             const IPAddress &serverIP,
                                             const char *connectionSharedSecret
                                             );

        virtual PUID getID() const override { return mID; }

        virtual void shutdown() override;

        virtual States getState() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RemoteClient => IStepDelegate
        #pragma mark

        virtual void onWake() override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RemoteClient (internal)
        #pragma mark

        static Log::Params slog(const char *message);
        Log::Params log(const char *message);

        void cancel();
        void step();

      protected:
        mutable RecursiveLock mLock;
        AutoPUID mID;
        RemoteClientWeakPtr mThisWeak;

        States mState {State_Pending};

        IRemoteClientDelegatePtr mDelegate;
        IPAddress mServerIP;
        String mSharedSecret;
      };
    }
  }
}
