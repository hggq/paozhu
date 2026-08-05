#include "pzzip.h"

#include <zlib.h>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <string_view>

// ==================== ZIP 格式常量 ====================
static const uint32_t LOCAL_FILE_HEADER_SIG   = 0x04034b50;
static const uint32_t CENTRAL_DIR_SIG         = 0x02014b50;
static const uint32_t EOCD_SIG                = 0x06054b50;

static const uint16_t COMPRESSION_STORED      = 0;
static const uint16_t COMPRESSION_DEFLATED    = 8;

namespace pz {

// ==================== 本地文件头结构（packed） ====================
#pragma pack(push, 1)
struct LocalFileHeaderRaw {
    uint32_t signature;
    uint16_t version_needed;
    uint16_t general_purpose_flag;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
};

struct CentralDirEntryRaw {
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t general_purpose_flag;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_attrs;
    uint32_t external_attrs;
    uint32_t local_header_offset;
};

struct EOCDRaw {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk_cd_start;
    uint16_t cd_entries_on_disk;
    uint16_t cd_entries_total;
    uint32_t cd_size;
    uint32_t cd_offset;
    uint16_t comment_length;
};
#pragma pack(pop)

// ==================== 析构 ====================
zip::~zip() {
    close();
}

// ==================== 辅助函数 ====================

std::string zip::safe_filename(const std::string& path) {
    // 1. 将 "\" 统一转为 "/"，过滤危险字符，截断 null 字节
    std::string s;
    s.reserve(path.size());
    for (char c : path) {
        if (c == '\0') break;
        if (c == '\\') { s += '/'; continue; }
        if (c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|' || ((unsigned char)c < 0x20)) {
            continue;
        }
        s += c;
    }

    // 2. 去掉开头的 "/"（拒绝绝对路径）
    size_t start = s.find_first_not_of('/');
    if (start == std::string::npos) return "unnamed";
    std::string_view sv(s.data() + start, s.size() - start);

    // 3. 按 '/' 分割，拒绝包含 ".." 的路径组件（防目录穿越）
    // 使用 string_view 避免 substr 分配
    std::string result;
    result.reserve(sv.size());
    size_t pos = 0;
    while (pos < sv.size()) {
        size_t slash = sv.find('/', pos);
        std::string_view comp = (slash == std::string_view::npos)
                                ? sv.substr(pos)
                                : sv.substr(pos, slash - pos);
        if (comp.find("..") != std::string_view::npos) {
            // 任何包含 ".." 的组件均视为危险，直接拒绝
            return "unnamed";
        } else if (comp == "." || comp.empty()) {
            // 跳过当前目录和空组件
        } else {
            if (!result.empty()) result += '/';
            result += comp;
        }
        if (slash == std::string_view::npos) break;
        pos = slash + 1;
    }

    return result.empty() ? "unnamed" : result;
}

void zip::msdos_time(uint16_t& mod_time, uint16_t& mod_date) {
    std::time_t t = std::time(nullptr);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);  // Windows (MSVC)
#else
    localtime_r(&t, &tm_buf);  // POSIX (Linux / macOS)
#endif
    std::tm* tm = &tm_buf;

    mod_time = static_cast<uint16_t>(
        (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2)
    );
    mod_date = static_cast<uint16_t>(
        ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday
    );
}

uint32_t zip::calc_crc32(const char* data, size_t len) {
    uint32_t crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(len));
    return crc;
}

// ==================== 关闭 ====================
void zip::close() {
    if (is_write_mode_) {
        bool writable = is_memory_mode_ || write_stream_.is_open();
        if (writable && !write_entries_.empty()) {
            // 先写入中央目录和 EOCD
            write_central_directory();
            write_eocd();
        }
        if (write_stream_.is_open()) {
            write_stream_.close();
        }
    }
    if (read_stream_.is_open()) {
        read_stream_.close();
    }
    entries_.clear();
    entry_index_.clear();
    write_entries_.clear();
    central_dir_offset_ = 0;
    total_entries_ = 0;
    central_dir_size_ = 0;
    central_dir_offset_in_file_ = 0;
    is_write_mode_ = false;
    is_memory_mode_ = false;
    buffer_.clear();
    write_pos_ = 0;
    file_size_ = 0;
}

void zip::set_compression_level(int level) {
    if (level < 0) level = 0;
    if (level > 9) level = 9;
    compression_level_ = level;
}

void zip::set_max_uncompressed_size(uint32_t max_size) {
    max_uncompressed_size_ = max_size;
}

// ==================== 读取 ZIP ====================

