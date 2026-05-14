<template>
  <Transition name="modal">
    <div v-if="show" class="fixed inset-0 z-50 flex items-center justify-center">
      <div class="absolute inset-0 bg-black bg-opacity-50" @click="$emit('close')"></div>
      <div class="relative bg-white rounded-lg shadow-xl w-96 max-h-[80vh] flex flex-col">
        <div class="p-4 border-b border-gray-200">
          <h3 class="text-lg font-semibold">Find User</h3>
        </div>
        
        <div class="p-4">
          <input
            v-model="searchQuery"
            type="text"
            placeholder="Search by login or name..."
            class="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
            @input="debouncedSearch"
          />
        </div>
        
        <div class="flex-1 overflow-y-auto px-4 pb-4">
          <div v-if="loading" class="text-center text-gray-500 py-4">
            Searching...
          </div>
          <div v-else-if="error" class="text-center text-red-500 py-4">
            {{ error }}
          </div>
          <div v-else-if="users.length === 0 && searchQuery.length >= 1" class="text-center text-gray-500 py-4">
            No users found
          </div>
          <div v-else>
            <div
              v-for="user in users"
              :key="user.login"
              class="flex items-center justify-between p-3 hover:bg-gray-50 rounded-lg cursor-pointer"
              @click="selectUser(user)"
            >
              <div class="flex items-center gap-3">
                <div class="w-10 h-10 rounded-full bg-blue-500 flex items-center justify-center text-white font-medium">
                  {{ user.name.charAt(0).toUpperCase() }}
                </div>
                <div>
                  <div class="font-medium text-gray-900">{{ user.name }}</div>
                  <div class="text-sm text-gray-500">@{{ user.login }}</div>
                </div>
              </div>
              <button class="px-3 py-1 bg-blue-500 text-white rounded-lg text-sm hover:bg-blue-600">
                Chat
              </button>
            </div>
          </div>
        </div>
        
        <div class="p-4 border-t border-gray-200">
          <button
            class="w-full px-4 py-2 bg-gray-100 text-gray-700 rounded-lg hover:bg-gray-200"
            @click="$emit('close')"
          >
            Cancel
          </button>
        </div>
      </div>
    </div>
  </Transition>
</template>

<script setup>
import { ref } from 'vue'
import { authApi } from '@/api'

const props = defineProps({
  show: {
    type: Boolean,
    default: false
  }
})

const emit = defineEmits(['close', 'select'])

const searchQuery = ref('')
const users = ref([])
const loading = ref(false)
const error = ref(null)
let debounceTimer = null

function debouncedSearch() {
  clearTimeout(debounceTimer)
  if (searchQuery.value.length < 1) {
    users.value = []
    return
  }
  debounceTimer = setTimeout(() => {
    searchUsers()
  }, 300)
}

async function searchUsers() {
  if (searchQuery.value.length < 1) return
  
  loading.value = true
  error.value = null
  
  try {
    const response = await authApi.searchUsers(searchQuery.value)
    users.value = response.data.users || []
  } catch (e) {
    console.error('Search failed:', e)
    error.value = 'Search failed. Please try again.'
    users.value = []
  } finally {
    loading.value = false
  }
}

function selectUser(user) {
  emit('select', user)
  searchQuery.value = ''
  users.value = []
  emit('close')
}
</script>

<style scoped>
.modal-enter-active,
.modal-leave-active {
  transition: opacity 0.2s ease;
}
.modal-enter-from,
.modal-leave-to {
  opacity: 0;
}
</style>
