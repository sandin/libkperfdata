# libkperfdata

Reverse engineering of kperfdata.


## Build

```bash
$ mkdir build && cd build
$ cmake ..
```

## Usage

```c
#include "kperfdata/kperfdata.h"

kpdecode_cursor* cursor = kpdecode_cursor_create();

kpdecode_cursor_set_option(cursor, 1, 0);
kpdecode_cursor_setchunk(cursor, buffer, buffer_size);

while (1) {
  kpdecode_record* record = NULL;
  kpdecode_cursor_next_record(cursor, record);
  if (record == NULL) {
    break;
  }

  // do something with the record...

  kpdecode_record_free(record);
}

kpdecode_cursor_clearchunk(cursor);
kpdecode_cursor_free(cursor);
```