bool zip::find_eocd() {
    read_stream_.seekg(0, std::ios::end);
    auto fsize = read_stream_.tellg();
    if (fsize < 22) return false;
    file_size_ = static_cast<uint64_t>(fsize);

    // 在文件末尾 65535 + 22 字节内搜索 EOCD 签名
    // 使用小块窗口逐次回退扫描，避免一次性分配 64KB+ 的缓冲区
    size_t search_size = std::min<size_t>(static_cast<size_t>(fsize), 65535 + 22);
    constexpr size_t kWindowSize = 4096 + 22;  // 每次读取一个窗口
    char buf[kWindowSize];

    long remaining = static_cast<long>(search_size);
    while (remaining >= 22) {
        long chunk = std::min<long>(remaining, static_cast<long>(kWindowSize));
        read_stream_.seekg(-chunk, std::ios::cur);
        size_t read_bytes = static_cast<size_t>(chunk);
        read_stream_.read(buf, read_bytes);
        if (!read_stream_.good() && !read_stream_.eof()) return false;

        // 从后往前搜索 EOCD 签名
        long start = static_cast<long>(read_bytes) - 22;
        for (long i = start; i >= 0; --i) {
            uint32_t sig;
            std::memcpy(&sig, &buf[i], sizeof(sig));  // 避免未对齐访问
            if (sig == EOCD_SIG) {
                EOCDRaw eocd;
                std::memcpy(&eocd, &buf[i], sizeof(eocd));
                total_entries_ = eocd.cd_entries_total;
                central_dir_size_ = eocd.cd_size;
                central_dir_offset_in_file_ = eocd.cd_offset;

                // 校验中央目录偏移 + 大小不超出文件范围
                uint64_t cd_end = static_cast<uint64_t>(central_dir_offset_in_file_)
                                + static_cast<uint64_t>(central_dir_size_);
                if (cd_end > file_size_) {
                    error_msg_ = "pzzip: 中央目录偏移/大小超出文件范围";
                    return false;
                }
                return true;
            }
        }
        // 回退搜索位置（保留 21 字节重叠区，防止 EOCD 签名跨窗口边界漏检）
        read_stream_.seekg(-(static_cast<long>(read_bytes) - 21), std::ios::cur);
        remaining -= static_cast<long>(read_bytes) - 21;
    }
    return false;
}

bool zip::parse_central_directory() {
    // 校验条目数
    if (total_entries_ > kMaxEntries) {
        error_msg_ = "pzzip: ZIP 条目数 " + std::to_string(total_entries_) + " 超过上限 " + std::to_string(kMaxEntries);
        return false;
    }

    read_stream_.seekg(central_dir_offset_in_file_, std::ios::beg);

    for (uint16_t i = 0; i < total_entries_; ++i) {
        CentralDirEntryRaw raw;
        read_stream_.read(reinterpret_cast<char*>(&raw), sizeof(raw));
        if (!read_stream_) return false;

        if (raw.signature != CENTRAL_DIR_SIG) {
            return false;
        }

        ZipEntry entry;
        entry.compression_method = raw.compression_method;
        entry.general_purpose_flag = raw.general_purpose_flag;
        entry.crc32 = raw.crc32;
        entry.compressed_size = raw.compressed_size;
        entry.uncompressed_size = raw.uncompressed_size;
        entry.local_header_offset = raw.local_header_offset;

        // 校验 local_header_offset 不超出文件范围
        if (entry.local_header_offset >= file_size_) {
            error_msg_ = "pzzip: 条目 local_header_offset 越界";
            continue;  // 跳过无效条目，不中断
        }

        // 读取文件名
        std::string filename;
        if (raw.filename_length > 0) {
            filename.resize(raw.filename_length);
            read_stream_.read(&filename[0], raw.filename_length);
            if (!read_stream_) return false;
        }
        entry.filename = safe_filename(filename);

        // 跳过额外字段和文件注释
        if (raw.extra_field_length > 0) {
            read_stream_.seekg(raw.extra_field_length, std::ios::cur);
        }
        if (raw.file_comment_length > 0) {
            read_stream_.seekg(raw.file_comment_length, std::ios::cur);
        }
        if (!read_stream_) return false;

        entries_.push_back(std::move(entry));
        entry_index_[entries_.back().filename] = entries_.size() - 1;
    }
    return true;
}

bool zip::open_zipfile(const std::string& filename) {
    close();

    read_stream_.open(filename, std::ios::binary);
    if (!read_stream_.is_open()) {
        error_msg_ = "pzzip: 无法打开文件 " + filename;
        return false;
    }

    if (!find_eocd()) {
        error_msg_ = "pzzip: 未找到 EOCD，不是有效的 ZIP 文件";
        close();
        return false;
    }

    if (!parse_central_directory()) {
        error_msg_ = "pzzip: 解析中央目录失败";
        close();
        return false;
    }

    is_write_mode_ = false;
    return true;
}

std::vector<std::string> zip::files_list() const {
    std::vector<std::string> names;
    for (const auto& e : entries_) {
        names.push_back(e.filename);
    }
    return names;
}

