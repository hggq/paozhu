#pragma once
#ifndef PZ_EXCEL_H
#define PZ_EXCEL_H

#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <list>
#include <string_view>
#include <cstddef>

#ifdef ENABLE_MINIZIP
namespace pz
{
struct PZ_EXCEL_CELL_STYLE
{
    unsigned char numfmtid = 0;
    unsigned char fontid   = 0;
    unsigned char fillid   = 0;
    unsigned char borderid = 0;
    unsigned char xfid     = 0;

    bool apply_fmt    = false;
    bool apply_font   = false;
    bool apply_fill   = false;
    bool apply_align  = false;
    bool apply_border = false;

    std::string align;
    std::string valign;
};

struct PZ_EXCEL_CELL_FONT
{
    bool bold                = false;
    bool italic              = false;
    bool strike              = false;
    unsigned char size       = 0;
    unsigned char color_type = 0;
    std::string underline;
    std::string color;
    std::string name;
    std::string family;
    std::string bgcolor;
    std::string charset;
    std::string scheme;
};

struct PZ_EXCEL_CELL_BORDER_SIDE
{
    std::string style;
    std::string color;
};

struct PZ_EXCEL_CELL_BORDER
{
    PZ_EXCEL_CELL_BORDER_SIDE top;
    PZ_EXCEL_CELL_BORDER_SIDE left;
    PZ_EXCEL_CELL_BORDER_SIDE right;
    PZ_EXCEL_CELL_BORDER_SIDE bottom;
    PZ_EXCEL_CELL_BORDER_SIDE diagonal;
};

struct PZ_EXCEL_CELL_FILL
{
    unsigned char bg_type = 0;
    std::string pattern_type;
    std::string fgColor;
    std::string bgcolor;
};

struct PZ_EXCEL_CELL
{
    unsigned char v_type  = 0;
    unsigned char t_type  = 0;
    unsigned char s_type  = 0;
    unsigned char width   = 0;
    unsigned char heigh   = 0;
    unsigned char h_align = 0;
    unsigned char v_align = 0;
    union PZ_EXCEL_CELL_VALUE_UN
    {
        long long i;
        double d;
        PZ_EXCEL_CELL_VALUE_UN() : i(0) {}
    } value;
};

struct PZ_EXCEL_SHEET
{
    std::map<unsigned int, std::map<unsigned int, PZ_EXCEL_CELL>> values;
    unsigned int rows        = 0;
    unsigned int cols        = 0;
    unsigned int sheet_index = 0;
    std::string name;
    std::string sheet_file;
    std::map<unsigned long long, PZ_EXCEL_CELL_FONT> fonts;
};

class excel
{
  public:
    excel();
    bool read(const std::string &);
    bool write(const std::string &);
    void clear();

    template <typename T, typename std::enable_if<std::is_floating_point_v<T>, bool>::type = true>
    excel &setCell(unsigned int row_num, unsigned int col_num, T value)
    {
        if (row_num == 0 || col_num == 0)
            return *this;
        if (row_num > MAX_ROWS || col_num > MAX_COLS)
            return *this;
        if (sheets.empty())
            addSheet("Sheet1");
        if (index >= sheets.size())
            index = 0;
        PZ_EXCEL_CELL temp_cell;
        temp_cell.v_type                       = 3;
        temp_cell.value.d                      = static_cast<double>(value);
        sheets[index].values[row_num][col_num] = std::move(temp_cell);
        if (row_num > sheets[index].rows)
            sheets[index].rows = row_num;
        if (col_num > sheets[index].cols)
            sheets[index].cols = col_num;
        return *this;
    }

    template <typename T, typename std::enable_if<std::is_integral_v<T>, bool>::type = true>
    excel &setCell(unsigned int row_num, unsigned int col_num, T value)
    {
        if (row_num == 0 || col_num == 0)
            return *this;
        if (row_num > MAX_ROWS || col_num > MAX_COLS)
            return *this;
        if (sheets.empty())
            addSheet("Sheet1");
        if (index >= sheets.size())
            index = 0;
        PZ_EXCEL_CELL temp_cell;
        temp_cell.v_type                       = 0;
        temp_cell.value.i                      = static_cast<long long>(value);
        sheets[index].values[row_num][col_num] = std::move(temp_cell);
        if (row_num > sheets[index].rows)
            sheets[index].rows = row_num;
        if (col_num > sheets[index].cols)
            sheets[index].cols = col_num;
        return *this;
    }

    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, bool>::type = true>
    excel &setCell(unsigned int row_num, unsigned int col_num, const T &value)
    {
        if (row_num == 0 || col_num == 0)
            return *this;
        if (row_num > MAX_ROWS || col_num > MAX_COLS)
            return *this;
        if (sheets.empty())
            addSheet("Sheet1");
        if (index >= sheets.size())
            index = 0;
        size_t sidx = find_shared_string(value);
        if (sidx == SIZE_MAX)
        {
            sidx = shared_strings.size();
            if (sidx >= MAX_SHARED_STRINGS)
                return *this;
            shared_strings.push_back(value);
        }
        PZ_EXCEL_CELL temp_cell;
        temp_cell.v_type                       = 5;
        temp_cell.value.i                      = static_cast<long long>(sidx);
        sheets[index].values[row_num][col_num] = std::move(temp_cell);
        if (row_num > sheets[index].rows)
            sheets[index].rows = row_num;
        if (col_num > sheets[index].cols)
            sheets[index].cols = col_num;
        return *this;
    }

