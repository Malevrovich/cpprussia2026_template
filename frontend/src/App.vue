<template>
  <div id="app">
    <nav v-if="isLoggedIn" class="navbar">
      <span class="brand">Min Messenger</span>
      <div class="nav-links">
        <router-link to="/messages">Messages</router-link>
        <router-link to="/notifications">Notifications</router-link>
        <router-link to="/files">Files</router-link>
        <router-link to="/reactions">Reactions</router-link>
        <router-link to="/status">Status</router-link>
        <router-link to="/messenger">Messenger</router-link>
        <button @click="logout" class="logout-btn">Logout ({{ currentUser.login }})</button>
      </div>
    </nav>
    <div :class="['container', { 'full-screen': isMessengerRoute }]">
      <router-view />
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { useAuthStore } from './stores/auth'

const route = useRoute()
const router = useRouter()
const authStore = useAuthStore()
const isLoggedIn = computed(() => authStore.isLoggedIn)
const currentUser = computed(() => authStore.currentUser)
const isMessengerRoute = computed(() => route.path === '/messenger')

function logout() {
  authStore.logout()
  router.push('/login')
}
</script>

<style>
* { box-sizing: border-box; margin: 0; padding: 0; }
body { font-family: Arial, sans-serif; background: #f5f5f5; }
#app { min-height: 100vh; }
.navbar { background: #2c3e50; color: white; padding: 1rem; display: flex; justify-content: space-between; align-items: center; }
.brand { font-size: 1.5rem; font-weight: bold; }
.nav-links { display: flex; gap: 1rem; align-items: center; }
.nav-links a { color: white; text-decoration: none; padding: 0.5rem 1rem; border-radius: 4px; }
.nav-links a:hover, .nav-links a.router-link-active { background: #34495e; }
.nav-links a.router-link-active { background: #3498db; }
.logout-btn { background: #e74c3c; color: white; border: none; padding: 0.5rem 1rem; border-radius: 4px; cursor: pointer; }
.logout-btn:hover { background: #c0392b; }
.container { max-width: 1200px; margin: 2rem auto; padding: 0 1rem; }
.container.full-screen { max-width: none; margin: 0; padding: 0; height: calc(100vh - 60px); }
.card { background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); padding: 1.5rem; margin-bottom: 1rem; }
.card h2 { margin-bottom: 1rem; color: #2c3e50; }
.form-group { margin-bottom: 1rem; }
.form-group label { display: block; margin-bottom: 0.5rem; color: #555; }
.form-group input, .form-group select, .form-group textarea { width: 100%; padding: 0.75rem; border: 1px solid #ddd; border-radius: 4px; font-size: 1rem; }
.btn { background: #3498db; color: white; border: none; padding: 0.75rem 1.5rem; border-radius: 4px; cursor: pointer; font-size: 1rem; }
.btn:hover { background: #2980b9; }
.btn:disabled { background: #bdc3c7; cursor: not-allowed; }
.error { color: #e74c3c; background: #fadbd8; padding: 0.75rem; border-radius: 4px; margin-bottom: 1rem; }
.success { color: #27ae60; background: #d5f4e6; padding: 0.75rem; border-radius: 4px; margin-bottom: 1rem; }
table { width: 100%; border-collapse: collapse; }
th, td { padding: 0.75rem; text-align: left; border-bottom: 1px solid #ddd; }
th { background: #f8f9fa; font-weight: bold; }
</style>
