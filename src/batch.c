#include "http.h"
#include "batch.h"
#include "base64.h"

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
    FIOBJ transferKey = fiobj_str_new("transfer", strlen("transfer"));
    FIOBJ basic = fiobj_str_new("basic", strlen("basic"));

    FIOBJ hash_algo_key = fiobj_str_new("hash_algo", strlen("hash_algo"));
    FIOBJ sha256 = fiobj_str_new("sha256", strlen("sha256"));
    fiobj_hash_set(res, transferKey, basic);
    fiobj_hash_set(res, hash_algo_key, sha256);

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
    FIOBJ f = fiobj_obj2json(res, 1);
    fio_str_info_s res_str = fiobj_obj2cstr(f);
    fiobj_free(f);

    printf("res %s\n", res_str.data);
    FIOBJ contentTypeKey = fiobj_str_new("Content-Type", strlen("Content-Type"));
    FIOBJ contentType = fiobj_str_new("application/vnd.git-lfs+json", strlen("application/vnd.git-lfs+json"));
    int r = http_set_header(h, contentTypeKey, contentType);
    printf("set content type %d\n", r);
    http_send_body(h, res_str.data, res_str.len);

    fiobj_free(objects);
    fiobj_free(objectsKey);

    fiobj_free(contentTypeKey);
    fiobj_free(contentType);

    fiobj_free(hash_algo_key);
    fiobj_free(sha256);

    fiobj_free(transferKey);
    fiobj_free(basic);

    fiobj_free(res);

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

    // FIOBJ contentTypeKey = fiobj_str_new("content-type", strlen("Content-Type"));

    // FIOBJ contentType = fiobj_hash_get(headers, contentTypeKey);

    // fiobj_free(contentTypeKey);

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
        fiobj_free(authorizationKey);
        if (!fiobj_type_is(authorization, FIOBJ_T_STRING))
        {
            FIOBJ WWW_Authenticate_Key = fiobj_str_new("WWW-Authenticate", strlen("WWW-Authenticate"));
            FIOBJ basic = fiobj_str_new("Basic", strlen("Basic"));
            int r = http_set_header(h, WWW_Authenticate_Key, basic);

            http_send_error(h, 401);
            fiobj_free(WWW_Authenticate_Key);
            fiobj_free(basic);
        }
        else
        {

            fio_str_info_s s = fiobj_obj2cstr(authorization);
            printf("au =[%s]\n", s.data);
            if (lfs_str_startWith(s.data, "Basic ") != 0 || s.len <= 6)
            {
                FIOBJ WWW_Authenticate_Key = fiobj_str_new("WWW-Authenticate", strlen("WWW-Authenticate"));
                FIOBJ basic = fiobj_str_new("Basic", strlen("Basic"));
                int r = http_set_header(h, WWW_Authenticate_Key, basic);

                http_send_error(h, 401);
                fiobj_free(WWW_Authenticate_Key);
                fiobj_free(basic);
                fiobj_free(authorization);
                return;
            }

            p_BasicAuth auth = encodeBasicAuth(s.data);
            if (auth == NULL || strcmp(auth->username, getenv("LFS_USERNAME")) != 0 || strcmp(auth->password, getenv("LFS_PASSWORD")) != 0)
            {
                http_send_error(h, 403);
                BASIC_AUTH_FREE(auth);
                fiobj_free(authorization);
                return;
            }
            _batch_request(h, jsonBody, _batch_request_upload_for_each);

            BASIC_AUTH_FREE(auth);

            // free(auth->username);
            // free(auth->password);
            // free(auth);
            // auth = NULL;
            fiobj_free(authorization);
        }
    }
    else if (strcmp(op.data, "download") == 0)
    {
        _batch_request(h, jsonBody, _batch_request_download_for_each);
    }
    fiobj_free(operationKey);
    fiobj_free(operation);
    fiobj_free(jsonBody);
}
