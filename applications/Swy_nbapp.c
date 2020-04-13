/*
 * File      : Swy_nbapp.c
 * Copyright Statement:
 * The software may not be copied and the information
 * contained herein may not be used or disclosed except with the written
 * permission of Soway Inc. (C) 2020
 * COPYRIGHT (C) 2020 - 2028, Soway Development Team
 *
 * 28 longguang Street, first Floor, SZ China.
 *
 * Change Logs:
 * Date           Author       Notes
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
 * 2020-03-13     frank       first version
 * 2020-03-26     frank       add semaphore
=======
 * 2020-03-13     frankliu       first version
 * 2020-03-26     frankliu       add semaphore
>>>>>>> aace7902e864f9c74aed1993ae92280333b588ee
=======
 * 2020-03-13     frank       first version
 * 2020-03-26     frank       add semaphore
>>>>>>> 39a5045071fe2516152064d447481b2224e60205
=======
 * 2020-03-13     frank       first version
 * 2020-03-26     frank       add semaphore
>>>>>>> develop
 */

#include <rtthread.h>
#include <at_device_bc26.h>
#include <Swy_nbapp.h>
#include <at_socket.h>
#include <string.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

struct at_socket *sock = RT_NULL;
struct sockaddr_in server_adr;
rt_sem_t dynamic_sem;
/* 邮箱控制块 */
static struct rt_mailbox mb;
/* 用于放邮件的内存池 */
static char mb_pool[128];
static char mb_str[] = "already";

Swy_tcp_info_t _tcp_conn;

Swy_mqtt_info_t _mq_conn;
Swy_save_info_t g_swy_data;

rt_int8_t SywAppTcpConnectToServer(const char* host, rt_uint16_t port);
extern void change_upload_inter_time(int timeout);

#define mqaccount  "866971030521988"
#define mqpasword  "866971030521988"
#define mqsecret   "e938e6daf27427554a01022333b8c5c8"

tsATCmd ATCmds[] = 
{			
	{"AT+QMTOPEN=0,\"%s\",%d"},
	{"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\""}, //"866971030521988","866971030521988","e938e6daf27427554a01022333b8c5c8"
	{"AT+QMTSUB=0,1,\"%s\",%d"},	//"quec/866971030521988/down", 2
	{"AT+QMTPUB=%d,%d,%d,%d,\"%s\""}, //,0,0,0,0,"quec/866971030521988/up"
	//,%d,\"%s\",\"%s\",%d,0,1 {"AT+QIOPEN=1,0,"TCP","121.41.12.44",6800,0,1"}
	{"AT+QIOPEN=1,%d,\"%s\",\"%s\",%d,0,1"},
	{"AT+QISEND=%d,%d"},
};

