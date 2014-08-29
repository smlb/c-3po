#ifndef __CURLBOT_H
#define __CURLBOT_H

int html_title(char* link,char* title);
int is_html_link(char* msg);
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

#endif
