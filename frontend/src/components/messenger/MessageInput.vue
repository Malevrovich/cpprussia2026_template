<template>
  <div class="message-input border-t border-gray-200 p-3 bg-white">
    <!-- File preview -->
    <div v-if="attachedFile" class="file-preview mb-2 p-2 bg-gray-100 rounded-lg flex items-center gap-2">
      <span class="text-sm">📎 {{ attachedFile.name }}</span>
      <button @click="removeFile" class="text-red-500 hover:text-red-600 ml-auto">✕</button>
    </div>

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
      <input
        type="file"
        ref="fileInputRef"
        class="hidden"
        @change="handleFileSelect"
      />
      <!-- Attach file button -->
      <button
        class="p-2 text-gray-500 hover:text-gray-600 hover:bg-gray-100 rounded-lg transition-colors flex-shrink-0"
        @click="triggerFileSelect"
        title="Attach file"
      >
        📎
      </button>
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
        {{ isUploading ? '…' : 'Send' }}
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, nextTick } from 'vue'
import { useMessagesStore } from '@/stores/messages'
import { useAuthStore } from '@/stores/auth'
import { useNotificationsStore } from '@/stores/notifications'
import { filesApi } from '@/api'

const messagesStore = useMessagesStore()
const authStore = useAuthStore()
const notificationsStore = useNotificationsStore()

const messageText = ref('')
const textareaRef = ref(null)
const fileInputRef = ref(null)
const attachedFile = ref(null)
const isUploading = ref(false)

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

const canSend = computed(() => {
  return (messageText.value.trim() || attachedFile.value) && !isUploading.value
})

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

function triggerFileSelect() {
  fileInputRef.value?.click()
}

async function handleFileSelect(event) {
  const file = event.target.files?.[0]
  if (!file) return
  attachedFile.value = file
}

function removeFile() {
  attachedFile.value = null
  if (fileInputRef.value) {
    fileInputRef.value.value = ''
  }
}

async function uploadFile(file) {
  try {
    const base64 = await fileToBase64(file)
    const response = await filesApi.upload({
      file_name: file.name,
      content: base64,
      current_user: {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name,
        token: authStore.currentUser.token
      }
    })
    return response.data?.uri || response.data?.file_uri
  } catch (error) {
    console.error('File upload failed:', error)
    throw error
  }
}

function fileToBase64(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = () => resolve(reader.result)
    reader.onerror = reject
    reader.readAsDataURL(file)
  })
}

async function handleSend() {
  if (!canSend.value) return

  const text = messageText.value.trim()
  let fileUri = null
  let fileName = null

  // Upload file if attached
  if (attachedFile.value) {
    isUploading.value = true
    try {
      fileUri = await uploadFile(attachedFile.value)
      fileName = attachedFile.value.name
    } catch (error) {
      console.error('Failed to upload file:', error)
      isUploading.value = false
      return
    } finally {
      isUploading.value = false
    }
  }

  if (!text && !fileUri) return

  // Send the message and get back the sent message (with server-assigned message_id)
  const sentMessage = await messagesStore.sendMessage(
    props.channelId,
    text,
    authStore.currentUser.login,
    fileUri,
    fileName
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
  removeFile()
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
