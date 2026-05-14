<template>
  <div class="reaction-bar flex items-center gap-1 bg-white rounded-full px-2 py-1 shadow-sm">
    <button
      v-for="reaction in reactionsList"
      :key="reaction.type"
      class="reaction-btn flex items-center gap-1 px-2 py-1 rounded-full transition-all"
      :class="[
        isReacted(reaction.type)
          ? 'bg-blue-100 text-blue-600'
          : 'hover:bg-gray-100 text-gray-600'
      ]"
      @click="handleToggle(reaction.type)"
    >
      <span class="text-sm">{{ reaction.emoji }}</span>
      <span v-if="getCount(reaction.type) > 0" class="text-xs font-medium">
        {{ getCount(reaction.type) }}
      </span>
    </button>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useReactionsStore } from '@/stores/reactions'

const props = defineProps({
  channelId: {
    type: [Number, String],
    required: true
  },
  messageId: {
    type: [Number, String],
    required: true
  },
  reactions: {
    type: Object,
    default: () => ({})
  }
})

const reactionsStore = useReactionsStore()

const reactionsList = [
  { type: 'like', emoji: '👍' },
  { type: 'dislike', emoji: '👎' },
  { type: 'heart', emoji: '❤️' },
  { type: 'fire', emoji: '🔥' },
  { type: 'okay', emoji: '👌' },
  { type: 'LOL', emoji: '😂' },
  { type: 'smile', emoji: '😊' }
]

const reactionKey = computed(() => `${props.channelId}-${props.messageId}`)

function getCount(type) {
  const allReactions = reactionsStore.reactions[reactionKey.value] || props.reactions
  return allReactions[type] || 0
}

function isReacted(type) {
  return getCount(type) > 0
}

async function handleToggle(type) {
  await reactionsStore.toggleReaction(props.channelId, props.messageId, type)
}
</script>
