<template>
  <div class="card">
    <h2>Files</h2>
    <p style="font-size: 0.85rem; color: #666; margin-top: -0.5rem;">
      Upload files and share them with other users. Anyone signed in can
      browse and download files uploaded by anybody.
    </p>

    <!-- ============ Upload ============ -->
    <h3 style="margin-top: 1rem;">Upload a file</h3>
    <div class="form-group">
      <label>Choose a file</label>
      <input
        ref="fileInputEl"
        type="file"
        :accept="ACCEPT_ATTR"
        @change="onFileSelected"
      />
      <div style="margin-top: 0.4rem; font-size: 0.8rem; color: #777;">
        Allowed: {{ ALLOWED_TYPES_LABEL }}. Max size: {{ formatSize(MAX_SIZE_BYTES) }}.
      </div>
      <div
        v-if="selectedFile"
        style="margin-top: 0.5rem; font-size: 0.9rem; color: #555;"
      >
        <strong>{{ selectedFile.name }}</strong>
        — {{ formatSize(selectedFile.size) }}
        — {{ selectedFile.type || 'application/octet-stream' }}
      </div>
      <div v-if="selectionError" class="error" style="margin-top: 0.4rem;">
        {{ selectionError }}
      </div>
    </div>

    <button
      @click="uploadFile"
      class="btn"
      :disabled="!selectedFile || !!selectionError || uploading"
    >
      {{ uploading ? 'Uploading…' : 'Upload' }}
    </button>

    <div v-if="uploadError" class="error" style="margin-top: 1rem;">{{ uploadError }}</div>
    <div v-if="uploadedUri" class="success" style="margin-top: 1rem; word-break: break-all;">
      Uploaded! URI: <code>{{ uploadedUri }}</code>
    </div>

    <!-- ============ Browse / Share ============ -->
    <h3 style="margin-top: 2rem;">All files</h3>
    <div class="form-group" style="display: flex; gap: 0.5rem; align-items: end; flex-wrap: wrap;">
      <div style="flex: 1; min-width: 200px;">
        <label>Filter by author login (optional)</label>
        <input
          v-model="filterLogin"
          type="text"
          placeholder="leave blank to see everyone's files"
          @keyup.enter="refreshList"
        />
      </div>
      <button @click="refreshList" class="btn" :disabled="listing">
        {{ listing ? 'Loading…' : 'Refresh' }}
      </button>
      <button
        v-if="filterLogin"
        @click="clearFilter"
        class="btn"
        style="background: #7f8c8d;"
      >
        Clear filter
      </button>
    </div>

    <div v-if="listError" class="error" style="margin-top: 0.5rem;">{{ listError }}</div>

    <div v-if="remoteFiles.length" style="margin-top: 1rem;">
      <table>
        <thead>
          <tr>
            <th>Filename</th>
            <th>Author</th>
            <th>Size</th>
            <th>Type</th>
            <th>URI</th>
            <th>Action</th>
          </tr>
        </thead>
        <tbody>
          <tr
            v-for="f in remoteFiles"
            :key="f.uri"
            :class="{ 'own-file': f.login === currentUser.login }"
          >
            <td>{{ f.filename }}</td>
            <td>
              {{ f.login }}
              <span
                v-if="f.login === currentUser.login"
                style="font-size: 0.7rem; color: #27ae60;"
              >
                (you)
              </span>
            </td>
            <td>{{ formatSize(f.size) }}</td>
            <td>{{ f.mime_type || '—' }}</td>
            <td><small style="word-break: break-all;">{{ f.uri }}</small></td>
            <td>
              <button
                @click="downloadFile(f)"
                class="btn"
                style="padding: 0.25rem 0.5rem; font-size: 0.8rem;"
                :disabled="downloadingUri === f.uri"
              >
                {{ downloadingUri === f.uri ? '…' : 'Download' }}
              </button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <div v-else-if="!listing && !listError" style="margin-top: 1rem; color: #777;">
      No files match this filter yet.
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, watch } from 'vue'
import { filesApi } from '../api'
import { useAuthStore } from '../stores/auth'

// ---------- Constants: allowed types & size ----------
// Limited to a reasonable subset for the workshop demo. Both MIME types and
// extensions are listed so the OS picker also gets a hint, and so we can
// fall back to extension checks when the browser reports a blank file.type.
const ALLOWED_MIME_TYPES = [
  'image/jpeg',
  'image/png',
  'image/gif',
  'image/webp',
  'application/pdf',
  'text/plain',
  'application/json',
  'application/zip',
  'application/x-zip-compressed',
]
const ALLOWED_EXTENSIONS = [
  'jpg', 'jpeg', 'png', 'gif', 'webp', 'pdf', 'txt', 'json', 'zip',
]
const ACCEPT_ATTR = [
  ...ALLOWED_MIME_TYPES,
  ...ALLOWED_EXTENSIONS.map((e) => `.${e}`),
].join(',')
const ALLOWED_TYPES_LABEL = 'images (jpg, png, gif, webp), pdf, txt, json, zip'
const MAX_SIZE_BYTES = 5 * 1024 * 1024 // 5 MB

