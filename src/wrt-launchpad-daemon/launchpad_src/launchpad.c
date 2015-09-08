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
 * simple AUL daemon - launchpad
 */

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <malloc.h>

#include "app_sock.h"
#include <aul.h>

#include <stdint.h>
#include <dbus/dbus.h>

#include "config.h"

#include "menu_db_util.h"
#include "simple_util.h"
#include "access_control.h"
#include "preload.h"
#include "preexec.h"
#include "perf.h"
#include "sigchild.h"
#include "aul_util.h"

#include "util_x.h"

#include "gl.h"

#include <app-checker.h>
#include <sqlite3.h>

#include "process_pool.h"
#include "launchpad_util.h"

#define _static_ static inline
#define SQLITE_FLUSH_MAX    (1048576)       /* (1024*1024) */
#define AUL_POLL_CNT        15
#define AUL_PR_NAME         16
#define PKG_ID_LENGTH       11

#define EXEC_DUMMY_EXPIRED 5
#define DIFF(a,b) (((a)>(b))?(a)-(b):(b)-(a))
#define WRT_CLIENT_PATH "/usr/bin/wrt-client"
#define LOWEST_PRIO 20
#define DUMMY_NONE 0
#define BOOST_TIME 3000
#define CHECK_Xorg "/tmp/.wm_ready"
#define DEBUG_NETWORK "system::debugging_network"

enum {
    LAUNCH_PAD = 0,
    POOL_SERVER,
    POOL_CLIENT,
    POLLFD_MAX
};

static char *launchpad_cmdline;
static int initialized = 0;
static int pool_client_used = 0; /* 0 = available, 1 = already used */
static int pool_client_ui_pid = DUMMY_NONE;
static int pool_client_web_pid = DUMMY_NONE;
static int pool_client_ui_fd = -1;
static int pool_client_creating = 0;
static int process_pool_disable = 0;
static struct pollfd pfds[POLLFD_MAX];

_static_ int __prepare_exec(const char *pkg_name,
                            const char *app_path, app_info_from_db * menu_info,
                            bundle * kb);
_static_ int __fake_launch_app(int cmd, int pid, bundle * kb);
_static_ char **__create_argc_argv(bundle * kb, int *margc);
_static_ void __real_launch(const char *app_path, bundle * kb);
_static_ int __dummy_launch(int dummy_client_fd, app_pkt_t* pkt);
_static_ int __child_raise_win_by_x(int pid, void *priv);
_static_ int __raise_win_by_x(int pid);
_static_ int __send_to_sigkill(int pid);
_static_ int __term_app(int pid);
_static_ int __resume_app(int pid);
_static_ int __real_send(int clifd, int ret);
_static_ void __send_result_to_caller(int clifd, int ret);
_static_ void __launchpad_exec_dummy(int launchpad_fd, int pool_server_fd, int client_fd);
_static_ void __launchpad_clear_dummy();
_static_ void __launchpad_main_loop(int launchpad_fd, int pool_server_fd);
_static_ int __launchpad_pre_init(int argc, char **argv);
_static_ int __launchpad_post_init();
_static_ void __launchpad_use_boost();

static int append_variant(DBusMessageIter *iter, const char *sig, char *param[]);
int invoke_dbus_method_sync(const char *dest, const char *path,
                            const char *interface, const char *method,
                            const char *sig, char *param[]);

int send_dbus_signal(const char *path, const char *iface, const char *name);


