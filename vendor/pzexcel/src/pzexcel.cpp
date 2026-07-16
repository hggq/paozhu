#include <cstdlib>
#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <map>

#ifdef ENABLE_MINIZIP
#include "minizip/zip.h"
#include "minizip/unzip.h"
#include "pzexcel.h"

namespace pz
{
static std::string xml_escape(const std::string &s)
{
    std::string result;
    result.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
        case '&': result += "&amp;"; break;
        case '<': result += "&lt;"; break;
        case '>': result += "&gt;"; break;
        case '"': result += "&quot;"; break;
        case '\'': result += "&apos;"; break;
        default: result += c;
        }
    }
    return result;
}

excel::excel()
{
}

void excel::clear()
{
    shared_strings.clear();
    sheets.clear();
    workbook_mapfiles.clear();
    error_msg.clear();
    fonts.clear();
    bgcolors.clear();
    borders.clear();
    styles.clear();
    numfmts.clear();
    index = 0;
}

size_t excel::find_shared_string(const std::string &s) const
{
    for (size_t i = 0; i < shared_strings.size(); i++)
    {
        if (shared_strings[i] == s)
        {
            return i;
        }
    }
    return SIZE_MAX;
}

bool excel::is_safe_zip_path(const std::string &path) const
{
    if (path.empty())
        return false;
    if (path[0] == '/')
        return false;
    if (path.find("..") != std::string::npos)
        return false;
    if (path.size() >= 2 && path[1] == ':')
        return false;
    return true;
}

bool excel::str_casecmp_pre_safe(const std::string &content, size_t pos, std::string_view prefix) const
{
    if (pos + prefix.size() > content.size())
    {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); i++)
    {
        char c1 = content[pos + i];
        char c2 = prefix[i];
        if (c1 == c2)
        {
            continue;
        }
        if (c1 >= 'A' && c1 <= 'Z')
        {
            if ((c1 + 32) == c2)
            {
                continue;
            }
        }
        else if (c2 >= 'A' && c2 <= 'Z')
        {
            if (c1 == (c2 + 32))
            {
                continue;
            }
        }
        return false;
    }
    return true;
}

