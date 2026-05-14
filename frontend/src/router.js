import { createRouter, createWebHistory } from 'vue-router'

const protectedRoutes = ['/messages', '/notifications', '/files', '/reactions', '/status', '/messenger']

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/', redirect: '/login' },
    { path: '/login', component: () => import('./views/Login.vue') },
    { path: '/register', component: () => import('./views/Register.vue') },
    { path: '/messages', component: () => import('./views/Messages.vue') },
    { path: '/notifications', component: () => import('./views/Notifications.vue') },
    { path: '/files', component: () => import('./views/Files.vue') },
    { path: '/reactions', component: () => import('./views/Reactions.vue') },
    { path: '/status', component: () => import('./views/Status.vue') },
    { path: '/messenger', component: () => import('./views/Messenger.vue') },
  ]
})

// Navigation guard for authentication
router.beforeEach((to, from, next) => {
  const token = localStorage.getItem('token')
  const isProtectedRoute = protectedRoutes.includes(to.path)
  
  if (isProtectedRoute && !token) {
    next('/login')
  } else if ((to.path === '/login' || to.path === '/register') && token) {
    next('/messages')
  } else {
    next()
  }
})

export default router
