#include "kperfdata/kperfdata.h"

#include <gtest/gtest.h>

using namespace kperfdata;

#define TEST_DIR "../../test/data/"
#define READ_CONTENT_FROM_FILE(filename)                                   \
  do {                                                                     \
    FILE* f = fopen(TEST_DIR filename, "rb");                              \
    if (!f) {                                                              \
      printf("can not open `%s` file\n", TEST_DIR filename);               \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    struct stat filestats;                                                 \
    stat(TEST_DIR filename, &filestats);                                   \
    buffer_size = filestats.st_size;                                       \
    if (buffer_size == 0) {                                                \
      printf("can not get `%s` file size\n", TEST_DIR filename);           \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    buffer = (char*)malloc(buffer_size);                                   \
    fread(buffer, 1, buffer_size, f);                                      \
    fclose(f);                                                             \
                                                                           \
    if (buffer == nullptr) {                                               \
      printf("can not read `%s` file\n", TEST_DIR filename);               \
      EXPECT_TRUE(false);                                                  \
      return;                                                              \
    }                                                                      \
    printf("filename=%s, buffer=%p, buffer_size=%zu\n", TEST_DIR filename, \
           static_cast<void*>(buffer), buffer_size);                       \
  } while (0)
//  idevice::hexdump(buffer, buffer_size, 0);                              \

TEST(kperfdata, Decode) {
  constexpr long kOk = 0;
  long ret = 0;

  char* buffer = NULL;
  size_t buffer_size = 0;
  READ_CONTENT_FROM_FILE("coreprofilesessiontap.bin");

  kpdecode_cursor* cursor = kpdecode_cursor_create();
  ASSERT_TRUE(cursor != NULL);

  kpdecode_cursor_set_option(cursor, 1, 0);
  ret = kpdecode_cursor_setchunk(cursor, buffer, buffer_size);
  ASSERT_EQ(ret, kOk);

  int record_count = 0;
  while (true) {
    kpdecode_record* record = NULL;
    kpdecode_cursor_next_record(cursor, &record);
    if (record == NULL) {
      break;
    }

    printf("record: %d\n", record_count);

    // ...

    kpdecode_record_free(record);
    record_count += 1;
  }
  ASSERT_TRUE(record_count > 0);

  kpdecode_cursor_clearchunk(cursor);
  kpdecode_cursor_free(cursor);
}
