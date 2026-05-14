import { describe, it, expect, beforeEach, vi } from 'vitest'
import { setActivePinia, createPinia } from 'pinia'
import { authApi } from '@/api'

vi.mock('@/api', () => ({
  authApi: {
    login: vi.fn(),
    register: vi.fn(),
  },
}))

describe('useAuthStore', () => {
  beforeEach(() => {
    localStorage.clear()
    setActivePinia(createPinia())
  })

  it('login() sets currentUser and saves to localStorage', async () => {
    const { useAuthStore } = await import('@/stores/auth')
    const store = useAuthStore()

    const mockUser = { token: 'test-token', login: 'testuser', name: 'Test User' }
    authApi.login.mockResolvedValue({ data: mockUser })

    const result = await store.login({ login: 'testuser', password: 'password' })

    expect(authApi.login).toHaveBeenCalledWith({ login: 'testuser', password: 'password' })
    expect(store.currentUser.token).toBe('test-token')
    expect(store.currentUser.login).toBe('testuser')
    expect(store.currentUser.name).toBe('Test User')
    expect(localStorage.getItem('token')).toBe('test-token')
    expect(localStorage.getItem('login')).toBe('testuser')
    expect(localStorage.getItem('name')).toBe('Test User')
  })

  it('logout() clears currentUser and localStorage', async () => {
    const { useAuthStore } = await import('@/stores/auth')
    const store = useAuthStore()

    localStorage.setItem('token', 'test-token')
    localStorage.setItem('login', 'testuser')
    localStorage.setItem('name', 'Test User')
    store.$patch({ token: 'test-token', login: 'testuser', name: 'Test User' })

    await store.logout()

    expect(store.currentUser.token).toBeNull()
    expect(store.currentUser.login).toBeNull()
    expect(store.currentUser.name).toBeNull()
    expect(localStorage.getItem('token')).toBeNull()
    expect(localStorage.getItem('login')).toBeNull()
    expect(localStorage.getItem('name')).toBeNull()
  })

  it('register() calls authApi.register', async () => {
    const { useAuthStore } = await import('@/stores/auth')
    const store = useAuthStore()

    const mockUser = { token: 'reg-token', login: 'newuser', name: 'New User' }
    authApi.register.mockResolvedValue({ data: mockUser })

    await store.register({ login: 'newuser', password: 'password', name: 'New User' })

    expect(authApi.register).toHaveBeenCalledWith({ login: 'newuser', password: 'password', name: 'New User' })
  })

  it('isLoggedIn returns true when there is a token', async () => {
    localStorage.setItem('token', 'test-token')
    localStorage.setItem('login', 'testuser')
    localStorage.setItem('name', 'Test User')

    const { useAuthStore } = await import('@/stores/auth')
    const store = useAuthStore()

    expect(store.isLoggedIn).toBe(true)
  })
})