_static_ int __prepare_exec(const char *pkg_name,
                            const char *app_path, app_info_from_db * menu_info,
                            bundle * kb)
{
    char *file_name;
    char process_name[AUL_PR_NAME];

    /* Set new session ID & new process group ID*/
    /* In linux, child can set new session ID without check permission */
    /* TODO : should be add to check permission in the kernel*/
    setsid();

    __preexec_run(menu_info->pkg_type, pkg_name, app_path);

    /* SET PRIVILEGES*/
    char pkg_id[PKG_ID_LENGTH];
    memset(pkg_id, '\0', PKG_ID_LENGTH);
    snprintf(pkg_id, PKG_ID_LENGTH, "%s", pkg_name);

    /* SET PR_SET_KEEPCAPS */
    if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
        _E("prctl(PR_SET_KEEPCAPS) failed.");
    }

    const char* is_debug = bundle_get_val(kb, "debug");
    if (is_debug != NULL && !strcmp(is_debug, "true")) {
        _D("Add system::debugging_network smack rule");
        __add_smack_rule(pkg_id, DEBUG_NETWORK, "rw");
        __add_smack_rule(DEBUG_NETWORK, pkg_id, "w");
    }

    if (__set_access(pkg_id, menu_info->pkg_type, app_path) < 0) {
        _D("fail to set privileges - check your package's credential\n");
        return -1;
    }

    __set_inherit_bit_for_CAP_MAC_ADMIN();

    /* SET DUMPABLE - for coredump*/
    prctl(PR_SET_DUMPABLE, 1);

    /* SET PROCESS NAME*/
    if (app_path == NULL) {
        _D("app_path should not be NULL - check menu db");
        return -1;
    }
    file_name = strrchr(app_path, '/');
    if (file_name == NULL) {
        _D("can't locate file name to execute");
        return -1;
    }
    memset(process_name, '\0', AUL_PR_NAME);
    snprintf(process_name, AUL_PR_NAME, "%s", file_name + 1);
    prctl(PR_SET_NAME, process_name);

    /* SET ENVIROMENT*/
    __set_env(menu_info, kb);

    return 0;
}

_static_ int __fake_launch_app(int cmd, int pid, bundle * kb)
{
    int datalen;
    int ret;
    bundle_raw *kb_data;

    bundle_encode(kb, &kb_data, &datalen);
    if ((ret = __app_send_raw(pid, cmd, kb_data, datalen)) < 0) {
        _E("error request fake launch - error code = %d", ret);
    }
    free(kb_data);
    return ret;
}

_static_ void __real_launch(const char *app_path, bundle * kb)
{
    int app_argc;
    char **app_argv;
    int i;

    app_argv = __create_argc_argv(kb, &app_argc);

#ifndef NATIVE_LAUNCHPAD
    if (__change_cmdline((char *)app_path) < 0) {
        _E("change cmdline fail");
        return;
    }

    app_argv[0] = g_argv[0];
#else
    app_argv[0] = strdup(app_path);
#endif

    for (i = 0; i < app_argc; i++) {
        _D("input argument %d : %s##", i, app_argv[i]);
    }

    PERF("setup argument done");
    _E("lock up test log(no error) : setup argument done");

    /* Temporary log: launch time checking */
    LOG(LOG_DEBUG, "LAUNCH", "[%s:Platform:launchpad:done]", app_path);

    __preload_exec(app_argc, app_argv);
}

_static_ int __dummy_launch(int dummy_client_fd, app_pkt_t* pkt)
{
    return __send_pkt_raw_data(dummy_client_fd, pkt);
}

_static_ int __child_raise_win_by_x(int pid, void *priv)
{
    // warning: unused parameter
    priv = priv;

    return x_util_raise_win(pid);
}

_static_ int __raise_win_by_x(int pid)
{
    int pgid;
    if (x_util_raise_win(pid) == 0) {
        return 0;
    }

    /* support app launched by shell script*/
    pgid = getpgid(pid);
    _D("X raise failed. try to find first child & raise it - c:%d p:%d\n",
       pgid, pid);

    if (pgid <= 1) {
        return -1;
    }
    if (__proc_iter_pgid(pgid, __child_raise_win_by_x, NULL) < 0) {
        return -1;
    }

    return 0;
}

_static_ int __send_to_sigkill(int pid)
{
    int pgid;

    pgid = getpgid(pid);
    if (pgid <= 1) {
        return -1;
    }

    if (killpg(pgid, SIGKILL) < 0) {
        return -1;
    }

    return 0;
}

