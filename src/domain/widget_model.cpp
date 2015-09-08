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
 * @file    widget_model.cpp
 * @author  Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
 * @version 1.0
 * @brief   Implementation file for widget model
 */
#include "widget_model.h"

#include <dpl/event/model_bind_to_dao.h>
#include <dpl/platform.h>
#include <dpl/sstream.h>
#include <dpl/utils/folder_size.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

using namespace WrtDB;

template <typename RetType, RetType(WidgetDAOReadOnly::*extFun) () const >
struct BindToWidgetDAO :
    DPL::Event::BindToDAO<WidgetModel,
                          RetType,
                          DPL::String,
                          WidgetDAOReadOnly,
                          &WidgetModel::getTizenId,
                          extFun>
{};

template <typename RetType, RetType(*extFun) (DPL::String)>
struct BindToWidgetDAOStatic :
    DPL::Event::BindToDAO_Static<WidgetModel,
                                 RetType,
                                 DPL::String,
                                 &WidgetModel::getTizenId,
                                 extFun>
{};

WidgetModel::WidgetModel(const std::string &tizenId) :
    TizenId(DPL::FromASCIIString(tizenId)),
    TzPkgId(this, &BindToWidgetDAO<WrtDB::TizenPkgId,
                                   &WidgetDAOReadOnly::getTizenPkgId>::Get),
    Type(this, &BindToWidgetDAO<WidgetType,
                                &WidgetDAOReadOnly::getWidgetType>::Get),
    CspPolicy(this, &BindToWidgetDAO<DPL::OptionalString,
                                     &WidgetDAOReadOnly::getCspPolicy>::Get),
    CspReportOnlyPolicy(this, &BindToWidgetDAO<DPL::OptionalString,
                        &WidgetDAOReadOnly::getCspPolicyReportOnly>::Get),
    StartURL(this),
    //localized, so not binded
    StartFileInfo(this),
#if ENABLE(APP_SCHEME)
    PrefixURL(this, DPL::String(L"app://") + DPL::FromASCIIString(tizenId) + DPL::String(L"/")),
#else
    //localized, so not binded
    // file:// + / : without "/" path, webkit return security error
    PrefixURL(this, DPL::String(L"file:///")),
#endif
    InstallPath(
        this,
        &BindToWidgetDAO<DPL::String, &WidgetDAOReadOnly::getFullPath>::Get),
    PersistentStoragePath(this),
    TemporaryStoragePath(this),
    defaultlocale(
        this,
        &BindToWidgetDAO<DPL::OptionalString,
                         &WidgetDAOReadOnly::getDefaultlocale>::Get),
    Name(this),
    //localized, so not binded
    ShortName(this),
    //localized, so not binded
    Description(this),
    //localized, so not binded
    License(this),
    //localized, so not binded
    LicenseHref(this),
    //localized, so not binded
    Icon(this),
    SplashImg(
        this,
        &BindToWidgetDAO<DPL::OptionalString,
                         &WidgetDAOReadOnly::getSplashImgSrc>::Get),
    RequiredVersion(this, &BindToWidgetDAO<DPL::OptionalString, &WidgetDAOReadOnly::getMinimumWacVersion>::Get),
    WindowModes(
        this,
        &BindToWidgetDAO<WindowModeList,
                         &WidgetDAOReadOnly::getWindowModes>::Get),
    //localized, so not binded
    //    AccessNetwork(this, false),
    //    WarpDefinitionEmpty(this),
    BackSupported(
        this,
        //TODO this type has to be here now, as Property constructor is wrongly
        //chosen
        (DPL::Event::Property<bool,
                              DPL::Event::PropertyReadOnly,
                              DPL::Event::PropertyStorageDynamicCached>::
             ReadDelegateType) &
        BindToWidgetDAO<bool, &WidgetDAOReadOnly::getBackSupported>::Get),
    AccessList(this),
    SettingList(this),
    AppControlList(this),
    WidgetPrivilegeList(this, &BindToWidgetDAO<PrivilegeList, &WidgetDAOReadOnly::getWidgetPrivilege>::Get),
    SecurityModelVersion(this, &BindToWidgetDAO<WidgetSecurityModelVersion, &WidgetDAOReadOnly::getSecurityModelVersion>::Get)
{}

DPL::String WidgetModel::getTizenId() const
{
    return TizenId;
}
