#include "main.h"
#include "stdio.h"
#include "cos_http_io.h"
#include "cos_api.h"
#include "cos_log.h"
#include "base64.h"
int main(int argc, char const *argv[])
{


  if (cos_http_io_initialize(NULL, 0) != COSE_OK)
  {
    exit(1);
  }

  printf("service started\n");

  /* accept command line arguments and setup default values, see "cli.c" */
  initialize_cli(argc, argv);

  /* initialize HTTP service, see "http_service.h" */
  initialize_http_service();

  /* start facil */
  fio_start(.threads = fio_cli_get_i("-t"), .workers = fio_cli_get_i("-w"));

  /* cleanup CLI, see "cli.c" */
  free_cli();
  cos_http_io_deinitialize();
  return 0;
}