_static_ int __term_app(int pid)
{
    int dummy;
    if (__app_send_raw
            (pid, APP_TERM_BY_PID, (unsigned char *)&dummy, sizeof(int)) < 0)
    {
        _D("terminate packet send error - use SIGKILL");
        if (__send_to_sigkill(pid) < 0) {
            _E("fail to killing - %d\n", pid);
            return -1;
        }
    }
    _D("term done\n");
    return 0;
}

_static_ int __resume_app(int pid)
{
    int dummy;
    int ret;
    if ((ret =
             __app_send_raw(pid, APP_RESUME_BY_PID, (unsigned char *)&dummy,
                            sizeof(int))) < 0)
    {
        if (ret == -EAGAIN) {
            _E("resume packet timeout error");
        } else {
            _D("resume packet send error - use raise win");
            if (__raise_win_by_x(pid) < 0) {
                _E("raise failed - %d resume fail\n", pid);
                _E("we will term the app - %d\n", pid);
                __send_to_sigkill(pid);
                ret = -1;
            } else {
                ret = 0;
            }
        }
    }
    _D("resume done\n");
    return ret;
}

static int __get_caller_pid(bundle *kb)
{
    const char *pid_str;
    int pid;

    pid_str = bundle_get_val(kb, AUL_K_ORG_CALLER_PID);
    if (pid_str) {
        goto end;
    }

    pid_str = bundle_get_val(kb, AUL_K_CALLER_PID);
    if (pid_str == NULL) {
        return -1;
    }

end:
    pid = atoi(pid_str);
    if (pid <= 1) {
        return -1;
    }

    return pid;
}

_static_ int __foward_cmd(int cmd, bundle *kb, int cr_pid)
{
    int pid;
    char tmp_pid[MAX_PID_STR_BUFSZ];
    int datalen;
    bundle_raw *kb_data;
    int res;

    if ((pid = __get_caller_pid(kb)) < 0) {
        return AUL_R_ERROR;
    }

    snprintf(tmp_pid, MAX_PID_STR_BUFSZ, "%d", cr_pid);

    bundle_add(kb, AUL_K_CALLEE_PID, tmp_pid);

    bundle_encode(kb, &kb_data, &datalen);
    if ((res = __app_send_raw(pid, cmd, kb_data, datalen)) < 0) {
        res = AUL_R_ERROR;
    }

    free(kb_data);

    return res;
}

_static_ int __real_send(int clifd, int ret)
{
    if (send(clifd, &ret, sizeof(int), MSG_NOSIGNAL) < 0) {
        if (errno == EPIPE) {
            _E("send failed due to EPIPE.\n");
            close(clifd);
            return -1;
        }
        _E("send fail to client");
    }

    close(clifd);
    return 0;
}

_static_ void __send_result_to_caller(int clifd, int ret)
{
    char *cmdline;
    int wait_count;
    int cmdline_changed = 0;
    int cmdline_exist = 0;
    int r;

    if (clifd == -1) {
        return;
    }

    if (ret <= 1) {
        __real_send(clifd, ret);
        return;
    }
    /* check normally was launched?*/
    wait_count = 1;
    do {
        cmdline = __proc_get_cmdline_bypid(ret);
        if (cmdline == NULL) {
            _E("error founded when being launched with %d", ret);
        } else if (strcmp(cmdline, launchpad_cmdline)) {
            free(cmdline);
            cmdline_changed = 1;
            break;
        } else {
            cmdline_exist = 1;
            free(cmdline);
        }

        _D("-- now wait to change cmdline --");
        struct timespec duration = { 0, 50 * 1000 * 1000 };
        nanosleep(&duration, NULL);     /* 50ms sleep*/
        wait_count++;
    } while (wait_count <= 20);         /* max 50*20ms will be sleep*/

    if ((!cmdline_exist) && (!cmdline_changed)) {
        __real_send(clifd, -1);         /* abnormally launched*/
        return;
    }

    if (!cmdline_changed) {
        _E("process launched, but cmdline not changed");
    }

    if (__real_send(clifd, ret) < 0) {
        r = kill(ret, SIGKILL);
        if (r == -1) {
            _E("send SIGKILL: %s", strerror(errno));
        }
    }

    return;
}