bool excel::str_casecmp_pre(std::string_view str1, std::string_view str2, unsigned int length)
{
    if (length == 0)
    {
        length = str2.size();
    }

    if (str1.size() < str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < length; i++)
    {
        if (i < str2.size())
        {
            if (str1[i] == str2[i])
            {
                continue;
            }
            else
            {
                if (str1[i] < 91 && str1[i] > 64)
                {
                    if ((str1[i] + 32) == str2[i])
                    {
                        continue;
                    }
                }
                else if (str2[i] < 91 && str2[i] > 64)
                {
                    if (str1[i] == (str2[i] + 32))
                    {
                        continue;
                    }
                }
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    return true;
}

bool excel::read(const std::string &zipfilename)
{
    clear();
    error_msg.clear();
    unzFile uf = unzOpen(zipfilename.c_str());
    if (uf == NULL)
    {
        error_msg = "Error: Unable to open zip file ";
        error_msg.append(zipfilename);
        return false;
    }

    unz_global_info globalInfo;
    if (unzGetGlobalInfo(uf, &globalInfo) != UNZ_OK)
    {
        error_msg = "Error: Unable to get global info for ";
        error_msg.append(zipfilename);
        unzClose(uf);
        return false;
    }

    if (unzGoToFirstFile(uf) != UNZ_OK)
    {
        error_msg = "Error: Unable to go to first file in ";
        error_msg.append(zipfilename);
        unzClose(uf);
        return false;
    }

    std::string tempname;
    std::string file_content;
    size_t zip_entry_count = 0;

    do
    {
        if (zip_entry_count >= MAX_ZIP_ENTRIES)
        {
            break;
        }
        zip_entry_count++;

        tempname.clear();
        char buffer[4096];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(uf, &fileInfo, buffer, sizeof(buffer), NULL, 0, NULL, 0) != UNZ_OK)
        {
            error_msg = "Error: Unable to get current file info in  ";
            error_msg.append(zipfilename);
            break;
        }
        tempname.append(buffer);

        if (!is_safe_zip_path(tempname))
        {
            unzCloseCurrentFile(uf);
            continue;
        }

        if (unzOpenCurrentFile(uf) != UNZ_OK)
        {
            error_msg = "Error: Unable to open current file in ";
            error_msg.append(zipfilename);
            break;
        }

        int bytesRead;
        file_content.clear();
        do
        {
            bytesRead = unzReadCurrentFile(uf, buffer, sizeof(buffer));
            if (bytesRead > 0)
            {
                if (file_content.size() + bytesRead > MAX_FILE_SIZE)
                {
                    error_msg = "Error: File size exceeds maximum limit";
                    unzCloseCurrentFile(uf);
                    unzClose(uf);
                    return false;
                }
                file_content.append(buffer, bytesRead);
            }
        } while (bytesRead > 0);
        unzCloseCurrentFile(uf);

        if (tempname == "xl/sharedStrings.xml")
        {
            process_shared_strings(tempname, file_content);
        }
        else if (tempname == "xl/workbook.xml")
        {
            process_workbook(tempname, file_content);
        }
        else if (tempname == "xl/_rels/workbook.xml.rels")
        {
            process_workbook_rels(tempname, file_content);
        }
        else if (str_casecmp_pre(tempname, "xl/worksheets/sheet"))
        {
            process_worksheets_sheet(tempname, file_content);
        }
        else if (str_casecmp_pre(tempname, "xl/styles.xml"))
        {
            process_styles(tempname, file_content);
        }

    } while (unzGoToNextFile(uf) == UNZ_OK);

    unzClose(uf);
    return true;
}
void excel::process_styles([[maybe_unused]] const std::string &filename, const std::string &file_content)
{
    size_t si = 0;
    std::string numfmt_id, numfmt_code;
    for (; si < file_content.size(); si++)
    {
        //<dimension
        if (str_casecmp_pre_safe(file_content, si, "<numFmts"))
        {
            si = si + sizeof("<numFmts") - 1;
            for (; si < file_content.size(); si++)
            {
                if (str_casecmp_pre_safe(file_content, si, "<numFmt "))
                {
                    si = si + sizeof("<numFmt ") - 1;
                    numfmt_id.clear();
                    numfmt_code.clear();
                    for (; si < file_content.size(); si++)
                    {
                        if (str_casecmp_pre_safe(file_content, si, "numFmtId=\""))
                        {
                            si = si + sizeof("numFmtId=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '\"' || file_content[si] == ' ')
                                {
                                    break;
                                }
                                numfmt_id.push_back(file_content[si]);
                            }

                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "formatCode=\""))
                        {
                            si = si + sizeof("formatCode=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '\"' || file_content[si] == ' ')
                                {
                                    break;
                                }
                                numfmt_code.push_back(file_content[si]);
                            }

                            continue;
                        }
                        else if (file_content[si] == 0x2F || file_content[si] == '>')
                        {
                            si = si - 1;
                            break;
                        }
                    }
                    numfmts.insert({numfmt_id, numfmt_code});
                }
                else if (str_casecmp_pre_safe(file_content, si, "</numFmts>"))
                {
                    si = si - 1;
                    break;
                }
            }
            break;
        }
    }
    //reset offset maybe has not numfmts tag
    si = 0;

    for (; si < file_content.size(); si++)
    {
        //<dimension
        if (str_casecmp_pre_safe(file_content, si, "<fonts "))
        {
            si = si + sizeof("<fonts ") - 1;
            for (; si < file_content.size(); si++)
            {
                if (str_casecmp_pre_safe(file_content, si, "<font>"))
                {
                    PZ_EXCEL_CELL_FONT temp_font;
                    si = si + sizeof("<font>") - 1;
                    for (; si < file_content.size(); si++)
                    {
                        if (str_casecmp_pre_safe(file_content, si, "<b/"))
                        {
                            si             = si + sizeof("<b/") - 1;
                            temp_font.bold = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<b "))
                        {
                            si             = si + sizeof("<b ") - 1;
                            temp_font.bold = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<u/"))
                        {
                            si               = si + sizeof("<u/") - 1;
                            temp_font.strike = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<u "))
                        {
                            si               = si + sizeof("<u ") - 1;
                            temp_font.strike = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<i/"))
                        {
                            si               = si + sizeof("<i/") - 1;
                            temp_font.italic = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<i "))
                        {
                            si               = si + sizeof("<i ") - 1;
                            temp_font.italic = true;
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<sz "))
                        {
                            si = si + sizeof("<sz ") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                unsigned int font_size_temp = 0;
                                if (str_casecmp_pre_safe(file_content, si, "val=\""))
                                {
                                    si = si + sizeof("val=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        font_size_temp = font_size_temp * 10 + (unsigned char)(file_content[si] - '0');
                                    }
                                    temp_font.size = (font_size_temp > 255) ? 255 : (unsigned char)font_size_temp;
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<color"))
                        {
                            si = si + sizeof("<color") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                numfmt_id.clear();
                                if (str_casecmp_pre_safe(file_content, si, "rgb=\""))
                                {
                                    si = si + sizeof("rgb=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                        if (numfmt_id.size() > 10)
                                        {
                                            break;
                                        }
                                    }
                                    temp_font.color_type = 10;
                                    temp_font.color      = numfmt_id;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "theme=\""))
                                {
                                    si = si + sizeof("theme=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                        if (numfmt_id.size() > 10)
                                        {
                                            break;
                                        }
                                    }
                                    temp_font.color_type = 1;
                                    temp_font.color      = numfmt_id;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "indexed=\""))
                                {
                                    si = si + sizeof("indexed=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                        if (numfmt_id.size() > 10)
                                        {
                                            break;
                                        }
                                    }
                                    temp_font.color_type = 5;
                                    temp_font.color      = numfmt_id;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "tint=\""))
                                {
                                    si = si + sizeof("tint=\"") - 1;
                                    numfmt_id.push_back(' ');
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                        if (numfmt_id.size() > 40)
                                        {
                                            break;
                                        }
                                    }
                                    temp_font.color_type = 15;
                                    temp_font.color      = numfmt_id;
                                }
                                else if (file_content[si] == '>')
                                {
                                    si = si - 1;
                                    break;
                                }
                            }
                            continue;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<name"))
                        {
                            si = si + sizeof("<name") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                numfmt_id.clear();
                                if (str_casecmp_pre_safe(file_content, si, "val=\""))
                                {
                                    si = si + sizeof("val=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                    }
                                    temp_font.name = numfmt_id;
                                    break;
                                }
                            }
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<family"))
                        {
                            si = si + sizeof("<family") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                numfmt_id.clear();
                                if (str_casecmp_pre_safe(file_content, si, "val=\""))
                                {
                                    si = si + sizeof("val=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                    }
                                    temp_font.family = numfmt_id;
                                    break;
                                }
                            }
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<charset"))
                        {
                            si = si + sizeof("<charset") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                numfmt_id.clear();
                                if (str_casecmp_pre_safe(file_content, si, "val=\""))
                                {
                                    si = si + sizeof("val=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                    }
                                    temp_font.charset = numfmt_id;
                                    break;
                                }
                            }
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "<scheme"))
                        {
                            si = si + sizeof("<scheme") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                numfmt_id.clear();
                                if (str_casecmp_pre_safe(file_content, si, "val=\""))
                                {
                                    si = si + sizeof("val=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (file_content[si] == '"')
                                        {
                                            break;
                                        }
                                        numfmt_id.push_back(file_content[si]);
                                    }
                                    temp_font.scheme = numfmt_id;
                                    break;
                                }
                            }
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "</font>"))
                        {
                            si = si + sizeof("</font>") - 1;
                            si = si - 1;
                            break;
                        }
                    }
                    fonts.push_back(temp_font);
                }
                else if (str_casecmp_pre_safe(file_content, si, "</fonts>"))
                {
                    si = si + sizeof("</fonts>") - 1;
                    si = si - 1;
                    break;
                }
            }

            continue;
        }
        else if (str_casecmp_pre_safe(file_content, si, "<fills"))
        {
            si = si + sizeof("<fills") - 1;
            for (; si < file_content.size(); si++)
            {
                if (str_casecmp_pre_safe(file_content, si, "<fill>"))
                {
                    PZ_EXCEL_CELL_FILL temp_fill;
                    si = si + sizeof("<fill>") - 1;
                    for (; si < file_content.size(); si++)
                    {
                        if (str_casecmp_pre_safe(file_content, si, "patternType=\""))
                        {
                            si = si + sizeof("patternType=\"") - 1;
                            std::string temp_ptype;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"' || file_content[si] == 0x2F || file_content[si] == '>')
                                {
                                    si = si - 1;
                                    break;
                                }
                                temp_ptype.push_back(file_content[si]);
                            }
                            temp_fill.pattern_type = std::move(temp_ptype);
                        }
                        else if (file_content[si] == 0x2F)
                        {
                            si = si - 1;
                            break;
                        }
                        else if (file_content[si] == '>')
                        {
                            for (; si < file_content.size(); si++)
                            {
                                if (str_casecmp_pre_safe(file_content, si, "<fgColor "))
                                {
                                    si = si + sizeof("<fgColor ") - 1;
                                    std::string bg_color;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (str_casecmp_pre_safe(file_content, si, "theme=\""))
                                        {
                                            temp_fill.bg_type = 1;
                                            si                = si + sizeof("theme=\"") - 1;
                                            for (; si < file_content.size(); si++)
                                            {
                                                if (file_content[si] == '"')
                                                {
                                                    si = si - 1;
                                                    break;
                                                }
                                                bg_color.push_back(file_content[si]);
                                            }
                                        }
                                        else if (str_casecmp_pre_safe(file_content, si, "tint=\""))
                                        {
                                            si = si + sizeof("tint=\"") - 1;
                                            for (; si < file_content.size(); si++)
                                            {
                                                if (file_content[si] == '"')
                                                {
                                                    si = si - 1;
                                                    break;
                                                }
                                                bg_color.push_back(file_content[si]);
                                            }
                                        }
                                        else if (file_content[si] == '"' || file_content[si] == 0x2F || file_content[si] == '>')
                                        {
                                            si = si - 1;
                                            break;
                                        }
                                    }
                                    temp_fill.fgColor = bg_color;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "<bgColor "))
                                {
                                    si = si + sizeof("<bgColor ") - 1;
                                    std::string bg_color;
                                    for (; si < file_content.size(); si++)
                                    {
                                        if (str_casecmp_pre_safe(file_content, si, "indexed=\""))
                                        {
                                            si = si + sizeof("indexed=\"") - 1;
                                            for (; si < file_content.size(); si++)
                                            {
                                                if (file_content[si] == '"')
                                                {
                                                    si = si - 1;
                                                    break;
                                                }
                                                bg_color.push_back(file_content[si]);
                                            }
                                        }
                                    }
                                    temp_fill.bgcolor = bg_color;
                                }
                            }
                            break;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "</fill>"))
                        {
                            si = si + sizeof("</fill>") - 1;
                            break;
                        }
                    }
                    bgcolors.push_back(temp_fill);
                }
                else if (str_casecmp_pre_safe(file_content, si, "</fills>"))
                {
                    si = si + sizeof("</fills>") - 1;
                    break;
                }
            }
        }
        else if (str_casecmp_pre_safe(file_content, si, "<borders"))
        {
            si = si + sizeof("<borders") - 1;
            for (; si < file_content.size(); si++)
            {
                if (str_casecmp_pre_safe(file_content, si, "<border>") || str_casecmp_pre_safe(file_content, si, "<border "))
                {
                    PZ_EXCEL_CELL_BORDER temp_border;
                    bool has_attr = (si + 7 < file_content.size() && file_content[si + 7] != '>');
                    if (!has_attr)
                        si = si + sizeof("<border>") - 1;
                    else
                    {
                        si = si + sizeof("<border ") - 1;
                        for (; si < file_content.size(); si++)
                        {
                            if (file_content[si] == '>')
                            {
                                si++;
                                break;
                            }
                        }
                    }
                    for (; si < file_content.size(); si++)
                    {
                        if (str_casecmp_pre_safe(file_content, si, "<left"))
                        {
                            si = si + sizeof("<left") - 1;
                            if (file_content[si] == '/')
                            {
                                si = si - 1;
                                break;
                            }
                            else if (str_casecmp_pre_safe(file_content, si, "style=\""))
                            {
                                si = si + sizeof("style=\"") - 1;
                                for (; si < file_content.size(); si++)
                                {
                                    if (file_content[si] == '"')
                                    {
                                        si = si - 1;
                                        break;
                                    }
                                    temp_border.left.style.push_back(file_content[si]);
                                }
                            }
                            else if (file_content[si] == '>')
                            {
                                for (; si < file_content.size(); si++)
                                {
                                    if (str_casecmp_pre_safe(file_content, si, "<color"))
                                    {
                                        si = si + sizeof("<color") - 1;
                                        for (; si < file_content.size(); si++)
                                        {
                                            if (str_casecmp_pre_safe(file_content, si, "indexed=\""))
                                            {
                                                si = si + sizeof("indexed=\"") - 1;
                                                for (; si < file_content.size(); si++)
                                                {
                                                    if (file_content[si] == '>' || file_content[si] == '/')
                                                    {
                                                        break;
                                                    }
                                                    temp_border.left.color.push_back(file_content[si]);
                                                }
                                            }
                                            else if (str_casecmp_pre_safe(file_content, si, "rgb=\""))
                                            {
                                                si = si + sizeof("rgb=\"") - 1;

                                                for (; si < file_content.size(); si++)
                                                {
                                                    if (file_content[si] == '>' || file_content[si] == '/')
                                                    {
                                                        break;
                                                    }
                                                    temp_border.left.color.push_back(file_content[si]);
                                                }
                                            }
                                            else if (file_content[si] == '>' || file_content[si] == '/')
                                            {
                                                si = si - 1;
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "</border>"))
                        {
                            si = si + sizeof("</border>") - 1;
                            borders.push_back(temp_border);
                            break;
                        }
                    }
                }
                else if (str_casecmp_pre_safe(file_content, si, "</borders>"))
                {
                    si = si + sizeof("</borders>") - 1;
                    break;
                }
            }
        }
    }
}
void excel::process_worksheets_sheet(const std::string &filename, const std::string &file_content)
{
    unsigned int offset   = sizeof("xl/worksheets/sheet") - 1;
    unsigned int sheet_id = 0, active_sheet_index = 0xFFFF;
    unsigned int max_row = 0;
    unsigned int max_col = 0;
    unsigned int row_num = 0;
    unsigned int col_num = 0;
    for (unsigned int i = offset; i < filename.size(); i++)
    {
        if (filename[i] == '.')
        {
            break;
        }
        else if (filename[i] > 0x2F && filename[i] < 0x3A)
        {
            sheet_id = sheet_id * 10 + (filename[i] - '0');
        }
    }
    //check sheet at
    for (row_num = 0; row_num < sheets.size(); row_num++)
    {
        if (sheets[row_num].sheet_index == sheet_id)
        {
            active_sheet_index = row_num;
            break;
        }
    }
    if (active_sheet_index == 0xFFFF)
    {
        PZ_EXCEL_SHEET temp_sheet;
        temp_sheet.sheet_index = sheet_id;
        sheets.push_back(std::move(temp_sheet));

        active_sheet_index = sheets.size() - 1;
    }
    size_t si = 0;

    for (; si < file_content.size(); si++)
    {
        //<dimension
        if (str_casecmp_pre_safe(file_content, si, "<dimension"))
        {
            si = si + sizeof("<dimension") - 1;
            for (; si < file_content.size(); si++)
            {
                //ref="
                if (str_casecmp_pre_safe(file_content, si, "ref=\""))
                {
                    si = si + sizeof("ref=\"") - 1;
                    std::string begin_str;

                    for (; si < file_content.size(); si++)
                    {
                        if (file_content[si] == ':')
                        {
                            si++;
                            break;
                        }
                    }
                    for (; si < file_content.size(); si++)
                    {
                        if (file_content[si] == '"')
                        {
                            break;
                        }
                        else if (file_content[si] > 0x2F && file_content[si] < 0x3A)
                        {
                            max_row = max_row * 10 + (file_content[si] - '0');
                        }
                        else if (file_content[si] > 0x40 && file_content[si] < 0x5B)
                        {
                            begin_str.push_back(file_content[si]);
                        }
                        else if (file_content[si] > 0x60 && file_content[si] < 0x7B)
                        {
                            begin_str.push_back((file_content[si] - 32));
                        }
                        else
                        {
                            break;
                        }
                    }

                    for (unsigned int i = 0; i < begin_str.size(); i++)
                    {
                        max_col = max_col * 26 + (begin_str[i] - 0x40);
                    }

                    break;
                }
            }

            break;
        }
    }
    sheets[active_sheet_index].cols = max_col;
    sheets[active_sheet_index].rows = max_row;

    for (; si < file_content.size(); si++)
    {
        //<sheetData>
        if (str_casecmp_pre_safe(file_content, si, "<sheetData>"))
        {
            si = si + sizeof("<sheetData>") - 1;
            for (; si < file_content.size(); si++)
            {
                //<row
                if (str_casecmp_pre_safe(file_content, si, "<row"))
                {
                    si = si + sizeof("<row") - 1;
                    for (; si < file_content.size(); si++)
                    {
                        //r="
                        if (str_casecmp_pre_safe(file_content, si, "r=\""))
                        {
                            row_num = 0;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"' || file_content[si] == '>')
                                {
                                    break;
                                }
                                else if (file_content[si] > 0x2F && file_content[si] < 0x3A)
                                {
                                    row_num = row_num * 10 + (file_content[si] - '0');
                                }
                            }
                            break;
                        }
                        else if (file_content[si] == '>')
                        {
                            break;
                        }
                    }
                    //end row head
                    //c tag

                    for (; si < file_content.size(); si++)
                    {
                        //<c
                        if (str_casecmp_pre_safe(file_content, si, "<c"))
                        {
                            row_num = 0;
                            col_num = 0;
                            PZ_EXCEL_CELL temp_cell;
                            std::string temp_v_str;
                            temp_cell.v_type = 0;
                            bool is_has_v    = true;
                            bool has_value   = false;

                            si = si + sizeof("<c") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                //r="
                                if (str_casecmp_pre_safe(file_content, si, "r=\""))
                                {
                                    si = si + sizeof("r=\"") - 1;
                                    for (; si < file_content.size(); si++)
                                    {
                                        // " or / space
                                        if (file_content[si] == '"' || file_content[si] == 0x2F || file_content[si] == 0x20)
                                        {
                                            break;
                                        }
                                        else if (file_content[si] > 0x2F && file_content[si] < 0x3A)
                                        {
                                            row_num = row_num * 10 + (file_content[si] - '0');
                                        }
                                        else if (file_content[si] > 0x40 && file_content[si] < 0x5B)
                                        {
                                            if (col_num < 16384)
                                            {
                                                col_num = col_num * 26 + (file_content[si] - 0x40);
                                            }
                                        }
                                        else if (file_content[si] > 0x60 && file_content[si] < 0x7B)
                                        {
                                            if (col_num < 16384)
                                            {
                                                col_num = col_num * 26 + (file_content[si] - 0x60);
                                            }
                                        }
                                    }
                                    //now cell
                                    //PZ_EXCEL_CELL
                                    continue;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "s=\""))
                                {
                                    //s="
                                    si                     = si + sizeof("s=\"") - 1;
                                    unsigned int style_num = 0;
                                    for (; si < file_content.size(); si++)
                                    {
                                        // " or / space
                                        if (file_content[si] == '"' || file_content[si] == 0x2F || file_content[si] == 0x20)
                                        {
                                            break;
                                        }
                                        else if (file_content[si] > 0x2F && file_content[si] < 0x3A)
                                        {
                                            style_num = style_num * 10 + (file_content[si] - '0');
                                        }
                                    }
                                    temp_cell.s_type = (style_num > 255) ? 255 : (unsigned char)style_num;
                                    continue;
                                }
                                else if (str_casecmp_pre_safe(file_content, si, "t=\""))
                                {
                                    //t="
                                    si = si + sizeof("t=\"") - 1;
                                    temp_v_str.clear();
                                    for (; si < file_content.size(); si++)
                                    {
                                        // " or / space
                                        if (file_content[si] == '"' || file_content[si] == 0x2F || file_content[si] == 0x20)
                                        {
                                            break;
                                        }
                                        temp_v_str.push_back(file_content[si]);
                                    }
                                    if (temp_v_str.size() > 0)
                                    {
                                        temp_cell.t_type = temp_v_str[0];
                                        if (temp_v_str[0] == 's' || temp_v_str[0] == 'S')
                                        {
                                            temp_cell.v_type = 5;
                                        }
                                    }
                                    temp_v_str.clear();
                                    continue;
                                }
                                else if (file_content[si] == '>')
                                {
                                    break;
                                }
                                else if (file_content[si] == 0x2F)
                                {
                                    // slash
                                    is_has_v = false;
                                    break;
                                }
                            }
                            si -= 1;
                            //v tag
                            if (is_has_v)
                            {
                                for (; si < file_content.size(); si++)
                                {
                                    //<v
                                    if (str_casecmp_pre_safe(file_content, si, "<v"))
                                    {
                                        si = si + sizeof("<v") - 1;

                                        for (; si < file_content.size(); si++)
                                        {
                                            if (file_content[si] == '>')
                                            {
                                                si++;
                                                break;
                                            }
                                        }
                                        //skip v tag head

                                        for (; si < file_content.size(); si++)
                                        {
                                            if (str_casecmp_pre_safe(file_content, si, "</v"))
                                            {
                                                si = si + sizeof("<v") - 1;
                                                break;
                                            }
                                            else if (file_content[si] == '.')
                                            {
                                                temp_cell.v_type = 3;
                                            }
                                            temp_v_str.push_back(file_content[si]);
                                        }

                                        break;
                                    }
                                }
                            }

                            //end v tag;
                            if (is_has_v && temp_v_str.size() > 0)
                            {
                                has_value = true;
                                if (temp_cell.v_type == 3)
                                {
                                    try
                                    {
                                        temp_cell.value.d = std::stold(temp_v_str.c_str());
                                    }
                                    catch (const std::exception &)
                                    {
                                        temp_cell.value.d = 0.0;
                                    }
                                }
                                else
                                {
                                    try
                                    {
                                        temp_cell.value.i = std::stoll(temp_v_str.c_str());
                                    }
                                    catch (const std::exception &)
                                    {
                                        temp_cell.value.i = 0;
                                    }
                                }
                            }

                            if (has_value && row_num > 0 && col_num > 0)
                            {
                                auto it = sheets[active_sheet_index].values.find(row_num);
                                if (it != sheets[active_sheet_index].values.end())
                                {
                                    it->second.insert({col_num, std::move(temp_cell)});
                                }
                                else
                                {
                                    std::map<unsigned int, PZ_EXCEL_CELL> tempb;
                                    tempb.insert({col_num, std::move(temp_cell)});
                                    sheets[active_sheet_index].values.insert({row_num, std::move(tempb)});
                                }
                                if (max_col < col_num)
                                {
                                    max_col = col_num;
                                }
                                if (max_row < row_num)
                                {
                                    max_row = row_num;
                                }
                            }
                            si -= 1;
                        }
                        else if (str_casecmp_pre_safe(file_content, si, "</row>"))
                        {
                            si = si + sizeof("</row>") - 2;
                            break;
                        }
                        //end c
                    }
                    //row
                }
                else if (str_casecmp_pre_safe(file_content, si, "</sheetData>"))
                {
                    si = si + sizeof("</sheetData>") - 2;
                    break;
                }
            }

            break;
        }
    }
    sheets[active_sheet_index].cols = max_col;
    sheets[active_sheet_index].rows = max_row;
}
void excel::process_workbook_rels([[maybe_unused]] const std::string &filename, const std::string &file_content)
{
    //workbook_mapfiles
    for (size_t si = 0; si < file_content.size(); si++)
    {
        //<Relationships
        if (str_casecmp_pre_safe(file_content, si, "<Relationships"))
        {
            si = si + sizeof("<Relationships") - 1;
            for (; si < file_content.size(); si++)
            {
                //<Relationship
                if (str_casecmp_pre_safe(file_content, si, "<Relationship "))
                {
                    si = si + sizeof("<Relationship ") - 1;
                    std::string temp_id_str;
                    std::string temp_target_str;
                    for (; si < file_content.size(); si++)
                    {
                        //Id="
                        if (str_casecmp_pre_safe(file_content, si, "Id=\""))
                        {
                            si = si + sizeof("Id=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"')
                                {
                                    break;
                                }
                                temp_id_str.push_back(file_content[si]);
                            }
                            if (temp_target_str.size() > 0)
                            {
                                break;
                            }
                            continue;
                        }
                        //Target="
                        if (str_casecmp_pre_safe(file_content, si, "Target=\""))
                        {
                            si = si + sizeof("Target=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"')
                                {
                                    break;
                                }
                                temp_target_str.push_back(file_content[si]);
                            }
                            if (temp_id_str.size() > 0)
                            {
                                break;
                            }
                            continue;
                        }
                        else if (file_content[si] == '>')
                        {
                            si = si - 1;
                            break;
                        }
                    }

                    if (temp_id_str.size() > 0 && temp_target_str.size() > 0)
                    {
                        workbook_mapfiles.insert({std::move(temp_id_str), std::move(temp_target_str)});
                    }
                    //
                }
                else if (str_casecmp_pre_safe(file_content, si, "</Relationships"))
                {

                    break;
                }
            }
            break;
        }
    }
}
void excel::process_workbook([[maybe_unused]] const std::string &filename, const std::string &file_content)
{
    for (size_t si = 0; si < file_content.size(); si++)
    {
        //<sheets>
        if (str_casecmp_pre_safe(file_content, si, "<sheets>"))
        {
            si = si + sizeof("<sheets>") - 1;
            for (; si < file_content.size(); si++)
            {
                //<sheet
                if (str_casecmp_pre_safe(file_content, si, "<sheet"))
                {
                    si = si + sizeof("<sheet") - 1;
                    std::string temp_s_str;
                    std::string temp_sheet_id;
                    std::string temp_sheet_rid;
                    for (; si < file_content.size(); si++)
                    {
                        //name="
                        if (str_casecmp_pre_safe(file_content, si, "name=\""))
                        {
                            si = si + sizeof("name=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"')
                                {
                                    break;
                                }
                                temp_s_str.push_back(file_content[si]);
                            }
                            if (temp_sheet_id.size() > 0 && temp_sheet_rid.size() > 0)
                            {
                                break;
                            }
                            continue;
                        }
                        //sheetId="
                        if (str_casecmp_pre_safe(file_content, si, "sheetId=\""))
                        {
                            si = si + sizeof("sheetId=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"')
                                {
                                    break;
                                }
                                temp_sheet_id.push_back(file_content[si]);
                            }
                            if (temp_s_str.size() > 0 && temp_sheet_rid.size() > 0)
                            {
                                break;
                            }
                            continue;
                        }
                        //r:id="
                        if (str_casecmp_pre_safe(file_content, si, "r:id=\""))
                        {
                            si = si + sizeof("r:id=\"") - 1;
                            for (; si < file_content.size(); si++)
                            {
                                if (file_content[si] == '"')
                                {
                                    break;
                                }
                                temp_sheet_rid.push_back(file_content[si]);
                            }
                            if (temp_s_str.size() > 0 && temp_sheet_id.size() > 0)
                            {
                                break;
                            }
                            continue;
                        }
                        if (file_content[si] == 0x2F || file_content[si] == '>')
                        {
                            si = si - 1;
                            break;
                        }
                    }
                    //prevent first process sheet data
                    if (sheets.size() == 1 && sheets[0].sheet_index == 0)
                    {
                        sheets[0].name       = std::move(temp_s_str);
                        sheets[0].sheet_file = std::move(temp_sheet_rid);
                        ;
                        char *end_ptr           = nullptr;
                        unsigned long parsed_id = std::strtoul(temp_sheet_id.c_str(), &end_ptr, 10);
                        sheets[0].sheet_index   = (end_ptr != temp_sheet_id.c_str()) ? (unsigned int)parsed_id : 0;
                    }
                    else
                    {
                        unsigned int sheet_index = 0;
                        char *end_ptr2           = nullptr;
                        unsigned long parsed_id2 = std::strtoul(temp_sheet_id.c_str(), &end_ptr2, 10);
                        sheet_index              = (end_ptr2 != temp_sheet_id.c_str()) ? (unsigned int)parsed_id2 : 0;

                        unsigned int offset = 0;
                        for (; offset < sheets.size(); offset++)
                        {
                            if (sheets[offset].sheet_index == sheet_index)
                            {
                                sheets[offset].name       = std::move(temp_s_str);
                                sheets[offset].sheet_file = std::move(temp_sheet_rid);
                                ;
                                break;
                            }
                        }
                        if (offset == sheets.size())
                        {
                            PZ_EXCEL_SHEET temp_sheet;
                            temp_sheet.name       = std::move(temp_s_str);
                            temp_sheet.sheet_file = std::move(temp_sheet_rid);
                            ;
                            temp_sheet.sheet_index = sheet_index;
                            sheets.push_back(std::move(temp_sheet));
                        }
                    }
                    continue;
                }
                //</sheets>
                if (str_casecmp_pre_safe(file_content, si, "</sheets>"))
                {
                    si = si + sizeof("</sheets>") - 2;
                    break;
                }
            }
            break;
        }
    }
}
void excel::process_shared_strings([[maybe_unused]] const std::string &filename, const std::string &file_content)
{
    std::string temp_s_str;
    for (size_t si = 0; si < file_content.size(); si++)
    {
        if (str_casecmp_pre_safe(file_content, si, "<si>") || str_casecmp_pre_safe(file_content, si, "<si "))
        {
            if (file_content[si + 2] == '>')
                si = si + sizeof("<si>") - 1;
            else
                si = si + sizeof("<si ") - 1;
            for (; si < file_content.size(); si++)
            {
                if (str_casecmp_pre_safe(file_content, si, "<t>") || str_casecmp_pre_safe(file_content, si, "<t "))
                {
                    bool has_attr = (file_content[si + 2] != '>');
                    if (!has_attr)
                        si = si + sizeof("<t>") - 1;
                    else
                    {
                        si = si + sizeof("<t ") - 1;
                        for (; si < file_content.size(); si++)
                        {
                            if (file_content[si] == '>')
                            {
                                si++;
                                break;
                            }
                        }
                    }
                    temp_s_str.clear();
                    for (; si < file_content.size(); si++)
                    {
                        if (str_casecmp_pre_safe(file_content, si, "</t>"))
                        {
                            si = si + sizeof("</t>") - 2;
                            break;
                        }
                        temp_s_str.push_back(file_content[si]);
                    }
                    if (shared_strings.size() < MAX_SHARED_STRINGS)
                        shared_strings.push_back(temp_s_str);
                    continue;
                }
                if (str_casecmp_pre_safe(file_content, si, "</si>"))
                {
                    si = si + sizeof("</si>") - 2;
                    break;
                }
            }
        }
    }
}
std::string excel::getCell(unsigned int row_num, unsigned int col_num)
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
                if (idx < shared_strings.size())
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
std::string excel::getCell(const std::string &key)
{
    auto [row_num, col_num] = cellstr_to_num(key);
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
                if (idx < shared_strings.size())
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
unsigned int excel::getUnixTime(const std::string &key)
{
    auto [row_num, col_num] = cellstr_to_num(key);
    double cell_vall        = 0.0;
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
                cell_vall = cell_it->second.value.i;
            }
            else if (cell_it->second.v_type < 5)
            {
                cell_vall = cell_it->second.value.d;
            }
        }
    }
    //2209161600  1900 ~ 1970
    unsigned long long t_v = std::ceil(cell_vall * 86400);
    if (t_v < 2209161600)
    {
        return t_v;
    }
    return (t_v - 2209161600);
}

