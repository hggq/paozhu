#pragma once

#include <zlib.h>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace pz {

// ZIP 内部条目信息
struct ZipEntry {
    std::string filename;         // 文件名（相对路径）
    uint16_t compression_method;  // 0=存储, 8=DEFLATE
    uint16_t general_purpose_flag = 0; // 通用标志位
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint32_t local_header_offset; // 本地文件头在 ZIP 中的偏移
};

class zip {
public:
    zip() = default;
    ~zip();

    // ==================== 读取 ZIP ====================

    // 打开一个已存在的 ZIP 文件用于读取
    bool open_zipfile(const std::string& filename);

    // 获取 ZIP 中所有文件列表
    std::vector<std::string> files_list() const;

    // 解压全部文件到指定目录
    bool extract_all(const std::string& output_dir = ".");

    // 解压指定文件到指定目录
    bool extract_file(const std::string& filename, const std::string& output_dir = ".");

    // 读取 ZIP 中指定文件到字符串（适合小文本文件如 XML）
    bool read_file_to_string(const std::string& filename, std::string& out);

    // 读取 ZIP 中指定文件到二进制向量（适合图片等二进制文件）
    bool read_file_to_vector(const std::string& filename, std::vector<unsigned char>& out);

    // ==================== 创建 ZIP ====================

    // 创建一个新的 ZIP 文件用于写入
    bool create_zipfile(const std::string& filename);

    // 创建内存 ZIP 输出（新增，不落磁盘）：后续 add_file*/close 全部写入内部 buffer
    bool create_zipbuffer();

    // 结束写入（补写中央目录/EOCD）并取回完整 ZIP 字节流（新增，供 pzword 等免临时文件场景使用）
    // 失败返回空串，原因见 error_msg()
    std::string write_to_buffer();

    // 向 ZIP 中添加一个文件（传入磁盘上的文件路径）
    // stored_name: 在 ZIP 中存储的路径名，为空则自动取 basename
    // streaming: true=流式压缩（大数据用），false=内存压缩（小文件默认）
    bool add_file(const std::string& filepath, const std::string& stored_name = "",
                  bool streaming = false);

    // 向 ZIP 中添加一个内存文件（从字符串，适合 XML 等文本）
    bool add_file_from_string(const std::string& zip_name, const std::string& content);

    // 向 ZIP 中添加一个内存文件（从二进制数据，适合图片）
    bool add_file_from_memory(const std::string& zip_name, const char* data, size_t len);

    // 关闭当前打开的 ZIP 文件
    void close();

    // 设置压缩级别（0=不压缩, 1=最快, 9=最优, 默认 Z_BEST_COMPRESSION）
    void set_compression_level(int level);

    // 设置解压时单文件最大原始大小（字节，默认 100MB，防 zip bomb）
    void set_max_uncompressed_size(uint32_t max_size);

    // 获取最后一次错误信息（成功时为空）
    const std::string& error_msg() const { return error_msg_; }

    // 路径安全过滤：防止目录穿越攻击（如 ../）
    // 返回安全的纯文件名（去掉路径中的危险成分）
    static std::string safe_filename(const std::string& path);

private:

    // 获取当前时间（MS-DOS 格式）
    static void msdos_time(uint16_t& mod_time, uint16_t& mod_date);

    // 计算 CRC32
    static uint32_t calc_crc32(const char* data, size_t len);

    // ========== 读取相关 ==========
    bool find_eocd();
    bool parse_central_directory();

    // ========== 写入相关 ==========
    bool write_local_header(const std::string& zip_name, uint32_t uncompressed_size,
                            uint32_t crc32, uint32_t compressed_size,
                            bool use_data_descriptor = false);
    bool write_central_directory();
    bool write_eocd();

    // 统一写出助手（新增）：内存模式追加到 buffer_，文件模式写 write_stream_
    bool append_(const void* data, size_t len);
    // 是否处于可写状态（文件模式流已打开，或内存模式已创建）
    bool can_write_() const;
    // 重置写入状态（create_zipfile / create_zipbuffer 共用）
    void begin_write_mode_();

    // ========== 成员变量 ==========
    std::ifstream read_stream_;
    std::ofstream write_stream_;
    std::vector<ZipEntry> entries_;
    std::unordered_map<std::string, size_t> entry_index_;  // filename → entries_ 下标

    // 内存输出模式（新增）：true 时写出目标为 buffer_ 而非 write_stream_
    bool is_memory_mode_ = false;
    std::string buffer_;        // 内存 ZIP 输出缓冲区
    uint32_t write_pos_ = 0;    // 当前写出位置（等价于文件模式的 tellp）

    // 写入状态
    std::vector<ZipEntry> write_entries_;
    uint32_t central_dir_offset_ = 0;

    // EOCD 信息
    uint16_t total_entries_ = 0;
    uint32_t central_dir_size_ = 0;
    uint32_t central_dir_offset_in_file_ = 0;

    // 是否处于写入模式
    bool is_write_mode_ = false;

    // 压缩级别
    int compression_level_ = Z_BEST_COMPRESSION;

    // 错误信息
    mutable std::string error_msg_;

    // 当前打开的 ZIP 文件大小（用于越界检查）
    uint64_t file_size_ = 0;

    // 解压单文件最大原始大小
    uint32_t max_uncompressed_size_ = 100 * 1024 * 1024;  // 100MB

    // 最大条目数
    static const uint16_t kMaxEntries = 10000;

    // 非流式压缩的文件大小阈值（超过则自动切换为流式）
    static const size_t kMemoryCompressThreshold = 256 * 1024 * 1024;  // 256MB
};

} // namespace pz
