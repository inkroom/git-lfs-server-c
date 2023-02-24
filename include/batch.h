#include "http.h"

#ifndef INK_LFS_BATCH_H
#define INK_LFS_BATCH_H

#ifndef LFS_BATCH_URL_PATH

#define LFS_BATCH_URL_PATH "/objects/batch"

#endif

#ifndef LFS_HTTP_CONTENT_TYPE

#define LFS_HTTP_CONTENT_TYPE "application/vnd.git-lfs+json"

#endif

typedef void (*lfs_item_each)(FIOBJ);

void batch_request(http_s *h);


#endif
