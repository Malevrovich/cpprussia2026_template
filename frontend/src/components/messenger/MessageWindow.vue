<template>
  <div class="message-window flex flex-col h-full bg-gray-50">
    <div class="header p-4 bg-white border-b border-gray-200 flex justify-between items-center">
      <h2 class="text-lg font-semibold text-gray-900">
        {{ chatName }}
      </h2>
      <button
        v-if="isDirectMessage"
        class="text-sm text-blue-500 hover:text-blue-600"
        @click="viewProfile"
      >
        View Profile
      </button>
    </div>
    <div
      ref="messagesContainer"
      class="messages-container flex-1 overflow-y-auto p-4 space-y-4"
    >
      <MessageBubble
        v-for="(msg, index) in currentMessages"
        :key="msg.message_id || msg.id || index"
        :channel-id="props.channelId"
        :message="msg"
      />
    </div>
    <MessageInput :channel-id="props.channelId" @message-sent="scrollToBottom" />
  </div>
</template>

<script setup>
import { ref, computed, watch, nextTick, onUnmounted } from 'vue'
import { useChatsStore } from '@/stores/chats'
import { useMessagesStore } from '@/stores/messages'
import MessageBubble from './MessageBubble.vue'
import MessageInput from './MessageInput.vue'

const props = defineProps({
  channelId: {
    type: [Number, String],
    required: true
  }
})

const chatsStore = useChatsStore()
const messagesStore = useMessagesStore()
const messagesContainer = ref(null)
let pollingTimer = null

const chatName = computed(() => {
  const chat = chatsStore.chats.find(c => c.id === props.channelId)
  return chat?.name || 'Chat'
})

const isDirectMessage = computed(() => {
  const chat = chatsStore.chats.find(c => c.id === props.channelId)
  return chat?.type === 'direct'
})

function viewProfile() {
  console.log('View profile for chat:', props.channelId)
}

const currentMessages = computed(() => {
  return messagesStore.messages[props.channelId] || []
})

function startPolling() {
  stopPolling()
  pollingTimer = setInterval(() => {
    messagesStore.fetchMessages(props.channelId)
  }, 5000)
}

function stopPolling() {
  if (pollingTimer) {
    clearInterval(pollingTimer)
    pollingTimer = null
  }
}

watch(
  () => currentMessages.value.length,
  () => {
    nextTick(() => scrollToBottom())
  }
)

watch(
  () => props.channelId,
  async (newId, oldId) => {
    if (oldId) {
      stopPolling()
    }
    await messagesStore.fetchMessages(newId)
    startPolling()
    nextTick(() => scrollToBottom())
  },
  { immediate: true }
)

onUnmounted(() => {
  stopPolling()
})

function scrollToBottom() {
  if (messagesContainer.value) {
    messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
  }
}
</script>
