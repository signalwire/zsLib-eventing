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

#include <zsLib/eventing/internal/zsLib_eventing_RemoteClient.h>

#include <zsLib/eventing/IHelper.h>

#include <zsLib/IMessageQueueManager.h>
#include <zsLib/Numeric.h>
#include <zsLib/Log.h>

namespace zsLib { namespace eventing { ZS_DECLARE_SUBSYSTEM(zsLib_eventing); } }


namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IHelper, UseEventingHelper);

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteClient
      #pragma mark

      //-----------------------------------------------------------------------
      RemoteClient::RemoteClient(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 IRemoteClientDelegatePtr connectionDelegate,
                                 const IPAddress &serverIP,
                                 const char *connectionSharedSecret
                                 ) :
        MessageQueueAssociator(queue)
      {
        ZS_LOG_DETAIL(log("Created"));
      }

      //-----------------------------------------------------------------------
      RemoteClient::~RemoteClient()
      {
        mThisWeak.reset();
        ZS_LOG_DETAIL(log("Destroyed"));
      }

      //-----------------------------------------------------------------------
      void RemoteClient::init()
      {
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteClient => IRemoteClient
      #pragma mark

      //-----------------------------------------------------------------------
      RemoteClientPtr RemoteClient::connectServer(
                                                  IRemoteClientDelegatePtr connectionDelegate,
                                                  const IPAddress &serverIP,
                                                  const char *connectionSharedSecret
                                                  )
      {
        auto queue = IMessageQueueManager::getMessageQueue("org.zsLib.eventing.RemoteClient");
        auto pThis = make_shared<RemoteClient>(make_private{}, queue, connectionDelegate, serverIP, connectionSharedSecret);
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void RemoteClient::shutdown()
      {
        ZS_LOG_DEBUG(log("shutdown called"));

        AutoRecursiveLock lock(*this);
        cancel();
      }

      //-----------------------------------------------------------------------
      IRemoteClient::States RemoteClient::getState() const
      {
        AutoRecursiveLock lock(mLock);
        return mState;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteClient => IStepDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RemoteClient::onWake()
      {
        ZS_LOG_DEBUG(log("on wake"));

        AutoRecursiveLock lock(*this);
        step();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteClient => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RemoteClient::slog(const char *message)
      {
        return Log::Params(message, "eventing::RemoteClient");
      }

      //-----------------------------------------------------------------------
      Log::Params RemoteClient::log(const char *message)
      {
        ElementPtr objectEl = Element::create("eventing::RemoteClient");
        objectEl->adoptAsLastChild(UseEventingHelper::createElementWithNumber("id", string(mID)));
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void RemoteClient::cancel()
      {
        ZS_LOG_TRACE(log("cancel called"));
      }

      //-----------------------------------------------------------------------
      void RemoteClient::step()
      {
        ZS_LOG_DEBUG(log("step"));
      }

    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteClient
    #pragma mark

    //-------------------------------------------------------------------------
    IRemoteClientPtr IRemoteClient::connectServer(
                                                  IRemoteClientDelegatePtr connectionDelegate,
                                                  const IPAddress &serverIP,
                                                  const char *connectionSharedSecret
                                                  )
    {
      return internal::RemoteClient::connectServer(connectionDelegate, serverIP, connectionSharedSecret);
    }

  } // namespace eventing
} // namespace zsLib
