<template>
  <div class="messenger-layout flex h-screen w-full overflow-hidden">
    <ChatList 
      @chat-selected="handleChatSelected"
      @start-direct-chat="handleStartDirectChat"
    />
    <div class="message-area flex-1">
      <MessageWindow
        v-if="chatStore.selectedChatId"
        :key="chatStore.selectedChatId"
        :channel-id="chatStore.selectedChatId"
      />
      <div v-else class="flex flex-col items-center justify-center h-full text-gray-500">
        <p class="text-lg mb-4">Select a chat to start messaging</p>
        <p class="text-sm">or search for a user to start a direct message</p>
      </div>
    </div>
    
    <!-- Notifications Panel Toggle -->
    <button
      class="notifications-toggle fixed top-20 right-4 z-50 p-3 bg-white rounded-full shadow-lg hover:bg-gray-100 transition-colors"
      :class="{ 'bg-blue-100': showNotifications }"
      @click="toggleNotifications"
    >
      🔔
      <span
        v-if="notificationsStore.totalUnreadCount > 0"
        class="absolute -top-1 -right-1 bg-red-500 text-white text-xs rounded-full w-5 h-5 flex items-center justify-center"
      >
        {{ notificationsStore.totalUnreadCount }}
      </span>
    </button>
    
    <!-- Notifications Panel -->
    <Transition name="slide">
      <div
        v-if="showNotifications"
        class="notifications-panel fixed top-20 right-4 w-80 max-h-96 bg-white rounded-lg shadow-xl z-50 overflow-hidden"
      >
        <div class="p-4 border-b border-gray-200 flex justify-between items-center bg-gray-50">
          <h3 class="font-semibold">
            Notifications
            <span v-if="notificationsStore.totalUnreadCount > 0" class="ml-2 text-xs bg-red-500 text-white rounded-full px-2 py-0.5">
              {{ notificationsStore.totalUnreadCount }} new
            </span>
          </h3>
          <button @click="showNotifications = false" class="text-gray-500 hover:text-gray-700">✕</button>
        </div>
        <div class="max-h-80 overflow-y-auto">
          <div
            v-if="notificationsStore.allNotifications.length === 0"
            class="p-4 text-center text-gray-500 text-sm"
          >
            No notifications
          </div>
          <div
            v-for="notification in notificationsStore.allNotifications"
            :key="`${notification.channelId}-${notification.message_id}`"
            class="p-3 border-b border-gray-100 flex items-start gap-2"
            :class="{ 'bg-blue-50': !notification.read }"
          >
            <span class="text-lg">{{ notification.read ? '📭' : '📩' }}</span>
            <div class="flex-1 min-w-0">
              <div class="text-sm font-medium text-gray-800 truncate">
                Message #{{ notification.message_id }}
              </div>
              <div class="text-xs text-gray-500">
                Channel: {{ getChatName(notification.channelId) }}
              </div>
              <span
                v-if="!notification.read"
                class="inline-block mt-1 text-xs bg-red-100 text-red-600 rounded px-1.5 py-0.5 font-semibold"
              >
                New
              </span>
              <span v-else class="inline-block mt-1 text-xs text-gray-400">Read</span>
            </div>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useChatsStore } from '@/stores/chats'
import { useNotificationsStore } from '@/stores/notifications'
import ChatList from './ChatList.vue'
import MessageWindow from './MessageWindow.vue'

const chatStore = useChatsStore()
const notificationsStore = useNotificationsStore()
const showNotifications = ref(false)

function getChatName(channelId) {
  const chat = chatStore.chats.find(c => String(c.id) === String(channelId))
  return chat?.name || `#${channelId}`
}

async function toggleNotifications() {
  showNotifications.value = !showNotifications.value
  if (showNotifications.value) {
    // Refresh all notifications when opening the panel
    await notificationsStore.fetchAllNotifications()
    // Auto-mark current channel's notifications as read
    if (chatStore.selectedChatId) {
      notificationsStore.markAllReadForChannel(chatStore.selectedChatId)
    }
  }
}

function handleChatSelected(chatId) {
  console.log('Chat selected:', chatId)
  // Fetch notifications for the newly selected channel immediately
  notificationsStore.fetchNotifications(chatId)
}

function handleStartDirectChat(login) {
  console.log('Start direct chat with:', login)
}

onMounted(() => {
  // Start polling all channels for notifications
  notificationsStore.startPolling()
})

onUnmounted(() => {
  notificationsStore.stopPolling()
})
</script>

<style scoped>
.slide-enter-active,
.slide-leave-active {
  transition: transform 0.2s ease, opacity 0.2s ease;
}
.slide-enter-from,
.slide-leave-to {
  transform: translateX(100%);
  opacity: 0;
}
</style>
