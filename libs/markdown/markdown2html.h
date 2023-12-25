#ifndef LIBS_MARKDOWN_MARKDOWN2HTML_H
#define LIBS_MARKDOWN_MARKDOWN2HTML_H
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <map>
namespace http
{
class markdown2html
{
  public:
    void parser(const std::string &);
    void parserfile(const std::string &);
    void strip_blank();
    void process_blockquote();
    void process_bold();
    void process_href();
    void process_p();
    void process_h();
    void process_hr();
    void process_table();
    void process_img();
    void process_dan();
    void process_html();
    void process_delline();
    void process_codeblock();
    void process_listul();
    void process_listuldot();
    void process_listulnum();
    void process();
    void backslash_transfer();

    void set_tags_classname(const std::string &, const std::string &);
    std::string get_htmlcontent();

  public:
    std::string mdcontent;
    std::string htmlcontent;
    std::string error_msg;
    std::vector<std::pair<unsigned char, std::string>> toc_list;
    std::vector<std::pair<std::string, std::string>> ref_list;
    unsigned int md_size;
    unsigned int read_offset;
    std::map<std::string, std::string> tags_classname;
};
}// namespace http
#endif