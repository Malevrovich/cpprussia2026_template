import { defineStore } from 'pinia'
import { computed, reactive, ref } from 'vue'
import { statusApi } from '@/api'

export const useStatusStore = defineStore('status', () => {
  const statuses = reactive({})
  const pollingInterval = ref(null)

  const VALID_STATUSES = ['online', 'away', 'busy', 'offline']

  function getStatusForLogin(login) {
    return computed(() => statuses[login] || 'offline')
  }

  async function fetchStatus(login) {
    try {
      const response = await statusApi.get(login, login)
      statuses[login] = response.data.status || 'offline'
    } catch (error) {
      console.error('Failed to fetch status:', error)
      statuses[login] = 'offline'
    }
  }

  async function updateMyStatus(status) {
    if (!VALID_STATUSES.includes(status)) {
      throw new Error(`Invalid status. Must be one of: ${VALID_STATUSES.join(', ')}`)
    }
    try {
      const currentUser = localStorage.getItem('login')
      await statusApi.update({ status, current_user: currentUser })
      statuses[currentUser] = status
    } catch (error) {
      console.error('Failed to update status:', error)
      throw error
    }
  }

  function startPolling() {
    stopPolling()
    pollingInterval.value = setInterval(() => {
      Object.keys(statuses).forEach(login => fetchStatus(login))
    }, 10000)
  }

  function stopPolling() {
    if (pollingInterval.value) {
      clearInterval(pollingInterval.value)
      pollingInterval.value = null
    }
  }

  return {
    statuses,
    pollingInterval,
    VALID_STATUSES,
    getStatusForLogin,
    fetchStatus,
    updateMyStatus,
    startPolling,
    stopPolling,
  }
})
