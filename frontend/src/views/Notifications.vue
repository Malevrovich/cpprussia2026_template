<template>
  <div class="card">
    <h2>Notifications</h2>
    <div class="form-group">
      <label>Channel</label>
      <select v-model="selectedChannel">
        <option :value="1">General</option>
        <option :value="2">Random</option>
      </select>
    </div>
    <div class="form-group">
      <label>Message ID</label>
      <input v-model="messageId" type="number" placeholder="Message ID to notify about" />
    </div>
    <div class="form-group">
      <label>Notify User (login)</label>
      <input v-model="notifyUser" type="text" placeholder="User login to notify" />
    </div>
    <button @click="createNotification" class="btn" :disabled="!messageId || !notifyUser">Create Notification</button>
    <button @click="loadNotifications" class="btn" style="margin-left: 0.5rem; background: #27ae60;">Refresh</button>
    <div v-if="error" class="error" style="margin-top: 1rem;">{{ error }}</div>
    <div v-if="successMsg" class="success" style="margin-top: 1rem;">{{ successMsg }}</div>
    <div v-if="notifications.length" style="margin-top: 1rem;">
      <h3>Your Notifications</h3>
      <table>
        <thead>
          <tr><th>Message ID</th><th>Read</th></tr>
        </thead>
        <tbody>
          <tr v-for="n in notifications" :key="n.message_id">
            <td>{{ n.message_id }}</td>
            <td>{{ n.read ? 'Yes' : 'No' }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { notificationsApi } from '../api'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const selectedChannel = ref(1)
const messageId = ref('')
const notifyUser = ref('')
const notifications = ref([])
const error = ref('')
const successMsg = ref('')

const user = { token: authStore.currentUser.token, login: authStore.currentUser.login, name: authStore.currentUser.name }

async function loadNotifications() {
  error.value = ''
  try {
    const response = await notificationsApi.list(selectedChannel.value, user)
    notifications.value = response.data.notifications
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to load notifications'
  }
}

async function createNotification() {
  error.value = ''
  successMsg.value = ''
  try {
    await notificationsApi.create({
      current_user: user,
      channel_id: selectedChannel.value,
      message_id: parseInt(messageId.value),
      other_user_login: notifyUser.value,
    })
    successMsg.value = 'Notification created!'
    setTimeout(() => successMsg.value = '', 3000)
    await loadNotifications()
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to create notification'
  }
}

onMounted(loadNotifications)
</script>
