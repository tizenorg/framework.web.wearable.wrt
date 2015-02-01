/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        evas_object.h
 * @author      Lukasz Wrzosek (l.wrzosel@samsung.com)
 * @version     1.0
 * @brief       This file is the header for Evas_Object wrapper from Efl.
 */
#ifndef WRT_SRC_DOMAIN_EFL_EVAS_OBJECT_H
#define WRT_SRC_DOMAIN_EFL_EVAS_OBJECT_H

#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <memory>

#include <dpl/framework_efl.h>
#include <dpl/assert.h>
#include <dpl/foreach.h>
#include <dpl/apply.h>
#include <dpl/noncopyable.h>

class EvasObject
{
    class EvasObjectShared;
    typedef std::shared_ptr<EvasObjectShared> EvasObjectSharedPtr;

  public:
    class IConnection
    {
      public:
        Evas_Object* GetEvasObject();
        void Disconnect();

      private:
        IConnection(EvasObjectShared* object);
        virtual ~IConnection()
        {}
        virtual void Call(void* /*event_info*/) = 0;

        static void SmartCallbackWrapper(void* data,
                                         Evas_Object* /*object*/,
                                         void* event_info);
        static void EvasCallbackWrapper(void* data,
                                        Evas* /*evas*/,
                                        Evas_Object* /*object*/,
                                        void* event_info);

        virtual void ConnectPrv() = 0;
        virtual void DisconnectPrv() = 0;

        friend class EvasObjectShared;

        EvasObjectShared* m_object;
    };

  private:
    class EvasObjectShared : DPL::Noncopyable
    {
      public:
        friend class IConnection;
        Evas_Object* GetObject();

        typedef std::set<IConnection*> IConnectionsSet;

        class SmartConnectionBase : public IConnection
        {
          public:
            SmartConnectionBase(const std::string& name,
                                EvasObjectShared* object);

            virtual void ConnectPrv();
            virtual void DisconnectPrv();
            std::string m_callbackName;
        };

        template<typename ... Args>
        class SmartConnection : public SmartConnectionBase
        {
          public:
            typedef void (*CbType)(IConnection* connection,
                                   void* event_info,
                                   Args ... args);

            SmartConnection(const std::string& name,
                            CbType callback,
                            EvasObjectShared* object,
                            Args ... args) :
                SmartConnectionBase(name, object),
                m_callback(callback),
                m_args(args ...)
            {}

            virtual ~SmartConnection()
            {}

            virtual void Call(void* event_info)
            {
                DPL::Apply<void,
                           DPL::ExtraArgsInsertPolicy::Prepend>(m_callback,
                                                                m_args,
                                                                this,
                                                                event_info);
            }

          private:
            CbType m_callback;
            std::tuple<Args ...> m_args;
        };

        template <class ThisType, class ArgType1>
        class SmartMemberConnection1 : public SmartConnectionBase
        {
          public:
            typedef void (ThisType::*CbType)(IConnection* connection,
                                             void* event_info, ArgType1 *arg1);

            SmartMemberConnection1(const std::string& name,
                                   CbType callback,
                                   ThisType* callee,
                                   ArgType1* arg1,
                                   EvasObjectShared* object) :
                SmartConnectionBase(name, object),
                m_callback(callback),
                m_callee(callee),
                m_arg1(arg1)
            {}

            virtual ~SmartMemberConnection1()
            {}

            virtual void Call(void* event_info)
            {
                (m_callee->*m_callback)(this, event_info, m_arg1);
            }

          private:
            CbType m_callback;
            ThisType* m_callee;
            ArgType1* m_arg1;
        };

        template <class ThisType, class ArgType1, class ArgType2>
        class SmartMemberConnection2 : public SmartConnectionBase
        {
          public:
            typedef void (ThisType::*CbType)(IConnection* connection,
                                             void* event_info, ArgType1 *arg1,
                                             ArgType2* arg2);

            SmartMemberConnection2(const std::string& name,
                                   CbType callback,
                                   ThisType* callee,
                                   ArgType1* arg1,
                                   ArgType2* arg2,
                                   EvasObjectShared* object) :
                SmartConnectionBase(name, object),
                m_callback(callback),
                m_callee(callee),
                m_arg1(arg1),
                m_arg2(arg2)
            {}

