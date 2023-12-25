#include <chrono>
#include <thread>
#include "httppeer.h"
#include "markdown2html.h"

namespace http
{
//@urlpath(null,testmarkdown2html)
std::string testmarkdown2html(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "<p>markdown to html</p>";
    std::unique_ptr<markdown2html> md = std::make_unique<markdown2html>();
    md->parser(R"(
### 2.3 无序列表

无序列表的**使用**，在符号`-`后加空格使用。如下：

- 无序~~列表~~ 1
- 无序列表 2
- 无序列表 3
    )");
    client << md->get_htmlcontent();
    client << "</form>";
    return "";
}

}// namespace http