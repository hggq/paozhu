// ================================================================
//  菜单配置 MENU_CONFIG 由页面内联脚本提供（后台动态输出），
//  本文件为公共 JS，不在此定义。
//  格式：[{ group: '分组名', items: [{ text, href, icon, html }] }]
// ================================================================

// ================================================================
//  请求工具（全局可用）
// ================================================================
const requestPublic = (url, options = {}) => {
    return fetch(url, {
        ...options,
        headers: { 'Content-Type': 'application/json', ...options.headers }
    });
};

const requestAuth = (url, options = {}) => {
    const token = localStorage.getItem('token');
    return fetch(url, {
        ...options,
        headers: {
            'Content-Type': 'application/json',
            ...(token && { 'Authorization': `Bearer ${token}` }),
            ...options.headers
        }
    });
};

const requestWithCustom = (url, customHeaders = {}, options = {}) => {
    const token = localStorage.getItem('token');
    return fetch(url, {
        ...options,
        headers: {
            'Content-Type': 'application/json',
            ...(token && { 'Authorization': `Bearer ${token}` }),
            ...customHeaders,
            ...options.headers
        }
    });
};

const fetchJSON = async (url, options = {}, withAuth = true) => {
    const fetcher = withAuth ? requestAuth : requestPublic;
    const response = await fetcher(url, options);
    if (!response.ok) throw new Error(`HTTP ${response.status}: ${response.statusText}`);
    return response.json();
};

window.__api = { requestPublic, requestAuth, requestWithCustom, fetchJSON };

// ================================================================
//  2. 模板注册表（JSON 模式）
// ================================================================
const TemplateRegistry = {
    default: (data) => `
        <div class="page-card">
            <div class="page-title">📄 ${data.title || '未注册'} <span class="sub">Fallback</span></div>
            <div class="page-desc">此路径无模板，显示 fallback。</div>
        </div>
    `
};

// ================================================================
//  3. 内容获取函数（向后台请求页面内容）
// ================================================================
function fetchPageContent(path, loadType) {
    const headers = { 'X-Requested-With': 'XMLHttpRequest' };
    if (loadType === 'html') {
        return requestAuth(path, { headers })
            .then(response => {
                if (!response.ok) throw new Error(`HTTP ${response.status}: ${response.statusText}`);
                return response.text();
            })
            .then(html => ({ type: 'html', content: html }));
    }
    return fetchJSON(path, { headers })
        .then(data => ({ type: 'json', content: data }));
}

// ================================================================
//  4. 渲染函数
// ================================================================
function renderPage(path, result) {
    if (result.type === 'html') {
        return result.content;
    } else {
        const fn = TemplateRegistry[path] || TemplateRegistry.default;
        return fn(result.content);
    }
}

// ================================================================
//  5. 脚本执行器（统一挂载 Vue，支持 Pinia）
// ================================================================
function executeScripts(container, path) {
    const scripts = container.querySelectorAll('script');
    let vueComponent = null;

    scripts.forEach(oldScript => {
        if (oldScript.hasAttribute('template') || oldScript.hasAttribute('data-template')) {
            const name = oldScript.getAttribute('data-template-name') || path;
            try {
                const fn = new Function('data', `"use strict"; return (${oldScript.textContent});`);
                TemplateRegistry[name] = fn;
                console.log(`✅ 模板已注册: ${name}`);
                oldScript.remove();
            } catch (e) {
                console.error(`模板注册失败: ${name}`, e);
                oldScript.remove();
            }
            return;
        }

        if (oldScript.hasAttribute('data-vue')) {
            const scriptText = oldScript.textContent.trim();
            try {
                const fn = new Function(`"use strict"; return (${scriptText});`);
                const componentDef = fn();
                if (componentDef && typeof componentDef === 'object') {
                    vueComponent = componentDef;
                    console.log(`✅ 提取 Vue 组件定义成功`);
                } else {
                    console.warn('Vue 组件定义无效，应为对象');
                }
                oldScript.remove();
            } catch (e) {
                console.error('解析 Vue 组件脚本失败:', e);
                oldScript.remove();
            }
            return;
        }

        const newScript = document.createElement('script');
        Array.from(oldScript.attributes).forEach(attr => {
            newScript.setAttribute(attr.name, attr.value);
        });
        if (!oldScript.src) {
            newScript.textContent = oldScript.textContent;
        }
        oldScript.parentNode.replaceChild(newScript, oldScript);
    });

    if (vueComponent) {
        const mountSelector = container.querySelector('[data-vue-mount]')?.getAttribute('data-vue-mount') || '#vue-app';
        const mountEl = container.querySelector(mountSelector);
        if (mountEl) {
            try {
                const app = Vue.createApp(vueComponent);
                // --- Pinia 集成 ---
                if (window.__pinia) {
                    app.use(window.__pinia);
                    console.log('🔌 Pinia 已注入');
                }
                // 注册 Element Plus（如果全局可用）
                if (window.ElementPlus) {
                    app.use(window.ElementPlus);
                }
                // --- 挂载 ---
                const vm = app.mount(mountEl);
                container._vueApp = app;
                mountEl._vueApp = app;
                console.log('✅ Vue 应用已挂载');
            } catch (e) {
                console.error('Vue 挂载失败:', e);
            }
        } else {
            console.warn(`未找到 Vue 挂载点: ${mountSelector}`);
        }
    }
}

