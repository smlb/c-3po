#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <regex.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

int html_title(char* link,char* title){
	regex_t regex;
	size_t nmatch = 1;                                                        
  regmatch_t pmatch[1];
  int reti;
  CURL *curl_handle;
  CURLcode res;
  
  struct MemoryStruct chunk;
 
  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, title);

  /* no progress meter please */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION,1);

    /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */ 
		reti = regcomp(&regex, "<title>.*</title>", 0);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }
    
    printf("%lu bytes retrieved\n", (long)chunk.size);
    
    reti = regexec(&regex, chunk.memory, nmatch, pmatch, 0);
    if( !reti ){ 
			title = (char*)malloc(pmatch[0].rm_eo-8 - pmatch[0].rm_so-7);
			strncpy(title, &chunk.memory[pmatch[0].rm_so+7], pmatch[0].rm_eo-8 - pmatch[0].rm_so-7);
			printf("ok finale");
		}
    regfree(&regex);
  }
 
  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle);
 
  if(chunk.memory)
    free(chunk.memory);
 
  /* we're done with libcurl, so clean it up */ 
  curl_global_cleanup();
 
  return 0;
}

int is_html_link(char* msg){
	regex_t regex;
  int reti;	
  
  printf("ok1");
  
	reti = regcomp(&regex, "http://.*", 0);
	if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }
	
	reti = regexec(&regex, msg, 0, NULL, 0);
	if( !reti ){
		printf("no1");
		return 1;
	}
	regfree(&regex);
	printf("ok2");
	return 0;
}
