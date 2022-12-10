#ifndef _qrcode_H_
#define _qrcode_H_ 

#include <cstdio>
#include <cstring>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <memory>
#include <qrencode.h>
#include "gd.h"
#include "qrcode.h"
 
//g++ libqrencode.cpp -std=c++20 -lgd -lpng -lz -ljpeg -lfreetype -lm -lqrencode
namespace http {
    class qrcode{

        public:
            qrcode();
            qrcode(std::string text):qrtext(text){}
            qrcode(std::string text,std::string pathfile):qrtext(text),filename(pathfile){
            }
            qrcode(std::string text,unsigned char scale):qrtext(text),scale(scale){
            }
            qrcode(std::string text,std::string pathfile,unsigned char scale):qrtext(text),filename(pathfile),scale(scale){
            }
            void setScale(unsigned char);
            std::string randname(unsigned char);
            std::string save(std::string,std::string);
        public:
            std::string  qrtext;
            std::string  filename;
            int version=5;
            QRecLevel level = QR_ECLEVEL_H;
            QRencodeMode hint = QR_MODE_8;
            int casesensitive = 1;

            unsigned scale=1;
    };
}
#endif 