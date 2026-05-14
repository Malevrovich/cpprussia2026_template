import { describe, it, expect, beforeEach, vi } from 'vitest'
import { setActivePinia, createPinia } from 'pinia'
import { messagingApi } from '@/api'

vi.mock('@/api', () => ({
  messagingApi: {
    getMessages: vi.fn(),
    sendMessage: vi.fn(),
  },
}))

describe('useMessagesStore', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    setActivePinia(createPinia())
  })

  it('fetchMessages() calls messagingApi.getMessages', async () => {
    const { useMessagesStore } = await import('@/stores/messages')
    const store = useMessagesStore()

    const mockMessages = [
      { id: 1, channel_id: 1, text: 'Hello', author: 'user1' },
    ]
    messagingApi.getMessages.mockResolvedValue({ data: { messages: mockMessages } })

    await store.fetchMessages(1)

    expect(messagingApi.getMessages).toHaveBeenCalledWith(1, 0, expect.any(Number))
  })

  it('sendMessage() adds message to store', async () => {
    const { useMessagesStore } = await import('@/stores/messages')
    const store = useMessagesStore()

    const newMessage = { id: 1, channel_id: 1, text: 'Hello', author: 'user1' }
    messagingApi.sendMessage.mockResolvedValue({ data: newMessage })

    await store.sendMessage(1, 'Hello', 'user1')

    expect(store.messages[1]).toContainEqual(newMessage)
  })

  it('addMessage() adds message to correct channelId', async () => {
    const { useMessagesStore } = await import('@/stores/messages')
    const store = useMessagesStore()

    const message1 = { id: 1, channel_id: 1, text: 'Hello', author: 'user1' }
    const message2 = { id: 2, channel_id: 2, text: 'Hi', author: 'user2' }

    store.addMessage(message1)
    store.addMessage(message2)

    expect(store.messages[1]).toHaveLength(1)
    expect(store.messages[1][0].text).toBe('Hello')
    expect(store.messages[2]).toHaveLength(1)
    expect(store.messages[2][0].text).toBe('Hi')
  })

  it('messagesForChannel returns messages for channel', async () => {
    const { useMessagesStore } = await import('@/stores/messages')
    const store = useMessagesStore()

    store.addMessage({ id: 1, channel_id: 1, text: 'Hello', author: 'user1' })
    store.addMessage({ id: 2, channel_id: 1, text: 'World', author: 'user2' })
    store.addMessage({ id: 3, channel_id: 2, text: 'Other', author: 'user1' })

    const channelMessages = store.messagesForChannel(1)

    expect(channelMessages.value).toHaveLength(2)
    expect(channelMessages.value[0].text).toBe('Hello')
    expect(channelMessages.value[1].text).toBe('World')
  })

  it('startPolling/stopPolling work with interval', async () => {
    vi.useFakeTimers()
    const { useMessagesStore } = await import('@/stores/messages')
    const store = useMessagesStore()

    messagingApi.getMessages.mockResolvedValue({ data: { messages: [] } })

    store.startPolling(1)

    expect(store.pollingInterval).not.toBeNull()

    vi.advanceTimersByTime(5000)

    store.stopPolling()

    expect(store.pollingInterval).toBeNull()

    vi.useRealTimers()
  })
})