    template <typename T>
    excel &setCell(const std::string &key, T value)
    {
        auto [row_num, col_num] = cellstr_to_num(key);
        return setCell<T>(row_num, col_num, value);
    }

    //设置单元格 文字内容颜色 利用PZ_EXCEL_CELL_STYLE PZ_EXCEL_CELL_STYLE
    void setCellColor(const std::string &key, std::string_view value);

    excel &addSheet(const std::string &name);

    template <typename T, typename std::enable_if<std::is_floating_point_v<T>, bool>::type = true>
    double getCell(unsigned int row_num, unsigned int col_num)
    {
        if (index >= sheets.size())
            return 0.0;
        auto it = sheets[index].values.find(row_num);
        if (it != sheets[index].values.end())
        {
            auto cell_it = it->second.find(col_num);
            if (cell_it != it->second.end())
            {
                if (cell_it->second.v_type < 3)
                {
                    return static_cast<T>(cell_it->second.value.i);
                }
                else if (cell_it->second.v_type < 5)
                {
                    return cell_it->second.value.d;
                }
                else if (cell_it->second.v_type < 7)
                {
                    size_t idx = static_cast<size_t>(cell_it->second.value.i);
                    if (idx >= 0 && idx < shared_strings.size())
                    {
                        try
                        {
                            return std::stold(shared_strings[idx].c_str());
                        }
                        catch (const std::exception &)
                        {
                            return 0.0;
                        }
                    }
                }
                else if (cell_it->second.v_type < 10)
                {
                    return 0.0;
                }
            }
        }
        return 0.0;
    }

    template <typename T, typename std::enable_if<std::is_integral_v<T>, bool>::type = true>
    T getCell(unsigned int row_num, unsigned int col_num)
    {
        if (index >= sheets.size())
            return 0;
        auto it = sheets[index].values.find(row_num);
        if (it != sheets[index].values.end())
        {
            auto cell_it = it->second.find(col_num);
            if (cell_it != it->second.end())
            {
                if (cell_it->second.v_type < 3)
                {
                    return static_cast<T>(cell_it->second.value.i);
                }
                else if (cell_it->second.v_type < 5)
                {
                    return static_cast<T>(cell_it->second.value.d);
                }
                else if (cell_it->second.v_type < 7)
                {
                    size_t idx = static_cast<size_t>(cell_it->second.value.i);
                    if (idx >= 0 && idx < shared_strings.size())
                    {
                        try
                        {
                            return static_cast<T>(std::stoll(shared_strings[idx].c_str()));
                        }
                        catch (const std::exception &)
                        {
                            return 0;
                        }
                    }
                }
                else if (cell_it->second.v_type < 10)
                {
                    return 0;
                }
            }
        }
        return 0;
    }

    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, bool>::type = true>
    T getCell(unsigned int row_num, unsigned int col_num)
    {
        if (index >= sheets.size())
            return "";
        auto it = sheets[index].values.find(row_num);
        if (it != sheets[index].values.end())
        {
            auto cell_it = it->second.find(col_num);
            if (cell_it != it->second.end())
            {
                if (cell_it->second.v_type < 3)
                {
                    return std::to_string(cell_it->second.value.i);
                }
                else if (cell_it->second.v_type < 5)
                {
                    return std::to_string(cell_it->second.value.d);
                }
                else if (cell_it->second.v_type < 7)
                {
                    size_t idx = static_cast<size_t>(cell_it->second.value.i);
                    if (idx >= 0 && idx < shared_strings.size())
                    {
                        return shared_strings[idx];
                    }
                }
                else if (cell_it->second.v_type < 10)
                {
                    return "";
                }
            }
        }
        return "";
    }

    template <typename T, typename std::enable_if<std::is_floating_point_v<T>, bool>::type = true>
    double getCell(const std::string &key)
    {
        auto [row_num, col_num] = cellstr_to_num(key);
        return getCell<T>(row_num, col_num);
    }

