#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <agn_audioenc.h>
#include <nopoll.h>
#include <aiengine.h>
#include <pthread.h>
#include <tts.h>
#include <stdlib.h>
#include "timer.h"
#include "q.h"
#include "capture.h"
#include "ipcm_api.h"
#include "cJSON.h"
#if 0
	s-test.api.aispeech.com:10000

mico:
	"error":"appKey has exceeded the expired date"
	appKey: 14709983278595d8
	secretKey: 85d1e668eace0ce6539c299aa02b2334

#endif

char *url_strip(char *url)
{
	int len;
	char *cp;
	int i, j = 0;
	if (url == NULL) {
		printf("url error\n");
		return NULL;
	}
	len = strlen(url);
	cp = malloc(len+1);
	if (cp == NULL) {
		printf("malloc error\n");
		return NULL;
	}
	memset(cp, 0, len + 1);
	for (i = 0; i < len; i++) {
		if (url[i] == '\\')
			i++;
		cp[j] = url[i];
		j++;
	}
	return cp;
}

char *fetch_url(unsigned char const *start, unsigned long length)
{
	int i;
	char *c = NULL;
	if ((start == NULL) || (length == 0)) {
		printf("start error\n");
		return NULL;
	}
	for (i = 0; i < (length - 5); i++) {
		if ((start[i] == 'u')
			&& start[i+1] == 'r'
			&& start[i+2] == 'l'
			&& start[i+3] == '"'
			&& start[i+4] == ':'
			&& start[i+5] == '"') {
			int j;
			c = malloc(length-i-5);
			if (c == NULL) {
				printf("%s %s %d\n", __FILE__, __func__, __LINE__);
				return NULL;
			}
			memset(c, 0, length-i-5);

			/*notice "url" is interrupt*/
			for (j = 0; j < length-i-5; j++) {
				if (start[i+6+j] == '"')
					break;
				c[j] = start[i+6+j];
			}
			if (c[j] != '"') {
				free(c);
				c = NULL;
				printf("\nurl error\n");
			}
			break;
		}
	}
	return c;
}

char *fetch_output(unsigned char const *start, unsigned long length)
{
	static int z;
	int i;
	char *c = NULL;
	if ((start == NULL) || (length == 0)) {
		printf("start error\n");
		return NULL;
	}
	printf("\n");
	for (i = 0; i < (length - 8); i++) {
		if ((start[i] == 'o')
			&& start[i+1] == 'u'
			&& start[i+2] == 't'
			&& start[i+3] == 'p'
			&& start[i+4] == 'u'
			&& start[i+5] == 't'
			&& start[i+6] == '"'
			&& start[i+7] == ':'
			&& start[i+8] == '"') {
			int j;
			c = malloc(length-i-8);
			if (c == NULL) {
				printf("%s %s %d\n", __FILE__, __func__, __LINE__);
				return NULL;
			}
			memset(c, 0, length-i-8);

			/*notice "output" is interrupt*/
			for (j = 0; j < length-i-8; j++) {
				if (start[i+9+j] == '"')
					break;
				c[j] = start[i+9+j];
			}
		}
	}
	return c;
}

#if 0
/*ingenic*/
char *server_cfg = "{\
	\"appKey\": \"14327742440003c5\",\
	\"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
	\"provision\": \"auth/config.json\",\
	\"serialNumber\": \"bin/serialNumber\", \
	\"audiotype\": \"ogg\",\
	\"coretype\": \"cn.dlg.ita\",\
	\"res\": \"aihome\",\
	\"app\": {\
		\"userId\": \"wifiBox\"\
    	},\
	\"cloud\": {\
		\"server\": \"112.80.39.95\",\
		\"port\": \"8009\"\
	}\
}";

char *cloud_asr_param = "{\
	\"coreProvideType\": \"cloud\",\
	\"audio\": {\
	    \"audioType\": \"ogg\",\
	    \"sampleRate\": 16000,\
	    \"channel\": 1,\
	    \"compress\":\"raw\",\
	    \"sampleBytes\": 2\
	},\
	\"request\": {\
		\"coreType\": \"cn.dlg.ita\",\
		\"speechRate\":1.0,\
		\"res\": \"aihome\"\
	}\
}";
/*
rtt:
	appKey\": \"14796952588595df\",\
	ecretKey\": \"1cd1349a6ad1fe31de37ad4a9005f626\",\

english engine:
		\"server\": \"192.168.3.27\",\
		\"port\": \"8080\"\
*/
#endif

