/*
 * File      : Swy_nbapp.h
 * 
 * COPYRIGHT (C) 2020 - 2028, Soway Development Team
 *
 *  28 longguang Street, first Floor, China.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-3-13      frank     first version
 */

#ifndef __Swy_nbapp_H__
#define __Swy_nbapp_H__
#ifdef __cplusplus
extern "C" {
#endif

#define  Swy_tcp                       0
#define  Swy_udp                       0
#define  Swy_mqtt                      1

#if (Swy_tcp == 1)
#define  Swyurl "121.41.12.44" //"220.180.239.212"
#define  Swyport 6800  //8164  
//#define  Swyurl "220.180.239.212" 
//#define  Swyport 8164   

#elif (Swy_udp == 1) //udp  192.53.100.53,5683 121.41.12.44,7000 
#define  Swyurl "121.41.12.44"  //"192.53.100.53" 
#define  Swyport   7000
#elif (Swy_mqtt == 1) //mqtt  "southbound.quectel.com" "47.100.63.174" 1883
#define  Swyurl "47.100.63.174" 
#define  Swyport 1883 
#endif

#define  Swy_SOCK_BUFFER_LEN          256  
#define  Swy_SOCK_READ_BUFFER_LEN     512 

typedef enum
{
	SUCCESS_REC = 0,
	TIME_OUT,
	NO_REC
}teATStatus;

typedef struct
{
	char *ATSendStr;   //发送字符串
//	char *ATRecStr;    //接收字符串
//	uint16_t  TimeOut;	//超时变量
//	teATStatus ATStatus;//当前状态	
//	uint8_t  RtyNum;		//重发
}tsATCmd;

typedef enum
{
	AT_QMTOPEN = 0,
	AT_QMTCONN,
	AT_QMTSUB,
	AT_QMTPUB,
	AT_QIOPEN,
	AT_QISEND,
}teATCmdNum;
	
typedef struct
{
	char mqtt_open;
	char mqtt_conn;
	char mqtt_sub;
	char state;
}Swy_mqtt_info_t;
	
typedef struct
{
	unsigned char send_data[256];
	int send_size;
	char state;
	
}Swy_tcp_info_t;

typedef struct
{
	unsigned int upload_time;
	
}Swy_save_info_t;

void Swy_recv_data_handle(const char *data, int len);
rt_int8_t SywAppTcpConnectToServer(const char* host, rt_uint16_t port);
int Swy_nb_send(const       unsigned char *sendbuf,int n);
extern int ctrl_bc26_device_ops(const char *cmd);
int thread_sample(void);
#ifdef __cplusplus
}
#endif

#endif
