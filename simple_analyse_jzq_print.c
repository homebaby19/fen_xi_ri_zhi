#include <stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include <string.h>


#define LITTLE_ENDIAN_BYTE_TO_SHORT(x)      ((*((unsigned char *)(x)+1))<<8 |\
                                                (*((unsigned char *)(x)+0)))
const char *dir = "./files_698_print/";

int LEN_LEN = sizeof("len = 58")-1;
int TM_LEN = sizeof("<17-09-20 07:54:30>")-1;
int recv_num = 0, all_recv_num = 0;;
int recv_flag = 0;
int ciphertext_flag = 0;/*密文标志*/
int read_f2010201 = 0, read_f2090201 = 0;
int flag_f2090201 = 0;


#define D69845_CA_LEN (1)
#define D69845_HCS_LEN (2)



enum apdu_type
{
    LINK_REQUEST  = 1,
    LINK_RESPONSE = 129,

    CONNECT_REQ = 2,
    CONNECT_RESPONSE = 130,

    RELEASE_REQ  = 3,
    RELEASE_RESPONSE = 131,
    RELEASE_NOTIFICATION = 132,

    GET_REQ  = 5,
    GET_RESPONSE = 133,

    SET_REQ  = 6,
    SET_RESPONSE = 134,

    ACTION_REQ  = 7,
    ACTION_RESPONSE = 135,

    REPORT_RESPONSE  = 8,
    REPORT_NOTIFICATION = 136,

    PROXY_REQ  = 9,
    PROXY_RESPONSE = 137,

    SECURITY_REQUEST  = 16,
    SECURITY_RESPONSE = 144,
};



int ascll_bytes_to_digit_bytes(unsigned char ascll_buf[], unsigned char digit_buf[])
{
    int i, tmp;

    for(i=0; i<strlen(ascll_buf)/3; i++)
    {
        sscanf(ascll_buf+i*3, "%02x ", &tmp);
        digit_buf[i] = tmp;
    }

    return (i);
}


unsigned int big_endian_bytes_to_numeric(unsigned char indata[],  int len)
{
    unsigned int value =0;
    int i;

    if(len > 0x04)
    {
        return(0 );
    }
    for(i = 0; i < len; i++)
    {
        value <<= 8;
        value += indata[i];
    }
    return(value);
}


struct OAD_DESP_T
{
    unsigned int oad;
    unsigned char desp[0x80];
};
static struct OAD_DESP_T oad_desp[] =
{
    {0x20000200, "电压"},
    {0x20010200, "电流"},
    {0x20040200, "有功功率"},
    {0x20050200, "无功功率"},
    {0x200a0200, "功率因数"},

    {0x300f0200, "电能表电压逆相序事件"},
    {0x31000200, "终端初始化事件"},
    {0x31000400, "初始化事件当前记录数"},

    {0x40000200, "集中器的时间"},
    {0x40000300, "校时模式"},
    {0x40000400, "精准校时参数"},
    {0x40010200, "通信地址"},
    {0x40030200, "客户编号"},
    {0x40040200, "地理位置"},
    {0x40300200, "电压合格率参数"},

    {0x43000300, "集中器版本信息"},
    {0x43000400, "生产日期"},
    {0x43000500, "子设备列表"},
    {0x43000600, "支持规约列表"},
    {0x43000800, "允许主动上报"},
    {0x43000900, "允许与主站通话"},

    {0x45000200, "无线公网，通信配置"},
    {0x45000300, "无线公网。主站通信参数表"},

    {0x45100300, "以太网。主站通信参数表"},
    {0x45100400, "以太网，设置终端IP"},

    {0x50040200, "日冻结"},
    {0x50060200, "月冻结"},

    {0x60000200, "采集档案配置单元"},
    {0x60120200, "集中器的任务"},
    {0x60120300, "记录单元"},
    {0x60160200, "事件采集方案"},
    {0x60340200, "采集任务监控"},

    {0xf0010400, "传输块状态字"},
    
    {0xf2030200, "开关量单元"},
    {0xf2030400, "开关量接入标志"},

    {0xf2090201, "本地通信模块单元,版本信息"},
    {0xF2090600, "从节点对象列表更新周期"},
    {0xF2090700, "网络拓扑信息"},
    {0xF2090800, "多网信息"},
    {0xF2090900, "宽带载波频段序号"},

    {0xF1000200, "ESAM序列号"},
    {0xF1000400, "对称密钥版本"},
    {0xF1000700, "当前计数器"},
    {0xF1000A00, "终端证书"},
    {0xF1000C00, "主站证书"},
    {0xF1010200, "是否启用安全模式"},
};

