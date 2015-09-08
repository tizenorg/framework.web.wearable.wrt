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
 * @file       main_thread.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "main_thread.h"
#include <dpl/assert.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <ace_api_client.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/singleton_impl.h>
#include <popup-runner/popup-runner.h>
IMPLEMENT_SINGLETON(MainThread)

using namespace WrtDB;

MainThread::MainThread() : m_attached(false) {}

MainThread::~MainThread()
{
    if (m_attached) {
        LogError("Destroyed without detach");
    }
}

void MainThread::AttachDatabases()
{
    Assert(!m_attached);
    // Attach databases
    ace_return_t ret = ace_client_initialize(Wrt::Popup::run_popup);
    Assert(ACE_OK == ret);
    WrtDB::WrtDatabase::attachToThreadRO();
    m_attached = true;
}

void MainThread::DetachDatabases()
{
    Assert(m_attached);
    m_attached = false;
    // Detach databases
    ace_return_t ret = ace_client_shutdown();
    Assert(ACE_OK == ret);
    WrtDB::WrtDatabase::detachFromThread();
}
