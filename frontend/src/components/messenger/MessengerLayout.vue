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
      <span v-if="notificationsStore.getTotalUnreadCount() > 0" class="absolute -top-1 -right-1 bg-red-500 text-white text-xs rounded-full w-5 h-5 flex items-center justify-center">
        {{ notificationsStore.getTotalUnreadCount() }}
      </span>
    </button>
    
    <!-- Notifications Panel -->
    <Transition name="slide">
      <div
        v-if="showNotifications"
        class="notifications-panel fixed top-20 right-4 w-80 max-h-96 bg-white rounded-lg shadow-xl z-50 overflow-hidden"
      >
        <div class="p-4 border-b border-gray-200 flex justify-between items-center bg-gray-50">
          <h3 class="font-semibold">Notifications</h3>
          <button @click="showNotifications = false" class="text-gray-500 hover:text-gray-700">✕</button>
        </div>
        <div class="max-h-80 overflow-y-auto">
          <div
            v-for="(notifications, channelId) in notificationsStore.notificationsByChannel"
            :key="channelId"
            class="p-3 border-b border-gray-100"
          >
            <h4 class="text-sm font-semibold text-gray-600 mb-2">Channel {{ channelId }}</h4>
            <div
              v-for="notification in notifications"
              :key="notification.message_id"
              class="py-1 text-sm"
              :class="{ 'font-semibold': !notification.read }"
            >
              📩 Message #{{ notification.message_id }}
              <span v-if="notification.read" class="text-gray-400 text-xs ml-2">Read</span>
              <span v-else class="text-red-500 text-xs ml-2">New</span>
            </div>
          </div>
          <div v-if="Object.keys(notificationsStore.notificationsByChannel).length === 0" class="p-4 text-center text-gray-500 text-sm">
            No notifications
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue'
import { useChatsStore } from '@/stores/chats'
import { useNotificationsStore } from '@/stores/notifications'
import ChatList from './ChatList.vue'
import MessageWindow from './MessageWindow.vue'

const chatStore = useChatsStore()
const notificationsStore = useNotificationsStore()
const showNotifications = ref(false)

function toggleNotifications() {
  showNotifications.value = !showNotifications.value
  if (showNotifications.value && chatStore.selectedChatId) {
    notificationsStore.fetchNotifications(chatStore.selectedChatId)
  }
}

function handleChatSelected(chatId) {
  console.log('Chat selected:', chatId)
  notificationsStore.fetchNotifications(chatId)
  notificationsStore.startPolling(chatId)
}

function handleStartDirectChat(login) {
  console.log('Start direct chat with:', login)
}

onMounted(() => {
  if (chatStore.selectedChatId) {
    notificationsStore.fetchNotifications(chatStore.selectedChatId)
    notificationsStore.startPolling(chatStore.selectedChatId)
  }
})

onUnmounted(() => {
  notificationsStore.stopPolling()
})

watch(() => chatStore.selectedChatId, (newChannelId, oldChannelId) => {
  if (oldChannelId) {
    notificationsStore.stopPolling()
  }
  if (newChannelId) {
    notificationsStore.fetchNotifications(newChannelId)
    notificationsStore.startPolling(newChannelId)
  }
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
