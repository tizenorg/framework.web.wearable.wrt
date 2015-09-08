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
/**
 * @file    i_view_module.h
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @version 1.0
 * @brief   Interface for ViewModule
 */

#ifndef WRT_SRC_VIEW_I_VIEW_MODULE_H_
#define WRT_SRC_VIEW_I_VIEW_MODULE_H_

#include <memory>
#include <string>
#include <common/evas_object.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <user_delegates.h>

class WidgetModel; //FORWARD DECLARATION
namespace ViewModule {
/** \brief Interface to ViewModule. Object of IViewModule type is returned from
 *  ViewModuleMgr factory.
 */
class IViewModule
{
  public:
    virtual ~IViewModule() { }
    virtual bool createView(Ewk_Context* context, Evas_Object* window) = 0;
    virtual void prepareView(WidgetModel *, const std::string &, int category) = 0;
    virtual void showWidget() = 0;
    virtual void hideWidget() = 0;
    virtual void suspendWidget() = 0;
    virtual void resumeWidget() = 0;
    virtual void resetWidgetFromSuspend() = 0;
    virtual void resetWidgetFromResume() = 0;
    virtual void TimeTick(long time) = 0;
    virtual void AmbientTick(long time) = 0;
    virtual void AmbientModeChanged(bool ambient_mode) = 0;
    virtual void backward() = 0;
    virtual void reloadStartPage() = 0;
    virtual Evas_Object* getCurrentWebview() = 0;
    virtual void fireJavascriptEvent(int event, void* data) = 0;
    virtual void setUserCallbacks(const WRT::UserDelegatesPtr& cbs) = 0;
    virtual void checkSyncMessageFromBundle(
            const char* name,
            const char* body,
            char** returnData) = 0;
    virtual void checkAsyncMessageFromBundle(
            const char* name,
            const char* body) = 0;
    virtual void downloadData(const char* url) = 0;
    virtual void activateVibration(bool on, uint64_t time) = 0;
};

typedef std::shared_ptr<IViewModule> IViewModulePtr;

/**
 * \brief This is a function for retrieving View object. It returns a pointer
 * to IViewModule object.
 */
IViewModulePtr createView();
} //namespace

#endif /* WRT_SRC_VIEW_I_VIEW_MODULE_H_ */
