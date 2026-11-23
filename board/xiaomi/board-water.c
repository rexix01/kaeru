//
// SPDX-FileCopyrightText: 2026 rexix01 <rexix01@proton.me> & claxten10 <claxten10@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <board_ops.h>

void board_early_init(void) {
    printf("Entering early init for water\n");

    uint32_t addr = SEARCH_PATTERN(LK_START, LK_END,
        0xB570, 0x2304, 0xB086, 0xF243, 0x0505, 0x4606);

    // sec_get_lock_state_adapter (0x4801F8F0) normally combines
    // seccfg_get_lock_state with custom_get_lock_state -> mi_get_lock_state.
    // Tail-call its original seccfg getter instead of consulting RPMB.
    if (addr && READ16(addr + 0x1E) == 0xF046 &&
                READ16(addr + 0x20) == 0xFE13) {
        uint32_t native = DECODE_BL_TARGET(addr + 0x1E); // 0x48066538
        printf("Routing lock state reads at 0x%08X to seccfg 0x%08X\n", addr, native);
        PATCH_BRANCH(addr, native);
    } else {
        printf("water: seccfg read adapter not found\n");
    }

    addr = SEARCH_PATTERN(LK_START, LK_END,
        0xB510, 0x4604, 0xF046, 0xFE0C, 0xB160);

    // sec_set_lock_state_adapter (0x4801F978) writes seccfg first, then
    // custom_set_lock_state -> mi_set_lock_state requires Xiaomi's signed
    // unlock data and can reject the operation. Use only the native setter;
    // its argument and return value pass through unchanged.
    if (addr) {
        uint32_t native = DECODE_BL_TARGET(addr + 4); // 0x48066598
        printf("Routing lock state writes at 0x%08X to seccfg 0x%08X\n", addr, native);
        PATCH_BRANCH(addr, native);
    } else {
        printf("water: seccfg write adapter not found\n");
    }
}

void board_late_init(void) {
    printf("Entering late init for water\n");

    uint32_t addr = SEARCH_PATTERN(LK_START, LK_END,
        0xB530, 0xB083, 0xAB02, 0x2200, 0x4604, 0xF843, 0x2D04);
    if (addr) {
        printf("Disabling dm-verity corruption warning at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    } else {
        printf("water: dm-verity corruption warning not found\n");
    }
}