bool zip::extract_file(const std::string& filename, const std::string& output_dir) {
    if (!read_stream_.is_open()) return false;

    // 查找匹配的条目（使用索引 O(1) 查找）
    const ZipEntry* entry = nullptr;
    auto it = entry_index_.find(filename);
    if (it != entry_index_.end() && it->second < entries_.size()) {
        entry = &entries_[it->second];
    }
    if (!entry) {
        error_msg_ = "pzzip: 文件 " + filename + " 不在 ZIP 中";
        return false;
    }

    // 安全过滤输出文件名
    std::string safe_name = safe_filename(entry->filename);
    std::string out_path = output_dir + "/" + safe_name;

    // 防符号链接攻击：检查输出路径的每个父组件是否为 symlink
    {
        std::filesystem::path check_path(output_dir);
        std::filesystem::path rel(safe_name);
        std::error_code sym_ec;
        for (const auto& part : rel.parent_path()) {
            check_path /= part;
            if (std::filesystem::is_symlink(check_path, sym_ec)) {
                error_msg_ = "pzzip: 路径包含符号链接，拒绝解压: " + check_path.string();
                return false;
            }
        }
        // 目标文件本身若是已存在的 symlink，拒绝覆盖（防借道写入任意文件）
        if (std::filesystem::is_symlink(out_path, sym_ec)) {
            error_msg_ = "pzzip: 目标文件是符号链接，拒绝解压: " + out_path;
            return false;
        }
    }

    // 确保输出目录存在
    try {
        std::filesystem::create_directories(
            std::filesystem::path(out_path).parent_path()
        );
    } catch (const std::filesystem::filesystem_error& e) {
        error_msg_ = std::string("pzzip: 创建输出目录失败: ") + e.what();
        return false;
    }

    // 读取本地文件头
    read_stream_.seekg(entry->local_header_offset, std::ios::beg);
    LocalFileHeaderRaw local;
    read_stream_.read(reinterpret_cast<char*>(&local), sizeof(local));
    if (!read_stream_) {
        error_msg_ = "pzzip: 读取本地文件头失败";
        return false;
    }

    if (local.signature != LOCAL_FILE_HEADER_SIG) {
        error_msg_ = "pzzip: 本地文件头签名无效";
        return false;
    }

    // 跳过文件名和额外字段
    read_stream_.seekg(local.filename_length + local.extra_field_length, std::ios::cur);
    if (!read_stream_) {
        error_msg_ = "pzzip: 跳过本地文件头变量字段失败";
        return false;
    }

    uint32_t comp_size = entry->compressed_size;
    uint32_t uncomp_size = entry->uncompressed_size;

    // 防 zip bomb：检查解压后大小
    if (uncomp_size > max_uncompressed_size_) {
        error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小 "
                   + std::to_string(uncomp_size) + " 超过上限 "
                   + std::to_string(max_uncompressed_size_);
        return false;
    }

    // 检查压缩数据不超出文件范围
    uint64_t data_end = static_cast<uint64_t>(entry->local_header_offset)
                      + sizeof(LocalFileHeaderRaw)
                      + local.filename_length + local.extra_field_length
                      + comp_size;
    if (data_end > file_size_) {
        error_msg_ = "pzzip: 文件 " + entry->filename + " 压缩数据越界";
        return false;
    }

    // 存储/DEFLATE 之外的压缩方法不支持
    if (entry->compression_method != COMPRESSION_STORED &&
        entry->compression_method != COMPRESSION_DEFLATED) {
        error_msg_ = "pzzip: 不支持的压缩方法 " + std::to_string(entry->compression_method);
        return false;
    }

    // ========== 流式解压（stored 直接分块复制 / DEFLATE 走 inflate）==========
    static const size_t kChunkSize = 64 * 1024;  // 64KB
    const bool need_inflate = (entry->compression_method == COMPRESSION_DEFLATED);

    std::ofstream out(out_path, std::ios::binary);
    if (!out.is_open()) {
        error_msg_ = "pzzip: 无法创建输出文件 " + out_path;
        return false;
    }

    z_stream strm = {};
    if (need_inflate && inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        error_msg_ = "pzzip: zlib inflateInit2 失败";
        return false;
    }

    std::vector<char> in_buf(kChunkSize);
    std::vector<char> out_buf(kChunkSize);
    uint32_t crc_val = crc32(0L, Z_NULL, 0);
    uint64_t total_out = 0;  // 使用 64 位防止溢出绕过 zip bomb 检测
    uint32_t remaining = comp_size;
    bool success = true;

    while (remaining > 0) {
        size_t to_read = std::min<size_t>(remaining, kChunkSize);
        read_stream_.read(in_buf.data(), to_read);
        if (!read_stream_) {
            error_msg_ = "pzzip: 读取压缩数据失败";
            success = false;
            break;
        }
        remaining -= static_cast<uint32_t>(to_read);

        if (!need_inflate) {
            // 存储模式：分块直接写出（避免按 comp_size 一次性分配大缓冲区）
            crc_val = crc32(crc_val, reinterpret_cast<const Bytef*>(in_buf.data()),
                            static_cast<uInt>(to_read));
            total_out += static_cast<uint64_t>(to_read);
            if (total_out > static_cast<uint64_t>(max_uncompressed_size_)) {
                error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小超过上限";
                success = false;
                break;
            }
            out.write(in_buf.data(), to_read);
            if (!out.good()) {
                error_msg_ = "pzzip: 写入解压数据失败";
                success = false;
            }
            continue;
        }

        strm.avail_in = static_cast<uInt>(to_read);
        strm.next_in = reinterpret_cast<Bytef*>(in_buf.data());

        do {
            strm.avail_out = static_cast<uInt>(kChunkSize);
            strm.next_out = reinterpret_cast<Bytef*>(out_buf.data());

            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                error_msg_ = "pzzip: 解压失败 (inflate = " + std::to_string(ret) + ")";
                success = false;
                break;
            }

            size_t have = kChunkSize - strm.avail_out;
            if (have > 0) {
                crc_val = crc32(crc_val, reinterpret_cast<const Bytef*>(out_buf.data()),
                                static_cast<uInt>(have));
                total_out += static_cast<uint64_t>(have);

                if (total_out > static_cast<uint64_t>(max_uncompressed_size_)) {
                    error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小超过上限";
                    success = false;
                    break;
                }

                out.write(out_buf.data(), have);
                if (!out.good()) {
                    error_msg_ = "pzzip: 写入解压数据失败";
                    success = false;
                    break;
                }
            }

            if (ret == Z_STREAM_END) break;
        } while (strm.avail_out == 0);

        if (!success) break;
    }

    if (need_inflate) {
        inflateEnd(&strm);
    }
    out.close();

    if (!success) {
        // 解压失败时删除不完整的输出文件
        std::error_code ec;
        std::filesystem::remove(out_path, ec);
        return false;
    }

    // 校验 CRC32（仅当 data descriptor 标志未设置时允许 CRC=0 跳过）
    bool has_data_descriptor = (entry->general_purpose_flag & (1u << 3)) != 0;
    if (crc_val != entry->crc32 && (entry->crc32 != 0 || !has_data_descriptor)) {
        std::ostringstream oss;
        oss << "pzzip: CRC32 校验失败! 期望 0x" << std::hex << entry->crc32
            << " 实际 0x" << crc_val;
        error_msg_ = oss.str();
        std::error_code ec;
        std::filesystem::remove(out_path, ec);
        return false;
    }

    if (total_out != static_cast<uint64_t>(uncomp_size)) {
        error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小不一致";
        std::error_code ec;
        std::filesystem::remove(out_path, ec);
        return false;
    }

    return true;
}

