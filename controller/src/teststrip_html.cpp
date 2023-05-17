#include <chrono>
#include <thread>
#include "httppeer.h"
#include "teststrip_html.h"
#include "func.h"
namespace http
{
//@urlpath(null,teststrip_html)
std::string teststrip_html(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "<p>test strip_html</p>";
    std::string htmlcontent = R"(<th scope="col"><script lang="javascript">alert("abc")</script>th content</th>)";

    client << strip_html(htmlcontent);

    htmlcontent = R"(<!doctype html>
<html lang="en" data-bs-theme="auto">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="description" content="">
  <title>test strip_html</title>
    <link rel="stylesheet" href="/assets/icons/font/bootstrap-icons.css">
    <script type="text/javascript" charset="utf-8" src="/ueditor/lang/zh-cn/zh-cn.js"></script>
    <style type="text/css">
    div {
      width: 100%;
    }

    .edui-editor {
      width: 830px !important;
      min-height: 300px;
      height: auto;
    }

    #edui1_iframeholder {
      min-height: 200px;
    }

    body {
      background-color: #FFFFFF;
    }
  </style>
  <body>
<table class="table">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">First</th>
      <th scope="col">Last</th>
      <th scope="col">Handle</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <th scope="row">1</th>
      <td>Mark</td>
      <td>Otto</td>
      <td>@mdo</td>
    </tr>
    <tr>
      <th scope="row">2</th>
      <td>Jacob</td>
      <td>Thornton</td>
      <td>@fat</td>
    </tr>
    <tr>
      <th scope="row">3</th>
      <td colspan="2">Larry the Bird</td>
      <td>@twitter</td>
    </tr>
  </tbody>
</table>
<script lang="javascript">alert("abc")</script>
  <script src="/assets/dist/js/bootstrap.bundle.min.js"></script>
  <script lang="javascript">
    var ue = UE.getEditor('editor');
    var current_page=<% echo<<obj["pageinfo"]["current"].to_int(); %>;
    var min_page=<% echo<<obj["pageinfo"]["min"].to_int(); %>;
    var max_page=<% echo<<obj["pageinfo"]["max"].to_int(); %>;
    </script>
</body>

</html>
  )";

    client << strip_html(htmlcontent);

    return "";
}

} //namespace http