#if 0
char *server_cfg = "{\
	\"appKey\": \"14796952588595df\",\
	\"secretKey\": \"1cd1349a6ad1fe31de37ad4a9005f626\",\
	\"provision\": \"auth/config.json\",\
	\"serialNumber\": \"bin/serialNumber\", \
	\"audiotype\": \"pcm\",\
	\"coretype\": \"cn.asr.rec\",\
	\"res\": \"english\",\
	\"app\": {\
		\"userId\": \"wifiBox\"\
	},\
	\"cloud\": {\
		\"server\": \"s-test.api.aispeech.com\",\
		\"port\": \"10000\"\
	}\
}";

char *server_cfg = "{\
	\"appKey\": \"15111705248596ee\",\
	\"secretKey\": \"6f85495917a9b5da336cc55b43d0c71e\",\
	\"provision\": \"auth/config.json\",\
	\"serialNumber\": \"bin/serialNumber\", \
	\"audiotype\": \"pcm\",\
	\"coretype\": \"cn.sds\",\
	\"res\": \"airobot\",\
	\"app\": {\
		\"userId\": \"wifBox\"\
	},\
	\"cloud\": {\
		\"server\": \"s-test.api.aispeech.com\",\
		\"port\": \"10000\"\
	}\
}";

char *cloud_asr_param = "{\
	\"coreProvideType\": \"cloud\",\
	\"audio\": {\
	    \"audioType\": \"ogg\",\
	    \"sampleRate\": 16000,\
	    \"channel\": 1,\
	    \"compress\":\"raw\",\
	    \"sampleBytes\": 2\
	},\
	\"request\": {\
		\"coreType\": \"cn.sds\",\
		\"speechRate\":1.0,\
		\"res\": \"airobot\"\
	},\
	\"sdsExpand\":{\
		\"prevdomain\":\"\",\
		\"lastServiceType\": \"cloud\"\
		}\
}";
#endif

#define MILLION 1000000
long clock_get()
{
	long ret;
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ret = t.tv_sec * MILLION + t.tv_nsec/1000;
	return ret;
}

long calcu(long start, long end)
{
	long r = (end - start)/1000;
	return r;
}

long start;
long end;

static FILE *wav;
struct aiengine *agn;

char *tts_synth_config =  "{\
    \"appKey\": \"1531109559458467\",\
    \"secretKey\": \"ec939d53c0a5105f0f5ca7d9ef13cfcc\",\
    \"userId\": \"wifiBox\",\
    \"coretype\": \"cn.sent.syn\",\
    \"cloud\": {\
        \"server\": \"s.api.aispeech.com\",\
        \"port\": \"1028\"\
    },\
    \"audio\": {\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"channel\": 1, \
        \"audioType\": \"mp3\"\
    },\
    \"request\": {\
        \"speechVolume\": 100, \
        \"speechRate\": 0.85, \
        \"rightMargin\": 5, \
        \"realBack\": 0, \
        \"res\": \"syn_chnsnt_qianranf\"\
    }\
}";

char *tts_param =  "{\
	\"appKey\": \"1531109559458467\",\
	\"secretKey\": \"ec939d53c0a5105f0f5ca7d9ef13cfcc\",\
	\"userId\": \"wifiBox\",\
	\"coretype\": \"cn.sent.syn\",\
	\"cloud\": {\
        	\"server\": \"s.api.aispeech.com\",\
        	\"port\": \"1028\"\
	},\
	\"audio\": {\
		\"sampleBytes\": 2,\
		\"sampleRate\": 16000,\
		\"channel\": 1, \
		\"audioType\": \"mp3\"\
	},\
	\"request\": {\
		\"speechVolume\": 50, \
		\"speechRate\": 0.85, \
		\"rightMargin\": 5, \
		\"realBack\": 0, \
        	\"res\": \"syn_chnsnt_qianranf\"\
	}\
}";

char *result_process(char *buf, int len)
{
	char *ret_url = NULL;
	char *url = fetch_url(buf, len);
	if (url != NULL) {
		ret_url = url_strip(url);
		free(url);
	} else {
		url = fetch_output(buf, len);
		if (url == NULL) {
			printf("\noutput null\n");
			goto end;
		}
		ret_url = tts_url_output(tts_param, url);
		if (ret_url == NULL) {
			printf("%d: error\n", __LINE__);
			while (1); }
		free(url);
	}
end:
	return ret_url;
}