bool zip::extract_all(const std::string& output_dir) {
    if (!read_stream_.is_open()) return false;

    bool all_ok = true;
    for (const auto& e : entries_) {
        if (!extract_file(e.filename, output_dir)) {
            all_ok = false;
        }
    }
    return all_ok;
}

// ==================== 读取 ZIP 到内存 ====================

bool zip::read_file_to_string(const std::string& filename, std::string& out) {
    std::vector<unsigned char> data;
    if (!read_file_to_vector(filename, data)) {
        return false;
    }
    out.assign(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

bool zip::read_file_to_vector(const std::string& filename, std::vector<unsigned char>& out) {
    if (!read_stream_.is_open()) {
        error_msg_ = "pzzip: ZIP 未打开，无法读取 " + filename;
        return false;
    }

    const ZipEntry* entry = nullptr;
    auto it = entry_index_.find(filename);
    if (it != entry_index_.end() && it->second < entries_.size()) {
        entry = &entries_[it->second];
    }
    if (!entry) {
        error_msg_ = "pzzip: 文件 " + filename + " 不在 ZIP 中";
        return false;
    }

    read_stream_.seekg(entry->local_header_offset, std::ios::beg);
    LocalFileHeaderRaw local;
    read_stream_.read(reinterpret_cast<char*>(&local), sizeof(local));
    if (!read_stream_) {
        error_msg_ = "pzzip: 读取本地文件头失败";
        return false;
    }

    if (local.signature != LOCAL_FILE_HEADER_SIG) {
        error_msg_ = "pzzip: 本地文件头签名无效";
        return false;
    }

    read_stream_.seekg(local.filename_length + local.extra_field_length, std::ios::cur);
    if (!read_stream_) {
        error_msg_ = "pzzip: 跳过本地文件头变量字段失败";
        return false;
    }

    uint32_t comp_size = entry->compressed_size;
    uint32_t uncomp_size = entry->uncompressed_size;

    // 检查压缩数据不超出文件范围
    uint64_t data_end = static_cast<uint64_t>(entry->local_header_offset)
                      + sizeof(LocalFileHeaderRaw)
                      + local.filename_length + local.extra_field_length
                      + comp_size;
    if (data_end > file_size_) {
        error_msg_ = "pzzip: 文件 " + entry->filename + " 压缩数据越界";
        return false;
    }

    if (uncomp_size > max_uncompressed_size_) {
        error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小超过上限";
        return false;
    }

    if (entry->compression_method == COMPRESSION_STORED) {
        // stored 模式压缩大小应等于原始大小，防伪造头导致超大分配
        if (comp_size != uncomp_size) {
            error_msg_ = "pzzip: 文件 " + entry->filename + " stored 条目大小不一致";
            return false;
        }
        out.resize(comp_size);
        read_stream_.read(reinterpret_cast<char*>(out.data()), comp_size);
        if (!read_stream_) {
            error_msg_ = "pzzip: 读取存储数据失败";
            return false;
        }
        return true;
    }

    if (entry->compression_method != COMPRESSION_DEFLATED) {
        error_msg_ = "pzzip: 不支持的压缩方法 " + std::to_string(entry->compression_method);
        return false;
    }

    static const size_t kChunkSize = 64 * 1024;

    std::vector<char> in_buf(kChunkSize);
    std::vector<char> out_buf(kChunkSize);
    uint64_t total_out = 0;
    uint32_t remaining = comp_size;
    out.reserve(uncomp_size);  // 已知上限（受 max_uncompressed_size_ 约束），避免反复重分配

    z_stream strm = {};
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        error_msg_ = "pzzip: zlib inflateInit2 失败";
        return false;
    }

    bool success = true;
    while (remaining > 0) {
        size_t to_read = std::min<size_t>(remaining, kChunkSize);
        read_stream_.read(in_buf.data(), to_read);
        if (!read_stream_) {
            error_msg_ = "pzzip: 读取压缩数据失败";
            success = false;
            break;
        }
        remaining -= static_cast<uint32_t>(to_read);

        strm.avail_in = static_cast<uInt>(to_read);
        strm.next_in = reinterpret_cast<Bytef*>(in_buf.data());

        do {
            strm.avail_out = static_cast<uInt>(kChunkSize);
            strm.next_out = reinterpret_cast<Bytef*>(out_buf.data());

            int ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                error_msg_ = "pzzip: 解压失败 (inflate = " + std::to_string(ret) + ")";
                success = false;
                break;
            }

            size_t have = kChunkSize - strm.avail_out;
            if (have > 0) {
                out.insert(out.end(),
                           reinterpret_cast<unsigned char*>(out_buf.data()),
                           reinterpret_cast<unsigned char*>(out_buf.data()) + have);
                total_out += static_cast<uint64_t>(have);

                if (total_out > static_cast<uint64_t>(max_uncompressed_size_)) {
                    error_msg_ = "pzzip: 文件 " + entry->filename + " 解压后大小超过上限";
                    success = false;
                    break;
                }
            }

            if (ret == Z_STREAM_END) break;
        } while (strm.avail_out == 0);

        if (!success) break;
    }

    inflateEnd(&strm);

    if (!success) {
        out.clear();
        return false;
    }

    return true;
}

