import { defineStore } from 'pinia'
import { computed, reactive } from 'vue'
import { authApi } from '@/api'

export const useAuthStore = defineStore('auth', () => {
  const state = reactive({
    token: localStorage.getItem('token') || null,
    login: localStorage.getItem('login') || null,
    name: localStorage.getItem('name') || null,
  })

  const isLoggedIn = computed(() => !!state.token)

  const currentUser = computed(() => ({
    token: state.token,
    login: state.login,
    name: state.name,
  }))

  function setCurrentUser(user) {
    state.token = user.token
    state.login = user.login
    state.name = user.name
    localStorage.setItem('token', user.token)
    localStorage.setItem('login', user.login)
    localStorage.setItem('name', user.name)
  }

  function clearCurrentUser() {
    state.token = null
    state.login = null
    state.name = null
    localStorage.removeItem('token')
    localStorage.removeItem('login')
    localStorage.removeItem('name')
  }

  async function login(user) {
    const response = await authApi.login(user)
    setCurrentUser(response.data)
    return response.data
  }

  async function logout() {
    clearCurrentUser()
  }

  async function register(user) {
    const response = await authApi.register(user)
    setCurrentUser(response.data)
    return response.data
  }

  return {
    isLoggedIn,
    currentUser,
    login,
    logout,
    register,
    setCurrentUser,
  }
})
