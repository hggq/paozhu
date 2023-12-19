#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>

int main(int argc, char *argv[])
{
    //g++ ipconvert.hpp -std=c++20
    // download ip.merge.txt from https://github.com/lionsoul2014/ip2region
    // ./a.out 无参数 带省市 生成ip.data 6.8M
    // ./a.out 1 加多一个参数 不要市，只要省，ip合并只有1.7M ip.data,监管要求显示地区，国内到省可以了
    // 拷贝 ip.data 到 conf 目录
    // 测试文件 controller/src/testipsearch.cpp
    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen("ip.merge.txt", "rb"), std::fclose);

    if (!fp.get())
    {
        std::cout << " open ip.merge.txt file error! \n download from https://github.com/lionsoul2014/ip2region \n create ip.data file copy to conf/ directory  " << std::endl;
        return 0;
    }

    fseek(fp.get(), 0, SEEK_END);
    unsigned int file_size = ftell(fp.get());
    fseek(fp.get(), 0, SEEK_SET);

    std::cout << file_size << std::endl;
    std::string htmlcontent;
    htmlcontent.resize(file_size);
    file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
    htmlcontent.resize(file_size);

    std::string temp, argv_str;
    temp.clear();
    std::map<std::string, std::vector<std::pair<unsigned int, unsigned int>>> ipmap;
    std::map<std::string, std::vector<std::pair<unsigned int, unsigned int>>> shengmap;
    std::map<std::string, std::vector<std::pair<unsigned int, unsigned int>>> guomap;
    struct iparea
    {
        unsigned int a;
        unsigned int b;
        unsigned short c;
    } ipdata;
    std::vector<iparea> iplists;
    std::string presheng;
    std::string preguojia;
    unsigned int pre_a = 0, pre_b = 0;

    bool issheng = false;
    if (argc > 1)
    {
        issheng = true;
    }

    for (unsigned int i = 0; i < file_size; i++)
    {
        if (htmlcontent[i] == 0x0A)
        {
            unsigned int a = 0, b = 0;
            unsigned int j = 0;

            unsigned char tempip;
            tempip = 0;
            for (; j < temp.size(); j++)
            {
                if (temp[j] == 0x7C)
                {
                    j++;
                    break;
                }
                if (temp[j] == '.')
                {
                    a = a << 8 | tempip;

                    tempip = 0;
                    continue;
                }
                if (temp[j] < 0x3A && temp[j] > 0x2F)
                {
                    tempip = tempip * 10 + (temp[j] - '0');
                }
            }
            a      = a << 8 | tempip;
            tempip = 0;
            for (; j < temp.size(); j++)
            {
                if (temp[j] == 0x7C)
                {
                    j++;
                    break;
                }
                if (temp[j] == '.')
                {
                    b = (b * 256) + tempip;

                    tempip = 0;
                    continue;
                }
                if (temp[j] < 0x3A && temp[j] > 0x2F)
                {
                    tempip = tempip * 10 + (temp[j] - '0');
                }
            }
            b = b << 8 | tempip;

            std::string bb;

            if (a == 0)
            {
                bb = "内网";
            }
            if (a == 167772160)
            {
                bb = "内网";
            }
            if (a == 2130706432)
            {
                bb = "内网";
            }
            if (a == 2886729728)
            {
                bb = "内网";
            }
            if (a == 3232235520)
            {
                bb = "内网";
            }
            if (bb.size() == 0)
            {
                std::vector<std::string> lie;
                std::string aa;
                for (; j < temp.size(); j++)
                {
                    if (temp[j] == 0x7C)
                    {
                        lie.push_back(aa);
                        aa.clear();
                        continue;
                    }
                    aa.push_back(temp[j]);
                }

                if (lie[0] == "中国")
                {
                    std::string shiname;
                    if (lie[3] == "0")
                    {
                        if (lie[2] == "0")
                        {
                            shiname = lie[0];
                        }
                        else
                        {
                            shiname = lie[2];
                        }
                    }
                    else
                    {
                        shiname = lie[3];
                    }
                    unsigned char pos = shiname.size();
                    if (((unsigned char)shiname[pos - 1] == 0x82) && ((unsigned char)shiname[pos - 2] == 0xB8) && ((unsigned char)shiname[pos - 3] == 0xe5))
                    {
                        std::string dd = shiname.substr(0, pos - 3);
                        shiname        = dd;
                    }
                    pos = shiname.size();
                    if (((unsigned char)shiname[pos - 1] == 0x81) && ((unsigned char)shiname[pos - 2] == 0x9C) && ((unsigned char)shiname[pos - 3] == 0xe7))
                    {
                        std::string dd = shiname.substr(0, pos - 3);
                        shiname        = dd;
                    }
                    ipmap[shiname].emplace_back(std::pair(a, b));
                    if (lie[2] == "0")
                    {
                        shengmap[lie[0]].emplace_back(std::pair(a, b));
                    }
                    else
                    {
                        // E79C81 省
                        // E5B882 市
                        pos = lie[2].size();
                        if (((unsigned char)lie[2][pos - 1] == 0x81) && ((unsigned char)lie[2][pos - 2] == 0x9c) && ((unsigned char)lie[2][pos - 3] == 0xe7))
                        {
                            std::string dd = lie[2].substr(0, pos - 3);
                            shengmap[dd].emplace_back(std::pair(a, b));
                        }
                        else
                        {
                            shengmap[lie[2]].emplace_back(std::pair(a, b));
                        }
                    }
                }
                if (lie[0] == "0")
                {
                    unsigned char n = 0;
                    for (; n < lie.size(); n++)
                    {
                        if (lie[n] != "0")
                        {
                            if (lie[n] == "内网IP")
                            {
                                bb = "内网";
                                guomap[bb].emplace_back(std::pair(a, b));
                            }
                            else
                            {
                                guomap[lie[n]].emplace_back(std::pair(a, b));
                            }
                            break;
                        }
                    }
                    if (lie[n] == "0")
                    {
                        bb = "内网";
                        guomap[bb].emplace_back(std::pair(a, b));
                        // ipaddressmap[bb] = "";
                    }
                }
                else
                {
                    guomap[lie[0]].emplace_back(std::pair(a, b));
                    // ipaddressmap[lie[0]] = "";
                }
                // break;
            }
            else
            {
                guomap[bb].emplace_back(std::pair(a, b));
                // ipaddressmap[bb] = "";
            }
            temp.clear();
            continue;
        }
        temp.push_back(htmlcontent[i]);
    }

    std::vector<std::string> citylists;
    std::vector<std::string> provincelists;
    std::vector<std::string> countrylists;
    for (auto &[a, b] : ipmap)
    {
        citylists.push_back(a);
    }
    for (auto &[a, b] : guomap)
    {
        countrylists.push_back(a);
    }
    for (auto &[a, b] : shengmap)
    {
        provincelists.push_back(a);
    }

    std::string ipfilecontent;
    // header size
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);

    // body number
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);
    ipfilecontent.push_back(0x00);

    for (unsigned int i = 0; i < countrylists.size(); i++)
    {
        if (i > 0)
        {
            ipfilecontent.push_back(0x00);
        }
        ipfilecontent.append(countrylists[i]);
    }
    ipfilecontent.push_back(0x0A);

    for (unsigned int i = 0; i < provincelists.size(); i++)
    {
        if (i > 0)
        {
            ipfilecontent.push_back(0x00);
        }
        ipfilecontent.append(provincelists[i]);
    }
    ipfilecontent.push_back(0x0A);

    for (unsigned int i = 0; i < citylists.size(); i++)
    {
        if (i > 0)
        {
            ipfilecontent.push_back(0x00);
        }
        ipfilecontent.append(citylists[i]);
    }
    ipfilecontent.push_back(0x0D);

    unsigned int header_size = ipfilecontent.size();
    unsigned a = 0, b = 0, c = 0, d = 0;
    a = (header_size & 0xFF000000) >> 24;
    b = (header_size & 0xFF0000) >> 16;
    c = (header_size & 0xFF00) >> 8;
    d = (header_size & 0xFF);

    ipfilecontent[0] = a;
    ipfilecontent[1] = b;
    ipfilecontent[2] = c;
    ipfilecontent[3] = d;

    for (unsigned int i = 0; i < file_size; i++)
    {
        if (htmlcontent[i] == 0x0A)
        {
            unsigned int a = 0, b = 0;
            unsigned int j = 0;

            unsigned char tempip;
            tempip = 0;
            for (; j < temp.size(); j++)
            {
                if (temp[j] == 0x7C)
                {
                    j++;
                    break;
                }
                if (temp[j] == '.')
                {
                    a = a << 8 | tempip;

                    tempip = 0;
                    continue;
                }
                if (temp[j] < 0x3A && temp[j] > 0x2F)
                {
                    tempip = tempip * 10 + (temp[j] - '0');
                }
            }
            a      = a << 8 | tempip;
            tempip = 0;
            for (; j < temp.size(); j++)
            {
                if (temp[j] == 0x7C)
                {
                    j++;
                    break;
                }
                if (temp[j] == '.')
                {
                    b = (b * 256) + tempip;

                    tempip = 0;
                    continue;
                }
                if (temp[j] < 0x3A && temp[j] > 0x2F)
                {
                    tempip = tempip * 10 + (temp[j] - '0');
                }
            }
            b = b << 8 | tempip;

            std::string bb;
            std::string currentsheng;
            std::string currentguojia;
            unsigned int guojia_index = 0;
            unsigned int sheng_index  = 0;
            unsigned int shi_index    = 0;
            bool isguonei             = false;
            if (a == 0)
            {
                bb = "内网";
            }
            if (a == 167772160)
            {
                bb = "内网";
            }
            if (a == 2130706432)
            {
                bb = "内网";
            }
            if (a == 2886729728)
            {
                bb = "内网";
            }
            if (a == 3232235520)
            {
                bb = "内网";
            }
            if (bb.size() == 0)
            {
                unsigned char spm = 2;
                std::vector<std::string> lie;
                std::string aa;

                for (; j < temp.size(); j++)
                {
                    if (temp[j] == 0x7C)
                    {
                        lie.push_back(aa);
                        aa.clear();
                        continue;
                    }
                    aa.push_back(temp[j]);
                }

                if (lie[0] == "中国")
                {
                    isguonei = true;
                    std::string shiname;
                    if (lie[3] == "0")
                    {

                        if (lie[2] == "0")
                        {
                            shiname = lie[0];
                        }
                        else
                        {
                            shiname = lie[2];
                        }
                    }
                    else
                    {
                        shiname = lie[3];
                    }

                    unsigned char pos = shiname.size();
                    if (((unsigned char)shiname[pos - 1] == 0x82) && ((unsigned char)shiname[pos - 2] == 0xB8) && ((unsigned char)shiname[pos - 3] == 0xe5))
                    {
                        std::string dd = shiname.substr(0, pos - 3);
                        shiname        = dd;
                    }
                    if (((unsigned char)shiname[pos - 1] == 0x81) && ((unsigned char)shiname[pos - 2] == 0x9C) && ((unsigned char)shiname[pos - 3] == 0xe7))
                    {
                        std::string dd = shiname.substr(0, pos - 3);
                        shiname        = dd;
                    }
                    for (unsigned int mm = 0; mm < citylists.size(); mm++)
                    {
                        if (shiname == citylists[mm])
                        {
                            shi_index = mm;
                        }
                    }

                    if (lie[2] == "0")
                    {
                        currentsheng = lie[0];
                        for (unsigned int mm = 0; mm < provincelists.size(); mm++)
                        {
                            if (lie[0] == provincelists[mm])
                            {
                                sheng_index = mm;
                            }
                        }
                    }
                    else
                    {
                        // E79C81 省
                        // E5B882 市
                        pos = lie[2].size();
                        if (((unsigned char)lie[2][pos - 1] == 0x81) && ((unsigned char)lie[2][pos - 2] == 0x9c) && ((unsigned char)lie[2][pos - 3] == 0xe7))
                        {
                            std::string dd = lie[2].substr(0, pos - 3);
                            currentsheng   = dd;
                            for (unsigned int mm = 0; mm < provincelists.size(); mm++)
                            {
                                if (dd == provincelists[mm])
                                {
                                    sheng_index = mm;
                                }
                            }
                        }
                        else
                        {
                            currentsheng = lie[2];
                            for (unsigned int mm = 0; mm < provincelists.size(); mm++)
                            {
                                if (lie[2] == provincelists[mm])
                                {
                                    sheng_index = mm;
                                }
                            }
                        }
                    }
                }
                if (lie[0] == "0")
                {
                    unsigned char n = 0;
                    for (; n < lie.size(); n++)
                    {
                        if (lie[n] != "0")
                        {
                            if (lie[n] == "内网IP")
                            {
                                bb = "内网";
                                for (unsigned int mm = 0; mm < countrylists.size(); mm++)
                                {
                                    if (bb == countrylists[mm])
                                    {
                                        guojia_index = mm;
                                    }
                                }
                            }
                            else
                            {
                                for (unsigned int mm = 0; mm < countrylists.size(); mm++)
                                {
                                    if (lie[n] == countrylists[mm])
                                    {
                                        guojia_index = mm;
                                    }
                                }
                            }

                            break;
                        }
                    }
                    if (lie[n] == "0")
                    {
                        bb = "内网";
                        for (unsigned int mm = 0; mm < countrylists.size(); mm++)
                        {
                            if (bb == countrylists[mm])
                            {
                                guojia_index = mm;
                            }
                        }
                    }
                }
                else
                {
                    currentguojia = lie[0];
                    for (unsigned int mm = 0; mm < countrylists.size(); mm++)
                    {
                        if (lie[0] == countrylists[mm])
                        {
                            guojia_index = mm;
                        }
                    }
                }
                // break;
            }
            else
            {
                for (unsigned int mm = 0; mm < countrylists.size(); mm++)
                {
                    if (bb == countrylists[mm])
                    {
                        guojia_index = mm;
                    }
                }
            }

            unsigned short dddd;
            if (isguonei)
            {
                unsigned char s_i = 0;
                s_i               = sheng_index & 0x3F;
                unsigned short eeee;
                eeee = shi_index & 0x01FF;
                dddd = s_i << 9 | eeee;
            }
            else
            {
                dddd = guojia_index | 0x8000;
            }

            if (isguonei)
            {
                if (issheng && (presheng == currentsheng) && iplists.size() > 0)
                {
                    unsigned int lastpos = iplists.size() - 1;
                    iplists[lastpos].b   = b;
                }
                else
                {
                    ipdata.a = a;
                    ipdata.b = b;
                    ipdata.c = dddd;
                    iplists.push_back(ipdata);
                }
            }
            else
            {
                if (issheng && (preguojia == currentguojia) && iplists.size() > 0)
                {
                    unsigned int lastpos = iplists.size() - 1;
                    iplists[lastpos].b   = b;
                }
                else
                {
                    ipdata.a = a;
                    ipdata.b = b;
                    ipdata.c = dddd;
                    iplists.push_back(ipdata);
                }
            }

            presheng  = currentsheng;
            preguojia = currentguojia;
            currentguojia.clear();
            currentsheng.clear();

            pre_a = a, pre_b = b;
            temp.clear();
            continue;
        }
        temp.push_back(htmlcontent[i]);
    }

    header_size = iplists.size();
    std::cout << " ip clear " << header_size << std::endl;
    a = (header_size & 0xFF000000) >> 24;
    b = (header_size & 0xFF0000) >> 16;
    c = (header_size & 0xFF00) >> 8;
    d = (header_size & 0xFF);

    ipfilecontent[4] = a;
    ipfilecontent[5] = b;
    ipfilecontent[6] = c;
    ipfilecontent[7] = d;

    // ipfilecontent.append((char *)iplists.data(),(iplists.size()*10));
    for (unsigned kk = 0; kk < iplists.size(); kk++)
    {
        unsigned a = 0, b = 0, c = 0, d = 0;
        a = (iplists[kk].a & 0xFF000000) >> 24;
        b = (iplists[kk].a & 0xFF0000) >> 16;
        c = (iplists[kk].a & 0xFF00) >> 8;
        d = (iplists[kk].a & 0xFF);

        ipfilecontent.push_back(a);
        ipfilecontent.push_back(b);
        ipfilecontent.push_back(c);
        ipfilecontent.push_back(d);

        a = (iplists[kk].b & 0xFF000000) >> 24;
        b = (iplists[kk].b & 0xFF0000) >> 16;
        c = (iplists[kk].b & 0xFF00) >> 8;
        d = (iplists[kk].b & 0xFF);

        ipfilecontent.push_back(a);
        ipfilecontent.push_back(b);
        ipfilecontent.push_back(c);
        ipfilecontent.push_back(d);

        c = (iplists[kk].c & 0xFF00) >> 8;
        d = (iplists[kk].c & 0xFF);
        ipfilecontent.push_back(c);
        ipfilecontent.push_back(d);
    }
    std::unique_ptr<std::FILE, int (*)(FILE *)> fpc(std::fopen("ip.data", "wb"), std::fclose);

    if (!fpc.get())
    {
        return 0;
    }
    fwrite(&ipfilecontent[0], 1, ipfilecontent.size(), fpc.get());

    return 0;
}