<template>
  <div class="card" style="max-width: 400px; margin: 4rem auto;">
    <h2>Login</h2>
    <div v-if="error" class="error">{{ error }}</div>
    <form @submit.prevent="login">
      <div class="form-group">
        <label>Login</label>
        <input v-model="form.login" type="text" required />
      </div>
      <div class="form-group">
        <label>Password</label>
        <input v-model="form.password" type="password" required />
      </div>
      <button type="submit" class="btn" :disabled="loading">Login</button>
    </form>
    <p style="margin-top: 1rem;">
      Don't have an account? <router-link to="/register">Register</router-link>
    </p>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { authApi } from '../api'
import { useAuthStore } from '../stores/auth'

const router = useRouter()
const authStore = useAuthStore()
const form = ref({ login: '', password: '' })
const error = ref('')
const loading = ref(false)

async function login() {
  error.value = ''
  loading.value = true
  try {
    const response = await authApi.login(form.value)
    const user = response.data.current_user
    authStore.setCurrentUser(user)
    router.push('/messages')
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Login failed'
  } finally {
    loading.value = false
  }
}
</script>
