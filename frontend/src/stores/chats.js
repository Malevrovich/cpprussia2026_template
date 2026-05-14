import { defineStore } from 'pinia'
import { computed, ref } from 'vue'
import { messagingApi } from '@/api'
import { useAuthStore } from './auth'

export const useChatsStore = defineStore('chats', () => {
  // Channel chats (predefined)
  const channelChats = ref([
    { id: 1, name: 'General', avatar: 'G', type: 'channel' },
    { id: 2, name: 'Random', avatar: 'R', type: 'channel' },
  ])
  
  // Direct message chats (from API)
  const directChats = ref([])

  // All chats combined
  const chats = computed(() => [...channelChats.value, ...directChats.value])

  const selectedChatId = ref(null)

  const selectedChat = computed(() => {
    if (!selectedChatId.value) return null
    return chats.value.find(chat => chat.id === selectedChatId.value) || null
  })

  const selectedChatType = computed(() => {
    const chat = selectedChat.value
    if (!chat) return null
    return chat.type || 'channel'
  })

  function selectChat(id) {
    selectedChatId.value = id
  }

  function setChats(newChats) {
    chats.value = newChats
  }

  async function fetchDirectChats() {
    const authStore = useAuthStore()
    if (!authStore.currentUser) return
    
    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name,
        token: authStore.currentUser.token
      }
      
      const response = await messagingApi.listDirectChats(currentUser)
      directChats.value = response.data.direct_chats.map(chat => ({
        id: chat.channel_id,
        name: chat.other_user.name || chat.other_user.login,
        avatar: (chat.other_user.name || chat.other_user.login).charAt(0).toUpperCase(),
        type: 'direct',
        otherUser: chat.other_user,
        lastMessage: chat.last_message?.message || null
      }))
    } catch (error) {
      console.error('Failed to fetch direct chats:', error)
    }
  }

  async function getOrCreateDirectChat(otherUserLogin) {
    const authStore = useAuthStore()
    if (!authStore.currentUser) return null
    
    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name,
        token: authStore.currentUser.token
      }
      
      const response = await messagingApi.getOrCreateDirectChat(otherUserLogin, currentUser)
      const chatData = response.data
      
      // Check if we already have this chat
      const existingChat = directChats.value.find(c => c.id === chatData.channel_id)
      if (existingChat) {
        return existingChat
      }
      
      // Add new direct chat
      const newChat = {
        id: chatData.channel_id,
        name: chatData.other_user.name || chatData.other_user.login,
        avatar: (chatData.other_user.name || chatData.other_user.login).charAt(0).toUpperCase(),
        type: 'direct',
        otherUser: chatData.other_user
      }
      directChats.value.push(newChat)
      return newChat
    } catch (error) {
      console.error('Failed to get or create direct chat:', error)
      return null
    }
  }

  return {
    chats,
    channelChats,
    directChats,
    selectedChatId,
    selectedChat,
    selectedChatType,
    selectChat,
    setChats,
    fetchDirectChats,
    getOrCreateDirectChat,
  }
})