static void get_oad_desp(unsigned int oad, unsigned char typebuf[])
{
    int i;
    for(i=0; i<(sizeof(oad_desp))/(sizeof(oad_desp[0])); i++)
    {
        if((oad >> 8) == (oad_desp[i].oad >> 8))
        {
            strcat(typebuf, oad_desp[i].desp);
            strcat(typebuf, ".");
            return;
        }
    }
    strcat(typebuf, "待补充的oad.");
}


static int GET_REQ_1_2(int num, unsigned char oad_buf[], unsigned char typebuf[])
{
    int i;
    unsigned int oad;

    strcpy(typebuf, "，读取,");
    for(i=0; i<num; i++)
    {
        oad = big_endian_bytes_to_numeric(&oad_buf[i*4], 4);
        sprintf(typebuf+strlen(typebuf), "%08x-", oad);
        get_oad_desp(oad,typebuf);
    }
    strcat(typebuf, "\n");
    return (0);
}


static int GET_REQ_4(int num, unsigned char oad_buf[], unsigned char typebuf[])
{
    if(((4 == num) || (6 == num))
       &&(0x60000200 == big_endian_bytes_to_numeric(&oad_buf[0], 4))
       )
    {
        strcpy(typebuf, ",GetResponseRecordList，读取60000200。\n");
        return (0);
    }
    return (-1);
}


static int SET_REQ_1(unsigned int oad, unsigned char typebuf[])
{
    strcpy(typebuf, "，设置,");
    sprintf(typebuf+strlen(typebuf), "%08x-", oad);
    get_oad_desp(oad,typebuf);
    strcat(typebuf, "\n");
    return (0);
}



struct OMD_DESP_T
{
    unsigned int omd;
    unsigned char desp[0x80];
};
static struct OMD_DESP_T omd_desp[] =
{
    {0x40007F00, "4000, 方法127，广播校时。"},

    {0x43000100, "omd43000100，设备复位重启。"},
    {0x43000400, "omd43000400，恢复出厂参数。5秒后，集中器复位。"},
    {0xF2098000, "F209, 方法128,配置端口参数。"},
    {0x60007F00, "6000, 方法127，添加一个采集档案配置单元。"},
    {0x60008000, "6000, 方法128，批量添加采集档案配置单元。"},

    {0x60008600, "6000, 方法134，清库。"},
    {0x60027F00, "6002，方法127，搜表。"},

    {0x60127F00, "6012，方法127，配置任务。"},
    {0x60128100, "6012，方法129，清任务。"},
    {0x60147F00, "6014，方法127，配置方案。"},
    {0x60148100, "6014，方法129，清方案。"},
    {0x60167F00, "6016，方法127，配置事件采集方案。"},
    {0x60168000, "6016，方法128，删除一组事件采集方案。"},
    {0x60168100, "6016，方法129，清事件采集方案。"},
    {0x60168200, "6016，方法130，上报标识。"},

    {0xF2097F00, "F209，方法127，透明转发。"},
};


static int ACTION_REQ_1(unsigned int omd, unsigned char typebuf[])
{
    int i;
    for(i=0; i<(sizeof(omd_desp))/(sizeof(omd_desp[0])); i++)
    {
        if(omd == omd_desp[i].omd)
        {
            strcpy(typebuf, ",");
            strcat(typebuf, omd_desp[i].desp);
            strcat(typebuf, "\n");
            return (0);
        }
    }
    strcpy(typebuf, "待补充的omd.");
    return (-1);
}



static int PROXY_REQ_7(unsigned int oad, unsigned char typebuf[])
{
    switch(oad)
    {
    case 0xf2090201:
        strcpy(typebuf, ",透明转发f2090201\n");
        read_f2090201++;
        if(flag_f2090201)
        {
            printf("no recv of  f2090201 --------------\n");
        }
        flag_f2090201 = 1;
        return (0);
        break;
    case 0xf2010201:
        strcpy(typebuf, ",透明转发f2010201\n");
        read_f2010201++;
        return (0);
        break;
    }
    return (-1);
}



