<template>
  <div class="card">
    <h2>Files</h2>
    <div class="form-group">
      <label>Filename</label>
      <input v-model="filename" type="text" placeholder="example.txt" />
    </div>
    <div class="form-group">
      <label>Content (Base64 or plain text)</label>
      <textarea v-model="content" rows="4" placeholder="File content..."></textarea>
    </div>
    <button @click="uploadFile" class="btn" :disabled="!filename || !content">Upload</button>
    <button @click="loadFiles" class="btn" style="margin-left: 0.5rem; background: #27ae60;">Refresh</button>
    <div v-if="error" class="error" style="margin-top: 1rem;">{{ error }}</div>
    <div v-if="uploadedUri" class="success" style="margin-top: 1rem;">
      Uploaded! URI: {{ uploadedUri }}
    </div>
    <div v-if="files.length" style="margin-top: 1rem;">
      <h3>Uploaded Files</h3>
      <table>
        <thead>
          <tr><th>URI</th><th>Login</th><th>Filename</th><th>Action</th></tr>
        </thead>
        <tbody>
          <tr v-for="f in files" :key="f.uri">
            <td><small>{{ f.uri }}</small></td>
            <td>{{ f.login }}</td>
            <td>{{ f.filename }}</td>
            <td><button @click="downloadFile(f.uri)" class="btn" style="padding: 0.25rem 0.5rem; font-size: 0.8rem;">Download</button></td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { filesApi } from '../api'
import { useAuthStore } from '../stores/auth'

const authStore = useAuthStore()
const filename = ref('')
const content = ref('')
const files = ref([])
const error = ref('')
const uploadedUri = ref('')

const user = { token: authStore.currentUser.token, login: authStore.currentUser.login, name: authStore.currentUser.name }

async function uploadFile() {
  error.value = ''
  uploadedUri.value = ''
  try {
    const response = await filesApi.upload({
      login: user.login,
      filename: filename.value,
      content: btoa(content.value),
    })
    uploadedUri.value = response.data.uri
    content.value = ''
    await loadFiles()
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Upload failed'
  }
}

async function loadFiles() {
  files.value = files.value
}

async function downloadFile(uri) {
  error.value = ''
  try {
    const response = await filesApi.download(uri, user)
    const decoded = atob(response.data.content)
    alert(`File content:\n${decoded}`)
  } catch (e) {
    error.value = e.response?.data?.detail?.message || 'Download failed'
  }
}
</script>