            virtual ~SmartMemberConnection2()
            {}

            virtual void Call(void* event_info)
            {
                (m_callee->*m_callback)(this, event_info, m_arg1, m_arg2);
            }

          private:
            CbType m_callback;
            ThisType* m_callee;
            ArgType1* m_arg1;
            ArgType2* m_arg2;
        };

        class EvasConnectionBase : public IConnection
        {
          public:
            EvasConnectionBase(Evas_Callback_Type type,
                               EvasObjectShared* object);

            virtual void ConnectPrv();
            virtual void DisconnectPrv();

            Evas_Callback_Type m_callbackType;
        };

        template <class ArgType1>
        class EvasConnection1 : public EvasConnectionBase
        {
          public:
            typedef void (*CbType)(IConnection* connection, void* event_info,
                                   ArgType1 *arg1);

            EvasConnection1(Evas_Callback_Type type,
                            CbType callback,
                            ArgType1* arg1,
                            EvasObjectShared* object) :
                EvasConnectionBase(type, object),
                m_callback(callback),
                m_arg1(arg1)
            {}

            virtual ~EvasConnection1()
            {}

            virtual void Call(void* event_info)
            {
                m_callback(this, event_info, m_arg1);
            }

          private:
            CbType m_callback;
            ArgType1* m_arg1;
        };

        template <class ArgType1, class ArgType2>
        class EvasConnection2 : public EvasConnectionBase
        {
          public:
            typedef void (*CbType)(IConnection* connection, void* event_info,
                                   ArgType1 *arg1, ArgType2 *arg2);

            EvasConnection2(Evas_Callback_Type type,
                            CbType callback,
                            ArgType1* arg1,
                            ArgType2* arg2,
                            EvasObjectShared* object) :
                EvasConnectionBase(type, object),
                m_callback(callback),
                m_arg1(arg1),
                m_arg2(arg2)
            {}

            virtual ~EvasConnection2()
            {}

            virtual void Call(void* event_info)
            {
                m_callback(this, event_info, m_arg1, m_arg2);
            }

          private:
            CbType m_callback;
            ArgType1* m_arg1;
            ArgType2* m_arg2;
        };

        template <class ThisType, class ArgType1>
        class EvasMemberConnection1 : public EvasConnectionBase
        {
          public:
            typedef void (ThisType::*CbType)(IConnection* connection,
                                             void* event_info, ArgType1 *arg1);

            EvasMemberConnection1(Evas_Callback_Type type,
                                  CbType callback,
                                  ThisType* callee,
                                  ArgType1* arg1,
                                  EvasObjectShared* object) :
                EvasConnectionBase(type, object),
                m_callback(callback),
                m_callee(callee),
                m_arg1(arg1)
            {}

            virtual ~EvasMemberConnection1()
            {}

            virtual void Call(void* event_info)
            {
                (m_callee->*m_callback)(this, event_info, m_arg1);
            }

          private:
            CbType m_callback;
            ThisType* m_callee;
            ArgType1* m_arg1;
        };

        template <class ThisType, class ArgType1, class ArgType2>
        class EvasMemberConnection2 : public EvasConnectionBase
        {
          public:
            typedef void (ThisType::*CbType)(IConnection* connection,
                                             void* event_info, ArgType1* arg1,
                                             ArgType2* arg2);

            EvasMemberConnection2(Evas_Callback_Type type,
                                  CbType callback,
                                  ThisType* callee,
                                  ArgType1* arg1,
                                  ArgType2* arg2,
                                  EvasObjectShared* object) :
                EvasConnectionBase(type, object),
                m_callback(callback),
                m_callee(callee),
                m_arg1(arg1),
                m_arg2(arg2)
            {}

            virtual ~EvasMemberConnection2()
            {}

            virtual void Call(void* event_info)
            {
                (m_callee->*m_callback)(this, event_info, m_arg1, m_arg2);
            }

          private:
            CbType m_callback;
            ThisType* m_callee;
            ArgType1* m_arg1;
            ArgType2* m_arg2;
        };

