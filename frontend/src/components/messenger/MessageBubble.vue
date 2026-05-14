<template>
  <div
    class="message-bubble p-3 rounded-lg max-w-[70%] transition-opacity"
    :class="isOwnMessage ? 'bg-blue-500 text-white ml-auto' : 'bg-gray-100 text-gray-900 mr-auto'"
    @mouseenter="showReactions = true"
    @mouseleave="showReactions = false"
  >
    <div v-if="!isOwnMessage" class="author font-semibold text-sm mb-1">
      {{ messageAuthor }}
    </div>
    <div class="text break-words">{{ messageText }}</div>
    
    <!-- File attachment display -->
    <div v-if="message.file_uri" class="file-attachment mt-2 p-2 bg-white bg-opacity-20 rounded">
      <a :href="message.file_uri" target="_blank" class="text-sm underline">
        📎 File: {{ message.file_name || 'attachment' }}
      </a>
    </div>
    
    <div class="timestamp text-xs mt-1 opacity-70">
      {{ formattedTime }}
    </div>
    
    <!-- Reactions display - always visible if reactions exist -->
    <div v-if="hasReactions" class="reactions mt-1 flex flex-wrap gap-1">
      <span
        v-for="(count, type) in aggregatedReactions"
        :key="type"
        class="reaction-badge text-xs px-1.5 py-0.5 bg-white bg-opacity-30 rounded-full"
      >
        {{ getEmoji(type) }} {{ count }}
      </span>
    </div>
    
    <Transition name="fade">
      <div v-if="showReactions" class="reaction-bar-wrapper mt-2">
        <ReactionBar
          :channel-id="channelId"
          :message-id="messageId"
          :reactions="message.reactions || {}"
          @reaction-changed="handleReactionChanged"
        />
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted } from 'vue'
import { useAuthStore } from '@/stores/auth'
import { useReactionsStore } from '@/stores/reactions'
import ReactionBar from './ReactionBar.vue'

const props = defineProps({
  channelId: {
    type: [Number, String],
    required: true
  },
  message: {
    type: Object,
    required: true
  }
})

const emit = defineEmits(['reaction-changed'])

const authStore = useAuthStore()
const reactionsStore = useReactionsStore()
const showReactions = ref(false)

// Message text - handle both 'text' and 'message' fields
const messageText = computed(() => props.message.text || props.message.message || '')

// Message author - handle both 'author' and 'current_user.name'
const messageAuthor = computed(() => {
  if (props.message.author) return props.message.author
  if (props.message.current_user) return props.message.current_user.name || props.message.current_user.login
  return 'Unknown'
})

// Message ID - extract real ID (not temp)
const messageId = computed(() => props.message.id || props.message.message_id)

// Check if message has a real ID (not temp)
const hasRealId = computed(() => {
  const id = messageId.value
  return id && !String(id).startsWith('temp_')
})

// Check if own message
const isOwnMessage = computed(() => {
  if (!authStore.currentUser) return false
  return messageAuthor.value === authStore.currentUser.login || 
         messageAuthor.value === authStore.currentUser.name
})

// Formatted time
const formattedTime = computed(() => {
  if (!props.message.timestamp) return ''
  const date = new Date(props.message.timestamp)
  return date.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit', hour12: false })
})

// Fetch reactions only when message has real ID
async function fetchReactionsIfReady() {
  if (hasRealId.value) {
    await reactionsStore.fetchReactions(props.channelId, messageId.value)
  }
}

// Fetch on mount
onMounted(() => {
  fetchReactionsIfReady()
})

// Watch for messageId changes (temp -> real)
watch(messageId, (newId, oldId) => {
  // Re-fetch when ID changes from temp to real
  if (newId && oldId && String(oldId).startsWith('temp_') && !String(newId).startsWith('temp_')) {
    fetchReactionsIfReady()
  }
})

// Get aggregated reactions from store
const reactionKey = computed(() => `${props.channelId}-${messageId.value}`)
const aggregatedReactions = computed(() => {
  return reactionsStore.reactions[reactionKey.value] || props.message.reactions || {}
})

const hasReactions = computed(() => {
  const reactions = aggregatedReactions.value
  return reactions && Object.keys(reactions).length > 0 && Object.values(reactions).some(v => v > 0)
})

function getEmoji(type) {
  const emojis = {
    like: '👍',
    dislike: '👎',
    heart: '❤️',
    fire: '🔥',
    okay: '👌',
    LOL: '😂',
    smile: '😊'
  }
  return emojis[type] || type
}

function handleReactionChanged() {
  emit('reaction-changed')
}
</script>

<style scoped>
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.2s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
