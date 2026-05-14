import axios from 'axios'

const API_BASE = '/api'

const apiClient = axios.create({
  baseURL: API_BASE,
})

apiClient.interceptors.request.use((config) => {
  const token = localStorage.getItem('token')
  if (token) {
    config.headers['Authorization'] = token
  }
  return config
})

export const authApi = {
  register(data) {
    return apiClient.post('/auth/v1/user/registration', data)
  },
  login(data) {
    return apiClient.post('/auth/v1/user/authorization', data)
  },
  searchUsers(query, limit = 10) {
    return apiClient.post('/auth/v1/user/search', { query, limit })
  },
}

export const messagingApi = {
  sendMessage(data) {
    return apiClient.post('/messaging/v1/channel/message/new', data)
  },
  getMessages(channelId, from, to, currentUser) {
    return apiClient.post('/messaging/v1/channel/message/by-timestamp', {
      current_user: currentUser,
      channel_id: channelId,
      from,
      to,
    })
  },
  // Direct messages
  getOrCreateDirectChat(otherUserLogin, currentUser) {
    return apiClient.post('/messaging/v1/direct/get-or-create', {
      current_user: currentUser,
      other_user_login: otherUserLogin,
    })
  },
  listDirectChats(currentUser) {
    return apiClient.post('/messaging/v1/direct/list', {
      current_user: currentUser,
    })
  },
}

export const notificationsApi = {
  list(channelId, currentUser) {
    return apiClient.post('/notifications/v1/channel/notification/list', {
      current_user: currentUser,
      channel_id: channelId,
    })
  },
  create(data) {
    return apiClient.post('/notifications/v1/channel/notification/new', data)
  },
}

export const filesApi = {
  upload(data) {
    return apiClient.post('/files/v1/file/new', data)
  },
  download(uri, currentUser) {
    return apiClient.post('/files/v1/file/by-uri', {
      current_user: currentUser,
      uri,
    })
  },
}

export const reactionsApi = {
  trigger(data) {
    return apiClient.post('/reactions/v1/like/trigger', data)
  },
  get(channelId, messageId) {
    return apiClient.get(`/reactions/v1/like/${channelId}/${messageId}`)
  },
}

export const statusApi = {
  update(data) {
    return apiClient.post('/status/v1/user/status/update', data)
  },
  get(login, currentUser) {
    return apiClient.post('/status/v1/user/status/by-login', {
      current_user: currentUser,
      login,
    })
  },
}

export default apiClient