// ==================== 创建 ZIP ====================

// 统一写出助手（新增）：内存模式追加到 buffer_，文件模式写 write_stream_
bool zip::append_(const void* data, size_t len) {
    if (len == 0) return true;
    // 非 ZIP64 格式单文件 4GB 上限，防 write_pos_ 32 位回绕产生损坏文件
    if (static_cast<uint64_t>(write_pos_) + len > UINT32_MAX) {
        error_msg_ = "pzzip: 输出超过 4GB 限制（不支持 ZIP64）";
        return false;
    }
    if (is_memory_mode_) {
        buffer_.append(reinterpret_cast<const char*>(data), len);
        write_pos_ = static_cast<uint32_t>(buffer_.size());
        return true;
    }
    write_stream_.write(reinterpret_cast<const char*>(data), len);
    if (!write_stream_.good()) return false;
    // 同步写出位置（等价于 tellp），供 header_offset / central_dir_offset_ 使用
    write_pos_ = static_cast<uint32_t>(write_stream_.tellp());
    return true;
}

bool zip::can_write_() const {
    return is_write_mode_ && (is_memory_mode_ || write_stream_.is_open());
}

void zip::begin_write_mode_() {
    is_write_mode_ = true;
    write_entries_.clear();
    central_dir_offset_ = 0;
    write_pos_ = 0;
}

bool zip::create_zipfile(const std::string& filename) {
    close();

    write_stream_.open(filename, std::ios::binary);
    if (!write_stream_.is_open()) {
        error_msg_ = "pzzip: 无法创建文件 " + filename;
        return false;
    }

    is_memory_mode_ = false;
    begin_write_mode_();
    return true;
}

// 创建内存 ZIP 输出（新增，不落磁盘）
bool zip::create_zipbuffer() {
    close();

    buffer_.clear();
    is_memory_mode_ = true;
    begin_write_mode_();
    return true;
}

// 结束写入并取回完整 ZIP 字节流（新增）
std::string zip::write_to_buffer() {
    if (!is_write_mode_ || !is_memory_mode_) {
        error_msg_ = "pzzip: 请先调用 create_zipbuffer 创建内存 ZIP";
        return std::string();
    }

    if (write_entries_.empty()) {
        error_msg_ = "pzzip: 无条目可输出";
        return std::string();
    }

    if (!write_central_directory() || !write_eocd()) {
        if (error_msg_.empty()) {
            error_msg_ = "pzzip: 写入中央目录/EOCD 失败";
        }
        buffer_.clear();
        is_memory_mode_ = false;
        close();
        return std::string();
    }

    // 先取走字节流；退出内存模式后再 close() 清理状态，避免 close() 重复写中央目录
    std::string result = std::move(buffer_);
    is_memory_mode_ = false;
    close();
    return result;
}

