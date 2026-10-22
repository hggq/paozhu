
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
#include "test_orm.h"
#include "test_ormfk.h"
#include "test_ormprepared.h"
#include "test_parse_ini.h"
#include "test_pg_crud.h"
#include "test_pzexcel.h"
#include "test_pzexcel_p2.h"
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
        reg_raw("", "/admin/addarticle", admin_islogin, nullptr, admin_addarticle, nullptr, 0);
        reg_raw("", "/admin/addarticlepost", admin_islogin, nullptr, admin_addarticlepost, nullptr, 0);
        reg_raw("", "/admin/editarticle", admin_islogin, nullptr, admin_editarticle, nullptr, 0);
        reg_raw("", "/admin/editarticlepost", admin_islogin, nullptr, admin_editarticlepost, nullptr, 0);
        reg_raw("", "/admin/deletearticle", admin_isloginjson, nullptr, admin_deletearticle, nullptr, 0);
        reg_raw("", "/admin/gettoparticle", admin_isloginjson, nullptr, admin_gettoparticle, nullptr, 0);
        reg_raw("", "/admin/updatearticlesort", admin_isloginjson, nullptr, admin_updatearticlesort, nullptr, 0);
        reg_raw("", "/admin/updatearticleview", admin_isloginjson, nullptr, admin_updatearticleview, nullptr, 0);
        reg_raw("", "/admin/updatearticleishome", admin_isloginjson, nullptr, admin_updatearticleishome, nullptr, 0);
        reg_raw("", "/admin/listarticle", admin_islogin, nullptr, admin_listarticle, nullptr, 0);
        reg_raw("", "/attachfile/gateway", admin_islogin, nullptr, attachfile_gateway, nullptr, 0);
        reg_raw("", "/attachimg/gateway", admin_islogin, nullptr, attachimg_gateway, nullptr, 0);
        reg_raw("", "/attachfile/delete", admin_islogin, nullptr, attachfile_delete, nullptr, 0);
        reg_raw("", "/attachimg/delete", admin_islogin, nullptr, attachimg_delete, nullptr, 0);
        reg_raw("", "/attachfile/upload", admin_islogin, nullptr, attachfile_upload, nullptr, 0);
        reg_raw("", "/attachfile/uploadpost", admin_islogin, nullptr, attachfile_uploadpost, nullptr, 0);
        reg_raw("", "/admin/attachselectfiles", admin_islogin, nullptr, admin_attachselectfiles, nullptr, 0);
        reg_raw("", "/admin/marcatalogue", admin_islogin, nullptr, admin_marcatalogue, nullptr, 0);
        reg_raw("", "/admin/addcataloguepost", admin_islogin, nullptr, admin_addcataloguepost, nullptr, 0);
        reg_raw("", "/admin/editcataloguepost", admin_islogin, nullptr, admin_editcataloguepost, nullptr, 0);
        reg_raw("", "/admin/deletecatalogue", admin_islogin, nullptr, admin_deletecatalogue, nullptr, 0);
        reg_raw("", "/admin/login", nullptr, nullptr, admin_login, nullptr, 0);
        reg_raw("", "/admin/loginpost", nullptr, nullptr, admin_loginpost, nullptr, 0);
        reg_raw("", "/admin/logout", nullptr, nullptr, admin_logout, nullptr, 0);
        reg_raw("", "/admin/islogin", nullptr, nullptr, admin_islogin, nullptr, 0);
        reg_raw("", "/admin/isloginjson", nullptr, nullptr, admin_isloginjson, nullptr, 0);
        reg_raw("", "/admin/main", admin_islogin, nullptr, admin_main, nullptr, 0);
        reg_raw("", "/admin/welcome", admin_islogin, nullptr, admin_welcome, nullptr, 0);
        reg_raw("", "/admin/siteinfo", admin_islogin, nullptr, admin_siteinfo, nullptr, 0);
        reg_raw("", "/admin/siteinfopost", admin_islogin, nullptr, admin_siteinfopost, nullptr, 0);
        reg_raw("", "/admin/footscript", admin_islogin, nullptr, admin_footscript, nullptr, 0);
        reg_raw("", "/admin/footscriptpost", admin_islogin, nullptr, admin_footscriptpost, nullptr, 0);
        reg_raw("", "/admin/copyright", admin_islogin, nullptr, admin_copyright, nullptr, 0);
        reg_raw("", "/admin/copyrightpost", admin_islogin, nullptr, admin_copyrightpost, nullptr, 0);
        reg_raw("", "/admin/sitelogo", admin_islogin, nullptr, admin_sitelogo, nullptr, 0);
        reg_raw("", "/admin/sitelogopost", admin_islogin, nullptr, admin_sitelogopost, nullptr, 0);
        reg_raw("", "/admin/sitebannerpost", admin_islogin, nullptr, admin_sitebannerpost, nullptr, 0);
        reg_raw("", "/admin/userinfo", admin_islogin, nullptr, admin_userinfo, nullptr, 0);
        reg_raw("", "/admin/editpassword", admin_islogin, nullptr, admin_editpassword, nullptr, 0);
        reg_raw("", "/admin/editpwdpost", admin_islogin, nullptr, admin_editpwdpost, nullptr, 0);
        reg_raw("", "/admin/favicon", admin_islogin, nullptr, admin_favicon, nullptr, 0);
        reg_raw("", "/admin/faviconpost", admin_islogin, nullptr, admin_faviconpost, nullptr, 0);
        reg_raw("", "/admin/marbrand", admin_islogin, nullptr, admin_marbrand, nullptr, 0);
        reg_raw("", "/admin/brandaddpost", admin_islogin, nullptr, admin_brandaddpost, nullptr, 0);
        reg_raw("", "/admin/deletebrand", admin_islogin, nullptr, admin_deletebrand, nullptr, 0);
        reg_raw("", "/admin/updatebrandsort", admin_isloginjson, nullptr, admin_updatebrandsort, nullptr, 0);
        reg_raw("", "/admin/marhome", admin_islogin, nullptr, admin_marhome, nullptr, 0);
        reg_raw("", "/admin/edithomeblockpic", admin_islogin, nullptr, admin_edithomeblockpic, nullptr, 0);
        reg_raw("", "/admin/updatehomeblocksort", admin_islogin, nullptr, admin_updatehomeblocksort, nullptr, 0);
        reg_raw("", "/admin/addhomepic", admin_islogin, nullptr, admin_addhomepic, nullptr, 0);
        reg_raw("", "/admin/addhometext", admin_islogin, nullptr, admin_addhometext, nullptr, 0);
        reg_raw("", "/admin/addhometopic", admin_islogin, nullptr, admin_addhometopic, nullptr, 0);
        reg_raw("", "/admin/addhomecontent", admin_islogin, nullptr, admin_addhomecontent, nullptr, 0);
        reg_raw("", "/admin/edithomeblockcontent", admin_islogin, nullptr, admin_edithomeblockcontent, nullptr, 0);
        reg_raw("", "/admin/edithomeblocktopic", admin_islogin, nullptr, admin_edithomeblocktopic, nullptr, 0);
        reg_raw("", "/admin/edithomeblockpost", admin_islogin, nullptr, admin_edithomeblockpost, nullptr, 0);
        reg_raw("", "/admin/addhomeblockpost", admin_islogin, nullptr, admin_addhomeblockpost, nullptr, 0);
        reg_raw("", "/admin/addhomehot", admin_islogin, nullptr, admin_addhomehot, nullptr, 0);
        reg_raw("", "/admin/edithomeblockhot", admin_islogin, nullptr, admin_edithomeblockhot, nullptr, 0);
        reg_raw("", "/admin/edithomeblocktext", admin_islogin, nullptr, admin_edithomeblocktext, nullptr, 0);
        reg_raw("", "/admin/deletehomeblock", admin_islogin, nullptr, admin_deletehomeblock, nullptr, 0);
        reg_raw("", "/admin/homedesign", admin_islogin, nullptr, admin_homedesign, nullptr, 0);
        reg_raw("", "/admin/edithomeblockmulit", admin_islogin, nullptr, admin_edithomeblockmulit, nullptr, 0);
        reg_raw("", "/admin/addproduct", admin_islogin, nullptr, admin_addproduct, nullptr, 0);
        reg_raw("", "/admin/addproductpost", admin_islogin, nullptr, admin_addproductpost, nullptr, 0);
        reg_raw("", "/admin/editproductpost", admin_islogin, nullptr, admin_editproductpost, nullptr, 0);
        reg_raw("", "/admin/getcategorytopproduct", admin_islogin, nullptr, admin_getcategorytopproduct, nullptr, 0);
        reg_raw("", "/admin/editproduct", admin_islogin, nullptr, admin_editproduct, nullptr, 0);
        reg_raw("", "/admin/listproduct", admin_islogin, nullptr, admin_listproduct, nullptr, 0);
        reg_raw("", "/admin/marproductattach", admin_islogin, nullptr, admin_marproductattach, nullptr, 0);
        reg_raw("", "/admin/deleteproduct", admin_islogin, nullptr, admin_deleteproduct, nullptr, 0);
        reg_raw("", "/admin/deleteproductajax", admin_islogin, nullptr, admin_deleteproductajax, nullptr, 0);
        reg_raw("", "/admin/updateproducthome", admin_islogin, nullptr, admin_updateproducthome, nullptr, 0);
        reg_raw("", "/admin/updateproductstore", admin_islogin, nullptr, admin_updateproductstore, nullptr, 0);
        reg_raw("", "/admin/updateproductview", admin_islogin, nullptr, admin_updateproductview, nullptr, 0);
        reg_raw("", "/admin/updateproductsort", admin_islogin, nullptr, admin_updateproductsort, nullptr, 0);
        reg_raw("", "/admin/menu", nullptr, nullptr, admin_menu, nullptr, 0);
        reg_raw("", "/admin/vuetest", nullptr, nullptr, admin_vuetest, nullptr, 0);
        reg_raw("", "/admin/dashboard", nullptr, nullptr, admin_dashboard, nullptr, 0);
        reg_raw("", "/admin/jstimetest", nullptr, nullptr, admin_jstimetest, nullptr, 0);
        reg_raw("", "/admin/testelementplus", nullptr, nullptr, admin_testelementplus, nullptr, 0);
        reg_raw("", "/admin/testgetjson", nullptr, nullptr, admin_testgetjson, nullptr, 0);
        reg_raw("", "/admin/addtopic", admin_islogin, nullptr, admin_addtopic, nullptr, 0);
        reg_raw("", "/admin/edittopic", admin_islogin, nullptr, admin_edittopic, nullptr, 0);
        reg_raw("", "/admin/martopic", admin_islogin, nullptr, admin_martopic, nullptr, 0);
        reg_raw("", "/admin/updatetopicsort", admin_isloginjson, nullptr, admin_updatetopicsort, nullptr, 0);
        reg_raw("", "/admin/addtopicpost", admin_isloginjson, nullptr, admin_addtopicpost, nullptr, 0);
        reg_raw("", "/admin/deletetopic", admin_isloginjson, nullptr, admin_deletetopic, nullptr, 0);
        reg_raw("", "/admin/edittopicpost", admin_isloginjson, nullptr, admin_edittopicpost, nullptr, 0);
        reg_raw("", "/admin/topicfileupload", admin_isloginjson, nullptr, admin_topicfileupload, nullptr, 0);
        reg_raw("", "/admin/topicimgtextupload", admin_isloginjson, nullptr, admin_topicimgtextupload, nullptr, 0);
        reg_raw("", "/admin/updatetopicview", admin_isloginjson, nullptr, admin_updatetopicview, nullptr, 0);
        reg_raw("", "/admin/updatetopicsideblocksort", admin_isloginjson, nullptr, admin_updatetopicsideblocksort, nullptr, 0);
        reg_raw("", "/admin/edittopicside", admin_isloginjson, nullptr, admin_edittopicside, nullptr, 0);
        reg_raw("", "/admin/addtopicsidepick", admin_isloginjson, nullptr, admin_addtopicsidepick, nullptr, 0);
        reg_raw("", "/admin/addtopicsidetext", admin_isloginjson, nullptr, admin_addtopicsidetext, nullptr, 0);
        reg_raw("", "/admin/addtopicsideblockpost", admin_isloginjson, nullptr, admin_addtopicsideblockpost, nullptr, 0);
        reg_raw("", "/admin/deletetopicsideblock", admin_isloginjson, nullptr, admin_deletetopicsideblock, nullptr, 0);
        reg_raw("", "/admin/edittopicsideblocktext", admin_isloginjson, nullptr, admin_edittopicsideblocktext, nullptr, 0);
        reg_raw("", "/admin/edittopicsideblockpick", admin_isloginjson, nullptr, admin_edittopicsideblockpick, nullptr, 0);
        reg_raw("", "/admin/edittopicsideblockpost", admin_isloginjson, nullptr, admin_edittopicsideblockpost, nullptr, 0);
        reg_raw("", "/api/departments/addpost", nullptr, nullptr, pxapidepartmentsaddpost, nullptr, 0);
        reg_raw("", "/api/departments/editpost", nullptr, nullptr, pxapidepartmentseditpost, nullptr, 0);
        reg_raw("", "/api/departments/list", nullptr, nullptr, pxapidepartmentslist, nullptr, 0);
        reg_raw("", "/api/departments/deletedep", nullptr, nullptr, pxapipagesdepartlist, nullptr, 0);
        reg_raw("", "/api/dev/hostcors", nullptr, nullptr, api_dev_hostcors, nullptr, 0);
        reg_raw("", "/imageapi/gateway", nullptr, nullptr, imageapi_gateway, nullptr, 0);
        reg_raw("", "/imageapi/upload", nullptr, nullptr, imageapi_upload, nullptr, 0);
        reg_raw("", "/paozhu_status", nullptr, nullptr, paozhu_status, nullptr, 0);
        reg_raw("", "/frametasks_timeloop", nullptr, nullptr, frametasks_timeloop, nullptr, 0);
        reg_raw("", "/superadmin/login", nullptr, nullptr, superadmin_login, nullptr, 0);
        reg_raw("", "/superadmin/loginpost", nullptr, nullptr, superadmin_loginpost, nullptr, 0);
        reg_raw("", "/superadmin/logout", nullptr, nullptr, superadmin_logout, nullptr, 0);
        reg_raw("", "/superadmin/islogin", nullptr, nullptr, superadmin_islogin, nullptr, 0);
        reg_raw("", "/superadmin/isloginjson", nullptr, nullptr, superadmin_isloginjson, nullptr, 0);
        reg_raw("", "/superadmin/main", superadmin_islogin, nullptr, superadmin_main, nullptr, 0);
        reg_raw("", "/superadmin/editsiteinfo", superadmin_islogin, nullptr, superadmin_editsiteinfo, nullptr, 0);
        reg_raw("", "/superadmin/editsiteinfopost", superadmin_islogin, nullptr, superadmin_editsiteinfopost, nullptr, 0);
        reg_raw("", "/superadmin/deletesiteinfo", superadmin_islogin, nullptr, superadmin_deletesiteinfo, nullptr, 0);
        reg_raw("", "/superadmin/deletesiteinfopost", superadmin_islogin, nullptr, superadmin_deletesiteinfopost, nullptr, 0);
        reg_raw("", "/superadmin/addsiteinfo", superadmin_islogin, nullptr, superadmin_addsiteinfo, nullptr, 0);
        reg_raw("", "/superadmin/addsiteinfopost", superadmin_islogin, nullptr, superadmin_addsiteinfopost, nullptr, 0);
        reg_raw("", "/superadmin/welcome", superadmin_islogin, nullptr, superadmin_welcome, nullptr, 0);
        reg_raw("", "/superadmin/listuser", superadmin_islogin, nullptr, superadmin_listuser, nullptr, 0);
        reg_raw("", "/superadmin/edituser", superadmin_islogin, nullptr, superadmin_edituser, nullptr, 0);
        reg_raw("", "/superadmin/adduser", superadmin_islogin, nullptr, superadmin_adduser, nullptr, 0);
        reg_raw("", "/superadmin/adduserpost", superadmin_islogin, nullptr, superadmin_adduserpost, nullptr, 0);
        reg_raw("", "/superadmin/edituserpost", superadmin_islogin, nullptr, superadmin_edituserpost, nullptr, 0);
        reg_raw("", "/superadmin/deleteuser", superadmin_islogin, nullptr, superadmin_deleteuser, nullptr, 0);
        reg_raw("", "/superadmin/userinfo", superadmin_islogin, nullptr, superadmin_userinfo, nullptr, 0);
        reg_raw("", "/superadmin/editpassword", superadmin_islogin, nullptr, superadmin_editpassword, nullptr, 0);
        reg_raw("", "/superadmin/editpwdpost", superadmin_isloginjson, nullptr, superadmin_editpwdpost, nullptr, 0);
        reg_raw("", "/test_acme", nullptr, nullptr, test_acme, nullptr, 0);
        reg_raw("", "/testalipaydiag", nullptr, nullptr, test_alipay_diag, nullptr, 0);
        reg_raw("", "/testalipayqrcode", nullptr, nullptr, test_alipay_qrcode, nullptr, 0);
        reg_raw("", "/testalipayquery", nullptr, nullptr, test_alipay_query, nullptr, 0);
        reg_raw("", "/testalipaycancel", nullptr, nullptr, test_alipay_cancel, nullptr, 0);
        reg_raw("", "/testalipayrefund", nullptr, nullptr, test_alipay_refund, nullptr, 0);
        reg_raw("", "/test_chunked", nullptr, nullptr, test_chunked, nullptr, 0);
        reg_raw("", "/test_chunkedfile", nullptr, nullptr, test_chunkedfile, nullptr, 0);
        reg_raw("", "/test_cols", nullptr, nullptr, test_cols, nullptr, 0);
        reg_raw("", "/deepseek_api", nullptr, nullptr, test_deepseek_api, nullptr, 0);
        reg_raw("", "/test_dir_name", nullptr, nullptr, test_dir_name, nullptr, 0);
        reg_raw("", "/test_escapestr", nullptr, nullptr, test_escapestr, nullptr, 0);
        reg_raw("", "/test_httppeersend", nullptr, nullptr, test_httppeersend, nullptr, 0);
        reg_raw("", "/test_httppeersend_text", nullptr, nullptr, test_httppeersend_text, nullptr, 0);
        reg_raw("", "/test_ormfk_one", nullptr, nullptr, test_ormfk_one, nullptr, 0);
        reg_raw("", "/test_ormfk_many", nullptr, nullptr, test_ormfk_many, nullptr, 0);
        reg_raw("", "/test_ormfk_join", nullptr, nullptr, test_ormfk_join, nullptr, 0);
        reg_raw("", "/test_ormprepared_dsl", nullptr, nullptr, test_ormprepared_dsl, nullptr, 0);
        reg_raw("", "/test_ormprepared_exec", nullptr, nullptr, test_ormprepared_exec, nullptr, 0);
        reg_raw("", "/test_ormprepared_write", nullptr, nullptr, test_ormprepared_write, nullptr, 0);
        reg_raw("", "/test_parse_ini", nullptr, nullptr, test_parse_ini, nullptr, 0);
        reg_raw("", "/test_parse_fix", nullptr, nullptr, test_parse_fix, nullptr, 0);
        reg_raw("", "/test_fix_server_conf", nullptr, nullptr, test_fix_server_conf, nullptr, 0);
        reg_raw("", "/test_fix_orm_conf", nullptr, nullptr, test_fix_orm_conf, nullptr, 0);
        reg_raw("", "/cms/pglist", nullptr, nullptr, article_pg_list, nullptr, 0);
        reg_raw("", "/cms/pgshow", nullptr, nullptr, article_pg_show, nullptr, 0);
        reg_raw("", "/test_pzexcel", nullptr, nullptr, test_pzexcel, nullptr, 0);
        reg_raw("", "/test_pzexcel_p2", nullptr, nullptr, test_pzexcel_p2, nullptr, 0);
        reg_raw("", "/test_pzjpg", nullptr, nullptr, test_pzjpg, nullptr, 0);
        reg_raw("", "/test_outjpg", nullptr, nullptr, test_outjpg, nullptr, 0);
        reg_raw("", "/test_showjpg", nullptr, nullptr, test_showjpg, nullptr, 0);
        reg_raw("", "/test_pzpng", nullptr, nullptr, test_pzpng, nullptr, 0);
        reg_raw("", "/test_outpng", nullptr, nullptr, test_outpng, nullptr, 0);
        reg_raw("", "/test_captcha", nullptr, nullptr, test_captcha, nullptr, 0);
        reg_raw("", "/test_pzword", nullptr, nullptr, test_pzword, nullptr, 0);
        reg_raw("", "/test_pzzip", nullptr, nullptr, test_pzzip, nullptr, 0);
        reg_raw("", "/redis/bench/sync", nullptr, nullptr, test_redis_bench_sync, nullptr, 0);
        reg_raw("", "/redis/bench/direct", nullptr, nullptr, test_redis_bench_direct, nullptr, 0);
        reg_raw("", "/redis/parse", nullptr, nullptr, test_redis_parse, nullptr, 0);
        reg_raw("", "/redis/sync", nullptr, nullptr, test_redis_sync, nullptr, 0);
        reg_raw("", "/redis/types/sync", nullptr, nullptr, test_redis_types_sync, nullptr, 0);
        reg_raw("", "/testmb_reverse", nullptr, nullptr, testmb_reverse, nullptr, 0);
        reg_raw("", "/test_sql_commit", nullptr, nullptr, test_sql_commit, nullptr, 0);
        reg_raw("", "/testsqljson", nullptr, nullptr, testsqljson, nullptr, 0);
        reg_raw("", "/sqlquery", nullptr, nullptr, testsqlquery, nullptr, 0);
        reg_raw("", "/cms/sqlist", nullptr, nullptr, article_sqlite_list, nullptr, 0);
        reg_raw("", "/cms/sqshow", nullptr, nullptr, article_sqlite_show, nullptr, 0);
        reg_raw("", "/test_svgchart", nullptr, nullptr, test_svgchart, nullptr, 0);
        reg_raw("", "/test_svgstats", nullptr, nullptr, test_svgstats, nullptr, 0);
        reg_raw("", "/test_webpdf", nullptr, nullptr, test_webpdf, nullptr, 0);
        reg_raw("", "/test_ttfpdf", nullptr, nullptr, test_otfpdf, nullptr, 0);
        reg_raw("", "/test_table_linebreak", nullptr, nullptr, test_table_linebreak, nullptr, 0);
        reg_raw("", "/testgetopenid", nullptr, nullptr, test_getopenid, nullptr, 0);
        reg_raw("", "/testweixinpay", nullptr, nullptr, test_weixinpay, nullptr, 0);
        reg_raw("", "/testweixinnative", nullptr, nullptr, test_weixin_native, nullptr, 0);
        reg_raw("", "/xcxgetphone", nullptr, nullptr, test_xcxgetphone, nullptr, 0);
        reg_raw("", "/testweixin_order_new", nullptr, nullptr, test_weixin_order_new, nullptr, 0);
        reg_raw("", "/testweixin_order_list", nullptr, nullptr, test_weixin_order_list, nullptr, 0);
        reg_raw("", "/testweixin_order_detail", nullptr, nullptr, test_weixin_order_detail, nullptr, 0);
        reg_raw("", "/testweixin_refund", nullptr, nullptr, test_weixin_refund, nullptr, 0);
        reg_raw("", "/testweixin_refund_query", nullptr, nullptr, test_weixin_refund_query, nullptr, 0);
        reg_raw("", "/testwxpaynative", nullptr, nullptr, test_wxpay_native, nullptr, 0);
        reg_raw("", "/testwxpayjsapi", nullptr, nullptr, test_wxpay_jsapi, nullptr, 0);
        reg_raw("", "/testwxpayapp", nullptr, nullptr, test_wxpay_app, nullptr, 0);
        reg_raw("", "/testwxpayh5", nullptr, nullptr, test_wxpay_h5, nullptr, 0);
        reg_raw("", "/testwxpayquery", nullptr, nullptr, test_wxpay_query, nullptr, 0);
        reg_raw("", "/testwxpayclose", nullptr, nullptr, test_wxpay_close, nullptr, 0);
        reg_raw("", "/wxpaydownloadcert", nullptr, nullptr, test_wxpay_download_cert, nullptr, 0);
        reg_raw("", "/testwxpayrefund", nullptr, nullptr, test_wxpay_refund, nullptr, 0);
        reg_raw("", "/testwxpayv2_order_new", nullptr, nullptr, testwxpayv2_order_new, nullptr, 0);
        reg_raw("", "/testwxpayv2_order_list", nullptr, nullptr, testwxpayv2_order_list, nullptr, 0);
        reg_raw("", "/testwxpayv2_order_detail", nullptr, nullptr, testwxpayv2_order_detail, nullptr, 0);
        reg_raw("", "/testwxpayv2_refund", nullptr, nullptr, testwxpayv2_refund, nullptr, 0);
        reg_raw("", "/testwxpayv2_refund_query", nullptr, nullptr, testwxpayv2_refund_query, nullptr, 0);
        reg_raw("", "/testwxpaycertislogin", nullptr, nullptr, testwxpaycert_islogin, nullptr, 0);
        reg_raw("", "/testwxpay", nullptr, nullptr, testwxpay, nullptr, 0);
        reg_raw("", "/testwxpayv3jsapi", nullptr, nullptr, testwxpayv3jsapi, nullptr, 0);
        reg_raw("", "/testwxpaydownloadcert", testwxpaycert_islogin, nullptr, testwxpaydownloadcert, nullptr, 0);
        reg_raw("", "/testwxpayv3_order_new", nullptr, nullptr, testwxpayv3_order_new, nullptr, 0);
        reg_raw("", "/testwxpayv3_order_list", nullptr, nullptr, testwxpayv3_order_list, nullptr, 0);
        reg_raw("", "/testwxpayv3_order_detail", nullptr, nullptr, testwxpayv3_order_detail, nullptr, 0);
        reg_raw("", "/testwxpayv3_refund", nullptr, nullptr, testwxpayv3_refund, nullptr, 0);
        reg_raw("", "/testwxpayv3_refund_query", nullptr, nullptr, testwxpayv3_refund_query, nullptr, 0);
        reg_raw("", "/testnotaddclienttaskpre", nullptr, nullptr, testaddclienttaskpre, nullptr, 0);
        reg_raw("", "/testaddclienttask", testaddclienttaskpre, nullptr, testaddclienttask, nullptr, 0);
        reg_raw("", "/executeclienttask", nullptr, nullptr, testexecuteclienttask, nullptr, 0);
        reg_raw("", "/ccmake", nullptr, nullptr, testcmake, nullptr, 0);
        reg_raw("", "/ccauto", nullptr, nullptr, testcauto, nullptr, 0);
        reg_raw("", "/testcommit", nullptr, nullptr, testcommit, nullptr, 0);
        reg_raw("", "/api/user/message", nullptr, nullptr, testcors, nullptr, 0);
        reg_raw("", "/api/user/info", nullptr, nullptr, testcorssimple, nullptr, 0);
        reg_raw("", "/api/user/vary", nullptr, nullptr, testcorsvary, nullptr, 0);
        reg_raw("", "/testcowaitclient4", nullptr, nullptr, testhttpclient_cowait_php, nullptr, 0);
        reg_raw("", "/testcowaitclient1", nullptr, nullptr, testhttpclient_cowait_body, nullptr, 0);
        reg_raw("", "/testcowaitclient5", nullptr, nullptr, testhttpclient_cowait_post, nullptr, 0);
        reg_raw("", "/testcowaitclient2", nullptr, nullptr, testhttpclient_cowait_urls, nullptr, 0);
        reg_raw("", "/testcowaitclient3", nullptr, nullptr, testhttpclient_cowait_spawn, nullptr, 0);
        reg_raw("", "/testclientgetrange", nullptr, nullptr, testhttpclient_get_range, nullptr, 0);
        reg_raw("", "/downfilelist", nullptr, nullptr, testhttpclient_downfilelist, nullptr, 0);
        reg_raw("", "/downfilecontent", nullptr, nullptr, testhttpclient_getdownfile, nullptr, 0);
        reg_raw("", "/cms/login", nullptr, nullptr, articlelogin, nullptr, 0);
        reg_raw("", "/cms/islogin", nullptr, nullptr, articleislogin, nullptr, 0);
        reg_raw("", "/cms/loginpost", nullptr, nullptr, articleloginpost, nullptr, 0);
        reg_raw("", "/cms/list", nullptr, nullptr, articlelist, nullptr, 0);
        reg_raw("", "/cms/show", nullptr, nullptr, articleshow, nullptr, 0);
        reg_raw("", "/cms/edit", articleislogin, nullptr, articleedit, nullptr, 0);
        reg_raw("", "/cms/editpost", articleislogin, nullptr, articleeditpost, nullptr, 0);
        reg_raw("", "/cms/add", articleislogin, nullptr, articleadd, nullptr, 0);
        reg_raw("", "/cms/addpost", articleislogin, nullptr, articleaddpost, nullptr, 0);
        reg_raw("", "/cms/delete", articleislogin, nullptr, articledelete, nullptr, 0);
        reg_raw("", "/cms/prepost", articleislogin, nullptr, articleprepost, nullptr, 0);
        reg_raw("", "/cms/preselect", articleislogin, nullptr, articlepreselect, nullptr, 0);
        reg_raw("", "/dpshadow", nullptr, nullptr, testdpshadowshort, nullptr, 0);
        reg_raw("", "/dpshadow/sub", nullptr, nullptr, testdpshadowexact, nullptr, 0);
        reg_raw("", "/dp6/a/b/c/d/e", nullptr, nullptr, testdpprobedepth6, nullptr, 0);
        reg_urlpath("", "/dp6/a/b/c/d/e", {"", "", "", "", "", "", "tail"});
        reg_raw("", "/dp7/a/b/c/d/e/f", nullptr, nullptr, testdpprobedepth7, nullptr, 0);
        reg_urlpath("", "/dp7/a/b/c/d/e/f", {"", "", "", "", "", "", "", "tail"});
        reg_raw("", "/testuser/info", nullptr, nullptr, testdpuserinfo2, nullptr, 0);
        reg_urlpath("", "/testuser/info", {"", "", "userid", "groupid"});
        reg_raw("", "/downloadfileauth", nullptr, nullptr, downloadfileauthmethod, nullptr, 0);
        reg_raw("", "/testfieldnum", nullptr, nullptr, testfieldnum, nullptr, 0);
        reg_raw("", "/fx/preonly", nullptr, nullptr, testfxpreonly, nullptr, 0);
        reg_raw("", "/fx/guarded", testfxpreonly, nullptr, testfxguarded, nullptr, 0);
        reg_raw("", "/fx/a", nullptr, nullptr, testfxa, nullptr, 0);
        reg_raw("", "/fx/b", nullptr, nullptr, testfxb, nullptr, 0);
        reg_raw("", "/fx/c", nullptr, nullptr, testfxc, nullptr, 0);
        reg_raw("", "/fx/d", nullptr, nullptr, testfxd, nullptr, 0);
        reg_raw("", "/fx/plain", nullptr, nullptr, testfxplain, nullptr, 0);
        reg_raw("", "/fx/loop", nullptr, nullptr, testfxloop, nullptr, 0);
        reg_raw("", "/fx/gone", nullptr, nullptr, testfxgone, nullptr, 0);
        reg_raw("", "/fx/exit", nullptr, nullptr, testfxexit, nullptr, 0);
        reg_raw("", "/fx/depa", nullptr, nullptr, testfxdepa, nullptr, 0);
        reg_raw("", "/fx/depb", nullptr, nullptr, testfxdepb, nullptr, 0);
        reg_raw("", "/fx/sync_push_coro", nullptr, nullptr, testfxsyncpushcoro, nullptr, 0);
        reg_raw("", "/fx/coro_pre_sync", nullptr, testfxcoropre, testfxcoropresync, nullptr, 0);
        reg_raw("", "/fx/mix_chain", nullptr, nullptr, testfxmixchain_sync, nullptr, 0);
        reg_raw("", "/fx/mix_tail", nullptr, nullptr, testfxmixchain_tail, nullptr, 0);
        reg_raw("", "/fx/exc/sync_reg", nullptr, nullptr, testfxexc_sync_reg, nullptr, 0);
        reg_raw("", "/fx/exc/sync_pre", nullptr, nullptr, testfxexc_sync_pre, nullptr, 0);
        reg_raw("", "/fx/exc/sync_pre_reg", testfxexc_sync_pre, nullptr, testfxexc_sync_pre_reg, nullptr, 0);
        reg_raw("", "/fx/exc/coro_pre_reg", nullptr, testfxexc_coro_pre, testfxexc_coro_pre_reg, nullptr, 0);
        reg_raw("", "/fx/exc/reject503", nullptr, testfxexc_reject503_pre, testfxexc_reject503, nullptr, 0);
        reg_raw("", "/fx/exc/after503", nullptr, nullptr, testfxexc_after503, nullptr, 0);
        reg_raw("", "/fx/lane/b", nullptr, nullptr, testfxlaneb, nullptr, 0);
        reg_raw("", "/fx/lane/read", nullptr, nullptr, testfxlaneread, nullptr, 0);
        reg_raw("", "/fx/loop/slow", nullptr, nullptr, testfxloopslow, nullptr, 0);
        reg_raw("", "/fxlooptick", nullptr, nullptr, testfxlooptick, nullptr, 0);
        reg_raw("", "/tformpost", nullptr, nullptr, testurlencoded, nullptr, 0);
        reg_raw("", "/tfilepost", nullptr, nullptr, testformmultipart, nullptr, 0);
        reg_raw("", "/tjsonpost", nullptr, nullptr, testformjsonpost, nullptr, 0);
        reg_raw("", "/txmlupload", nullptr, nullptr, testformxmlpost, nullptr, 0);
        reg_raw("", "/addpostfile", nullptr, nullptr, testuploadpostfile, nullptr, 0);
        reg_raw("", "/hellobusy", nullptr, nullptr, testhellobusy, nullptr, 0);
        reg_raw("", "/testhttpclient1", nullptr, nullptr, testhttpclient_get_body, nullptr, 0);
        reg_raw("", "/testhttpclient3", nullptr, nullptr, testhttpclient_get_timebody, nullptr, 0);
        reg_raw("", "/testhttpclient2", nullptr, nullptr, testhttpclient_get_file, nullptr, 0);
        reg_raw("", "/testipsearch", nullptr, nullptr, testipsearch, nullptr, 0);
        reg_raw("", "/ipsearchget", nullptr, nullptr, testipsearchget, nullptr, 0);
        reg_raw("", "/testjson", nullptr, nullptr, testjson, nullptr, 0);
        reg_raw("", "/test_requst", nullptr, nullptr, test_requst, nullptr, 0);
        reg_raw("", "/testjsonreflect", nullptr, nullptr, testjsonreflect, nullptr, 0);
        reg_raw("", "/testmarkdown2html", nullptr, nullptr, testmarkdown2html, nullptr, 0);
        reg_raw("", "/mfromjson", nullptr, nullptr, testmodelfromjson, nullptr, 0);
        reg_raw("", "/testtestmoneynum", nullptr, nullptr, testtestmoneynum, nullptr, 0);
        reg_raw("", "/minsert", nullptr, nullptr, testmysqlinsert, nullptr, 0);
        reg_raw("", "/mpagebar", nullptr, nullptr, testmysqlpagebar, nullptr, 0);
        reg_raw("", "/testormcache", nullptr, nullptr, testormcache, nullptr, 0);
        reg_raw("", "/testormcacheb", nullptr, nullptr, testormcacheb, nullptr, 0);
        reg_raw("", "/testormcachec", nullptr, nullptr, testormcachec, nullptr, 0);
        reg_raw("", "/testormcache_d", nullptr, nullptr, testormcache_d, nullptr, 0);
        reg_urlpath("", "/testormcache_d", {"", "aid"});
        reg_raw("", "/testormcache_d_invalidate", nullptr, nullptr, testormcache_d_invalidate, nullptr, 0);
        reg_urlpath("", "/testormcache_d_invalidate", {"", "aid"});
        reg_raw("", "/testormcache_e", nullptr, nullptr, testormcache_e, nullptr, 0);
        reg_urlpath("", "/testormcache_e", {"", "aid"});
        reg_raw("", "/testormclient", nullptr, nullptr, testormclient, nullptr, 0);
        reg_raw("", "/testpinyin", nullptr, nullptr, testpinyin, nullptr, 0);
        reg_raw("", "/testpinyin_loaded", nullptr, nullptr, testpinyin_loaded, nullptr, 0);
        reg_raw("", "/testcache", nullptr, nullptr, testpzcache, nullptr, 0);
        reg_raw("", "/testshowcache", nullptr, nullptr, testshowcache, nullptr, 0);
        reg_raw("", "/testqrcode", nullptr, nullptr, testqrcode, nullptr, 0);
        reg_raw("", "/testrand", nullptr, nullptr, testrand, nullptr, 0);
        reg_raw("", "/user/info", nullptr, nullptr, testrestfulpath, nullptr, 0);
        reg_urlpath("", "/user/info", {"", "", "userid"});
        reg_raw("", "/user/profile", nullptr, nullptr, testrestfulprofilepath, nullptr, 0);
        reg_urlpath("", "/user/profile", {"", "", "userid", "pathid"});
        reg_raw("", "/testsendmaildo", nullptr, nullptr, testsendmaildo, nullptr, 0);
        reg_raw("", "/testsetsession", nullptr, nullptr, testsetsession, nullptr, 0);
        reg_raw("", "/testshowsession", nullptr, nullptr, testshowsession, nullptr, 0);
        reg_raw("", "/testsiteid", nullptr, nullptr, testsiteid, nullptr, 0);
        reg_raw("", "/testsitepath", nullptr, nullptr, testsitepath, nullptr, 0);
        reg_raw("", "/testsoftremove", nullptr, nullptr, testsoftremove, nullptr, 0);
        reg_raw("", "/mtuple", nullptr, nullptr, testsqltuple, nullptr, 0);
        reg_raw("", "/teststr2int", nullptr, nullptr, teststr2int, nullptr, 0);
        reg_raw("", "/teststr_join", nullptr, nullptr, teststrjoin, nullptr, 0);
        reg_raw("", "/teststr_trim", nullptr, nullptr, teststrtrim, nullptr, 0);
        reg_raw("", "/teststrip_html", nullptr, nullptr, teststrip_html, nullptr, 0);
        reg_raw("", "/testtotree", nullptr, nullptr, testtotree, nullptr, 0);
        reg_raw("", "/plaintext", nullptr, nullptr, nullptr, techempowerplaintext, 0);
        reg_raw("", "/json", nullptr, nullptr, nullptr, techempowerjson, 0);
        reg_raw("", "/db", nullptr, nullptr, nullptr, techempowerdb, 0);
        reg_raw("", "/queries", nullptr, nullptr, nullptr, techempowerqueries, 0);
        reg_raw("", "/fortunes", nullptr, nullptr, nullptr, techempowerfortunes, 0);
        reg_raw("", "/updates", nullptr, nullptr, nullptr, techempowerupdates, 0);
        reg_raw("", "/cached-queries", nullptr, nullptr, nullptr, techempowercached_queries, 0);
        reg_raw("", "/cached-db", nullptr, nullptr, nullptr, techempowercached_db, 0);
        reg_raw("", "/testalipayapp", nullptr, nullptr, nullptr, test_alipay_app, 0);
        reg_raw("", "/testalipaypage", nullptr, nullptr, nullptr, test_alipay_page, 0);
        reg_raw("", "/testalipayfront", nullptr, nullptr, nullptr, test_alipay_front, 0);
        reg_raw("", "/testalipayorders", nullptr, nullptr, nullptr, test_alipay_orders, 0);
        reg_raw("", "/testalipayaquery", nullptr, nullptr, nullptr, test_alipay_aquery, 0);
        reg_raw("", "/alipaynotify", nullptr, nullptr, nullptr, test_alipay_notify, 0);
        reg_raw("", "/alipayreturn", nullptr, nullptr, nullptr, test_alipay_return, 0);
        reg_raw("", "/test_chunked_async", nullptr, nullptr, nullptr, test_chunked_async, 0);
        reg_raw("", "/testcohandle", nullptr, nullptr, nullptr, test_co_handle, 123456);
        reg_raw("", "/test_cols_co", nullptr, nullptr, nullptr, test_cols_co, 0);
        reg_raw("", "/test_leftjoin", nullptr, nullptr, nullptr, test_leftjoin, 0);
        reg_raw("", "/test_customstruct", nullptr, nullptr, nullptr, test_customstruct, 0);
        reg_raw("", "/deepseek_async", nullptr, nullptr, nullptr, test_deepseek_async, 0);
        reg_raw("", "/deepseek_sse", nullptr, nullptr, nullptr, test_deepseek_sse, 0);
        reg_raw("", "/deepseek_chunk", nullptr, nullptr, nullptr, test_deepseek_chunk, 0);
        reg_raw("", "/test_httppeersend_async", nullptr, nullptr, nullptr, test_httppeersend_async, 0);
        reg_raw("", "/test_leftjoinlimit", nullptr, nullptr, nullptr, test_leftjoinlimit, 0);
        reg_raw("", "/test_leftjoinfull", nullptr, nullptr, nullptr, test_leftjoinfull, 0);
        reg_raw("", "/test_mqtt_client", nullptr, nullptr, nullptr, test_mqtt_client, 0);
        reg_raw("", "/test_mqtt_tick_push", nullptr, nullptr, nullptr, test_mqtt_tick_push, 0);
        reg_raw("", "/test_mqtt_tick_push_co", nullptr, nullptr, nullptr, test_mqtt_tick_push_co, 0);
        reg_raw("", "/start_mqtt_loop", nullptr, nullptr, nullptr, start_mqtt_loop, 0);
        reg_raw("", "/stop_mqtt_loop", nullptr, nullptr, nullptr, stop_mqtt_loop, 0);
        reg_raw("", "/list_mqtt_loop", nullptr, nullptr, nullptr, list_mqtt_loop, 0);
        reg_raw("", "/testormhello", nullptr, nullptr, nullptr, testormhello, 0);
        reg_raw("", "/test_ormfk_co", nullptr, nullptr, nullptr, test_ormfk_co, 0);
        reg_raw("", "/test_ormprepared_co", nullptr, nullptr, nullptr, test_ormprepared_co, 0);
        reg_raw("", "/test_ormprepared_appendco", nullptr, nullptr, nullptr, test_ormprepared_appendco, 0);
        reg_raw("", "/stresshello", nullptr, nullptr, nullptr, stresshello, 0);
        reg_raw("", "/redis/bench/async", nullptr, nullptr, nullptr, test_redis_bench_async, 0);
        reg_raw("", "/redis/bench/async_concurrent", nullptr, nullptr, nullptr, test_redis_bench_async_concurrent, 0);
        reg_raw("", "/redis/bench/pool", nullptr, nullptr, nullptr, test_redis_bench_pool, 0);
        reg_raw("", "/redis/bench/ioc_probe", nullptr, nullptr, nullptr, test_ioc_probe, 0);
        reg_raw("", "/redis/bench/async_profile", nullptr, nullptr, nullptr, test_async_profile, 0);
        reg_raw("", "/redis/pool", nullptr, nullptr, nullptr, test_redis_pool, 0);
        reg_raw("", "/redis/pubsub", nullptr, nullptr, nullptr, test_redis_pubsub, 0);
        reg_raw("", "/redis/hardfix", nullptr, nullptr, nullptr, test_redis_hardfix, 0);
        reg_raw("", "/redis/types", nullptr, nullptr, nullptr, test_redis_types, 0);
        reg_raw("", "/test_rpcclient", nullptr, nullptr, nullptr, test_rpcclient, 0);
        reg_raw("", "/test_rpcclientssl", nullptr, nullptr, nullptr, test_rpcclientssl, 0);
        reg_raw("", "/test_rpcserver", nullptr, nullptr, nullptr, test_rpcserver, 0);
        reg_raw("", "/test_rpc_chunkc", nullptr, nullptr, nullptr, test_rpc_chunkc, 0);
        reg_raw("", "/test_rpc_chunks", nullptr, nullptr, nullptr, test_rpc_chunks, 0);
        reg_raw("", "/test_rpc_binary", nullptr, nullptr, nullptr, test_rpc_binary, 0);
        reg_raw("", "/test_rpc_binary_echo", nullptr, nullptr, nullptr, test_rpc_binary_echo, 0);
        reg_raw("", "/test_socket_client", nullptr, nullptr, nullptr, test_socket_client, 0);
        reg_raw("", "/co_sql_commit", nullptr, nullptr, nullptr, test_co_sql_commit, 0);
        reg_raw("", "/co_sql_orm", nullptr, nullptr, nullptr, test_co_sql_orm, 0);
        reg_raw("", "/co_sqlquery", nullptr, nullptr, nullptr, test_co_sqlquery, 0);
        reg_raw("", "/test_websocket_client", nullptr, nullptr, nullptr, test_websocket_client, 0);
        reg_raw("", "/xcxnotify", nullptr, nullptr, nullptr, test_xcxnotify, 0);
        reg_raw("", "/wxpaynotify", nullptr, nullptr, nullptr, test_wxpay_notify, 0);
        reg_raw("", "/testwxpaynative_co", nullptr, nullptr, nullptr, test_wxpay_native_co, 0);
        reg_raw("", "/testwxpayjsapi_co", nullptr, nullptr, nullptr, test_wxpay_jsapi_co, 0);
        reg_raw("", "/testwxpayv2_order_new_co", nullptr, nullptr, nullptr, testwxpayv2_order_new_co, 0);
        reg_raw("", "/testwxpayv2_refund_co", nullptr, nullptr, nullptr, testwxpayv2_refund_co, 0);
        reg_raw("", "/testwxpayv2_refund_query_co", nullptr, nullptr, nullptr, testwxpayv2_refund_query_co, 0);
        reg_raw("", "/testwxpaycertislogin_co", nullptr, nullptr, nullptr, testwxpaycert_islogin_co, 0);
        reg_raw("", "/testwxpaydownloadcert_co", nullptr, testwxpaycert_islogin_co, nullptr, testwxpaydownloadcert_co, 0);
        reg_raw("", "/testwxpayv3native_co", nullptr, nullptr, nullptr, testwxpayv3native_co, 0);
        reg_raw("", "/testwxpayv3jsapi_co", nullptr, nullptr, nullptr, testwxpayv3jsapi_co, 0);
        reg_raw("", "/testwxpayv3_order_new_co", nullptr, nullptr, nullptr, testwxpayv3_order_new_co, 0);
        reg_raw("", "/testwxpayv3_order_query_co", nullptr, nullptr, nullptr, testwxpayv3_order_query_co, 0);
        reg_raw("", "/testwxpayv3_order_status", nullptr, nullptr, nullptr, testwxpayv3_order_status, 0);
        reg_raw("", "/testwxpayv3_refund_co", nullptr, nullptr, nullptr, testwxpayv3_refund_co, 0);
        reg_raw("", "/ordernotify", nullptr, nullptr, nullptr, test_wxpay_v3_notify, 0);
        reg_raw("", "/testcowaitclient21", nullptr, nullptr, nullptr, testhttpclient21_cowait_body, 0);
        reg_raw("", "/testcowaitclient22", nullptr, nullptr, nullptr, testhttpclient22_cowait_body, 0);
        reg_raw("", "/fx/coro", nullptr, nullptr, nullptr, testfxcoro, 0);
        reg_raw("", "/fx/coroa", nullptr, nullptr, nullptr, testfxcoroa, 0);
        reg_raw("", "/fx/corob", nullptr, nullptr, nullptr, testfxcorob, 0);
        reg_raw("", "/fx/coroc", nullptr, nullptr, nullptr, testfxcoroc, 0);
        reg_raw("", "/fx/coro_pre", nullptr, nullptr, nullptr, testfxcoropre, 0);
        reg_raw("", "/fx/mix_coro", nullptr, nullptr, nullptr, testfxmixchain_coro, 0);
        reg_raw("", "/fx/exc/coro_reg", nullptr, nullptr, nullptr, testfxexc_coro_reg, 0);
        reg_raw("", "/fx/exc/coro_pre", nullptr, nullptr, nullptr, testfxexc_coro_pre, 0);
        reg_raw("", "/fx/exc/coro_unknown", nullptr, nullptr, nullptr, testfxexc_coro_unknown, 0);
        reg_raw("", "/fx/exc/reject503_pre", nullptr, nullptr, nullptr, testfxexc_reject503_pre, 0);
        reg_raw("", "/fx/lane/a", nullptr, nullptr, nullptr, testfxlanea, 0);
        reg_raw("", "/fx/lane/rejectonce", nullptr, nullptr, nullptr, testfxlanerejectonce, 0);
        reg_raw("", "/hello", nullptr, nullptr, nullptr, testhello, 0);
        reg_raw("", "/testcosendmaildo", nullptr, nullptr, nullptr, test_co_sendmaildo, 0);
        reg_raw("saas.com", "/testcohandle", nullptr, nullptr, nullptr, saas::test_co_handle, 0);

    }

}

#endif
