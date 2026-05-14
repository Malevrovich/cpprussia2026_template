<template>
  <div class="card">
    <h2>Messages</h2>
    <div class="form-group">
      <label>Channel</label>
      <select v-model="selectedChannel">
        <option :value="1">General</option>
        <option :value="2">Random</option>
      </select>
    </div>
    <div class="form-group">
      <label>Message</label>
      <input v-model="newMessage" type="text" placeholder="Type a message..." @keyup.enter="sendMessage" />
    </div>
    <button @click="sendMessage" class="btn" :disabled="!newMessage">Send</button>
    <button @click="loadMessages" class="btn" style="margin-left: 0.5rem; background: #27ae60;">Refresh</button>
    <div v-if="error" class="error" style="margin-top: 1rem;">{{ error }}</div>
    <div v-if="messages.length" style="margin-top: 1rem;">
      <h3>Messages</h3>
      <table>
        <thead>
          <tr><th>Time</th><th>User</th><th>Message</th></tr>
        </thead>
        <tbody>
          <tr v-for="msg in messages" :key="msg.id">
            <td>{{ msg.timestamp }}</td>
            <td>{{ msg.current_user.name }}</td>
            <td>{{ msg.message }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { messagingApi, notificationsApi } from '../api'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const selectedChannel = ref(1)
const newMessage = ref('')
const messages = ref([])
const error = ref('')

const user = { token: authStore.currentUser.token, login: authStore.currentUser.login, name: authStore.currentUser.name }

async function loadMessages() {
  error.value = ''
  try {
    const response = await messagingApi.getMessages(selectedChannel.value, '2020-01-01T00:00:00Z', '2030-01-01T00:00:00Z')
    messages.value = response.data.messages
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to load messages'
  }
}

async function sendMessage() {
  if (!newMessage.value.trim()) return
  error.value = ''
  try {
    await messagingApi.sendMessage({
      current_user: user,
      channel_id: selectedChannel.value,
      message: newMessage.value,
    })
    newMessage.value = ''
    await loadMessages()
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to send message'
  }
}

onMounted(loadMessages)
</script>
