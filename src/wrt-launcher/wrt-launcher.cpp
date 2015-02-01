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
#include <iostream>
#include <string>
#include <app.h>
#include <app_manager.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <dpl/exception.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/feature_dao_read_only.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <dpl/wrt-dao-ro/wrt_db_types.h>
#include <dpl/wrt-dao-ro/config_parser_data.h>
#include <appsvc.h>

#define TIMEOUT_DEFAULT     10
#define ROOT_DEFAULT_UID 0
#define ROOT_DEFAULT_GID 0
#define WEBAPP_DEFAULT_UID  5000
#define WEBAPP_DEFAULT_GID  5000
#define LOGGING_DEFAULT_GID 6509
#define RETURN_ERROR -1
#define BUF_SIZE 1024

static const char *program;

class DBConnection
{
  public:
    void AttachDatabase()
    {
        WrtDB::WrtDatabase::attachToThreadRW();
    }

    void DetachDatabase()
    {
        WrtDB::WrtDatabase::detachFromThread();
    }
};

static std::unique_ptr<DBConnection> g_dbConnection;

typedef struct
{
    char* guid;           /**< the widget's id
                           * (read from its config.xml during installation)*/
    char* name;         /**< the widget's name
                         * (read from its config.xml during installation)*/
    char* version;      /**< the widget's varsion
                         * (read from its config.xml during installation)*/
    char* pkg_id;       /**< the widget's pkg id */

    char* app_id;
} widget_info;

static void free_widget_info(widget_info* widget_info)
{
    if (widget_info) {
        delete[] widget_info->guid;
        delete[] widget_info->name;
        delete[] widget_info->version;
        delete[] widget_info->pkg_id;
        delete[] widget_info->app_id;
        delete widget_info;
    }
}

static char* new_strdup(const char *str)
{
    size_t size = strlen(str);
    char* ret = new char[size + 1];
    strcpy(ret, str);
    return ret;
}

static bool attachDbConnection()
{
    if (NULL == g_dbConnection.get()) {
        Try {
            g_dbConnection.reset(new DBConnection());
            g_dbConnection->AttachDatabase();
        }
        Catch(DPL::DB::SqlConnection::Exception::Base) {
            LogDebug("Fail to connect DB");
            return FALSE;
        }
    }
    return TRUE;
}

static bool display_widget_info()
{
    if (!attachDbConnection()) {
        return FALSE;
    }

    WidgetDAOReadOnlyList widgetList;

    Try {
        widgetList = WrtDB::WidgetDAOReadOnly::getWidgetList();
    } Catch (WrtDB::WidgetDAOReadOnly::Exception::DatabaseError) {
        LogError("Fail to get WidgetList");
        return FALSE;
    }

    printf("%3s %32s %16s %64s %16s %24s\n",
           "No", "Name", "Version", "GUID", "Package ID", "App ID");
    printf("%3s %32s %16s %64s %16s %24s\n",
           "--", "--", "----", "-------", "-----", "-----");

    int number = 1;
    FOREACH(dao, widgetList) {
        Try {
            widget_info *info = new widget_info;
            memset(info, 0x00, sizeof(widget_info));

            WrtDB::WidgetGUID guid = (*dao)->getGUID();
            DPL::OptionalString version = (*dao)->getVersion();
            WrtDB::TizenAppId appid = (*dao)->getTizenAppId();
            WrtDB::TizenPkgId pkgid = (*dao)->getTizenPkgId();


            /*get WidgetName*/
            DPL::OptionalString widget_name;
            DPL::OptionalString dl = (*dao)->getDefaultlocale();
            WrtDB::WidgetLocalizedInfo localizedInfo;
            if (!dl) {
                DPL::String languageTag(L"");
                localizedInfo = (*dao)->getLocalizedInfo(languageTag);
            } else {
                localizedInfo = (*dao)->getLocalizedInfo(*dl);
            }

            widget_name = localizedInfo.name;

            /*end get WidgetName*/
            if (!!widget_name) {
                info->name = new_strdup(DPL::ToUTF8String(*widget_name).c_str());
            }
            if (!!version) {
                info->version = new_strdup(DPL::ToUTF8String(*version).c_str());
            } else {
                std::string installedWidgetVersionString;
                installedWidgetVersionString = "";
                info->version = new_strdup(installedWidgetVersionString.c_str());
            }
            if (!!guid) {
                info->guid = new_strdup(DPL::ToUTF8String(*guid).c_str());
            }

            info->app_id = new_strdup(DPL::ToUTF8String(appid).c_str());
            info->pkg_id = new_strdup(DPL::ToUTF8String(pkgid).c_str());

            printf("%3i %32s %16s %64s %16s %24s\n",
                   number++,
                   !info->name ? "[NULL]" : info->name,
                   !info->version ? "[NULL]" : info->version,
                   !info->guid ? "[NULL]" : info->guid,
                   !info->pkg_id ? "[NULL]" : info->pkg_id,
                   !info->app_id ? "[NULL]" : info->app_id);

            free_widget_info(info);
        } Catch (WrtDB::WidgetDAOReadOnly::Exception::WidgetNotExist) {
            LogError("Installed application list is updated");
        }
    }

    return 1;
}

