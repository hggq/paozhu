
#ifndef __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP
#define __HTTP_AUTO_REG_CONTROL_HTTPMETHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "httppeer.h" 

#include "admin/articles.h"
#include "admin/attachfile.h"
#include "admin/category.h"
#include "admin/main.h"
#include "admin/marbrand.h"
#include "admin/marhome.h"
#include "admin/marproduct.h"
#include "admin/menu.h"
#include "admin/topics.h"
#include "apicrudtest.h"
#include "devcors.h"
#include "imageapi.h"
#include "saas.com/test_co_handle.h"
#include "serverwatch.h"
#include "superadmin/supermain.h"
#include "techempower.h"
#include "test_acme.h"
#include "test_alipay.h"
#include "test_chunked.h"
#include "test_co_handle.h"
#include "test_cols.h"
#include "test_customstruct.h"
#include "test_deepseekapi.h"
#include "test_dir_name.h"
#include "test_escape_str.h"
#include "test_httppeersend.h"
#include "test_leftjoin.h"
#include "test_mqtt_client.h"
#include "test_ormfk.h"
#include "test_ormprepared.h"
#include "test_parse_ini.h"
#include "test_pg_crud.h"
#include "test_pzexcel.h"
#include "test_pzjpg.h"
#include "test_pzpng.h"
#include "test_pzword.h"
#include "test_pzzip.h"
#include "test_redis_bench.h"
#include "test_redis_client.h"
#include "test_redis_types.h"
#include "test_reverse.h"
#include "test_rpc_handle.h"
#include "test_socket_handle.h"
#include "test_sql_commit.h"
#include "test_sql_json.h"
#include "test_sql_query.h"
#include "test_sqlite_crud.h"
#include "test_svgchart.h"
#include "test_webpdf.h"
#include "test_websocket_handle.h"
#include "test_weixin.h"
#include "test_wxpay.h"
#include "test_wxpayv3.h"
#include "testaddclienttask.h"
#include "testcmake.h"
#include "testcommit.h"
#include "testcors.h"
#include "testcowaitclient.h"
#include "testcrud.h"
#include "testdepthprobe.h"
#include "testdownloadauth.h"
#include "testfield_num.h"
#include "testflowstack.h"
#include "testformpost.h"
#include "testhello.h"
#include "testhttpclient.h"
#include "testipsearch.h"
#include "testjson.h"
#include "testjsonreflect.h"
#include "testmarkdown.h"
#include "testmodelfromjson.h"
#include "testmoney_num.h"
#include "testmysqlinsert.h"
#include "testormcache.h"
#include "testormclient.h"
#include "testpinyin.h"
#include "testpzcache.h"
#include "testqrcode.h"
#include "testrand.h"
#include "testrestfulpath.h"
#include "testsendmail.h"
#include "testsessionid.h"
#include "testsiteid.h"
#include "testsitepath.h"
#include "testsoft_remove.h"
#include "testsqltuple.h"
#include "teststr2int.h"
#include "teststr_join.h"
#include "teststr_trim.h"
#include "teststrip_html.h"
#include "testto_tree.h"

namespace http
{