std::string excel::getDate(const std::string &key, const std::string &date_format)
{
    auto [row_num, col_num] = cellstr_to_num(key);
    double cell_vall        = 0.0;
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
                cell_vall = cell_it->second.value.i;
            }
            else if (cell_it->second.v_type < 5)
            {
                cell_vall = cell_it->second.value.d;
            }
        }
    }
    unsigned int t_v      = std::ceil(cell_vall * 86400);
    unsigned int temp_val = 1900;
    std::tm output_tm{};
    output_tm.tm_year  = 0;
    output_tm.tm_mon   = 0;
    output_tm.tm_mday  = 0;
    output_tm.tm_wday  = 0;
    output_tm.tm_yday  = 0;
    output_tm.tm_hour  = 0;
    output_tm.tm_min   = 0;
    output_tm.tm_sec   = 0;
    output_tm.tm_isdst = -1;

    for (; temp_val < 2238; temp_val++)
    {
        if (temp_val == 1900)
        {
            if (t_v > 31622400)
            {
                t_v -= 31622400;
                continue;
            }
            break;
        }
        else if (temp_val % 400 == 0 || (temp_val % 4 == 0 && temp_val % 100 != 0))
        {
            if (t_v > 31622400)
            {
                t_v -= 31622400;
                continue;
            }
            break;
        }
        else
        {
            if (t_v > 31536000)
            {
                t_v -= 31536000;
                continue;
            }
            break;
        }
    }

    output_tm.tm_year            = temp_val;
    const static int monthdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    temp_val                     = 0;
    for (; temp_val < 12; temp_val++)
    {
        unsigned int t_month = monthdays[temp_val] * 86400;
        if (temp_val == 1)
        {
            if (output_tm.tm_year % 400 == 0 || (output_tm.tm_year % 4 == 0 && output_tm.tm_year % 100 != 0))
            {
                t_month += 86400;
            }
        }
        if (t_v >= t_month)
        {
            t_v -= t_month;
        }
        else
        {
            break;
        }
    }
    output_tm.tm_year = output_tm.tm_year - 1900;
    output_tm.tm_mon  = temp_val;
    output_tm.tm_mday = t_v / 86400;
    t_v               = t_v % 86400;
    output_tm.tm_hour = t_v / 3600;
    t_v               = t_v % 3600;
    output_tm.tm_min  = t_v / 60;
    t_v               = t_v % 60;
    output_tm.tm_sec  = t_v;
    std::string temp(36, 0x00);
    strftime(temp.data(), temp.length(), date_format.c_str(), &output_tm);
    for (unsigned int i = 0; i < temp.size(); i++)
    {
        if (temp[i] == 0x00)
        {
            temp.resize(i);
            break;
        }
    }
    return temp;
}
std::tuple<unsigned int, unsigned int> excel::cellstr_to_num(const std::string &key)
{
    if (key.empty())
    {
        return std::make_tuple(0, 0);
    }
    unsigned int row_num = 0;
    unsigned int col_num = 0;
    for (unsigned int i = 0; i < key.size(); i++)
    {
        if (key[i] > 0x2F && key[i] < 0x3A)
        {
            if (row_num < 1048576)
            {
                row_num = row_num * 10 + (key[i] - '0');
            }
        }
        else if (key[i] > 0x40 && key[i] < 0x5B)
        {
            if (col_num < 16384)
            {
                col_num = col_num * 26 + (key[i] - 0x40);
            }
        }
        else if (key[i] > 0x60 && key[i] < 0x7B)
        {
            if (col_num < 16384)
            {
                col_num = col_num * 26 + (key[i] - 0x60);
            }
        }
    }
    return std::make_tuple(row_num, col_num);
}
excel &excel::setIndex(unsigned int num_index)
{
    if (num_index < sheets.size())
    {
        index = num_index;
    }
    return *this;
}
excel &excel::setActiveSheet(unsigned int num_index)
{
    if (num_index < sheets.size())
    {
        index = num_index;
    }
    return *this;
}
excel &excel::setActiveName(const std::string &key)
{
    for (unsigned int i = 0; i < sheets.size(); i++)
    {
        if (sheets[i].name == key)
        {
            index = i;
            break;
        }
    }
    return *this;
}

