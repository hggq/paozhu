// store.js
(function() {
    if (typeof Vue === 'undefined') {
        console.error('❌ Vue 未加载，请先引入 vue.global.js');
        return;
    }
    const { reactive } = Vue;

    const userState = reactive({
        name: '管理员',
        role: 'admin',
        token: localStorage.getItem('token') || '',
    });

    window.useUserStore = function() {
        return {
            state: userState,
            setUser(user) {
                userState.name = user.name;
                userState.role = user.role;
                userState.token = user.token;
                localStorage.setItem('token', user.token);
            },
            logout() {
                userState.name = '';
                userState.role = '';
                userState.token = '';
                localStorage.removeItem('token');
            }
        };
    };

    console.log('✅ store.js 加载成功');
})();