char *fetch_key(const char *start, char *key, unsigned long length)
{
	char *p = strstr(start, key);
	if (p == NULL) return NULL;
	char *url = malloc(148);
	memset(url, 0, 148);
	int i = 0;
	int len = strlen(key);
	p = p + len + 3;
	/*TODO: add size judge*/
	while (!(p[i] == '"')) {
		url[i] = p[i];
		if (p[i] == "\0") break;
		i++;
		if (i == 147) break;
	}

	printf("[%s %s %d]%s\n", __FILE__, __func__, __LINE__, url);
	return url;
}

void player(char *os)
{
	char aaa[32000];
	char tmp[1024] = {0};
	snprintf(tmp, 1024, "wget %s -O tmp.wav", os);
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s\n", tmp);
	system(tmp);
	system("ffplay tmp.wav -autoexit -nodisp");
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< player over >>>>>>>>>>>>>>>>\n");
//	snd_pcm_drop(rec_obj.handle);  
}

int agn_cb(const void *usrdata,
		const void *message,
		int size)
{
	if (message == NULL) {
		printf("cb message NULL\n");
	}

	printf("message:%s, size:%d\n", (char *)message, size);
	int ttt = strlen((char *)message);
	end = clock_get();
	long t = calcu(start, end);
	printf("server return time interval: %ld milliseconds\n", t);
	int aispeech_len = size;
	char *g_url;
	aispeech_len = strlen(message);
	char os[1024] ={0};
	char ol[1024] ={0};
#if DUI
	if (aispeech_len != 0) {
		dui_result_process((char *)message, aispeech_len, os, ol);
		if (strlen(os) != 0) {
			player(os);
			//char *url = player_url(os);
			//printf("\nurl: %s\n", url);
			//free(url);
		}
	}
#endif

	return 0;
}

extern int cloud_auth_do(const char *cfg);

char *server_cfg = "{\
	\"appKey\": \"1531109559458467\",\
	\"secretKey\": \"ec939d53c0a5105f0f5ca7d9ef13cfcc\",\
	\"provision\": \"auth/config.json\",\
	\"serialNumber\": \"bin/serialNumber\", \
	\"audiotype\": \"ogg\",\
	\"coretype\": \"cn.asr.rec\",\
	\"res\": \"wifi\",\
	\"app\": {\
		\"userId\": \"wifi\"\
	},\
	\"cloud\": {\
		\"server\": \"s.api.aispeech.com\",\
		\"port\": \"1028\"\
	}\
}";

char *cloud_asr_param = "{\
	\"coreProvideType\": \"cloud\",\
	\"audio\": {\
	    \"audioType\": \"ogg\",\
	    \"sampleRate\": 16000,\
	    \"channel\": 1,\
	    \"compress\":\"raw\",\
	    \"sampleBytes\": 2\
	},\
	\"request\": {\
		\"coreType\": \"cn.sds\",\
		\"speechRate\":1.0,\
		\"res\": \"airobot\"\
	},\
	\"sdsExpand\":{\
		\"prevdomain\":\"\",\
		\"lastServiceType\": \"cloud\"\
		}\
}";