excel &excel::addSheet(const std::string &name)
{
    PZ_EXCEL_SHEET temp_sheet;
    temp_sheet.name        = name;
    temp_sheet.sheet_index = sheets.size() + 1;
    sheets.push_back(std::move(temp_sheet));
    index = sheets.size() - 1;
    return *this;
}

static void ensure_default_style(std::vector<PZ_EXCEL_CELL_FONT> &fonts,
                                 std::vector<PZ_EXCEL_CELL_FILL> &bgcolors,
                                 std::vector<PZ_EXCEL_CELL_BORDER> &borders,
                                 std::vector<PZ_EXCEL_CELL_STYLE> &styles)
{
    if (fonts.empty())
    {
        PZ_EXCEL_CELL_FONT f;
        f.name = "Calibri";
        f.size = 11;
        fonts.push_back(f);
    }
    if (bgcolors.empty())
    {
        PZ_EXCEL_CELL_FILL f1, f2;
        f1.pattern_type = "none";
        f2.pattern_type = "gray125";
        bgcolors.push_back(f1);
        bgcolors.push_back(f2);
    }
    if (borders.empty())
    {
        PZ_EXCEL_CELL_BORDER b;
        borders.push_back(b);
    }
    if (styles.empty())
    {
        PZ_EXCEL_CELL_STYLE s;
        s.numfmtid = 0;
        s.fontid   = 0;
        s.fillid   = 0;
        s.borderid = 0;
        s.xfid     = 0;
        styles.push_back(s);
    }
}

