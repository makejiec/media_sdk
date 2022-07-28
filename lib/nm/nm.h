/*
 * nm.h
 * Copyright (C) 2022 makej1ec <makej1ec@MacOS>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __NM_H__
#define __NM_H__

#ifdef __cplusplus
extern "C" {
#endif 

void* nm_pull_listen(char *s, int (*recv)(char *msg, int size, int err));
int nm_pull_close(void* ctx);

int nm_push_conn(char *s);
int nm_push_close(int fd);
int nm_push_send(int fd, char *msg, int size);

#ifdef __cplusplus
}
#endif

#endif /* !__NM_H__ */
