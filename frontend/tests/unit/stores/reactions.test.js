import { describe, it, expect, beforeEach, vi } from 'vitest'
import { setActivePinia, createPinia } from 'pinia'
import { reactionsApi } from '@/api'

vi.mock('@/api', () => ({
  reactionsApi: {
    get: vi.fn(),
    trigger: vi.fn(),
  },
}))

describe('useReactionsStore', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    setActivePinia(createPinia())
  })

  it('getReactionKey() returns correct format "channelId-messageId"', async () => {
    const { useReactionsStore } = await import('@/stores/reactions')
    const store = useReactionsStore()

    expect(store.getReactionKey(1, 100)).toBe('1-100')
    expect(store.getReactionKey('abc', 'xyz')).toBe('abc-xyz')
  })

  it('fetchReactions() gets reactions for message', async () => {
    const { useReactionsStore } = await import('@/stores/reactions')
    const store = useReactionsStore()

    const mockReactions = { like: 5, heart: 3 }
    reactionsApi.get.mockResolvedValue({ data: mockReactions })

    await store.fetchReactions(1, 100)

    expect(reactionsApi.get).toHaveBeenCalledWith(1, 100)
    expect(store.reactions['1-100']).toEqual(mockReactions)
  })

  it('toggleReaction() calls reactionsApi.trigger', async () => {
    const { useReactionsStore } = await import('@/stores/reactions')
    const store = useReactionsStore()

    reactionsApi.trigger.mockResolvedValue({ data: {} })

    await store.toggleReaction(1, 100, 'like')

    expect(reactionsApi.trigger).toHaveBeenCalledWith({
      channel_id: 1,
      message_id: 100,
      reaction_type: 'like',
    })
  })
})
