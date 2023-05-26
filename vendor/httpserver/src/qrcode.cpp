#include <cstdio>
#include <cstring>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <memory>

#ifdef ENABLE_GD
#include <qrencode.h>
#include "gd.h"
#endif

#include "qrcode.h"

// g++ libqrencode.cpp -std=c++20 -lgd -lpng -lz -ljpeg -lfreetype -lm -lqrencode
namespace http
{
void qrcode::setScale(unsigned char a) { scale = a; }
std::string qrcode::randname(unsigned char length = 0)
{
    if (length == 0)
    {
        length = 10;
    }
    std::string str = "abcdefghijklmnopqrstuvwxyz1234567890", name;
    srand(time(0));
    for (int i = 0; i < 10; i++)
    {

        name.push_back(str[rand() % 35]);
    }
    return name;
}
std::string qrcode::save(std::string url, std::string pathfile = "")
{
    if (pathfile.size() > 4)
    {
        if (pathfile.substr(pathfile.size() - 4, pathfile.size()) == ".png")
        {
            for (unsigned int i = 0; i < pathfile.size(); i++)
            {

                if (pathfile[i] == '/')
                {
                    filename.clear();
                    continue;
                }
                filename.push_back(pathfile[i]);
            }
        }
        else
        {
            if (filename.size() > 4 && filename.substr(filename.size() - 4, filename.size()) == ".png")
            {
            }
            else
            {

                filename.append(randname());

                filename.append(".png");
            }
            if (pathfile.back() == '/')
            {
                pathfile.append(filename);
            }
            else
            {
                if (pathfile.size() > 0)
                    pathfile.push_back('/');
                pathfile.append(filename);
            }
        }
    }
    else
    {

        if (filename.size() > 4 && filename.substr(filename.size() - 4, filename.size()) == ".png")
        {
        }
        else
        {

            filename.append(randname());

            filename.append(".png");
        }

        if (pathfile.back() == '/')
        {
            pathfile.append(filename);
        }
        else
        {
            if (pathfile.size() > 0)
                pathfile.push_back('/');
            pathfile.append(filename);
        }
    }

#ifdef ENABLE_GD
    QRcode *qrCode;
    qrCode = QRcode_encodeString(qrtext.c_str(), version, level, hint, casesensitive);
    if (NULL == qrCode)
    {

        return "";
    }
    if (scale == 0)
    {
        scale = 1;
    }
    if (scale > 20)
    {
        scale = 20;
    }
    int beginx = 10;
    int beginy = 10;

    int totalwidth  = qrCode->width * scale + beginx * 2;
    int totalheight = qrCode->width * scale + beginy * 2;

    int pixoffset = 0;
    int owidth    = qrCode->width;

    gdImagePtr im;
    FILE *pngout;

    im = gdImageCreate(totalwidth, totalheight);
    int black;
    int white;
    white = gdImageColorAllocate(im, 255, 255, 255);
    black = gdImageColorAllocate(im, 0, 0, 0);
    for (int i = 0; i < owidth; i++)
    {
        for (int j = 0; j < owidth; j++)
        {

            if (qrCode->data[pixoffset] & 1)
            {
                for (unsigned int n = 0; n < scale; n++)
                {
                    for (unsigned int m = 0; m < scale; m++)
                    {
                        gdImageSetPixel(im, beginx + i * scale + n, beginy + j * scale + m, black);
                    }
                }
            }
            pixoffset += 1;
        }
    }
    try
    {
        pngout = fopen(pathfile.c_str(), "wb");
        gdImagePng(im, pngout);
        fclose(pngout);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    gdImageDestroy(im);
    QRcode_free(qrCode);
#endif
    if (url.back() == '/')
    {
        url.append(filename);
    }
    else
    {
        if (url.size() > 0)
            url.push_back('/');
        url.append(filename);
    }
    return url;
}

}// namespace http