static int get_apdu_info(unsigned char apdu[], unsigned char typebuf[])
{
    int idx, k;
    unsigned char apdu_type, apdu_subtype;

    idx = 0;
    apdu_type = apdu[idx];
    apdu_subtype = apdu[idx+1];
    switch(apdu_type)
    {
    case LINK_RESPONSE:
        strcpy(typebuf, ",登录、心跳回应。\n");
        return (0);
        break;
    case CONNECT_REQ:
        strcpy(typebuf, ",建立应用连接。\n");
        return (0);
        break;
    case GET_REQ:
        if(1 == apdu_subtype)
        {
            if(0 == GET_REQ_1_2(1, &apdu[idx+3], typebuf))
            {
                return (0);
            }
        }
        if(2 == apdu_subtype)
        {
            if(0 == GET_REQ_1_2(apdu[idx+3], &apdu[idx+4], typebuf))
            {
                return (0);
            }

        }
        if(3 == apdu_subtype)
        {
            if(0 == GET_REQ_1_2(1, &apdu[idx+3], typebuf))
            {
                return (0);
            }

        }
        if(4 == apdu_subtype)
        {
            if(0 == GET_REQ_4(apdu[idx+3], &apdu[idx+4], typebuf))
            {
                return (0);
            }
        }
        if(5 == apdu_subtype)
        {
            strcpy(typebuf, ",读取下一帧。\n");
            return (0);
        }
        break;
    case SET_REQ:
        if(1 == apdu_subtype)
        {
            if(0 == SET_REQ_1(big_endian_bytes_to_numeric(&apdu[idx+3], 4), typebuf))
            {
                return (0);
            }
        }
        if(2 == apdu_subtype)
        {
            if(0 == SET_REQ_1(big_endian_bytes_to_numeric(&apdu[idx+4], 4), typebuf))
            {
                return (0);
            }
        }
        break;
    case ACTION_REQ:
        if(1 == apdu_subtype)
        {
            if(0 == ACTION_REQ_1(big_endian_bytes_to_numeric(&apdu[idx+3], 4), typebuf))
            {
                return (0);
            }
        }
        if(3 == apdu_subtype)
        {
        }
        break;
    case REPORT_RESPONSE:
        if(2 == apdu_subtype)
        {
            strcpy(typebuf, ",上报若干个记录型对象属性的响应.PIID==。\n");
            return (0);
        }
        if(3 == apdu_subtype)
        {
            strcpy(typebuf, ",收到主动上报的确认报文.PIID==。\n");
            return (0);
        }
        break;
    case PROXY_REQ:
        if(1 == apdu_subtype)
        {
            k = 8;
            sprintf(typebuf+strlen(typebuf), ",代理1。addr:%02x%02x%02x%02x%02x%02x.oad:%02x%02x%02x%02x\n",
                   apdu[k],apdu[k+1],apdu[k+2],apdu[k+3],apdu[k+4],apdu[k+5],
                   apdu[k+9],apdu[k+10],apdu[k+11],apdu[k+12]);
            return (0);
        }
        if(7 == apdu_subtype)
        {
            if(0 == PROXY_REQ_7(big_endian_bytes_to_numeric(&apdu[idx+3], 4), typebuf))
            {
                return (0);
            }
        }
        break;
    case SECURITY_REQUEST:
    case SECURITY_RESPONSE:
        ciphertext_flag = 1;
        strcpy(typebuf, ",密文，后面详细解析。\n");
        return (0);
        break;
    }

    printf("unknown, apdu_type = %x, apdu_subtype = %x\n", apdu_type, apdu_subtype);


    return (-1);
}

#define RTSTARTCH   0x68        /* Frame start character */
#define TAIL  0x16 /** 帧尾字符 */

/*无用报文，即首字节不是0x68的报文*/
static int find_698_head(const unsigned char *buf, int len)
{
    int i, flen;

    for (i =0;i < len;i++)
    {
        if((RTSTARTCH == buf[i]) && (i+8 < len))
        {
            return (i);
            flen = LITTLE_ENDIAN_BYTE_TO_SHORT(&buf[i+1]) & 0x3FFF;
            if((flen+2+i <= len) && (TAIL == buf[i+flen+1]))
            {
                return (i);
            }
        }
    }

    return (-1);
}



static int get_frame_type(unsigned char buf[], unsigned char typebuf[])
{
    char *p;
    unsigned char hex_buf[0x400];
    int idx, len;
    
    if(p = strstr(buf,">INFO: 68 "))
    {
        p += strlen(">INFO: ");
    }
    else if(p = strstr(buf,"up recv:"))
    {
        p += strlen("up recv:");
    }
    else
    {
        return (-1);
    }
        

	len = ascll_bytes_to_digit_bytes(p, hex_buf);
	idx = find_698_head(hex_buf, len);
    if(idx < 0)
    {
	    strcpy(typebuf, "error, can not find frame!\n");
        return (0);
    }
	if(0x10 == (hex_buf[idx+4]&0xf0))
	{
	    strcpy(typebuf, ",逻辑地址为1。");
	}
	else
	{
	    strcpy(typebuf, "");
	}
	idx += 1 + 2 + 1 + (2 + hex_buf[idx+4]&0x0f) + D69845_CA_LEN + D69845_HCS_LEN;
	return (get_apdu_info(&hex_buf[idx], typebuf));
}