_static_ void __launchpad_exec_dummy(int launchpad_fd, int pool_server_fd, int client_fd)
{
    _D("%s - pool_client_creating = %d" ,__func__, pool_client_creating);
    // dummy process was created but does not connected with daemon, so we should wait
    if( pool_client_creating )
        return;

    pool_client_creating = 1;

    int pid;
    pid = fork();

    if (pid == 0) // child
    {
        setpriority(PRIO_PROCESS, 0, LOWEST_PRIO);
        // add this dummy process to background cgroup 
        // later this would be changed after recieving launch request
        // at this time, there is no web process
        if (__set_background_cgroup(1, getpid(), 0) < 0) {
            _D("failed to set dummy process into background cgroup");
        } else {
            _D("success to set dummy process into background cgroup");
        }

        _D("Launch dummy process...");

        /* Set new session ID & new process group ID*/
        /* In linux, child can set new session ID without check permission */
        /* TODO : should be add to check permission in the kernel*/
        setsid();

        if (launchpad_fd != -1)
        {
            close(launchpad_fd);
        }

        if (pool_server_fd != -1)
        {
            close(pool_server_fd);
        }

        if (client_fd != -1)
        {
            close(client_fd);
        }

        __signal_unset_sigchld();
        __signal_fini();

        /* SET PR_SET_KEEPCAPS */
        if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
            _E("prctl(PR_SET_KEEPCAPS) failed.");
        }

        /* SET DUMPABLE - for coredump*/
        prctl(PR_SET_DUMPABLE, 1);

        {
            void *handle = NULL;
            int (*dl_main) (int, char **);

            handle = dlopen(WRT_CLIENT_PATH, RTLD_NOW | RTLD_GLOBAL);

            if (handle == NULL)
            {
                _E("dlopen failed.");
                exit(-1);
            }

            dl_main = dlsym(handle, "main");

            sprintf(g_argv[1], "%s", "-d");

            if (dl_main != NULL)
            {
                dl_main(g_argc, g_argv);
            }
            else
            {
                _E("dlsym not founded. bad preloaded app - check fpie pie");
            }

            exit(0);
        }
    }

    // clear resource of dummy process
    __launchpad_clear_dummy();
}

_static_ void __launchpad_clear_dummy()
{
    if (pfds[POOL_CLIENT].fd > 0)
    {
        close(pfds[POOL_CLIENT].fd);
    }

    // in this case, resources for dummy process should be clear
    pool_client_ui_pid = DUMMY_NONE;
    pool_client_web_pid = DUMMY_NONE;
    pool_client_ui_fd = -1;

    pfds[POOL_CLIENT].fd = -1;
    pfds[POOL_CLIENT].events  = 0;
    pfds[POOL_CLIENT].revents = 0;
}

_static_ void __launchpad_use_boost()
{
    char *arr[2];
    char val[32];
    snprintf(val, sizeof(val), "%d", BOOST_TIME);
    arr[0] = val;
    arr[1] = NULL;
    /* TODO - Handle return value */
    invoke_dbus_method_sync(SYSTEM_BUS_NAME, SYSTEM_OBJECT_PATH,
                                      SYSTEM_INTERFACE_NAME, SYSTEM_METHOD_NAME, "i", arr);
}

