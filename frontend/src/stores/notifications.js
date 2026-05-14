import { defineStore } from 'pinia'
import { reactive, ref } from 'vue'
import { notificationsApi } from '@/api'
import { useAuthStore } from './auth'

export const useNotificationsStore = defineStore('notifications', () => {
  // Store notifications by channel: { [channelId]: [{ message_id, read }] }
  const notificationsByChannel = reactive({})
  const pollingInterval = ref(null)

  function generateIdempotencyToken() {
    return Math.random().toString(36).substring(2) + Date.now().toString(36)
  }

  async function fetchNotifications(channelId) {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login) return

    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name || authStore.currentUser.login,
        token: authStore.currentUser.token
      }

      const response = await notificationsApi.list(channelId, currentUser)
      notificationsByChannel[channelId] = response.data?.notifications || []
    } catch (error) {
      console.error('Failed to fetch notifications:', error)
    }
  }

  async function createNotification(channelId, messageId, otherUserLogin) {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login) return

    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name || authStore.currentUser.login,
        token: authStore.currentUser.token
      }

      await notificationsApi.create({
        current_user: currentUser,
        channel_id: channelId,
        message_id: messageId,
        other_user_login: otherUserLogin,
        idempotency_token: generateIdempotencyToken()
      })
    } catch (error) {
      console.error('Failed to create notification:', error)
    }
  }

  function getUnreadCount(channelId) {
    const notifications = notificationsByChannel[channelId] || []
    return notifications.filter(n => !n.read).length
  }

  function getTotalUnreadCount() {
    let total = 0
    for (const channelId of Object.keys(notificationsByChannel)) {
      total += getUnreadCount(channelId)
    }
    return total
  }

  function startPolling(channelId) {
    stopPolling()
    pollingInterval.value = setInterval(() => {
      fetchNotifications(channelId)
    }, 10000) // Poll every 10 seconds
  }

  function stopPolling() {
    if (pollingInterval.value) {
      clearInterval(pollingInterval.value)
      pollingInterval.value = null
    }
  }

  return {
    notificationsByChannel,
    fetchNotifications,
    createNotification,
    getUnreadCount,
    getTotalUnreadCount,
    startPolling,
    stopPolling,
  }
})