static std::string normalize_color(std::string_view value)
{
    std::string s(value);
    if (s.size() == 6)
    {
        return "FF" + s;
    }
    return s;
}

void excel::setCellColor(const std::string &key, std::string_view value)
{
    if (index >= sheets.size())
        return;

    auto [row_num, col_num] = cellstr_to_num(key);
    if (row_num == 0 || col_num == 0)
        return;
    if (row_num > MAX_ROWS || col_num > MAX_COLS)
        return;

    ensure_default_style(fonts, bgcolors, borders, styles);

    std::string color_str = normalize_color(value);

    size_t font_idx = SIZE_MAX;
    for (size_t i = 0; i < fonts.size(); i++)
    {
        if (fonts[i].name == "Calibri" && fonts[i].size == 11 && fonts[i].color == color_str && !fonts[i].bold && !fonts[i].italic && !fonts[i].strike && fonts[i].underline.empty())
        {
            font_idx = i;
            break;
        }
    }

    if (font_idx == SIZE_MAX)
    {
        PZ_EXCEL_CELL_FONT f;
        f.name  = "Calibri";
        f.size  = 11;
        f.color = color_str;
        fonts.push_back(f);
        font_idx = fonts.size() - 1;
    }

    size_t style_idx = SIZE_MAX;
    for (size_t i = 0; i < styles.size(); i++)
    {
        if (styles[i].fontid == font_idx && styles[i].apply_font &&
            styles[i].fillid == 0 && styles[i].borderid == 0 &&
            styles[i].numfmtid == 0 && styles[i].xfid == 0)
        {
            style_idx = i;
            break;
        }
    }

    if (style_idx == SIZE_MAX)
    {
        PZ_EXCEL_CELL_STYLE s;
        s.numfmtid   = 0;
        s.fontid     = (font_idx > 255) ? 255 : static_cast<unsigned char>(font_idx);
        s.fillid     = 0;
        s.borderid   = 0;
        s.xfid       = 0;
        s.apply_font = true;
        styles.push_back(s);
        style_idx = styles.size() - 1;
    }

    auto it = sheets[index].values.find(row_num);
    if (it != sheets[index].values.end())
    {
        auto cell_it = it->second.find(col_num);
        if (cell_it != it->second.end())
        {
            cell_it->second.s_type = (style_idx > 255) ? 255 : static_cast<unsigned char>(style_idx);
            return;
        }
    }

    PZ_EXCEL_CELL temp_cell;
    temp_cell.s_type                       = (style_idx > 255) ? 255 : static_cast<unsigned char>(style_idx);
    sheets[index].values[row_num][col_num] = std::move(temp_cell);
    if (row_num > sheets[index].rows)
        sheets[index].rows = row_num;
    if (col_num > sheets[index].cols)
        sheets[index].cols = col_num;
}

