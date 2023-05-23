var TABSNAV = {
	refreshvport: function () {
		//刷新内空区
		var theight = $(window).height() - $("#myNavbarContent").offset().top - 7;
		if (theight < $("#sidebar").height()) {
			theight = $("#sidebar").height()
		}
		$("#myNavbarContent").height(theight + "px");
		$("#myNavbarContent").children().height(theight + "px");
	},
	addtabs: function (href, text) {
		var idname = href.replace(/[^\w]/ig, '');
		if (document.getElementById(idname)) {
			//如果已经存在则显示和刷新  
			$('#myNavbarTab button#' + idname + '').tab('show');

			return false;
		}
		//删除多于8个标签	
		if ($("#myNavbarTab").children().length > 8) {
			$($("#myNavbarTab").children(":not(.fixmytabs)").first().children().first().attr("button")).remove();
			$("#myNavbarTab").children(":not(.fixmytabs)").first().remove();
		}
		//生成窗体
		$("<li class=\"nav-item\" role =\"presentation\"><button class=\"nav-link\" id=\"" + idname + "\" data-bs-toggle=\"tab\" data-bs-target=\"#" + idname + "-pane\" type=\"button\" role=\"tab\" aria-controls=\"" + idname + "-pane\" aria-selected=\"false\"><i class=\"bi bi-arrow-clockwise\"></i> " + text + " <i class=\"bi bi-x-lg\"></i></button></li>").appendTo("#myNavbarTab");
		$("<div class=\"tab-pane fade\" id=\"" + idname + "-pane\" role=\"tabpanel\" aria-labelledby=\"" + idname + "\" tabindex=\"0\"><iframe title=\"" + href + "\" id=\"" + idname + "-iframe\" width=\"100%\" height=\"100%\" frameborder=\"0\" src=\"" + href + "\"></iframe></div>").appendTo($("#myNavbarTab").next());

		$('#myNavbarTab #' + idname + '').tab('show');
		TABSNAV.addevents(idname);
		TABSNAV.refreshvport();
	},
	addevents: function (isactive) {
		//添加事件
		//点击进修激活tabs
		if (isactive == "") {
			return;
		}
		//remove event
		$("#myNavbarTab button#" + isactive + " i.bi-x-lg").click(function (e) {

			if ($(this).parent().parent().hasClass('fixmytabs')) {
				return false;
			}
			if (confirm("确是否真的关闭")) {
				$("#" + $(this).parent().attr("id") + "-pane").remove();
				$(this).parent().parent().remove();
			}
			return false;
		});
		//remove event
		$("#myNavbarTab button#" + isactive + " i.bi-arrow-clockwise").click(function (e) {
			if ($(this).parent().parent().hasClass('fixmytabs')) {
				return false;
			}
			var idname = "#" + $(this).parent().attr("id") + "-iframe";
			$(idname).attr("src", $(idname).attr("title") + "?" + Math.random());
			return false;
		});
		//add double fix tab
		$("#myNavbarTab button#" + isactive + "").parent().dblclick(function (e) {

			$(this).toggleClass('fixmytabs');
			if ($(this).hasClass('fixmytabs')) {
				$(" <i class=\"bi bi-bag\"></i> ").prependTo($(this).find("button"));
			} else {
				$(this).find(".bi-bag").remove();
			}
			return false;
		});

		//add refresh tab event
		$("#myNavbarTab button#" + isactive + " .fa-refresh").click(function (e) {
			var idname = $(this).parent().attr("href");
			$(idname).children().first().attr("src", $(idname).children().first().attr("src") + "?" + Math.random());
			return false;
		});
	},
	init: function () {
		$("#sidebarMenu a").each(function (e) {

			if ($(this).attr("href") != "#" && $(this).attr("href") != "javascript:") {

				$(this).click(function (e) {
					TABSNAV.addtabs($(this).attr("href"), $(this).text());
					$(".nav-list li").removeClass('active');
					$(this).parent().addClass("active");
					e.preventDefault();

					return false;
				});

			}

		});
		$("#menbarpop a").each(function (e) {

			if ($(this).attr("href") != "#" && $(this).attr("href") != "javascript:") {

				$(this).click(function (e) {
					TABSNAV.addtabs($(this).attr("href"), $(this).text());
					// $(".nav-list li").removeClass('active');
					// $(this).parent().addClass("active");
					e.preventDefault();

					return false;
				});

			}

		});

		//	TABSNAV.addevents('');
		TABSNAV.refreshvport();
	}

};