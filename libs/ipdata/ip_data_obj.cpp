#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<utility>
#include<memory>
#include "ip_data_obj.h"
#include "serverconfig.h"

namespace http
{
ip_data_obj &get_ip_data_obj()
{
    static ip_data_obj instance;
    return instance;
}

void ip_data_obj::init()
{
    serverconfig &sysconfigpath = getserversysconfig();
    std::string htmlcontent=sysconfigpath.configpath;
    htmlcontent.append("ip.data");//data from ip2region
    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen(htmlcontent.c_str(), "rb"), std::fclose);

    if (!fp.get())
    {
        return;
    }

    fseek(fp.get(), 0, SEEK_END);
    unsigned int file_size = ftell(fp.get());
    fseek(fp.get(), 0, SEEK_SET);
    
    if(file_size<12)
    {
        return;
    }

    htmlcontent.clear();
    htmlcontent.resize(8);
    fread(&htmlcontent[0], 1, 8, fp.get());

    unsigned int header_size; 
    header_size=(unsigned char)htmlcontent[0]<<24;
    header_size=header_size|(unsigned char)htmlcontent[1]<<16;
    header_size=header_size|(unsigned char)htmlcontent[2]<<8;
    header_size=header_size|(unsigned char)htmlcontent[3];

    ipdatasize=(unsigned char)htmlcontent[4]<<24;
    ipdatasize=ipdatasize|(unsigned char)htmlcontent[5]<<16;
    ipdatasize=ipdatasize|(unsigned char)htmlcontent[6]<<8;
    ipdatasize=ipdatasize|(unsigned char)htmlcontent[7];

    if(file_size<header_size)
    {
        return;
    }
    header_size=header_size-8;


    htmlcontent.clear();
    htmlcontent.resize(header_size);
    fread(&htmlcontent[0], 1, header_size, fp.get());
    
    unsigned int offsetsize=0;
    std::string testr;
    for(;offsetsize<header_size;offsetsize++)
    {
        if(htmlcontent[offsetsize]==0x0A)
        {
            offsetsize++;
            countrylists.push_back(testr);
            testr.clear(); 
            break;
        }
        if(htmlcontent[offsetsize]==0x00)
        {
           countrylists.push_back(testr);
           testr.clear(); 
           continue;
        }
        testr.push_back(htmlcontent[offsetsize]);
    }

    testr.clear(); 
     for(;offsetsize<header_size;offsetsize++)
    {
        if(htmlcontent[offsetsize]==0x0A)
        {
            offsetsize++;
            provincelists.push_back(testr);
            testr.clear(); 
            break;
        }
        if(htmlcontent[offsetsize]==0x00)
        {
           provincelists.push_back(testr);
           testr.clear(); 
           continue;
        }
        testr.push_back(htmlcontent[offsetsize]);
    }

    testr.clear(); 
     for(;offsetsize<header_size;offsetsize++)
    {
        if(htmlcontent[offsetsize]==0x0D)
        {
            offsetsize++;
            citylists.push_back(testr);
            testr.clear(); 
            break;
        }
        if(htmlcontent[offsetsize]==0x00)
        {
           citylists.push_back(testr);
           testr.clear(); 
           continue;
        }
        testr.push_back(htmlcontent[offsetsize]);
    }

    htmlcontent.clear();
    htmlcontent.resize(10);

    if(file_size<(header_size+ipdatasize*10))
    {
        ipdatasize=0;
        return;
    }
    ipdatalist =std::make_unique<ipdata_area_t[]>(ipdatasize);
    for(unsigned int kj=0;kj<ipdatasize;kj++)
    {
        fread(&htmlcontent[0], 1, 10, fp.get());
        unsigned a;
        a=(unsigned char)htmlcontent[0]<<24;
        a=a|(unsigned char)htmlcontent[1]<<16;
        a=a|(unsigned char)htmlcontent[2]<<8;
        a=a|(unsigned char)htmlcontent[3];
        ipdatalist[kj].a=a;

        a=(unsigned char)htmlcontent[4]<<24;
        a=a|(unsigned char)htmlcontent[5]<<16;
        a=a|(unsigned char)htmlcontent[6]<<8;
        a=a|(unsigned char)htmlcontent[7];
        ipdatalist[kj].b=a;

        a=(unsigned char)htmlcontent[8]<<8;
        a=a|(unsigned char)htmlcontent[9];
        ipdatalist[kj].c=a&0xFFFF;
    }

}
unsigned int ip_data_obj::ip2long(const std::string &ipsearch)
{
    if(ipsearch.size()>0)
    {   
        unsigned char tempip=0;
        unsigned char p_num=ipsearch.size();
        if(p_num>17)
        { 
            p_num=18;
        }
        for(unsigned int i=0;i<p_num;i++)
        {
            if(ipsearch[i]=='.')
            {
                ip=ip<<8|tempip;
                tempip=0;
                continue;
            }
            if(ipsearch[i]< 0x3A && ipsearch[i] > 0x2F)
            {
                tempip=tempip*10+(ipsearch[i]-'0');
            }
        }
        ip=ip<<8|tempip;
        return ip;
    }
    return 0;
}
 
std::string ip_data_obj::search(const std::string &ipsearch,bool iscity)
{
    ip=0;
    std::string temptext;
    if(ip2long(ipsearch)==0)
    {
        return temptext;
    }
    if(ipdatasize==0)
    {
        init();
        if(ipdatasize==0)
        {
            return temptext;
        }
    }
    result=0xFFFF;
    dichotomy_search(0,ipdatasize);
    if(result!=0xFFFF)
    {
        if(result>=0x8000)
        {
            unsigned char guojia=result&0xFF;
            if(guojia<countrylists.size())
            {
                temptext=countrylists[guojia];
            }
        }
        else
        {
            unsigned char shengindex=(result&0x7E00)>>9;
            if(shengindex<provincelists.size())
            {
                temptext=provincelists[shengindex];
            }
            if(iscity)
            {
                unsigned short shiindex=result&0x1FF;
                if(shiindex<citylists.size())
                {
                    temptext.append(citylists[shiindex]);
                }
            }
        }
    }
    return temptext;
}
void ip_data_obj::dichotomy_search(unsigned int a,unsigned int b)
{
     unsigned int temp_num=a+(b-a)/2;
     if(ip>=ipdatalist[temp_num].a && ip<=ipdatalist[temp_num].b)
     {
        result=ipdatalist[temp_num].c;
        return;
     }
     if(ip<ipdatalist[temp_num].a)
     {
        dichotomy_search(a,temp_num-1);
     }else
     {
        dichotomy_search(temp_num+1,b);
     }
}
}