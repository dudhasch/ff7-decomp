# Unnamed-data map: eye/mouth texture tables (KawaiLoadEyesMouthTexToVram)

Research output for the `KawaiLoadEyesMouthTexToVram` tier of field.c. These
three symbols were the blockers to writing a verified-correct body. All offsets
verified against `asm/us/field/nonmatchings/field/KawaiLoadEyesMouthTexToVram.s`
and the data definitions in `asm/us/field/data/3A5B8.data.s`.

## The three symbols

| Symbol | Kind | Meaning |
| --- | --- | --- |
| `D_800DFCA0` | `u_long*` (single `.word`) | Pointer to the loaded model texture block. Set at runtime to `0x801E01F0` by `LoadLocalFieldModelAndInitAll`; used as a DS_read destination in `FieldModelLoadGlobalModels`. The code reads `base = *D_800DFCA0; texData = *(base + 8)` then `LoadImage(..., texData + (index << 9))`. So `+8` within the block is the offset of the texture pixel data (TIM image data past the TIM header flags/bbox). |
| `D_800DFCA4` | `u8[]` mouth/face table | Per-`textureFaceId` mouth-texture index, **stride 7**, `0x7E` = "no texture this slot". Accessed `D_800DFCA4[textureFaceId * 7 + faceByte]`, result `<< 9` (×0x200, one texture-page-sized step into the block). |
| `D_800DFD94` | `u8[]` eye table | Per-`textureFaceId` eye-texture index, **stride 3**, `0x7E` = absent. Accessed `D_800DFD94[textureFaceId * 3 + eyeByte]`, result `<< 9`. |

## Table shapes (from the data)

`D_800DFCA4` — sentinel `0x7E` at offsets 0, 7, 14, 21, 28, 35 (stride 7):

```
7E 00 01 02 03 04 07   7E 08 09 0A 0B 0C 0F   7E 10 11 12 13 14 17 ...
```

`D_800DFD94` — sentinel `0x7E` at offsets 0, 3, 6, 9, ... (stride 3):

```
7E 05 06   7E 0D 0E   7E 15 16   7E 1D 1E   7E 26 27 ...
```

The `0x7E` sentinel is the "this face has no eye/mouth variant" marker; entries
below it are the texture-page index within the model's texture block. The
`*7` / `*3` strides are visible in the target as the `sll3;subu` (= ×7) and
`sll1;addu` (= ×3) strength-reduced multiplies on `textureFaceId`.

## The function's data flow (now fully resolved)

```
s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model /*arg0, +0x15 = textureFaceId*/,
                                u8* faceSel /*arg1, 4 selector bytes*/) {
    RECT rect;                       // at sp+0x10
    u8 faceId = model->textureFaceId;
    if (faceSel[3] >= 0x21) return 1;         // no animated face -> done
    // mouth (table D_800DFCA4, stride 7):
    idx  = D_800DFCA4[faceId*7 + faceSel[0]]; // <<9 -> byte offset into block
    rect = { (faceSel[3]%4)*16 + 0x300, (faceSel[3]/4)*32 + 0x100, 8, 0x20 };
    LoadImage(&rect, texData + (idx<<9));
    // second mouth frame: faceSel[1], rect.x += 8
    // eyes (table D_800DFD94, stride 3):
    idx  = D_800DFD94[faceId*3 + faceSel[2]];
    rect = { (faceSel[3]%8)*8 + 0x300, (faceSel[3]/8)*32 + 0x1A0, 8, 0x20 };
    LoadImage(&rect, texData + (idx<<9));
    return 1;
}
```

The `0x7E` byte compare in the target (`beq v0, sentinel`) skips the LoadImage
for faces with no texture. The `(faceSel[3] % 4) * 16` / `/ 4 * 32` and `% 8`/
`/ 8` splits are the VRAM tile coordinates of the eye/mouth rectangles; gcc 2.6.3
renders them as the `sra`/`sll`/`subu` pairs seen in the target.

## Status

`KawaiLoadEyesMouthTexToVram` is now pinnable: the data flow is fully resolved,
the only remaining work is writing the C with the RECT literal layout that gcc
turns into the target's `sh` sequence to `sp+0x10..0x16`, and pinning via
`MASPSX_OVERRIDE`. The tables belong in a header as named symbols
(`g_FieldFaceTexIndex` / `g_FieldEyeTexIndex` / `g_FieldModelTextureBlock`)
via `mako.sh symbols add` once the body is confirmed.