_static_ void __launchpad_main_loop(int launchpad_fd, int pool_server_fd)
{
    bundle *kb = NULL;
    app_pkt_t *pkt = NULL;
    app_info_from_db *menu_info = NULL;

    const char *pkg_name = NULL;
    const char *app_path = NULL;
    int pid = -1;
    int clifd = -1;
    struct ucred cr;
    char sock_path[UNIX_PATH_MAX] = { 0, };
    char *arr[4];

    // add power lock to guarrenty app launching
    arr[0] = "lcdoff";
    arr[1] = "staycurstate";
    arr[2] = "NULL";
    arr[3] = "10000";
    invoke_dbus_method_sync("org.tizen.system.deviced",
                            "/Org/Tizen/System/DeviceD/Display",
                            "org.tizen.system.deviced.display",
                            "lockstate", "sssi", arr);

    pkt = __app_recv_raw(launchpad_fd, &clifd, &cr);
    if (!pkt) {
        _D("packet is NULL");
        goto end;
    }

    kb = bundle_decode(pkt->data, pkt->len);
    if (!kb) {
        _D("bundle decode error");
        goto end;
    }

    INIT_PERF(kb);
    PERF("packet processing start");

    pkg_name = bundle_get_val(kb, AUL_K_PKG_NAME);
    SECURE_LOGD("pkg name : %s\n", pkg_name);

    menu_info = _get_app_info_from_bundle_by_pkgname(pkg_name, kb);
    if (menu_info == NULL) {
        _D("such pkg no found");
        goto end;
    }

    app_path = _get_app_path(menu_info);
    if (app_path == NULL) {
        _E("app_path is NULL");
        goto end;
    }
    if (app_path[0] != '/') {
        _D("app_path is not absolute path");
        goto end;
    }

    __modify_bundle(kb, cr.pid, menu_info, pkt->cmd);
    pkg_name = _get_pkgname(menu_info);

    PERF("get package information & modify bundle done");

    // use system boost temporarily
    __launchpad_use_boost();

    // check if this is first launching or not
    if (!process_pool_disable && pool_client_ui_pid != DUMMY_NONE
            && !pool_client_used)
    {
        snprintf(sock_path, UNIX_PATH_MAX, "%s/%d", AUL_SOCK_PREFIX, pool_client_ui_pid);
        unlink(sock_path);

        // this should be changed for launching new web app
        if (__set_background_cgroup(0, pool_client_ui_pid, pool_client_web_pid) < 0) {
            _D("failed to set dummy process into foreground cgroup");
        } else {
            _D("success to set dummy process into foreground cgroup");
        }
        pool_client_used = 1;
        __dummy_launch(pool_client_ui_fd, pkt);
        pid = pool_client_ui_pid;
        _D("==> app launch using dummy process(%d) : %s\n", pid, app_path);
    } else {
        pid = fork();
        if (pid == 0)
        {
            PERF("fork done");

            // this process should not be set to background cgroup!
            __set_background_cgroup(0, getpid(), 0);

            // clear resources of launchpad process like opened fd
            close(clifd);
            close(launchpad_fd);
            close(pool_server_fd);
            __signal_unset_sigchld();
            __signal_fini();

            snprintf(sock_path, UNIX_PATH_MAX, "%s/%d", AUL_SOCK_PREFIX, getpid());
            unlink(sock_path);

            PERF("prepare exec - first done");

            if (__prepare_exec(pkg_name, app_path,
                               menu_info, kb) < 0)
            {
                SECURE_LOGE("preparing work fail to launch - "
                   "can not launch %s\n", pkg_name);
                exit(-1);
            }

            PERF("prepare exec - second done");

            __real_launch(app_path, kb);
            exit(-1);
        }
        _D("==> app launch using process forked directly(%d) : %s\n", pid, app_path);
    }

end:
    __send_result_to_caller(clifd, pid);

    /*TODO: retry*/
    __signal_block_sigchld();
    __send_app_launch_signal(pid);
    __signal_unblock_sigchld();

    if (menu_info != NULL) {
        _free_app_info_from_db(menu_info);
    }

    if (kb != NULL) {
        bundle_free(kb);
    }
    if (pkt != NULL) {
        free(pkt);
    }

    /* Active Flusing for Daemon */
    if (initialized > AUL_POLL_CNT) {
        sqlite3_release_memory(SQLITE_FLUSH_MAX);
        malloc_trim(0);
        initialized = 1;
    }
}

_static_ int __launchpad_pre_init(int argc, char **argv)
{
    /* signal init*/
    __signal_init();

    /* get my(launchpad) command line*/
    launchpad_cmdline = __proc_get_cmdline_bypid(getpid());
    if (launchpad_cmdline == NULL) {
        _E("launchpad cmdline fail to get");
        return -1;
    }
    _D("launchpad cmdline = %s", launchpad_cmdline);

    __preload_init(argc, argv);

    __preload_init_for_wrt();

    __preexec_init(argc, argv);

    return 0;
}