int speech()
{
	timer_reset("free_speech");
//	char *a = tran_output("EN", "zh-CHS", "hello world");
//	free(a);
//	a = tran_output("zh-CHS", "EN", "你好");
//	free(a);
//	return 0;
#if 0
	char *url = NULL;
	url = tts_url_output(tts_param,
			"今天");
	printf("\n%s\n", url);
	free(url);
#endif
#if 1
	int size;
	FILE *fp;
	char *buffer;
#if 0
	if ((fp = fopen("sound.pcm","w")) < 0)
		printf("open sound.pcm fial\n");
#endif
	printf("compile: %s\n", __TIME__);
	int i;

	for (i = 0; i < 1; i++) {
		record_init();
		size = rec_obj.frames * 2; /* 2 bytes/sample, 1 channels */  
		buffer = (char *) malloc(size);
		memset(buffer, 0, size);

		agn = aiengine_new(server_cfg);
		cloud_auth_do(server_cfg);
		if (agn == NULL) {
			pf("error\n");
			return 0;
		}
		printf(">>>>>>>>>>>>>>>record start<<<<<<<<<<<<<<<\n");
		aiengine_start(agn, cloud_asr_param, agn_cb, NULL);
		int loops = 1120;
		while (loops > 0) {
			loops--;
			int rc = snd_pcm_readi(rec_obj.handle, buffer, rec_obj.frames); 
			if (rc == -EPIPE) {  
				/* EPIPE means overrun */  
				fprintf(stderr, "overrun occurred/n");  
				snd_pcm_prepare(rec_obj.handle);  
			} else if (rc < 0) {  
				fprintf(stderr,  
						"error from read: %s/n",  
						snd_strerror(rc));  
			} else if (rc != (int)rec_obj.frames) {  
				fprintf(stderr, "short read, read %d frames/n", rc);  
			}  
			aiengine_feed(agn, buffer, rec_obj.frames);
#if 0
			rc = fwrite(buffer, 1, size, fp);  
			if (rc != size)  
				fprintf(stderr,  "short write: wrote %d bytes/n", rc);  
			if (i == 2) {
				fclose(fp); 
				return 0;
			}
#endif
			memset(buffer, 0, size);
		}
		record_deinit();
		printf(">>>>>>>>>>>>>>>record end<<<<<<<<<<<<<<<\n");
		start = clock_get();
		aiengine_stop(agn);
		sleep(1);
		aiengine_delete(agn);
		//return 0;
		free(buffer);
	}
#endif
#if 0
	char *to = tran_output("en", "zh", "COULD YOU HELP ME PLEASE");
	printf("tran: %s\n", to);
	char *top = fetch_key(to, "dst", strlen(to));
	printf("\n%s\n", top);
	free(to);
	free(top);
#endif
out:
	return 0;
}

enum {
	BAD_NETWORK = 0,
	BAD_MOTION = 1,
	CLOSE = 2,
	END
};

char *event[END] = {"BAD_NETWORK", "BAD_MOTION", "CLOSE"};
static queue_t *q_obj;
static char *music[4] ={
	"/home/jim/workspace/hero/aispeech/tts-tools/status1.mp3", 
	"/home/jim/workspace/hero/aispeech/tts-tools/status2.mp3",
	"/home/jim/workspace/hero/aispeech/tts-tools/status3.mp3",
	"/home/jim/workspace/hero/aispeech/tts-tools/status4.mp3",
};

int event_handle()
{
	int i;
	char buf[100] = {0};
	int retval;
	while (1) {
		retval = msg_get_buf(q_obj, buf, 100);
		if (retval == -1) {
			sleep(1);
			continue;
		}
		timer_reset("free_speech");
		for (i = 0; i < END; i++) {
			if (0 == strncmp(buf, event[i], strlen(event[i]))) {
				break;
			}
		}

		print("event number: %d,  %s\n", i, buf);

		switch (i) {
		case BAD_NETWORK:
			system("play /home/jim/workspace/hero/aispeech/tts-tools/badnetwork.mp3");
			print("\n");
		break;
		case BAD_MOTION:
			system("play /home/jim/workspace/hero/aispeech/tts-tools/badmotion.mp3");
			print("\n");
		break;
		case CLOSE:
			speech();
			print("\n");
		break;
		default:
			print("wrong info\n");
		break;
		}
		memset(buf, 0, 100);
		sleep(1);
	}
}

int herodisplay_cb(int fd, char *data, int len)
{
	printf("[%s %d]data: %.*s\n", __func__, __LINE__, len, data);
	cJSON *root = cJSON_Parse(data);
	cJSON *action = cJSON_GetObjectItem(root, "action");
	printf("action: %s\n", action->valuestring);
	msg_put_buf(q_obj, action->valuestring, strlen(action->valuestring));
	cJSON_Delete(root);

	return 0;
}

void free_speech(int arg)
{
	char *pub_str = "{\"action\":\"FREE\"}";
	int i;
	pub("herodisplay", pub_str, strlen(pub_str));
	char buf[100] = {0};
	i = time(NULL);
	i= i%4;
	memset(buf, 0, 100);
	snprintf(buf, 100, "play %s &", music[i]);
	//printf("\n>>>music: %s\n", buf);
	system(buf);
	timer_reset("free_speech");
}

int main(int argc, char *argv[])
{
	/*timer for free time speech*/
	timer_init();
	user_timer_create("free_speech", 30, free_speech);

	/*buffer queue*/
	msg_init(&q_obj, "speech", 1024 * 10);

	/*ipcm init*/
	mqtt_init();
	sub("herodisplay", herodisplay_cb);

	/*handle othe process message*/
	event_handle();

	/*never reach here*/
	while (1) {
		printf("end\n");
		sleep(1);
	};
	return 0;
}