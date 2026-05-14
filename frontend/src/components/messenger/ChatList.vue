<template>
  <div class="chat-list w-[300px] bg-white border-r border-gray-200 flex flex-col h-full">
    <div class="p-4 border-b border-gray-200">
      <div class="flex items-center justify-between mb-2">
        <h2 class="text-lg font-semibold text-gray-900">Chats</h2>
        <button
          class="p-2 text-blue-500 hover:bg-blue-50 rounded-lg"
          title="Find user to chat"
          @click="showUserSearch = true"
        >
          ➕
        </button>
      </div>
      <input
        v-model="searchQuery"
        type="text"
        placeholder="Search chats..."
        class="w-full px-3 py-2 border border-gray-300 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-blue-500"
      />
    </div>
    
    <div class="flex-1 overflow-y-auto">
      <div v-if="!searchQuery.trim()" class="py-2">
        <div class="px-4 py-1 text-xs text-gray-500 uppercase tracking-wide">Channels</div>
        <ChatListItem
          v-for="chat in channelChats"
          :key="chat.id"
          :chat="chat"
          :is-selected="chat.id === chatStore.selectedChatId"
          status="offline"
          :unread-count="0"
          @select="handleSelectChat(chat)"
        />
        
        <div v-if="directChats.length > 0" class="mt-4">
          <div class="px-4 py-1 text-xs text-gray-500 uppercase tracking-wide">Direct Messages</div>
          <ChatListItem
            v-for="chat in directChats"
            :key="chat.id"
            :chat="chat"
            :is-selected="chat.id === chatStore.selectedChatId"
            status="offline"
            :unread-count="0"
            @select="handleSelectChat(chat)"
          />
        </div>
      </div>
      
      <div v-else>
        <ChatListItem
          v-for="chat in filteredChats"
          :key="chat.id"
          :chat="chat"
          :is-selected="chat.id === chatStore.selectedChatId"
          status="offline"
          :unread-count="0"
          @select="handleSelectChat(chat)"
        />
        <div v-if="filteredChats.length === 0" class="p-4 text-center text-gray-500 text-sm">
          No chats found
        </div>
      </div>
    </div>
    
    <!-- User Search Modal -->
    <UserSearchModal
      :show="showUserSearch"
      @close="showUserSearch = false"
      @select="handleUserSelected"
    />
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useChatsStore } from '@/stores/chats'
import ChatListItem from './ChatListItem.vue'
import UserSearchModal from './UserSearchModal.vue'

const chatStore = useChatsStore()
const searchQuery = ref('')
const showUserSearch = ref(false)

const emit = defineEmits(['chat-selected'])

const channelChats = computed(() => chatStore.channelChats)
const directChats = computed(() => chatStore.directChats)

const filteredChats = computed(() => {
  if (!searchQuery.value.trim()) {
    return chatStore.chats
  }
  const query = searchQuery.value.toLowerCase()
  return chatStore.chats.filter(chat => 
    chat.name.toLowerCase().includes(query)
  )
})

onMounted(() => {
  chatStore.fetchDirectChats()
})

function handleSelectChat(chat) {
  chatStore.selectChat(chat.id)
  emit('chat-selected', chat.id)
}

async function handleUserSelected(user) {
  // Get or create direct chat with selected user
  const chat = await chatStore.getOrCreateDirectChat(user.login)
  if (chat) {
    chatStore.selectChat(chat.id)
    emit('chat-selected', chat.id)
  }
}
</script>
