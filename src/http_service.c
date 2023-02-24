#include "fio_cli.h"
#include "main.h"
#include "stdio.h"
#include "batch.h"
/* TODO: edit this function to handle HTTP data and answer Websocket requests.*/
static void on_http_request(http_s *h)
{
  /* set a response and send it (finnish vs. destroy). */
  printf("test\n");

  fio_str_info_s path = fiobj_obj2cstr(h->path);

  fio_str_info_s method = fiobj_obj2cstr(h->method);

  fio_str_info_s body = fiobj_obj2cstr(h->body);

  printf("body type = %s\n", fiobj_type_name(h->body));

  if (strcmp(path.data, LFS_BATCH_URL_PATH) == 0)
  {
    batch_request(h);
  }
  else
  {

    if (strcmp(method.data, "GET") == 0)
    {
      printf("this is GET request\n");
    }

    printf("path = %s \n", path.data);
    printf("method = %s \n", method.data);

    http_send_body(h, "helli World!", 12);
  }
}

/* starts a listeninng socket for HTTP connections. */
void initialize_http_service(void)
{
  /* listen for inncoming connections */
  if (http_listen(fio_cli_get("-p"), fio_cli_get("-b"),
                  .on_request = on_http_request,
                  .max_body_size = fio_cli_get_i("-maxbd") * 1024 * 1024,
                  .ws_max_msg_size = fio_cli_get_i("-max-msg") * 1024,
                  .public_folder = fio_cli_get("-public"),
                  .log = fio_cli_get_bool("-log"),
                  .timeout = fio_cli_get_i("-keep-alive"),
                  .ws_timeout = fio_cli_get_i("-ping")) == -1)
  {
    /* listen failed ?*/
    perror("ERROR: facil couldn't initialize HTTP service (already running?)");
    exit(1);
  }
}