static void print_help(FILE *stream, int /*exit_code*/)
{
    fprintf(stream, "Usage : %s [ ... ]\n", program);
    fprintf(
        stream,
        "   -h                        --help              Display this usage information.\n"
        "   -l                        --list              Display installed widgets list\n"
        "   -s [tizen application ID] --start             Launch widget with tizen application ID\n"
        "   -k [tizen application ID] --kill              Kill widget with tizen application ID\n"
        "   -r [tizen application ID] --is-running        Check whether widget is running by tizen application ID,\n"
        "                                                 If widget is running, 0(zero) will be returned.\n"
        "   -d                        --debug             Activate debug mode\n"
        "   -t [second]               --timeout           Set timeout of response from widget in debug mode\n"
        "    if you emit this option, 5 seconds is set in debug mode\n"
        );
}

extern "C" int app_control_to_bundle(app_control_h app_control, bundle** data);

static void app_control_ReplyCallback(app_control_h /*request*/,
                                 app_control_h reply,
                                 app_control_result_e /*result*/,
                                 void* data)
{
    Ecore_Timer* app_control_Timer = static_cast<Ecore_Timer*>(data);
    if (app_control_Timer != NULL) {
        ecore_timer_del(app_control_Timer);
    }

    bundle* b = NULL;
    app_control_to_bundle(reply, &b);
    const char* port = appsvc_get_data(b, "port");
    if (port != NULL && strlen(port) > 0) {
        printf("port: %s\n", port);
        printf("result: %s\n", "launched");
    } else {
        printf("result: %s\n", "failed");
    }
    ecore_main_loop_quit();
    return;
}

static Eina_Bool timerCallback(void* /*data*/)
{
    printf("result: %s\n", "failed");
    ecore_main_loop_quit();
    return EINA_FALSE;
}

