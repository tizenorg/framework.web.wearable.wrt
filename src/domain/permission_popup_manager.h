/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    permission_popup_manager.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#ifndef PERMISSION_POPUP_MANAGER_H_
#define PERMISSION_POPUP_MANAGER_H_

#include <list>
#include <utility>

#include <dpl/singleton.h>
#include <Evas.h>

class PermissionPopupManager
{
  public:
    void registerPopup(Evas_Object* webview, Evas_Object* popup);

  private:
    PermissionPopupManager();
    ~PermissionPopupManager();

    void unregisterPopup(Evas_Object* popup);
    void unregisterWebview(Evas_Object* webview);

    void addWebview(Evas_Object* webview);
    void addPopup(Evas_Object* popup);
    void removeWebview(Evas_Object* webview);
    void removePopup(Evas_Object* popup);

    static void deleteWebviewCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo);
    static void deletePopupCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo);

    typedef std::pair<Evas_Object*, Evas_Object* > Pair; // std::pair<webview, popup>
    typedef std::list<Pair> PairList;
    PairList m_pairList;

    friend class DPL::Singleton<PermissionPopupManager>;
};

typedef DPL::Singleton<PermissionPopupManager> PermissionPopupManagerSingleton;

#endif // PERMISSION_POPUP_MANAGER_H_
