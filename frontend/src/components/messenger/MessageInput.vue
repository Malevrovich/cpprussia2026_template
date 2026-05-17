<template>
  <div class="message-input border-t border-gray-200 p-3 bg-white">
    <!-- Mention panel (collapsible) -->
    <Transition name="mention-slide">
      <div v-if="showMention" class="mention-panel mb-2 p-2 bg-yellow-50 border border-yellow-200 rounded-lg flex items-center gap-2">
        <span class="text-sm text-yellow-700 whitespace-nowrap">🔔 Notify:</span>
        <input
          v-model="mentionLogin"
          type="text"
          placeholder="user login"
          class="flex-1 text-sm border border-yellow-300 rounded px-2 py-1 focus:outline-none focus:ring-1 focus:ring-yellow-400 bg-white"
          @keydown.enter.prevent="handleSend"
        />
        <button
          @click="closeMention"
          class="text-yellow-500 hover:text-yellow-700 text-xs"
          title="Remove mention"
        >✕</button>
      </div>
    </Transition>

    <div class="flex items-end gap-2">
      <!-- Mention toggle button -->
      <button
        class="p-2 rounded-lg transition-colors flex-shrink-0"
        :class="showMention
          ? 'text-yellow-600 bg-yellow-100 hover:bg-yellow-200'
          : 'text-gray-500 hover:text-gray-600 hover:bg-gray-100'"
        @click="toggleMention"
        title="Notify a user about this message"
      >
        🔔
      </button>
      <textarea
        ref="textareaRef"
        v-model="messageText"
        class="flex-1 resize-none border border-gray-300 rounded-lg px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        placeholder="Type a message..."
        rows="1"
        @keydown.enter.exact.prevent="handleSend"
        @input="autoResize"
      ></textarea>
      <button
        class="send-btn px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition-colors disabled:opacity-50 disabled:cursor-not-allowed flex-shrink-0"
        :disabled="!canSend"
        @click="handleSend"
      >
        Send
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, nextTick } from 'vue'
import { useMessagesStore } from '@/stores/messages'
import { useAuthStore } from '@/stores/auth'
import { useNotificationsStore } from '@/stores/notifications'

const messagesStore = useMessagesStore()
const authStore = useAuthStore()
const notificationsStore = useNotificationsStore()

const messageText = ref('')
const textareaRef = ref(null)

// Mention state
const showMention = ref(false)
const mentionLogin = ref('')

const props = defineProps({
  channelId: {
    type: [Number, String],
    required: true
  }
})

const emit = defineEmits(['message-sent'])

const canSend = computed(() => !!messageText.value.trim())

function toggleMention() {
  showMention.value = !showMention.value
  if (!showMention.value) {
    mentionLogin.value = ''
  }
}

function closeMention() {
  showMention.value = false
  mentionLogin.value = ''
}

async function handleSend() {
  if (!canSend.value) return

  const text = messageText.value.trim()
  if (!text) return

  // Send the message and get back the sent message (with server-assigned message_id)
  const sentMessage = await messagesStore.sendMessage(
    props.channelId,
    text,
    authStore.currentUser.login
  )

  // If a mention login was provided, send a notification to that user
  const loginToNotify = mentionLogin.value.trim()
  if (loginToNotify && sentMessage?.message_id) {
    // Fire-and-forget: don't block the UI on notification delivery
    notificationsStore.createNotification(
      props.channelId,
      sentMessage.message_id,
      loginToNotify
    ).catch(err => console.error('Failed to send mention notification:', err))
  }

  messageText.value = ''
  // Keep mention panel open but clear the login for next message
  mentionLogin.value = ''
  await nextTick()
  autoResize()
  emit('message-sent')
}

function autoResize() {
  if (textareaRef.value) {
    textareaRef.value.style.height = 'auto'
    textareaRef.value.style.height = textareaRef.value.scrollHeight + 'px'
  }
}
</script>

<style scoped>
.mention-slide-enter-active,
.mention-slide-leave-active {
  transition: max-height 0.2s ease, opacity 0.2s ease;
  overflow: hidden;
}
.mention-slide-enter-from,
.mention-slide-leave-to {
  max-height: 0;
  opacity: 0;
}
.mention-slide-enter-to,
.mention-slide-leave-from {
  max-height: 60px;
  opacity: 1;
}
</style>