void SwyAppReconnectServer(void)
{
	char    res,*hostname = NULL;
	// unsigned short   port = 0;

	at_closesocket(server_adr.sock);

	res =  SywAppTcpConnectToServer(Swyurl,Swyport);;
	if(res==0)
	{
		rt_kprintf("re_connect_ser success\n");
	} 
	//SwyAppSetConnectStartTime();
}
void Swy_big_endian_cpy(unsigned char * dest, const void *data, int size)
{
	unsigned char *src = (unsigned char *)data;

	if (dest == NULL || src == NULL || size < 1)
		return;
#ifndef WIN32
	if (size < 2)// || _is_big_endian == 1
	{
		memcpy(dest, src, size);
	}
	else
#endif
	{
		int i = 0;
		for (i=0; i<size; i++)
		{
			memcpy(dest, (src + i ), 1);
			dest ++;
		}
	}
}
/******  send to ser   ****/
int Swy_nb_send(const       unsigned char *sendbuf,int n)
{
  char sowaysen[6]={0x31,0x36,0x37,0x38,0x39,0x1a};
  int result=0;
	static int count=0;
	
#if defined(Swy_tcp) || defined(Swy_mqtt)
	result = at_send(server_adr.sock,sowaysen,6, 0);
#else
	result = at_sendto(server_adr.sock, sowaysen, strlen(sowaysen), 0,
						(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
#endif	
	if (result <0)
		{
		#ifdef Swy_mqtt
		 _mq_conn.mqtt_open=0;
	   _mq_conn.mqtt_conn=0;
	   _mq_conn.mqtt_sub=0;
		#endif
		 //SwyAppReconnectServer();
		}
	else
		{
		 ctrl_bc26_device_ops("sleep");
		}
	rt_kprintf("Swy_nb_send_result %d\n",result);
	return result ;
}
void Swy_recv_data_handle(const char *data, int len)
{
	unsigned char com, need_reply=0;
	unsigned short com_len;
	int cout = 0;
	unsigned char token[4] = {0x00,0x00,0x00,0x00};
	char *p=NULL,tempbut[12]= {0};
	rt_kprintf("Swy_recv_data_handle_data&len:%s %d \n", data,len);

#if (Swy_mqtt == 1)
	if(strstr(data,"+QMTOPEN: 0,0"))
	{
	 _mq_conn.mqtt_open = 1;
	}
	else if(strstr(data,"+QMTCONN: 0,0,0"))
	{
	  _mq_conn.mqtt_conn = 1;
	}
	else if(strstr(data,"+QMTSUB: 0,1,0,2"))
	{
	 _mq_conn.mqtt_sub = 1;
	 _mq_conn.state = AT_SOCKET_CONNECT;
	 rt_kprintf("_mqtt_conn.state %d\n",_mq_conn.state);
	}

#elif (Swy_tcp ==1 || Swy_udp ==1)
	if(strstr(data,"+QIOPEN: 0,0"))
	{
		_tcp_conn.state = AT_SOCKET_CONNECT;
	  rt_kprintf("_tcp_conn.state %d\n",_tcp_conn.state);
	}
#endif
	
	if (rt_strstr(data,"set_time"))//(strncmp(data, "set_time", strlen("set_time")) == 0)//
	{ 
	   p = strstr(data, "#");
		 p +=1;
		// data +=54;
		 Swy_big_endian_cpy(tempbut, p, 5);
		
		 g_swy_data.upload_time = atoi(tempbut);
		 if (g_swy_data.upload_time > 5000)
		 rt_kprintf("set upload_time success!%d\n",g_swy_data.upload_time);		
    p=NULL;
	} 
	p = rt_strstr( data,"AT+");	
	if (rt_strncmp( p,"AT+QPOWD=0",strlen("AT+QPOWD=0"))==0)
	{ 
		 rt_kprintf("AT+QPOWD=0 comin1!\n");
		
		 ctrl_bc26_device_ops("AT+QPOWD=0");
		 p=NULL;
	} 
//	Swy_big_endian_cpy((unsigned char *)&com_len, data+3, 2);// test frank
//	com = *(data +0);
//	if (com == 0x7E && len > 3)
//	{
//		com = *(data +12); 
//		rt_kprintf("com:%x", com);
//		/*第三字节为主信令*/
//		if (com == 0x81)
//		{    
//		    data+=33;
//			Swy_big_endian_cpy(token, data, 4);
//		
//	   }
//			rt_kprintf("token:%x,%x,%x,%x", token[0],token[1],token[2],token[3]);		
//	
//	}
}
rt_int8_t SywAppTcpConnectToServer(const char* host, rt_uint16_t port)
{
	  struct hostent *Swyhost;
	//struct sockaddr_in server_addr;
	  Swyhost = gethostbyname(host);
#if (Swy_tcp == 1 || Swy_mqtt == 1)
		server_adr.sock = at_socket(AF_INET, SOCK_STREAM, 0); 
#elif(Swy_udp == 1)
		server_adr.sock = at_socket(AF_INET, SOCK_DGRAM, 0);
#endif
		if(server_adr.sock == -1)
    {
        
        rt_kprintf("Socket error\n");
        return -3;
    }
			
		rt_kprintf("creat_Socket %d\n",server_adr.sock);

    /* init pre connent of ser addr */
    server_adr.sin_family = AF_INET;
    server_adr.sin_port = htons(port);
   // server_addr.sin_addr = *((struct in_addr *)host->h_addr);
		server_adr.sin_addr.s_addr = inet_addr(host);
    rt_memset(&(server_adr.sin_zero), 0, sizeof(server_adr.sin_zero));
		if(server_adr.sock >= 0)
		{
    #if ((Swy_tcp == 1)  || (Swy_mqtt == 1))
			if (at_connect(server_adr.sock, (struct sockaddr *)&server_adr, sizeof(struct sockaddr)) == -1)
	    {
	        /* connect fail */
         rt_kprintf("Connect fail!\n");
				 ctrl_bc26_device_ops("reset");//fix confail frank
				 rt_thread_mdelay(5000);
				 SwyAppReconnectServer();
				 rt_kprintf("reset-NB-MODULE1\n");
	       at_closesocket(server_adr.sock);
	       return -1;
	    }
	  #endif
		}
   else
		{
		  at_closesocket(server_adr.sock);
      return -2;
  	}
		rt_kprintf("connect_ser ok\n");
		return 0;
}

static void upload_sensor_data_thread(void *parameter)
{
	  rt_uint32_t count = 0,conect_ser=0,send_len=0;
		rt_err_t result = RT_NULL;
		static rt_int32_t sockstate = 0;
	  char *str;
		struct at_device_bc26 *bc26;
		struct at_device *device;
    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, "bc26");
    bc26 = (struct at_device_bc26 *)device->user_data;
		if (g_swy_data.upload_time < 5000 )
		{
	  	g_swy_data.upload_time = 30000;// default 10s
		}
		while(1)
		{
		 //  sock = at_get_socket(server_adr.sock);
			 result = rt_sem_take(dynamic_sem,RT_WAITING_FOREVER);
			 if(result != RT_EOK)
				{
					return;
				}
	     if (Swy_tcp == 1)
	    	{
	        sockstate = _tcp_conn.state ;
	    	}
	     else if (Swy_mqtt == 1)
	      { 
	        rt_kprintf("sockstate=%d\n",_mq_conn.state);
	        sockstate = _mq_conn.state;
	  	  }
			  else if (Swy_udp == 1)
		  	{
	        sockstate = _tcp_conn.state ;
		  	}
		    if (sockstate != AT_SOCKET_CONNECT)
		 		{
				   at_closesocket(server_adr.sock);
				   rt_kprintf("sockstate=close\n");
				   SywAppTcpConnectToServer(Swyurl,Swyport);
		    }
		
				if (sockstate == AT_SOCKET_CONNECT)//(conect_ser==0)
				{
    		 //if (rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
         //{
         // rt_kprintf("get a mail from mailbox, the content:%s\n", str);
         // if (str == mb_str)
					rt_kprintf("uploade check bc26->sleep_statu %d\n",bc26->sleep_status);
		      if(bc26->sleep_status == 1)
		      {
						rt_kprintf("already_sleep wakeup psm_int start\n");
						ctrl_bc26_device_ops("wakeup");
						rt_thread_mdelay(200);
					  rt_kprintf("already_sleep wakeup psm_int end\n");
		      }
					send_len=Swy_nb_send(_tcp_conn.send_data,_tcp_conn.send_size);
		      if(send_len <0)//fix serial send fail frank
				  {
				  	rt_kprintf("reset-NB-MODULE\n");
					  ctrl_bc26_device_ops("reset");
					  rt_thread_mdelay(3000);
					  SwyAppReconnectServer();//
      	  }
				}
			 else
			  {
				   SwyAppReconnectServer();
			  }
		rt_kprintf("upload_comin&upload_time=%d\n",g_swy_data.upload_time);
		rt_sem_release(dynamic_sem);
		rt_thread_mdelay(g_swy_data.upload_time);
		
  }
}
void get_sensor_data_thread(void *parameter)
{
		float humidity, temperature;
	//	aht10_device_t dev; 							 /* device object */
	//	const char *i2c_bus_name = "i2c2"; /* i2c bus station */
		int count = 0,Swysendlen=0;
	//  unsigned char sendbuf[50] = {0};
		unsigned char *p = NULL;
		rt_err_t result = RT_NULL;
	
		while(1)
		{
			result = rt_sem_take(dynamic_sem,RT_WAITING_FOREVER);
			if(result != RT_EOK)
			{
				return;
			}
		/* initializes aht10, registered device driver */
		/*	dev = aht10_init(i2c_bus_name);
			if(dev == RT_NULL)
			{
					rt_kprintf(" The sensor initializes failure");
					return 0;
			}
   	*/
			rt_memset(_tcp_conn.send_data, 0, sizeof(_tcp_conn.send_data));
      p = _tcp_conn.send_data;
			/* read humidity */
			humidity = 230;//aht10_read_humidity(dev);
			//rt_kprintf("humidity	 : %d.%d %%\n", (int)humidity, (int)(humidity * 10) % 10); /* former is integer and behind is decimal */

			/* read temperature */
			temperature = 560;//aht10_read_temperature(dev);
			// %d.%d \n", (int)temperature, (int)(temperature * 10) % 10); /* former is integer and behind is decimal */
      Swy_big_endian_cpy(p, &humidity, 4);
			p +=4;
			Swysendlen +=4;
      Swy_big_endian_cpy(p, &temperature, 4);
			Swysendlen +=4;
      _tcp_conn.send_size = Swysendlen;//xz_encode_send_data(_udp_conn.send_data);
			rt_kprintf("sensor_comin\n");
			rt_sem_release(dynamic_sem);
		  //rt_mb_send(&mb, (rt_uint32_t)&mb_str);
			rt_thread_mdelay(15000);
			
		}
	}

int thread_sample(void)
{   
    rt_err_t result;
		dynamic_sem = rt_sem_create("dsem",1,RT_IPC_FLAG_FIFO);
		if(dynamic_sem == RT_NULL)
		{
			rt_kprintf("Failed to create dynamic semaphore! \n");
			return 1;
		}
    /* 初始化一个 mailbox */
//    result = rt_mb_init(&mb,
//                        "mbt",                      /* 名称是 mbt */
//                        &mb_pool[0],                /* 邮箱用到的内存池是 mb_pool */
//                        sizeof(mb_pool) / 4,        /* 邮箱中的邮件数目，因为一封邮件占 4 字节 */
//                        RT_IPC_FLAG_FIFO);          /* 采用 FIFO 方式进行线程等待 */
//    if (result != RT_EOK)
//    {
//        rt_kprintf("init mailbox failed.\n");
//        return -1;
//    }
		tid1 = rt_thread_create("sensor",
															 get_sensor_data_thread, RT_NULL,
															 1024,
															 28, 10);
		
			 if (tid1 != RT_NULL)
					 rt_thread_startup(tid1);
			 
    tid2 = rt_thread_create("upload",
                            upload_sensor_data_thread, RT_NULL,
                            2048,   //THREAD_STACK_SIZE
                            29, 10);//THREAD_PRIORITY  THREAD_TIMESLICE

    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
		
    return 0;
}