bool zip::write_local_header(const std::string& zip_name, uint32_t uncompressed_size,
                           uint32_t crc, uint32_t compressed_size,
                           bool use_data_descriptor) {
    uint16_t mod_time, mod_date;
    msdos_time(mod_time, mod_date);

    LocalFileHeaderRaw local = {};
    local.signature = LOCAL_FILE_HEADER_SIG;
    local.version_needed = 20;  // 2.0
    local.general_purpose_flag = use_data_descriptor ? (1u << 3) : 0;  // bit 3 = data descriptor
    local.compression_method = COMPRESSION_DEFLATED;
    local.last_mod_time = mod_time;
    local.last_mod_date = mod_date;
    local.crc32 = use_data_descriptor ? 0 : crc;
    local.compressed_size = use_data_descriptor ? 0 : compressed_size;
    local.uncompressed_size = use_data_descriptor ? 0 : uncompressed_size;
    if (zip_name.size() > 256) {
        error_msg_ = "pzzip: 文件名超过256字符限制: " + zip_name;
        return false;
    }
    local.filename_length = static_cast<uint16_t>(zip_name.size());
    local.extra_field_length = 0;

    if (!append_(&local, sizeof(local))) return false;
    if (!append_(zip_name.data(), zip_name.size())) {
        error_msg_ = "pzzip: 写入本地文件头失败";
        return false;
    }
    return true;
}