static std::string col_num_to_str(unsigned int col_num)
{
    std::string result;
    while (col_num > 0)
    {
        col_num--;
        result.insert(result.begin(), 'A' + (col_num % 26));
        col_num /= 26;
    }
    return result;
}

static std::string gen_content_types_xml(unsigned int sheet_count)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">";
    xml += "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>";
    xml += "<Default Extension=\"xml\" ContentType=\"application/xml\"/>";
    xml += "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>";
    for (unsigned int i = 1; i <= sheet_count; i++)
    {
        xml += "<Override PartName=\"/xl/worksheets/sheet" + std::to_string(i) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
    }
    xml += "<Override PartName=\"/xl/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>";
    xml += "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>";
    xml += "<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>";
    xml += "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>";
    xml += "<Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>";
    xml += "</Types>";
    return xml;
}

static std::string gen_rels_xml()
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    xml += "<Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties\" Target=\"docProps/app.xml\"/>";
    xml += "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" Target=\"docProps/core.xml\"/>";
    xml += "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>";
    xml += "</Relationships>";
    return xml;
}

static std::string gen_workbook_xml(const std::vector<PZ_EXCEL_SHEET> &sheets)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    xml += "<sheets>";
    for (unsigned int i = 0; i < sheets.size(); i++)
    {
        std::string name = sheets[i].name.empty() ? "Sheet" + std::to_string(i + 1) : sheets[i].name;
        xml += "<sheet name=\"" + xml_escape(name) + "\" sheetId=\"" + std::to_string(i + 1) + "\" r:id=\"rId" + std::to_string(i + 1) + "\"/>";
    }
    xml += "</sheets>";
    xml += "</workbook>";
    return xml;
}

static std::string gen_workbook_rels_xml(unsigned int sheet_count)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    for (unsigned int i = 1; i <= sheet_count; i++)
    {
        xml += "<Relationship Id=\"rId" + std::to_string(i) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet" + std::to_string(i) + ".xml\"/>";
    }
    xml += "<Relationship Id=\"rId" + std::to_string(sheet_count + 1) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>";
    xml += "<Relationship Id=\"rId" + std::to_string(sheet_count + 2) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"theme/theme1.xml\"/>";
    xml += "<Relationship Id=\"rId" + std::to_string(sheet_count + 3) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\" Target=\"sharedStrings.xml\"/>";
    xml += "</Relationships>";
    return xml;
}

