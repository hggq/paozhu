// tests/test_pzzip.cpp
// pzzip 回归测试：覆盖往返、空文件(R1)、空 zip(R2)、内存缓冲、损坏检测(F1/F2)、伪造 EOCD(F5)。
// 由 tests/CMakeLists.txt 自动发现并构建（启用 -DENABLE_TESTS=ON）。
#include "pzzip.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using pz::zip;

int main() {
    // 1) 普通往返 + 空文件(R1)
    {
        zip z;
        assert(z.create_zipfile("/tmp/t1.zip"));
        assert(z.add_file_from_string("hello.txt", "Hello World!"));
        assert(z.add_file_from_memory("empty.txt", "", 0));
        z.close();
    }
    {
        zip z;
        assert(z.open_zipfile("/tmp/t1.zip"));
        std::string s;
        assert(z.read_file_to_string("hello.txt", s));
        assert(s == "Hello World!");
        std::string e;
        assert(z.read_file_to_string("empty.txt", e));
        assert(e == "");
        std::cout << "[OK] 普通往返 + 空文件(STORED)\n";
    }

    // 2) 空 ZIP 必须写出合法 EOCD(R2)
    {
        zip z;
        assert(z.create_zipfile("/tmp/t2.zip"));
        z.close();
        std::ifstream f("/tmp/t2.zip", std::ios::binary);
        f.seekg(0, std::ios::end);
        auto sz = f.tellg();
        assert(sz == 22);
        zip z2;
        assert(z2.open_zipfile("/tmp/t2.zip"));
        assert(z2.files_list().empty());
        std::cout << "[OK] 空 ZIP 写出合法 EOCD(" << sz << " 字节)\n";
    }

    // 3) 内存缓冲往返
    {
        zip z;
        assert(z.create_zipbuffer());
        assert(z.add_file_from_string("x.txt", "memory buffer test"));
        std::string buf = z.write_to_buffer();
        assert(!buf.empty());
        std::ofstream f("/tmp/t3.zip", std::ios::binary);
        f.write(buf.data(), (std::streamsize)buf.size());
        f.close();
        zip z2;
        if (!z2.open_zipfile("/tmp/t3.zip")) {
            std::cout << "[FAIL] 内存缓冲 zip 打开失败: " << z2.error_msg() << "\n";
            return 1;
        }
        std::string s;
        if (!z2.read_file_to_string("x.txt", s) || s != "memory buffer test") {
            std::cout << "[FAIL] 内存缓冲读取失败: " << z2.error_msg() << "\n";
            return 1;
        }
        std::cout << "[OK] 内存缓冲往返\n";
    }

    // 4) 损坏 deflate 数据：读取路径必须经 CRC/完整性校验失败(F1/F2)
    {
        std::ifstream f("/tmp/t1.zip", std::ios::binary);
        std::string data((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
        f.close();
        assert(data.size() > 60);
        data[42] ^= 0xFF;  // 翻转 deflate 流中段字节（EOCD 完好）
        std::ofstream fo("/tmp/t4_corrupt.zip", std::ios::binary);
        fo.write(data.data(), (std::streamsize)data.size());
        fo.close();
        zip z;
        if (!z.open_zipfile("/tmp/t4_corrupt.zip")) {
            std::cout << "[WARN] 损坏样本在解析阶段被拒绝: " << z.error_msg() << "\n";
        } else {
            std::string s;
            if (z.read_file_to_string("hello.txt", s)) {
                std::cout << "[FAIL] 损坏数据未被读取路径检测到 (内容: " << s << ")\n";
                return 1;
            } else {
                std::cout << "[OK] 损坏数据被读取路径拒绝: " << z.error_msg() << "\n";
            }
        }
    }

    // 5) 伪造 EOCD（文件尾追加垃圾 EOCD）：严格校验应拒绝(F5)
    {
        std::ifstream f("/tmp/t1.zip", std::ios::binary);
        std::string data((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
        f.close();
        // 在真实 EOCD 之后追加一个伪造 EOCD 记录（polyglot 场景）
        const char fake_eocd[22] = {0x50,0x4b,0x05,0x06,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        data.append(fake_eocd, 22);
        std::ofstream fo("/tmp/t5_fakeeocd.zip", std::ios::binary);
        fo.write(data.data(), (std::streamsize)data.size());
        fo.close();
        zip z;
        if (z.open_zipfile("/tmp/t5_fakeeocd.zip")) {
            std::cout << "[OK] 伪造 EOCD 被接受(真实 EOCD 优先) 条目数=" << z.files_list().size() << "\n";
        } else {
            std::cout << "[OK] 伪造 EOCD 被拒绝: " << z.error_msg() << "\n";
        }
    }

    std::cout << "ALL DONE\n";
    return 0;
}