bool zip::add_file(const std::string& filepath, const std::string& stored_name, bool streaming) {
    if (!can_write_()) {
        error_msg_ = "pzzip: 请先调用 create_zipfile / create_zipbuffer 创建 ZIP";
        return false;
    }
    if (write_entries_.size() >= kMaxEntries) {
        error_msg_ = "pzzip: ZIP 条目数超过上限 " + std::to_string(kMaxEntries);
        return false;
    }

    // 确定 ZIP 内存储的文件名
    std::string zip_name;
    if (!stored_name.empty()) {
        zip_name = safe_filename(stored_name);
    } else {
        auto pos = filepath.find_last_of("/\\");
        if (pos != std::string::npos) {
            zip_name = safe_filename(filepath.substr(pos + 1));
        } else {
            zip_name = safe_filename(filepath);
        }
    }

    // 打开源文件，获取大小
    std::ifstream src(filepath, std::ios::binary | std::ios::ate);
    if (!src.is_open()) {
        error_msg_ = "pzzip: 无法打开源文件 " + filepath;
        return false;
    }
    size_t file_size = static_cast<size_t>(src.tellg());
    src.seekg(0, std::ios::beg);

    // ZIP 格式不支持 ZIP64, 文件大小不超过 4GB
    if (file_size > UINT32_MAX) {
        error_msg_ = "pzzip: 文件大小超过 4GB 限制: " + filepath;
        return false;
    }

    uint32_t crc = 0;
    uint32_t compressed_size = 0;
    uint32_t header_offset = write_pos_;

    // 文件超过阈值时自动切换为流式压缩，防止内存耗尽
    if (!streaming && file_size > kMemoryCompressThreshold) {
        streaming = true;
    }

    if (!streaming) {
        // ========== 内存压缩模式（小文件） ==========
        std::vector<char> data(file_size);
        src.read(data.data(), file_size);
        src.close();

        crc = calc_crc32(data.data(), data.size());

        uLongf comp_bound = compressBound(static_cast<uLongf>(file_size));
        std::vector<char> compressed(comp_bound);

        z_stream strm = {};
        if (deflateInit2(&strm, compression_level_, Z_DEFLATED,
                         -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
            error_msg_ = "pzzip: deflateInit2 失败";
            return false;
        }

        strm.avail_in = static_cast<uInt>(file_size);
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
        strm.avail_out = static_cast<uInt>(comp_bound);
        strm.next_out = reinterpret_cast<Bytef*>(compressed.data());

        int ret = deflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            error_msg_ = "pzzip: 压缩失败 (deflate = " + std::to_string(ret) + ")";
            deflateEnd(&strm);
            return false;
        }

        if (strm.total_out > UINT32_MAX) {
            error_msg_ = "pzzip: 压缩后大小超过 4GB 限制: " + filepath;
            deflateEnd(&strm);
            return false;
        }
        compressed_size = static_cast<uint32_t>(strm.total_out);
        deflateEnd(&strm);

        if (!write_local_header(zip_name, static_cast<uint32_t>(file_size), crc, compressed_size)) {
            return false;
        }
        if (!append_(compressed.data(), compressed_size)) {
            error_msg_ = "pzzip: 写入文件失败";
            return false;
        }
    } else {
        // ========== 流式压缩模式（大数据） ==========
        static const size_t kChunkSize = 64 * 1024;  // 64KB

        // 写入本地文件头（bit 3=1, sizes=0, 使用数据描述符）
        if (!write_local_header(zip_name, 0, 0, 0, true)) {
            return false;
        }

        z_stream strm = {};
        if (deflateInit2(&strm, compression_level_, Z_DEFLATED,
                         -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
            error_msg_ = "pzzip: deflateInit2 失败";
            return false;
        }

        std::vector<char> in_buf(kChunkSize);
        std::vector<char> out_buf(kChunkSize);
        uint32_t crc_val = crc32(0L, Z_NULL, 0);
        bool success = true;

        for (;;) {
            src.read(in_buf.data(), kChunkSize);
            size_t bytes_read = static_cast<size_t>(src.gcount());
            if (bytes_read == 0) break;
            bool is_last = (bytes_read < kChunkSize);

            crc_val = crc32(crc_val, reinterpret_cast<const Bytef*>(in_buf.data()),
                            static_cast<uInt>(bytes_read));

            strm.avail_in = static_cast<uInt>(bytes_read);
            strm.next_in = reinterpret_cast<Bytef*>(in_buf.data());

            int flush = is_last ? Z_FINISH : Z_NO_FLUSH;

            do {
                strm.avail_out = static_cast<uInt>(kChunkSize);
                strm.next_out = reinterpret_cast<Bytef*>(out_buf.data());

                int ret = deflate(&strm, flush);
                if (ret == Z_STREAM_ERROR) {
                    error_msg_ = "pzzip: 流式压缩失败";
                    success = false;
                    break;
                }

                size_t have = kChunkSize - strm.avail_out;
                if (!append_(out_buf.data(), have)) {
                    error_msg_ = "pzzip: 写入压缩数据失败";
                    success = false;
                    break;
                }

                if (ret == Z_STREAM_END) break;
            } while (strm.avail_out == 0);

            if (!success) break;
            if (is_last) break;  // Z_FINISH 已完成
        }

        // 文件大小恰好为 chunk 整数倍时，循环以 bytes_read==0 退出，
        // 此时 Z_FINISH 尚未调用，需要最终化 deflate 流
        if (success) {
            strm.avail_in = 0;
            strm.next_in = nullptr;
            int ret;
            do {
                strm.avail_out = static_cast<uInt>(kChunkSize);
                strm.next_out = reinterpret_cast<Bytef*>(out_buf.data());
                ret = deflate(&strm, Z_FINISH);
                if (ret == Z_STREAM_ERROR) {
                    error_msg_ = "pzzip: 流式压缩最终化失败";
                    success = false;
                    break;
                }
                size_t have = kChunkSize - strm.avail_out;
                if (have > 0) {
                    if (!append_(out_buf.data(), have)) {
                        error_msg_ = "pzzip: 写入压缩数据失败";
                        success = false;
                        break;
                    }
                }
            } while (ret != Z_STREAM_END);
        }

        crc = crc_val;
        if (strm.total_out > UINT32_MAX) {
            error_msg_ = "pzzip: 流式压缩后大小超过 4GB 限制: " + filepath;
            deflateEnd(&strm);
            return false;
        }
        compressed_size = static_cast<uint32_t>(strm.total_out);
        deflateEnd(&strm);
        src.close();

        if (!success) return false;

        // 写入数据描述符（CRC32 + compressed_size + uncompressed_size）
        static const uint32_t kDataDescriptorSig = 0x08074b50;
        uint32_t uncompressed_size32 = static_cast<uint32_t>(file_size);
        if (!append_(&kDataDescriptorSig, 4) || !append_(&crc, 4) ||
            !append_(&compressed_size, 4) || !append_(&uncompressed_size32, 4)) {
            error_msg_ = "pzzip: 写入数据描述符失败";
            return false;
        }
    }

    // 记录条目信息
    ZipEntry entry;
    entry.filename = zip_name;
    entry.compression_method = COMPRESSION_DEFLATED;
    entry.general_purpose_flag = streaming ? (1u << 3) : 0;  // 与本地文件头保持一致
    entry.crc32 = crc;
    entry.compressed_size = compressed_size;
    entry.uncompressed_size = static_cast<uint32_t>(file_size);
    entry.local_header_offset = header_offset;
    write_entries_.push_back(std::move(entry));

    central_dir_offset_ = write_pos_;
    return true;
}

bool zip::add_file_from_string(const std::string& zip_name, const std::string& content) {
    return add_file_from_memory(zip_name, content.data(), content.size());
}

bool zip::add_file_from_memory(const std::string& zip_name, const char* data, size_t len) {
    if (!can_write_()) {
        error_msg_ = "pzzip: 请先调用 create_zipfile / create_zipbuffer 创建 ZIP";
        return false;
    }
    if (write_entries_.size() >= kMaxEntries) {
        error_msg_ = "pzzip: ZIP 条目数超过上限 " + std::to_string(kMaxEntries);
        return false;
    }
    // 非 ZIP64 格式 4GB 限制，须在 calc_crc32（uInt 参数）之前拦截
    if (len > UINT32_MAX) {
        error_msg_ = "pzzip: 数据大小超过 4GB 限制";
        return false;
    }

    std::string safe_name = safe_filename(zip_name);
    if (safe_name.empty()) safe_name = "unnamed";

    uint32_t crc = calc_crc32(data, len);
    uint32_t compressed_size = 0;
    uint32_t header_offset = write_pos_;

    if (len == 0) {
        if (!write_local_header(safe_name, 0, 0, 0)) {
            if (error_msg_.empty()) {
                error_msg_ = "pzzip: 写入本地文件头失败";
            }
            return false;
        }
        ZipEntry entry;
        entry.filename = safe_name;
        entry.compression_method = COMPRESSION_DEFLATED;
        entry.general_purpose_flag = 0;
        entry.crc32 = 0;
        entry.compressed_size = 0;
        entry.uncompressed_size = 0;
        entry.local_header_offset = header_offset;
        write_entries_.push_back(std::move(entry));
        central_dir_offset_ = write_pos_;
        return true;
    }

    if (len > kMemoryCompressThreshold) {
        error_msg_ = "pzzip: 内存数据过大，建议使用 add_file() 从文件添加";
        return false;
    }

    uLongf comp_bound = compressBound(static_cast<uLongf>(len));
    std::vector<char> compressed(comp_bound);

    z_stream strm = {};
    if (deflateInit2(&strm, compression_level_, Z_DEFLATED,
                     -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
        error_msg_ = "pzzip: deflateInit2 失败";
        return false;
    }

    strm.avail_in = static_cast<uInt>(len);
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data));
    strm.avail_out = static_cast<uInt>(comp_bound);
    strm.next_out = reinterpret_cast<Bytef*>(compressed.data());

    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        error_msg_ = "pzzip: 压缩失败 (deflate = " + std::to_string(ret) + ")";
        deflateEnd(&strm);
        return false;
    }

    compressed_size = static_cast<uint32_t>(strm.total_out);
    deflateEnd(&strm);

    if (!write_local_header(safe_name, static_cast<uint32_t>(len), crc, compressed_size)) {
        return false;
    }
    if (!append_(compressed.data(), compressed_size)) {
        error_msg_ = "pzzip: 写入文件失败";
        return false;
    }

    ZipEntry entry;
    entry.filename = safe_name;
    entry.compression_method = COMPRESSION_DEFLATED;
    entry.general_purpose_flag = 0;
    entry.crc32 = crc;
    entry.compressed_size = compressed_size;
    entry.uncompressed_size = static_cast<uint32_t>(len);
    entry.local_header_offset = header_offset;
    write_entries_.push_back(std::move(entry));

    central_dir_offset_ = write_pos_;
    return true;
}

