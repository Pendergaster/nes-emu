/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef PPUDEBUGGER_H
#define PPUDEBUGGER_H


struct nk_image renderImage;
struct nk_image patternImage[2];

static void
ppu_debugger_init() {

    renderImage = nk_image_id(ppu.screen.tex);
    patternImage[0] = nk_image_id(ppu.pattern[0].tex);
    patternImage[1] = nk_image_id(ppu.pattern[1].tex);
}

static void
ppu_debugger_draw() {

    if (nk_begin(ctx, "ppu debugger", nk_rect(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 300, 300, 300),
                NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_NO_SCROLLBAR|
                NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {

        nk_layout_row_static(ctx, 30, 80, 1);
        static int palette;

        //if(nk_button_label(ctx, "renderpalette")) {
        ppu_render_patterntable(0, palette % 8);
        imageview_update(&ppu.pattern[0]);

        ppu_render_patterntable(1, palette % 8);
        imageview_update(&ppu.pattern[1]);
        //}

        char reqString[64];
        sprintf (reqString,"palette %d", palette % 8);
        nk_label(ctx, reqString, NK_TEXT_LEFT);

        nk_layout_row_static(ctx, 30, 100, 2);
        if(nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT)) {
            palette = palette == 0 ? 7 : palette - 1;
        }
        if(nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT)) {
            palette++;
        }

    }
    nk_end(ctx);

    if (nk_begin(ctx, "game view", nk_rect(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 300 * 2, 300, 300),
                NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_NO_SCROLLBAR|
                NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {

        struct nk_rect bounds;
        bounds = nk_window_get_bounds(ctx);

        if(bounds.h > 60)
            bounds.h -= 60;


        if(bounds.w > 15)
            bounds.w -= 15;

        float target = (float)TEX_WIDTH/ (float)TEX_HEIGHT;
        float ratio = (float)bounds.w/ (float)bounds.h;

        if(ratio > target) { // larger width

            nk_layout_space_begin(ctx, NK_STATIC, bounds.h, 1);
            int gap = (bounds.w - bounds.h * target) / 2;
            nk_layout_space_push(ctx, nk_rect(gap, 0, bounds.h *  target, bounds.h));
            nk_image(ctx, renderImage);

            nk_layout_space_end(ctx);
        } else { // larger height

            nk_layout_space_begin(ctx, NK_STATIC, bounds.h, 1);

            target = (float)TEX_HEIGHT/ (float)TEX_WIDTH;
            int gap = (bounds.h - bounds.w * target) / 2;
            nk_layout_space_push(ctx, nk_rect(0, gap, bounds.w, bounds.w *  target));
            nk_image(ctx, renderImage);

            nk_layout_space_end(ctx);
        }
    }

    nk_end(ctx);



    if (nk_begin(ctx, "pattern view", nk_rect(SCREEN_WIDTH - 300 * 2, SCREEN_HEIGHT - 300, 300, 300),
                NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_NO_SCROLLBAR|
                NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {

        struct nk_rect bounds;
        bounds = nk_window_get_bounds(ctx);

        if(bounds.h > 60)
            bounds.h -= 60;


        if(bounds.w > 15)
            bounds.w -= 15;

        bounds.w /= 2.0;

        nk_layout_space_begin(ctx, NK_STATIC, bounds.h, 2);
        for(int i = 0; i < 2; i++) {

            float woffset = bounds.w * i;
            float target = (float)ppu.pattern[i].w/ (float)ppu.pattern[i].h;
            float ratio = (float)bounds.w/ (float)bounds.h;

            if(ratio > target) { // larger width

                int gap = (bounds.w - bounds.h * target) / 2;
                nk_layout_space_push(ctx, nk_rect(gap + woffset, 0,
                            bounds.h *  target * 0.9, bounds.h * 0.9));
                nk_image(ctx, patternImage[i]);

                nk_layout_space_end(ctx);
            } else { // larger height

                target = (float)ppu.pattern[i].h/ (float)ppu.pattern[i].w;
                int gap = (bounds.h - bounds.w * target) / 2;
                nk_layout_space_push(ctx, nk_rect(0 + woffset, gap,
                            bounds.w * 0.9, bounds.w *  target * 0.9));
                nk_image(ctx, patternImage[i]);

                nk_layout_space_end(ctx);
            }
        }
    }

    nk_end(ctx);

    if (nk_begin(ctx, "PPU nametable debugger", nk_rect(0, 600, 790, 680),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        static u32 selectedAttribute = numeric_max_u32;
        static u8  selectedAttributeValue = 0;
        static u32 selectedAttributeIndex = numeric_max_u32;

        nk_layout_row_static(ctx, 15, 200, 1);
        switch(cartridge.mirrorType){
            case VERTICAL:
                {
                    nk_label(ctx, "Vertical Mirroring", NK_TEXT_LEFT);
                } break;
            case HORIZONTAL:
                {
                    nk_label(ctx, "Horizontal Mirroring", NK_TEXT_LEFT);
                } break;
            default:
                ABORT("unknown mirroring type");

        }

        nk_layout_row_static(ctx, 15, 200, 2);
        static int active = 1;

        active ^= 1;
        if(nk_checkbox_label(ctx, "First", &active)) {
            selectedAttribute = numeric_max_u32;
            selectedAttributeValue = 0;
        }
        active ^= 1;
        if(nk_checkbox_label(ctx, "Second", &active)) {
            selectedAttribute = numeric_max_u32;
            selectedAttributeValue = 0;
        }

        nk_layout_row_static(ctx, 15, 20, 32);

        char tableIdString[32];
        // BG IDs
        for(u32 y = 0; y < 30; y++) {
            u32 attributeY = y / 4;
            for(u32 x = 0; x < 32; x++) {

                u32 attributeX = x / 4;

                sprintf(tableIdString, "%02X",
                        ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + y * 32 + x]);

                if((attributeY * (32 / 4) + attributeX) == selectedAttribute) {
                    //[0x00][0x01]
                    //[0x10][0x11]
                    u32 temp = (((y / 2) & 0x1) << 1) |  (((x / 2) & 0x1));
                    if(temp == selectedAttributeIndex) {
                        nk_label_colored(ctx, tableIdString, NK_TEXT_LEFT, nk_rgb(0, 200, 200));
                    } else {
                        nk_label_colored(ctx, tableIdString, NK_TEXT_LEFT, nk_rgb(200, 200, 0));
                    }
                } else {
                    nk_label(ctx, tableIdString, NK_TEXT_LEFT);
                }

            }
        }

        nk_layout_row_static(ctx, 15, 200, 1);
        nk_label(ctx, "Attribute Table", NK_TEXT_LEFT);

        nk_layout_row_static(ctx, 15, 20, 32);
        // Palettes IDs
        for(u32 y = 0; y < 2; y++) {
            for(u32 x = 0; x < 32; x++) {
                sprintf(tableIdString, "%02X",
                        ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + (y + 30) * 32 + x]);
                i32 selected = (y * 32 + x) == selectedAttribute;
                //nk_label(ctx, tableIdString, NK_TEXT_LEFT);

                if(nk_selectable_label(ctx, tableIdString, NK_TEXT_LEFT, &selected)) {
                    //LOG("Attrinute table 0x0%X", y * 32 + x);
                    selectedAttribute = selected ? (y * 32 + x) : numeric_max_u32;
                    selectedAttributeValue = selected ?
                        ppu.nameTables[(NAMETABLE_SIZE * (active ^ 1)) + (y + 30) * 32 + x] : 0;
                }
            }
        }

        char binaryText[32];
        nk_layout_row_static(ctx, 15, 200, 1);
        nk_label(ctx, "Attribute View", NK_TEXT_LEFT);
        u8 tempAttribVal = selectedAttributeValue;

        nk_layout_row_static(ctx, 15, 20, 2);
        for(u32 i = 0; i < 4; i++) {
            u8 paletteID = tempAttribVal & 0x3;
            sprintf(binaryText, "%c%c", paletteID & 0x2 ? '1' : '0', paletteID & 0x1 ? '1' : '0');
            i32 selected = i == selectedAttributeIndex;
            if(nk_selectable_label(ctx, binaryText, NK_TEXT_LEFT, &selected)) {
                selectedAttributeIndex = selected ? i : numeric_max_u32;
            }
            tempAttribVal >>= 2;
        }


    }
    nk_end(ctx);
}


#endif /* PPUDEBUGGER_H */
