<template>
  <div class="card">
    <h2>Reactions</h2>
    <div class="form-group">
      <label>Channel ID</label>
      <input v-model="channelId" type="number" />
    </div>
    <div class="form-group">
      <label>Message ID</label>
      <input v-model="messageId" type="number" />
    </div>
    <div class="form-group">
      <label>Animation</label>
      <select v-model="animation">
        <option value="like">Like</option>
        <option value="dislike">Dislike</option>
        <option value="heart">Heart</option>
        <option value="fire">Fire</option>
        <option value="okay">Okay</option>
        <option value="LOL">LOL</option>
        <option value="smile">Smile</option>
      </select>
    </div>
    <div class="form-group">
      <label>Idempotency Token (random string, min 16 chars)</label>
      <input v-model="token" type="text" />
    </div>
    <button @click="triggerReaction" class="btn">Toggle Reaction</button>
    <button @click="getReactions" class="btn" style="margin-left: 0.5rem; background: #27ae60;">Get Reactions</button>
    <div v-if="error" class="error" style="margin-top: 1rem;">{{ error }}</div>
    <div v-if="result" class="success" style="margin-top: 1rem;">{{ result }}</div>
    <div v-if="reactions.length" style="margin-top: 1rem;">
      <h3>Current Reactions</h3>
      <table>
        <thead>
          <tr><th>Animation</th><th>Count</th><th>Users</th></tr>
        </thead>
        <tbody>
          <tr v-for="r in reactions" :key="r.animation">
            <td>{{ r.animation }}</td>
            <td>{{ r.count }}</td>
            <td>{{ r.user_logins.join(', ') }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { reactionsApi } from '../api'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const channelId = ref(1)
const messageId = ref(1)
const animation = ref('like')
const token = ref('')
const error = ref('')
const result = ref('')
const reactions = ref([])

const user = { token: authStore.currentUser.token, login: authStore.currentUser.login, name: authStore.currentUser.name }

function generateToken() {
  return Math.random().toString(36).substring(2) + Date.now().toString(36)
}

async function triggerReaction() {
  error.value = ''
  result.value = ''
  if (!token.value || token.value.length < 16) {
    token.value = generateToken()
  }
  try {
    const response = await reactionsApi.trigger({
      current_user: user,
      idempotency_token: token.value,
      channel_id: parseInt(channelId.value),
      message_id: parseInt(messageId.value),
      animation: animation.value,
    })
    result.value = `Action: ${response.data.action}, Current reaction: ${response.data.current_user_reaction || 'none'}`
    await getReactions()
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to trigger reaction'
  }
}

async function getReactions() {
  error.value = ''
  try {
    const response = await reactionsApi.get(parseInt(channelId.value), parseInt(messageId.value))
    reactions.value = response.data.reactions
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Failed to get reactions'
  }
}
</script>
