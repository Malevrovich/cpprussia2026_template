import { describe, it, expect, beforeEach, vi } from 'vitest'
import { setActivePinia, createPinia } from 'pinia'
import { statusApi } from '@/api'

vi.mock('@/api', () => ({
  statusApi: {
    update: vi.fn(),
    get: vi.fn(),
  },
}))

describe('useStatusStore', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    localStorage.clear()
    setActivePinia(createPinia())
  })

  it('updateMyStatus() calls statusApi.update', async () => {
    const { useStatusStore } = await import('@/stores/status')
    const store = useStatusStore()

    localStorage.setItem('login', 'testuser')
    statusApi.update.mockResolvedValue({ data: { status: 'online' } })

    await store.updateMyStatus('online')

    expect(statusApi.update).toHaveBeenCalledWith({ status: 'online', current_user: 'testuser' })
    expect(store.statuses['testuser']).toBe('online')
  })

  it('fetchStatus() gets status for login', async () => {
    const { useStatusStore } = await import('@/stores/status')
    const store = useStatusStore()

    statusApi.get.mockResolvedValue({ data: { status: 'busy' } })

    await store.fetchStatus('testuser')

    expect(statusApi.get).toHaveBeenCalledWith('testuser', 'testuser')
    expect(store.statuses['testuser']).toBe('busy')
  })

  it('getStatusForLogin returns correct status', async () => {
    const { useStatusStore } = await import('@/stores/status')
    const store = useStatusStore()

    store.statuses['user1'] = 'online'
    store.statuses['user2'] = 'busy'

    expect(store.getStatusForLogin('user1').value).toBe('online')
    expect(store.getStatusForLogin('user2').value).toBe('busy')
  })

  it('VALID_STATUSES contains all statuses', async () => {
    const { useStatusStore } = await import('@/stores/status')
    const store = useStatusStore()

    expect(store.VALID_STATUSES).toContain('online')
    expect(store.VALID_STATUSES).toContain('away')
    expect(store.VALID_STATUSES).toContain('busy')
    expect(store.VALID_STATUSES).toContain('offline')
    expect(store.VALID_STATUSES).toHaveLength(4)
  })
})