static std::string gen_shared_strings_xml(const std::vector<std::string> &shared_strings)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"" + std::to_string(shared_strings.size()) + "\" uniqueCount=\"" + std::to_string(shared_strings.size()) + "\">";
    for (auto &s : shared_strings)
    {
        xml += "<si><t>" + xml_escape(s) + "</t></si>";
    }
    xml += "</sst>";
    return xml;
}

static std::string gen_styles_xml(const std::vector<PZ_EXCEL_CELL_FONT> &fonts,
                                  const std::vector<PZ_EXCEL_CELL_FILL> &bgcolors,
                                  const std::vector<PZ_EXCEL_CELL_BORDER> &borders,
                                  const std::vector<PZ_EXCEL_CELL_STYLE> &styles)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";

    xml += "<fonts count=\"" + std::to_string(fonts.size()) + "\">";
    for (auto &f : fonts)
    {
        xml += "<font>";
        if (f.bold)
            xml += "<b/>";
        if (f.italic)
            xml += "<i/>";
        if (f.strike)
            xml += "<strike/>";
        if (!f.underline.empty())
            xml += "<u val=\"" + xml_escape(f.underline) + "\"/>";
        xml += "<sz val=\"" + std::to_string(f.size) + "\"/>";
        if (!f.color.empty())
        {
            xml += "<color rgb=\"" + xml_escape(f.color) + "\"/>";
        }
        else
        {
            xml += "<color theme=\"1\"/>";
        }
        xml += "<name val=\"" + xml_escape(f.name) + "\"/>";
        if (!f.family.empty())
            xml += "<family val=\"" + xml_escape(f.family) + "\"/>";
        if (!f.charset.empty())
            xml += "<charset val=\"" + xml_escape(f.charset) + "\"/>";
        if (!f.scheme.empty())
            xml += "<scheme val=\"" + xml_escape(f.scheme) + "\"/>";
        xml += "</font>";
    }
    xml += "</fonts>";

    xml += "<fills count=\"" + std::to_string(bgcolors.size()) + "\">";
    for (auto &f : bgcolors)
    {
        xml += "<fill><patternFill patternType=\"" + f.pattern_type + "\">";
        if (!f.fgColor.empty())
            xml += "<fgColor rgb=\"" + f.fgColor + "\"/>";
        if (!f.bgcolor.empty())
            xml += "<bgColor rgb=\"" + f.bgcolor + "\"/>";
        xml += "</patternFill></fill>";
    }
    xml += "</fills>";

    xml += "<borders count=\"" + std::to_string(borders.size()) + "\">";
    for (auto &b : borders)
    {
        xml += "<border>";
        xml += "<left";
        if (!b.left.style.empty())
            xml += " style=\"" + xml_escape(b.left.style) + "\"";
        xml += ">";
        if (!b.left.color.empty())
            xml += "<color rgb=\"" + xml_escape(b.left.color) + "\"/>";
        xml += "</left>";
        xml += "<right";
        if (!b.right.style.empty())
            xml += " style=\"" + xml_escape(b.right.style) + "\"";
        xml += ">";
        if (!b.right.color.empty())
            xml += "<color rgb=\"" + xml_escape(b.right.color) + "\"/>";
        xml += "</right>";
        xml += "<top";
        if (!b.top.style.empty())
            xml += " style=\"" + xml_escape(b.top.style) + "\"";
        xml += ">";
        if (!b.top.color.empty())
            xml += "<color rgb=\"" + xml_escape(b.top.color) + "\"/>";
        xml += "</top>";
        xml += "<bottom";
        if (!b.bottom.style.empty())
            xml += " style=\"" + xml_escape(b.bottom.style) + "\"";
        xml += ">";
        if (!b.bottom.color.empty())
            xml += "<color rgb=\"" + xml_escape(b.bottom.color) + "\"/>";
        xml += "</bottom>";
        xml += "<diagonal";
        if (!b.diagonal.style.empty())
            xml += " style=\"" + xml_escape(b.diagonal.style) + "\"";
        xml += ">";
        if (!b.diagonal.color.empty())
            xml += "<color rgb=\"" + xml_escape(b.diagonal.color) + "\"/>";
        xml += "</diagonal>";
        xml += "</border>";
    }
    xml += "</borders>";

    xml += "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>";

    xml += "<cellXfs count=\"" + std::to_string(styles.size()) + "\">";
    for (auto &s : styles)
    {
        xml += "<xf numFmtId=\"" + std::to_string(s.numfmtid) + "\" fontId=\"" + std::to_string(s.fontid) + "\" fillId=\"" + std::to_string(s.fillid) + "\" borderId=\"" + std::to_string(s.borderid) + "\" xfId=\"" + std::to_string(s.xfid) + "\"";
        if (s.apply_font)
            xml += " applyFont=\"1\"";
        if (s.apply_fill)
            xml += " applyFill=\"1\"";
        if (s.apply_border)
            xml += " applyBorder=\"1\"";
        if (s.apply_fmt)
            xml += " applyNumberFormat=\"1\"";
        xml += ">";
        if (!s.align.empty() || !s.valign.empty())
        {
            xml += "<alignment";
            if (!s.align.empty())
                xml += " horizontal=\"" + s.align + "\"";
            if (!s.valign.empty())
                xml += " vertical=\"" + s.valign + "\"";
            xml += "/>";
        }
        xml += "</xf>";
    }
    xml += "</cellXfs>";

    xml += "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>";
    xml += "</styleSheet>";
    return xml;
}

static std::string gen_sheet_xml(const PZ_EXCEL_SHEET &sheet,[[maybe_unused]] const std::vector<std::string> &shared_strings, const std::vector<PZ_EXCEL_CELL_STYLE> &styles)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";

    unsigned int max_row = sheet.rows;
    unsigned int max_col = sheet.cols;
    if (max_row > 0 && max_col > 0)
    {
        xml += "<dimension ref=\"A1:" + col_num_to_str(max_col) + std::to_string(max_row) + "\"/>";
    }

    xml += "<sheetViews><sheetView workbookViewId=\"0\"/></sheetViews>";
    xml += "<sheetFormatPr defaultRowHeight=\"15\"/>";
    xml += "<sheetData>";

    for (auto &row_pair : sheet.values)
    {
        unsigned int row_num = row_pair.first;
        xml += "<row r=\"" + std::to_string(row_num) + "\">";
        for (auto &col_pair : row_pair.second)
        {
            unsigned int col_num      = col_pair.first;
            std::string cell_ref      = col_num_to_str(col_num) + std::to_string(row_num);
            const PZ_EXCEL_CELL &cell = col_pair.second;

            std::string attrs = "r=\"" + cell_ref + "\"";
            if (!styles.empty() && cell.s_type < styles.size() && cell.s_type != 0)
            {
                attrs += " s=\"" + std::to_string(cell.s_type) + "\"";
            }

            if (cell.v_type < 3)
            {
                xml += "<c " + attrs + "><v>" + std::to_string(cell.value.i) + "</v></c>";
            }
            else if (cell.v_type < 5)
            {
                xml += "<c " + attrs + "><v>" + std::to_string(cell.value.d) + "</v></c>";
            }
            else if (cell.v_type < 7)
            {
                xml += "<c " + attrs + " t=\"s\"><v>" + std::to_string(cell.value.i) + "</v></c>";
            }
            else if (!styles.empty() && cell.s_type < styles.size() && cell.s_type != 0)
            {
                xml += "<c " + attrs + "/>";
            }
        }
        xml += "</row>";
    }

    xml += "</sheetData>";
    xml += "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>";
    xml += "</worksheet>";
    return xml;
}

static std::string gen_core_xml()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
           "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
           "<dc:creator>pz_excel</dc:creator>"
           "<cp:lastModifiedBy>pz_excel</cp:lastModifiedBy>"
           "</cp:coreProperties>";
}

