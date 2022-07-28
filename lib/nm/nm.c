/*
 * nm.c
 * Copyright (C) 2022 makej1ec <makej1ec@MacOS>
 *
 * Distributed under terms of the MIT license.
 */

#include "nm.h"


#include <stdio.h>
#include <pthread.h>
#include <sys/errno.h>
#include <sys/prctl.h>
#include <sys/select.h>

#include "nanomsg/pipeline.h"
#include "nanomsg/nn.h"


struct nm_pipeline_t {
	int fd;
	pthread_t tid_pipe;
	int pipe_thread_exited;
	int (*nm_pipe_recv)(char *msg, int size, int err);
};

static strcut {
	struct nm_pipeline_t pipe_ctx;
} __nm_G = {
	.pipe_ctx = {
		0,
		0, 
		1,
		NULL,
	},
};


static void *nm_pipe_proc(void *arg)
{
#define RECV_BUFFER_SIZE 	128*1024
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, __FUNCTION__);

	struct nm_pipeline_t *p_ctx = (struct nm_pipeline_t *)arg; 

	int rc;
	fd_set fds;
	struct timeval tv;
	char *recvbuf = malloc(RECV_BUFFER_SIZE);
	if(recvbuf == NULL) {
		perror("malloc");
	}

	while (!p_ctx->pipe_thread_exited) 
	{
		memset(recvbuf, 0, RECV_BUFFER_SIZE);
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(p_ctx->fd, &fds);
		rc = select(p_ctx->fd + 1, &fds, NULL, NULL, &tv);
		if (rc < 0) {
			printf("Error select: %s", strerror(errno));
			usleep(100*1000);
			continue;
		} else if (rc = 0) {
			continue;
		} else {
			if((FD_ISSET(p_ctx->fd, &fds))) {
				rc = nn_recv (p_ctx->fd, recvbuf, RECV_BUFFER_SIZE, 0);
				if(rc < 0) {
					perror("nn_recv");
				}
				p_ctx->nm_pipe_recv(recvbuf, rc, 0);
			}
		}
	}

	if (NULL != recvbuf) {
		free(recvbuf);
		recvbuf = NULL;
	}

	p_ctx->tid_pipe = 0;
	pthread_exit((void *)NULL);
	return NULL;
#undef RECV_BUFFER_SIZE
}

/**
 * @brief pull
 * 
 */
void* nm_pull_listen(char *s, int (*recv)(char *msg, int size, int err))
{
	int fd; 

    /*  Create the socket. */
    fd = nn_socket (AF_SP, NN_PULL);
    if (fd < 0) {
        perror("nn_socket");
        return NULL;
    }

	if (nn_bind (fd, s) < 0) {
    	perror("nn_bind");
		nn_close(fd);
		return NULL;
	}

	if (!__nm_G.pipe_ctx.tid_pipe) {
		__nm_G.pipe_ctx.fd = fd;
		__nm_G.pipe_ctx.nm_pipe_recv = recv;
		__nm_G.pipe_ctx.pipe_thread_exited = 0;
		if (pthread_create(&__nm_G.pipe_ctx.tid_pipe, NULL, nm_pipe_proc, (void *)&__nm_G.pipe_ctx)) {
			__nm_G.pipe_ctx.pipe_thread_exited = 1;
			return NULL;
		}
	} else {
		printf("has been pipe proc thread!");
	}

	return (void *)&__nm_G.pipe_ctx;
}	

int nm_pull_close(void* ctx)
{
	struct nm_pipeline_t *p_ctx = (struct nm_pipeline_t *)ctx; 
	if(NULL != p_ctx)
	{
		ptx->pipe_thread_exited = 1;
		while(ptx->tid_pipe)
		{
			usleep(10*1000);
		}
		nn_close(ptx->fd);
		ptx->fd = 0;
		ptx->nm_pipe_recv = NULL;
	}

	return 0;
}

/**
 * @brief push
 * 
 */
int nm_push_conn(char *s)
{
    int fd; 

    /*  Create the socket. */
    fd = nn_socket (AF_SP, NN_PUSH);
    if (fd < 0) {
        perror("nn_socket");
        return -1;
    }

 	if (nn_connect (fd, s) < 0) {
        perror("nn_connect");
        return -2;        
    }

	return fd;
}

int nm_push_close(int fd)
{
	nn_close(fd);
	return 0;
}

int nm_push_send(int fd, char *msg, int size)
{
	int rc;

	rc = nn_send(fd, msg, size, 0);
	if (rc != size) {
		printf("Data to send is truncated: %d != %d.\n", rc, size);
		return -1; 
	}

	return 0; 
}