bool zip::write_central_directory() {
    // 顺序追加写出：文件模式 seek 到记录位置，内存模式 write_pos_ 恰好等于 central_dir_offset_
    if (!is_memory_mode_) {
        write_stream_.seekp(central_dir_offset_, std::ios::beg);
    }

    // 中央目录所有条目使用同一时间戳，循环外调用一次即可
    uint16_t mod_time, mod_date;
    msdos_time(mod_time, mod_date);

    for (const auto& entry : write_entries_) {

        CentralDirEntryRaw cd = {};
        cd.signature = CENTRAL_DIR_SIG;
        cd.version_made_by = 20;
        cd.version_needed = 20;
        cd.general_purpose_flag = entry.general_purpose_flag;  // 与本地文件头一致
        cd.compression_method = entry.compression_method;
        cd.last_mod_time = mod_time;
        cd.last_mod_date = mod_date;
        cd.crc32 = entry.crc32;
        cd.compressed_size = entry.compressed_size;
        cd.uncompressed_size = entry.uncompressed_size;
        if (entry.filename.size() > 256) {
            error_msg_ = "pzzip: 文件名超过256字符限制: " + entry.filename;
            return false;
        }
        cd.filename_length = static_cast<uint16_t>(entry.filename.size());
        cd.extra_field_length = 0;
        cd.file_comment_length = 0;
        cd.disk_number_start = 0;
        cd.internal_attrs = 0;
        cd.external_attrs = 0;
        cd.local_header_offset = entry.local_header_offset;

        if (!append_(&cd, sizeof(cd)) || !append_(entry.filename.data(), entry.filename.size())) {
            error_msg_ = "pzzip: 写入中央目录失败";
            return false;
        }
    }

    return true;
}

bool zip::write_eocd() {
    uint32_t cd_start = central_dir_offset_;
    uint32_t cd_end = write_pos_;
    uint32_t cd_size = cd_end - cd_start;

    EOCDRaw eocd = {};
    eocd.signature = EOCD_SIG;
    eocd.disk_number = 0;
    eocd.disk_cd_start = 0;
    eocd.cd_entries_on_disk = static_cast<uint16_t>(write_entries_.size());
    eocd.cd_entries_total = static_cast<uint16_t>(write_entries_.size());
    eocd.cd_size = cd_size;
    eocd.cd_offset = cd_start;
    eocd.comment_length = 0;

    if (!append_(&eocd, sizeof(eocd))) {
        error_msg_ = "pzzip: 写入 EOCD 失败";
        return false;
    }
    return true;
}

} // namespace pz