    template <typename T, typename std::enable_if<std::is_integral_v<T>, bool>::type = true>
    T getCell(const std::string &key)
    {
        auto [row_num, col_num] = cellstr_to_num(key);
        return getCell<T>(row_num, col_num);
    }

    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, bool>::type = true>
    T getCell(const std::string &key)
    {
        auto [row_num, col_num] = cellstr_to_num(key);
        return getCell<T>(row_num, col_num);
    }

    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, bool>::type = true>
    std::vector<T> getRangeVector(const std::string &key)
    {
        std::vector<std::string> vec;
        if (index >= sheets.size())
            return vec;
        std::string begin_key, end_key;
        size_t offset = 0;
        for (; offset < key.size(); offset++)
        {
            if (key[offset] == ':')
            {
                offset++;
                break;
            }
            begin_key.push_back(key[offset]);
        }
        for (; offset < key.size(); offset++)
        {
            end_key.push_back(key[offset]);
        }
        auto [row_a_begin, col_a_end] = cellstr_to_num(begin_key);
        auto [row_b_begin, col_b_end] = cellstr_to_num(end_key);

        if (row_a_begin == row_b_begin || col_a_end == col_b_end)
        {
            if (row_a_begin == row_b_begin)
            {
                auto it = sheets[index].values.find(row_b_begin);
                if (it != sheets[index].values.end())
                {
                    for (; col_a_end <= col_b_end && col_a_end > 0; col_a_end++)
                    {
                        auto cell_it = it->second.find(col_a_end);
                        std::string temp_str;
                        if (cell_it != it->second.end())
                        {
                            if (cell_it->second.v_type < 3)
                            {
                                temp_str = std::to_string(cell_it->second.value.i);
                            }
                            else if (cell_it->second.v_type < 5)
                            {
                                temp_str = std::to_string(cell_it->second.value.d);
                            }
                            else if (cell_it->second.v_type < 7)
                            {
                                size_t idx = static_cast<size_t>(cell_it->second.value.i);
                                if (idx < shared_strings.size())
                                {
                                    temp_str = shared_strings[idx];
                                }
                            }
                        }
                        vec.push_back(temp_str);
                    }
                }
            }
            else if (col_a_end == col_b_end)
            {
                for (; row_a_begin <= row_b_begin && row_a_begin > 0; row_a_begin++)
                {
                    auto it = sheets[index].values.find(row_a_begin);
                    if (it != sheets[index].values.end())
                    {
                        auto cell_it = it->second.find(col_a_end);
                        std::string temp_str;
                        if (cell_it != it->second.end())
                        {
                            if (cell_it->second.v_type < 3)
                            {
                                temp_str = std::to_string(cell_it->second.value.i);
                            }
                            else if (cell_it->second.v_type < 5)
                            {
                                temp_str = std::to_string(cell_it->second.value.d);
                            }
                            else if (cell_it->second.v_type < 7)
                            {
                                size_t idx = static_cast<size_t>(cell_it->second.value.i);
                                if (idx < shared_strings.size())
                                {
                                    temp_str = shared_strings[idx];
                                }
                            }
                        }
                        vec.push_back(temp_str);
                    }
                }
            }
        }
        return vec;
    }

    std::string getCell(const std::string &key);
    std::string getCell(unsigned int, unsigned int);
    std::tuple<unsigned int, unsigned int> cellstr_to_num(const std::string &key);
    std::string getDate(const std::string &key, const std::string &date_format);
    unsigned int getUnixTime(const std::string &key);
    excel &setIndex(unsigned int);
    excel &setActiveSheet(unsigned int);
    excel &setActiveName(const std::string &key);
    size_t getSheetCount() const { return sheets.size(); }
    const std::string &getSheetName(unsigned int idx) const
    {
        static const std::string empty;
        return idx < sheets.size() ? sheets[idx].name : empty;
    }

  public:
    std::vector<std::string> shared_strings;
    std::map<std::string, std::string> workbook_mapfiles;
    std::vector<PZ_EXCEL_SHEET> sheets;
    std::string error_msg;

    std::map<std::string, std::string> numfmts;

    std::vector<PZ_EXCEL_CELL_FONT> fonts;
    std::vector<PZ_EXCEL_CELL_FILL> bgcolors;
    std::vector<PZ_EXCEL_CELL_BORDER> borders;

    std::vector<PZ_EXCEL_CELL_STYLE> styles;

    static const unsigned int MAX_ROWS     = 1048576;
    static const unsigned int MAX_COLS     = 16384;
    static const size_t MAX_SHARED_STRINGS = 1000000;
    static const size_t MAX_FILE_SIZE      = 100 * 1024 * 1024;
    static const size_t MAX_ZIP_ENTRIES    = 10000;

  private:
    unsigned int index = 0;

    size_t find_shared_string(const std::string &s) const;
    void process_styles(const std::string &filename, const std::string &file_content);
    bool str_casecmp_pre_safe(const std::string &content, size_t pos, std::string_view prefix) const;
    bool str_casecmp_pre(std::string_view str1, std::string_view str2, unsigned int length = 0);
    bool str_casecmp_last(std::string_view str1, std::string_view str2, unsigned int length = 0);
    void process_worksheets_sheet(const std::string &filename, const std::string &file_content);
    void process_workbook_rels(const std::string &filename, const std::string &file_content);
    void process_workbook(const std::string &filename, const std::string &file_content);
    void process_shared_strings(const std::string &filename, const std::string &file_content);
    bool is_safe_zip_path(const std::string &path) const;
};

}// namespace pz

#endif  // ENABLE_MINIZIP
#endif
