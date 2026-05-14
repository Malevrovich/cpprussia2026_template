import { describe, it, expect, beforeEach } from 'vitest'
import { setActivePinia, createPinia } from 'pinia'

describe('useChatsStore', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('setChats() sets the list of chats', async () => {
    const { useChatsStore } = await import('@/stores/chats')
    const store = useChatsStore()

    const newChats = [
      { id: 1, name: 'General', avatar: 'G' },
      { id: 2, name: 'Random', avatar: 'R' },
      { id: 3, name: 'Tech', avatar: 'T' },
    ]

    store.setChats(newChats)

    expect(store.chats).toHaveLength(3)
    expect(store.chats[2].name).toBe('Tech')
  })

  it('selectChat() sets selectedChatId', async () => {
    const { useChatsStore } = await import('@/stores/chats')
    const store = useChatsStore()

    store.setChats([
      { id: 1, name: 'General', avatar: 'G' },
      { id: 2, name: 'Random', avatar: 'R' },
    ])

    store.selectChat(1)

    expect(store.selectedChatId).toBe(1)
  })

  it('selectedChat getter returns the selected chat', async () => {
    const { useChatsStore } = await import('@/stores/chats')
    const store = useChatsStore()

    store.setChats([
      { id: 1, name: 'General', avatar: 'G' },
      { id: 2, name: 'Random', avatar: 'R' },
    ])
    store.selectChat(2)

    expect(store.selectedChat).toEqual({ id: 2, name: 'Random', avatar: 'R' })
  })

  it('selectedChat getter returns null when nothing is selected', async () => {
    const { useChatsStore } = await import('@/stores/chats')
    const store = useChatsStore()

    store.setChats([
      { id: 1, name: 'General', avatar: 'G' },
      { id: 2, name: 'Random', avatar: 'R' },
    ])

    expect(store.selectedChat).toBeNull()
  })
})
