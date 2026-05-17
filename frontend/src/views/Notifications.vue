<template>
  <div class="card">
    <h2>Notifications</h2>

    <!-- Create Notification Form -->
    <div class="card" style="background: #f8f9fa; margin-bottom: 1.5rem;">
      <h3 style="margin-bottom: 1rem; color: #555;">Send a Notification</h3>
      <div class="form-group">
        <label>Channel</label>
        <select v-model="selectedChannel">
          <option v-for="chat in allChats" :key="chat.id" :value="chat.id">
            {{ chat.name }} ({{ chat.type === 'direct' ? 'DM' : 'Channel' }})
          </option>
        </select>
        <div v-if="allChats.length === 0" style="color: #888; font-size: 0.85rem; margin-top: 0.25rem;">
          No channels available. Open the Messenger to load chats.
        </div>
      </div>
      <div class="form-group">
        <label>Message ID</label>
        <input v-model="messageId" type="number" placeholder="Message ID to notify about" />
      </div>
      <div class="form-group">
        <label>Notify User (login)</label>
        <input v-model="notifyUser" type="text" placeholder="User login to notify" />
      </div>
      <button
        @click="handleCreateNotification"
        class="btn"
        :disabled="!messageId || !notifyUser || !selectedChannel || creating"
      >
        {{ creating ? 'Sending…' : 'Send Notification' }}
      </button>
      <div v-if="createError" class="error" style="margin-top: 1rem;">{{ createError }}</div>
      <div v-if="successMsg" class="success" style="margin-top: 1rem;">{{ successMsg }}</div>
    </div>

    <!-- Notifications List -->
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 1rem; flex-wrap: wrap; gap: 0.5rem;">
      <h3>
        Your Notifications
        <span v-if="unreadCount > 0" style="background: #e74c3c; color: white; padding: 0.2rem 0.6rem; border-radius: 12px; font-size: 0.8rem; margin-left: 0.5rem;">
          {{ unreadCount }} unread
        </span>
      </h3>
      <div style="display: flex; gap: 0.5rem;">
        <button
          v-if="unreadCount > 0"
          @click="markAllRead"
          class="btn"
          style="background: #8e44ad; padding: 0.5rem 1rem; font-size: 0.9rem;"
          :disabled="markingAll"
        >
          {{ markingAll ? 'Marking…' : '✓ Mark all read' }}
        </button>
        <button
          @click="loadAllNotifications"
          class="btn"
          style="background: #27ae60; padding: 0.5rem 1rem; font-size: 0.9rem;"
          :disabled="loading"
        >
          {{ loading ? 'Loading…' : '↻ Refresh' }}
        </button>
      </div>
    </div>

    <div v-if="loadError" class="error">{{ loadError }}</div>

    <div v-if="loading && allNotifications.length === 0" style="text-align: center; padding: 2rem; color: #888;">
      Loading notifications…
    </div>

    <div v-else-if="allNotifications.length === 0 && !loading" style="text-align: center; padding: 2rem; color: #888;">
      No notifications yet.
    </div>

    <div v-else>
      <table>
        <thead>
          <tr>
            <th>Status</th>
            <th>Channel</th>
            <th>Message ID</th>
            <th>Action</th>
          </tr>
        </thead>
        <tbody>
          <tr
            v-for="n in allNotifications"
            :key="`${n.channelId}-${n.message_id}`"
            :style="!n.read ? 'background: #eaf4fb;' : ''"
          >
            <td>
              <span v-if="!n.read" style="color: #e74c3c; font-weight: bold;">● New</span>
              <span v-else style="color: #aaa;">✓ Read</span>
            </td>
            <td>{{ getChatName(n.channelId) }}</td>
            <td>#{{ n.message_id }}</td>
            <td>
              <button
                v-if="!n.read"
                @click="markRead(n.channelId, n.message_id)"
                style="background: none; border: 1px solid #3498db; color: #3498db; padding: 0.2rem 0.6rem; border-radius: 4px; cursor: pointer; font-size: 0.8rem;"
              >
                Mark read
              </button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useNotificationsStore } from '../stores/notifications'
import { useChatsStore } from '../stores/chats'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const chatsStore = useChatsStore()
const notificationsStore = useNotificationsStore()

const selectedChannel = ref(null)
const messageId = ref('')
const notifyUser = ref('')
const loading = ref(false)
const creating = ref(false)
const markingAll = ref(false)
const loadError = ref('')
const createError = ref('')
const successMsg = ref('')

// Use chats from the store (includes both channels and DMs)
const allChats = computed(() => chatsStore.chats)

// All notifications flattened from the store (reactive computed)
const allNotifications = computed(() => notificationsStore.allNotifications)

const unreadCount = computed(() => notificationsStore.totalUnreadCount)

function getChatName(channelId) {
  const chat = chatsStore.chats.find(c => String(c.id) === String(channelId))
  return chat?.name || `Channel ${channelId}`
}

async function loadAllNotifications() {
  if (!authStore.currentUser?.token) {
    loadError.value = 'You must be logged in to view notifications.'
    return
  }
  loading.value = true
  loadError.value = ''
  try {
    await notificationsStore.fetchAllNotifications()
  } catch (e) {
    loadError.value = 'Failed to load notifications. Please try again.'
  } finally {
    loading.value = false
  }
}

async function handleCreateNotification() {
  if (!authStore.currentUser?.token) {
    createError.value = 'You must be logged in.'
    return
  }
  if (!selectedChannel.value) {
    createError.value = 'Please select a channel.'
    return
  }
  creating.value = true
  createError.value = ''
  successMsg.value = ''
  try {
    await notificationsStore.createNotification(
      selectedChannel.value,
      parseInt(messageId.value),
      notifyUser.value.trim()
    )
    successMsg.value = `Notification sent to "${notifyUser.value}" for message #${messageId.value}!`
    messageId.value = ''
    notifyUser.value = ''
    setTimeout(() => (successMsg.value = ''), 4000)
    // Refresh list after creating
    await loadAllNotifications()
  } catch (e) {
    createError.value = e?.response?.data?.detail?.message || 'Failed to send notification.'
  } finally {
    creating.value = false
  }
}

async function markRead(channelId, msgId) {
  await notificationsStore.markNotificationRead(channelId, msgId)
}

async function markAllRead() {
  markingAll.value = true
  try {
    // Mark all unread across all channels
    const unread = allNotifications.value.filter(n => !n.read)
    await Promise.allSettled(
      unread.map(n => notificationsStore.markNotificationRead(n.channelId, n.message_id))
    )
  } finally {
    markingAll.value = false
  }
}

onMounted(async () => {
  // Load chats if not already loaded (needed for channel selector)
  if (chatsStore.chats.length === 0) {
    await chatsStore.fetchDirectChats()
  }
  // Set default selected channel
  if (!selectedChannel.value && allChats.value.length > 0) {
    selectedChannel.value = allChats.value[0].id
  }
  await loadAllNotifications()
})
</script>
