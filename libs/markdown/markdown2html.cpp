#include <functional>
#include <string>
#include "markdown2html.h"
#include <vector>
#include <list>
#include <memory>

namespace http
{
void markdown2html::parser(const std::string &content)
{
    mdcontent = content;
    md_size   = mdcontent.size();
    process();
}
std::string markdown2html::get_htmlcontent()
{
    return htmlcontent;
}

void markdown2html::parserfile(const std::string &filename)
{
    std::unique_ptr<std::FILE, int (*)(FILE *)> fp(std::fopen(filename.c_str(), "rb"), std::fclose);

    if (!fp.get())
    {
        error_msg = " open markdown file error! ";
        return;
    }
    fseek(fp.get(), 0, SEEK_END);
    md_size = ftell(fp.get());
    fseek(fp.get(), 0, SEEK_SET);

    mdcontent.resize(md_size);
    md_size = fread((char *)&mdcontent[0], 1, md_size, fp.get());
    mdcontent.resize(md_size);
    process();
}
void markdown2html::strip_blank()
{
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] != 0x20)
        {
            return;
        }
    }
}
void markdown2html::process_blockquote()
{
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            if (read_offset > 1 && mdcontent[read_offset - 1] == 0x20 && mdcontent[read_offset - 2] == 0x20)
            {
                htmlcontent.append("<br/>");
            }
            read_offset += 1;
            strip_blank();
            if (read_offset < md_size && mdcontent[read_offset] != '>')
            {
                htmlcontent.append("</p></blockquote>");
                htmlcontent.push_back(0x0A);
                return;
            }
        }
        if (mdcontent[read_offset] == '>')
        {
            htmlcontent.append("<blockquote");
            if (tags_class.size() > 0)
            {
                auto tagiter = tags_class.find("blockquote");
                if (tagiter != tags_class.end())
                {
                    htmlcontent.append(" class=\"");
                    htmlcontent.append(tagiter->second);
                    htmlcontent.append("\" ");
                }
            }
            htmlcontent.append("><p>");
            continue;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
    if (read_offset == md_size)
    {
        htmlcontent.append("</p></blockquote>");
        htmlcontent.push_back(0x0A);
        return;
    }
}
void markdown2html::process_bold()
{
    std::string tempbold;
    unsigned int inboffset = read_offset;
    if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] == '*' && mdcontent[read_offset + 2] == '*')
    {
        htmlcontent.append("<strong><em>");
        read_offset += 3;
    }
    else if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] == '*')
    {
        htmlcontent.append("<strong>");
        read_offset += 2;
    }
    else if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] != '*')
    {
        htmlcontent.append("<em>");
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            if (mdcontent[read_offset + 1] == '>' || mdcontent[read_offset + 1] == '-' || mdcontent[read_offset + 1] == '#')
            {
                read_offset = inboffset;
                return;
            }
        }
        else if (mdcontent[read_offset] == '*')
        {
            if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] == '*' && mdcontent[read_offset + 2] == '*')
            {
                read_offset += 2;

                htmlcontent.append(tempbold);
                htmlcontent.append("</em></strong>");
            }
            else if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] == '*')
            {
                read_offset += 1;

                htmlcontent.append(tempbold);
                htmlcontent.append("</strong>");
            }
            else if (mdcontent[read_offset] == '*' && mdcontent[read_offset + 1] != '*')
            {
                // read_offset+=1;

                htmlcontent.append(tempbold);
                htmlcontent.append("</em>");
            }
            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '[')
        {
            unsigned int tempbnum = read_offset;
            process_href();
            if (tempbnum != read_offset)
            {
                continue;
            }
        }
        tempbold.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_href()
{
    if (mdcontent[read_offset] == '[' && mdcontent[read_offset + 1] == 0x20 && mdcontent[read_offset + 2] == ']')
    {
        htmlcontent.append("☑️");
        read_offset += 3;
        return;
    }
    else if (mdcontent[read_offset] == '[' && mdcontent[read_offset + 1] == 'x' && mdcontent[read_offset + 2] == ']')
    {
        htmlcontent.append("✅");
        read_offset += 3;
        return;
    }
    else if (mdcontent[read_offset] == '[' && mdcontent[read_offset + 1] == 'X' && mdcontent[read_offset + 2] == ']')
    {
        htmlcontent.append("✅");
        read_offset += 3;
        return;
    }
    if (mdcontent[read_offset] == '[' && mdcontent[read_offset + 1] == ']')
    {
        return;
    }
    std::string tempstr;
    unsigned int isspacenum = 0;
    unsigned int inoffset   = read_offset;
    if (mdcontent[read_offset] == '[')
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == ']')
        {
            if (isspacenum == tempstr.size())
            {
                read_offset = inoffset;
                return;
            }
            break;
        }
        if (mdcontent[read_offset] == 0x20)
        {
            isspacenum++;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        tempstr.push_back(mdcontent[read_offset]);
    }
    if (mdcontent[read_offset] == ']')
    {
        read_offset += 1;
    }
    // strip_blank();
    if (mdcontent[read_offset] == '(')
    {
        read_offset += 1;
    }
    else
    {
        read_offset = inoffset;
        return;
    }
    std::string hreftemp;
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == ')')
        {
            htmlcontent.append("<a href=\"");

            htmlcontent.append(hreftemp);
            htmlcontent.append("\" ");
            if (tags_class.size() > 0)
            {
                auto tagiter = tags_class.find("a");
                if (tagiter != tags_class.end())
                {
                    htmlcontent.append(" class=\"");
                    htmlcontent.append(tagiter->second);
                    htmlcontent.append("\" ");
                }
            }
            htmlcontent.append(">");
            htmlcontent.append(tempstr);
            htmlcontent.append("</a>");
            // read_offset+=1;
            return;
        }
        if (mdcontent[read_offset] == 0x20)
        {
            read_offset += 1;
            if (mdcontent[read_offset] == '"')
            {
                read_offset += 1;
            }
            else
            {
                read_offset = inoffset;
                return;
            }
            std::string tempref;
            for (; read_offset < md_size; read_offset++)
            {
                if (mdcontent[read_offset] == '"')
                {
                    htmlcontent.append("<span class=\"footnote-word\">");//<span class="footnote-word">title</span>
                    htmlcontent.append(tempstr);
                    htmlcontent.append("</span>");//<sup class="footnote-ref">[4]</sup>
                    htmlcontent.append("<sup class=\"footnote-ref\">[");
                    unsigned char tempnum = ref_list.size() + 1 + '0';
                    ref_list.push_back({tempref, hreftemp});
                    htmlcontent.push_back(tempnum);
                    htmlcontent.append("]</sup>");
                    for (; read_offset < md_size; read_offset++)
                    {
                        if (mdcontent[read_offset] == ')')
                        {
                            break;
                        }
                    }
                    break;
                }
                tempref.push_back(mdcontent[read_offset]);
            }
            return;
        }
        hreftemp.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_dan()
{
    htmlcontent.append("<code>");
    if (mdcontent[read_offset] == 0x60)
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x60)
        {
            htmlcontent.append("</code>");
            return;
        }
        if (mdcontent[read_offset] == '\\')
        {
            backslash_transfer();
            continue;
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_codeblock()
{
    htmlcontent.append("<pre");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("code");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append("><code class=\"language");
    if (mdcontent[read_offset] == 0x60)
    {
        read_offset += 3;
    }
    std::string tempstr;
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            read_offset += 1;
            if (tempstr.size() > 0)
            {
                htmlcontent.push_back('-');
            }
            htmlcontent.append(tempstr);
            htmlcontent.push_back('"');
            htmlcontent.append(">");
            break;
        }
        tempstr.push_back(mdcontent[read_offset]);
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            if (mdcontent[read_offset + 1] == 0x60 && mdcontent[read_offset + 2] == 0x60 && mdcontent[read_offset + 3] == 0x60)
            {
                htmlcontent.append("</code></pre>");
                htmlcontent.push_back(0x0A);
                read_offset += 4;
                return;
            }
        }
        if (mdcontent[read_offset] == '<')
        {
            htmlcontent.append("&lt;");
            continue;
        }
        else if (mdcontent[read_offset] == '>')
        {
            htmlcontent.append("&gt;");
            continue;
        }
        else if (mdcontent[read_offset] == '"')
        {
            htmlcontent.append("&quot;");
            continue;
        }
        else if (mdcontent[read_offset] == 0x27)
        {
            htmlcontent.append("&apos;");
            continue;
        }
        //&apos;
        htmlcontent.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_h()
{
    std::string temphstr;
    unsigned char typefront = 0;
    if (mdcontent[read_offset] == '#' && mdcontent[read_offset + 1] == '#' && mdcontent[read_offset + 2] == '#' && mdcontent[read_offset + 3] == '#' && mdcontent[read_offset + 4] == '#' && mdcontent[read_offset + 5] == '#')
    {
        htmlcontent.append("<h6");
        read_offset += 6;
        typefront = 6;
    }
    else if (mdcontent[read_offset] == '#' && mdcontent[read_offset + 1] == '#' && mdcontent[read_offset + 2] == '#' && mdcontent[read_offset + 3] == '#' && mdcontent[read_offset + 4] == '#')
    {
        htmlcontent.append("<h5");
        read_offset += 5;
        typefront = 5;
    }
    else if (mdcontent[read_offset] == '#' && mdcontent[read_offset + 1] == '#' && mdcontent[read_offset + 2] == '#' && mdcontent[read_offset + 3] == '#')
    {
        htmlcontent.append("<h4");
        read_offset += 4;
        typefront = 4;
    }
    else if (mdcontent[read_offset] == '#' && mdcontent[read_offset + 1] == '#' && mdcontent[read_offset + 2] == '#')
    {
        htmlcontent.append("<h3");
        read_offset += 3;
        typefront = 3;
    }
    else if (mdcontent[read_offset] == '#' && mdcontent[read_offset + 1] == '#')
    {
        htmlcontent.append("<h2");
        read_offset += 2;
        typefront = 2;
    }
    else if (mdcontent[read_offset] == '#')
    {
        htmlcontent.append("<h1");
        read_offset += 1;
        typefront = 1;
    }
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("h");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append(">");
    if (mdcontent[read_offset] == 0x20)
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            htmlcontent.append(temphstr);
            htmlcontent.append("</h");
            toc_list.push_back({typefront, temphstr});
            typefront += '0';
            htmlcontent.push_back(typefront);
            htmlcontent.append(">");

            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        if (mdcontent[read_offset] == 0x60)
        {
            process_dan();
            continue;
        }
        temphstr.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_hr()
{
    int j = (int)read_offset - 1;
    for (; j >= 0; j--)
    {
        if (mdcontent[j] != 0x20)
        {
            break;
        }
    }
    if (j > 0 && mdcontent[j] != 0x0A)
    {
        process_p();
        return;
    }
    j--;
    for (; j >= 0; j--)
    {
        if (mdcontent[j] != 0x20 && mdcontent[j] != 0x0A)
        {
            process_p();
            return;
        }
        if (mdcontent[j] == 0x0A)
        {
            break;
        }
    }
    htmlcontent.push_back(0x0A);
    htmlcontent.append("<hr />");
    htmlcontent.push_back(0x0A);
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            read_offset += 1;
            return;
        }
    }
}
void markdown2html::process_p()
{
    htmlcontent.append("<p");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("p");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append(">");
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            if (read_offset > 1 && mdcontent[read_offset - 1] == 0x20 && mdcontent[read_offset - 2] == 0x20)
            {
                htmlcontent.append("<br/>");
            }
            read_offset += 1;
            htmlcontent.append("</p>");
            htmlcontent.push_back(0x0A);
            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        if (mdcontent[read_offset] == 0x60)
        {
            process_dan();
            continue;
        }
        if (mdcontent[read_offset] == '[')
        {
            unsigned int tempbnum = read_offset;
            process_href();
            if (tempbnum != read_offset)
            {
                continue;
            }
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
    if (read_offset == md_size)
    {
        htmlcontent.append("</p>");
        htmlcontent.push_back(0x0A);
        return;
    }
}
void markdown2html::process_delline()
{
    unsigned char isdel   = 0;
    unsigned int inoffset = read_offset;
    std::string tempstr;
    if (mdcontent[read_offset] == '~')
    {
        read_offset += 1;
        isdel += 1;
    }
    if (mdcontent[read_offset] == '~')
    {
        read_offset += 1;
        isdel += 1;
    }
    if (isdel < 2)
    {
        read_offset = inoffset;
        return;
    }
    isdel = 0;
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == '~')
        {
            tempstr.push_back(mdcontent[read_offset]);
        }
        break;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == '~')
        {
            if (read_offset < md_size && mdcontent[read_offset + 1] == '~')
            {
                htmlcontent.append("<del>");
                htmlcontent.append(tempstr);
                htmlcontent.append("</del>");
                read_offset += 1;
                return;
            }
        }
        if (mdcontent[read_offset] == 0x0A)
        {
            read_offset = inoffset;
            return;
        }
        tempstr.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_html()
{
    std::string tags;
    std::vector<std::string> taglevels;
    unsigned char nilinktype = 1;
    if (mdcontent[read_offset] == '<')
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x20)
        {
            nilinktype = 0;
            break;
        }
        if (mdcontent[read_offset] == '>')
        {
            if (tags.size() > 0 && tags.back() == '/')
            {
                tags.resize(tags.size() - 1);
                htmlcontent.push_back('<');
                htmlcontent.append(tags);
                htmlcontent.append(" />");
                read_offset += 1;
                nilinktype = 2;
                return;
            }
            if (tags == "img" || tags == "hr" || tags == "br")
            {
                htmlcontent.push_back('<');
                htmlcontent.append(tags);
                htmlcontent.append(" />");
                read_offset += 1;
                nilinktype = 3;
                return;
            }

            for (unsigned int j = 0; j < tags.size(); j++)
            {
                if (tags[j] == '/')
                {
                    nilinktype = 11;
                    break;
                }
                else if (tags[j] == '@')
                {
                    nilinktype = 12;
                    break;
                }
            }
            if (nilinktype == 12)
            {
                htmlcontent.append("<a href=\"mailto:");
            }
            else if (nilinktype == 11)
            {
                htmlcontent.append("<a href=\"");
            }

            if (nilinktype > 9)
            {
                htmlcontent.append(tags);
                htmlcontent.append("\">");
                htmlcontent.append(tags);
                htmlcontent.append("</a>");
                read_offset += 1;
                return;
            }
            break;
        }
        tags.push_back(mdcontent[read_offset]);
    }
    htmlcontent.push_back('<');
    htmlcontent.append(tags);
    if (nilinktype == 1)
    {
        htmlcontent.push_back('>');
        read_offset += 1;
        taglevels.push_back(tags);
    }
    // htmlcontent.push_back('1');
    if (nilinktype == 0)
    {
        for (; read_offset < md_size; read_offset++)
        {
            if (mdcontent[read_offset] == '>')
            {
                htmlcontent.push_back(mdcontent[read_offset]);
                if (mdcontent[read_offset - 1] == '/')
                {
                    read_offset += 1;
                    return;
                }
                if (tags == "img" || tags == "hr" || tags == "br")
                {
                    read_offset += 1;
                    return;
                }
                read_offset += 1;
                taglevels.push_back(tags);
                break;
            }
            htmlcontent.push_back(mdcontent[read_offset]);
        }
    }

    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == '<')
        {
            htmlcontent.push_back(mdcontent[read_offset]);
            read_offset += 1;
            if (read_offset < md_size && mdcontent[read_offset] == '/')
            {
                htmlcontent.push_back(mdcontent[read_offset]);
                read_offset += 1;
                std::string close_tags;
                for (; read_offset < md_size; read_offset++)
                {
                    htmlcontent.push_back(mdcontent[read_offset]);
                    close_tags.push_back(mdcontent[read_offset]);
                    if (mdcontent[read_offset] == '>')
                    {
                        close_tags.resize(close_tags.size() - 1);
                        for (int m = (int)taglevels.size() - 1; m > -1; m--)
                        {
                            if (taglevels[m] == close_tags)
                            {
                                taglevels.pop_back();
                                if (taglevels.size() == 0)
                                {
                                    read_offset += 1;
                                    return;
                                }
                                break;
                            }
                            taglevels.pop_back();
                            m = (int)taglevels.size();
                            if (m == 0)
                            {
                                return;
                            }
                        }
                        break;
                    }
                }
            }
            else
            {
                tags.clear();
                nilinktype = 1;
                for (; read_offset < md_size; read_offset++)
                {
                    if (mdcontent[read_offset] == 0x20)
                    {
                        nilinktype = 0;
                        break;
                    }
                    if (mdcontent[read_offset] == '>')
                    {
                        if (tags.size() > 0 && tags.back() == '/')
                        {
                            tags.resize(tags.size() - 1);
                            htmlcontent.append(tags);
                            htmlcontent.append(" />");
                            read_offset += 1;
                            tags.clear();
                            break;
                        }
                        if (tags == "img" || tags == "hr" || tags == "br")
                        {
                            htmlcontent.append(tags);
                            htmlcontent.append(" />");
                            read_offset += 1;
                            nilinktype = 3;
                            tags.clear();
                            break;
                        }

                        for (unsigned int j = 0; j < tags.size(); j++)
                        {
                            if (tags[j] == '/')
                            {
                                nilinktype = 11;
                                break;
                            }
                            else if (tags[j] == '@')
                            {
                                nilinktype = 12;
                                break;
                            }
                        }
                        if (nilinktype == 12)
                        {
                            htmlcontent.append("a href=\"mailto:");
                        }
                        else if (nilinktype == 11)
                        {
                            htmlcontent.append("a href=\"");
                        }

                        if (nilinktype > 9)
                        {
                            htmlcontent.append(tags);
                            htmlcontent.append("\">");
                            htmlcontent.append(tags);
                            htmlcontent.append("</a>");
                            read_offset += 1;
                            tags.clear();
                            break;
                        }
                        break;
                    }
                    tags.push_back(mdcontent[read_offset]);
                }
                if (tags.empty())
                {
                    continue;
                }
                htmlcontent.append(tags);
                if (nilinktype == 1)
                {
                    htmlcontent.push_back('>');
                    // read_offset += 1;
                }

                if (nilinktype == 0)
                {
                    for (; read_offset < md_size; read_offset++)
                    {
                        if (mdcontent[read_offset] == '>')
                        {
                            htmlcontent.push_back(mdcontent[read_offset]);
                            if (mdcontent[read_offset - 1] == '/')
                            {
                                break;
                            }
                            if (tags == "img" || tags == "hr" || tags == "br")
                            {
                                break;
                            }
                            taglevels.push_back(tags);
                            break;
                        }
                        htmlcontent.push_back(mdcontent[read_offset]);
                    }
                }
                else if (nilinktype == 1)
                {
                    taglevels.push_back(tags);
                }
            }
            continue;
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
}
void markdown2html::process_listul()
{
    unsigned char levelnum = 0;
    htmlcontent.append("<ul");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("ul");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append("><li>");
    if (mdcontent[read_offset] == '-')
    {
        read_offset += 1;
    }
    if (mdcontent[read_offset] == 0x20)
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            unsigned int bioffset = read_offset;
            read_offset += 1;
            unsigned char nextis  = 0;
            unsigned char temp_lv = 0;
            for (; read_offset < md_size; read_offset++)
            {
                if (mdcontent[read_offset] == 0x20)
                {
                    temp_lv += 1;
                    continue;
                }
                else if (mdcontent[read_offset] == 0x09)
                {
                    temp_lv += 2;
                    continue;
                }
                break;
            }
            temp_lv = temp_lv / 2;
            nextis  = 0;
            if ((read_offset + 2) < md_size && mdcontent[read_offset] == '-' && mdcontent[read_offset + 1] == 0x20)
            {
                nextis = 1;
                read_offset += 1;
            }
            if (nextis == 1)
            {
                if (temp_lv > levelnum)
                {
                    htmlcontent.append("<ul><li>");
                    levelnum = temp_lv;
                    continue;
                }
                else if (temp_lv < levelnum)
                {
                    htmlcontent.append("</li></ul>");
                    htmlcontent.append("<li>");
                    levelnum = temp_lv;
                    continue;
                }
                levelnum = temp_lv;
                htmlcontent.append("</li><li>");
                continue;
            }
            for (unsigned char kp = 0; kp < levelnum; kp++)
            {
                htmlcontent.append("</li></ul>");
            }
            htmlcontent.append("</li></ul>");
            read_offset = bioffset;
            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        if (mdcontent[read_offset] == 0x60)
        {
            process_dan();
            continue;
        }
        if (mdcontent[read_offset] == '[')
        {
            unsigned int tempbnum = read_offset;
            process_href();
            if (tempbnum != read_offset)
            {
                continue;
            }
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
    if (read_offset == md_size)
    {
        htmlcontent.append("</li></ul>");
        htmlcontent.push_back(0x0A);
        return;
    }
}
void markdown2html::process_listuldot()
{
    unsigned char levelnum = 0;
    htmlcontent.append("<ul");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("ul*");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append("><li>");
    if (mdcontent[read_offset] == '*')
    {
        read_offset += 1;
    }
    if (mdcontent[read_offset] == 0x20)
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            unsigned int bioffset = read_offset;
            read_offset += 1;
            unsigned char nextis  = 0;
            unsigned char temp_lv = 0;
            for (; read_offset < md_size; read_offset++)
            {
                if (mdcontent[read_offset] == 0x20)
                {
                    temp_lv += 1;
                    continue;
                }
                else if (mdcontent[read_offset] == 0x09)
                {
                    temp_lv += 2;
                    continue;
                }
                break;
            }
            temp_lv = temp_lv / 2;
            nextis  = 0;
            if ((read_offset + 2) < md_size && mdcontent[read_offset] == '-' && mdcontent[read_offset + 1] == 0x20)
            {
                nextis = 1;
                read_offset += 1;
            }
            if (nextis == 1)
            {
                if (temp_lv > levelnum)
                {
                    htmlcontent.append("<ul><li>");
                    levelnum = temp_lv;
                    continue;
                }
                else if (temp_lv < levelnum)
                {
                    htmlcontent.append("</li></ul>");
                    htmlcontent.append("<li>");
                    levelnum = temp_lv;
                    continue;
                }
                levelnum = temp_lv;
                htmlcontent.append("</li><li>");
                continue;
            }
            for (unsigned char kp = 0; kp < levelnum; kp++)
            {
                htmlcontent.append("</li></ul>");
            }
            htmlcontent.append("</li></ul>");
            read_offset = bioffset;
            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        if (mdcontent[read_offset] == 0x60)
        {
            process_dan();
            continue;
        }
        if (mdcontent[read_offset] == '[')
        {
            unsigned int tempbnum = read_offset;
            process_href();
            if (tempbnum != read_offset)
            {
                continue;
            }
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
    if (read_offset == md_size)
    {
        htmlcontent.append("</li></ul>");
        htmlcontent.push_back(0x0A);
        return;
    }
}
void markdown2html::process_listulnum()
{
    unsigned char levelnum = 0;
    htmlcontent.append("<ol");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("ol");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append("><li>");
    if (mdcontent[read_offset] > 0x2F && mdcontent[read_offset] < 0x3A)
    {
        if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] > 0x2F && mdcontent[read_offset + 1] < 0x3A)
        {
            if (mdcontent[read_offset + 2] == '.' && mdcontent[read_offset + 3] == 0x20)
            {
                read_offset += 3;
            }
        }
        else if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] == '.' && mdcontent[read_offset + 2] == 0x20)
        {
            read_offset += 2;
        }
    }
    if (mdcontent[read_offset] == 0x20)
    {
        read_offset += 1;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            unsigned int bioffset = read_offset;
            read_offset += 1;
            unsigned char nextis  = 0;
            unsigned char temp_lv = 0;
            for (; read_offset < md_size; read_offset++)
            {
                if (mdcontent[read_offset] == 0x20)
                {
                    temp_lv += 1;
                    continue;
                }
                else if (mdcontent[read_offset] == 0x09)
                {
                    temp_lv += 2;
                    continue;
                }
                break;
            }
            temp_lv = temp_lv / 2;
            nextis  = 0;
            if (mdcontent[read_offset] > 0x2F && mdcontent[read_offset] < 0x3A)
            {
                if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] > 0x2F && mdcontent[read_offset + 1] < 0x3A)
                {
                    if (mdcontent[read_offset + 2] == '.' && mdcontent[read_offset + 3] == 0x20)
                    {
                        nextis = 1;
                        read_offset += 3;
                    }
                }
                else if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] == '.' && mdcontent[read_offset + 2] == 0x20)
                {
                    nextis = 1;
                    read_offset += 2;
                }
            }
            if (nextis == 1)
            {
                if (temp_lv > levelnum)
                {
                    htmlcontent.append("<ol><li>");
                    levelnum = temp_lv;
                    continue;
                }
                else if (temp_lv < levelnum)
                {
                    htmlcontent.append("</li></ol>");
                    htmlcontent.append("<li>");
                    levelnum = temp_lv;
                    continue;
                }
                levelnum = temp_lv;
                htmlcontent.append("</li><li>");
                continue;
            }
            for (unsigned char kp = 0; kp < levelnum; kp++)
            {
                htmlcontent.append("</li></ol>");
            }
            htmlcontent.append("</li></ol>");
            read_offset = bioffset;
            return;
        }
        if (mdcontent[read_offset] == 0x5C)
        {
            backslash_transfer();
            continue;
        }
        if (mdcontent[read_offset] == '*')
        {
            process_bold();
            continue;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        if (mdcontent[read_offset] == 0x60)
        {
            process_dan();
            continue;
        }
        if (mdcontent[read_offset] == '[')
        {
            unsigned int tempbnum = read_offset;
            process_href();
            if (tempbnum != read_offset)
            {
                continue;
            }
        }
        htmlcontent.push_back(mdcontent[read_offset]);
    }
    if (read_offset == md_size)
    {
        htmlcontent.append("</li></ol>");
        htmlcontent.push_back(0x0A);
        return;
    }
}
void markdown2html::backslash_transfer()
{
    switch (mdcontent[read_offset])
    {
    case 0x5C:
        htmlcontent.push_back(mdcontent[read_offset]);
        break;
    case '*':
        htmlcontent.push_back(mdcontent[read_offset]);
        break;
    case '[':
        htmlcontent.push_back(mdcontent[read_offset]);
        break;
    }
}
void markdown2html::process_table()
{
    std::vector<std::string> headers;
    std::vector<unsigned char> headers_align;
    std::vector<std::vector<std::string>> tables_value;
    std::vector<std::string> headers_value_one;
    std::string headers_temp;
    read_offset += 1;
    strip_blank();
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == '|')
        {
            unsigned int a = 0;
            unsigned int b = 0;
            for (a = 0; a < headers_temp.size(); a++)
            {
                if (headers_temp[a] == 0x20)
                {
                    continue;
                }
                break;
            }
            for (int m = (int)headers_temp.size() - 1; m > -1; m--)
            {
                if (headers_temp[m] == 0x20)
                {
                    b += 1;
                    continue;
                }
                break;
            }
            b = headers_temp.size() - b - a;
            if (b > headers_temp.size())
            {
                b = headers_temp.size();
            }
            headers.push_back(headers_temp.substr(a, b));
            headers_temp.clear();
            read_offset += 1;
            strip_blank();
            read_offset -= 1;
            continue;
        }
        else if (mdcontent[read_offset] == 0x0A)
        {
            read_offset += 1;
            break;
        }
        headers_temp.push_back(mdcontent[read_offset]);
    }

    strip_blank();
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == 0x0A)
        {
            read_offset += 1;
            break;
        }
        if (mdcontent[read_offset] == '|')
        {
            read_offset += 1;
            strip_blank();
            unsigned char align_type = 0;
            if (read_offset < md_size && mdcontent[read_offset] == ':')
            {
                align_type = 1;
                read_offset += 1;
            }
            for (; read_offset < md_size; read_offset++)
            {
                if (mdcontent[read_offset] == ':')
                {
                    align_type += 3;
                    break;
                }
                if (mdcontent[read_offset] == '|')
                {
                    read_offset -= 1;
                    break;
                }
                if (mdcontent[read_offset] == 0x0A)
                {
                    break;
                }
            }
            if (mdcontent[read_offset] == 0x0A)
            {
                read_offset += 1;
                break;
            }
            headers_align.push_back(align_type);
            continue;
        }
    }
    strip_blank();
    if (mdcontent[read_offset] == '|')
    {
        read_offset += 1;
    }
    headers_temp.clear();
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == '|')
        {
            unsigned int a = 0;
            unsigned int b = 0;
            for (a = 0; a < headers_temp.size(); a++)
            {
                if (headers_temp[a] == 0x20)
                {
                    continue;
                }
                break;
            }
            for (int m = (int)headers_temp.size() - 1; m > -1; m--)
            {
                if (headers_temp[m] == 0x20)
                {
                    b += 1;
                    continue;
                }
                break;
            }
            b = headers_temp.size() - b - a;
            if (b > headers_temp.size())
            {
                b = headers_temp.size();
            }
            headers_value_one.push_back(headers_temp.substr(a, b));
            headers_temp.clear();
            read_offset += 1;
            strip_blank();
            read_offset -= 1;
            continue;
        }
        else if (mdcontent[read_offset] == 0x0A)
        {
            read_offset += 1;
            tables_value.push_back(headers_value_one);
            headers_value_one.clear();
            headers_temp.clear();
            strip_blank();
            if (mdcontent[read_offset] == '|')
            {
                continue;
            }
            break;
        }
        if (mdcontent[read_offset] == '~')
        {
            process_delline();
            continue;
        }
        headers_temp.push_back(mdcontent[read_offset]);
    }
    htmlcontent.append("<table");
    if (tags_class.size() > 0)
    {
        auto tagiter = tags_class.find("table");
        if (tagiter != tags_class.end())
        {
            htmlcontent.append(" class=\"");
            htmlcontent.append(tagiter->second);
            htmlcontent.append("\" ");
        }
    }
    htmlcontent.append(">\n<thead>\n<tr>\n");
    for (unsigned int i = 0; i < headers.size(); i++)
    {
        htmlcontent.append("<th");
        if (i < headers_align.size())
        {
            if (headers_align[i] == 1)
            {
                htmlcontent.append(" style=\"text-align:left\"");
            }
            else if (headers_align[i] == 3)
            {
                htmlcontent.append(" style=\"text-align:right\"");
            }
            else if (headers_align[i] == 4)
            {
                htmlcontent.append(" style=\"text-align:center\"");
            }
        }
        htmlcontent.append(">");
        htmlcontent.append(headers[i]);
        htmlcontent.append("</th>");
    }
    htmlcontent.append("\n</tr>\n</thead>\n<tbody>");
    for (unsigned int i = 0; i < tables_value.size(); i++)
    {
        htmlcontent.append("<tr>\n");
        for (unsigned int j = 0; j < tables_value[i].size(); j++)
        {
            htmlcontent.append("<td");
            if (j < headers_align.size())
            {
                if (headers_align[j] == 1)
                {
                    htmlcontent.append(" style=\"text-align:left\"");
                }
                else if (headers_align[j] == 3)
                {
                    htmlcontent.append(" style=\"text-align:right\"");
                }
                else if (headers_align[j] == 4)
                {
                    htmlcontent.append(" style=\"text-align:center\"");
                }
            }
            htmlcontent.append(">");
            htmlcontent.append(tables_value[i][j]);
            htmlcontent.append("</td>");
        }
        htmlcontent.append("\n</tr>\n");
    }
    htmlcontent.append("</tbody></table>");
}
void markdown2html::process_img()
{
    unsigned char imgtype = 0;
    unsigned int inoffset = read_offset;
    std::string tempstr;
    std::string tempstrurl;

    if (mdcontent[read_offset] == '!')
    {
        read_offset += 1;
    }
    if (mdcontent[read_offset] == 'v')
    {
        imgtype = 1;
        read_offset += 1;
    }
    if (mdcontent[read_offset] == '[')
    {
        read_offset += 1;
    }
    else
    {
        read_offset = inoffset;
        process_p();
        return;
    }
    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == ']')
        {
            read_offset += 1;
            break;
        }
        tempstr.push_back(mdcontent[read_offset]);
    }
    strip_blank();
    if (mdcontent[read_offset] == '(')
    {
        read_offset += 1;
    }

    for (; read_offset < md_size; read_offset++)
    {
        if (mdcontent[read_offset] == ')')
        {
            read_offset += 1;
            break;
        }
        tempstrurl.push_back(mdcontent[read_offset]);
    }
    if (imgtype == 0)
    {
        htmlcontent.append("<img src=\"");
        htmlcontent.append(tempstrurl);
        htmlcontent.append("\" ");
        if (tags_class.size() > 0)
        {
            auto tagiter = tags_class.find("img");
            if (tagiter != tags_class.end())
            {
                htmlcontent.append(" class=\"");
                htmlcontent.append(tagiter->second);
                htmlcontent.append("\" ");
            }
        }
        htmlcontent.append(" alt=\"");
        htmlcontent.append(tempstr);
        htmlcontent.append("\">");
    }
    else
    {
        htmlcontent.append("<video src=\"");
        htmlcontent.append(tempstrurl);
        htmlcontent.append("\" ");
        if (tags_class.size() > 0)
        {
            auto tagiter = tags_class.find("video");
            if (tagiter != tags_class.end())
            {
                htmlcontent.append(" class=\"");
                htmlcontent.append(tagiter->second);
                htmlcontent.append("\" ");
            }
        }
        htmlcontent.append(tempstr);
        htmlcontent.append(" controls=\"controls\">");
    }
}

