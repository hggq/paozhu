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
    md->set_tags_class("ul", "cssuldemo");
    md->set_tags_class("blockquote", "blockquote");
    md->set_tags_class("table", "table");
    md->parser(R"(
<link rel="stylesheet" media="all" href="/assets/dist/css/bootstrap.min.css" />

## 1 Markdown简介

- 可以简单编辑，无html
- 方便书写
- github平台粘贴

## 2 Markdown语法教程

### 2.1 标题

不同数量的`#`可以完成不同的标题，如下：

# 一级标题

## 二级标题

### 三级标题

### 2.2 字体

粗体、斜体、粗体和斜体，删除线，需要在文字前后加不同的标记符号。如下：

**这个是粗体**

*这个是斜体*

***这个是粗体加斜体***

~~这里想用删除线~~

注：如果想给字体换颜色、字体或者居中显示，需要使用内嵌HTML来实现。

### 2.3 无序列表

无序列表的使用，在符号`-`后加空格使用。如下：

- 无序列表 1
- 无序列表 2
- 无序列表 3

如果要控制列表的层级，则需要在符号`-`前使用空格。如下：

- 无序列表 1
- 无序列表 2
  - 无序列表 2.1
  - 无序列表 2.2

**由于微信原因，最多支持到二级列表**。

### 2.4 有序列表

有序列表的使用，在数字及符号`.`后加空格后输入内容，如下：

1. 有序列表 1
2. 有序列表 2
3. 有序列表 3

### 2.5 引用

引用的格式是在符号`>`后面书写文字。如下：

> 读一本好书，就是在和高尚的人谈话。 ——歌德

> 雇用制度对工人不利，但工人根本无力摆脱这个制度。 ——阮一峰

### 2.9 分割线

可以在一行中用三个以上的减号来建立一个分隔线，同时需要在分隔线的上面空一行。如下：

---

### 2.10 表格

可以使用冒号来定义表格的对齐方式，如下：

| 左对齐   | 文本居中 |     右对齐 |
| :----- | :--: | -------: |
| 左对齐 |  文本  | 右对齐 |
| 内容左对齐 |  文本  | 内容右对齐 |
| 所有内容左对齐 |  文本  | 所有内容右对齐 |


<style type="text/css">

table {
border-collapse:collapse;
border-right:#ccc 1px solid;
border-bottom:#ccc 1px solid;
border-color: #ccc;
}
td {
border-left:#ccc 1px solid;
border-top:#ccc 1px solid;
}
th {
border-left:#ccc 1px solid;
border-top:#ccc 1px solid;
}

blockquote {
	padding: .5rem 1rem;
	margin: .8rem 0;
	color: #7a7a7a;
	border-left: .25rem solid #e5e5e5;
}
</style> 

<div align="center">
<br />
<hr>
<https://github.com/hggq/paozhu>

<aa.~aa@aaa.com.cn>

<p>
<img src="https://hggq.github.io/paozhu/images/logo.svg">
</p>
<br>
</div>

    )");
    client << md->get_htmlcontent();
    client << "</form>";
    return "";
}

}// namespace http