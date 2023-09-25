how to:
```ruby
test_hash = {"do_we_test?" => true,
                "more_data" => [
                    1.2,
                    2.3,
                    nil,
                ],
                1428 => false,
                :another_key => "data string",
            }
data = __test_serialize(test_hash)
__debug_save(data, "debug_data.bin")
```

legend:

|    |                         |
|----|-------------------------|
| u  | unsigned                |
| s  | signed                  |
| le | little endian           |
| be | big endian              |
| …  | not representable ASCII |
| θ  | 0 / NULL                |


| type         | size (in bytes) | representation |
|--------------|-----------------|----------------|
| byte         | 1               | u, le          |
| mrb_int      | 8               | s, le          |
| mrb_float    | 8               | s, le          |
| st_counter_t | 2               | u, le          |

| ruby type         | id |
|-------------------|----|
| ST_FALSE          | 0  |
| ST_TRUE           | 1  |
| ST_INT            | 2  |
| ST_FLOAT          | 3  |
| ST_SYMBOL         | 4  |
| ST_HASH           | 5  |
| ST_ARRAY          | 6  |
| ST_STRING         | 7  |
| ST_UNDEF (unused) | 8  |
| ST_NIL            | 9  |

every ruby types uses one byte for identification

example data:
```
|    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII            |                                          | text             |
|------------------------------------------------------|------------------|
| 00 | 05 04 00 07 0C 00 64 6F 5F 77 65 5F 74 65 73 74 | ……θ……θdo_we_test |
| 10 | 3F 00 01 07 0A 00 6D 6F 72 65 5F 64 61 74 61 00 | ?θ………θmore_dataθ |
| 20 | 06 03 00 03 33 33 33 33 33 33 F3 3F 03 66 66 66 | ……θ…333333…?…fff |
| 30 | 66 66 66 02 40 09 02 94 05 00 00 00 00 00 00 00 | fff…@…………θθθθθθθ |
| 40 | 04 0C 00 61 6E 6F 74 68 65 72 5F 6B 65 79 00 07 | ……θanother_keyθ… |
| 50 | 0C 00 64 61 74 61 20 73 74 72 69 6E 67 00       | …θdata_stringθ   |
```

```
(00)    05                          type            ST_HASH
(01)    04 00                       st_counter_t    4
(03)    07                          type            ST_STRING
(04)    0C 00                       st_counter_t    12
(06)                                CSTRING         do_we_test?θ
(12)    01                          type            ST_TRUE
(13)    07                          type            ST_STRING
(14)    0A 00                       st_counter_t    10
(16)                                CSTRING         more_dataθ
(20)    06                          type            ST_ARRAY
(21)    03 00                       st_counter_t    3    
(23)    03                          type            ST_FLOAT
(24)    33 33 33 33 33 33 F3 3F     mrb_float       1.2
(2C)    03                          type            ST_FLOAT
(2D)    66 66 66 66 66 66 02 40     mrb_float       2.3
(35)    09                          type            ST_NIL
(36)    02                          type            ST_INT
(37)    94 05 00 00 00 00 00 00     mrb_int         1428
(3F)    00                          type            ST_FALSE
(40)    04                          type            ST_SYMBOL
(41)    0C 00                       st_counter_t    12
(42)                                CSTRING         another_keyθ
(4F)    07                          type            ST_STRING
(50)    0C 00                       st_counter_t    12
(52)                                CSTRING         data stringθ
```





