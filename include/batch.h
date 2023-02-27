#include "http.h"

#ifndef INK_LFS_BATCH_H
#define INK_LFS_BATCH_H

#ifndef LFS_BATCH_URL_PATH

#define LFS_BATCH_URL_PATH "/objects/batch"

#endif

#ifndef LFS_HTTP_CONTENT_TYPE

#define LFS_HTTP_CONTENT_TYPE "application/vnd.git-lfs+json"

#endif


typedef struct
{
    char *username;
    char *password;
} BasicAuth, *p_BasicAuth;


#define BASIC_AUTH_FREE(auth) free(auth->username);\
    free(auth->password); \
    free(auth); \
    auth = NULL;


typedef void (*lfs_item_each)(FIOBJ);

void batch_request(http_s *h);

void _batch_request_upload_for_each(FIOBJ item);
void _batch_request_download_for_each(FIOBJ item);

#endif