// ================================================================
//  6. 卸载 Vue 应用
// ================================================================
function unmountVueApp(container) {
    if (container._vueApp && typeof container._vueApp.unmount === 'function') {
        container._vueApp.unmount();
        console.log('🗑️ Vue 应用已卸载（从容器）');
        delete container._vueApp;
    }
    const mountEl = container.querySelector('#vue-app, [data-vue-mount]');
    if (mountEl && mountEl._vueApp && typeof mountEl._vueApp.unmount === 'function') {
        mountEl._vueApp.unmount();
        delete mountEl._vueApp;
        console.log('🗑️ Vue 应用已卸载（从挂载点）');
    }
}

// ================================================================
//  7. Toast
// ================================================================
let toastTimer = null;
function showToast(msg, duration = 2500) {
    const el = document.getElementById('toast');
    el.textContent = msg;
    el.classList.add('show');
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => el.classList.remove('show'), duration);
}

// ================================================================
//  8. TabManager
// ================================================================
class TabManager {
    constructor(tabBarEl, contentWrapperEl) {
        this.tabBar = tabBarEl;
        this.contentWrapper = contentWrapperEl;
        this.tabs = new Map();
        this.activeKey = null;
        this._idCounter = 0;
        this._render();
    }

    static pathToKey(path) {
        return path.replace(/\//g, '-').replace(/^-/, '') || 'home';
    }

    async open(path, title, loadType = 'json') {
        const key = TabManager.pathToKey(path);
        if (this.tabs.has(key)) {
            const tab = this.tabs.get(key);
            tab.paneEl.innerHTML = `<div style="padding:40px;text-align:center;color:#8a9aaa;">⏳ 刷新中...</div>`;
            this.activate(key);
            try {
                const result = await fetchPageContent(path, tab.loadType);
                tab.paneEl.innerHTML = renderPage(path, result);
                if (result.type === 'html') {
                    executeScripts(tab.paneEl, path);
                }
                showToast(`🔄 已刷新：${title}`);
            } catch (err) {
                tab.paneEl.innerHTML = `<div style="padding:40px;text-align:center;color:#e74c3c;">❌ 刷新失败</div>`;
            }
            history.replaceState({ tabKey: key, path, loadType: tab.loadType }, title, path);
            return key;
        }

        const id = ++this._idCounter;
        const paneEl = document.createElement('div');
        paneEl.className = 'tab-pane';
        paneEl.dataset.key = key;
        paneEl.innerHTML = `<div style="padding:40px;text-align:center;color:#8a9aaa;">⏳ 加载中...</div>`;
        this.contentWrapper.appendChild(paneEl);

        const tabData = { id, key, title, path, paneEl, loadType };
        this.tabs.set(key, tabData);
        this._renderTabs();
        this.activate(key);

        try {
            const result = await fetchPageContent(path, loadType);
            paneEl.innerHTML = renderPage(path, result);
            if (result.type === 'html') {
                executeScripts(paneEl, path);
            }
            showToast(`✅ 加载完成：${title}`);
        } catch (err) {
            paneEl.innerHTML = `<div style="padding:40px;text-align:center;color:#e74c3c;">❌ 加载失败</div>`;
        }

        history.pushState({ tabKey: key, path, loadType }, title, path);
        return key;
    }

    activate(key) {
        if (!this.tabs.has(key)) return;
        if (this.activeKey === key) return;
        document.querySelectorAll('.tab-pane').forEach(el => el.classList.remove('active'));
        const tab = this.tabs.get(key);
        tab.paneEl.classList.add('active');
        this.activeKey = key;
        this._renderTabs();
        const currentPath = window.location.pathname;
        if (tab.path !== currentPath) {
            history.pushState({ tabKey: key, path: tab.path, loadType: tab.loadType }, tab.title, tab.path);
        }
    }

    close(key) {
        if (!this.tabs.has(key)) return;
        if (this.tabs.size <= 1) { showToast('⚠️ 至少保留一个标签页', 2000); return; }
        const tab = this.tabs.get(key);
        const isActive = this.activeKey === key;
        if (tab.paneEl.__cleanup && typeof tab.paneEl.__cleanup === 'function') {
            tab.paneEl.__cleanup();
        }
        unmountVueApp(tab.paneEl);

        tab.paneEl.remove();
        this.tabs.delete(key);
        if (isActive) {
            const keys = Array.from(this.tabs.keys());
            const idx = keys.indexOf(key);
            let nextKey = idx > 0 ? keys[idx - 1] : keys[0];
            if (nextKey) this.activate(nextKey);
            else this.activeKey = null;
        }
        this._renderTabs();
        showToast(`🗑️ 已关闭：${tab.title}`);
    }

    _renderTabs() {
        this.tabBar.innerHTML = '';
        const keys = Array.from(this.tabs.keys());
        if (!keys.length) return;
        keys.forEach(key => {
            const tab = this.tabs.get(key);
            const isActive = key === this.activeKey;
            const div = document.createElement('div');
            div.className = `tab-item${isActive ? ' active' : ''}`;
            div.dataset.key = key;
            const span = document.createElement('span');
            span.textContent = tab.title;
            div.appendChild(span);
            const close = document.createElement('span');
            close.className = 'tab-close';
            close.textContent = '×';
            close.addEventListener('click', (e) => { e.stopPropagation(); this.close(key); });
            div.appendChild(close);
            div.addEventListener('click', () => this.activate(key));
            this.tabBar.appendChild(div);
        });
        if (this.activeKey && !this.tabs.has(this.activeKey)) {
            const firstKey = Array.from(this.tabs.keys())[0];
            if (firstKey) this.activate(firstKey);
        }
    }

    _render() {
        const first = (MENU_CONFIG[0] && MENU_CONFIG[0].items && MENU_CONFIG[0].items[0]) || null;
        const defaultPath = first ? first.href : '/';
        const defaultTitle = first ? first.text : '首页';
        const defaultLoadType = first && first.html ? 'html' : 'json';
        const key = TabManager.pathToKey(defaultPath);
        const paneEl = document.createElement('div');
        paneEl.className = 'tab-pane active';
        paneEl.dataset.key = key;
        fetchPageContent(defaultPath, defaultLoadType).then(result => {
            paneEl.innerHTML = renderPage(defaultPath, result);
            if (result.type === 'html') {
                executeScripts(paneEl, defaultPath);
            }
        });
        this.contentWrapper.appendChild(paneEl);
        const tabData = { id: ++this._idCounter, key, title: defaultTitle, path: defaultPath, paneEl, loadType: defaultLoadType };
        this.tabs.set(key, tabData);
        this.activeKey = key;
        this._renderTabs();
        history.replaceState({ tabKey: key, path: defaultPath, loadType: defaultLoadType }, defaultTitle, defaultPath);
    }
}

// ================================================================
//  9. LinkInterceptor
// ================================================================
class LinkInterceptor {
    constructor(tabManager) {
        this.tabManager = tabManager;
        this._bindEvents();
        this._handlePopState();
    }

