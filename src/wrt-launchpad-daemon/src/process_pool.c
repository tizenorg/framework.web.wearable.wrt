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
 * @file    process_pool.c
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version 0.1
 * @brief   process pool socket apis
 */

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/un.h>
#include <errno.h>
#include <stdio.h>
#include <systemd/sd-daemon.h>

#include "process_pool.h"
#include "simple_util.h"

#define TMP_PATH "/tmp"
#define PROCESS_POOL_SERVER "wrt_process_pool_server"
#define MAX_PENDING_CONNECTIONS 3
#define CONNECT_RETRY_TIME 100 * 1000
#define CONNECT_RETRY_COUNT 3

int __create_process_pool_server()
{
    struct sockaddr_un addr;
    int fd = -1;
    int listen_fds=0;
    int i;

    memset(&addr, 0x00, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, UNIX_PATH_MAX, "%s/%s", TMP_PATH, PROCESS_POOL_SERVER);

    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0)
    {
        _E("socket error");
        goto err_create_process_pool_server;
    }

    unlink(addr.sun_path);

    _D("bind to %s", addr.sun_path);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        _E("bind error");
        goto err_create_process_pool_server;
    }

    _D("chmod to %s", addr.sun_path);
    if (chmod(addr.sun_path, (S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
    {
        _E("chmod error");
        goto err_create_process_pool_server;
    }

    _D("listen to %s", addr.sun_path);
    if (listen(fd, MAX_PENDING_CONNECTIONS) == -1)
    {
        _E("listen error");
        goto err_create_process_pool_server;
    }

    _D("__create_process_pool_server done : %d", fd);
    return fd;


err_create_process_pool_server:

    if (fd != -1)
    {
        close(fd);
    }

    return -1;
}


int __connect_process_pool_server()
{
    struct sockaddr_un addr;
    int fd = -1;
    int retry = CONNECT_RETRY_COUNT;
    int send_ret = -1;
    int ui_pid = getpid();

    char pid_buff[6];
    size_t len = 5;
    char command[256] = {0};
    char payload[16] = {0};

    memset(&addr, 0x00, sizeof(struct sockaddr_un));

    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (fd < 0)
    {
        _E("socket error");

        goto err_connect_process_pool_server;
    }

    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, UNIX_PATH_MAX, "%s/%s", TMP_PATH, PROCESS_POOL_SERVER);

    // find child process of ui process and set it to payload
    sprintf(command, "/usr/bin/pgrep -P %d", ui_pid);
    FILE *fp = (FILE*) popen(command, "r");

    if( fp == NULL ){
        _E("popen error");
        goto err_connect_process_pool_server;
    }

    if(fgets(pid_buff, len, fp)) {
        if(!ferror(fp)) {
            _D("child pid: %s", pid_buff);
            sprintf(payload, "%7d %7s", ui_pid, pid_buff);
        } else {
            sprintf(payload, "%15d", ui_pid);
        }
    }
    pclose(fp);

    _D("connect to %s", addr.sun_path);
    while (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        if (errno != ETIMEDOUT || retry <= 0)
        {
            _E("connect error : %d", errno);

            goto err_connect_process_pool_server;
        }

        usleep(CONNECT_RETRY_TIME);
        retry--;
        _D("re-connect to %s (%d)", addr.sun_path, retry);
    }

    send_ret = send(fd, payload, strlen(payload) + 1, 0);
    _D("send(payload: %s) : %d", payload, send_ret);

    if (send_ret == -1)
    {
        _E("send error");

        goto err_connect_process_pool_server;
    }

    _D("__connect_process_pool_server done : %d", fd);
    return fd;

err_connect_process_pool_server:

    if (fd != -1)
    {
        close(fd);
    }

    return -1;
}


int __accept_process_pool_client(
        int server_fd, int* out_client_ui_fd,
        int* out_client_ui_pid, int* out_client_web_pid)
{
    int client_ui_fd = -1, client_ui_pid = 0, client_web_pid = 0, recv_ret = 0;

    char* pid;
    char payload[17] = {0};

    if (server_fd == -1 || out_client_ui_fd == NULL || out_client_ui_pid == NULL)
    {
        _E("arguments error!");

        goto err__accept_process_pool_client;
    }

    client_ui_fd = accept(server_fd, NULL, NULL);

    if (client_ui_fd == -1)
    {
        _E("accept error!");

        goto err__accept_process_pool_client;
    }

    recv_ret = recv(client_ui_fd, payload, 16, MSG_WAITALL);

    if (recv_ret == -1)
    {
        _E("recv error!");

        goto err__accept_process_pool_client;
    }

    // parse payload and get pid of ui process and web process 
    pid = strtok(payload, " ");
    if (!pid) {
        // this case would have only pid of ui process 
        *out_client_ui_pid = atoi(payload);
        *out_client_web_pid = 0;
    } else {
        *out_client_ui_pid = atoi(pid);
        pid = strtok(NULL, "\0");
        if (pid == NULL) {
            *out_client_web_pid = 0;
        } else {
            *out_client_web_pid = atoi(pid);
        }
    }
    _D("pool client is connected! (ui pid: %d, web pid: %d)",
            *out_client_ui_pid, *out_client_web_pid);

    *out_client_ui_fd = client_ui_fd;
    return *out_client_ui_fd;

err__accept_process_pool_client:

    if (client_ui_fd != -1)
    {
        close(client_ui_fd);
    }

    return -1;
}

void __refuse_process_pool_client(int server_fd)
{
    int client_fd = -1;

    if (server_fd == -1)
    {
        _E("arguments error!");

        goto err__refuse_process_pool_client;
    }

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1)
    {
        _E("accept error!");

        goto err__refuse_process_pool_client;;
    }

    close(client_fd);
    _D("refuse connection!");

    err__refuse_process_pool_client:

    return;

}


int __send_pkt_raw_data(int client_fd, app_pkt_t* pkt)
{
    int send_ret = 0;
    int pkt_size = 0;

    if (client_fd == -1 || pkt == NULL)
    {
        _E("arguments error!");

        goto err__send_pkt_raw_data;
    }

    pkt_size = sizeof(pkt->cmd) + sizeof(pkt->len) + pkt->len;

    send_ret = send(client_fd, pkt, pkt_size, 0);
    _D("send(%d) : %d / %d", client_fd, send_ret, pkt_size);

    if (send_ret == -1)
    {
        _E("send error!");

        goto err__send_pkt_raw_data;
    }
    else if (send_ret != pkt_size)
    {
        _E("send byte fail!");

        goto err__send_pkt_raw_data;
    }

    return 0;

err__send_pkt_raw_data:

    return -1;
}

int __set_background_cgroup(int flag, int ui_pid, int web_pid)
{
    int i, fd;
    char* filename;
    char pid_buff[8];
    int pids[2];

    if (ui_pid <= 0) {
        return -1;
    }

    pids[0] = ui_pid;
    pids[1] = web_pid;

    if (flag) {
        filename = "/sys/fs/cgroup/cpu/background/cgroup.procs";
    } else {
        filename = "/sys/fs/cgroup/cpu/cgroup.procs";
    }

    fd = open(filename, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        _D("failed to open sysfs");
        return -1;
    }

    for (i = 0; i < 2; i++) {
        if (!pids[i]) {
            continue;
        }

        sprintf(pid_buff, "%d", pids[i]);
        if (write(fd, pid_buff, 8) < 0) {
            _D("failed to write '%s' (%s); policy=%d", pid_buff, strerror(errno), flag);
            continue;
        }
    }

    close(fd);
    return 0;
}
