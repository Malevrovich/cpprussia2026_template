import { defineStore } from 'pinia'
import { reactive } from 'vue'
import { reactionsApi } from '@/api'
import { useAuthStore } from './auth'

export const useReactionsStore = defineStore('reactions', () => {
  const reactions = reactive({})

  const VALID_REACTIONS = ['like', 'dislike', 'heart', 'fire', 'okay', 'LOL', 'smile']

  function getReactionKey(channelId, messageId) {
    return `${channelId}-${messageId}`
  }

  function generateIdempotencyToken() {
    return Math.random().toString(36).substring(2) + Date.now().toString(36)
  }

  async function fetchReactions(channelId, messageId) {
    try {
      const response = await reactionsApi.get(channelId, messageId)
      const key = getReactionKey(channelId, messageId)
      // Transform array response to object format { type: count }
      if (Array.isArray(response.data?.reactions)) {
        const reactionsObj = {}
        for (const r of response.data.reactions) {
          reactionsObj[r.animation] = reactionsObj[r.animation] ? reactionsObj[r.animation] + 1 : 1
        }
        reactions[key] = reactionsObj
      } else {
        reactions[key] = response.data || {}
      }
    } catch (error) {
      console.error('Failed to fetch reactions:', error)
    }
  }

  async function toggleReaction(channelId, messageId, reactionType) {
    if (!VALID_REACTIONS.includes(reactionType)) {
      throw new Error(`Invalid reaction type. Must be one of: ${VALID_REACTIONS.join(', ')}`)
    }
    
    const authStore = useAuthStore()
    
    try {
      const key = getReactionKey(channelId, messageId)
      const response = await reactionsApi.trigger({
        channel_id: channelId,
        message_id: messageId,
        animation: reactionType,
        idempotency_token: generateIdempotencyToken(),
        current_user: {
          login: authStore.currentUser.login,
          name: authStore.currentUser.name,
          token: authStore.currentUser.token
        }
      })
      
      // Update local state based on response
      if (!reactions[key]) {
        reactions[key] = {}
      }
      
      // Refresh reactions from server
      await fetchReactions(channelId, messageId)
      
      return response.data
    } catch (error) {
      console.error('Failed to toggle reaction:', error)
      throw error
    }
  }

  return {
    reactions,
    toggleReaction,
    fetchReactions,
    getReactionKey,
  }
})