int main(int argc, char* argv[])
{
    UNHANDLED_EXCEPTION_HANDLER_BEGIN
    {
        // TODO: replace to use secure_log.h
        DPL::Log::LogSystemSingleton::Instance().SetTag("WRT-LAUNCHER");

        int next_opt, opt_idx = 0;
        int timeout = TIMEOUT_DEFAULT;
        char applicationId[BUF_SIZE] = "";
        char temp_arg[BUF_SIZE] = "";
        char pid[6] = "";
        char op = '\0';
        bool isDebugMode = false;
        bool dispHelp = false;
        bool dispList = false;
        Ecore_Timer* app_control_Timer = NULL;

        app_control_h app_control = NULL;
        int ret = APP_CONTROL_ERROR_NONE;

        program = argv[0];

        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "list", no_argument, 0, 'l' },
            { "start", required_argument, 0, 's' },
            { "kill", required_argument, 0, 'k' },
            { "is-running", required_argument, 0, 'r' },
            { "debug", no_argument, 0, 'd' },
            { "timeout", required_argument, 0, 't' },
            { 0, 0, 0, 0 }
        };

        if (argv[1] == NULL) {
            /* exit if any argument doesn't exist */
            print_help(stdout, 0);
            return -1;
        }

        if (ROOT_DEFAULT_UID == geteuid()) {
            if (RETURN_ERROR == setuid(ROOT_DEFAULT_UID)) {
                perror("Fail to set uid");
            }
        }
        if (ROOT_DEFAULT_GID == getegid()) {
            if (RETURN_ERROR == setgid(ROOT_DEFAULT_GID)) {
                perror("Fail to set gid");
            }
        }

        do {
            next_opt = getopt_long(argc,
                                   argv,
                                   "hls:k:r:dt:v:c:i:m:",
                                   long_options,
                                   &opt_idx);

            switch (next_opt) {
            case 'h':
                if (!dispHelp) {
                    print_help(stdout, 0);
                    dispHelp = true;
                }
                break;

            case 'l':
                if (dispList) {
                    break;
                }
                if (!display_widget_info()) {
                    printf("Fail to display the list of installed widgets");
                    return -1;
                }
                dispList = true;
                break;

            case 's':
            case 'k':
            case 'r':
                strncpy(temp_arg, optarg, BUF_SIZE);
                temp_arg[BUF_SIZE-1] = '\0';
                op = next_opt;
                break;

            case 't':
                timeout = atoi(optarg);
                if (timeout < 0) {
                    timeout = TIMEOUT_DEFAULT;
                }
                break;

            case 'd':
                isDebugMode = true;
                break;

            case -1:
                break;

            default:
                print_help(stdout, 0);
                break;
            }
        } while (next_opt != -1);

        if ((op == 's') || (op == 'k') || (op == 'r')) {
            std::string temp;

            if (NULL == g_dbConnection.get()) {
                Try {
                    g_dbConnection.reset(new DBConnection());
                    g_dbConnection->AttachDatabase();
                }
                Catch(DPL::DB::SqlConnection::Exception::Base) {
                    LogDebug("Fail to connect DB");
                    return FALSE;
                }
            }
            DPL::OptionalString normal_str = DPL::FromUTF8String(temp_arg);
            WrtDB::NormalizeAndTrimSpaceString(normal_str);
            std::string normal_arg = DPL::ToUTF8String(*normal_str);

            WidgetDAOReadOnlyList widgetList =
                WrtDB::WidgetDAOReadOnly::getWidgetList();
            FOREACH(dao, widgetList) {
                WrtDB::TizenAppId tizenAppId = (*dao)->getTizenAppId();
                if (!strcmp(DPL::ToUTF8String(tizenAppId).c_str(),
                            normal_arg.c_str()))
                {
                    temp = DPL::ToUTF8String(tizenAppId);
                    break;
                }
            }
            if (!temp.empty()) {
                strncpy(applicationId, temp.c_str(), BUF_SIZE);
                applicationId[BUF_SIZE-1] = '\0';
            } else {
                printf("result: %s\n", "failed");
                return -1;
            }
        }

        if (op == 's') {
            if (strlen(applicationId) <= 0) {
                printf("result: %s\n", "failed");
                return -1;
            }

            // create app_control
            ret = app_control_create(&app_control);
            if (APP_CONTROL_ERROR_NONE != ret && NULL == app_control) {
                printf("result: %s\n", "failed");
                return -1;
            }

            // set package
            ret = app_control_set_app_id(app_control, applicationId);
            if (APP_CONTROL_ERROR_NONE != ret) {
                printf("result: %s\n", "failed");
                app_control_destroy(app_control);
                return -1;
            }

            if (true == isDebugMode) {
                // set debug mode
                ret = app_control_add_extra_data(app_control, "debug", "true");
                if (APP_CONTROL_ERROR_NONE != ret) {
                    LogError("Fail to set debug mode [" << ret << "]");
                    app_control_destroy(app_control);
                    return FALSE;
                }

                // set pid
                snprintf(pid, sizeof(pid), "%d", getpid());
                ret = app_control_add_extra_data(app_control, "pid", pid);
                if (APP_CONTROL_ERROR_NONE != ret) {
                    LogError("Fail to set pid [" << ret << "]");
                    app_control_destroy(app_control);
                    return FALSE;
                }

                ecore_init();
                app_control_Timer = ecore_timer_add(timeout, timerCallback, NULL);
                ret = app_control_send_launch_request(app_control,
                                                  app_control_ReplyCallback,
                                                  app_control_Timer);
            } else {
                ret = app_control_send_launch_request(app_control, NULL, NULL);
            }

            if (APP_CONTROL_ERROR_NONE != ret) {
                printf("result: %s\n", "");
                app_control_destroy(app_control);
                return -1;
            }

            app_control_destroy(app_control);

            if (true == isDebugMode) {
                // wait response from callee
                ecore_main_loop_begin();
                return 0;
            } else {
                // This text should be showed for IDE
                printf("result: %s\n", "launched");
            }
            return 0;
        } else if (op == 'k') {
            bool isRunning = false;

            //checks whether the application is running
            ret = app_manager_is_running(applicationId, &isRunning);
            if (APP_MANAGER_ERROR_NONE != ret) {
                printf("result: %s\n", "failed");
                return -1;
            }

            if (true == isRunning) {
                // get app_context for running application
                // app_context must be released with app_context_destroy
                app_context_h appCtx = NULL;
                ret = app_manager_get_app_context(applicationId, &appCtx);
                if (APP_MANAGER_ERROR_NONE != ret) {
                    printf("result: %s\n", "failed");
                    return -1;
                }

                // terminate app_context_h
                ret = app_manager_terminate_app(appCtx);
                if (APP_MANAGER_ERROR_NONE != ret) {
                    printf("result: %s\n", "failed");
                    app_context_destroy(appCtx);
                    return -1;
                } else {
                    printf("result: %s\n", "killed");
                    app_context_destroy(appCtx);
                    return 0;
                }
            } else {
                printf("result: %s\n", "App isn't running");
                return 0;
            }
        } else if (op == 'r') {
            bool isRunning = false;
            ret = app_manager_is_running(applicationId, &isRunning);

            if (APP_MANAGER_ERROR_NONE != ret) {
                printf("result: %s\n", "failed");
                return -1;
            }

            if (true == isRunning) {
                printf("result: %s\n", "running");
                return 0;
            } else {
                printf("result: %s\n", "not running");
                return -1;
            }
        }

        return 0 ;
    }
    UNHANDLED_EXCEPTION_HANDLER_END
}