        EvasObjectShared();
        explicit EvasObjectShared(Evas_Object* object);
        void SetObject(Evas_Object* object);
        ~EvasObjectShared();

        template<typename ... Args>
        IConnection* ConnectSmartCallback(
            const char* callbackName,
            typename SmartConnection<Args ...>::
                CbType callback,
            Args ... args)
        {
            Assert(m_object);
            Assert(callbackName);
            Assert(callback);
            IConnection* connection = new SmartConnection<Args ...>(
                    callbackName,
                    callback,
                    this,
                    args ...);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ThisType, class ArgType1, class ArgType2>
        IConnection* ConnectMemberSmartCallback(
            const char* callbackName,
            typename SmartMemberConnection2<ThisType, ArgType1,
                                            ArgType2>::CbType callback,
            ThisType* callee,
            ArgType1* arg1,
            ArgType2* arg2)
        {
            Assert(m_object);
            Assert(callee);
            Assert(callbackName);
            Assert(callback);
            IConnection* connection =
                new SmartMemberConnection2<ThisType, ArgType1, ArgType2>(
                    callbackName,
                    callback,
                    callee,
                    arg1,
                    arg2,
                    this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ThisType, class ArgType1>
        IConnection* ConnectMemberSmartCallback(
            const char* callbackName,
            typename SmartMemberConnection1<ThisType,
                                            ArgType1>::CbType callback,
            ThisType* callee,
            ArgType1* arg1)
        {
            Assert(m_object);
            Assert(callee);
            Assert(callbackName);
            Assert(callback);
            IConnection* connection =
                new SmartMemberConnection1<ThisType, ArgType1>(callbackName,
                                                               callback,
                                                               callee,
                                                               arg1,
                                                               this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ArgType1, class ArgType2>
        IConnection* ConnectEvasCallback(Evas_Callback_Type callbackType,
                                         typename EvasConnection2<ArgType1,
                                                                  ArgType2>::
                                             CbType callback,
                                         ArgType1* arg1,
                                         ArgType2* arg2)
        {
            Assert(m_object);
            Assert(callbackType);
            Assert(callback);
            IConnection* connection = new EvasConnection2<ArgType1, ArgType2>(
                    callbackType,
                    callback,
                    arg1,
                    arg2,
                    this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ArgType1>
        IConnection* ConnectEvasCallback(
            Evas_Callback_Type callbackType,
            typename EvasConnection1<ArgType1>::
                CbType callback,
            ArgType1* arg1)
        {
            Assert(m_object);
            Assert(callbackType);
            Assert(callback);
            IConnection* connection = new EvasConnection1<ArgType1>(
                    callbackType,
                    callback,
                    arg1,
                    this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ThisType, class ArgType1, class ArgType2>
        IConnection* ConnectMemberEvasCallback(
            Evas_Callback_Type callbackType,
            typename EvasMemberConnection2<ThisType, ArgType1,
                                           ArgType2>::CbType callback,
            ThisType* callee,
            ArgType1* arg1,
            ArgType2* arg2)
        {
            Assert(m_object);
            Assert(callee);
            Assert(callbackType);
            Assert(callback);
            IConnection* connection =
                new EvasMemberConnection2<ThisType, ArgType1, ArgType2>(
                    callbackType,
                    callback,
                    callee,
                    arg1,
                    arg2,
                    this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        template <class ThisType, class ArgType1>
        IConnection* ConnectMemberEvasCallback(
            Evas_Callback_Type callbackType,
            typename EvasMemberConnection1<ThisType,
                                           ArgType1>::CbType callback,
            ThisType* callee,
            ArgType1* arg1)
        {
            Assert(m_object);
            Assert(callee);
            Assert(callbackType);
            Assert(callback);
            IConnection* connection =
                new EvasMemberConnection1<ThisType, ArgType1>(callbackType,
                                                              callback,
                                                              callee,
                                                              arg1,
                                                              this);
            m_connections.insert(connection);
            connection->ConnectPrv();
            return connection;
        }

        bool DisconnectCallback(IConnection* connection);
        void DisconnectAll();

        static void StaticOnDelEvent(void* data,
                                     Evas* /*e*/,
                                     Evas_Object* /*o*/,
                                     void* /*ev*/);

        IConnectionsSet m_connections;
        Evas_Object* m_object;
    };

  public:
    EvasObject();
    explicit EvasObject(Evas_Object* object);
    EvasObject(const EvasObject& other);
    ~EvasObject();

    EvasObject& operator=(const EvasObject& other);
    EvasObject* operator=(Evas_Object* object);

    operator Evas_Object *();

    bool IsValid() const
    {
        Assert(m_object);
        return m_object->GetObject() != NULL;
    }

    bool DisconnectCallback(IConnection* connection);
    void DisconnectAll();

    template <class ... Args>
    IConnection* ConnectSmartCallback(
        const char* callbackName,
        typename EvasObjectShared::SmartConnection<Args ...>::CbType
        callback,
        Args ... args)
    {
        Assert(m_object);
        return m_object->ConnectSmartCallback(callbackName, callback, args ...);
    }

    template <class ThisType, class ArgType1, class ArgType2>
    IConnection* ConnectMemberSmartCallback(
        const char* callbackName,
        typename EvasObjectShared::SmartMemberConnection2<ThisType,
                                                          ArgType1,
                                                          ArgType2>::CbType
        callback,
        ThisType* callee,
        ArgType1* arg1,
        ArgType2* arg2)
    {
        Assert(m_object);
        Assert(callee);
        Assert(callback);
        return m_object->ConnectMemberSmartCallback(callbackName,
                                                    callback,
                                                    callee,
                                                    arg1,
                                                    arg2);
    }

    template <class ThisType, class ArgType1>
    IConnection* ConnectMemberSmartCallback(
        const char* callbackName,
        typename EvasObjectShared::SmartMemberConnection1<ThisType,
                                                          ArgType1>::CbType
        callback,
        ThisType* callee,
        ArgType1* arg1)
    {
        Assert(m_object);
        Assert(callee);
        Assert(callback);
        return m_object->ConnectMemberSmartCallback(callbackName,
                                                    callback,
                                                    callee,
                                                    arg1);
    }

    template <class ArgType1, class ArgType2>
    IConnection* ConnectEvasCallback(
        Evas_Callback_Type callbackType,
        typename EvasObjectShared::EvasConnection1<ArgType1>::CbType
        callback,
        ArgType1* arg1,
        ArgType2* arg2)
    {
        Assert(m_object);
        return m_object->ConnectEvasCallback(callbackType, callback, arg1, arg2);
    }

    template <class ArgType1>
    IConnection* ConnectEvasCallback(
        Evas_Callback_Type callbackType,
        typename EvasObjectShared::EvasConnection1<ArgType1>::CbType
        callback,
        ArgType1* arg1)
    {
        Assert(m_object);
        return m_object->ConnectEvasCallback(callbackType, callback, arg1);
    }

    template <class ThisType, class ArgType1>
    IConnection* ConnectMemberEvasCallback(
        Evas_Callback_Type callbackType,
        typename EvasObjectShared::EvasMemberConnection1<ThisType,
                                                         ArgType1>::CbType
        callback,
        ThisType* callee,
        ArgType1* arg1)
    {
        Assert(m_object);
        Assert(callee);
        Assert(callback);
        return m_object->ConnectMemberEvasCallback(callbackType,
                                                   callback,
                                                   callee,
                                                   arg1);
    }

    template <class ThisType, class ArgType1, class ArgType2>
    IConnection* ConnectMemberEvasCallback(
        Evas_Callback_Type callbackType,
        typename EvasObjectShared::EvasMemberConnection2<ThisType, ArgType1,
                                                         ArgType2>::CbType
        callback,
        ThisType* callee,
        ArgType1* arg1,
        ArgType2* arg2)
    {
        Assert(m_object);
        Assert(callee);
        Assert(callback);
        return m_object->ConnectMemberEvasCallback(callbackType,
                                                   callback,
                                                   callee,
                                                   arg1,
                                                   arg2);
    }

  private:
    EvasObjectSharedPtr m_object;
};

#endif //WRT_SRC_DOMAIN_EFL_EVAS_OBJECT_H