_static_ int __launchpad_post_init()
{
    /* Setting this as a global variable to keep track
     * of launchpad poll cnt */
    /* static int initialized = 0;*/

    if (initialized) {
        initialized++;
        return 0;
    }

    if (__signal_set_sigchld() < 0) {
        return -1;
    }

    initialized++;

    return 0;
}

int send_dbus_signal(const char *path, const char *iface, const char *name){
    DBusConnection *conn;
    DBusMessage *msg;

    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, NULL);
    dbus_connection_set_exit_on_disconnect(conn, FALSE);

    msg = dbus_message_new_signal(path, iface, name);
    if (!msg) {
        _E("dbus_message_new_signal(%s:%s-%s)", path, iface, name);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -EBADMSG;
    }

    dbus_connection_send (conn, msg, NULL);
    dbus_message_unref(msg);

    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    return 0;
}


int invoke_dbus_method_sync(const char *dest, const char *path,
                            const char *interface, const char *method,
                            const char *sig, char *param[])
{
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessageIter iter;
    DBusMessage *reply;
    DBusError err;
    int r, ret;

    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, NULL);
    dbus_connection_set_exit_on_disconnect(conn, FALSE);

    msg = dbus_message_new_method_call(dest, path, interface, method);
    if (!msg) {
        _E("dbus_message_new_method_call(%s:%s-%s)", path, interface, method);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -EBADMSG;
    }

    dbus_message_iter_init_append(msg, &iter);
    r = append_variant(&iter, sig, param);
    if (r < 0) {
        _E("append_variant error(%d)", r);
        dbus_message_unref(msg);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -EBADMSG;
    }

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, 500, &err);
    dbus_message_unref(msg);
    if (!reply) {
        _E("dbus_connection_send error(%s:%s)", err.name, err.message);
        dbus_error_free(&err);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -EBADMSG;
    }

    r = dbus_message_get_args(reply, &err, DBUS_TYPE_INT32, &ret, DBUS_TYPE_INVALID);
    dbus_message_unref(reply);
    if (!r) {
        _E("no message : [%s:%s]", err.name, err.message);
        dbus_error_free(&err);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -EBADMSG;
    }

    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    return ret;
}

static int append_variant(DBusMessageIter *iter, const char *sig, char *param[])
{
    char *ch;
    int i;
    int int_type;
    uint64_t int64_type;

    if (!sig || !param)
        return 0;

    for (ch = (char*)sig, i = 0; *ch != '\0'; ++i, ++ch) {
        switch (*ch) {
        case 'i':
            int_type = atoi(param[i]);
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &int_type);
            break;
        case 'u':
            int_type = atoi(param[i]);
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &int_type);
            break;
        case 't':
            int64_type = atoi(param[i]);
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &int64_type);
            break;
        case 's':
            dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &param[i]);
            break;
        default:
            return -EINVAL;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int launchpad_fd = -1, pool_server_fd = -1;
    int ret, count = 0;
    memset(pfds, 0x00, sizeof(pfds));

    // process pool feature disable
    if (getenv("WRT_PROCESS_POOL_DISABLE"))
    {
        process_pool_disable = 1;
    }

    /* preloading */
    if( __launchpad_pre_init(argc, argv) < 0){
        _E("launchpad pre init failed");
        goto exit_main;
    }


    /* create launchpad sock */
    launchpad_fd = __create_server_sock(WRT_LAUNCHPAD_PID);
    if (launchpad_fd < 0) {
        _E("server sock error");
        goto exit_main;
    }
    pfds[LAUNCH_PAD].fd = launchpad_fd;
    pfds[LAUNCH_PAD].events = POLLIN;
    pfds[LAUNCH_PAD].revents = 0;

    pool_server_fd = __create_process_pool_server();
    if (pool_server_fd == -1)
    {
        _E("Error creationg pool server!");
        goto exit_main;
    }

    pfds[POOL_SERVER].fd = pool_server_fd;
    pfds[POOL_SERVER].events = POLLIN;
    pfds[POOL_SERVER].revents = 0;

    /* waiting initialzing Xorg process during 2 seconds */
    while (count != 5) {
        ret = access(CHECK_Xorg, F_OK);
        if (ret == 0)
            break;
        usleep(500*1000);
        count++;
    }