// ---------- State ----------
const authStore = useAuthStore()
const currentUser = computed(() => authStore.currentUser)

const fileInputEl = ref(null)
const selectedFile = ref(null)
const selectionError = ref('')

const uploading = ref(false)
const uploadError = ref('')
const uploadedUri = ref('')

const remoteFiles = ref([])
const listing = ref(false)
const listError = ref('')
const filterLogin = ref('')

const downloadingUri = ref('')

// ---------- Helpers ----------
function formatSize(bytes) {
  if (bytes == null) return '—'
  if (bytes < 1024) return `${bytes} B`
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`
  return `${(bytes / 1024 / 1024).toFixed(2)} MB`
}

function getExtension(name) {
  const idx = name.lastIndexOf('.')
  return idx >= 0 ? name.slice(idx + 1).toLowerCase() : ''
}

function isAllowed(file) {
  const ext = getExtension(file.name)
  if (ALLOWED_MIME_TYPES.includes(file.type)) return true
  if (ALLOWED_EXTENSIONS.includes(ext)) return true
  return false
}

function fileToBase64(file) {
  // FileReader handles binary correctly; btoa() does not.
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = () => {
      const result = reader.result // "data:<mime>;base64,AAAA..."
      const commaIdx = result.indexOf(',')
      resolve(commaIdx >= 0 ? result.slice(commaIdx + 1) : result)
    }
    reader.onerror = () => reject(reader.error)
    reader.readAsDataURL(file)
  })
}

function base64ToBlob(base64, mimeType) {
  const binary = atob(base64)
  const bytes = new Uint8Array(binary.length)
  for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i)
  return new Blob([bytes], { type: mimeType || 'application/octet-stream' })
}

function extractApiError(e, fallback) {
  return (
    e?.response?.data?.detail?.message ||
    e?.response?.data?.detail ||
    e?.response?.data?.error ||
    e?.message ||
    fallback
  )
}

// ---------- Selection ----------
function onFileSelected(event) {
  uploadError.value = ''
  uploadedUri.value = ''
  selectionError.value = ''
  const file = event.target.files?.[0] || null
  selectedFile.value = file
  if (!file) return

  if (!isAllowed(file)) {
    selectionError.value = `File type not allowed. Allowed: ${ALLOWED_TYPES_LABEL}.`
    return
  }
  if (file.size > MAX_SIZE_BYTES) {
    selectionError.value =
      `File is too large (${formatSize(file.size)}). ` +
      `Maximum allowed size is ${formatSize(MAX_SIZE_BYTES)}.`
  }
}

// ---------- Upload ----------
async function uploadFile() {
  if (!selectedFile.value || selectionError.value) return
  uploadError.value = ''
  uploadedUri.value = ''
  uploading.value = true
  try {
    const file = selectedFile.value
    const contentBase64 = await fileToBase64(file)

    const response = await filesApi.upload({
      login: currentUser.value.login,
      filename: file.name,
      content: contentBase64,
      mime_type: file.type || 'application/octet-stream',
      size: file.size,
    })
    uploadedUri.value = response.data.uri

    // Reset picker
    selectedFile.value = null
    if (fileInputEl.value) fileInputEl.value.value = ''

    // Refresh the shared list so the new file appears to everyone.
    await refreshList()
  } catch (e) {
    uploadError.value = extractApiError(e, 'Upload failed')
  } finally {
    uploading.value = false
  }
}

// ---------- List ----------
async function refreshList() {
  listError.value = ''
  listing.value = true
  try {
    const loginFilter = filterLogin.value.trim() || null
    const response = await filesApi.list(currentUser.value, loginFilter)
    remoteFiles.value = response.data.files || []
  } catch (e) {
    listError.value = extractApiError(e, 'Failed to load file list')
    remoteFiles.value = []
  } finally {
    listing.value = false
  }
}

function clearFilter() {
  filterLogin.value = ''
  refreshList()
}

// ---------- Download ----------
async function downloadFile(fileEntry) {
  uploadError.value = ''
  downloadingUri.value = fileEntry.uri
  try {
    const response = await filesApi.download(fileEntry.uri, currentUser.value)
    const { content, filename, mime_type } = response.data
    const blob = base64ToBlob(content, mime_type || fileEntry.mime_type)

    // Trigger a real browser download.
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = filename || fileEntry.filename || 'download'
    document.body.appendChild(a)
    a.click()
    a.remove()
    setTimeout(() => URL.revokeObjectURL(url), 0)
  } catch (e) {
    uploadError.value = extractApiError(e, 'Download failed')
  } finally {
    downloadingUri.value = ''
  }
}

// ---------- Lifecycle ----------
onMounted(refreshList)
watch(() => currentUser.value.login, refreshList)
</script>

<style scoped>
.own-file {
  background: #f6fff6;
}
</style>
