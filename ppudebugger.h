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
            palette--;
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





}


#endif /* PPUDEBUGGER_H */
