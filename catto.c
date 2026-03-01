#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>


struct MemoryStruct {
	unsigned char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	unsigned char *ptr = realloc(mem->memory, mem->size + realsize);
	if(!ptr) {
		printf("realloc failed\n");
		return 0;
	}
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	return realsize;
}

int main() {
	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;
	FILE *fp;
	char cmd[512];

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://cataas.com/cat/cute?width=400&height=300");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "cat-fetcher/1.0");

		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {

			fp = fopen("cat.jpg", "wb");
			if (fp) {
				fwrite(chunk.memory, 1, chunk.size, fp);
				fclose(fp);
				snprintf(cmd, sizeof(cmd), "chafa --size=120x60 cat.jpg");
				system(cmd);
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