    _bindEvents() {
        document.addEventListener('click', (e) => {
            const anchor = e.target.closest('a');
            if (!anchor) return;
            if (anchor.target === '_blank') return;
            const href = anchor.getAttribute('href');
            if (!href || href.startsWith('javascript:') || href.startsWith('#')) return;
            const url = new URL(href, window.location.origin);
            if (url.origin !== window.location.origin) {
                anchor.target = '_blank';
                return;
            }
            e.preventDefault();
            const loadType = anchor.dataset.load || 'html';
            const path = url.pathname + url.search + url.hash;
            const title = anchor.textContent.trim() || path;
            this.tabManager.open(path, title, loadType);
        });
    }

    _handlePopState() {
        window.addEventListener('popstate', (event) => {
            const state = event.state;
            if (state && state.tabKey && state.path) {
                const key = state.tabKey;
                if (this.tabManager.tabs.has(key)) {
                    this.tabManager.activate(key);
                } else {
                    this.tabManager.open(state.path, state.title || state.path, state.loadType || 'html');
                }
            }
        });
    }
}

// ================================================================
//  10. 菜单渲染函数（分组可折叠，状态持久化到 localStorage）
// ================================================================
const MENU_COLLAPSE_KEY = 'menuGroupCollapsed';

function readMenuCollapseMap() {
    try {
        return JSON.parse(localStorage.getItem(MENU_COLLAPSE_KEY)) || {};
    } catch (e) {
        return {};
    }
}

function writeMenuCollapseMap(map) {
    localStorage.setItem(MENU_COLLAPSE_KEY, JSON.stringify(map));
}

function isGroupCollapsed(name, defaultCollapsed) {
    const map = readMenuCollapseMap();
    return Object.prototype.hasOwnProperty.call(map, name) ? !!map[name] : defaultCollapsed;
}

function setGroupCollapsed(name, collapsed) {
    const map = readMenuCollapseMap();
    map[name] = collapsed;
    writeMenuCollapseMap(map);
}

function renderMenu() {
    const container = document.getElementById('menuContainer');
    container.innerHTML = '';
    const currentPath = window.location.pathname;
    MENU_CONFIG.forEach(group => {
        if (!group.items || group.items.length === 0) return;
        // 当前路径命中该分组时强制展开，避免激活项被藏住
        const hasActive = group.items.some(item => item.href === currentPath);
        const collapsed = hasActive ? false : isGroupCollapsed(group.group, group.collapsed === true);

        const li = document.createElement('li');
        li.className = 'menu-group' + (collapsed ? ' collapsed' : '');

        const title = document.createElement('div');
        title.className = 'group-title';
        // 分组图标：支持 emoji 或 bootstrap icon 类名（bi-xxx），缺省用 📂
        const groupIcon = group.icon || '📂';
        const iconHtml = /^bi-/.test(groupIcon)
            ? `<i class="bi ${groupIcon} group-icon"></i>`
            : `<span class="group-icon">${groupIcon}</span>`;
        title.innerHTML = `<span class="group-label">${iconHtml}<span>${group.group}</span></span><i class="bi bi-chevron-down group-arrow"></i>`;
        title.addEventListener('click', () => {
            const nowCollapsed = li.classList.toggle('collapsed');
            setGroupCollapsed(group.group, nowCollapsed);
        });
        li.appendChild(title);

        const itemsWrap = document.createElement('div');
        itemsWrap.className = 'group-items';
        const itemsInner = document.createElement('div');
        itemsInner.className = 'group-items-inner';
        group.items.forEach(item => {
            const a = document.createElement('a');
            a.className = 'menu-item';
            a.href = item.href;
            a.dataset.load = item.html ? 'html' : 'json';
            a.innerHTML = `<span>${item.icon || '📄'}</span><span>${item.text}</span>`;
            if (currentPath === item.href) a.classList.add('active');
            itemsInner.appendChild(a);
        });
        itemsWrap.appendChild(itemsInner);
        li.appendChild(itemsWrap);
        container.appendChild(li);
    });
}

window.renderMenu = renderMenu;

// ================================================================
//  11. 初始化
// ================================================================
document.addEventListener('DOMContentLoaded', () => {
    renderMenu();

    const tabBar = document.getElementById('tabBar');
    const contentWrapper = document.getElementById('tabContentWrapper');
    const tabManager = new TabManager(tabBar, contentWrapper);
    const interceptor = new LinkInterceptor(tabManager);
    window.__app = { tabManager, interceptor };

    const sidebar = document.getElementById('sidebar');
    const backdrop = document.getElementById('sidebarBackdrop');
    const btnSidebarToggle = document.getElementById('btnSidebarToggle');
    const openSidebar = () => { sidebar.classList.add('open'); backdrop.classList.add('show'); };
    const closeSidebar = () => { sidebar.classList.remove('open'); backdrop.classList.remove('show'); };
    if (btnSidebarToggle) {
        btnSidebarToggle.addEventListener('click', () => {
            sidebar.classList.contains('open') ? closeSidebar() : openSidebar();
        });
        backdrop.addEventListener('click', closeSidebar);
        window.addEventListener('resize', () => { if (window.innerWidth > 768) closeSidebar(); });
    }

    const originalActivate = tabManager.activate.bind(tabManager);
    tabManager.activate = function(key) {
        originalActivate(key);
        closeSidebar();
        const tab = this.tabs.get(key);
        if (tab) {
            document.querySelectorAll('.menu-item').forEach(el => el.classList.remove('active'));
            document.querySelectorAll('.menu-item').forEach(el => {
                if (el.getAttribute('href') === tab.path) el.classList.add('active');
            });
        }
    };
    if (tabManager.activeKey) tabManager.activate(tabManager.activeKey);

    document.getElementById('btnLogout').addEventListener('click', () => {
        showToast('👋 已退出', 2000);
    });

    console.log('✅ 系统启动，支持自定义添加菜单');
});