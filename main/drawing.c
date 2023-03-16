
/*****************************************************************************
 * Made with junk food, tears and sleepless nights in Florida and a Vegas hotel
 *
 * (C) Copyright 2022 Black Badge Raffle Industries, LLC (http://www.blackbadgeraffle.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL JULY 30th, 2022 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge (may the odds be ever in your favor), or just think it's neat,
 * ping me on Twitter @DefconRaffle or consider buying me a drink or a badge.
 * I like collecting and tinkering with badges and learing about the process
 * that others undertake to make them.
 * 
 * Most importantly: You must be present to win!
 *
 * Please read the notice file for additional credits
 * 
 *****************************************************************************/

#ifdef CONFIG_BADGE_TYPE_MASTER

#include "system.h"


#define _DRAWINGTPL_MAP \
  "A(S(UuvvU))" /* Mapping string for TPL to pack and unpack draw entry data */

const static char *TAG = "RAFFLE::Drawing";
static SemaphoreHandle_t m_mutex = NULL;
static StaticSemaphore_t m_mutex_buffer;


static drawing_t *p_drawings = NULL;

static int __timestamp_sort(drawing_t *a, drawing_t *b)
{
  return a->timestamp - b->timestamp;
}

static uint8_t __count_flags(uint16_t n) {
  uint8_t count = 0;
  while(n) {
    count++;
    n = n & (n-1);
  }
  
  return count;
}

uint32_t drawings_count()
{
  drawings_mutex_lock();
  return HASH_COUNT(p_drawings);
  drawings_mutex_unlock();
}

void drawing_start(){
  // start the chaos
  mesh_flag_add(MESH_RAFFLE_STARTED);
}

void drawing_select_winner(){
  // select from entrant badges seen in last 5 minutes
  // for each entrant badge, roll a number

  uint32_t now = time_manager_now_sec();
  drawing_t *s;
  uint32_t winning_badge_serial = 0;
  uint32_t badge_high_roll = 0;

  drawings_mutex_lock();

  for (s = p_drawings; s != NULL; s = s->hh.next)
  {
    
    if (s->timestamp > (now - DRAWING_NEARBY_TIME_SEC) && s->timestamp <= now)
    {

      // If you didnt earn your way in, you can't win
      if(s->badge_type != BADGE_TYPE_ENTRANT) continue;
      uint8_t rolls = __count_flags(s->unlock_flags) +2; // Make sure everyone gets at least a FEW rolls

      uint8_t i;
      for(i=0; i < rolls; i++ ){
        uint32_t roll = esp_random()%65535;
        printf("%8x - Turn %d/%d rolled %u\r\n", s->serial, i+1, rolls, roll);
        if (roll == badge_high_roll){
          printf("Tiebreaker...\r\n");
          uint32_t flip = esp_random()%2;
          if(flip == 1) { // heads you win
            printf("%8x is the new king of the hill... for now\r\n", s->serial);
            winning_badge_serial = s->serial;
          }
        }
        if (roll > badge_high_roll){
          badge_high_roll = roll;
          winning_badge_serial = s->serial;
          printf("%8x is the new king of the hill... for now\r\n", s->serial);
        }
      }
    }
  }
  drawings_mutex_unlock();
  printf("%8x is the winner of the DC30 Black Badge Raffle. The winning number was %u.\r\n", winning_badge_serial, badge_high_roll);
  mesh_set_winner(winning_badge_serial);
}

/**
 * @brief Dump drawings to console
 */
void drawings_dump()
{
  uint32_t now = time_manager_now_sec();
  drawing_t *s;

  uint32_t total = 0;
  uint32_t counter = 0;
  drawings_mutex_lock();
  for (s = p_drawings; s != NULL; s = s->hh.next)
  {
    total++; // count all drawings

    // Only show drawings from the last 2 minutes
    if (s->timestamp > (now - DRAWING_NEARBY_TIME_SEC) && s->timestamp <= now)
    {
      // First drawing print out header
      if (counter == 0)
      {
        printf("  ====================drawingS=====================\n");
        printf("  #  Serial\tType\tFlags\t\t\tLast Seen (sec)\n");
      }
      char flags[20];
      itoa(s->unlock_flags,flags,2);
      printf("  %u: %08x\t%d\t%16s\t%lu\n", counter++, s->serial, s->badge_type, flags,
             now - s->timestamp);
             
    }
  }
  drawings_mutex_unlock();
  if (counter == 0)
  {
    printf("  No drawings nearby right now :(\n");
  }
  else
  {
    printf("  ====================drawingS=====================\n");
  }
  printf("  This badge has seen %d drawings in its lifetime.\n\n", total);
}


drawing_t *drawings_get_by_serial(uint32_t serial)
{
  drawing_t *drawing = NULL;
  drawings_mutex_lock();
  HASH_FIND_INT(p_drawings, &serial, drawing);
  drawings_mutex_unlock();
  return drawing;
}

drawing_t *drawings_hashtable_get()
{
  drawings_mutex_lock();
  return p_drawings;
  drawings_mutex_unlock();
}

bool drawings_init()
{
  m_mutex = xSemaphoreCreateRecursiveMutexStatic(&m_mutex_buffer);
  m_mutex = xSemaphoreCreateMutexStatic(&m_mutex_buffer);
  
  util_heap_stats_dump();
  return true;
}

/**
 * Load drawings from SD
 */