static std::string gen_app_xml(const std::vector<PZ_EXCEL_SHEET> &sheets)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">";
    xml += "<Application>pz_excel</Application>";
    xml += "<HeadingPairs><vt:vector size=\"2\" baseType=\"variant\"><vt:variant><vt:lpstr>工作表</vt:lpstr></vt:variant><vt:variant><vt:i4>" + std::to_string(sheets.size()) + "</vt:i4></vt:variant></vt:vector></HeadingPairs>";
    xml += "<TitlesOfParts><vt:vector size=\"" + std::to_string(sheets.size()) + "\" baseType=\"lpstr\">";
    for (auto &s : sheets)
    {
        std::string name = s.name.empty() ? "Sheet1" : s.name;
        xml += "<vt:lpstr>" + xml_escape(name) + "</vt:lpstr>";
    }
    xml += "</vt:vector></TitlesOfParts>";
    xml += "</Properties>";
    return xml;
}

static std::string gen_theme_xml()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
           "<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"Office Theme\">"
           "<a:themeElements>"
           "<a:clrScheme name=\"Office\">"
           "<a:dk1><a:sysClr val=\"windowText\" lastClr=\"000000\"/></a:dk1>"
           "<a:lt1><a:sysClr val=\"window\" lastClr=\"FFFFFF\"/></a:lt1>"
           "<a:dk2><a:srgbClr val=\"44546A\"/></a:dk2>"
           "<a:lt2><a:srgbClr val=\"E7E6E6\"/></a:lt2>"
           "<a:accent1><a:srgbClr val=\"4472C4\"/></a:accent1>"
           "<a:accent2><a:srgbClr val=\"ED7D31\"/></a:accent2>"
           "<a:accent3><a:srgbClr val=\"A5A5A5\"/></a:accent3>"
           "<a:accent4><a:srgbClr val=\"FFC000\"/></a:accent4>"
           "<a:accent5><a:srgbClr val=\"5B9BD5\"/></a:accent5>"
           "<a:accent6><a:srgbClr val=\"70AD47\"/></a:accent6>"
           "<a:hlink><a:srgbClr val=\"0563C1\"/></a:hlink>"
           "<a:folHlink><a:srgbClr val=\"954F72\"/></a:folHlink>"
           "</a:clrScheme>"
           "<a:fontScheme name=\"Office\">"
           "<a:majorFont><a:latin typeface=\"Calibri Light\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:majorFont>"
           "<a:minorFont><a:latin typeface=\"Calibri\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:minorFont>"
           "</a:fontScheme>"
           "<a:fmtScheme name=\"Office\">"
           "<a:fillStyleLst><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
           "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:tint val=\"50000\"/><a:satMod val=\"300000\"/></a:schemeClr></a:gs><a:gs pos=\"35000\"><a:schemeClr val=\"phClr\"><a:tint val=\"37000\"/><a:satMod val=\"300000\"/></a:schemeClr></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:tint val=\"15000\"/><a:satMod val=\"350000\"/></a:schemeClr></a:gs></a:gsLst><a:lin ang=\"16200000\" scaled=\"1\"/></a:gradFill>"
           "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:shade val=\"51000\"/><a:satMod val=\"130000\"/></a:schemeClr></a:gs><a:gs pos=\"80000\"><a:schemeClr val=\"phClr\"><a:shade val=\"93000\"/><a:satMod val=\"130000\"/></a:schemeClr></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:shade val=\"94000\"/><a:satMod val=\"135000\"/></a:schemeClr></a:gs></a:gsLst><a:lin ang=\"16200000\" scaled=\"0\"/></a:gradFill>"
           "</a:fillStyleLst>"
           "<a:lnStyleLst><a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
           "<a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
           "<a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\"><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill></a:ln>"
           "</a:lnStyleLst>"
           "<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>"
           "<a:bgFillStyleLst><a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"
           "<a:solidFill><a:schemeClr val=\"phClr\"><a:tint val=\"95000\"/><a:satMod val=\"170000\"/></a:schemeClr></a:solidFill>"
           "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:tint val=\"93000\"/><a:satMod val=\"150000\"/><a:shade val=\"98000\"/><a:lumMod val=\"102000\"/></a:schemeClr></a:gs><a:gs pos=\"50000\"><a:schemeClr val=\"phClr\"><a:tint val=\"98000\"/><a:satMod val=\"130000\"/><a:shade val=\"90000\"/><a:lumMod val=\"103000\"/></a:schemeClr></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:shade val=\"63000\"/><a:satMod val=\"120000\"/></a:schemeClr></a:gs></a:gsLst><a:path path=\"circle\"><a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/></a:path></a:gradFill>"
           "</a:bgFillStyleLst>"
           "</a:fmtScheme>"
           "</a:themeElements>"
           "<a:objectDefaults/>"
           "<a:extraClrSchemeLst/>"
           "</a:theme>";
}

static bool zip_add_file(zipFile zf, const char *filename, const std::string &content)
{
    zip_fileinfo fileInfo = {};
    if (zipOpenNewFileInZip(zf, filename, &fileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        return false;
    }
    if (zipWriteInFileInZip(zf, content.c_str(), content.size()) != Z_OK)
    {
        zipCloseFileInZip(zf);
        return false;
    }
    zipCloseFileInZip(zf);
    return true;
}

bool excel::write(const std::string &zipfilename)
{
    error_msg.clear();

    if (sheets.empty())
    {
        PZ_EXCEL_SHEET s;
        s.name        = "Sheet1";
        s.sheet_index = 1;
        sheets.push_back(std::move(s));
    }

    ensure_default_style(fonts, bgcolors, borders, styles);

    zipFile zf = zipOpen(zipfilename.c_str(), APPEND_STATUS_CREATE);
    if (zf == NULL)
    {
        error_msg = "Error: Unable to create zip file " + zipfilename;
        return false;
    }

    unsigned int sheet_count = sheets.size();

    if (!zip_add_file(zf, "[Content_Types].xml", gen_content_types_xml(sheet_count)))
    {
        error_msg = "Error: Failed to add [Content_Types].xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "_rels/.rels", gen_rels_xml()))
    {
        error_msg = "Error: Failed to add _rels/.rels";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "docProps/core.xml", gen_core_xml()))
    {
        error_msg = "Error: Failed to add docProps/core.xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "docProps/app.xml", gen_app_xml(sheets)))
    {
        error_msg = "Error: Failed to add docProps/app.xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "xl/workbook.xml", gen_workbook_xml(sheets)))
    {
        error_msg = "Error: Failed to add xl/workbook.xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "xl/_rels/workbook.xml.rels", gen_workbook_rels_xml(sheet_count)))
    {
        error_msg = "Error: Failed to add xl/_rels/workbook.xml.rels";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "xl/sharedStrings.xml", gen_shared_strings_xml(shared_strings)))
    {
        error_msg = "Error: Failed to add xl/sharedStrings.xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "xl/styles.xml", gen_styles_xml(fonts, bgcolors, borders, styles)))
    {
        error_msg = "Error: Failed to add xl/styles.xml";
        zipClose(zf, NULL);
        return false;
    }

    if (!zip_add_file(zf, "xl/theme/theme1.xml", gen_theme_xml()))
    {
        error_msg = "Error: Failed to add xl/theme/theme1.xml";
        zipClose(zf, NULL);
        return false;
    }

    for (unsigned int i = 0; i < sheet_count; i++)
    {
        std::string sheet_path = "xl/worksheets/sheet" + std::to_string(i + 1) + ".xml";
        if (!zip_add_file(zf, sheet_path.c_str(), gen_sheet_xml(sheets[i], shared_strings, styles)))
        {
            error_msg = "Error: Failed to add " + sheet_path;
            zipClose(zf, NULL);
            return false;
        }
    }

    zipClose(zf, NULL);
    return true;
}

}// namespace pz

#endif// ENABLE_MINIZIP