    void _initauto_all_httputils()
    {
        reg_raw("", "/admin/addarticle", admin_islogin, nullptr, admin_addarticle, nullptr);
        reg_raw("", "/admin/addarticlepost", admin_islogin, nullptr, admin_addarticlepost, nullptr);
        reg_raw("", "/admin/editarticle", admin_islogin, nullptr, admin_editarticle, nullptr);
        reg_raw("", "/admin/editarticlepost", admin_islogin, nullptr, admin_editarticlepost, nullptr);
        reg_raw("", "/admin/deletearticle", admin_isloginjson, nullptr, admin_deletearticle, nullptr);
        reg_raw("", "/admin/gettoparticle", admin_isloginjson, nullptr, admin_gettoparticle, nullptr);
        reg_raw("", "/admin/updatearticlesort", admin_isloginjson, nullptr, admin_updatearticlesort, nullptr);
        reg_raw("", "/admin/updatearticleview", admin_isloginjson, nullptr, admin_updatearticleview, nullptr);
        reg_raw("", "/admin/updatearticleishome", admin_isloginjson, nullptr, admin_updatearticleishome, nullptr);
        reg_raw("", "/admin/listarticle", admin_islogin, nullptr, admin_listarticle, nullptr);
        reg_raw("", "/attachfile/gateway", admin_islogin, nullptr, attachfile_gateway, nullptr);
        reg_raw("", "/attachimg/gateway", admin_islogin, nullptr, attachimg_gateway, nullptr);
        reg_raw("", "/attachfile/delete", admin_islogin, nullptr, attachfile_delete, nullptr);
        reg_raw("", "/attachimg/delete", admin_islogin, nullptr, attachimg_delete, nullptr);
        reg_raw("", "/attachfile/upload", admin_islogin, nullptr, attachfile_upload, nullptr);
        reg_raw("", "/attachfile/uploadpost", admin_islogin, nullptr, attachfile_uploadpost, nullptr);
        reg_raw("", "/admin/attachselectfiles", admin_islogin, nullptr, admin_attachselectfiles, nullptr);
        reg_raw("", "/admin/marcatalogue", admin_islogin, nullptr, admin_marcatalogue, nullptr);
        reg_raw("", "/admin/addcataloguepost", admin_islogin, nullptr, admin_addcataloguepost, nullptr);
        reg_raw("", "/admin/editcataloguepost", admin_islogin, nullptr, admin_editcataloguepost, nullptr);
        reg_raw("", "/admin/deletecatalogue", admin_islogin, nullptr, admin_deletecatalogue, nullptr);
        reg_raw("", "/admin/login", nullptr, nullptr, admin_login, nullptr);
        reg_raw("", "/admin/loginpost", nullptr, nullptr, admin_loginpost, nullptr);
        reg_raw("", "/admin/logout", nullptr, nullptr, admin_logout, nullptr);
        reg_raw("", "/admin/islogin", nullptr, nullptr, admin_islogin, nullptr);
        reg_raw("", "/admin/isloginjson", nullptr, nullptr, admin_isloginjson, nullptr);
        reg_raw("", "/admin/main", admin_islogin, nullptr, admin_main, nullptr);
        reg_raw("", "/admin/welcome", admin_islogin, nullptr, admin_welcome, nullptr);
        reg_raw("", "/admin/siteinfo", admin_islogin, nullptr, admin_siteinfo, nullptr);
        reg_raw("", "/admin/siteinfopost", admin_islogin, nullptr, admin_siteinfopost, nullptr);
        reg_raw("", "/admin/footscript", admin_islogin, nullptr, admin_footscript, nullptr);
        reg_raw("", "/admin/footscriptpost", admin_islogin, nullptr, admin_footscriptpost, nullptr);
        reg_raw("", "/admin/copyright", admin_islogin, nullptr, admin_copyright, nullptr);
        reg_raw("", "/admin/copyrightpost", admin_islogin, nullptr, admin_copyrightpost, nullptr);
        reg_raw("", "/admin/sitelogo", admin_islogin, nullptr, admin_sitelogo, nullptr);
        reg_raw("", "/admin/sitelogopost", admin_islogin, nullptr, admin_sitelogopost, nullptr);
        reg_raw("", "/admin/sitebannerpost", admin_islogin, nullptr, admin_sitebannerpost, nullptr);
        reg_raw("", "/admin/userinfo", admin_islogin, nullptr, admin_userinfo, nullptr);
        reg_raw("", "/admin/editpassword", admin_islogin, nullptr, admin_editpassword, nullptr);
        reg_raw("", "/admin/editpwdpost", admin_islogin, nullptr, admin_editpwdpost, nullptr);
        reg_raw("", "/admin/favicon", admin_islogin, nullptr, admin_favicon, nullptr);
        reg_raw("", "/admin/faviconpost", admin_islogin, nullptr, admin_faviconpost, nullptr);
        reg_raw("", "/admin/marbrand", admin_islogin, nullptr, admin_marbrand, nullptr);
        reg_raw("", "/admin/brandaddpost", admin_islogin, nullptr, admin_brandaddpost, nullptr);
        reg_raw("", "/admin/deletebrand", admin_islogin, nullptr, admin_deletebrand, nullptr);
        reg_raw("", "/admin/updatebrandsort", admin_isloginjson, nullptr, admin_updatebrandsort, nullptr);
        reg_raw("", "/admin/marhome", admin_islogin, nullptr, admin_marhome, nullptr);
        reg_raw("", "/admin/edithomeblockpic", admin_islogin, nullptr, admin_edithomeblockpic, nullptr);
        reg_raw("", "/admin/updatehomeblocksort", admin_islogin, nullptr, admin_updatehomeblocksort, nullptr);
        reg_raw("", "/admin/addhomepic", admin_islogin, nullptr, admin_addhomepic, nullptr);
        reg_raw("", "/admin/addhometext", admin_islogin, nullptr, admin_addhometext, nullptr);
        reg_raw("", "/admin/addhometopic", admin_islogin, nullptr, admin_addhometopic, nullptr);
        reg_raw("", "/admin/addhomecontent", admin_islogin, nullptr, admin_addhomecontent, nullptr);
        reg_raw("", "/admin/edithomeblockcontent", admin_islogin, nullptr, admin_edithomeblockcontent, nullptr);
        reg_raw("", "/admin/edithomeblocktopic", admin_islogin, nullptr, admin_edithomeblocktopic, nullptr);
        reg_raw("", "/admin/edithomeblockpost", admin_islogin, nullptr, admin_edithomeblockpost, nullptr);
        reg_raw("", "/admin/addhomeblockpost", admin_islogin, nullptr, admin_addhomeblockpost, nullptr);
        reg_raw("", "/admin/addhomehot", admin_islogin, nullptr, admin_addhomehot, nullptr);
        reg_raw("", "/admin/edithomeblockhot", admin_islogin, nullptr, admin_edithomeblockhot, nullptr);
        reg_raw("", "/admin/edithomeblocktext", admin_islogin, nullptr, admin_edithomeblocktext, nullptr);
        reg_raw("", "/admin/deletehomeblock", admin_islogin, nullptr, admin_deletehomeblock, nullptr);
        reg_raw("", "/admin/homedesign", admin_islogin, nullptr, admin_homedesign, nullptr);
        reg_raw("", "/admin/edithomeblockmulit", admin_islogin, nullptr, admin_edithomeblockmulit, nullptr);
        reg_raw("", "/admin/addproduct", admin_islogin, nullptr, admin_addproduct, nullptr);
        reg_raw("", "/admin/addproductpost", admin_islogin, nullptr, admin_addproductpost, nullptr);
        reg_raw("", "/admin/editproductpost", admin_islogin, nullptr, admin_editproductpost, nullptr);
        reg_raw("", "/admin/getcategorytopproduct", admin_islogin, nullptr, admin_getcategorytopproduct, nullptr);
        reg_raw("", "/admin/editproduct", admin_islogin, nullptr, admin_editproduct, nullptr);
        reg_raw("", "/admin/listproduct", admin_islogin, nullptr, admin_listproduct, nullptr);
        reg_raw("", "/admin/marproductattach", admin_islogin, nullptr, admin_marproductattach, nullptr);
        reg_raw("", "/admin/deleteproduct", admin_islogin, nullptr, admin_deleteproduct, nullptr);
        reg_raw("", "/admin/deleteproductajax", admin_islogin, nullptr, admin_deleteproductajax, nullptr);
        reg_raw("", "/admin/updateproducthome", admin_islogin, nullptr, admin_updateproducthome, nullptr);
        reg_raw("", "/admin/updateproductstore", admin_islogin, nullptr, admin_updateproductstore, nullptr);
        reg_raw("", "/admin/updateproductview", admin_islogin, nullptr, admin_updateproductview, nullptr);
        reg_raw("", "/admin/updateproductsort", admin_islogin, nullptr, admin_updateproductsort, nullptr);
        reg_raw("", "/admin/menu", nullptr, nullptr, admin_menu, nullptr);
        reg_raw("", "/admin/vuetest", nullptr, nullptr, admin_vuetest, nullptr);
        reg_raw("", "/admin/dashboard", nullptr, nullptr, admin_dashboard, nullptr);
        reg_raw("", "/admin/jstimetest", nullptr, nullptr, admin_jstimetest, nullptr);
        reg_raw("", "/admin/testelementplus", nullptr, nullptr, admin_testelementplus, nullptr);
        reg_raw("", "/admin/testgetjson", nullptr, nullptr, admin_testgetjson, nullptr);
        reg_raw("", "/admin/addtopic", admin_islogin, nullptr, admin_addtopic, nullptr);
        reg_raw("", "/admin/edittopic", admin_islogin, nullptr, admin_edittopic, nullptr);
        reg_raw("", "/admin/martopic", admin_islogin, nullptr, admin_martopic, nullptr);
        reg_raw("", "/admin/updatetopicsort", admin_isloginjson, nullptr, admin_updatetopicsort, nullptr);
        reg_raw("", "/admin/addtopicpost", admin_isloginjson, nullptr, admin_addtopicpost, nullptr);
        reg_raw("", "/admin/deletetopic", admin_isloginjson, nullptr, admin_deletetopic, nullptr);
        reg_raw("", "/admin/edittopicpost", admin_isloginjson, nullptr, admin_edittopicpost, nullptr);
        reg_raw("", "/admin/topicfileupload", admin_isloginjson, nullptr, admin_topicfileupload, nullptr);
        reg_raw("", "/admin/topicimgtextupload", admin_isloginjson, nullptr, admin_topicimgtextupload, nullptr);
        reg_raw("", "/admin/updatetopicview", admin_isloginjson, nullptr, admin_updatetopicview, nullptr);
        reg_raw("", "/admin/updatetopicsideblocksort", admin_isloginjson, nullptr, admin_updatetopicsideblocksort, nullptr);
        reg_raw("", "/admin/edittopicside", admin_isloginjson, nullptr, admin_edittopicside, nullptr);
        reg_raw("", "/admin/addtopicsidepick", admin_isloginjson, nullptr, admin_addtopicsidepick, nullptr);
        reg_raw("", "/admin/addtopicsidetext", admin_isloginjson, nullptr, admin_addtopicsidetext, nullptr);
        reg_raw("", "/admin/addtopicsideblockpost", admin_isloginjson, nullptr, admin_addtopicsideblockpost, nullptr);
        reg_raw("", "/admin/deletetopicsideblock", admin_isloginjson, nullptr, admin_deletetopicsideblock, nullptr);
        reg_raw("", "/admin/edittopicsideblocktext", admin_isloginjson, nullptr, admin_edittopicsideblocktext, nullptr);
        reg_raw("", "/admin/edittopicsideblockpick", admin_isloginjson, nullptr, admin_edittopicsideblockpick, nullptr);
        reg_raw("", "/admin/edittopicsideblockpost", admin_isloginjson, nullptr, admin_edittopicsideblockpost, nullptr);
        reg_raw("", "/api/departments/addpost", nullptr, nullptr, pxapidepartmentsaddpost, nullptr);
        reg_raw("", "/api/departments/editpost", nullptr, nullptr, pxapidepartmentseditpost, nullptr);
        reg_raw("", "/api/departments/list", nullptr, nullptr, pxapidepartmentslist, nullptr);
        reg_raw("", "/api/departments/deletedep", nullptr, nullptr, pxapipagesdepartlist, nullptr);
        reg_raw("", "/api/dev/hostcors", nullptr, nullptr, api_dev_hostcors, nullptr);
        reg_raw("", "/imageapi/gateway", nullptr, nullptr, imageapi_gateway, nullptr);
        reg_raw("", "/imageapi/upload", nullptr, nullptr, imageapi_upload, nullptr);
        reg_raw("", "/paozhu_status", nullptr, nullptr, paozhu_status, nullptr);
        reg_raw("", "/paozhu_routes", nullptr, nullptr, paozhu_routes, nullptr);
        reg_raw("", "/frametasks_timeloop", nullptr, nullptr, frametasks_timeloop, nullptr);
        reg_raw("", "/superadmin/login", nullptr, nullptr, superadmin_login, nullptr);
        reg_raw("", "/superadmin/loginpost", nullptr, nullptr, superadmin_loginpost, nullptr);
        reg_raw("", "/superadmin/logout", nullptr, nullptr, superadmin_logout, nullptr);
        reg_raw("", "/superadmin/islogin", nullptr, nullptr, superadmin_islogin, nullptr);
        reg_raw("", "/superadmin/isloginjson", nullptr, nullptr, superadmin_isloginjson, nullptr);
        reg_raw("", "/superadmin/main", superadmin_islogin, nullptr, superadmin_main, nullptr);
        reg_raw("", "/superadmin/editsiteinfo", superadmin_islogin, nullptr, superadmin_editsiteinfo, nullptr);
        reg_raw("", "/superadmin/editsiteinfopost", superadmin_islogin, nullptr, superadmin_editsiteinfopost, nullptr);
        reg_raw("", "/superadmin/deletesiteinfo", superadmin_islogin, nullptr, superadmin_deletesiteinfo, nullptr);
        reg_raw("", "/superadmin/deletesiteinfopost", superadmin_islogin, nullptr, superadmin_deletesiteinfopost, nullptr);
        reg_raw("", "/superadmin/addsiteinfo", superadmin_islogin, nullptr, superadmin_addsiteinfo, nullptr);
        reg_raw("", "/superadmin/addsiteinfopost", superadmin_islogin, nullptr, superadmin_addsiteinfopost, nullptr);
        reg_raw("", "/superadmin/welcome", superadmin_islogin, nullptr, superadmin_welcome, nullptr);
        reg_raw("", "/superadmin/listuser", superadmin_islogin, nullptr, superadmin_listuser, nullptr);
        reg_raw("", "/superadmin/edituser", superadmin_islogin, nullptr, superadmin_edituser, nullptr);
        reg_raw("", "/superadmin/adduser", superadmin_islogin, nullptr, superadmin_adduser, nullptr);
        reg_raw("", "/superadmin/adduserpost", superadmin_islogin, nullptr, superadmin_adduserpost, nullptr);
        reg_raw("", "/superadmin/edituserpost", superadmin_islogin, nullptr, superadmin_edituserpost, nullptr);
        reg_raw("", "/superadmin/deleteuser", superadmin_islogin, nullptr, superadmin_deleteuser, nullptr);
        reg_raw("", "/superadmin/userinfo", superadmin_islogin, nullptr, superadmin_userinfo, nullptr);
        reg_raw("", "/superadmin/editpassword", superadmin_islogin, nullptr, superadmin_editpassword, nullptr);
        reg_raw("", "/superadmin/editpwdpost", superadmin_isloginjson, nullptr, superadmin_editpwdpost, nullptr);
        reg_raw("", "/test_acme", nullptr, nullptr, test_acme, nullptr);
        reg_raw("", "/testalipaydiag", nullptr, nullptr, test_alipay_diag, nullptr);
        reg_raw("", "/testalipayqrcode", nullptr, nullptr, test_alipay_qrcode, nullptr);
        reg_raw("", "/testalipayquery", nullptr, nullptr, test_alipay_query, nullptr);
        reg_raw("", "/testalipaycancel", nullptr, nullptr, test_alipay_cancel, nullptr);
        reg_raw("", "/testalipayrefund", nullptr, nullptr, test_alipay_refund, nullptr);
        reg_raw("", "/test_chunked", nullptr, nullptr, test_chunked, nullptr);
        reg_raw("", "/test_chunkedfile", nullptr, nullptr, test_chunkedfile, nullptr);
        reg_raw("", "/test_cols", nullptr, nullptr, test_cols, nullptr);
        reg_raw("", "/deepseek_api", nullptr, nullptr, test_deepseek_api, nullptr);
        reg_raw("", "/test_dir_name", nullptr, nullptr, test_dir_name, nullptr);
        reg_raw("", "/test_escapestr", nullptr, nullptr, test_escapestr, nullptr);
        reg_raw("", "/test_httppeersend", nullptr, nullptr, test_httppeersend, nullptr);
        reg_raw("", "/test_httppeersend_text", nullptr, nullptr, test_httppeersend_text, nullptr);
        reg_raw("", "/test_ormfk_one", nullptr, nullptr, test_ormfk_one, nullptr);
        reg_raw("", "/test_ormfk_many", nullptr, nullptr, test_ormfk_many, nullptr);
        reg_raw("", "/test_ormfk_join", nullptr, nullptr, test_ormfk_join, nullptr);
        reg_raw("", "/test_ormprepared_dsl", nullptr, nullptr, test_ormprepared_dsl, nullptr);
        reg_raw("", "/test_ormprepared_exec", nullptr, nullptr, test_ormprepared_exec, nullptr);
        reg_raw("", "/test_ormprepared_write", nullptr, nullptr, test_ormprepared_write, nullptr);
        reg_raw("", "/test_parse_ini", nullptr, nullptr, test_parse_ini, nullptr);
        reg_raw("", "/test_parse_fix", nullptr, nullptr, test_parse_fix, nullptr);
        reg_raw("", "/test_fix_server_conf", nullptr, nullptr, test_fix_server_conf, nullptr);
        reg_raw("", "/test_fix_orm_conf", nullptr, nullptr, test_fix_orm_conf, nullptr);
        reg_raw("", "/cms/pglist", nullptr, nullptr, article_pg_list, nullptr);
        reg_raw("", "/cms/pgshow", nullptr, nullptr, article_pg_show, nullptr);
        reg_raw("", "/test_pzexcel", nullptr, nullptr, test_pzexcel, nullptr);
        reg_raw("", "/test_pzjpg", nullptr, nullptr, test_pzjpg, nullptr);
        reg_raw("", "/test_outjpg", nullptr, nullptr, test_outjpg, nullptr);
        reg_raw("", "/test_showjpg", nullptr, nullptr, test_showjpg, nullptr);
        reg_raw("", "/test_pzpng", nullptr, nullptr, test_pzpng, nullptr);
        reg_raw("", "/test_outpng", nullptr, nullptr, test_outpng, nullptr);
        reg_raw("", "/test_captcha", nullptr, nullptr, test_captcha, nullptr);
        reg_raw("", "/test_pzword", nullptr, nullptr, test_pzword, nullptr);
        reg_raw("", "/test_pzzip", nullptr, nullptr, test_pzzip, nullptr);
        reg_raw("", "/redis/bench/sync", nullptr, nullptr, test_redis_bench_sync, nullptr);
        reg_raw("", "/redis/bench/direct", nullptr, nullptr, test_redis_bench_direct, nullptr);
        reg_raw("", "/redis/parse", nullptr, nullptr, test_redis_parse, nullptr);
        reg_raw("", "/redis/sync", nullptr, nullptr, test_redis_sync, nullptr);
        reg_raw("", "/redis/types/sync", nullptr, nullptr, test_redis_types_sync, nullptr);
        reg_raw("", "/testmb_reverse", nullptr, nullptr, testmb_reverse, nullptr);
        reg_raw("", "/test_sql_commit", nullptr, nullptr, test_sql_commit, nullptr);
        reg_raw("", "/testsqljson", nullptr, nullptr, testsqljson, nullptr);
        reg_raw("", "/sqlquery", nullptr, nullptr, testsqlquery, nullptr);
        reg_raw("", "/cms/sqlist", nullptr, nullptr, article_sqlite_list, nullptr);
        reg_raw("", "/cms/sqshow", nullptr, nullptr, article_sqlite_show, nullptr);
        reg_raw("", "/test_svgchart", nullptr, nullptr, test_svgchart, nullptr);
        reg_raw("", "/test_svgstats", nullptr, nullptr, test_svgstats, nullptr);
        reg_raw("", "/test_webpdf", nullptr, nullptr, test_webpdf, nullptr);
        reg_raw("", "/test_ttfpdf", nullptr, nullptr, test_otfpdf, nullptr);
        reg_raw("", "/test_table_linebreak", nullptr, nullptr, test_table_linebreak, nullptr);
        reg_raw("", "/testgetopenid", nullptr, nullptr, test_getopenid, nullptr);
        reg_raw("", "/testweixinpay", nullptr, nullptr, test_weixinpay, nullptr);
        reg_raw("", "/testweixinnative", nullptr, nullptr, test_weixin_native, nullptr);
        reg_raw("", "/xcxgetphone", nullptr, nullptr, test_xcxgetphone, nullptr);
        reg_raw("", "/testweixin_order_new", nullptr, nullptr, test_weixin_order_new, nullptr);
        reg_raw("", "/testweixin_order_list", nullptr, nullptr, test_weixin_order_list, nullptr);
        reg_raw("", "/testweixin_order_detail", nullptr, nullptr, test_weixin_order_detail, nullptr);
        reg_raw("", "/testweixin_refund", nullptr, nullptr, test_weixin_refund, nullptr);
        reg_raw("", "/testweixin_refund_query", nullptr, nullptr, test_weixin_refund_query, nullptr);
        reg_raw("", "/testwxpaynative", nullptr, nullptr, test_wxpay_native, nullptr);
        reg_raw("", "/testwxpayjsapi", nullptr, nullptr, test_wxpay_jsapi, nullptr);
        reg_raw("", "/testwxpayapp", nullptr, nullptr, test_wxpay_app, nullptr);
        reg_raw("", "/testwxpayh5", nullptr, nullptr, test_wxpay_h5, nullptr);
        reg_raw("", "/testwxpayquery", nullptr, nullptr, test_wxpay_query, nullptr);
        reg_raw("", "/testwxpayclose", nullptr, nullptr, test_wxpay_close, nullptr);
        reg_raw("", "/wxpaydownloadcert", nullptr, nullptr, test_wxpay_download_cert, nullptr);
        reg_raw("", "/testwxpayrefund", nullptr, nullptr, test_wxpay_refund, nullptr);
        reg_raw("", "/testwxpayv2_order_new", nullptr, nullptr, testwxpayv2_order_new, nullptr);
        reg_raw("", "/testwxpayv2_order_list", nullptr, nullptr, testwxpayv2_order_list, nullptr);
        reg_raw("", "/testwxpayv2_order_detail", nullptr, nullptr, testwxpayv2_order_detail, nullptr);
        reg_raw("", "/testwxpayv2_refund", nullptr, nullptr, testwxpayv2_refund, nullptr);
        reg_raw("", "/testwxpayv2_refund_query", nullptr, nullptr, testwxpayv2_refund_query, nullptr);
        reg_raw("", "/testwxpaycertislogin", nullptr, nullptr, testwxpaycert_islogin, nullptr);
        reg_raw("", "/testwxpay", nullptr, nullptr, testwxpay, nullptr);
        reg_raw("", "/testwxpayv3jsapi", nullptr, nullptr, testwxpayv3jsapi, nullptr);
        reg_raw("", "/testwxpaydownloadcert", testwxpaycert_islogin, nullptr, testwxpaydownloadcert, nullptr);
        reg_raw("", "/testwxpayv3_order_new", nullptr, nullptr, testwxpayv3_order_new, nullptr);
        reg_raw("", "/testwxpayv3_order_list", nullptr, nullptr, testwxpayv3_order_list, nullptr);
        reg_raw("", "/testwxpayv3_order_detail", nullptr, nullptr, testwxpayv3_order_detail, nullptr);
        reg_raw("", "/testwxpayv3_refund", nullptr, nullptr, testwxpayv3_refund, nullptr);
        reg_raw("", "/testwxpayv3_refund_query", nullptr, nullptr, testwxpayv3_refund_query, nullptr);
        reg_raw("", "/testnotaddclienttaskpre", nullptr, nullptr, testaddclienttaskpre, nullptr);
        reg_raw("", "/testaddclienttask", testaddclienttaskpre, nullptr, testaddclienttask, nullptr);
        reg_raw("", "/executeclienttask", nullptr, nullptr, testexecuteclienttask, nullptr);
        reg_raw("", "/ccmake", nullptr, nullptr, testcmake, nullptr);
        reg_raw("", "/ccauto", nullptr, nullptr, testcauto, nullptr);
        reg_raw("", "/testcommit", nullptr, nullptr, testcommit, nullptr);
        reg_raw("", "/api/user/message", nullptr, nullptr, testcors, nullptr);
        reg_raw("", "/api/user/info", nullptr, nullptr, testcorssimple, nullptr);
        reg_raw("", "/api/user/vary", nullptr, nullptr, testcorsvary, nullptr);
        reg_raw("", "/testcowaitclient4", nullptr, nullptr, testhttpclient_cowait_php, nullptr);
        reg_raw("", "/testcowaitclient1", nullptr, nullptr, testhttpclient_cowait_body, nullptr);
        reg_raw("", "/testcowaitclient5", nullptr, nullptr, testhttpclient_cowait_post, nullptr);
        reg_raw("", "/testcowaitclient2", nullptr, nullptr, testhttpclient_cowait_urls, nullptr);
        reg_raw("", "/testcowaitclient3", nullptr, nullptr, testhttpclient_cowait_spawn, nullptr);
        reg_raw("", "/testclientgetrange", nullptr, nullptr, testhttpclient_get_range, nullptr);
        reg_raw("", "/downfilelist", nullptr, nullptr, testhttpclient_downfilelist, nullptr);
        reg_raw("", "/downfilecontent", nullptr, nullptr, testhttpclient_getdownfile, nullptr);
        reg_raw("", "/cms/login", nullptr, nullptr, articlelogin, nullptr);
        reg_raw("", "/cms/islogin", nullptr, nullptr, articleislogin, nullptr);
        reg_raw("", "/cms/loginpost", nullptr, nullptr, articleloginpost, nullptr);
        reg_raw("", "/cms/list", nullptr, nullptr, articlelist, nullptr);
        reg_raw("", "/cms/show", nullptr, nullptr, articleshow, nullptr);
        reg_raw("", "/cms/edit", articleislogin, nullptr, articleedit, nullptr);
        reg_raw("", "/cms/editpost", articleislogin, nullptr, articleeditpost, nullptr);
        reg_raw("", "/cms/add", articleislogin, nullptr, articleadd, nullptr);
        reg_raw("", "/cms/addpost", articleislogin, nullptr, articleaddpost, nullptr);
        reg_raw("", "/cms/delete", articleislogin, nullptr, articledelete, nullptr);
        reg_raw("", "/cms/prepost", articleislogin, nullptr, articleprepost, nullptr);
        reg_raw("", "/cms/preselect", articleislogin, nullptr, articlepreselect, nullptr);
        reg_raw("", "/dpshadow", nullptr, nullptr, testdpshadowshort, nullptr);
        reg_raw("", "/dpshadow/sub", nullptr, nullptr, testdpshadowexact, nullptr);
        reg_raw("", "/dp6/a/b/c/d/e", nullptr, nullptr, testdpprobedepth6, nullptr);
        reg_urlpath("", "/dp6/a/b/c/d/e", {"", "", "", "", "", "", "tail"});
        reg_raw("", "/dp7/a/b/c/d/e/f", nullptr, nullptr, testdpprobedepth7, nullptr);
        reg_urlpath("", "/dp7/a/b/c/d/e/f", {"", "", "", "", "", "", "", "tail"});
        reg_raw("", "/testuser/info", nullptr, nullptr, testdpuserinfo2, nullptr);
        reg_urlpath("", "/testuser/info", {"", "", "userid", "groupid"});
        reg_raw("", "/downloadfileauth", nullptr, nullptr, downloadfileauthmethod, nullptr);
        reg_raw("", "/testfieldnum", nullptr, nullptr, testfieldnum, nullptr);
        reg_raw("", "/fx/preonly", nullptr, nullptr, testfxpreonly, nullptr);
        reg_raw("", "/fx/guarded", testfxpreonly, nullptr, testfxguarded, nullptr);
        reg_raw("", "/fx/a", nullptr, nullptr, testfxa, nullptr);
        reg_raw("", "/fx/b", nullptr, nullptr, testfxb, nullptr);
        reg_raw("", "/fx/c", nullptr, nullptr, testfxc, nullptr);
        reg_raw("", "/fx/d", nullptr, nullptr, testfxd, nullptr);
        reg_raw("", "/fx/plain", nullptr, nullptr, testfxplain, nullptr);
        reg_raw("", "/fx/loop", nullptr, nullptr, testfxloop, nullptr);
        reg_raw("", "/fx/gone", nullptr, nullptr, testfxgone, nullptr);
        reg_raw("", "/fx/exit", nullptr, nullptr, testfxexit, nullptr);
        reg_raw("", "/fx/depa", nullptr, nullptr, testfxdepa, nullptr);
        reg_raw("", "/fx/depb", nullptr, nullptr, testfxdepb, nullptr);
        reg_raw("", "/fx/sync_push_coro", nullptr, nullptr, testfxsyncpushcoro, nullptr);
        reg_raw("", "/fx/coro_pre_sync", nullptr, testfxcoropre, testfxcoropresync, nullptr);
        reg_raw("", "/fx/mix_chain", nullptr, nullptr, testfxmixchain_sync, nullptr);
        reg_raw("", "/fx/mix_tail", nullptr, nullptr, testfxmixchain_tail, nullptr);
        reg_raw("", "/fx/exc/sync_reg", nullptr, nullptr, testfxexc_sync_reg, nullptr);
        reg_raw("", "/fx/exc/sync_pre", nullptr, nullptr, testfxexc_sync_pre, nullptr);
        reg_raw("", "/fx/exc/sync_pre_reg", testfxexc_sync_pre, nullptr, testfxexc_sync_pre_reg, nullptr);
        reg_raw("", "/fx/exc/coro_pre_reg", nullptr, testfxexc_coro_pre, testfxexc_coro_pre_reg, nullptr);
        reg_raw("", "/fx/exc/reject503", nullptr, testfxexc_reject503_pre, testfxexc_reject503, nullptr);
        reg_raw("", "/fx/exc/after503", nullptr, nullptr, testfxexc_after503, nullptr);
        reg_raw("", "/fx/lane/b", nullptr, nullptr, testfxlaneb, nullptr);
        reg_raw("", "/fx/lane/read", nullptr, nullptr, testfxlaneread, nullptr);
        reg_raw("", "/fx/loop/slow", nullptr, nullptr, testfxloopslow, nullptr);
        reg_raw("", "/fxlooptick", nullptr, nullptr, testfxlooptick, nullptr);
        reg_raw("", "/tformpost", nullptr, nullptr, testurlencoded, nullptr);
        reg_raw("", "/tfilepost", nullptr, nullptr, testformmultipart, nullptr);
        reg_raw("", "/tjsonpost", nullptr, nullptr, testformjsonpost, nullptr);
        reg_raw("", "/txmlupload", nullptr, nullptr, testformxmlpost, nullptr);
        reg_raw("", "/addpostfile", nullptr, nullptr, testuploadpostfile, nullptr);
        reg_raw("", "/hello", nullptr, nullptr, testhello, nullptr);
        reg_raw("", "/hellobusy", nullptr, nullptr, testhellobusy, nullptr);
        reg_raw("", "/testhttpclient1", nullptr, nullptr, testhttpclient_get_body, nullptr);
        reg_raw("", "/testhttpclient3", nullptr, nullptr, testhttpclient_get_timebody, nullptr);
        reg_raw("", "/testhttpclient2", nullptr, nullptr, testhttpclient_get_file, nullptr);
        reg_raw("", "/testipsearch", nullptr, nullptr, testipsearch, nullptr);
        reg_raw("", "/ipsearchget", nullptr, nullptr, testipsearchget, nullptr);
        reg_raw("", "/testjson", nullptr, nullptr, testjson, nullptr);
        reg_raw("", "/test_requst", nullptr, nullptr, test_requst, nullptr);
        reg_raw("", "/testjsonreflect", nullptr, nullptr, testjsonreflect, nullptr);
        reg_raw("", "/testmarkdown2html", nullptr, nullptr, testmarkdown2html, nullptr);
        reg_raw("", "/mfromjson", nullptr, nullptr, testmodelfromjson, nullptr);
        reg_raw("", "/testtestmoneynum", nullptr, nullptr, testtestmoneynum, nullptr);
        reg_raw("", "/minsert", nullptr, nullptr, testmysqlinsert, nullptr);
        reg_raw("", "/mpagebar", nullptr, nullptr, testmysqlpagebar, nullptr);
        reg_raw("", "/testormcache", nullptr, nullptr, testormcache, nullptr);
        reg_raw("", "/testormcacheb", nullptr, nullptr, testormcacheb, nullptr);
        reg_raw("", "/testormcachec", nullptr, nullptr, testormcachec, nullptr);
        reg_raw("", "/testormcache_d", nullptr, nullptr, testormcache_d, nullptr);
        reg_urlpath("", "/testormcache_d", {"", "aid"});
        reg_raw("", "/testormcache_d_invalidate", nullptr, nullptr, testormcache_d_invalidate, nullptr);
        reg_urlpath("", "/testormcache_d_invalidate", {"", "aid"});
        reg_raw("", "/testormcache_e", nullptr, nullptr, testormcache_e, nullptr);
        reg_urlpath("", "/testormcache_e", {"", "aid"});
        reg_raw("", "/testormclient", nullptr, nullptr, testormclient, nullptr);
        reg_raw("", "/testpinyin", nullptr, nullptr, testpinyin, nullptr);
        reg_raw("", "/testpinyin_loaded", nullptr, nullptr, testpinyin_loaded, nullptr);
        reg_raw("", "/testcache", nullptr, nullptr, testpzcache, nullptr);
        reg_raw("", "/testshowcache", nullptr, nullptr, testshowcache, nullptr);
        reg_raw("", "/testqrcode", nullptr, nullptr, testqrcode, nullptr);
        reg_raw("", "/testrand", nullptr, nullptr, testrand, nullptr);
        reg_raw("", "/user/info", nullptr, nullptr, testrestfulpath, nullptr);
        reg_urlpath("", "/user/info", {"", "", "userid"});
        reg_raw("", "/user/profile", nullptr, nullptr, testrestfulprofilepath, nullptr);
        reg_urlpath("", "/user/profile", {"", "", "userid", "pathid"});
        reg_raw("", "/testsendmaildo", nullptr, nullptr, testsendmaildo, nullptr);
        reg_raw("", "/testsetsession", nullptr, nullptr, testsetsession, nullptr);
        reg_raw("", "/testshowsession", nullptr, nullptr, testshowsession, nullptr);
        reg_raw("", "/testsiteid", nullptr, nullptr, testsiteid, nullptr);
        reg_raw("", "/testsitepath", nullptr, nullptr, testsitepath, nullptr);
        reg_raw("", "/testsoftremove", nullptr, nullptr, testsoftremove, nullptr);
        reg_raw("", "/mtuple", nullptr, nullptr, testsqltuple, nullptr);
        reg_raw("", "/teststr2int", nullptr, nullptr, teststr2int, nullptr);
        reg_raw("", "/teststr_join", nullptr, nullptr, teststrjoin, nullptr);
        reg_raw("", "/teststr_trim", nullptr, nullptr, teststrtrim, nullptr);
        reg_raw("", "/teststrip_html", nullptr, nullptr, teststrip_html, nullptr);
        reg_raw("", "/testtotree", nullptr, nullptr, testtotree, nullptr);
        reg_raw("", "/resident_stop", nullptr, nullptr, nullptr, resident_stop);
        reg_raw("", "/plaintext", nullptr, nullptr, nullptr, techempowerplaintext);
        reg_raw("", "/json", nullptr, nullptr, nullptr, techempowerjson);
        reg_raw("", "/db", nullptr, nullptr, nullptr, techempowerdb);
        reg_raw("", "/queries", nullptr, nullptr, nullptr, techempowerqueries);
        reg_raw("", "/fortunes", nullptr, nullptr, nullptr, techempowerfortunes);
        reg_raw("", "/updates", nullptr, nullptr, nullptr, techempowerupdates);
        reg_raw("", "/cached-queries", nullptr, nullptr, nullptr, techempowercached_queries);
        reg_raw("", "/cached-db", nullptr, nullptr, nullptr, techempowercached_db);
        reg_raw("", "/testalipayapp", nullptr, nullptr, nullptr, test_alipay_app);
        reg_raw("", "/testalipaypage", nullptr, nullptr, nullptr, test_alipay_page);
        reg_raw("", "/testalipayfront", nullptr, nullptr, nullptr, test_alipay_front);
        reg_raw("", "/testalipayorders", nullptr, nullptr, nullptr, test_alipay_orders);
        reg_raw("", "/testalipayaquery", nullptr, nullptr, nullptr, test_alipay_aquery);
        reg_raw("", "/alipaynotify", nullptr, nullptr, nullptr, test_alipay_notify);
        reg_raw("", "/alipayreturn", nullptr, nullptr, nullptr, test_alipay_return);
        reg_raw("", "/test_chunked_async", nullptr, nullptr, nullptr, test_chunked_async);
        reg_raw("", "/testcohandle", nullptr, nullptr, nullptr, test_co_handle);
        reg_raw("", "/test_cols_co", nullptr, nullptr, nullptr, test_cols_co);
        reg_raw("", "/test_leftjoin", nullptr, nullptr, nullptr, test_leftjoin);
        reg_raw("", "/test_customstruct", nullptr, nullptr, nullptr, test_customstruct);
        reg_raw("", "/deepseek_async", nullptr, nullptr, nullptr, test_deepseek_async);
        reg_raw("", "/deepseek_sse", nullptr, nullptr, nullptr, test_deepseek_sse);
        reg_raw("", "/deepseek_chunk", nullptr, nullptr, nullptr, test_deepseek_chunk);
        reg_raw("", "/test_httppeersend_async", nullptr, nullptr, nullptr, test_httppeersend_async);
        reg_raw("", "/test_leftjoinlimit", nullptr, nullptr, nullptr, test_leftjoinlimit);
        reg_raw("", "/test_leftjoinfull", nullptr, nullptr, nullptr, test_leftjoinfull);
        reg_raw("", "/test_mqtt_client", nullptr, nullptr, nullptr, test_mqtt_client);
        reg_raw("", "/test_mqtt_tick_push", nullptr, nullptr, nullptr, test_mqtt_tick_push);
        reg_raw("", "/test_mqtt_tick_push_co", nullptr, nullptr, nullptr, test_mqtt_tick_push_co);
        reg_raw("", "/start_mqtt_loop", nullptr, nullptr, nullptr, start_mqtt_loop);
        reg_raw("", "/stop_mqtt_loop", nullptr, nullptr, nullptr, stop_mqtt_loop);
        reg_raw("", "/list_mqtt_loop", nullptr, nullptr, nullptr, list_mqtt_loop);
        reg_raw("", "/test_ormfk_co", nullptr, nullptr, nullptr, test_ormfk_co);
        reg_raw("", "/test_ormprepared_co", nullptr, nullptr, nullptr, test_ormprepared_co);
        reg_raw("", "/redis/bench/async", nullptr, nullptr, nullptr, test_redis_bench_async);
        reg_raw("", "/redis/bench/async_concurrent", nullptr, nullptr, nullptr, test_redis_bench_async_concurrent);
        reg_raw("", "/redis/bench/pool", nullptr, nullptr, nullptr, test_redis_bench_pool);
        reg_raw("", "/redis/bench/ioc_probe", nullptr, nullptr, nullptr, test_ioc_probe);
        reg_raw("", "/redis/bench/async_profile", nullptr, nullptr, nullptr, test_async_profile);
        reg_raw("", "/redis/pool", nullptr, nullptr, nullptr, test_redis_pool);
        reg_raw("", "/redis/pubsub", nullptr, nullptr, nullptr, test_redis_pubsub);
        reg_raw("", "/redis/hardfix", nullptr, nullptr, nullptr, test_redis_hardfix);
        reg_raw("", "/redis/types", nullptr, nullptr, nullptr, test_redis_types);
        reg_raw("", "/test_rpcclient", nullptr, nullptr, nullptr, test_rpcclient);
        reg_raw("", "/test_rpcclientssl", nullptr, nullptr, nullptr, test_rpcclientssl);
        reg_raw("", "/test_rpcserver", nullptr, nullptr, nullptr, test_rpcserver);
        reg_raw("", "/test_rpc_chunkc", nullptr, nullptr, nullptr, test_rpc_chunkc);
        reg_raw("", "/test_rpc_chunks", nullptr, nullptr, nullptr, test_rpc_chunks);
        reg_raw("", "/test_rpc_binary", nullptr, nullptr, nullptr, test_rpc_binary);
        reg_raw("", "/test_rpc_binary_echo", nullptr, nullptr, nullptr, test_rpc_binary_echo);
        reg_raw("", "/test_socket_client", nullptr, nullptr, nullptr, test_socket_client);
        reg_raw("", "/co_sql_commit", nullptr, nullptr, nullptr, test_co_sql_commit);
        reg_raw("", "/co_sql_orm", nullptr, nullptr, nullptr, test_co_sql_orm);
        reg_raw("", "/co_sqlquery", nullptr, nullptr, nullptr, test_co_sqlquery);
        reg_raw("", "/test_websocket_client", nullptr, nullptr, nullptr, test_websocket_client);
        reg_raw("", "/xcxnotify", nullptr, nullptr, nullptr, test_xcxnotify);
        reg_raw("", "/wxpaynotify", nullptr, nullptr, nullptr, test_wxpay_notify);
        reg_raw("", "/testwxpaynative_co", nullptr, nullptr, nullptr, test_wxpay_native_co);
        reg_raw("", "/testwxpayjsapi_co", nullptr, nullptr, nullptr, test_wxpay_jsapi_co);
        reg_raw("", "/testwxpayv2_order_new_co", nullptr, nullptr, nullptr, testwxpayv2_order_new_co);
        reg_raw("", "/testwxpayv2_refund_co", nullptr, nullptr, nullptr, testwxpayv2_refund_co);
        reg_raw("", "/testwxpayv2_refund_query_co", nullptr, nullptr, nullptr, testwxpayv2_refund_query_co);
        reg_raw("", "/testwxpaycertislogin_co", nullptr, nullptr, nullptr, testwxpaycert_islogin_co);
        reg_raw("", "/testwxpaydownloadcert_co", nullptr, testwxpaycert_islogin_co, nullptr, testwxpaydownloadcert_co);
        reg_raw("", "/testwxpayv3native_co", nullptr, nullptr, nullptr, testwxpayv3native_co);
        reg_raw("", "/testwxpayv3jsapi_co", nullptr, nullptr, nullptr, testwxpayv3jsapi_co);
        reg_raw("", "/testwxpayv3_order_new_co", nullptr, nullptr, nullptr, testwxpayv3_order_new_co);
        reg_raw("", "/testwxpayv3_order_query_co", nullptr, nullptr, nullptr, testwxpayv3_order_query_co);
        reg_raw("", "/testwxpayv3_order_status", nullptr, nullptr, nullptr, testwxpayv3_order_status);
        reg_raw("", "/testwxpayv3_refund_co", nullptr, nullptr, nullptr, testwxpayv3_refund_co);
        reg_raw("", "/ordernotify", nullptr, nullptr, nullptr, test_wxpay_v3_notify);
        reg_raw("", "/testcowaitclient21", nullptr, nullptr, nullptr, testhttpclient21_cowait_body);
        reg_raw("", "/testcowaitclient22", nullptr, nullptr, nullptr, testhttpclient22_cowait_body);
        reg_raw("", "/fx/coro", nullptr, nullptr, nullptr, testfxcoro);
        reg_raw("", "/fx/coroa", nullptr, nullptr, nullptr, testfxcoroa);
        reg_raw("", "/fx/corob", nullptr, nullptr, nullptr, testfxcorob);
        reg_raw("", "/fx/coroc", nullptr, nullptr, nullptr, testfxcoroc);
        reg_raw("", "/fx/coro_pre", nullptr, nullptr, nullptr, testfxcoropre);
        reg_raw("", "/fx/mix_coro", nullptr, nullptr, nullptr, testfxmixchain_coro);
        reg_raw("", "/fx/exc/coro_reg", nullptr, nullptr, nullptr, testfxexc_coro_reg);
        reg_raw("", "/fx/exc/coro_pre", nullptr, nullptr, nullptr, testfxexc_coro_pre);
        reg_raw("", "/fx/exc/coro_unknown", nullptr, nullptr, nullptr, testfxexc_coro_unknown);
        reg_raw("", "/fx/exc/reject503_pre", nullptr, nullptr, nullptr, testfxexc_reject503_pre);
        reg_raw("", "/fx/lane/a", nullptr, nullptr, nullptr, testfxlanea);
        reg_raw("", "/fx/lane/rejectonce", nullptr, nullptr, nullptr, testfxlanerejectonce);
        reg_raw("", "/testcosendmaildo", nullptr, nullptr, nullptr, test_co_sendmaildo);
        reg_raw("saas.com", "/testcohandle", nullptr, nullptr, nullptr, saas::test_co_handle);

    }

}

#endif
