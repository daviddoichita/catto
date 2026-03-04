#define _GNU_SOURCE

#include <argp.h>
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CATAAS_URL "https://cataas.com"
#define ESCAPE(curl, input) curl_easy_escape(curl, input, strlen(input))

const char *argp_program_version = "catto 0.1";
const char *argp_program_bug_address = "<daviddoichita@proton.me>";

struct arguments {
  int help;
  char *tag;
  char *text;
} arguments;

static struct argp_option options[] = {
    {"help", 'h', 0, 0, "Display this help and exit", 0},
    {"tag", 't', "TAG", 0, "The tag to fetch", 0},
    {"text", 'T', "TEXT", 0, "The text the cat image will have", 0},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
  case 'h':
    arguments->help = 1;
    argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
    return 0;

  case 't':
    arguments->tag = arg;
    return 0;

  case 'T':
    arguments->text = arg;
    return 0;

  case ARGP_KEY_ARG:
    if (!arguments->tag) {
      arguments->tag = state->argv[state->next - 1];
      return 0;
    }
    argp_usage(state);
    return ARGP_ERR_UNKNOWN;

  case ARGP_KEY_END:
    return 0;

  default:
    return ARGP_ERR_UNKNOWN;
  }
}

static char args_doc[] = "";
static char doc[] = "catto -- fetch cat from " CATAAS_URL;

static struct argp argp = {options, parse_opt, args_doc, doc};

struct MemoryStruct {
  unsigned char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  unsigned char *ptr = realloc(mem->memory, mem->size + realsize);
  if (!ptr) {
    printf("realloc failed\n");
    return 0;
  }
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}

int main(int argc, char *argv[]) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();

  CURLcode res;
  struct MemoryStruct chunk;
  FILE *fp;
  char cmd[512];

  struct arguments args = {0};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  char url[512];
  if (args.tag && args.text) {
    snprintf(url, sizeof(url),
             CATAAS_URL "/cat/%s/says/%s", args.tag,
             ESCAPE(curl, args.text));
  } else if (args.tag) {
    snprintf(url, sizeof(url), CATAAS_URL "/cat/%s",
             args.tag);
  } else if (args.text) {
    snprintf(url, sizeof(url), CATAAS_URL "/cat/says/%s",
             ESCAPE(curl, args.text));
  } else {
    snprintf(url, sizeof(url), CATAAS_URL "/cat");
  }

  chunk.memory = malloc(1);
  chunk.size = 0;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cat-fetcher/1.0");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    } else {

      fp = fopen("cat.jpg", "wb");
      if (fp) {
        fwrite(chunk.memory, 1, chunk.size, fp);
        fclose(fp);
        snprintf(cmd, sizeof(cmd), "chafa --size=120x60 cat.jpg 2>/dev/null");
        int res = system(cmd);
        if (res != 0) {
          printf("Error fetching cat, tag `%s` might not exist or server could "
                 "not be reached\n",
                 args.tag);
          system("rm cat.jpg");
          free(chunk.memory);
          curl_global_cleanup();
          exit(1);
        }
      } else {
      }
    }
    curl_easy_cleanup(curl);
  }

  system("rm cat.jpg");
  free(chunk.memory);
  curl_global_cleanup();
  return 0;
}
