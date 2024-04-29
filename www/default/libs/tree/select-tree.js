/**
 * License:MIT
 * 下拉树插件
 * selectTree
 * author:lyt
 */
(function ($, window, document, undefined) {
    var selectTreePlugin = 'selectTree',
        defaults = {
            expandAll: false,//是否全部展开，如果为tree，expandLevel不生效
            expandLevel: 0,//展开层级
            changeCallBack: null//change回调
        };
    var SelectTree = function (element, options) {
        this.element = element;
        this.original = $(element);//元select组件
        this.options = $.extend({}, defaults, options);
        this.data = [];//数据集合
        this.treeData = [];//树结构数据集合
        this.container = null;//构造容器
        this.selectPanel = null;//选择面板
        this.selectData = new Array();//选中数据集合
        this.multiple = this.checkMultipleMode();//是否多选模式
        this.placeholder = this.original.attr("placeholder") ? this.original.attr("placeholder") : '请选择';//默认提示内容

        this.init();
        this.initDom();
        this.initPanelEvent();
        this.expandDom();
    }
    SelectTree.prototype = {
        //初始化树数据
        init: function () {
            var _this = this;
            _this.data = _this.loadData();
            _this.treeData = _this.loadTreeData(_this.data);
        }, loadData: function () {
            var _this = this;
            var data = [];
            var options = _this.original.find('option');
            var selectValues = [];
            if (_this.multiple) {//多选
                selectValues = _this.original.val();
            } else {
                selectValues.push(this.original.val());
            }
            if (options && options.length > 0) {
                var valueMap = new Map();
                $.each(options, function () {
                    var option = $(this);
                    var text = option.text();
                    var value = option.val();
                    var pid = option.data("pid");
                    if(valueMap.has(value)) {
                        return true;
                    } else {
                        valueMap.set(value);
                    }
                    var optionData = {};
                    if (text && value) {
                        optionData.text = text;
                        optionData.value = value;
                        optionData.pid = pid;
                        optionData.isDisabled = false;
                        if (option.attr("disabled") == 'disabled') {
                            optionData.isDisabled = true;
                        }
                        if (selectValues && selectValues.length > 0) {
                            if ($.inArray(optionData.value, selectValues) >= 0) {
                                var selectObj = {
                                    value: optionData.value,
                                    text: optionData.text
                                }
                                _this.selectData.push(selectObj);
                            }
                        }
                        data.push(optionData);
                    }
                });
            }
            return data;
        }, loadTreeData: function (data) {
            var rootNode = [];
            $.each(data, function () {
                var pid = this.pid;
                var hasChild = false;
                $.each(data, function () {
                    if (pid == this.value) {
                        hasChild = true;
                    }
                });
                if (!hasChild) {
                    rootNode.push(this);
                }
            });
            for (var i = 0; i < rootNode.length; i++) {
                rootNode[i].level = 0;
                rootNode[i].children = getTree(data, rootNode[i].value, 0);
            }
            return rootNode;
        }, initDom: function () {
            var _this = this;
            var container = $('<div tabindex="0"  class="select-tree-container"></div>');
            _this.selectPanel = $('<div class="select-panel"></div>');
            var selectValueBox = $('<div class="select-value"></div>');
            _this.selectPanel.append(selectValueBox);
            _this.setSelectedValue();
            _this.setSelectPanelText();

            container.append('<span class="select-tree-arrow"></span>');
            container.append(_this.selectPanel);
            container.append(_this.initTreeDom());
            _this.original.after(container);
            _this.original.hide();
            _this.container = container;
        },
        initTreeDom: function () {//组装树结构
            var treePanel = $('<div class="tree-panel"></div>');
            var paddingValue = 10;
            treePanel.append(this.assembleTreeDom(this.treeData, paddingValue));
            treePanel.on('click', function (e) {
                e.stopPropagation();
            });
            return treePanel;
        },
        initPanelEvent: function () {
            var _this = this;
            _this.selectPanel.on('click', function (e) {

                //其他的容器处理
                if (_this.container.hasClass('open')) {
                    _this.container.removeClass("open");
                    $(".select-tree-container").removeClass('open');
                } else {
                    $(".select-tree-container").removeClass('open');
                    _this.container.addClass("open");
                }
                e.stopPropagation();
            });
            $(document).on('click', function () {
                _this.container.removeClass("open");
            });
        }, assembleTreeDom: function (nodeArr, paddingValue) {
            var _this = this;
            var nextPaddingValue = paddingValue + 20;

            var ulDom = '';
            if (nodeArr && nodeArr.length > 0) {
                ulDom = $('<ul class="tree-ul"></ul>');
                for (var i = 0; i < nodeArr.length; i++) {
                    var liData = nodeArr[i];
                    var liDom = $('<li></li>');
                    var optionDom = $('<div class="tree-option" style="padding-left:' + paddingValue + 'px;" ></div>');
                    var arrowBox = $('<span class="tree-option-arrow"></span>');
                    var optionValue = $('<span class="tree-option-value" data-id="' + liData.value + '">' + liData.text + '</span>');
                    if (liData.isDisabled) {
                        optionDom.addClass("disabled");
                    }
                    optionDom.append(arrowBox);
                    optionDom.append(optionValue);
                    liDom.append(optionDom);
                    if (!liData.isDisabled) {
                        optionDom.on('click', function (e) {
                            var currentOptionText = $(this).find('.tree-option-value').text();
                            var currentOptionId = $(this).find('.tree-option-value').data('id');
                            var selectOptionObj = {};
                            selectOptionObj.value = currentOptionId + '';
                            selectOptionObj.text = currentOptionText;

                            if (_this.multiple) {
                                if ($(this).hasClass('selected')) {//取消选中操作
                                    $(this).removeClass('selected');
                                    var newSelectArray = new Array();
                                    if (_this.selectData && _this.selectData.length > 0) {
                                        $.each(_this.selectData, function (i, item) {
                                            if (currentOptionId != item.value) {
                                                newSelectArray.push(item);
                                            }
                                        });
                                    }
                                    _this.selectData = newSelectArray;

                                } else {//选中操作
                                    $(this).addClass('selected');
                                    _this.selectData.push(selectOptionObj);
                                }
                                _this.setSelectedValue();
                                _this.setSelectPanelText();

                            } else {
                                _this.container.find('.tree-option').removeClass('selected');
                                $(this).addClass('selected');
                                _this.selectData = new Array();//单选模式，直接清空选中对象数组，添加新选择内容
                                _this.selectData.push(selectOptionObj);
                                _this.setSelectedValue();
                                _this.setSelectPanelText();
                                _this.container.removeClass('open');
                            }
                            if (_this.options.changeCallBack != null && typeof _this.options.changeCallBack == 'function') {
                                _this.options.changeCallBack(_this.selectData);
                            }

                            e.stopPropagation();
                        });
                    }
                    if (liData.children && liData.children.length > 0) {
                        arrowBox.append("<b></b>");
                        arrowBox.addClass('open');
                        arrowBox.on('click', function (e) {
                            $(this).toggleClass('open');
                            var childTree = $(this).closest('li').children('.tree-ul');
                            if ($(this).hasClass('open')) {
                                childTree.slideDown('fast');
                            } else {
                                childTree.slideUp('fast');
                            }
                            e.stopPropagation();
                        });
                        liDom.append(_this.assembleTreeDom(liData.children, nextPaddingValue));
                    }
                    ulDom.append(liDom);
                    if (!_this.options.expandAll) {//不是全部展开
                        var showLevel = _this.options.expandLevel;
                        var currentLevel = liData.level;
                        if (currentLevel > showLevel) {
                            arrowBox.removeClass('open');
                            ulDom.hide();
                        } else if (currentLevel == showLevel) {
                            arrowBox.removeClass('open');
                        }
                    }
                    if (_this.selectData && _this.selectData != null && _this.selectData.length > 0) {

                        $.each(_this.selectData, function (i, item) {
                            if (liData.value == item.value) {
                                optionDom.addClass('selected');
                                return false;
                            }
                        });

                    }
                }
                return ulDom;
            }

        }, expandDom: function () {
            var _this = this;
            var selectDom = _this.container.find(".tree-option.selected");
            if (selectDom && selectDom.length > 0) {
                var parentsDom = selectDom.parents('ul');
                parentsDom.show();
                parentsDom.siblings('.tree-option').find('.tree-option-arrow').addClass('open');
            }
        }, checkMultipleMode: function () {
            if (this.original.attr("multiple") && this.original.attr("multiple") == 'multiple') {
                return true;
            }
            return false;
        }, setSelectPanelText() {
            var _this = this;
            var selectPanel = _this.selectPanel.find('.select-value');
            selectPanel.empty();
            if (_this.selectData && _this.selectData.length > 0) {
                $.each(_this.selectData, function (index, item) {
                    if (_this.multiple) {//多选模式
                        var spanItem = $('<span class="select-item">' + item.text + '</span>');
                        var removeItem = $('<i>×</i>');
                        spanItem.append(removeItem);
                        selectPanel.append(spanItem);
                        removeItem.click(function (e) {
                            _this.unSelectedItem(item);
                            spanItem.remove();
                            e.stopPropagation();
                        });
                    } else {
                        selectPanel.html(item.text);
                    }
                });
            } else {
                selectPanel.html(_this.placeholder);
            }
        }, setSelectedValue() {
            var _this = this;
            _this.original.find('option').each(function () {
                var _option = this;
                $(_option).prop('selected', false);
                var optionValue = $(this).val();
                if (_this.selectData && _this.selectData.length > 0) {
                    $.each(_this.selectData, function (i, item) {
                        if (optionValue == item.value) {
                            item.text = $(_option).text();
                            $(_option).prop('selected', true);
                            return false;
                        }
                    });
                }
            });
        }, unSelectedItem(obj) {
            var newSelectObj = [];
            if (this.selectData && this.selectData.length > 0) {
                $.each(this.selectData, function (i, item) {
                    if (obj.value != item.value) {
                        newSelectObj.push(item);
                    }
                });
            }
            this.selectData = newSelectObj;
            this.setSelectedValue();
            if (this.options.changeCallBack != null && typeof this.options.changeCallBack == 'function') {
                this.options.changeCallBack(this.selectData);
            }
            this.selectData = [];
            this.container.remove();
            this.init();
            this.initDom();
            this.initPanelEvent();
            this.expandDom();
        }
    };

    SelectTree.prototype.selectOption = function (values) {
        var _this = this;
        if (values.constructor == Array) {
            _this.selectData = [];
            $.each(values, function () {
                var obj = {};
                obj.value = this;
                _this.selectData.push(obj);
            })
        } else {
            _this.selectData = [{
                value: values + ''
            }]
        }
        this.setSelectedValue();
        if (this.options.changeCallBack != null && typeof this.options.changeCallBack == 'function') {
            this.options.changeCallBack(this.selectData);
        }
        this.selectData = [];
        this.container.remove();
        this.init();
        this.initDom();
        this.initPanelEvent();
        this.expandDom();
    }
    SelectTree.prototype.clearSelect = function () {
        this.selectData = new Array();
        this.setSelectedValue();
        if (this.options.changeCallBack != null && typeof this.options.changeCallBack == 'function') {
            this.options.changeCallBack(this.selectData);
        }

        this.container.remove();
        this.init();
        this.initDom();
        this.initPanelEvent();
        this.expandDom();
    }
    SelectTree.prototype.expandAll = function () {
        this.container.find(".tree-ul").show();
    }
    SelectTree.prototype.collapseAll = function () {
        this.container.find(".tree-ul .tree-ul").hide();
    }
    SelectTree.prototype.getSelectInfo = function () {
        return this.selectData;
    }

    function getTree(treeData, pid, level) {
        level = level + 1;
        var treeArr = [];
        for (var i = 0; i < treeData.length; i++) {
            var node = treeData[i];
            if (node.pid == pid) {
                node.level = level;
                node.children = getTree(treeData, node.value, level);
                treeArr.push(node);
            }
        }
        return treeArr;
    }


    $.fn[selectTreePlugin] = function (options) {
        var stArray = [];
        this.each(function () {
            if (!$.data(this, 'plugin_' + selectTreePlugin)) {
                $.data(this, 'plugin_' + selectTreePlugin, new SelectTree(this, options));
                stArray.push($(this).data()['plugin_' + selectTreePlugin]);
            }
        });
        if (this.length === 1) {
            return stArray[0];
        } else {
            return stArray;
        }
    }

})(jQuery, window, document);