#ifdef USE_POOL_FOR_FIRST_LAUNCH
    // dummy process is created in advance for first launching
    if(!process_pool_disable){
        __launchpad_exec_dummy(launchpad_fd, pool_server_fd, -1);
    }
#endif

    send_dbus_signal(DBUS_WRT_OBJECT_PATH, DBUS_WRT_INTERFACE_NAME, DBUS_WRT_SIGNAL_READYDONE);

    while (1)
    {
        if (poll(pfds, POLLFD_MAX, -1) < 0)
        {
            continue;
        }

        _D("pfds[LAUNCH_PAD].revents  : 0x%x", pfds[LAUNCH_PAD].revents) ;
        _D("pfds[POOL_SERVER].revents : 0x%x", pfds[POOL_SERVER].revents) ;
        _D("pfds[POOL_CLIENT].revents : 0x%x", pfds[POOL_CLIENT].revents) ;

        /* init with concerning X & EFL (because of booting
        * sequence problem)*/
        if (__launchpad_post_init() < 0)
        {
            _E("launcpad post init failed");
            goto exit_main;
        }

        if ((pfds[LAUNCH_PAD].revents & POLLIN) != 0)
        {
            _D("pfds[LAUNCH_PAD].revents & POLLIN");
            __launchpad_main_loop(pfds[LAUNCH_PAD].fd, pfds[POOL_SERVER].fd);
        }

        if (process_pool_disable) {
            continue;
        }

        // this is available in case of only process pool
        if ((pfds[POOL_SERVER].revents & POLLIN) != 0)
        {
            _D("pfds[POOL_SERVER].revents & POLLIN");
            __accept_process_pool_client(
                    pfds[POOL_SERVER].fd, &pool_client_ui_fd, 
                    &pool_client_ui_pid, &pool_client_web_pid);

            pfds[POOL_CLIENT].fd = pool_client_ui_fd;
            pfds[POOL_CLIENT].events = POLLIN | POLLHUP | POLLNVAL;
            pfds[POOL_CLIENT].revents = 0;
            pool_client_creating = 0;
            pool_client_used = 0;
        }

        if ((pfds[POOL_CLIENT].revents & POLLIN) != 0)
        {
            _D("pfds[POOL_CLIENT].revents & POLLIN (pid:%d)", pool_client_ui_pid);
            // 1. define status that app finishes to be launched normally
            // 2. recieve status value from launched app
            // 3. handle proper action according to status value
            //
            // status description
            //    0  : success to launch app
            //    -1 : launching is delayed. that is, 5 seconds was gone.
            //    -2 : failed to launch app
            int status = -2;
            int ret = 0;
            ret = recv(pool_client_ui_fd, &status, sizeof(status), 0);
            if (ret < 0) {
                _E("failed to receive launch status");
            } else {
                _D("launch status: %d", status);
            }

            if (pool_client_used) {
                // create new dummy process unconditionally,
                // regardless received status as now.
                __launchpad_exec_dummy(launchpad_fd, pool_server_fd, -1);
            }
        }

        if ((pfds[POOL_CLIENT].revents & (POLLHUP|POLLNVAL)) != 0)
        {
            // this is executed when dummy process disconnects normally or abnormally
            _D("pfds[POOL_CLIENT].revents & (POLLHUP|POLLNVAL) (pid:%d)", pool_client_ui_pid);
            // clear resource of dummy process
            __launchpad_clear_dummy();
            //re-create dummy process
            __launchpad_exec_dummy(launchpad_fd, pool_server_fd, -1);
        }
    }

    return 0;

    exit_main:
    if (launchpad_fd != -1)
    {
        close(launchpad_fd);
    }

    if (pool_server_fd != -1)
    {
        close(pool_server_fd);
    }

    return -1;
}
