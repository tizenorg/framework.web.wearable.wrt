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
 * @file    view_logic_authentication_request_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "view_logic_authentication_request_support.h"

#include <string>

#include <dpl/assert.h>
#include <dpl/availability.h>
#include <dpl/log/secure_log.h>
#include <efl_assist.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#include <widget_string.h>

namespace ViewModule {
namespace {

const char* const EVAS_SMART_CALLBACK_CLICKED = "clicked";
const char* const EVAS_SMART_CALLBACK_CHANGED = "changed";
const char* const EVAS_SMART_CALLBACK_PREEDIT_CHANGED = "preedit,changed";
const char* const EVAS_SMART_CALLBACK_FOCUSED = "focused";
const char* const EVAS_SMART_CALLBACK_UNFOCUSED = "unfocused";
const char* const ELM_SIGNAL_ERASER_CLICKED = "elm,eraser,clicked";
const char* const ELM_SIGNAL_STATE_GUIDETEXT_HIDE = "elm,state,guidetext,hide";
const char* const ELM_SIGNAL_STATE_ERASER_SHOW = "elm,state,eraser,show";
const char* const ELM_SIGNAL_STATE_HIDE_SHOW = "elm,state,eraser,hide";
const char* const ELM_SWALLOW_CONTENT = "elm.swallow.content";
const char* const ELM_SWALLOW_LABEL = "elm.swallow.label";
const char* const ELM_SWALLOW_IDFIELD = "elm.swallow.idfield";
const char* const ELM_SWALLOW_PASSWDFIELD = "elm.swallow.pwfield";

const char* const GROUP_NAME_AUTHENTICATION_REQUEST_POPUP =
    "authRequestPopup";
const char* const THEME_EDITFIELD = "editfield";
const char* const THEME_DEFAULT = "default";

const char* const STYLE_POPUP_BUTTON_DEFAULT = "popup_button/default";
const char* const STYLE_DEFAULT_EDITFIELD =
    "DEFAULT='font_size=34 color=#808080 ellipsis=1'";

const char* const PART_IDFIELD_TEXT = "elm.swallow.idtext";
const char* const PART_PASSWORDFIELD_TEXT = "elm.swallow.pwtext";
const char* const PART_BUTTON1 = "button1";
const char* const PART_BUTTON2 = "button2";

const char* const ELM = "elm";
const char* const TITLE_TEXT = "title,text";
const char* const LAYOUT = "layout";
const char* const ERASER = "eraser";
const char* const WIDGET_NAME_POPUP = "elm_popup";
const char* const WIDGET_NAME_NAVIFRAME = "elm_naviframe";

const char* const AUTHENTICATION_REQUEST_TITLE_TEXT =
    "Authentication Requested";
const char* const AUTHENTICATION_REQUEST_BODY_PRETEXT =
    "A username and password are being requested by ";
const char* const AUTHENTICATION_REQUEST_BODY_MIDDLETEXT =
    ". The site says: ";

const char* const TEXT_DOUBLE_QUOTATION_MARKS = " \"";
const char* const TEXT_ID_FIELD = " User Name: ";
const char* const TEXT_PASSWORD_FIELD = " Password: ";
const char* const TEXT_OK = "Ok";
const char* const TEXT_CANCEL = "Cancel";

struct authenticationData {
    Ewk_Auth_Request* m_authRequest;
    std::string m_bodyText;
    Evas_Object* m_navi;
    Evas_Object* m_idEdit;
    Evas_Object* m_pwEdit;
};

// function declare
void askUserInformation(authenticationData* authData);
void loginAuthentication(authenticationData* authData);
void cancelAuthentication(authenticationData* authData);
Evas_Object* getEvasObjectByWidgetName(Evas_Object* obj, const char* name);
Evas_Object* createEdit(Evas_Object* parent, bool isIdEdit = false);
void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);
void buttonClickedCallback(void* data, Evas_Object* obj, void* eventInfo);
void editActivatedCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserInformation(authenticationData* authData)
{
    Evas_Object* popup = elm_popup_add(authData->m_navi);
    evas_object_size_hint_weight_set(popup,
                                     EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(popup,
                             TITLE_TEXT,
                             AUTHENTICATION_REQUEST_TITLE_TEXT);

    Evas_Object* label = elm_label_add(popup);
    evas_object_size_hint_weight_set(label,
                                     EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);

    elm_label_line_wrap_set(label , ELM_WRAP_WORD);
    elm_object_text_set(label, authData->m_bodyText.c_str());

    authData->m_idEdit = createEdit(popup, true);
    authData->m_pwEdit = createEdit(popup);

    Evas_Object* popupLayout = elm_layout_add(popup);
    elm_layout_file_set(popupLayout,
                        WRT_EDJ_PATH,
                        GROUP_NAME_AUTHENTICATION_REQUEST_POPUP);
    evas_object_size_hint_weight_set(popupLayout,
                                     EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
    /* FIXME : The text should be translated. */
    edje_object_part_text_set(elm_layout_edje_get(popupLayout),
                              PART_IDFIELD_TEXT,
                              TEXT_ID_FIELD);
    edje_object_part_text_set(elm_layout_edje_get(popupLayout),
                              PART_PASSWORDFIELD_TEXT,
                              TEXT_PASSWORD_FIELD);

    elm_object_part_content_set(popupLayout, ELM_SWALLOW_LABEL, label);
    elm_object_part_content_set(popupLayout,
                                ELM_SWALLOW_IDFIELD,
                                authData->m_idEdit);

    elm_object_part_content_set(popupLayout,
                                ELM_SWALLOW_PASSWDFIELD,
                                authData->m_pwEdit);

    elm_object_content_set(popup, popupLayout);

    Evas_Object* lButton = elm_button_add(popup);
    elm_object_text_set(lButton, TEXT_OK);
    elm_object_style_set(lButton, STYLE_POPUP_BUTTON_DEFAULT);
    elm_object_part_content_set(popup, PART_BUTTON1, lButton);
    evas_object_smart_callback_add(lButton,
                                   EVAS_SMART_CALLBACK_CLICKED,
                                   buttonClickedCallback,
                                   static_cast<void *>(authData));

    Evas_Object* rButton= elm_button_add(popup);
    elm_object_text_set(rButton, TEXT_CANCEL);
    elm_object_style_set(rButton, STYLE_POPUP_BUTTON_DEFAULT);
    elm_object_part_content_set(popup, PART_BUTTON2, rButton);
    evas_object_smart_callback_add(rButton,
                                   EVAS_SMART_CALLBACK_CLICKED,
                                   buttonClickedCallback,
                                   static_cast<void *>(authData));

    evas_object_show(popup);
}

void loginAuthentication(authenticationData* authData)
{
    _D("called");

    Assert(authData);

    const char* id = elm_entry_entry_get(authData->m_idEdit);
    const char* pw = elm_entry_entry_get(authData->m_pwEdit);
    ewk_auth_request_authenticate(authData->m_authRequest, const_cast<char*>(id), const_cast<char*>(pw));

    Evas_Object* popup = getEvasObjectByWidgetName(authData->m_idEdit, "elm_popup");
    if (popup) {
        evas_object_hide(popup);
        evas_object_del(popup);
    }
}

void cancelAuthentication(authenticationData* authData)
{
    _D("called");

    Assert(authData);

    ewk_auth_request_cancel(authData->m_authRequest);

    Evas_Object* popup = getEvasObjectByWidgetName(authData->m_idEdit, "elm_popup");
    if (popup) {
        evas_object_hide(popup);
        evas_object_del(popup);
    }
}

Evas_Object* getEvasObjectByWidgetName(Evas_Object* obj, const char* name)
{
    Assert(obj);
    Evas_Object* current = elm_object_parent_widget_get(obj);
    while (strcmp(elm_object_widget_type_get(current), name)) {
        current = elm_object_parent_widget_get(current);
        if (!current) {
            return NULL;
        }
    }
    return current;
}

Evas_Object* createEdit(Evas_Object* parent, bool isIdEdit)
{
    Evas_Object* edit = ea_editfield_add(parent, EA_EDITFIELD_SCROLL_SINGLELINE);
    elm_entry_cnp_mode_set(edit, ELM_CNP_MODE_PLAINTEXT);
    //elm_object_style_set(edit, "editfield/password/popup");
    elm_entry_single_line_set(edit, EINA_TRUE);
    elm_entry_scrollable_set(edit, EINA_TRUE);
    elm_entry_prediction_allow_set(edit, EINA_FALSE);
    elm_object_signal_emit(edit, "elm,action,hide,search_icon", "");
    elm_entry_autocapital_type_set(edit, ELM_AUTOCAPITAL_TYPE_NONE);

    if (isIdEdit) {
        evas_object_smart_callback_add(edit, "activated", editActivatedCallback, NULL);
    } else {
        elm_entry_password_set(edit, EINA_TRUE);
        elm_entry_input_panel_layout_set(edit, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
    }

    return edit;
}

void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    Assert(data);

    authenticationData* authData = static_cast<authenticationData*>(data);
    cancelAuthentication(authData);
}

void buttonClickedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(eventInfo);

    Assert(data);
    Assert(obj);

    authenticationData* authData = static_cast<authenticationData *>(data);
    Evas_Object* popup = getEvasObjectByWidgetName(obj, WIDGET_NAME_POPUP);

    bool allow = !strcmp(TEXT_OK, elm_object_text_get(obj));
    if (allow) {
        const char* id = elm_entry_entry_get(authData->m_idEdit);
        const char* pw = elm_entry_entry_get(authData->m_pwEdit);
        ewk_auth_request_authenticate(authData->m_authRequest,
                                      const_cast<char *>(id),
                                      const_cast<char *>(pw));
    } else {
        ewk_auth_request_cancel(authData->m_authRequest);
    }
    evas_object_hide(popup);
    evas_object_del(popup);
    delete authData;
}

void editActivatedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(eventInfo);
    DPL_UNUSED_PARAM(data);

    Assert(obj);
    elm_object_focus_set(obj, EINA_TRUE);
}
} // namespace

void AuthenticationRequestSupport::authenticationRequest(
    Evas_Object* webview,
    std::string url,
    void* data)
{
    _D("authenticationRequest called");
    authenticationData* authData = new authenticationData();
    Assert(webview);
    authData->m_navi = getEvasObjectByWidgetName(webview, "elm_naviframe");

    Assert(data);
    authData->m_authRequest = (Ewk_Auth_Request*)data;

    // create body text
    // TODO : The text should be translated
    const char* authRealm = ewk_auth_request_realm_get(authData->m_authRequest);
    if (authRealm != NULL) {
        authData->m_bodyText =
            std::string(AUTHENTICATION_REQUEST_BODY_PRETEXT) +
            std::string(url) +
            std::string(AUTHENTICATION_REQUEST_BODY_MIDDLETEXT) +
            std::string(TEXT_DOUBLE_QUOTATION_MARKS) +
            std::string(authRealm) +
            std::string(TEXT_DOUBLE_QUOTATION_MARKS);
    }

    ewk_auth_request_suspend(authData->m_authRequest);

    // ask to user
    askUserInformation(authData);
}
} // namespace ViewModule
