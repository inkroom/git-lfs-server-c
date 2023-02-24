#include "http.h"
#include "batch.h"
#include "cos_api.h"
#include "cos_auth.h"
#include "cos_http_io.h"
#include "cos_define.h"
#include "cos_string.h"
#include "base64.h"

typedef struct
{
    char *username;
    char *password;
} BasicAuth, *p_BasicAuth;

static cos_pool_t *p = NULL;
cos_request_options_t *init()
{

    int is_cname = 0;
    cos_request_options_t *options = NULL;
    printf("1\n");
    if (p == NULL)
    {
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

void _batch_request(http_s *h, FIOBJ jsonBody, lfs_item_each each)
{

    printf("request 1\n");
    FIOBJ objectsKey = fiobj_str_new("objects", strlen("objects"));
    FIOBJ objects = fiobj_hash_get(jsonBody, objectsKey);

    if (!fiobj_type_is(objects, FIOBJ_T_ARRAY))
    {
        printf("not allowed json body ");

        fio_free(objects);
        fio_free(objectsKey);

        return;
    }
    printf("request 2\n");

    // 构建要返回的数据结构
    FIOBJ res = fiobj_hash_new2(3);

    fiobj_hash_set(res, fiobj_str_new("transfer", strlen("transfer")), fiobj_str_new("basic", strlen("basic")));
    fiobj_hash_set(res, fiobj_str_new("hash_algo", strlen("hash_algo")), fiobj_str_new("sha256", strlen("sha256")));

    size_t count = fiobj_ary_count(objects);
    printf("request 3");

    // FIOBJ authenticated = fiobj_str_new("true", strlen("true"));

    int i = 0;
    for (i = 0; i < count; i++)
    {
        FIOBJ item = fiobj_ary_index(objects, i);
        printf("request 4\n");

        each(item);
        printf("request 5\n");

        // fio_free(item);
    }

    fiobj_hash_set(res, objectsKey, objects);
    fio_str_info_s res_str = fiobj_obj2cstr(fiobj_obj2json(res, 1));

    printf("res %s\n", res_str.data);
    int r = http_set_header(h, fiobj_str_new("Content-Type", strlen("Content-Type")), fiobj_str_new("application/vnd.git-lfs+json", strlen("application/vnd.git-lfs+json")));
    printf("set content type %d\n", r);
    http_send_body(h, res_str.data, res_str.len);

    // fio_free(oidKey);
    // fio_free(sizeKey);
    // fio_free(objects);
    // fio_free(objectsKey);
}

/**
 * 字符串originString以字符串prefix开头，返回0；否则返回1；异常返回-1
 */
int lfs_str_startWith(const char *originString, char *prefix)
{
    // 参数校验
    if (originString == NULL || prefix == NULL || strlen(prefix) > strlen(originString))
    {
        printf("参数异常，请重新输入！\n");
        return -1;
    }

    int n = strlen(prefix);
    int i;
    for (i = 0; i < n; i++)
    {
        if (originString[i] != prefix[i])
        {
            return 1;
        }
    }
    return 0;
}
/**
 * 解码 basic认证，返回username和password
 * @param base64 包括 Basic在内，例如 Basic MTIzNDU2
 * @return 1 正常，0不正确
 */
p_BasicAuth encodeBasicAuth(char *base64)

{
    int len = strlen(base64);
    char * or = lfs_base64_decode(base64 + 6, len - 6);
    len = strlen(or);

    char *username = NULL;
    char *password = NULL;
    int i = 0;
    // printf("or =[%s]\n", or);
    for (i = 0; i < len; i++)
    {
        if (or [i] == ':')
        {
            username = (char *)malloc(sizeof(char) * (i + 1));
            // 截取 username
            strncpy(username, or, i);
            username[i] = '\0';
            // printf("username = [%s]\n", username);
            break;
        }
    }

    if (i == len)
    {
        free(username);
        return NULL;
    }

    // 截取password
    password = (char *)malloc(sizeof(char) * (len - i + 1));

    strncpy(password, or +i + 1, len - i);
    password[len - i] = '\0';
    // printf("password = [%s]\n", password);

    free(or);

    p_BasicAuth out = (p_BasicAuth)malloc(sizeof(BasicAuth));
    out->username = username;
    out->password = password;
    return out;
}

void batch_request(http_s *h)
{

    // 不允许的数据
    if (fiobj_type_is(h->body, FIOBJ_T_NULL))
    {
        printf("null body not allowed\n");
        return;
    }

    FIOBJ headers = h->headers;

    FIOBJ contentTypeKey = fiobj_str_new("content-type", strlen("Content-Type"));

    FIOBJ contentType = fiobj_hash_get(headers, contentTypeKey);

    fiobj_free(contentTypeKey);

    // if (!fiobj_type_is(contentType, FIOBJ_T_STRING))
    // {
    //     printf("not allowed contentType %s\n", fiobj_type_name(contentType));
    //     return;
    // }
    // fio_str_info_s c = fiobj_obj2cstr(contentType);

    // printf("content type = %s\n", c.data);

    // if (strcmp(c.data, LFS_HTTP_CONTENT_TYPE) != 0)
    // {
    //     printf("2 not allowed contentType %s\n", c.data);
    //     return;
    // }

    // FIOBJ jsonBody = fiobj_obj2json(h->body, 1);

    fio_str_info_s body = fiobj_obj2cstr(h->body);

    printf("body = %s\n", body.data);

    FIOBJ jsonBody = FIOBJ_INVALID;
    size_t consumed = fiobj_json2obj(&jsonBody, body.data, body.len);
    // test for errors
    if (!consumed || !jsonBody)
    {
        printf("\nERROR, couldn't parse data.\n");
        exit(-1);
    }

    FIOBJ operationKey = fiobj_str_new("operation", strlen("operation"));

    FIOBJ operation = fiobj_hash_get(jsonBody, operationKey);
    fio_str_info_s op = fiobj_obj2cstr(operation);
    printf("operation = %s\n", op.data);
    if (strcmp(op.data, "upload") == 0)
    {

        // 检查认证
        FIOBJ authorizationKey = fiobj_str_new("authorization", strlen("authorization"));
        FIOBJ authorization = fiobj_hash_get(h->headers, authorizationKey);
        if (!fiobj_type_is(authorization, FIOBJ_T_STRING))
        {
            int r = http_set_header(h, fiobj_str_new("WWW-Authenticate", strlen("WWW-Authenticate")), fiobj_str_new("Basic", strlen("Basic")));

            http_send_error(h, 401);
        }
        else
        {

            fio_str_info_s s = fiobj_obj2cstr(authorization);
            printf("au =[%s]\n", s.data);
            if (lfs_str_startWith(s.data, "Basic ") != 0 || s.len <= 6)
            {
                int r = http_set_header(h, fiobj_str_new("WWW-Authenticate", strlen("WWW-Authenticate")), fiobj_str_new("Basic", strlen("Basic")));

                http_send_error(h, 401);
                return;
            }

            p_BasicAuth auth = encodeBasicAuth(s.data);
            if (auth == NULL || strcmp(auth->username, getenv("LFS_USERNAME")) != 0 || strcmp(auth->password, getenv("LFS_PASSWORD")) != 0)
            {
                http_send_error(h, 403);
                return;
            }

            // free(or);
            // or = NULL;

            _batch_request(h, jsonBody, _batch_request_upload_for_each);
        }
    }
    else if (strcmp(op.data, "download") == 0)
    {
        _batch_request(h, jsonBody, _batch_request_download_for_each);
    }
    // fiobj_free(operationKey);
    // fiobj_free(operation);
    // fiobj_free(jsonBody);
}
