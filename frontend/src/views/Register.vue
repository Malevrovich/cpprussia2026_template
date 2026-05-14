<template>
  <div class="card" style="max-width: 400px; margin: 4rem auto;">
    <h2>Register</h2>
    <div v-if="error" class="error">{{ error }}</div>
    <div v-if="success" class="success">Registration successful! Redirecting...</div>
    <form @submit.prevent="register">
      <div class="form-group">
        <label>Login</label>
        <input v-model="form.login" type="text" required minlength="3" />
      </div>
      <div class="form-group">
        <label>Name</label>
        <input v-model="form.name" type="text" required />
      </div>
      <div class="form-group">
        <label>Email</label>
        <input v-model="form.email" type="email" required />
      </div>
      <div class="form-group">
        <label>Phone</label>
        <input v-model="form.phone" type="tel" required />
      </div>
      <div class="form-group">
        <label>Password</label>
        <input v-model="form.password" type="password" required minlength="6" />
      </div>
      <button type="submit" class="btn" :disabled="loading">Register</button>
    </form>
    <p style="margin-top: 1rem;">
      Already have an account? <router-link to="/login">Login</router-link>
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
const form = ref({ login: '', name: '', email: '', phone: '', password: '' })
const error = ref('')
const success = ref(false)
const loading = ref(false)

async function register() {
  error.value = ''
  loading.value = true
  try {
    const response = await authApi.register(form.value)
    const user = response.data.current_user
    authStore.setCurrentUser(user)
    success.value = true
    setTimeout(() => router.push('/messages'), 1500)
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Registration failed'
  } finally {
    loading.value = false
  }
}
</script>
