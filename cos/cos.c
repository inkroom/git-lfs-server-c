// #include "batch.h"
#include "http.h"
#include "cos_api.h"
#include "cos_auth.h"
#include "cos_http_io.h"
#include "cos_define.h"
#include "cos_string.h"

static cos_pool_t *p = NULL;
cos_request_options_t *init()
{

    int is_cname = 0;
    cos_request_options_t *options = NULL;
    printf("1\n");
    if (p == NULL)
    {

        if (cos_http_io_initialize(NULL, 0) != COSE_OK)
        {
            exit(1);
        }
        // create memory pool
        cos_pool_create(&p, NULL);
    }

    printf("2\n");

    // init request options
    options = cos_request_options_create(p);
    printf("3\n");

    options->config = cos_config_create(options->pool);
    printf("4\n");

    cos_str_set(&options->config->endpoint, getenv("COS_ENDPOINT"));
    printf("5\n");

    printf("key %s\n", getenv("COS_KEY_ID"));
    cos_str_set(&options->config->access_key_id, getenv("COS_KEY_ID"));
    printf("6\n");

    cos_str_set(&options->config->access_key_secret, getenv("COS_KEY_SECRET"));
    printf("7\n");

    cos_str_set(&options->config->appid, getenv("COS_APP_ID"));
    printf("8\n");

    options->config->is_cname = is_cname;
    options->ctl = cos_http_controller_create(options->pool, 0);

    return options;
}

cos_string_t _cos_sign(char *oid)
{
    cos_request_options_t *options = init();
    cos_string_t bucket;
    cos_string_t object;

    cos_str_set(&bucket, getenv("COS_BUCKET"));

    cos_str_set(&object, oid);
    printf("11\n");

    cos_string_t url;
    printf("12\n");

    cos_gen_presigned_url(options, &bucket, &object, 3600, HTTP_PUT, &url);
    printf("13\n");

    // destroy memory pool
    // cos_pool_destroy(p);
    printf("14\n");

    return url;
}

cos_string_t _cos_get(char *oid)
{
    cos_request_options_t *options = init();
    cos_string_t bucket;
    cos_string_t object;
    printf("1\n");

    cos_str_set(&bucket, getenv("COS_BUCKET"));
    printf("10\n");

    cos_str_set(&object, oid);
    printf("11\n");

    cos_string_t url;
    printf("12\n");

    url.data = cos_gen_object_url(options, &bucket, &object);
    printf("13\n");
    url.len = strlen(url.data);
    // destroy memory pool
    // cos_pool_destroy(p);
    printf("14\n");

    return url;
}

void _batch_request_upload_for_each(FIOBJ item)
{

    FIOBJ authenticatedKey = fiobj_str_new("authenticated", strlen("authenticated"));
    FIOBJ oidKey = fiobj_str_new("oid", strlen("oid"));
    FIOBJ sizeKey = fiobj_str_new("size", strlen("size"));
    FIOBJ hrefKey = fiobj_str_new("href", strlen("href"));
    FIOBJ authenticated = fiobj_true();
    FIOBJ uploadKey = fiobj_str_new("upload", strlen("upload"));
    FIOBJ actionKey = fiobj_str_new("actions", strlen("actions"));

    fiobj_hash_set(item, authenticatedKey, authenticated);

    FIOBJ oid = fiobj_hash_get(item, oidKey);
    FIOBJ size = fiobj_hash_get(item, sizeKey);

    printf("upload 1");
    cos_string_t url = _cos_sign(fiobj_obj2cstr(oid).data);
    printf("upload 2");
    FIOBJ upload = fiobj_hash_new();

    printf("url=  %s length = %d\n", url.data, url.len);

    fiobj_hash_set(upload, hrefKey, fiobj_str_new(url.data, url.len));
    // fiobj_hash_set(download, hrefKey, fiobj_str_new("href", fiobj_str_new(strlen("href")));
    fiobj_hash_set(upload, hrefKey, fiobj_str_new(url.data, url.len));

    FIOBJ actions = fiobj_hash_new();
    fiobj_hash_set(actions, uploadKey, upload);
    fiobj_hash_set(item, actionKey, actions);
}

void _batch_request_download_for_each(FIOBJ item)
{
    FIOBJ authenticatedKey = fiobj_str_new("authenticated", strlen("authenticated"));
    FIOBJ oidKey = fiobj_str_new("oid", strlen("oid"));
    FIOBJ sizeKey = fiobj_str_new("size", strlen("size"));
    FIOBJ hrefKey = fiobj_str_new("href", strlen("href"));
    FIOBJ authenticated = fiobj_true();
    FIOBJ downloadKey = fiobj_str_new("download", strlen("download"));
    FIOBJ actionKey = fiobj_str_new("actions", strlen("actions"));

    fiobj_hash_set(item, authenticatedKey, authenticated);

    FIOBJ oid = fiobj_hash_get(item, oidKey);
    FIOBJ size = fiobj_hash_get(item, sizeKey);

    printf("download ");
    cos_string_t url = _cos_get(fiobj_obj2cstr(oid).data);

    FIOBJ download = fiobj_hash_new();

    printf("url=  %s length = %d\n", url.data, url.len);

    fiobj_hash_set(download, hrefKey, fiobj_str_new(url.data, url.len));
    // fiobj_hash_set(download, hrefKey, fiobj_str_new("href", fiobj_str_new(strlen("href")));
    fiobj_hash_set(download, hrefKey, fiobj_str_new(url.data, url.len));

    FIOBJ actions = fiobj_hash_new();
    fiobj_hash_set(actions, downloadKey, download);
    fiobj_hash_set(item, actionKey, actions);
}