void markdown2html::process()
{
    unsigned int testoffset = 0;
    for (; read_offset < md_size;)
    {
        strip_blank();
        testoffset = read_offset;
        if (mdcontent[read_offset] > 0x2F && mdcontent[read_offset] < 0x3A)
        {
            if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] > 0x2F && mdcontent[read_offset + 1] < 0x3A)
            {
                if (mdcontent[read_offset + 2] == '.' && mdcontent[read_offset + 3] == 0x20)
                {
                    process_listulnum();
                }
            }
            else if ((read_offset + 3 < md_size) && mdcontent[read_offset + 1] == '.' && mdcontent[read_offset + 2] == 0x20)
            {
                process_listulnum();
            }
        }
        switch (mdcontent[read_offset])
        {
        case '>':
            process_blockquote();
            break;
        case '|':
            process_table();
            break;
        case '!':
            process_img();
            break;
        case '~':
            process_p();
            break;
        case '<':
            if ((mdcontent[read_offset + 1] > 0x40 && mdcontent[read_offset + 1] < 0x5B) || (mdcontent[read_offset + 1] > 0x60 && mdcontent[read_offset + 1] < 0x7B))
            {
                process_html();
            }
            else
            {
                process_p();
            }
            break;
        case '*':
            if (mdcontent[read_offset + 1] == 0x20)
            {
                process_listuldot();
                break;
            }
            process_p();
            read_offset += 1;
            break;
        case '-':
            if (mdcontent[read_offset + 1] == 0x20)
            {
                process_listul();
            }
            else if (mdcontent[read_offset + 1] == '-' && mdcontent[read_offset + 2] == '-')
            {
                process_hr();
            }
            else
            {
                process_p();
            }
            break;
        case 0x0A:
            read_offset += 1;
            if (htmlcontent.size() > 0 && htmlcontent.back() != 0x0A)
            {
                htmlcontent.push_back(0x0A);
            }
            break;
        case 0x23:
            process_h();
            break;
        case 0x60:
            if (mdcontent[read_offset + 1] == 0x60 && mdcontent[read_offset + 2] == 0x60)
            {
                process_codeblock();
            }
            else
            {
                process_p();
            }
            break;
        default:
            process_p();
            break;
        }
        if (testoffset == read_offset)
        {
            read_offset += 1;
        }
    }
}

void markdown2html::set_tags_class(const std::string &k, const std::string &v)
{
    tags_class[k] = v;
}
}// namespace http