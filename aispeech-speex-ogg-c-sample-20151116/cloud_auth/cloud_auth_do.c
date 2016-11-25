#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "http.h"
#include <sha1.c>
#include <time.h>
#include <unistd.h>
#include <cJSON.h>
#include "base.h"

#define BUFFER_SIZE 1024
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"

static int http_tcpclient_create(const char *host, int port)
{
	struct hostent *he;
	struct sockaddr_in server_addr; 
	int socket_fd;

	if((he = gethostbyname(host)) == NULL){
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)he->h_addr);

	if((socket_fd = socket(AF_INET,SOCK_STREAM,0))==-1){
		return -1;
	}

	if(connect(socket_fd,
		(struct sockaddr *)&server_addr,
		sizeof(struct sockaddr)) == -1){
		return -1;
	}

	return socket_fd;
}

static void http_tcpclient_close(int socket)
{
	close(socket);
}

static int http_parse_url(const char *url,char *host,char *file,int *port)
{
	char *ptr1,*ptr2;
	int len = 0;
	if (!url || !host || !file || !port) {
		return -1;
	}

	ptr1 = (char *)url;

	if (!strncmp(ptr1,"http://",strlen("http://"))) {
		ptr1 += strlen("http://");
	}else{
		return -1;
	}

	ptr2 = strchr(ptr1,'/');
	if (ptr2){
		len = strlen(ptr1) - strlen(ptr2);
		memcpy(host,ptr1,len);
		host[len] = '\0';
		if(*(ptr2 + 1)){
			memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );
			file[strlen(ptr2) - 1] = '\0';
		}
	} else {
		memcpy(host,ptr1,strlen(ptr1));
		host[strlen(ptr1)] = '\0';
	}
	//get host and ip
	ptr1 = strchr(host,':');
	if (ptr1) {
		*ptr1++ = '\0';
		*port = atoi(ptr1);
	} else {
		*port = MY_HTTP_DEFAULT_PORT;
	}

	return 0;
}

static int http_tcpclient_recv(int socket,char *lpbuff)
{
	int recvnum = 0;
	recvnum = recv(socket, lpbuff,BUFFER_SIZE*4,0);

	return recvnum;
}
static int http_tcpclient_send(int socket,char *buff,int size)
{
	int sent=0,tmpres=0;

	while (sent < size) {
		tmpres = send(socket,buff+sent,size-sent,0);
		if (tmpres == -1) {
			return -1;
		}
		sent += tmpres;
	}
	return sent;
}

static char *http_parse_result(const char*lpbuf)
{
	char *ptmp = NULL; 
	char *response = NULL;
	ptmp = (char*)strstr(lpbuf,"HTTP/1.1");
	if (!ptmp) {
		printf("http/1.1 not faind\n");
		return NULL;
	}
	if (atoi(ptmp + 9) != 200) {
		printf("result:\n%s\n",lpbuf);
		return NULL;
	}

	ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
	if (!ptmp) {
		printf("ptmp is NULL\n");
		return NULL;
	}
	response = (char *)malloc(strlen(ptmp)+1);
	if (!response) {
		printf("malloc failed \n");
		return NULL;
	}
	strcpy(response,ptmp+4);
	return response;
}

char * _http_get(const char *url)
{
	int socket_fd = -1;
	char lpbuf[BUFFER_SIZE*4] = {'\0'};
	char host_addr[BUFFER_SIZE] = {'\0'};
	char file[BUFFER_SIZE] = {'\0'};
	int port = 0;

	if(!url){
		printf("      failed!\n");
		return NULL;
	}

	if(http_parse_url(url,host_addr,file,&port)){
		printf("http_parse_url failed!\n");
		return NULL;
	}

	socket_fd =  http_tcpclient_create(host_addr,port);
	if(socket_fd < 0){
		printf("http_tcpclient_create failed\n");
		return NULL;
	}

	sprintf(lpbuf,HTTP_GET,file,host_addr,port);

	if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf)) < 0){
		printf("http_tcpclient_send failed..\n");
		return NULL;
	}

	if(http_tcpclient_recv(socket_fd,lpbuf) <= 0){
		printf("http_tcpclient_recv failed\n");
		return NULL;
	}
	http_tcpclient_close(socket_fd);

	return http_parse_result(lpbuf);
}

int cloud_auth_do(const char *cfg)
{
	cJSON *root = cJSON_Parse(cfg);
	cJSON *tmp = cJSON_GetObjectItem(root, "appKey");
	char *appKey = tmp->valuestring;

	tmp = cJSON_GetObjectItem(root, "secretKey");
	char *secretKey = tmp->valuestring;

	char *sig_in = malloc(80);
	memset(sig_in, 0, 80);
	int size;

	char *deviceId = "8c705ac0a268";
	/*XXX: maybe need the system time*/
	char timestamp[12] = {0};
	int sec= time(NULL);
	itoa(sec, timestamp);
	//printf("sec: %d, timestamp: %s\n", sec, timestamp);
	//sprintf(timestamp, "%d", (int)rand());

	memset(sig_in, 0, sizeof(1024));
	sprintf(sig_in, "%s%s%s%s", appKey, timestamp, secretKey, deviceId);
	char sig[41] = {0};
	memset(sig, 0, sizeof(sig));
	size = strlen(sig_in);

	sha1(sig_in, size, sig);
	char url[1024];
	memset(url, 0, sizeof(url));
	sprintf(url,
		"http://%s?appKey=%s&timestamp=%s&deviceId=%s&userId=MXCHIP&sig=%s",
		"auth.api.aispeech.com/device",
		appKey,
		timestamp,
		deviceId,
		sig);
	printf("%s\n", url);
	char *str = _http_get(url);
	printf("return:%s\n", str);
	free(str);
	free(sig_in);
	cJSON_Delete(root);
	return 0;
}
