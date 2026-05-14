<template>
  <div class="card">
    <h2>Status</h2>
    <div class="form-group">
      <label>Status Type</label>
      <select v-model="statusType">
        <option value="online">Online</option>
        <option value="away">Away</option>
        <option value="busy">Busy</option>
        <option value="offline">Offline</option>
      </select>
    </div>
    <div class="form-group">
      <label>Status Message</label>
      <input v-model="statusMessage" type="text" placeholder="What are you doing?" />
    </div>
    <div class="form-group">
      <label>Visibility</label>
      <select v-model="visibility">
        <option value="public">Public</option>
        <option value="private">Private</option>
      </select>
    </div>
    <button @click="updateStatus" class="btn">Update Status</button>
    <button @click="loadStatus" class="btn" style="margin-left: 0.5rem; background: #27ae60;">Refresh</button>
    <div v-if="error" class="error" style="margin-top: 1rem;">{{ error }}</div>
    <div v-if="successMsg" class="success" style="margin-top: 1rem;">{{ successMsg }}</div>
    <div v-if="myStatus" style="margin-top: 1rem;">
      <h3>Your Status</h3>
      <p><strong>Type:</strong> {{ myStatus.status_type }}</p>
      <p><strong>Message:</strong> {{ myStatus.status_message }}</p>
      <p><strong>Visibility:</strong> {{ myStatus.visibility }}</p>
      <p><strong>Updated:</strong> {{ myStatus.updated_at }}</p>
    </div>
    <div class="form-group" style="margin-top: 2rem; border-top: 1px solid #ddd; padding-top: 1rem;">
      <h3>Check Another User's Status</h3>
      <div class="form-group">
        <label>User Login</label>
        <input v-model="lookupLogin" type="text" placeholder="Enter user login" />
      </div>
      <button @click="lookupStatus" class="btn" :disabled="!lookupLogin">Lookup</button>
      <div v-if="lookupResult" style="margin-top: 1rem;">
        <p><strong>Status:</strong> {{ lookupResult.status_type }}</p>
        <p><strong>Message:</strong> {{ lookupResult.status_message }}</p>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { statusApi } from '../api'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const statusType = ref('online')
const statusMessage = ref('')
const visibility = ref('public')
const myStatus = ref(null)
const error = ref('')
const successMsg = ref('')
const lookupLogin = ref('')
const lookupResult = ref(null)

const user = { token: authStore.currentUser.token, login: authStore.currentUser.login, name: authStore.currentUser.name }

async function updateStatus() {
  error.value = ''
  successMsg.value = ''
  try {
    const response = await statusApi.update({
      current_user: user,
      status: {
        status_type: statusType.value,
        status_message: statusMessage.value,
        visibility: visibility.value,
      },
    })
    successMsg.value = `Status updated! Expires: ${response.data.expires_at}`
    await loadStatus()
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to update status'
  }
}

async function loadStatus() {
  error.value = ''
  try {
    const response = await statusApi.get(user.login, user)
    myStatus.value = response.data.status
  } catch (e) {
    myStatus.value = null
  }
}

async function lookupStatus() {
  error.value = ''
  lookupResult.value = null
  try {
    const response = await statusApi.get(lookupLogin.value, user)
    lookupResult.value = response.data.status
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Status not found or private'
  }
}

loadStatus()
</script>
