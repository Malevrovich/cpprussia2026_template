<template>
  <span
    class="status-indicator inline-block relative cursor-default"
    :style="{ backgroundColor: color }"
    :title="statusText"
  ></span>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  status: {
    type: String,
    required: true,
    validator: (value) => ['online', 'away', 'busy', 'offline'].includes(value)
  }
})

const colorMap = {
  online: '#22c55e',
  away: '#eab308',
  busy: '#ef4444',
  offline: '#9ca3af'
}

const textMap = {
  online: 'Online',
  away: 'Away',
  busy: 'Busy',
  offline: 'Offline'
}

const color = computed(() => colorMap[props.status] || colorMap.offline)
const statusText = computed(() => textMap[props.status] || 'Offline')
</script>

<style scoped>
.status-indicator {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}
</style>
