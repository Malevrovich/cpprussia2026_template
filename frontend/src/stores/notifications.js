import { defineStore } from 'pinia'
import { reactive, ref, computed } from 'vue'
import { notificationsApi } from '@/api'
import { useAuthStore } from './auth'
import { useChatsStore } from './chats'


export const useNotificationsStore = defineStore('notifications', () => {
  // Store notifications by channel: { [channelId]: [{ message_id, read }] }
  const notificationsByChannel = reactive({})
  const pollingInterval = ref(null)

  function generateIdempotencyToken() {
    return Math.random().toString(36).substring(2) + Date.now().toString(36)
  }

  // Fetch notifications for a specific channel
  async function fetchNotifications(channelId) {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login || !authStore.currentUser?.token) return

    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name || authStore.currentUser.login,
        token: authStore.currentUser.token,
      }

      const response = await notificationsApi.list(channelId, currentUser)
      notificationsByChannel[channelId] = response.data?.notifications || []
    } catch (error) {
      console.error(`Failed to fetch notifications for channel ${channelId}:`, error)
    }
  }

  // Fetch notifications for ALL known channels at once
  async function fetchAllNotifications() {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login || !authStore.currentUser?.token) return

    const chatsStore = useChatsStore()
    const allChannelIds = chatsStore.chats.map(c => c.id)

    await Promise.allSettled(allChannelIds.map(id => fetchNotifications(id)))
  }

  async function createNotification(channelId, messageId, otherUserLogin) {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login || !authStore.currentUser?.token) return

    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name || authStore.currentUser.login,
        token: authStore.currentUser.token,
      }

      await notificationsApi.create({
        current_user: currentUser,
        channel_id: channelId,
        message_id: messageId,
        other_user_login: otherUserLogin,
        idempotency_token: generateIdempotencyToken(),
      })
    } catch (error) {
      console.error('Failed to create notification:', error)
    }
  }

  // Computed: unread count per channel
  function getUnreadCount(channelId) {
    const notifications = notificationsByChannel[channelId] || []
    return notifications.filter(n => !n.read).length
  }

  // Reactive computed total unread count across all channels
  const totalUnreadCount = computed(() => {
    let total = 0
    for (const channelId of Object.keys(notificationsByChannel)) {
      total += getUnreadCount(channelId)
    }
    return total
  })

  // All notifications flattened with channel info, newest first
  const allNotifications = computed(() => {
    const result = []
    for (const [channelId, notifications] of Object.entries(notificationsByChannel)) {
      for (const n of notifications) {
        result.push({ ...n, channelId })
      }
    }
    // Sort unread first, then by message_id descending
    return result.sort((a, b) => {
      if (a.read !== b.read) return a.read ? 1 : -1
      return b.message_id - a.message_id
    })
  })

  // Mark a specific notification as read (optimistic update + API call)
  async function markNotificationRead(channelId, messageId) {
    const authStore = useAuthStore()
    if (!authStore.currentUser?.login || !authStore.currentUser?.token) return

    // Optimistic update: mark locally immediately so UI responds instantly
    const channelNotifications = notificationsByChannel[channelId]
    if (channelNotifications) {
      for (const n of channelNotifications) {
        if (n.message_id === messageId) {
          n.read = true
        }
      }
    }

    try {
      const currentUser = {
        login: authStore.currentUser.login,
        name: authStore.currentUser.name || authStore.currentUser.login,
        token: authStore.currentUser.token,
      }
      await notificationsApi.markRead(channelId, messageId, currentUser)
    } catch (error) {
      console.error('Failed to mark notification as read:', error)
      // Revert optimistic update on failure
      await fetchNotifications(channelId)
    }
  }

  // Mark ALL unread notifications as read for a channel
  async function markAllReadForChannel(channelId) {
    const notifications = notificationsByChannel[channelId] || []
    const unread = notifications.filter(n => !n.read)
    await Promise.allSettled(
      unread.map(n => markNotificationRead(channelId, n.message_id))
    )
  }

  // Poll ALL channels every 10 seconds
  function startPolling() {
    stopPolling()
    // Fetch immediately, then on interval
    fetchAllNotifications()
    pollingInterval.value = setInterval(() => {
      fetchAllNotifications()
    }, 10000)
  }

  function stopPolling() {
    if (pollingInterval.value) {
      clearInterval(pollingInterval.value)
      pollingInterval.value = null
    }
  }

  return {
    notificationsByChannel,
    allNotifications,
    totalUnreadCount,
    fetchNotifications,
    fetchAllNotifications,
    createNotification,
    markNotificationRead,
    markAllReadForChannel,
    getUnreadCount,
    startPolling,
    stopPolling,
  }
})
