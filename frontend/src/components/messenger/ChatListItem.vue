<template>
  <div
    class="chat-list-item flex items-center p-3 cursor-pointer transition-colors"
    :class="[
      isSelected ? 'bg-blue-50 border-l-2 border-blue-500' : 'hover:bg-gray-100'
    ]"
    @click="$emit('select')"
  >
    <div class="avatar w-10 h-10 rounded-full bg-blue-500 flex items-center justify-center text-white font-medium mr-3">
      {{ chat.name.charAt(0).toUpperCase() }}
    </div>
    <div class="flex-1 min-w-0">
      <div class="flex items-center justify-between">
        <span class="font-medium text-gray-900 truncate">{{ chat.name }}</span>
        <span
          v-if="unreadCount > 0"
          class="unread-badge inline-flex items-center justify-center min-w-[20px] h-5 text-xs font-medium text-white bg-red-500 rounded-full px-1.5"
        >
          {{ unreadCount > 99 ? '99+' : unreadCount }}
        </span>
      </div>
      <div class="flex items-center mt-0.5">
        <StatusIndicator :status="status" class="mr-2" />
        <span class="text-sm text-gray-500">Click to chat</span>
      </div>
    </div>
  </div>
</template>

<script setup>
import StatusIndicator from './StatusIndicator.vue'

defineProps({
  chat: {
    type: Object,
    required: true,
    validator: (value) => 'id' in value && 'name' in value
  },
  isSelected: {
    type: Boolean,
    default: false
  },
  unreadCount: {
    type: Number,
    default: 0
  },
  status: {
    type: String,
    default: 'offline'
  }
})

defineEmits(['select'])
</script>