void drawings_load()
{
  // Make sure SD card is inserted
  if (!drv_sd_mounted())
  {
    ESP_LOGE(TAG, "SD card not mounted, cannot save drawings.");
    return;
  }

  uint32_t fsize = util_file_size(DRAWING_FILE);
  if (fsize == 0)
  {
    ESP_LOGE(TAG, "drawings file is 0 bytes.");
    return;
  }

  // Open the file
  FILE *file = fopen(DRAWING_FILE, "r");
  if (!file)
  {
    ESP_LOGE(TAG, "Unable to open '%s' for read. errno=%d", DRAWING_FILE, errno);
    return;
  }

  // Get some heap to store serialized drawings data
  void *buffer = util_heap_alloc_ext(fsize);

  // Read in the raw bytes
  if (fread(buffer, 1, fsize, file) == fsize)
  {
    // Unpack TPL data
    drawing_t drawing;
    tpl_node *tn;
    ESP_LOGD(TAG, "Mapping TPL");
    tn = tpl_map(_DRAWINGTPL_MAP, &drawing, STATE_NAME_LENGTH);
    ESP_LOGD(TAG, "%sLoading TPL from buffer", LOG_BLUE);

    // Read the buffer
    tpl_load(tn, TPL_MEM | TPL_PREALLOCD, buffer, fsize);

    // Unpack one drawing at a time
    while (tpl_unpack(tn, 1) > 0)
    {
      drawings_update_or_create(&drawing);
    }

    tpl_free(tn);
  }
  //drawings_dump();
  // Cleanup
  free(buffer);
  fclose(file);
}

uint16_t drawings_nearby_count()
{
  uint16_t count = 0;
  drawing_t *p;
  drawing_t drawing;
  uint32_t now = time_manager_now_sec();
  drawings_mutex_lock();
  for (p = p_drawings; p != NULL; p = p->hh.next)
  {
    drawing = *p;

    // If nearby, add to counter
    if (drawing.timestamp > (now - DRAWING_NEARBY_TIME_SEC) ||
        (now < DRAWING_NEARBY_TIME_SEC))
    {
      count++;
    }
  }
  drawings_mutex_unlock();
  return count;
}

void IRAM_ATTR drawings_save()
{
  drawing_t *s;

  // Make sure SD card is inserted
  if (!drv_sd_mounted())
  {
    ESP_LOGE(TAG, "SD card not mounted, cannot save drawings.");
    return;
  }

  ESP_LOGD(TAG, "Saving drawings");
  drawings_mutex_lock();
  // Walk through each drawing
  s = p_drawings;
  drawing_t drawing;
  tpl_node *tn = tpl_map(_DRAWINGTPL_MAP, &drawing, STATE_NAME_LENGTH);
  for (s = p_drawings; s != NULL; s = s->hh.next)
  {
    drawing = *s;
    tpl_pack(tn, 1);
  }
  drawings_mutex_unlock();

  // Write to memory
  size_t size;
  tpl_dump(tn, TPL_GETSIZE, &size);
  void *buffer = util_heap_alloc_ext(size);
  tpl_dump(tn, TPL_MEM | TPL_PREALLOCD, buffer, &size);
  tpl_free(tn);

  FILE *file = fopen(DRAWING_FILE, "w");
  if (!file)
  {
    ESP_LOGE(TAG, "Could not open '%s' for write. errno=%d", DRAWING_FILE, errno);
    return;
  }

  // Write to file
  fwrite(buffer, size, 1, file);
  // tpl_dump(tn, TPL_FD, fileno(file));
  fclose(file);



  ESP_LOGD(TAG, "drawings serialized. Size=%d", size);

  free(buffer);
}

/**
 * @brief Ensure local drawing database is updated with a drawing
 * @param other_drawing : drawing to add or update the DB with
 * @param hello : Set to true if badge should try to say hello
 * @return The authoritative copy of the drawing
 */
drawing_t *drawings_update_or_create(drawing_t *other_drawing)
{
  drawing_t *drawing;

  drawings_mutex_lock();
  // Look for drawing in the table
  HASH_FIND_INT(p_drawings, &other_drawing->serial, drawing);
  if (drawing == NULL)
  {
    drawing = (drawing_t *)util_heap_alloc_ext(sizeof(drawing_t));
    drawing->serial = other_drawing->serial;
    HASH_ADD_INT(p_drawings, serial, drawing);

  }

  // At this point drawing is guaranteed to be in the hashtable
  drawing->badge_type = other_drawing->badge_type;
  drawing->serial = other_drawing->serial;
  drawing->unlock_flags = other_drawing->unlock_flags;
  drawing->timestamp = time_manager_now_sec();

  HASH_SORT(p_drawings, __timestamp_sort);
  while (HASH_COUNT(p_drawings) > DRAWING_MAX)
  {
    drawing = p_drawings;
    while (drawing->hh.next != NULL)
    {
      drawing = drawing->hh.next;
    }
    HASH_DEL(p_drawings, drawing);
    ESP_LOGD(TAG, "Deleting drawing %lld", drawing->serial);
    free(drawing);
  }
  drawings_mutex_unlock();
  // if (hello) {
  //   // ESP_LOGD(TAG, "%sQueueing %d for hello", LOG_YELLOW, drawing->company_id);
  //   xQueueSendToFront(m_hello_evt_queue, drawing, 1000 / portTICK_PERIOD_MS);
  // }

  //	ESP_LOGD(TAG, "Added drawing %s[%d]", other_drawing->name,
  //(int)other_drawing->address);
  //	__drawings_dump();

  // Return pointer to authoritative copy of the drawing
  return drawing;
}

void drawings_process(drawing_t drawing){
  drawings_update_or_create(&drawing);
}

void drawings_mutex_lock(){
  xSemaphoreTake(m_mutex,portMAX_DELAY);
}
void drawings_mutex_unlock(){
  xSemaphoreGive(m_mutex);
}


#endif