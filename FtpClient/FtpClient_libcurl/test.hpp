#include <stdlib.h>
#include <stdio.h>

#include "curl/curl.h"
#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif
 
/* <DESC>
 * Checks a single file's size and mtime from an FTP server.
 * </DESC>
 */ 
 
static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
  (void)ptr;
  (void)data;
  /* we are not interested in the headers itself,
     so we only return the size we would have saved ... */ 
  return (size_t)(size * nmemb);
}
 
int test_main(void)
{
	char ftpurl[] = "ftp://127.0.0.1/ThreadPool_WinApi/ÖÐÎÄ.flv";
  CURL *curl;
  CURLcode res;
  long filetime = -1;
  double filesize = 0.0;
  const char *filename = strrchr(ftpurl, '/') + 1;
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, ftpurl);
	curl_easy_setopt(curl, CURLOPT_USERPWD, "kinkoo:6787.");
    /* No download if the file */ 
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    /* Ask for filetime */ 
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
    /* No header output: TODO 14.1 http-style HEAD output for ftp */ 
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, throw_away);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    /* Switch on full protocol/debug output */ 
    /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */ 
 
    res = curl_easy_perform(curl);
 
    if(CURLE_OK == res) {
      /* https://curl.haxx.se/libcurl/c/curl_easy_getinfo.html */ 
      res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
      if((CURLE_OK == res) && (filetime >= 0)) {
        time_t file_time = (time_t)filetime;
        printf("filetime %s: %s", filename, ctime(&file_time));
      }
      res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                              &filesize);
      if((CURLE_OK == res) && (filesize>0.0))
        printf("filesize %s: %0.0f bytes\n", filename, filesize);
    }
    else {
      /* we failed */ 
      fprintf(stderr, "curl told us %d\n", res);
    }
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
 
  return 0;
}