static int get_apdu_type(unsigned char buf[], unsigned char typebuf[])
{
    char *p;
    unsigned char hex_buf[0x400];
    int len;

    if(p = strstr(buf,"recv apdu: "))
    {
        p += strlen("recv apdu: ");
        len = ascll_bytes_to_digit_bytes(p, hex_buf);
        return (get_apdu_info(hex_buf, typebuf));
    }

    return (-1);
}

void analyse_one_line(unsigned char buf[], FILE *fp)
{
    char *p;
    unsigned char tm[0x30], wwjbuf[0x100];
    static unsigned char lenbuf[0x30], typebuf[0x100], no_buf[0x10];

    fputs(buf, fp);
    if(p = strstr(buf,"89 07 01 F2 09 02 01"))
    {
        flag_f2090201 = 0;
    }
    
    if(p = strstr(buf,"start to decode the frame"))
    {
        strcpy(lenbuf, p+strlen("start to decode the frame,"));
        recv_num++;
        recv_flag = 1;
        ciphertext_flag = 0;
        return;
    }
    if(recv_flag)
    {
        if(p = strstr(buf,"|msserver|<"))
        {
            memcpy(tm, p+strlen("|msserver|"), TM_LEN);

            sprintf(wwjbuf, "wwj:%04d", recv_num);
            strcat(wwjbuf, tm);

            if(0 == get_frame_type(buf, typebuf))
            {
                strcat(wwjbuf, typebuf);
            }
            else
            {
                strcat(wwjbuf, ",,, ");
                strcat(wwjbuf, lenbuf);
            }

            printf("%s", wwjbuf);
            fputs(wwjbuf, fp);
        }
        recv_flag = 0;
    }
    if(ciphertext_flag)
    {
        if(p = strstr(buf,"recv apdu: "))
        {
            strcpy(typebuf, "");
            if(0 == get_apdu_type(buf, typebuf))
            {
                strcpy(wwjbuf, "wwj-pwd: ");
                strcat(wwjbuf, typebuf);
                printf("%s", wwjbuf);
                fputs(wwjbuf, fp);
            }
            ciphertext_flag = 0;
        }
    }
}


void assemble_whole_name(char *name, char *whole_name, char *target_name)
{
    strcpy(whole_name, dir);
    strcat(whole_name, name);


    strcpy(target_name, dir);
    strcat(target_name, "analyse-");
    strcat(target_name, name);

    printf("whole name: %s\n", whole_name);
    printf("target name: %s\n", target_name);
}

void analyse_one_file(char *name)
{
    char file_name[0x100], target_name[0x100];
    unsigned char buf[0x800];
    FILE *fps = NULL, *fpt = NULL;

    assemble_whole_name(name,file_name, target_name);
    fps = fopen(file_name, "r");
    if(NULL == fps)
    {
        return;
    }


    fpt = fopen(target_name, "w+");
    if(NULL == fpt)
    {
        return;
    }

    while (1)
    {
        fgets(buf,sizeof(buf),fps);

        analyse_one_line(buf, fpt);
        if (feof(fps))
        {
            break;
        }
    }

    all_recv_num += recv_num;
    printf("recv frame num = %d\n", recv_num);
    printf("read_f2010201 = %d\n", read_f2010201);
    printf("read_f2090201 = %d\n", read_f2090201);

    fclose(fps);
    fclose(fpt);


}


void check_dir(void)
{
    DIR *dirptr = NULL;  
    struct dirent *entry; 

    if(NULL == (dirptr = opendir(dir)))
    {  
        printf("open dir !\n");  
    }  
    else  
    {  
        while (entry = readdir(dirptr))  
        {  
            if((strstr(entry->d_name,"txt")) && (!strstr(entry->d_name,"analyse")))
            {
                recv_num = 0;
                read_f2010201 = 0;
                read_f2090201 = 0;
                printf("begin analyse 698 jzq print: %s\n", entry->d_name);/* 打印出该目录下的所有内容 */ 
                analyse_one_file(entry->d_name); 
            }

        }  
        printf("all files recv frame num = %d\n", all_recv_num);
        closedir(dirptr);  
    }  
}


int main (int argc, char *argv[])
{

    check_dir();
    return(0);
}



